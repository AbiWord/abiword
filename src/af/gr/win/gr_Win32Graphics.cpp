/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#define WIN32_LEAN_AND_MEAN
#define NOWINABLE
#define NOMETAFILE
#define NOSERVICE
#define NOIME
#define NOMCX
#include <windows.h>

#include "gr_Win32Graphics.h"
#include "gr_Win32Image.h"
#include <xap_Win32App.h>

#include <xap_Win32Res_Cursors.rc2>

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_Win32OS.h"

/*****************************************************************/

#define _UUD(a) (a) = tdu((a))

const char* GR_Graphics::findNearestFont(const char* pszFontFamily,
										 const char* pszFontStyle,
										 const char* pszFontVariant,
										 const char* pszFontWeight,
										 const char* pszFontStretch,
										 const char* pszFontSize)
{
	return pszFontFamily;
}

// A small helper class
class private_FontReverter
{
public:
	private_FontReverter(GR_Win32Graphics& gr, GR_Font* pOldFont)
	:	m_gr(gr),
		m_pOldFont(pOldFont)
	{
	}
	~private_FontReverter()
	{
		if (m_pOldFont)
		{
			m_gr.setFont(m_pOldFont);
		}
		else
		{
			// Oh joy, no old font to revert to... :-<
		}
	}

private:
	private_FontReverter(const private_FontReverter&);	// no impl
	void operator=(const private_FontReverter&);		// no impl

	GR_Win32Graphics&	m_gr;
	GR_Font*			m_pOldFont;
};


/*****************************************************************/

void GR_Win32Graphics::_constructorCommonCode(HDC hdc)
{
	UT_ASSERT(hdc);

	m_hdc = hdc;
	m_hwnd = 0;
	m_iLineWidth = 0;	// default to a hairline
	m_bPrint = false;
	m_bStartPrint = false;
	m_bStartPage = false;
	m_pFont = NULL;
	m_pFontGUI = NULL;

	m_cs = GR_Graphics::GR_COLORSPACE_COLOR;
	m_cursor = GR_CURSOR_INVALID;

	m_clrXorPen = 0;
	m_hXorPen = 0;

	setCursor(GR_CURSOR_DEFAULT);

	m_remapBuffer = NULL;
	m_remapBufferSize = 0;
	m_remapIndices = NULL;

	m_eJoinStyle = JOIN_MITER;
	m_eCapStyle  = CAP_BUTT;
	m_eLineStyle = LINE_SOLID;

	setBrush((HBRUSH) GetStockObject(WHITE_BRUSH));	// Default brush
}

GR_Win32Graphics::GR_Win32Graphics(HDC hdc, HWND hwnd, XAP_App * app)
{
	_constructorCommonCode(hdc);
	m_pApp = app;
	m_hwnd = hwnd;
}

GR_Win32Graphics::GR_Win32Graphics(HDC hdc, const DOCINFO * pDocInfo, XAP_App * app)
{
	_constructorCommonCode(hdc);
	m_pApp = app;
	m_bPrint = true;
	m_pDocInfo = pDocInfo;
}

GR_Win32Graphics::~GR_Win32Graphics()
{
	UT_VECTOR_PURGEALL(UT_Rect*, m_vSaveRect);
	UT_VECTOR_PURGEALL(COLORREF*, m_vSaveRectBuf);
	DELETEP(m_pFontGUI);
	if (m_hXorPen) {
		DeleteObject(m_hXorPen);
	}

	delete [] m_remapBuffer;
	delete [] m_remapIndices;
}

bool GR_Win32Graphics::queryProperties(GR_Graphics::Properties gp) const
{
	switch (gp)
	{
	case DGP_SCREEN:
		return !m_bPrint;
	case DGP_PAPER:
		return m_bPrint;
	case DGP_OPAQUEOVERLAY:
		return true;
	default:
		UT_ASSERT(0);
		return false;
	}
}

GR_Font* GR_Win32Graphics::getGUIFont(void)
{
	if (!m_pFontGUI)
	{
		// lazily grab this (once)
		HFONT f = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
		m_pFontGUI = new GR_Win32Font(f, this);
		UT_ASSERT(m_pFontGUI);
	}

	return m_pFontGUI;
}

extern "C"
int CALLBACK
win32Internal_fontEnumProcedure(ENUMLOGFONT* pLogFont,
								NEWTEXTMETRICEX* pTextMetric,
								int Font_type,
								LPARAM lParam)
{
	LOGFONT *lf = (LOGFONT*)lParam;
	lf->lfCharSet = pLogFont->elfLogFont.lfCharSet;
	return 0;
}

GR_Font* GR_Win32Graphics::findFont(const char* pszFontFamily,
									const char* pszFontStyle,
									const char* pszFontVariant,
									const char* pszFontWeight,
									const char* pszFontStretch,
									const char* pszFontSize)
{
	LOGFONT lf = { 0 };

	/*
		TODO we need to fill out the LOGFONT object such that
		we'll get the closest possible matching font.  For
		now, we're hard-coding a hack.
	*/

	// we need to get the size in logpixels for the current DC, which
	// simply means to divide points by 72 and multiply by device Y resolution
	UT_sint32 iHeight = (UT_sint32)UT_convertToPoints(pszFontSize);
	lf.lfHeight = -MulDiv(/*UT_LOG_UNITS*/(iHeight), GetDeviceCaps(m_hdc, LOGPIXELSY), 72);

	// TODO note that we don't support all those other ways of expressing weight.
	if (0 == UT_stricmp(pszFontWeight, "bold"))
	{
		lf.lfWeight = 700;
	}

	// TODO -- remove this block entirely, since oblique is no longer a valid style
	// We squash oblique into italic
	if (0 == UT_stricmp(pszFontStyle, "italic") || 0 == UT_stricmp(pszFontStyle, "oblique"))
	{
		lf.lfItalic = TRUE;
	}

	// TODO note that we currently think pszFontFamily is a single name, not a list!
	// TODO why don't these generic family names work?!?
	if (0 == UT_stricmp(pszFontFamily, "serif"))
	{
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_ROMAN;
	}
	else if (0 == UT_stricmp(pszFontFamily, "sans-serif"))
	{
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
	}
	else if (0 == UT_stricmp(pszFontFamily, "cursive"))
	{
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_SCRIPT;
	}
	else if (0 == UT_stricmp(pszFontFamily, "fantasy"))
	{
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DECORATIVE;
	}
	else if (0 == UT_stricmp(pszFontFamily, "monospace"))
	{
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_MODERN;
	}
	else
	{
		strcpy(lf.lfFaceName, pszFontFamily);
	}

	// Get character set value from the font itself
	LOGFONT enumlf = { 0 };
	enumlf.lfCharSet = DEFAULT_CHARSET;
	strcpy(enumlf.lfFaceName, lf.lfFaceName);
	EnumFontFamiliesEx(GetDC(NULL), &enumlf,
		(FONTENUMPROC)win32Internal_fontEnumProcedure, (LPARAM)&lf, 0);

	lf.lfOutPrecision = OUT_TT_ONLY_PRECIS;		// Choose only True Type fonts.
	lf.lfQuality = PROOF_QUALITY;

	HFONT hFont = CreateFontIndirect(&lf);
	UT_ASSERT(hFont);
	if (!hFont)
		return 0;

	return new GR_Win32Font(hFont, this);
}

void GR_Win32Graphics::drawGlyph(UT_uint32 Char, UT_sint32 xoff, UT_sint32 yoff)
{
	UT_ASSERT(UT_TODO);
}

void GR_Win32Graphics::drawChar(UT_UCSChar Char, UT_sint32 xoff, UT_sint32 yoff)
{
	xoff = tdu(xoff);
	yoff = tdu(yoff);

	HFONT hFont = GR_Win32Font::Acq::getHFONT(*m_pFont);
	SelectObject(m_hdc, hFont);
	SetTextAlign(m_hdc, TA_LEFT | TA_TOP);
	SetBkMode(m_hdc, TRANSPARENT);		// TODO: remember and reset?

	UT_UCSChar currentChar = remapGlyph(Char, false);
	//TODO this is a temporary hack -- need a way to handle true 32-bit chars
	UT_UCS2Char aChar = (UT_UCS2Char) currentChar;

	if(currentChar == 0x200B || currentChar == 0xFEFF)
		return;
	// Windows NT and Windows 95 support the Unicode Font file.
	// All of the Unicode glyphs can be rendered if the glyph is found in
	// the font file. However, Windows 95 does  not support the Unicode
	// characters other than the characters for which the particular codepage
	// of the font file is defined.
	// Reference Microsoft knowledge base:
	// Q145754 - PRB ExtTextOutW or TextOutW Unicode Text Output Is Blank
	LOGFONT lf;
	int iRes = GetObject(hFont, sizeof(LOGFONT), &lf);
	UT_ASSERT(iRes);
	if (UT_IsWinNT() == false && lf.lfCharSet == SYMBOL_CHARSET)
	{
		// Symbol character handling for Win9x
		char str[sizeof(UT_UCS2Char)];

		int iConverted = WideCharToMultiByte(CP_ACP, NULL,
			(LPCWSTR) &aChar, 1,
			str, sizeof(str), NULL, NULL);
		ExtTextOutA(m_hdc, xoff, yoff, 0, NULL, str, iConverted, NULL);
	}
	else
	{
		// Unicode font and default character set handling for WinNT and Win9x
		ExtTextOutW(m_hdc, xoff, yoff, 0/*ETO_GLYPH_INDEX*/, NULL, (LPCWSTR) &aChar, 1, NULL);
	}
}

void GR_Win32Graphics::drawChars(const UT_UCSChar* pChars,
								 int iCharOffset, int iLengthOrig,
								 UT_sint32 xoff, UT_sint32 yoff,
								 int * pCharWidths)
{
	UT_ASSERT(pChars);

	xoff = tdu(xoff);
	yoff = tdu(yoff);

	// iLength can be modified by _remapGlyphs
	int iLength = iLengthOrig;
	HFONT hFont = GR_Win32Font::Acq::getHFONT(*m_pFont);
	SelectObject(m_hdc, hFont);
	SetTextAlign(m_hdc, TA_LEFT | TA_TOP);
	SetBkMode(m_hdc, TRANSPARENT);		// TODO: remember and reset?

	UT_uint16* currentChars = _remapGlyphs(pChars, iCharOffset, iLength);

	// Windows NT and Windows 95 support the Unicode Font file.
	// All of the Unicode glyphs can be rendered if the glyph is found in
	// the font file. However, Windows 95 does  not support the Unicode
	// characters other than the characters for which the particular codepage
	// of the font file is defined.
	// Reference Microsoft knowledge base:
	// Q145754 - PRB ExtTextOutW or TextOutW Unicode Text Output Is Blank
	LOGFONT lf;
	int iRes = GetObject(hFont, sizeof(LOGFONT), &lf);
	UT_ASSERT(iRes);

	if (UT_IsWinNT() == false && lf.lfCharSet == SYMBOL_CHARSET)
	{
		// Symbol character handling for Win9x
		char* str = new char[iLength * sizeof(UT_UCS2Char)];
		int iConverted = WideCharToMultiByte(CP_ACP, NULL,
			(LPCWSTR) currentChars, iLength,
			str, iLength * sizeof(UT_UCSChar), NULL, NULL);
		ExtTextOutA(m_hdc, xoff, yoff, 0, NULL, str, iConverted, NULL);
		delete [] str;
	}
	else
	{
		int duCharWidths [256];
		int *pCharAdvances = 0;

		if (iLengthOrig > sizeof(duCharWidths))
			pCharAdvances = new int [iLengthOrig];
		else
			pCharAdvances = duCharWidths;

		for (UT_uint32 i = 0; i < iLengthOrig; i++)
			pCharAdvances[i] = tdu (pCharWidths[i]);

		// Unicode font and default character set handling for WinNT and Win9x

		// The bidi aspect of the drawing is handled as follows:
		//     1. The OS has no bidi support: use simple ExTextOut
		//        call, since all the bidi processing has been already
		//        done in the layout engine
		//
		//     2. If the OS has bidi support: we will not use it for
		//        drawing in the main window, only let the system to
		//        handle processing for the GUI. This requires that we
		//        call GetCharacterPlacement function without
		//        requesting reordering and then feed the indices to
		//        ExTextOut (direct call to ExTextOut automatically reorders)
		if(XAP_App::getApp()->theOSHasBidiSupport() != XAP_App::BIDI_SUPPORT_NONE)
		{
			UT_ASSERT(m_remapIndices);
			GCP_RESULTSW gcpResult;
			gcpResult.lStructSize = sizeof(GCP_RESULTS);
			gcpResult.lpOutString = NULL;			// Output string
			gcpResult.lpOrder = NULL;				// Ordering indices
			gcpResult.lpDx = pCharAdvances;	    // Distances between character cells
			gcpResult.lpCaretPos = NULL;			// Caret positions	
			gcpResult.lpClass = NULL;				// Character classifications
#ifdef __MINGW32__
			gcpResult.lpGlyphs = (UINT *) m_remapIndices;	// Character glyphs
#else			
			gcpResult.lpGlyphs = (LPWSTR) m_remapIndices;	// Character glyphs
#endif			
			gcpResult.nGlyphs = m_remapBufferSize;  // Array size

			DWORD placementResult;

			if(XAP_App::getApp()->theOSHasBidiSupport() == XAP_App::BIDI_SUPPORT_GUI)
				placementResult = GetCharacterPlacementW(m_hdc, (LPCWSTR) currentChars, iLength, 0, &gcpResult, 0);
			else
				placementResult = GetCharacterPlacementW(m_hdc, (LPCWSTR) currentChars, iLength, 0, &gcpResult, GCP_REORDER);
			

			if(placementResult)
			{
				ExtTextOutW(m_hdc, xoff, yoff, ETO_GLYPH_INDEX, NULL, (LPCWSTR) m_remapIndices, gcpResult.nGlyphs, s_pCharAdvances);
			}
			else
			{
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				goto simple_exttextout;
			}
		}
		else
		{
simple_exttextout:
			ExtTextOutW(m_hdc, xoff, yoff, 0, NULL, (LPCWSTR) currentChars, iLength, s_pCharAdvances);
		}

		if (iLengthOrig > sizeof(duCharWidths))
			delete[] pCharAdvances;
	}

}

UT_uint16*	GR_Win32Graphics::_remapGlyphs(const UT_UCSChar* pChars, int iCharOffset, int &iLength)
{
	// TODO -- make this handle 32-bit chars properly
	if (iLength > (int)m_remapBufferSize)
	{
		delete [] m_remapBuffer;

		if(XAP_App::getApp()->theOSHasBidiSupport() != XAP_App::BIDI_SUPPORT_NONE)
		{
			delete [] m_remapIndices;
			m_remapIndices = new UT_UCS2Char[iLength];
		}

		m_remapBuffer = new UT_UCS2Char[iLength];
		m_remapBufferSize = iLength;
	}

    // Need to handle zero-width spaces correctly
	int i, j;
	for (i = 0, j = 0; i < iLength; ++i, ++j)
	{
		m_remapBuffer[j] = (UT_UCS2Char)remapGlyph(pChars[iCharOffset + i], false);
		
		if(m_remapBuffer[j] == 0x200B || m_remapBuffer[j] == 0xFEFF)
			j--;
	}

	iLength -= (i - j);

	return m_remapBuffer;
}

void GR_Win32Graphics::setFont(GR_Font* pFont)
{
	UT_ASSERT(pFont);	// TODO should we allow pFont == NULL?

	GR_Win32Font* pWin32Font = static_cast<GR_Win32Font*>(pFont);

// TMN: This currently won't work since there is no way to tell
// GR_Win32Graphics to "drop" a font. If a user of this class:
// 1. Creates a new font on the heap.
// 2. Calls this setFont()
// 3. Deletes the font.
// 4. Creates a new font on the heap.
// 5. Calls this setFont()
// it is possible (and it has happened) that the pointers are the same,
// but it's two different fonts.
//

	// this should work though, the allocation number is unique, even
	// if the pointers are identical
	if (pFont->getAllocNumber() != m_iFontAllocNo)
	{
		m_pFont = pWin32Font;
		m_iFontAllocNo = pFont->getAllocNumber();
		GR_Win32Font::Acq::selectFontIntoDC(*m_pFont, m_hdc);
	}
}

UT_uint32 GR_Win32Graphics::getFontHeight(GR_Font * fnt)
{
	private_FontReverter janitor_(*this, m_pFont);

	setFont(fnt);

	return getFontHeight();
}

UT_uint32 GR_Win32Graphics::getFontHeight()
{
	return GR_Win32Font::Acq::getFontHeight(*m_pFont);
}

UT_uint32 GR_Win32Graphics::getFontAscent(GR_Font* fnt)
{
	private_FontReverter janitor_(*this, m_pFont);

	setFont(fnt);

	return getFontAscent();
}

UT_uint32 GR_Win32Graphics::getFontAscent()
{
	return GR_Win32Font::Acq::getAscent(*m_pFont);
}

UT_uint32 GR_Win32Graphics::getFontDescent(GR_Font* fnt)
{
	private_FontReverter janitor_(*this, m_pFont);

	setFont(fnt);

	return getFontDescent();
}

UT_uint32 GR_Win32Graphics::getFontDescent()
{
	return GR_Win32Font::Acq::getDescent(*m_pFont);
}

void GR_Win32Graphics::getCoverage(UT_Vector& coverage)
{
	coverage.clear();
	coverage.push_back((void*) ' ');
	coverage.push_back((void*) (255 - ' '));
	
	//UT_ASSERT(UT_TODO);
}

UT_uint32 GR_Win32Graphics::measureUnRemappedChar(const UT_UCSChar c)
{
	UT_ASSERT(m_pFont);
	return GR_Win32Font::Acq::measureUnRemappedChar(*m_pFont, c);
}

UT_uint32 GR_Win32Graphics::getDeviceResolution(void) const
{
	return GetDeviceCaps(m_hdc, LOGPIXELSY); // NOTE: assumes square pixels
}

void GR_Win32Graphics::setColor(const UT_RGBColor& clr)
{
	m_curColor = clr;
	_setColor(RGB(clr.m_red, clr.m_grn, clr.m_blu));
}

void GR_Win32Graphics::getColor(UT_RGBColor& clr)
{
	clr = m_curColor;
}

void GR_Win32Graphics::_setColor(DWORD dwColor)
{
	m_clrCurrent = dwColor;
	SetTextColor(m_hdc, m_clrCurrent);
}

void GR_Win32Graphics::drawLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2, UT_sint32 y2)
{
	if (m_eLineStyle == LINE_SOLID &&
		((x1 == x2 && y1 != y2) || (y1 == y2 && x1 != x2))
	 && m_iLineWidth <= tlu(1))
	{
		// TMN: A little bloaty, though a TREMENDOUS speedup (like 45 TIMES
		// last I checked).
		UT_RGBColor color(	GetRValue(m_clrCurrent),
							GetGValue(m_clrCurrent),
							GetBValue(m_clrCurrent));
		const bool bVert = x1 == x2;
		const UT_sint32 nLineWidth = m_iLineWidth ? m_iLineWidth : tlu(1);

		if (bVert)
		{
			if (y2 < y1)
			{
				++y2;
				const UT_sint32 temp = y1;
				y1 = y2;
				y2 = temp;
			}
			else
			{
				--y2;
			}
		}
		else
		{
			if (x2 < x1) {
				++x2;
				const UT_sint32 temp = x1;
				x1 = x2;
				x2 = temp;
			}
			else
			{
				--x2;
			}
		}
		UT_ASSERT(x1 <= x2);
		UT_ASSERT(y1 <= y2);
		fillRect(	color,
					x1, y1,
					nLineWidth + (!bVert ? x2 - x1: 0),
					nLineWidth + ( bVert ? y2 - y1: 0));
		return;
	}

	_UUD(x1);
	_UUD(x2);
	_UUD(y1);
	_UUD(y2);
	
	int penStyle;
	switch(m_eLineStyle)
	{
		case LINE_DOUBLE_DASH:
		case LINE_ON_OFF_DASH:      penStyle = PS_DASH;  break;
		case LINE_SOLID:            penStyle = PS_SOLID; break;
		case LINE_DOTTED:           penStyle = PS_DOT;   break;

		default:
			UT_ASSERT( UT_NOT_IMPLEMENTED );
			penStyle = PS_SOLID;
	}

	HPEN hPen = CreatePen(penStyle, m_iLineWidth, m_clrCurrent);
	HPEN hOldPen = (HPEN) SelectObject(m_hdc, hPen);

	MoveToEx(m_hdc, x1, y1, NULL);
	LineTo(m_hdc, x2, y2);

	(void) SelectObject(m_hdc, hOldPen);
	DeleteObject(hPen);
}

void GR_Win32Graphics::setLineProperties(double inWidthPixels,
										 JoinStyle inJoinStyle,
										 CapStyle inCapStyle,
										 LineStyle inLineStyle)
{
	m_eJoinStyle = inJoinStyle;
	m_eCapStyle  = inCapStyle;
	m_eLineStyle = inLineStyle;
	m_iLineWidth = (int)inWidthPixels;
}


void GR_Win32Graphics::setLineWidth(UT_sint32 iLineWidth)
{
	m_iLineWidth = iLineWidth;
}

void GR_Win32Graphics::xorLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2, UT_sint32 y2)
{
	/*
	  Note that we always use a pixel width of 1 for xorLine, since
	  this should always be done to the screen.
	*/

	_UUD(x1);
	_UUD(x2);
	_UUD(y1);
	_UUD(y2);

	if (m_clrCurrent != m_clrXorPen || !m_hXorPen)
	{
		if (m_hXorPen)
		{
			DeleteObject(m_hXorPen);
		}
		m_hXorPen = CreatePen(PS_SOLID, 1, m_clrCurrent);
		m_clrXorPen = m_clrCurrent;
	}

	int iROP = SetROP2(m_hdc, R2_XORPEN);
	HPEN hOldPen = (HPEN)SelectObject(m_hdc, m_hXorPen);

	MoveToEx(m_hdc, x1, y1, NULL);
	LineTo(m_hdc, x2, y2);

	SelectObject(m_hdc, hOldPen);
	SetROP2(m_hdc, iROP);
}

void GR_Win32Graphics::polyLine(UT_Point * pts, UT_uint32 nPoints)
{
	// TODO do we really want the line width to ALWAYS be 1?
	HPEN hPen = CreatePen(PS_SOLID, 1, m_clrCurrent);
	HPEN hOldPen = (HPEN) SelectObject(m_hdc, hPen);

	POINT * points = (POINT *)UT_calloc(nPoints, sizeof(POINT));
	UT_ASSERT(points);

	for (UT_uint32 i = 0; i < nPoints; i++)
	{
		points[i].x = tdu(pts[i].x);
		points[i].y = tdu(pts[i].y);
	}

	Polyline(m_hdc, points, nPoints);

	(void) SelectObject(m_hdc, hOldPen);
	DeleteObject(hPen);
	FREEP(points);
}

void GR_Win32Graphics::fillRect(const UT_RGBColor& c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h)
{
	RECT r;
	r.left = tdu(x);
	r.top = tdu(y);
	r.right = r.left + tdu(w);
	r.bottom = r.top + tdu(h);

	// This might look wierd (and I think it is), but it's MUCH faster.
	// CreateSolidBrush is dog slow.
	HDC hdc = m_hdc;
	const COLORREF cr = ::SetBkColor(hdc, RGB(c.m_red, c.m_grn, c.m_blu));
	::ExtTextOut(hdc, 0, 0, ETO_OPAQUE, &r, NULL, 0, NULL);
	::SetBkColor(hdc, cr);
}

bool GR_Win32Graphics::startPrint(void)
{
	UT_ASSERT(m_bPrint);
	UT_ASSERT(!m_bStartPrint);
	m_bStartPrint = ( StartDoc(m_hdc,m_pDocInfo) > 0 );

	return m_bStartPrint;
}

bool GR_Win32Graphics::startPage(const char * szPageLabel, UT_uint32 pageNumber,
									bool bPortrait, UT_uint32 iWidth, UT_uint32 iHeight)
{
	_UUD(iWidth);
	_UUD(iHeight);
	if (m_bStartPage)
	{
		EndPage(m_hdc);
	}

	// Correct for Portrait vs Lanscape mode
	DEVMODE pDevMode;
	memset(&pDevMode, 0, sizeof(pDevMode));
	pDevMode.dmSpecVersion = DM_SPECVERSION;
	pDevMode.dmSize = sizeof(DEVMODE);
	pDevMode.dmDriverExtra = 0;
	pDevMode.dmFields = DM_ORIENTATION;
	pDevMode.dmOrientation = (bPortrait) ? DMORIENT_PORTRAIT : DMORIENT_LANDSCAPE;
	ResetDC( m_hdc, &pDevMode );

	const int iRet = StartPage(m_hdc);

	m_bStartPage = iRet > 0;

	if (iRet > 0)
	{
		// PHYSICALOFFSETX returns offset of printable area from the left edge
		// of the physical printer paper. The value returned is in device units.
		// Since the current mapping mode is MM_TEXT, this code _should_ work.
		const POINT ptNew = {
			-GetDeviceCaps(m_hdc, PHYSICALOFFSETX),
			-GetDeviceCaps(m_hdc, PHYSICALOFFSETY)
		};
		SetViewportOrgEx(m_hdc, ptNew.x, ptNew.y, 0);
	}

	return m_bStartPage;
}

bool GR_Win32Graphics::endPrint(void)
{
	if (m_bStartPage)
		EndPage(m_hdc);

	return (EndDoc(m_hdc) > 0);
}

/**
 **  The View calls this when it receives a SetX/YScrollOffset
 **  call.  Move the contents of the window appropriately.  View
 **  will draw after you exit from this call.  View will only
 **  draw into the "exposed" area.
 **
 ** dx & dy are the change in x/y from the current scrolled position
 ** negative values indcate left/up movement, positive right/down movement
 **/
void GR_Win32Graphics::scroll(UT_sint32 dx, UT_sint32 dy)
{
	_UUD(dx);
	_UUD(dy);
	ScrollWindowEx(m_hwnd, -dx, -dy, NULL, NULL, NULL, 0, SW_INVALIDATE);
}

void GR_Win32Graphics::scroll(UT_sint32 x_dest, UT_sint32 y_dest,
							  UT_sint32 x_src, UT_sint32 y_src,
							  UT_sint32 width, UT_sint32 height)
{
	_UUD(x_dest);
	_UUD(y_dest);
	_UUD(x_src);
	_UUD(y_src);
	_UUD(width);
	_UUD(height);
	RECT r;
	r.left = x_src;
	r.top = y_src;
	r.right = r.left + width;
	r.bottom = r.top + height;
	ScrollWindowEx(m_hwnd, (x_dest - x_src), (y_dest - y_src),
				   &r, NULL, NULL, NULL, SW_ERASE);
}

void GR_Win32Graphics::clearArea(UT_sint32 x, UT_sint32 y, UT_sint32 width, UT_sint32 height)
{

//	UT_ASSERT((x + width) < 800);
	_UUD(x);
	_UUD(y);
	_UUD(width);
	_UUD(height);
	
	
	RECT r;
	r.left = x;
	r.top = y;
	r.right = r.left + width;
	r.bottom = r.top + height;

	FillRect(m_hdc, &r, m_hClearBrush);
}

void GR_Win32Graphics::invertRect(const UT_Rect* pRect)
{
	RECT r;

	r.left = tdu(pRect->left);
	r.top = tdu(pRect->top);
	r.right = tdu(pRect->left + pRect->width);
	r.bottom = tdu(pRect->top + pRect->height);

	InvertRect(m_hdc, &r);
}

void GR_Win32Graphics::setClipRect(const UT_Rect* pRect)
{
	// This causes a lot of drawing and screen refresh problems.
	// It can be removed from here without problems, but may
	// not work for other things.  For now leave code in place.
	// return;

	int res;
	m_pRect = pRect;
	if (pRect)
	{
		// set the clip rectangle
		HRGN hrgn = CreateRectRgn(tdu(pRect->left),
								  tdu(pRect->top),
								  tdu(pRect->left + pRect->width),
								  tdu(pRect->top + pRect->height));
		UT_ASSERT(hrgn);

		res = SelectClipRgn(m_hdc, hrgn);

		DeleteObject(hrgn);
	}
	else
	{
		// stop clipping
		res = SelectClipRgn(m_hdc, NULL);
	}

	UT_ASSERT(res != ERROR);
}

GR_Image* GR_Win32Graphics::createNewImage(const char* pszName, const UT_ByteBuf* pBB,
					   UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight,
					   GR_Image::GRType iType)
{
	_UUD(iDisplayWidth);
	_UUD(iDisplayHeight);
	
	GR_Image* pImg = NULL;
	if (iType == GR_Image::GRT_Raster)
		pImg = new GR_Win32Image(pszName);
	else
		pImg = new GR_VectorImage(pszName);

	pImg->convertFromBuffer(pBB, iDisplayWidth,iDisplayHeight);

	return pImg;
}

void GR_Win32Graphics::drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest)
{
	UT_ASSERT(pImg);

	_UUD(xDest);
	_UUD(yDest);
	
	if (pImg->getType() != GR_Image::GRT_Raster)
	{
		pImg->render(this, xDest, yDest);
		return;
	}

	// When Printing yDest must be within Page Height for each
	// Page.
	if (queryProperties(GR_Graphics::DGP_PAPER))
	{
		UT_sint32 nPageHeight = GetDeviceCaps(m_hdc, PHYSICALHEIGHT);
		yDest = ( yDest % nPageHeight );
	}

	GR_Win32Image* pWin32Img = static_cast<GR_Win32Image*>(pImg);

	BITMAPINFO* pDIB = pWin32Img->getDIB();

	UT_uint32 iSizeOfColorData = pDIB->bmiHeader.biClrUsed * sizeof(RGBQUAD);
	UT_Byte* pBits = ((unsigned char*) pDIB) + pDIB->bmiHeader.biSize + iSizeOfColorData;

	int iRes = StretchDIBits(m_hdc,
							 xDest, yDest,
							 tdu(pImg->getDisplayWidth()), tdu(pImg->getDisplayHeight()),
							 0, 0,
							 pDIB->bmiHeader.biWidth, pDIB->bmiHeader.biHeight,
							 pBits, pDIB, DIB_RGB_COLORS, SRCCOPY);

	if (iRes == GDI_ERROR)
	{
		DWORD err = GetLastError();
		UT_DEBUGMSG(("StretchDIBits failed with err %d\n", err));
	}
}

HWND GR_Win32Graphics::getHwnd(void) const
{
	return m_hwnd;
}

void GR_Win32Graphics::setColorSpace(GR_Graphics::ColorSpace c)
{
	// TODO:  maybe?
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

GR_Graphics::ColorSpace GR_Win32Graphics::getColorSpace(void) const
{
	return m_cs;
}

void GR_Win32Graphics::setCursor(GR_Graphics::Cursor c)
{
	// set the cursor type, but wait for a WM_SETCURSOR
	// to do anything about it.
	m_cursor = c;
}

GR_Graphics::Cursor GR_Win32Graphics::getCursor(void) const
{
	return m_cursor;
}

void GR_Win32Graphics::handleSetCursorMessage(void)
{
	// deal with WM_SETCURSOR message.

	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);
	HINSTANCE hinst = pWin32App->getInstance();
	LPCTSTR cursor_name;

	switch (m_cursor)
	{
	default:
		UT_ASSERT(UT_NOT_IMPLEMENTED);
		/*FALLTHRU*/
	case GR_CURSOR_DEFAULT:
		cursor_name = IDC_ARROW;		// top-left arrow
		hinst = NULL;
		break;

	case GR_CURSOR_LINK:
	case GR_CURSOR_GRAB:
		cursor_name = MAKEINTRESOURCE(IDC_ABIHAND);
		break;

	case GR_CURSOR_EXCHANGE:
		cursor_name = MAKEINTRESOURCE(IDC_EXCHANGE);
		break;

	case GR_CURSOR_IBEAM:
		cursor_name = IDC_IBEAM;
		hinst = NULL;
		break;

	case GR_CURSOR_RIGHTARROW:
		cursor_name = IDC_ARROW;		// TODO change this
		hinst = NULL;
		break;

	case GR_CURSOR_LEFTARROW:
		cursor_name = IDC_ARROW;		// TODO change this
		hinst = NULL;
		break;

	case GR_CURSOR_IMAGE:
		cursor_name = IDC_SIZEALL;
		hinst = NULL;
		break;

	case GR_CURSOR_IMAGESIZE_NW:
	case GR_CURSOR_IMAGESIZE_SE:
		cursor_name = IDC_SIZENWSE;
		hinst = NULL;
		break;

	case GR_CURSOR_HLINE_DRAG:
	case GR_CURSOR_UPDOWN:
	case GR_CURSOR_IMAGESIZE_N:
	case GR_CURSOR_IMAGESIZE_S:
		cursor_name = IDC_SIZENS;
		hinst = NULL;
		break;

	case GR_CURSOR_IMAGESIZE_NE:
	case GR_CURSOR_IMAGESIZE_SW:
		cursor_name = IDC_SIZENESW;
		hinst = NULL;
		break;

	case GR_CURSOR_VLINE_DRAG:
	case GR_CURSOR_LEFTRIGHT:
	case GR_CURSOR_IMAGESIZE_E:
	case GR_CURSOR_IMAGESIZE_W:
		cursor_name = IDC_SIZEWE;
		hinst = NULL;
		break;

	case GR_CURSOR_WAIT:
		cursor_name = IDC_WAIT;
		hinst = NULL;
		break;
	}

	HCURSOR hCursor = LoadCursor(hinst,cursor_name);
	if (hCursor != NULL)
	{
		SetCursor(hCursor);
	}
}

void GR_Win32Graphics::setColor3D(GR_Color3D c)
{
	UT_ASSERT(c < COUNT_3D_COLORS);
	_setColor(m_3dColors[c]);
}

void GR_Win32Graphics::init3dColors(void)
{
	m_3dColors[CLR3D_Foreground] = GetSysColor(COLOR_BTNTEXT);
	m_3dColors[CLR3D_Background] = GetSysColor(COLOR_3DFACE);
	m_3dColors[CLR3D_BevelUp]    = GetSysColor(COLOR_3DHIGHLIGHT);
	m_3dColors[CLR3D_BevelDown]  = GetSysColor(COLOR_3DSHADOW);
	m_3dColors[CLR3D_Highlight]  = GetSysColor(COLOR_WINDOW);
}

void GR_Win32Graphics::fillRect(GR_Color3D c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h)
{
	UT_ASSERT(c < COUNT_3D_COLORS);
	HBRUSH hBrush = CreateSolidBrush(m_3dColors[c]);

	RECT r;
	r.left = tdu(x);
	r.top = tdu(y);
	r.right = r.left + tdu(w);
	r.bottom = r.top + tdu(h);

	FillRect(m_hdc, &r, hBrush);
	DeleteObject(hBrush);
}

void GR_Win32Graphics::fillRect(GR_Color3D c, UT_Rect &r)
{
	UT_ASSERT(c < COUNT_3D_COLORS);
	fillRect(c,r.left,r.top,r.width,r.height);
}

//////////////////////////////////////////////////////////////////
// This is a static method in the GR_Font base class implemented
// in platform code.
//////////////////////////////////////////////////////////////////

void GR_Font::s_getGenericFontProperties(const char * szFontName,
										 FontFamilyEnum * pff,
										 FontPitchEnum * pfp,
										 bool * pbTrueType)
{
	// describe in generic terms the named font.

	// we borrow some code from GR_Win32Graphics::findFont()

	LOGFONT lf = { 0 };
	TEXTMETRIC tm = { 0 };

	// TODO i'm not sure why we special case these, but the other
	// TODO code did, so i'm going to here.

	if (UT_stricmp(szFontName, "serif") == 0)
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_ROMAN;
	else if (UT_stricmp(szFontName, "sans-serif") == 0)
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
	else if (UT_stricmp(szFontName, "cursive") == 0)
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_SCRIPT;
	else if (UT_stricmp(szFontName, "fantasy") == 0)
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DECORATIVE;
	else if (UT_stricmp(szFontName, "monospace") == 0)
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_MODERN;
	else
		strcpy(lf.lfFaceName, szFontName);

	// let the system create the font with the given name or
	// properties and then query it and see what was actually
	// created.  hopefully, this will let us more accurately
	// reflect what is being seen on screen.

	HFONT hFont = CreateFontIndirect(&lf);
	HDC hdc = CreateDC("DISPLAY",NULL,NULL,NULL);
	HFONT hFontOld = (HFONT) SelectObject(hdc,hFont);
	GetTextMetrics(hdc,&tm);
	SelectObject(hdc,hFontOld);
	DeleteObject(hFont);
	DeleteDC(hdc);

	BYTE xx = tm.tmPitchAndFamily;

	switch (xx & 0xf0)
	{
	default:
	case FF_DONTCARE:	*pff = FF_Unknown;		break;
	case FF_ROMAN:		*pff = FF_Roman;		break;
	case FF_SWISS:		*pff = FF_Swiss;		break;
	case FF_MODERN:		*pff = FF_Modern;		break;
	case FF_SCRIPT:		*pff = FF_Script;		break;
	case FF_DECORATIVE:	*pff = FF_Decorative;	break;
	}

	if ((xx & TMPF_FIXED_PITCH) == TMPF_FIXED_PITCH)
		*pfp = FP_Variable;				// yes these look backwards
	else								// but that is how windows
		*pfp = FP_Fixed;				// defines the bits...

	*pbTrueType = ((xx & TMPF_TRUETYPE) == TMPF_TRUETYPE);

	return;
}

GR_Win32Font::GR_Win32Font(HFONT hFont, GR_Graphics * pG)
:	m_oldHDC(0),
	m_hFont(hFont),
	m_defaultCharWidth(0),
	m_tm(TEXTMETRIC()),
	m_pG(pG)
{
	UT_ASSERT(m_hFont);

	//
	// TMN: We need to initialize 'this' to _something_, why we use the
	// screen DC. Note that this is a *bad hack* forced by bad design. :-(
	//
	HDC hDC = GetDC(0);
	if (!hDC)
	{
		// NOW what? We can't throw an exeption, and this object isn't
		// legal yet...
	}
	else
	{
		m_oldHDC = hDC;
		HFONT hOldFont = (HFONT)SelectObject(hDC, hFont);
		if (hOldFont == (HFONT)GDI_ERROR)
		{
			// Same here... What do do?
		}
		else
		{
			setupFontInfo();
			SelectObject(hDC, (HGDIOBJ)hOldFont);
		}
		m_oldHDC = 0;
		ReleaseDC(0, hDC);
	}
}

GR_Win32Font::~GR_Win32Font()
{
	UT_ASSERT(m_hFont);

	// We currently can't delete the font, since it might
	// be selected into a DC. Also, we don't keep track of previously
	// selected font [into the DC] why it's impossible to revert to that
	// one! :-<
	// But, let's try it like this...

	const DWORD dwObjType = GetObjectType((HGDIOBJ)m_oldHDC);
	const bool bIsDC = dwObjType == OBJ_DC || dwObjType == OBJ_MEMDC;
	if (!bIsDC || m_hFont != (HFONT)::GetCurrentObject(m_oldHDC, OBJ_FONT))
	{
		DeleteObject(m_hFont);
	}
}

void GR_Win32Font::setupFontInfo()
{
	m_cw.zeroWidths();

	// preload US-ASCII subset of Latin-1 as these
	// are rather frequently used.  we can demand
	// load the other half of latin-1 as well as
	// the rest of unicode.
	//m_cw.setCharWidthsOfRange(hdc,0,255);

	m_cw.setCharWidthsOfRange(m_oldHDC, 0x20, 0x7f, m_pG);

	GetTextMetrics(m_oldHDC, &m_tm);

	UINT d = m_tm.tmDefaultChar;
	m_cw.setCharWidthsOfRange(m_oldHDC, d, d, m_pG);
	m_defaultCharWidth = m_cw.getWidth(d);
}


UT_uint32 GR_Win32Font::Acq::measureUnRemappedChar(GR_Win32Font& font, UT_UCSChar c)
{
	int iCharWidth = 0;
	int iWidth;
	// try to get cached value for the width of this char.
	// if that fails, force fill the cache for this char
	// and then re-hit the cache.
	// if that fails, the char is probably not defined in
	// the font, so we substitute the default char width.
	iWidth = font.m_cw.getWidth(c);
	if (iWidth == GR_CW_UNKNOWN)
	{
		font.m_cw.setCharWidthsOfRange(font.m_oldHDC, c, c, m_pG);
		iWidth = font.m_cw.getWidth(c);
		// [[Why the default width?  I think zero is better in that case?]]
		// because the win32 font rendering engine will remap the
		// glyph to the default glyph, so we need to be advancing by
		// the default glyph's width
		if (iWidth == GR_CW_UNKNOWN) iWidth = font.m_defaultCharWidth;
	}
	return iWidth;
}

#if 0
UT_uint32 GR_Win32Font::Acq::measureString(GR_Win32Font& font, const UT_UCSChar* s, int iOffset, int num, unsigned short* pWidths)
{
	UT_ASSERT(s);
	if (!s)
		return 0;

	int iCharWidth = 0;
	int iWidth;
	for (int i=0; i<num; i++)
	{
		// try to get cached value for the width of this char.
		// if that fails, force fill the cache for this char
		// and then re-hit the cache.
		// if that fails, the char is probably not defined in
		// the font, so we substitute the default char width.
		UT_UCSChar currentChar;
		currentChar = remapGlyph(s[i + iOffset]);
		iWidth = font.m_cw.getWidth(currentChar);
		if (!iWidth)
		{
			font.m_cw.setCharWidthsOfRange(font.m_oldHDC,currentChar,currentChar, m_pG);
			iWidth = font.m_cw.getWidth(currentChar);
		}

		iCharWidth += iWidth;
		if (pWidths)
			pWidths[i] = iWidth;
	}

	return iCharWidth;
}
#endif

void GR_Win32Font::Acq::selectFontIntoDC(GR_Win32Font& font, HDC hdc)
{
	SelectObject(hdc, font.m_hFont);
	if (hdc != font.m_oldHDC)
	{
		// invalidate cached info when we change hdc's.
		// this is probably unnecessary except when
		// sharing a font with screen and printer.
		// TODO consider changing our invalidate test
		// TODO to not invalidate if old and new are
		// TODO both on screen.

		font.m_oldHDC = hdc;

		font.setupFontInfo();
	}
}


void GR_Win32Graphics::polygon(UT_RGBColor& c,UT_Point *pts,UT_uint32 nPoints)
{
	HPEN hPen = CreatePen(PS_SOLID, 1, RGB(c.m_red, c.m_grn, c.m_blu));
	HPEN hOldPen = (HPEN)SelectObject(m_hdc,hPen);

	HBRUSH hBrush = CreateSolidBrush(RGB(c.m_red, c.m_grn, c.m_blu));
	HBRUSH hOldBrush = (HBRUSH)SelectObject(m_hdc, hBrush);

	POINT * points = new POINT[nPoints];
	UT_ASSERT(points);

	for (UT_uint32 i = 0; i < nPoints; ++i)
	{
		points[i].x = tdu(pts[i].x);
		points[i].y = tdu(pts[i].y);
	}

	Polygon(m_hdc, points, nPoints);

	(void) SelectObject(m_hdc, hOldBrush);
	DeleteObject(hBrush);

	(void) SelectObject(m_hdc, hOldPen);
	DeleteObject(hPen);

	delete[] points;
}

bool GR_Win32Graphics::_setTransform(const GR_Transform & tr)
{
#if 0
	if(UT_IsWinNT())
	{
		XFORM xForm;
		float fScale = (float)((tlu(1) * getZoomPercentage()) / 100);
	
		xForm.eM11 = (float)(tr.getM11() * fScale);
		xForm.eM12 = (float)tr.getM12();
		xForm.eM21 = (float)tr.getM21();
		xForm.eM22 = (float)(tr.getM22() * fScale);
		xForm.eDx  = (float)(tr.getDx());
		xForm.eDy  = (float)(tr.getDy());

		bool ret = (SetWorldTransform(m_hdc, & xForm) != 0);
		UT_ASSERT( ret );
		return ret;
	}
	else
	{
		// world tranforms are not supported, fiddle with the view
		// port, etc
		UT_sint32 iScale = (tlu(1) * getZoomPercentage())/100;
		
		bool res = (SetWindowExtEx(m_hdc, iScale, iScale, NULL) != 0);
		UT_ASSERT( res );

		if(res)
		{
			res = (SetViewportExtEx(m_hdc,
									GetDeviceCaps(m_hdc, LOGPIXELSX), 
									GetDeviceCaps(m_hdc, LOGPIXELSY),
									NULL) != 0);
			UT_ASSERT( res );
		}
		
		// change of origins is not implemented yet
		UT_ASSERT(!tr.getDx() && !tr.getDy());

		return res;
	}
#else
	return true;
#endif
}


void GR_Win32Graphics::saveRectangle(UT_Rect & r, UT_uint32 iIndx) {	
	void * oldR = NULL;
	m_vSaveRect.setNthItem(iIndx, (void*)new UT_Rect(r),&oldR);
	if(oldR)
		delete (UT_Rect*)oldR;

	void * oldC = NULL;
	m_vSaveRectBuf.setNthItem(iIndx, (void*) new COLORREF[r.width * r.height], &oldC);
	if(oldC)
		delete (COLORREF*)oldC;
		
	COLORREF *p = (COLORREF *)m_vSaveRectBuf.getNthItem(iIndx);
	for (int x = 0; x < r.width; x++) {
		for (int y = 0; y < r.height; y++) {
			*(p++) = GetPixel(m_hdc, r.left + x, r.top + y);
			}
		}
	}

void GR_Win32Graphics::restoreRectangle(UT_uint32 iIndx) {
	UT_Rect * r = (UT_Rect*)m_vSaveRect.getNthItem(iIndx);
	COLORREF *p = (COLORREF *)m_vSaveRectBuf.getNthItem(iIndx);
	
	if ( r && p) {
		for (int x = 0; x < r->width; x++) {
			for (int y = 0; y < r->height; y++) {
				SetPixel(m_hdc, r->left + x, r->top + y, *(p++));
				}
			}
		}
}

