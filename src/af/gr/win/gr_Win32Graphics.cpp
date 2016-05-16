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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#define WIN32_LEAN_AND_MEAN
#define NOWINABLE
#define NOMETAFILE
#define NOSERVICE
#define NOIME
#define NOMCX

#include <windows.h>
#include <winspool.h>

#include "gr_Win32Graphics.h"
#include "gr_Win32Image.h"
#include "gr_Painter.h"

#include <xap_Win32App.h>
#include <xap_Win32Res_Cursors.rc2>

#include "xap_Prefs.h"
#include "xap_Frame.h"
#include "xap_Dialog_Id.h"
#include "xap_Win32Dlg_Print.h"
#include "xap_EncodingManager.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_std_string.h"
#include "ut_Win32OS.h"
#include "ut_Win32LocaleString.h"

#ifdef __MINGW32__
#include <w32api.h>
#endif

UT_uint32 GR_Win32Graphics::s_iInstanceCount = 0;
HDC GR_Win32Graphics::m_defPrintHDC = NULL;

#define LOG_WIN32_EXCPT(msg)                                                  \
{                                                                             \
    UT_String __s;                                                            \
    UT_String_sprintf(__s, "%s (%d): %s",__FILE__, __LINE__, msg);            \
	UT_DEBUGMSG(("%s",__s.c_str()));                                          \
    if(XAP_App::getApp()->getPrefs())                                         \
    {                                                                         \
	    XAP_App::getApp()->getPrefs()->log("gr_Win32Graphics", __s.c_str());  \
    }                                                                         \
}

//#define GR_GRAPHICS_DEBUG	1

/*****************************************************************/

// A small helper class
class ABI_EXPORT private_FontReverter
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
			m_gr.setFont(m_pOldFont);
		// else, Oh joy, no old font to revert to... :-<
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

	m_iDCFontAllocNo = 0;
	m_iPrintDCFontAllocNo = 0;
	m_hdc = hdc;
	m_printHDC = NULL;
	m_nPrintLogPixelsY = 0;
	m_hwnd = 0;
	m_iLineWidth = 0;	// default to a hairline
	m_bPrint = false;
	m_bStartPrint = false;
	m_bStartPage = false;
	m_pFont = NULL;
	m_pFontGUI = NULL;
	m_bIsPreview = false;

	m_cs = GR_Graphics::GR_COLORSPACE_COLOR;
	m_cursor = GR_CURSOR_INVALID;

	m_clrXorPen = 0;
	m_hXorPen = 0;

	setCursor(GR_CURSOR_DEFAULT);

	m_remapBuffer = NULL;
	m_remapBufferSize = 0;
	m_remapIndices = NULL;

	m_eJoinStyle = JOIN_MITER;
	m_eCapStyle  = CAP_PROJECTING;
	m_eLineStyle = LINE_SOLID;

	setBrush((HBRUSH) GetStockObject(WHITE_BRUSH));	// Default brush

	m_nLogPixelsY = GetDeviceCaps(m_hdc, LOGPIXELSY);
	int nLogPixelsX = GetDeviceCaps(m_hdc, LOGPIXELSX);
	if(m_nLogPixelsY)
	{
		m_fXYRatio = (double)nLogPixelsX / (double) m_nLogPixelsY;
	}
	else
	{
		UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
		m_fXYRatio = 1;
	}
	
	m_hDevMode = NULL;
	
	
	typedef struct
	{
		HPEN hPen;
		int	 nStyle;
		int  nWidth;
		int	 nColour;
	
	} CACHE_PEN;
	
	m_pArPens = new GR_Win32Graphics::CACHE_PEN [_MAX_CACHE_PENS];
	memset (m_pArPens, 0, _MAX_CACHE_PENS*sizeof(CACHE_PEN));
	m_nArPenPos = 0;

	_DoubleBuffering_SetUpDummyBuffer();
}

GR_Win32Graphics::GR_Win32Graphics(HDC hdc, HWND hwnd)
{
	_constructorCommonCode(hdc);
	m_hwnd = hwnd;

	// init the print HDC with one for the default printer
	
	if (m_defPrintHDC == NULL) {
		m_defPrintHDC = UT_GetDefaultPrinterDC();
	}
	
	m_printHDC = m_defPrintHDC;
	s_iInstanceCount++;

	if(m_printHDC)
	{
		m_nPrintLogPixelsY = GetDeviceCaps(getPrintDC(), LOGPIXELSY);
		int nLogPixelsX = GetDeviceCaps(getPrintDC(), LOGPIXELSX);
		if(m_nPrintLogPixelsY)
		{
			m_fXYRatioPrint = (double)nLogPixelsX / (double) m_nPrintLogPixelsY;
		}
		else
		{
			UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
			m_fXYRatioPrint = 1;
		}
	}
}

void GR_Win32Graphics::setPrintDC(HDC dc)
{
	if(!m_bPrint)
	{
		// only do this for screen graphics
		m_printHDC = dc;

		// now make our views to rebuild themselves ...
		// (the win32 graphics currently cannot take advantate of the print dc, so we do nothing
	}
}


GR_Win32Graphics::GR_Win32Graphics(HDC hdc, const DOCINFOW * pDocInfo, HGLOBAL hDevMode)
{
	_constructorCommonCode(hdc);
 	m_bPrint = true;
	m_pDocInfo = pDocInfo;
	m_hDevMode = hDevMode;
	m_bIsPreview = (hDevMode == NULL);
}

GR_Win32Graphics::~GR_Win32Graphics()
{
	_destroyFonts ();
	UT_VECTOR_SPARSEPURGEALL( UT_Rect*, m_vSaveRect);
	s_iInstanceCount--;
		
	/* Release saved bitmaps */
	HBITMAP hBit;
	for (UT_sint32 i = 0; i < m_vSaveRectBuf.size (); i++)
	{
		hBit = (HBITMAP)m_vSaveRectBuf.getNthItem (i);
		DeleteObject(hBit);			
	}

	if (m_hXorPen)
		DeleteObject(m_hXorPen);

	delete [] m_remapBuffer;
	delete [] m_remapIndices;
	
	/*Release created pens*/
	CACHE_PEN * pArPens =  m_pArPens;
	for (int n = 0; n<m_nArPenPos; n++, pArPens++)
		DeleteObject(pArPens->hPen);	

	DELETEPV(m_pArPens);

	DELETEP(m_pFontGUI);
	
	_DoubleBuffering_ReleaseDummyBuffer();

	if(m_printHDC && m_printHDC != m_defPrintHDC)
		DeleteDC(m_printHDC);
	
	if (s_iInstanceCount == 0)
		DeleteDC(m_defPrintHDC);

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
		UT_ASSERT_HARMLESS(0);
		return false;
	}
}

GR_Win32Font * GR_Win32Graphics::_newFont(LOGFONTW & lf, double fPoints, HDC hdc, HDC printHDC)
{
	return GR_Win32Font::newFont(lf, fPoints, hdc, printHDC);
}

GR_Font* GR_Win32Graphics::getGUIFont(void)
{
	if (!m_pFontGUI)
	{
		// lazily grab this (once)
		HFONT f = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
		LOGFONTW lf;
		/*int iRes =*/ GetObjectW(f, sizeof(LOGFONTW), &lf);
		m_pFontGUI = _newFont(lf, 0, m_hdc, m_hdc);
		UT_ASSERT(m_pFontGUI);
		DeleteObject(f);
	}
	if(m_pFontGUI)
		m_pFontGUI->markGUIFont();
	
	return m_pFontGUI;
}

extern "C"
int CALLBACK
win32Internal_fontEnumProcedure(ENUMLOGFONTW* pLogFont,
								NEWTEXTMETRICEXW* /*pTextMetric*/,
								int /*Font_type*/,
								LPARAM lParam)
{
	LOGFONTW *lf = (LOGFONTW*)lParam;
	lf->lfCharSet = pLogFont->elfLogFont.lfCharSet;
	return 0;
}

GR_Font* GR_Win32Graphics::_findFont(const char* pszFontFamily,
									 const char* pszFontStyle,
									 const char* /*pszFontVariant*/,
									 const char* pszFontWeight,
									 const char* /*pszFontStretch*/,
									 const char* pszFontSize,
									 const char* /*pszLang*/)
{	
	#ifdef GR_GRAPHICS_DEBUG
	UT_DEBUGMSG(("GR_Win32Graphics::findFont %s %s %s\n", pszFontFamily, pszFontStyle, pszFontSize));	
	#endif
	
	LOGFONTW lf;
	memset(&lf, 0, sizeof(lf));

	HDC hPrintDC = m_printHDC ? m_printHDC : m_hdc;

	/*
		TODO we need to fill out the LOGFONT object such that
		we'll get the closest possible matching font.  For
		now, we're hard-coding a hack.
	*/

	// we need to get the size in logpixels for the current DC, which
	// simply means to divide points by 72 and multiply by device Y resolution

	// See: http://support.microsoft.com/support/kb/articles/Q74/2/99.asp
	double fPointSize = UT_convertToPoints(pszFontSize);
	lf.lfHeight = (int)(-fPointSize * (double)GetDeviceCaps(m_hdc, LOGPIXELSY) / 72.0);		

	// TODO note that we don't support all those other ways of expressing weight.
	if (0 == g_ascii_strcasecmp(pszFontWeight, "bold"))
		lf.lfWeight = 700;

	// TODO -- remove this block entirely, since oblique is no longer a valid style
	// We squash oblique into italic
	if (0 == g_ascii_strcasecmp(pszFontStyle, "italic") || 0 == g_ascii_strcasecmp(pszFontStyle, "oblique"))
		lf.lfItalic = TRUE;

	// TODO note that we currently think pszFontFamily is a single name, not a list!
	// TODO why don't these generic family names work?!?
	if (0 == g_ascii_strcasecmp(pszFontFamily, "serif"))
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_ROMAN;
	else if (0 == g_ascii_strcasecmp(pszFontFamily, "sans-serif"))
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
	else if (0 == g_ascii_strcasecmp(pszFontFamily, "cursive"))
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_SCRIPT;
	else if (0 == g_ascii_strcasecmp(pszFontFamily, "fantasy"))
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DECORATIVE;
	else if (0 == g_ascii_strcasecmp(pszFontFamily, "monospace"))
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_MODERN;
	else
	{
		// NB: lf.lfFaceName is a statically allocated char buffer of len 
		// LF_FACESIZE. strToNative() will truncate the string at 500 bytes,
		// but as LF_FACESIZE is generally 32, this shouldn't matter.
		//
		// This convertion is needed because the windows API expects fontnames
		// in the native encoding and later lf will be passing to a windows
		// API funcion (CreateFontIndirect()).
    	UT_Win32LocaleString str;
		str.fromUTF8 (pszFontFamily);
		lstrcpynW(lf.lfFaceName, 
		        str.c_str(),
		        LF_FACESIZE);
	}

	// Get character set value from the font itself
	LOGFONTW enumlf;
	memset(&enumlf, 0, sizeof(enumlf));

	enumlf.lfCharSet = DEFAULT_CHARSET;
	wcscpy(enumlf.lfFaceName, lf.lfFaceName);
	EnumFontFamiliesExW(m_hdc, &enumlf,
		(FONTENUMPROCW)win32Internal_fontEnumProcedure, (LPARAM)&lf, 0);

	lf.lfOutPrecision = OUT_TT_ONLY_PRECIS;		// Choose only True Type fonts.
	lf.lfQuality = PROOF_QUALITY;

	return _newFont(lf, fPointSize, m_hdc, hPrintDC);
}

void GR_Win32Graphics::drawGlyph(UT_uint32 /*Char*/, UT_sint32 /*xoff*/, UT_sint32 /*yoff*/)
{
	UT_ASSERT_HARMLESS(UT_TODO);
}

void GR_Win32Graphics::drawChar(UT_UCSChar Char, UT_sint32 xoff, UT_sint32 yoff)
{
	#ifdef GR_GRAPHICS_DEBUG
	UT_DEBUGMSG(("GR_Win32Graphics::drawChar %c %u %u\n", Char, xoff, yoff));	
	#endif	
	
	xoff = (UT_sint32)((double)_tduX(xoff) * m_fXYRatio);
	yoff = _tduY(yoff);

#if 0 // this bypassesthe font handling mechanism
	HFONT hFont = m_pFont->getDisplayFont(this);
	SelectObject(m_hdc, hFont);
#else
	m_pFont->selectFontIntoDC(this, m_hdc);
#endif
	
	SetTextAlign(m_hdc, TA_LEFT | TA_TOP);
	SetBkMode(m_hdc, TRANSPARENT);		// TODO: remember and reset?

	UT_UCSChar currentChar = Char;
	//TODO this is a temporary hack -- need a way to handle true 32-bit chars
	UT_UCS2Char aChar = (UT_UCS2Char) currentChar;

	if(currentChar == 0x200B || currentChar == 0xFEFF
	   /*|| currentChar == UCS_LIGATURE_PLACEHOLDER*/)
		return;
	// Windows NT and Windows 95 support the Unicode Font file.
	// All of the Unicode glyphs can be rendered if the glyph is found in
	// the font file. However, Windows 95 does  not support the Unicode
	// characters other than the characters for which the particular codepage
	// of the font file is defined.
	// Reference Microsoft knowledge base:
	// Q145754 - PRB ExtTextOutW or TextOutW Unicode Text Output Is Blank
	LOGFONTW lf;
	UT_DebugOnly<int> iRes = GetObjectW(m_pFont->getFontHandle(), sizeof(LOGFONTW), &lf);
	UT_ASSERT(iRes);
	if (UT_IsWinNT() == false && lf.lfCharSet == SYMBOL_CHARSET)
	{
		// Symbol character handling for Win9x
		char str[sizeof(UT_UCS2Char)];

		int iConverted = WideCharToMultiByte(CP_ACP, 0,
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
		m_remapBuffer[j] = (UT_UCS2Char)pChars[iCharOffset + i];
		
		if(m_remapBuffer[j] == 0x200B || m_remapBuffer[j] == 0xFEFF
		   /*|| m_remapBuffer[j] == UCS_LIGATURE_PLACEHOLDER*/)
			j--;
	}

	iLength -= (i - j);

	return m_remapBuffer;
}

void GR_Win32Graphics::drawChars(const UT_UCSChar* pChars,
								 int iCharOffset, int iLengthOrig,
								 UT_sint32 xoff, UT_sint32 yoff,
								 int * pCharWidths)
{
	UT_ASSERT(pChars);

	if(!iLengthOrig)
	{
		// I do not want to assert here, as this makes debugging
		// virtually impossible
		UT_DEBUGMSG(("GR_Win32Graphics::drawChars: asked to draw zero chars !!!\n"));
		return;
	}
	
	#ifdef GR_GRAPHICS_DEBUG
	UT_DEBUGMSG(("GR_Win32Graphics::drawChars %c %u %u\n", pChars, xoff, yoff));	
	#endif
	
	xoff = (UT_sint32)((double)_tduX(xoff) * m_fXYRatio);
	yoff = _tduY(yoff);
	int *pCharAdvances = NULL;
	
	// iLength can be modified by _remapGlyphs
	int iLength = iLengthOrig;
	HFONT hFont = m_pFont->getDisplayFont(this);
	SelectObject(m_hdc, hFont);
	m_iDCFontAllocNo = m_pFont->getAllocNumber();
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
	LOGFONTW lf;
	int iRes = GetObjectW(hFont, sizeof(LOGFONTW), &lf);
	UT_ASSERT(iRes);

	if (UT_IsWinNT() == false && lf.lfCharSet == SYMBOL_CHARSET)
	{
		// Symbol character handling for Win9x
		char* str = new char[iLength * sizeof(UT_UCS2Char)];
		int iConverted = WideCharToMultiByte(CP_ACP, 0,
			(LPCWSTR) currentChars, iLength,
			str, iLength * sizeof(UT_UCSChar), NULL, NULL);

		ExtTextOutA(m_hdc, xoff, yoff, 0, NULL, str, iConverted, NULL);
		delete [] str;
	}
	else
	{
		int duCharWidths [256];

		if (pCharWidths)
		{
			if (iLengthOrig > (sizeof(duCharWidths)/sizeof(int)) )
				pCharAdvances = new int [iLengthOrig];
			else
				pCharAdvances = duCharWidths;
			UT_ASSERT(pCharAdvances != NULL);

			// convert width into display units; since we have removed
			// all 0x200B and 0xFEFF characters, we also have to
			// remove their entires from the advances
			UT_sint32 i,j;
			UT_sint32 iwidth = 0;
			UT_sint32 iadvance = 0;
			UT_sint32 inextAdvance = 0;
			for (i = 0, j = 0; i < iLengthOrig; i++)
			{
				if(! (pChars[iCharOffset+i] == 0x200B || pChars[iCharOffset+i] == 0xFEFF
				   /*|| pChars[iCharOffset+i] == UCS_LIGATURE_PLACEHOLDER*/ ) )
				{
#if 1
					// I rather stupidly changed this to the version
					// below the #else, but this one produces much
					// smaller rounding errors. Tomas, Dec 6, 2003
					iwidth += pCharWidths[iCharOffset + i];
					inextAdvance = (UT_sint32)((double)_tduX(iwidth) * m_fXYRatio);
					pCharAdvances[j] = inextAdvance - iadvance;
					iadvance = inextAdvance;
#else
					pCharAdvances[j] = tdu(pCharWidths[iCharOffset + i]);
#endif
					j++;
				}
			}
		}
		else
		  {
			pCharAdvances=NULL;
		  }

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
			gcpResult.lStructSize = sizeof(GCP_RESULTSW);
			gcpResult.lpOutString = NULL;			// Output string
			gcpResult.lpOrder = NULL;				// Ordering indices
			// we must set here lpDx to NULL so that
			// GetCharacterPlacement does not change our values ...
			gcpResult.lpDx = NULL;	                // Distances between character cells
			gcpResult.lpCaretPos = NULL;			// Caret positions	
			gcpResult.lpClass = NULL;				// Character classifications
// w32api changed lpGlyphs from UINT * to LPWSTR to match MS PSDK in w32api v2.4
#if defined(__MINGW32__) && (__W32API_MAJOR_VERSION == 2 && __W32API_MINOR_VERSION < 4)
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
			// now we set the character advances ...
			gcpResult.lpDx = pCharAdvances;	    // Distances between character cells
			
			if(placementResult)
			{
				ExtTextOutW(m_hdc, xoff, yoff, ETO_GLYPH_INDEX, NULL, (LPCWSTR) m_remapIndices, gcpResult.nGlyphs, pCharAdvances);
			}
			else
			{
				UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
				goto simple_exttextout;
			}
		}
		else
		{
            simple_exttextout:
			ExtTextOutW(m_hdc, xoff, yoff, 0, NULL, (LPCWSTR) currentChars, iLength, pCharAdvances);
		}

		if (pCharAdvances && (iLengthOrig > (sizeof(duCharWidths)/sizeof(int))) )
			delete[] pCharAdvances;
	}
}

void GR_Win32Graphics::setFont(const GR_Font* pFont)
{
	UT_ASSERT(pFont);	// TODO should we allow pFont == NULL?

	const GR_Win32Font* pWin32Font = static_cast<const GR_Win32Font*>(pFont);

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
	if (m_pFont == NULL || pFont->getAllocNumber() != m_iFontAllocNo
		|| pWin32Font->getPrimaryHDC() != m_hdc)
	{
		m_pFont = const_cast<GR_Win32Font*>(pWin32Font);
		m_iFontAllocNo = pFont->getAllocNumber();
		m_pFont->selectFontIntoDC(this, m_hdc);
	}
}

UT_uint32 GR_Win32Graphics::getFontHeight(const GR_Font * fnt)
{
	private_FontReverter janitor_(*this, m_pFont);

	setFont(fnt);

	return getFontHeight();
}

UT_uint32 GR_Win32Graphics::getFontHeight()
{
	UT_return_val_if_fail( m_pFont, 0 );

	return (UT_uint32)(m_pFont->getHeight(m_hdc, m_printHDC));
}

UT_uint32 GR_Win32Graphics::getFontAscent(const GR_Font* fnt)
{
	private_FontReverter janitor_(*this, m_pFont);

	setFont(fnt);

	return getFontAscent();
}

UT_uint32 GR_Win32Graphics::getFontAscent()
{
	UT_return_val_if_fail( m_pFont, 0 );
	return (UT_uint32)(m_pFont->getAscent(m_hdc, m_printHDC));
}

UT_uint32 GR_Win32Graphics::getFontDescent(const GR_Font* fnt)
{
	private_FontReverter janitor_(*this, m_pFont);

	setFont(fnt);

	return getFontDescent();
}

UT_uint32 GR_Win32Graphics::getFontDescent()
{
	UT_return_val_if_fail( m_pFont, 0 );
	return (UT_uint32)(m_pFont->getDescent(m_hdc, m_printHDC));
}

void GR_Win32Graphics::getCoverage(UT_NumberVector& coverage)
{
	coverage.clear();
	coverage.push_back(' ');
	coverage.push_back((255 - ' '));
	
	//UT_ASSERT(UT_TODO);
}

UT_sint32 GR_Win32Graphics::measureUnRemappedChar(const UT_UCSChar c, UT_uint32 * /*height*/)
{
	#ifdef GR_GRAPHICS_DEBUG
	UT_DEBUGMSG(("GR_Win32Graphics::measureUnRemappedChar\n"));	
	#endif

	UT_return_val_if_fail(m_pFont,0);
	UT_sint32 iWidth = m_pFont->measureUnRemappedChar(c);

	if (iWidth==GR_CW_UNKNOWN || iWidth==GR_CW_ABSENT)
		return iWidth;
	
	iWidth *= (UT_sint32)getResolution();
	iWidth /= (UT_sint32)getDeviceResolution();
	iWidth = (UT_sint32)((double)iWidth * m_fXYRatio);
	return iWidth;	
}

UT_uint32 GR_Win32Graphics::getDeviceResolution(void) const
{	
	return m_nLogPixelsY; // NOTE: assumes square pixels
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

#ifdef GR_GRAPHICS_DEBUG
static nCacheHit = 0;
static nCacheFailed = 0;
#endif

void GR_Win32Graphics::drawLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2, UT_sint32 y2)
{	
	#ifdef GR_GRAPHICS_DEBUG
	UT_DEBUGMSG(("GR_Win32Graphics::drawLine %u %u %u %u\n", x1,  y1, x2,  y2));	
	#endif			   	
	
	x1 = (UT_sint32)((double)_tduX(x1) * m_fXYRatio);
	x2 = (UT_sint32)((double)_tduX(x2) * m_fXYRatio);
	y1 = _tduY(y1);
	y2 = _tduY(y2);	

	int penStyle;
	HPEN hPen = NULL;
	bool bCached = false;
	UT_sint32 iLineWidth = (UT_sint32)((double)_tduR(m_iLineWidth) * m_fXYRatio);
	
	switch(m_eLineStyle)
	{
		case LINE_DOUBLE_DASH:
		case LINE_ON_OFF_DASH:      penStyle = PS_DASH;  break;
		case LINE_SOLID:            penStyle = PS_SOLID; break;
		case LINE_DOTTED:           penStyle = PS_DOT;   break;

		default:
			UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
			penStyle = PS_SOLID;
	}
	
	/*Look for a cached pen*/
	CACHE_PEN * pArPens =  m_pArPens;
	for (int n = 0; n<m_nArPenPos; n++, pArPens++)
	{
		if (pArPens->nStyle==penStyle &&
			pArPens->nWidth==iLineWidth &&
			pArPens->dwColour==m_clrCurrent)
			{
				hPen = pArPens->hPen;
				bCached = true;
#ifdef GR_GRAPHICS_DEBUG
				nCacheHit++;
#endif
				break;
			}		
	}
	
	/*If not cached, let's create it*/
	if (!hPen)
	{
		hPen = CreatePen(penStyle, iLineWidth, m_clrCurrent);

		if (m_nArPenPos<_MAX_CACHE_PENS)
		{			
			pArPens =  m_pArPens + m_nArPenPos;
			pArPens->nStyle=penStyle;
			pArPens->nWidth=iLineWidth;
			pArPens->dwColour=m_clrCurrent;
			pArPens->hPen = hPen;
			bCached = true;
			m_nArPenPos++;
#ifdef GR_GRAPHICS_DEBUG
			nCacheFailed++;
#endif
		}
	}	

	#ifdef GR_GRAPHICS_DEBUG
	UT_DEBUGMSG(("GR_Win32Graphics::drawline: cached elements %u cached hits %u  failed %u\n", m_nArPenPos, 
		 nCacheHit, nCacheFailed));	
	#endif
	
	HPEN hOldPen = (HPEN) SelectObject(m_hdc, hPen);	

	MoveToEx(m_hdc, x1, y1, NULL);
	LineTo(m_hdc, x2, y2);

	(void) SelectObject(m_hdc, hOldPen);
	
	if (!bCached)
		DeleteObject(hPen);
}

void GR_Win32Graphics::setLineProperties(double iLineWidth,
										 JoinStyle inJoinStyle,
										 CapStyle inCapStyle,
										 LineStyle inLineStyle)
{
	m_eJoinStyle = inJoinStyle;
	m_eCapStyle  = inCapStyle;
	m_eLineStyle = inLineStyle;
	m_iLineWidth = static_cast<UT_sint32>(iLineWidth);
}

void GR_Win32Graphics::setLineWidth(UT_sint32 iLineWidth)
{
	m_iLineWidth = iLineWidth;
}

void GR_Win32Graphics::xorLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2, UT_sint32 y2)
{
	#ifdef GR_GRAPHICS_DEBUG
	UT_DEBUGMSG(("GR_Win32Graphics::xorLine %u %u %u %u\n", x1,  y1, x2,  y2));	
	#endif
	
	/*
	  Note that we always use a pixel width of 1 for xorLine, since
	  this should always be done to the screen.
	*/

	x1 = (UT_sint32)((double)_tduX(x1) * m_fXYRatio);
	x2 = (UT_sint32)((double)_tduX(x2) * m_fXYRatio);
	y1 = _tduY(y1);
	y2 = _tduY(y2);

	if (m_clrCurrent != m_clrXorPen || !m_hXorPen)
	{
		if (m_hXorPen)
			DeleteObject(m_hXorPen);
		m_hXorPen = CreatePen(PS_SOLID, (UT_sint32)((double)_tduR(m_iLineWidth) * m_fXYRatio), m_clrCurrent);
		m_clrXorPen = m_clrCurrent;
	}

	int iROP = SetROP2(m_hdc, R2_XORPEN);
	HPEN hOldPen = (HPEN)SelectObject(m_hdc, m_hXorPen);

	MoveToEx(m_hdc, x1, y1, NULL);
	LineTo(m_hdc, x2, y2);

	SelectObject(m_hdc, hOldPen);
	SetROP2(m_hdc, iROP);
}

void GR_Win32Graphics::polyLine(const UT_Point * pts, UT_uint32 nPoints)
{
	#ifdef GR_GRAPHICS_DEBUG
	UT_DEBUGMSG(("GR_Win32Graphics::polyLine %u\n", nPoints));	
	#endif
	
	HPEN hPen = CreatePen(PS_SOLID, (UT_sint32)((double)_tduR(m_iLineWidth) * m_fXYRatio), m_clrCurrent);
	HPEN hOldPen = (HPEN) SelectObject(m_hdc, hPen);

	POINT * points = (POINT *)UT_calloc(nPoints, sizeof(POINT));
	UT_return_if_fail(points);

	for (UT_uint32 i = 0; i < nPoints; i++)
	{
		points[i].x = (UT_sint32)((double)_tduX(pts[i].x) * m_fXYRatio);
		points[i].y = _tduY(pts[i].y);
	}

	Polyline(m_hdc, points, nPoints);

	(void) SelectObject(m_hdc, hOldPen);
	DeleteObject(hPen);
	FREEP(points);
}

void GR_Win32Graphics::fillRect(const UT_RGBColor& c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h)
{
	
	RECT r;
	r.left = (UT_sint32)((double)_tduX(x) * m_fXYRatio);
	r.top = _tduY(y);
	r.right = (UT_sint32)(_tduX(x+w) * m_fXYRatio);
	r.bottom = _tduY(y+h);

	COLORREF clr = RGB(c.m_red, c.m_grn, c.m_blu);

	#ifdef GR_GRAPHICS_DEBUG
	w=(UT_sint32)((double)_tduR(w) * m_fXYRatio);
	h=_tduR(h);
	UT_DEBUGMSG(("GR_Win32Graphics::fillRect %x %u %u %u %u\n",  clr, r.left, r.top, w, h));	
	#endif
		
	// This might look wierd (and I think it is), but it's MUCH faster.
	// CreateSolidBrush is dog slow.
	HDC hdc = m_hdc;
	const COLORREF cr = ::SetBkColor(hdc,  clr);
	::ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &r, NULL, 0, NULL);
	::SetBkColor(hdc, cr);
}

bool GR_Win32Graphics::startPrint(void)
{
	UT_ASSERT(m_bPrint);
	UT_ASSERT(!m_bStartPrint);

	if (!m_bIsPreview)
	  {
	    m_bStartPrint = ( StartDocW(m_hdc,m_pDocInfo) > 0 );
	  }
	else
	  {
	    m_bStartPrint = true;
	  }

	return m_bStartPrint;
}

bool GR_Win32Graphics::startPage(const char * /*szPageLabel*/, UT_uint32 /*pageNumber*/,
									bool bPortrait, UT_uint32 iWidth, UT_uint32 iHeight)
{
	// need these in 10th of milimiters
	iWidth = (int)((double)iWidth * m_fXYRatio * 254.0 / (double)getResolution() + 0.5);
	iHeight = iHeight * 254 / getResolution();

	if (m_bStartPage)
	{
		EndPage(m_hdc);
	}

	// Correct for Portrait vs Lanscape mode
	if (m_hDevMode)
	{
		DEVMODEW *pDevMode = (DEVMODEW*) GlobalLock(m_hDevMode);
		UT_return_val_if_fail(pDevMode, false); //GlobalLock can return NULL
		pDevMode->dmFields = DM_ORIENTATION | DM_PAPERLENGTH | DM_PAPERWIDTH;
		pDevMode->dmOrientation = (bPortrait) ? DMORIENT_PORTRAIT : DMORIENT_LANDSCAPE;
		pDevMode->dmPaperSize = 0;
		pDevMode->dmPaperLength = (bPortrait) ? iHeight : iWidth;
		pDevMode->dmPaperWidth = (bPortrait) ? iWidth : iHeight;
		
		GlobalUnlock(m_hDevMode);

		// call DocumentProperties() on the DEVMODE to ensure changes propagate down into
		// the private part
		fixDevMode(m_hDevMode);
		
		pDevMode = (DEVMODEW*) GlobalLock(m_hDevMode);
		ResetDCW(m_hdc, pDevMode);
		GlobalUnlock(m_hDevMode);
	}

	const int iRet = StartPage(m_hdc);

	if (!m_bIsPreview) {
	  m_bStartPage = iRet > 0;

	  if (m_bStartPage) {
	      // PHYSICALOFFSETX returns offset of printable area from the left edge
	      // of the physical printer paper. The value returned is in device units.
	      // Since the current mapping mode is MM_TEXT, this code _should_ work.
	      const POINT ptNew = {
		-GetDeviceCaps(m_hdc, PHYSICALOFFSETX),
		-GetDeviceCaps(m_hdc, PHYSICALOFFSETY)
	      };
	      SetViewportOrgEx(m_hdc, ptNew.x, ptNew.y, 0);
	    }
	}
	else {
	  m_bStartPage = true;
	}

	return m_bStartPage;
}

bool GR_Win32Graphics::endPrint(void)
{
  if (m_bStartPage) {
		EndPage(m_hdc);
  }

  if (!m_bIsPreview)
    return (EndDoc(m_hdc) > 0);
  
  return true;
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
	UT_sint32 oldDY = tdu(getPrevYOffset());
	UT_sint32 oldDX = (UT_sint32)((double)tdu(getPrevXOffset()) * m_fXYRatio);
	UT_sint32 newY = getPrevYOffset() + dy;
	UT_sint32 newX = getPrevXOffset() + dx;
	UT_sint32 ddx = -(UT_sint32)((double)(tdu(newX) - oldDX) * m_fXYRatio);
	UT_sint32 ddy = -(tdu(newY) - oldDY);
	setPrevYOffset(newY);
	setPrevXOffset(newX);
	if(ddx == 0 && ddy == 0)
	{
		return;
	}
	GR_Painter caretDisablerPainter(this); // not an elegant way to disable all carets, but it works beautifully - MARCM

	ScrollWindowEx(m_hwnd, ddx, ddy, NULL, NULL, NULL, 0, SW_INVALIDATE);
}

void GR_Win32Graphics::scroll(UT_sint32 x_dest, UT_sint32 y_dest,
							  UT_sint32 x_src, UT_sint32 y_src,
							  UT_sint32 width, UT_sint32 height)
{
	x_dest = (UT_sint32)((double)tdu(x_dest) * m_fXYRatio);
	y_dest = tdu(y_dest);
	x_src = (UT_sint32)((double)tdu(x_src) * m_fXYRatio);
	y_src = tdu(y_src);
	width = (UT_sint32)((double)tdu(width) * m_fXYRatio);
	height = tdu(height);
	RECT r;
	r.left = x_src;
	r.top = y_src;
	r.right = r.left + width;
	r.bottom = r.top + height;
	
	GR_Painter caretDisablerPainter(this); // not an elegant way to disable all carets, but it works beautifully - MARCM
	
	ScrollWindowEx(m_hwnd, (x_dest - x_src), (y_dest - y_src),
				   &r, NULL, NULL, NULL, SW_ERASE);
}

void GR_Win32Graphics::clearArea(UT_sint32 x, UT_sint32 y, UT_sint32 width, UT_sint32 height)
{
	#ifdef GR_GRAPHICS_DEBUG
	UT_DEBUGMSG(("GR_Win32Graphics::clearArea %u %u %u %u\n",  x, y, width, height));	
	#endif
	
	x = (UT_sint32)((double)_tduX(x) * m_fXYRatio);
	y = _tduY(y);
	width = (UT_sint32)((double)_tduR(width) * m_fXYRatio);
	height = _tduR(height);	
	
	RECT r;
	r.left = x;
	r.top = y;
	r.right = r.left + width;
	r.bottom = r.top + height;

#if 1
	// for the sake of consistency we should be using the same method as the fillRect()
	// does; however, we already have brush created, so FillRect is probably quicker here
	// (someone should do some profiling on this one day)
	FillRect(m_hdc, &r, m_hClearBrush);
#else
	// this is the method used by fillRect(); if we were to use it, it we should
	// cache lb.lbColor from inside setBrush() to avoid calling GetObject() all the time
	LOGBRUSHW lb;
	GetObjectW(m_hClearBrush, sizeof(LOGBRUSHW), &lb);
	
	const COLORREFW cr = ::SetBkColor(m_hdc,  lb.lbColor);
	::ExtTextOutW(m_hdc, 0, 0, ETO_OPAQUE, &r, NULL, 0, NULL);
	::SetBkColor(m_hdc, cr);
#endif
}

void GR_Win32Graphics::invertRect(const UT_Rect* pRect)
{
	#ifdef GR_GRAPHICS_DEBUG
	UT_DEBUGMSG(("GR_Win32Graphics::invertRect\n"));	
	#endif
	
	RECT r;

	r.left = (UT_sint32)((double)_tduX(pRect->left) * m_fXYRatio);
	r.top = _tduY(pRect->top);
	r.right = (UT_sint32)((double)_tduX(pRect->left + pRect->width) * m_fXYRatio);
	r.bottom = _tduY(pRect->top + pRect->height);

	InvertRect(m_hdc, &r);
}

void GR_Win32Graphics::setClipRect(const UT_Rect* pRect)
{
	// This causes a lot of drawing and screen refresh problems.
	// It can be removed from here without problems, but may
	// not work for other things.  For now leave code in place.
	// return;

	UT_DebugOnly<int> res;
	m_pRect = pRect;
	if (pRect)
	{
		// set the clip rectangle
		HRGN hrgn = CreateRectRgn((UT_sint32)((double)_tduX(pRect->left) * m_fXYRatio),
								  _tduY(pRect->top),
								  (UT_sint32)((double)_tduX(pRect->left + pRect->width) * m_fXYRatio),
								  _tduY(pRect->top + pRect->height));
		UT_ASSERT(hrgn);

		res = SelectClipRgn(m_hdc, hrgn);

		DeleteObject(hrgn);
	}
	else		// stop clipping
		res = SelectClipRgn(m_hdc, NULL);

	UT_ASSERT_HARMLESS(res != ERROR);
}

GR_Image* GR_Win32Graphics::createNewImage(const char* pszName, const UT_ByteBuf* pBB, const std::string& mimetype,
					   UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight,
					   GR_Image::GRType iType)
{
	iDisplayWidth = (UT_sint32)((double)_tduR(iDisplayWidth) * m_fXYRatio);
	iDisplayHeight = _tduR(iDisplayHeight);
	
	GR_Image* pImg = NULL;
	if (iType == GR_Image::GRT_Raster)
		pImg = new GR_Win32Image(pszName);
	else
		pImg = new GR_VectorImage(pszName);

	pImg->convertFromBuffer(pBB, mimetype, iDisplayWidth,iDisplayHeight);

	return pImg;
}

void GR_Win32Graphics::drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest)
{
	UT_return_if_fail(pImg);
	
	xDest = (UT_sint32)((double)_tduX(xDest) * m_fXYRatio);
	yDest = _tduY(yDest);
	
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

	if (pDIB == NULL)
		return;
		
	UT_uint32 iSizeOfColorData = pDIB->bmiHeader.biClrUsed * sizeof(RGBQUAD);
	UT_Byte* pBits = ((unsigned char*) pDIB) + pDIB->bmiHeader.biSize + iSizeOfColorData;

	int iRes = 0;

#if 0
	// AlphaBlend is supported from win98 upwards
	// we need to turn the DIB into DC first
	HDC memDC = CreateCompatibleDC(m_hdc);

	if(memDC)
	{
		// need to convert the dib pointer to handle here ...

		// now select into the memory dc and call AlphaBlend
		SelectObject(memDC, hDIB);
		iRes = AlphaBlend(m_hdc,
						  xDest, yDest,
						  pImg->getDisplayWidth(), pImg->getDisplayHeight(),
						  memDC,
						  0, 0,
						  pDIB->bmiHeader.biWidth, pDIB->bmiHeader.biHeight,
						  blendfnct);
	}
	
#endif
	
	if(!iRes)
	{
		SetStretchBltMode(m_hdc, COLORONCOLOR);
		iRes = StretchDIBits(m_hdc,
							 xDest, yDest,
							 pImg->getDisplayWidth(), pImg->getDisplayHeight(),
							 0, 0,
							 pDIB->bmiHeader.biWidth, pDIB->bmiHeader.biHeight,
							 pBits, pDIB, DIB_RGB_COLORS, SRCCOPY);
	}
	
	if (iRes == GDI_ERROR)
	{
		UT_DebugOnly<DWORD> err = GetLastError();
		UT_DEBUGMSG(("StretchDIBits failed with err %d\n", err));
	}
}

HWND GR_Win32Graphics::getHwnd(void) const
{
	return m_hwnd;
}

void GR_Win32Graphics::setColorSpace(GR_Graphics::ColorSpace /*c*/)
{
	// TODO:  maybe?
	UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
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

	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(XAP_App::getApp());
	HINSTANCE hinst = pWin32App->getInstance();
	LPCTSTR cursor_name;                //TODO : CHECK

	switch (m_cursor)
	{
	case GR_CURSOR_CROSSHAIR:	
		cursor_name = IDC_CROSS;
		hinst = NULL;
		break;

	default:
	{
		// this assert makes debugging virtuall impossible !!!
		static bool bDoneThisAlready = false;
		if(!bDoneThisAlready)
		{
			bDoneThisAlready = true;
			UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
		}
		
	}
		/*FALLTHRU*/
	case GR_CURSOR_DEFAULT:
		cursor_name = IDC_ARROW;		// top-left arrow
		hinst = NULL;
		break;

	case GR_CURSOR_LINK:
	case GR_CURSOR_GRAB:
#ifndef IDC_HAND
		cursor_name = MAKEINTRESOURCE(IDC_ABIHAND);
#else
		if (UT_IsWin95())
			cursor_name = MAKEINTRESOURCE(IDC_ABIHAND);
		else
		{
			cursor_name = IDC_HAND;
			hinst = NULL;
		}
#endif
		break;

	case GR_CURSOR_EXCHANGE:
		cursor_name = MAKEINTRESOURCE(IDC_EXCHANGE);
		break;

	case GR_CURSOR_IBEAM:
		cursor_name = IDC_IBEAM;
		hinst = NULL;
		break;	

	case GR_CURSOR_RIGHTARROW:
		cursor_name = MAKEINTRESOURCE(IDC_ABIRIGHTARROW);
		break;

	case GR_CURSOR_LEFTARROW:
		cursor_name = IDC_ARROW;		// TODO change this
		hinst = NULL;
		break;

	case GR_CURSOR_DOWNARROW:
		cursor_name = MAKEINTRESOURCE(IDC_ABIDOWNARROW);
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

	HCURSOR hCursor = LoadCursor(hinst,cursor_name); //TODO: Leaking resource
	if (hCursor != NULL)
		SetCursor(hCursor);
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

	#ifdef GR_GRAPHICS_DEBUG
	UT_DEBUGMSG(("GR_Win32Graphics::fillRect GR_Color3D  %x %u %u %u %u\n",  c, x, y, w, h));	
	#endif
	
	RECT r;
	r.left = (UT_sint32)((double)_tduX(x) * m_fXYRatio);
	r.top = _tduY(y);
	r.right = (UT_sint32)((double)_tduX(x+w) * m_fXYRatio);
	r.bottom = _tduY(y+h);

	// This might look wierd (and I think it is), but it's MUCH faster.
	// CreateSolidBrush is dog slow.
	HDC hdc = m_hdc;
	COLORREF clr = RGB(GetRValue(m_3dColors[c]), GetGValue(m_3dColors[c]), GetBValue(m_3dColors[c]));
	
	const COLORREF cr = ::SetBkColor(hdc,  clr);
	::ExtTextOutW(hdc, 0, 0, ETO_OPAQUE, &r, NULL, 0, NULL);
	::SetBkColor(hdc, cr);
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

	LOGFONTW lf;
	memset(&lf, 0, sizeof(lf));

	TEXTMETRICW tm;
	memset(&tm, 0, sizeof(tm));

	// TODO i'm not sure why we special case these, but the other
	// TODO code did, so i'm going to here.

	if (g_ascii_strcasecmp(szFontName, "serif") == 0)
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_ROMAN;
	else if (g_ascii_strcasecmp(szFontName, "sans-serif") == 0)
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
	else if (g_ascii_strcasecmp(szFontName, "cursive") == 0)
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_SCRIPT;
	else if (g_ascii_strcasecmp(szFontName, "fantasy") == 0)
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DECORATIVE;
	else if (g_ascii_strcasecmp(szFontName, "monospace") == 0)
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_MODERN;
	else
	{
		UT_Win32LocaleString str;
		str.fromUTF8 (szFontName);
			lstrcpynW(lf.lfFaceName, 
				    str.c_str(),
			        LF_FACESIZE);
	}

	// let the system create the font with the given name or
	// properties and then query it and see what was actually
	// created.  hopefully, this will let us more accurately
	// reflect what is being seen on screen.

	HFONT hFont = CreateFontIndirectW(&lf);
	HDC hdc = CreateDCW(L"DISPLAY",NULL,NULL,NULL);
	HFONT hFontOld = (HFONT) SelectObject(hdc,hFont);
	GetTextMetricsW(hdc,&tm);
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

/*!
    hdc - the primary hdc on which we are expected to draw
    printHDC - the hdc for which to retrive metrics
*/
GR_Win32Font::GR_Win32Font(LOGFONTW & lf, double fPoints, HDC hdc, HDC printHDC)
:	m_hdc(hdc),
	m_xhdc(0), // once all the processing is done, this is changed to printHDC
	m_yhdc(0), // once all the processing is done, this is changed to printHDC
	m_defaultCharWidth(0),
	m_layoutFont (0),
	m_tm(TEXTMETRICW()),
	m_bGUIFont(false),
	m_fPointSize(fPoints)
{
	m_iHeight = abs(lf.lfHeight);

	m_layoutFont = CreateFontIndirectW(&lf); // this is what we see to start with

	if(!m_layoutFont)
	{
		LOG_WIN32_EXCPT("CreateFontIndirectFailed");
		return;
	}
	
	insertFontInCache (m_iHeight, m_layoutFont);

	HFONT printFont = m_layoutFont;
	
	if(hdc != printHDC)
	{
		int nPrintLogPixelsY = GetDeviceCaps(printHDC, LOGPIXELSY);

		// use the point size rather than the screen pixel size to minimise rounding error
		// (lfHeight already carries a rounding error from conversion pts -> screen pixels)
		// lf.lfHeight = MulDiv(lf.lfHeight, nPrintLogPixelsY, nLogPixelsY);
		lf.lfHeight =  (int)(-fPoints * (double)nPrintLogPixelsY / 72.00 + 0.5);
		printFont = CreateFontIndirectW(&lf);
		
		if(!printFont)
		{
			LOG_WIN32_EXCPT("CreateFontIndirectFailed");
			return;
		}

		insertFontInCache(abs(lf.lfHeight), printFont);

	}
	
	//
	// TMN: We need to initialize 'this' to _something_, why we use the
	// screen DC. Note that this is a *bad hack* forced by bad design. :-(
	//
	if (!hdc || !printHDC)
	{
		// NOW what? We can't throw an exeption, and this object isn't
		// legal yet...
		LOG_WIN32_EXCPT("No DC")
		m_layoutFont = NULL;
		return;
	}
	else
	{
		// now we need to retrieve the y-axis metrics
		HFONT hOldFont = (HFONT)SelectObject(printHDC, printFont);
		if (hOldFont == (HFONT)GDI_ERROR)
		{
			LOG_WIN32_EXCPT("Could not select font into DC.")
			m_layoutFont = NULL;
			return;
		}
		else
		{
			// Setting the m_hashKey 
			wchar_t lpFaceName[1000];
			
			GetTextFaceW(printHDC, 1000, lpFaceName );

			_updateFontYMetrics(hdc, printHDC);

			m_hashKey = UT_std_string_sprintf("%s-%ld-%ld-%d-%d-%d-%d-%d",
							  lpFaceName,
							  m_tm.tmHeight, m_tm.tmWeight, m_tm.tmItalic, m_tm.tmUnderlined,
							  m_tm.tmStruckOut,
							  GetDeviceCaps(hdc, LOGPIXELSX),
							  GetDeviceCaps(hdc, LOGPIXELSY));

			// now we measure the default character
			// 
			// when we are called, the char widths might not exist yet; we
			// will force initialisation by a bogus call to
			// getCharWidthFromCache(); by measuring the space, we will
			// preload the entire Latin1 page
			getCharWidthFromCache(' ');
			
			UINT d = m_tm.tmDefaultChar;

			UT_return_if_fail(_getCharWidths());
			_getCharWidths()->setCharWidthsOfRange(printHDC, d, d);
			m_defaultCharWidth = getCharWidthFromCache(d);

			// this is not good SelectObject() is very expensive, and chances are this
			// call is unnecessary, but because of the design of the graphics class we
			// have no way of knowing
			SelectObject(printHDC, hOldFont);
		}

		m_xhdc = printHDC;
		m_yhdc = printHDC;
	}
}

GR_Win32Font * GR_Win32Font::newFont(LOGFONTW &lf, double fPoints, HDC hdc, HDC printHDC)
{
	GR_Win32Font * f = new GR_Win32Font(lf, fPoints, hdc, printHDC);

	if(!f || !f->getFontHandle())
	{
		delete f;
		f = NULL;
	}

	return f;
}


GR_Win32Font::~GR_Win32Font()
{
	// We currently can't delete the font, since it might
	// be selected into a DC. Also, we don't keep track of previously
	// selected font [into the DC] why it's impossible to revert to that
	// one! :-<
	// But, let's try it like this...
	//
	// the GetObjectType call with 0 throws a first chance exception
	// (this whole thing is rather messy)

	DWORD dwObjType;
	bool bIsDC = false;

	if(m_hdc) 
	{
		dwObjType = GetObjectType((HGDIOBJ)m_hdc);
		bIsDC = (dwObjType == OBJ_DC || dwObjType == OBJ_MEMDC);
	}

	for (UT_sint32 i = 0; i < m_allocFonts.getItemCount(); ++i)
	  {
		  allocFont *p = (allocFont *)m_allocFonts.getNthItem(i);

		  if(m_hdc) 
		  {
			  if (!bIsDC || p->hFont != (HFONT)::GetCurrentObject(m_hdc, OBJ_FONT))
			  {
				  DeleteObject(p->hFont);
			  }
		  }

		  delete p;
	  }
}

/*!
    hdc -- handle to the primary DC on which we draw
    printHDC -- handle to the DC on which we want to measure the font (typically the printer)
                can be NULL (in which case hdc will be used

    NB: the caller must ensure that the corresponding HFONT is already selected onto the
    DC represented by printHDC (or hdc if NULL).
*/
void GR_Win32Font::_updateFontYMetrics(HDC hdc, HDC printHDC)
{
	// have to have at least the prinary DC
	UT_return_if_fail( hdc );
	
	// only do this if we have reason to believe the cached values are stale
	if(!printHDC)
	{
		printHDC = hdc;
	}

	// we have to remeasure if (a) the printer changed, or (b) the primary device changed
	if(printHDC != m_yhdc)
	{
		GetTextMetricsW(printHDC,&m_tm);

		// now we have to scale the metrics down from the printHDC to our layout units
		// 
		// (we used to scale the metrics down to the device resolution, but that since we
		// then scale it back up to layout units all the time, this caused us huge
		// rounding error) Tomas, 16/9/05
		// 
		// NB: we do not want to obtain the metrics directly for the primary dc
		// Windows is designed to get slightly bigger font for the screen than the
		// user asks for (MS say to improve readibility), and this causes us
		// non-WYSIWYG behaviour (basically our characters are wider on screen than on
		// the printer and our lines are further apart)
		int nLogPixelsY      = GR_Graphics::getResolution();
		int nPrintLogPixelsY = GetDeviceCaps(printHDC, LOGPIXELSY);

		if(nLogPixelsY != nPrintLogPixelsY)
		{
			m_tm.tmAscent  = MulDiv(m_tm.tmAscent, nLogPixelsY, nPrintLogPixelsY);
			m_tm.tmDescent = MulDiv(m_tm.tmDescent, nLogPixelsY, nPrintLogPixelsY);
			m_tm.tmHeight  = MulDiv(m_tm.tmHeight, nLogPixelsY, nPrintLogPixelsY);
		}

		// now remember what HDC these values are for
		m_yhdc = printHDC;
	}
}


/*! hdc - handle to device context for which the measurment is required */
UT_uint32 GR_Win32Font::getAscent(HDC hdc, HDC printHDC)
{
	if(!m_bGUIFont)
		_updateFontYMetrics(hdc, printHDC);
	return m_tm.tmAscent;
}

/*! hdc - handle to device context for which the measurment is required */
UT_uint32 GR_Win32Font::getDescent(HDC hdc, HDC printHDC)
{
	if(!m_bGUIFont)
		_updateFontYMetrics(hdc, printHDC);
	return m_tm.tmDescent;
}

/*! hdc - handle to device context for which the measurment is required */
UT_uint32 GR_Win32Font::getHeight(HDC hdc, HDC printHDC)
{
	if(!m_bGUIFont)
		_updateFontYMetrics(hdc, printHDC);
	return m_tm.tmHeight;
}


UT_sint32 GR_Win32Font::measureUnremappedCharForCache(UT_UCSChar cChar) const
{
#ifndef ABI_GRAPHICS_PLUGIN_NO_WIDTHS
	// first of all, handle 0-width spaces ...
	if(cChar == 0xFEFF || cChar == 0x200b || cChar == UCS_LIGATURE_PLACEHOLDER)
		return 0;

	// this function is only called when there is no value in the
	// cache; we will set not only the value for the character
	// required, but also for everything on the same char-width page
    UT_return_val_if_fail(_getCharWidths(),0);

	// calculate the limits of the 256-char page
	UT_UCS4Char base = (cChar & 0xffffff00);
	UT_UCS4Char limit = (cChar | 0x000000ff);
	HDC hdc = CreateDCW(L"DISPLAY",NULL,NULL,NULL);
	SelectObject(hdc,m_layoutFont);
	_getCharWidths()->setCharWidthsOfRange(hdc, base, limit);
	DeleteDC(hdc);
	return _getCharWidths()->getWidth(cChar);
#else
	UT_return_val_if_fail(UT_NOT_IMPLEMENTED,0);
#endif
}

/*!
    UT_Rect of glyph in Logical units.
	rec.left = bearing Left (distance from origin to start)
	rec.width = width of the glyph
	rec.top = distance from the origin to the top of the glyph
	rec.height = total height of the glyph

    This function will only work on win NT -- check the return value before doing anything
    with the rectangle (win9x implementation would have to use GetGlyphOutlineA() and
    convert the UT_UCS4Char to an appropriate ansi value 
*/
bool GR_Win32Font::glyphBox(UT_UCS4Char g, UT_Rect & rec, GR_Graphics * pG)
{
	UT_return_val_if_fail( pG, false );
	GR_Win32Graphics * pWin32Gr = (GR_Win32Graphics *)pG;

	// we do all measurements on the printer DC and only later scale down to display to
	// ensure WYSIWYG behaviour
	HDC hdc = getPrimaryHDC();
	HDC printDC = pWin32Gr->getPrintDC() ? pWin32Gr->getPrintDC() : hdc;
	
	GLYPHMETRICS gm;
	MAT2 m = {{0,1}, {0,0}, {0,0}, {0,1}};

	int nPrintLogPixelsY = GetDeviceCaps(printDC, LOGPIXELSY);// resolution of printer
	int nPrintLogPixelsX = GetDeviceCaps(printDC, LOGPIXELSX);

	UT_uint32 pixels = getUnscaledHeight();

	// scale the pixel size to the printer resolution and get the font
	if(!m_bGUIFont)
	{
		// use point size to reduce rounding errors
		// pixels = MulDiv(pixels, nPrintLogPixelsY, 72);
		pixels = (int)(m_fPointSize * (double)nPrintLogPixelsY / 72.0 + 0.5);
	}
	
	HFONT pFont = getFontFromCache(pixels, false, 100);
	if (!pFont)
	{
		fetchFont(pixels);
		pFont = getFontFromCache(pixels, false, 100);
	}

	// select our font into the printer DC
	HGDIOBJ hRet = SelectObject(printDC, pFont);
	UT_return_val_if_fail( hRet != (void*)GDI_ERROR, false);

	// remember which font we selected into the dc
	if(printDC == pWin32Gr->getPrintDC())
		pWin32Gr->setPrintDCFontAllocNo(getAllocNumber());

	if(printDC == pWin32Gr->getPrimaryDC())
		pWin32Gr->setDCFontAllocNo(getAllocNumber());
	
	DWORD iRet = GDI_ERROR;
	
	if (UT_IsWinNT())
	{
		iRet = GetGlyphOutlineW(printDC, (UINT)g, GGO_METRICS, &gm, 0, NULL, &m);
	}
	else
	{
		// GetGlyphOutlineA() ... using the GGO_GLYPH_INDEX = 0x0080 to get the ttf glyph 
		// for the unicode character set in ANSI

		// first of all, we need to translate the ucs4 value to the index; we use
		// GetCharacterPlacementW() for this. Event though the docs state that this
		// function is only available on win9x via unicows, this is in fact incorrect
		// (whether the 9x version of the function is fully functional I do not know, but
		// it does glyph index lookup).
		UINT iIndx;
		GCP_RESULTSW gcpResult;
		memset(&gcpResult, 0, sizeof(gcpResult));

		gcpResult.lStructSize = sizeof(GCP_RESULTS);
		
		// w32api changed lpGlyphs from UINT * to LPWSTR to match MS PSDK in w32api v2.4
#if defined(__MINGW32__) && (__W32API_MAJOR_VERSION == 2 && __W32API_MINOR_VERSION < 4)
		gcpResult.lpGlyphs = (UINT *) &iIndx;	// Character glyphs
#else			
		gcpResult.lpGlyphs = (LPWSTR) &iIndx;	// Character glyphs
#endif			
		gcpResult.nGlyphs = 1;  // Array size

		if(GetCharacterPlacementW(printDC, (LPCWSTR)&g, 1, 0, &gcpResult, 0))
		{
			iRet = GetGlyphOutlineA(printDC, iIndx, 0x0080 | GGO_METRICS, &gm, 0, NULL, &m);
		}
		else
		{
			// OK, the GetCharacterPlacementW function probably not present, we just do
			// our best with the ANSI stuff
			iRet = GetGlyphOutlineA(printDC, g, GGO_METRICS, &gm, 0, NULL, &m);
		}
	}
	
	if(GDI_ERROR == iRet)
		return false;

	rec.left   = gm.gmptGlyphOrigin.x;
	rec.top    = gm.gmptGlyphOrigin.y;
	rec.width  = gm.gmBlackBoxX;
	rec.height = gm.gmBlackBoxY;
	
	// the metrics are in device units, scale them to layout
	int iResolution = pG->getResolution();
	rec.height  = MulDiv(rec.height, iResolution, nPrintLogPixelsY);
	rec.top  = MulDiv(rec.top, iResolution, nPrintLogPixelsY);

	rec.width  = MulDiv(rec.width, iResolution, nPrintLogPixelsX);
	rec.left  = MulDiv(rec.left, iResolution, nPrintLogPixelsX);

	UT_DEBUGMSG(("gr_Win32Graphics::glyphBox(l=%d, t=%d, w=%d, h=%d\n", rec.left, rec.top, rec.width, rec.height));
	return true;
}

/*!
	Implements a GR_CharWidths.
	Override if you which to instanciate a subclass.
 */
GR_CharWidths* GR_Win32Font::newFontWidths(void) const
{
#ifndef ABI_GRAPHICS_PLUGIN_NO_WIDTHS
	return new GR_Win32CharWidths();
#else
	UT_return_val_if_fail(UT_NOT_IMPLEMENTED,NULL);
#endif
}

HFONT GR_Win32Font::getFontFromCache(UT_uint32 pixelsize, bool /*bIsLayout*/, UT_uint32 /*zoomPercentage*/) const
{
	UT_uint32 l = 0;
	UT_uint32 count = m_allocFonts.getItemCount();
	allocFont *entry;

	while (l < count)
	  {
		  entry = (allocFont *)m_allocFonts.getNthItem(l);
		  if (entry && entry->pixelSize == pixelsize)
			  return entry->hFont;
		  l++;
	  }

	return NULL;
}

void GR_Win32Font::insertFontInCache(UT_uint32 pixelsize, HFONT pFont) const
{
	allocFont *entry = new allocFont;
	entry->pixelSize = pixelsize;
	entry->hFont = pFont;

	m_allocFonts.push_back(static_cast<void *>(entry));
}

void GR_Win32Font::fetchFont(UT_uint32 pixelsize) const
{
	LOGFONTW lf;

	GetObjectW(m_layoutFont, sizeof(LOGFONTW), &lf);
	lf.lfHeight = pixelsize;

	if (lf.lfHeight>0) lf.lfHeight = - lf.lfHeight;

	HFONT pFont = CreateFontIndirectW(&lf);

	insertFontInCache(pixelsize, pFont);
}

HFONT GR_Win32Font::getDisplayFont(GR_Graphics * pG)
{
	UT_uint32 zoom = m_bGUIFont ? 100 : pG->getZoomPercentage();
	UT_uint32 pixels = m_bGUIFont ? m_iHeight : m_iHeight*zoom/100;
		
	HFONT pFont = getFontFromCache(pixels, false, zoom);
	if (pFont)
		return pFont;

	fetchFont(pixels);
	return getFontFromCache(pixels, false, zoom);
}

UT_sint32 GR_Win32Font::measureUnRemappedChar(UT_UCSChar c, UT_uint32 * /*height*/)
{
#ifndef ABI_GRAPHICS_PLUGIN_NO_WIDTHS
	// first of all, handle 0-width spaces ...
	if(c == 0xFEFF || c == 0x200b || c == UCS_LIGATURE_PLACEHOLDER)
		return 0;

	UT_sint32 iWidth = getCharWidthFromCache(c);
	return iWidth;
#else
	UT_return_val_if_fail(UT_NOT_IMPLEMENTED,0);
#endif
}

void GR_Win32Font::selectFontIntoDC(GR_Graphics * pGr, HDC hdc)
{
	HFONT hFont = getDisplayFont(pGr);
	UT_DebugOnly<HGDIOBJ> hRet = SelectObject(hdc, hFont);

	// hate having to do the cast, here
	UT_ASSERT_HARMLESS( hRet != (void*)GDI_ERROR);
}

void GR_Win32Graphics::polygon(const UT_RGBColor& c, const UT_Point *pts,UT_uint32 nPoints)
{
	HPEN hPen = CreatePen(PS_SOLID, (UT_sint32)((double)_tduR(m_iLineWidth) * m_fXYRatio), RGB(c.m_red, c.m_grn, c.m_blu));
	HPEN hOldPen = (HPEN)SelectObject(m_hdc,hPen);

	HBRUSH hBrush = CreateSolidBrush(RGB(c.m_red, c.m_grn, c.m_blu));
	HBRUSH hOldBrush = (HBRUSH)SelectObject(m_hdc, hBrush);

	POINT * points = new POINT[nPoints];
	UT_ASSERT(points);

	for (UT_uint32 i = 0; i < nPoints; ++i)
	{
		points[i].x = (UT_sint32)((double)_tduX(pts[i].x) * m_fXYRatio);
		points[i].y = _tduY(pts[i].y);
	}

	Polygon(m_hdc, points, nPoints);

	(void) SelectObject(m_hdc, hOldBrush);
	DeleteObject(hBrush);

	(void) SelectObject(m_hdc, hOldPen);
	DeleteObject(hPen);

	delete[] points;
}

bool GR_Win32Graphics::_setTransform(const GR_Transform & /*tr*/)
{
#if 0
	if(UT_IsWinNT())
	{
		XFORM xForm;
		float fScale = static_cast<float>(static_cast<float>(tlu(1)) * static_cast<float>(getZoomPercentage()) / 100.0);
	
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
		UT_sint32 iScale = static_cast<UT_sint32>((static_cast<float>(tlu(1)) * static_cast<float>(getZoomPercentage())/100.0);
		
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


void GR_Win32Graphics::saveRectangle(UT_Rect & r, UT_uint32 iIndx) 
{		
	UT_Rect * oldR = NULL;
	m_vSaveRect.setNthItem(iIndx, (void*)new UT_Rect(r),(const void **)&oldR);
	DELETEP(oldR);

	UT_uint32 iWidth = (UT_sint32)((double)_tduR(r.width) * m_fXYRatio);
	UT_uint32 iHeight = _tduR(r.height);
	UT_sint32 x = (UT_sint32)((double)_tduX(r.left) * m_fXYRatio);
	UT_sint32 y = _tduY(r.top);	 

	#ifdef GR_GRAPHICS_DEBUG	
	UT_DEBUGMSG(("GR_Win32Graphics::saveRectangle %u, %u %u %u %u\n", iIndx,
		x,y, iWidth, iHeight));	
	#endif

	BITMAPINFO bmi;

	BYTE *imagedata;
	HDC	hMemDC = CreateCompatibleDC(m_hdc);
		
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 	
	bmi.bmiHeader.biWidth = iWidth;
	bmi.bmiHeader.biHeight = iHeight;
	bmi.bmiHeader.biPlanes = 1; 
	bmi.bmiHeader.biBitCount = 24; // as we want true-color
	bmi.bmiHeader.biCompression = BI_RGB; // no compression
	bmi.bmiHeader.biSizeImage = (((iWidth * bmi.bmiHeader.biBitCount + 31) & ~31) >> 3) * iHeight; 
	bmi.bmiHeader.biXPelsPerMeter = 0;
	bmi.bmiHeader.biYPelsPerMeter = 0; 
	bmi.bmiHeader.biClrImportant = 0;
	bmi.bmiHeader.biClrUsed = 0; // we are not using palette
		
	HBITMAP hBit = CreateDIBSection(hMemDC,&bmi,DIB_RGB_COLORS,(void**)&imagedata,0,0);
	GdiFlush();

	HBITMAP hOld = (HBITMAP) SelectObject(hMemDC, hBit);
	BitBlt(hMemDC, 0, 0, iWidth, iHeight, m_hdc, x, y, SRCCOPY);
	hBit =  (HBITMAP)SelectObject(hMemDC, hOld);
	DeleteDC(hMemDC);

	m_vSaveRectBuf.setNthItem(iIndx, (void*) hBit, (const void**)&hOld);
	DeleteObject(hOld);
}

void GR_Win32Graphics::restoreRectangle(UT_uint32 iIndx) 
{
	UT_Rect * r = (UT_Rect*)m_vSaveRect.getNthItem(iIndx);
	HBITMAP hBit = (HBITMAP)m_vSaveRectBuf.getNthItem(iIndx);
	
	UT_return_if_fail(r);
	UT_ASSERT(hBit);
	
	UT_uint32 iWidth = (UT_sint32)((double)_tduR(r->width) * m_fXYRatio);
	UT_uint32 iHeight = _tduR(r->height);
	UT_sint32 x = (UT_sint32)((double)_tduX(r->left) * m_fXYRatio);
	UT_sint32 y = _tduY(r->top);			

	#ifdef GR_GRAPHICS_DEBUG
	UT_DEBUGMSG(("GR_Win32Graphics::restoreRectangle %u, %u %u %u %u\n", iIndx,
		x,y, iWidth, iHeight));	
	#endif	

	HDC		hMemDC = CreateCompatibleDC(m_hdc);
	HBITMAP hOld = (HBITMAP) SelectObject(hMemDC, hBit);
	
	BitBlt(m_hdc, x, y, iWidth, iHeight, hMemDC, 0, 0, SRCCOPY);
	SelectObject(hMemDC, hOld);
	DeleteDC(hMemDC);		
}

void GR_Win32Graphics::flush(void)
{	
	GdiFlush();	
}

//--------------------------------------------------------------------------/
//--------------------------------------------------------------------------/

BITMAPINFO * GR_Win32Graphics::ConvertDDBToDIB(HBITMAP bitmap, HPALETTE hPal, DWORD dwCompression) {

	BITMAP				bm;	
	BITMAPINFOHEADER	bi;	
	LPBITMAPINFOHEADER 	lpbi;	
	DWORD				dwLen;
	HANDLE				hDIB;	
	HANDLE				handle;	
	HDC 				hDC;	
	
	if (dwCompression == BI_BITFIELDS)
		return NULL;

	if (hPal == NULL)
		hPal = (HPALETTE)GetStockObject(DEFAULT_PALETTE);	
		
	// Get bitmap information
	::GetObjectW(bitmap, sizeof(bm),(LPSTR)&bm);	
	
	// Initialize the bitmapinfoheader
	bi.biSize			= sizeof(BITMAPINFOHEADER);	
	bi.biWidth			= bm.bmWidth;
	bi.biHeight 		= bm.bmHeight;	
	bi.biPlanes 		= 1;
	bi.biBitCount		= bm.bmPlanes * bm.bmBitsPixel;
	bi.biCompression	= dwCompression;	
	bi.biSizeImage		= 0;	
	bi.biXPelsPerMeter	= 0;
	bi.biYPelsPerMeter	= 0;	
	bi.biClrUsed		= 0;	
	bi.biClrImportant	= 0;
	
	// Compute the size of the  infoheader and the color table
	int nColors = (1 << bi.biBitCount);	
	if( nColors > 256 ) 		
		nColors = 0;
	dwLen  = bi.biSize + nColors * sizeof(RGBQUAD);
	
	// We need a device context to get the DIB from	
	hDC = CreateCompatibleDC(m_hdc);
	hPal = SelectPalette(hDC, hPal, TRUE);	
	RealizePalette(hDC);
	
	// Allocate enough memory to hold bitmapinfoheader and color table
	hDIB = g_try_malloc(dwLen);	
	if (!hDIB){
		SelectPalette(hDC, hPal, TRUE);		
		DeleteDC(hDC);
		return NULL;	
		}
	lpbi = (LPBITMAPINFOHEADER)hDIB;	
	*lpbi = bi;
	
	// Call GetDIBits with a NULL lpBits param, so the device driver 
	// will calculate the biSizeImage field 
	GetDIBits(
		hDC, 
		bitmap, 
		0L, 
		(DWORD)bi.biHeight,
		(LPBYTE)NULL, 
		(LPBITMAPINFO)lpbi, 
		(DWORD)DIB_RGB_COLORS
		);	
	bi = *lpbi;
	
	// If the driver did not fill in the biSizeImage field, then compute it
	// Each scan line of the image is aligned on a DWORD (32bit) boundary
	if (bi.biSizeImage == 0) {
		bi.biSizeImage = ((((bi.biWidth * bi.biBitCount) + 31) & ~31) / 8) * bi.biHeight;
	
		// If a compression scheme is used the result may infact be larger
		// Increase the size to account for this.		
		if (dwCompression != BI_RGB)
			bi.biSizeImage = (bi.biSizeImage * 3) / 2;
		}
	
	// Realloc the buffer so that it can hold all the bits	
	dwLen += bi.biSizeImage;
	handle = g_try_realloc(hDIB, dwLen);
	if (handle)
		hDIB = handle;	
	else{
		g_free(hDIB);		
		// Reselect the original palette
		SelectPalette(hDC,hPal,TRUE);		
		DeleteDC(hDC);
		return NULL;	
		}
	
	// Get the bitmap bits	
	lpbi = (LPBITMAPINFOHEADER)hDIB;	
	
	// FINALLY get the DIB
	BOOL bGotBits = GetDIBits( 
		hDC, 
		bitmap, 
		0L,							// Start scan line				
		(DWORD)bi.biHeight,			// # of scan lines
		(LPBYTE)lpbi 				// address for bitmap bits
		+ (bi.biSize + nColors * sizeof(RGBQUAD)),
		(LPBITMAPINFO)lpbi,			// address of bitmapinfo
		(DWORD)DIB_RGB_COLORS		// Use RGB for color table
		);
	
	if (!bGotBits)	{
		g_free(hDIB);				
		SelectPalette(hDC,hPal,TRUE);		
		DeleteDC(hDC);
		return NULL;	
		}	
		
	SelectPalette(hDC,hPal,TRUE);	
	DeleteDC(hDC);

	return (BITMAPINFO*)hDIB;
	}

//--------------------------------------------------------------------------/
//--------------------------------------------------------------------------/

GR_Image * GR_Win32Graphics::genImageFromRectangle(const UT_Rect & r) {

	UT_uint32 iWidth = (UT_sint32)((double)tdu(r.width) * m_fXYRatio);
	UT_uint32 iHeight = tdu(r.height);
	UT_sint32 x = (UT_sint32)((double)tdu(r.left) * m_fXYRatio);
	UT_sint32 y = tdu(r.top);

	UT_return_val_if_fail((iWidth > 0) && (iHeight > 0) && (y >= 0) && (x >= 0), NULL);

	BITMAPINFO bmi; 
	BYTE *imagedata;
	HDC hMemDC = CreateCompatibleDC(m_hdc);
		
	bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER); 	
	bmi.bmiHeader.biWidth = iWidth;
	bmi.bmiHeader.biHeight = iHeight;
	bmi.bmiHeader.biPlanes = 1; 
	bmi.bmiHeader.biBitCount = 24; // as we want true-color
	bmi.bmiHeader.biCompression = BI_RGB; // no compression
	bmi.bmiHeader.biSizeImage = (((iWidth * bmi.bmiHeader.biBitCount + 31) & ~31) >> 3) * iHeight; 
	bmi.bmiHeader.biXPelsPerMeter = 0;
	bmi.bmiHeader.biYPelsPerMeter = 0; 
	bmi.bmiHeader.biClrImportant = 0;
	bmi.bmiHeader.biClrUsed = 0; // we are not using palette
		
	HBITMAP hBit = CreateDIBSection(hMemDC,&bmi,DIB_RGB_COLORS,(void**)&imagedata,0,0);
	
	GdiFlush();

	HBITMAP hOld = (HBITMAP) SelectObject(hMemDC, hBit);
		BitBlt(hMemDC, 0, 0, iWidth, iHeight, m_hdc, x, y, SRCCOPY);
		hBit =  (HBITMAP)SelectObject(hMemDC, hOld);
	DeleteDC(hMemDC);


	GR_Win32Image *img = new GR_Win32Image("Screenshot");
	img->setDIB((BITMAPINFO *)ConvertDDBToDIB(hBit, NULL, BI_RGB));
	return img;
	}


GR_Graphics * GR_Win32Graphics::graphicsAllocator(GR_AllocInfo &info)
{
#ifndef ABI_GRAPHICS_PLUGIN
	UT_return_val_if_fail(info.getType() == GRID_WIN32, NULL);
	
	GR_Win32AllocInfo &AI = (GR_Win32AllocInfo&)info;

	if(AI.m_pDocInfo)
	{
		// printer graphics required
		return new GR_Win32Graphics(AI.m_hdc, AI.m_pDocInfo, AI.m_hDevMode);
	}
	else
	{
		// screen graphics required
		return new GR_Win32Graphics(AI.m_hdc, AI.m_hwnd);
	}
#else
	UT_return_val_if_fail(UT_NOT_IMPLEMENTED,NULL);
#endif
}

#define _test_and_cleanup(x)                    \
if(!(x))                                        \
{                                               \
    UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN ); \
	goto cleanup;                               \
}

GR_Graphics * GR_Win32Graphics::getPrinterGraphics(const wchar_t * pPrinterName,
												   const wchar_t * pDocName)
{
	UT_return_val_if_fail( pDocName, NULL );
	GR_Win32Graphics *pGr = NULL;
	HGLOBAL hDM = NULL;
	HANDLE hPrinter = NULL;
	LONG iBuffSize = 0;
	HDC hPDC = NULL;
	DOCINFOW * pDI = NULL;
	PDEVMODEW pDM = NULL;
	
	bool bFreePN = false;

	// we will use '-' as a shortcut for default printer (the --print command has to have
	// a parameter)
	wchar_t * pPN = wcscmp(L"-", pPrinterName) == 0 ? NULL : (wchar_t *) pPrinterName;
	const wchar_t * pDriver = UT_IsWinNT() ? L"WINSPOOL" : NULL;
	
	if(!pPN)
	{
		pPN = UT_GetDefaultPrinterName();
		bFreePN = true;
	}

	_test_and_cleanup(pPN);

	hPDC = CreateDCW(pDriver, pPN, NULL, NULL);

	_test_and_cleanup(hPDC);
	
	_test_and_cleanup( OpenPrinterW(pPN, &hPrinter, NULL));
		
	// first, get the size of the buffer needed for the document mode
	iBuffSize = DocumentPropertiesW(NULL, hPrinter,	pPN, NULL, NULL, 0);
	_test_and_cleanup( iBuffSize );

	// must be global movable memory
	hDM = GlobalAlloc(GHND, iBuffSize);
	pDM = (PDEVMODEW)GlobalLock(hDM);
	_test_and_cleanup(hDM && 0<DocumentPropertiesW(NULL, hPrinter, pPN, pDM, NULL, DM_OUT_BUFFER));
	GlobalUnlock(hDM);

	// we have all we need to fill in the doc info structure ...
	pDI = (DOCINFOW*) UT_calloc(1, sizeof(DOCINFOW));
	_test_and_cleanup(pDI);

	memset(pDI, 0, sizeof(DOCINFOW));
	
	pDI->cbSize = sizeof(DOCINFOW);
	pDI->lpszDocName = pDocName;
	pDI->lpszOutput  = NULL; // for now we do not support printing into file from cmd line
	pDI->lpszDatatype = NULL;
	pDI->fwType = 0;
	
	{
		GR_Win32AllocInfo ai(hPDC, pDI, hDM);
	
		pGr = (GR_Win32Graphics *)XAP_App::getApp()->newGraphics(ai);
		UT_ASSERT_HARMLESS(pGr);
	}
	
 cleanup:
	if(bFreePN)
		g_free(pPN);

	if(hDM && !pGr)
		GlobalFree(hDM);

	if(pDI && !pGr)
		g_free(pDI);
	
	if(hPrinter)
		ClosePrinter(hPrinter);
	
	return pGr;
}


/*!
    This function absolutely must be called anytime the DEVMODE structure is modified in
    any way; it also must be called on the DEVMODE handle returned by PringDlg. Those who
    will modify DEVMODE structure and not call this function on it will be shot, without
    exception.

    The issue with the DEVMODE structure is this: it consists of two parts, a public part
    declared by the win32 headers, and a private part, the size and contents of which 
    depend on the printer. Before a printer can be passed the DEVMODE structure, two
    things have to be satisfied:

    1. The memory allocated for the structure has to be large enough to hold both the
       public and private parts of the structure; we obtain the handle from the
       PrintDlg, and it _should_ allocate the necessary size correctly. In any case, we
       cannot meddle with the size, since reallocating means loosing the private part.

    2. The settings represented by the public part have to be transferred into the private
       part; this is achieved by calling the DocumentProperties() function on the
       structure. So any time we modify the contents of DEVMODE in any way we have to call
       DocumentProperties() before we can pass the pointer / handle to any other
       function. In addition, I have a strong suspicion that the PrintDlg function only
       sets the public section, but does not call DocumentProperties(), so we have to do
       that ourselves once we get the handle back.
*/
bool GR_Win32Graphics::fixDevMode(HGLOBAL hDevMode)
{
	DEVMODEW * pDM = (DEVMODEW*)GlobalLock(hDevMode);
	UT_return_val_if_fail( pDM, false );
	
	// find out how big structure this particular printer really needs
	HANDLE      hPrinter;
	DWORD       dwNeeded, dwRet;

	//Start by opening the printer
	if(!OpenPrinterW((wchar_t*)& pDM->dmDeviceName, &hPrinter, NULL))
	{
		UT_String msg;
		UT_String_sprintf(msg,"Unable to open printer [%s]", (char*)& pDM->dmDeviceName);
		LOG_WIN32_EXCPT(msg.c_str());
		GlobalUnlock(hDevMode);
		UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
		return false;
	}
	
	dwNeeded = DocumentPropertiesW(NULL,hPrinter, (wchar_t*)& pDM->dmDeviceName, NULL, NULL, 0);

	if( (int) dwNeeded > pDM->dmSize + pDM->dmDriverExtra )
	{
		// we are knees deep in crap here ... log it into the users profile, so we have
		// some debug info to go on
		UT_String msg;
		UT_String_sprintf(msg,"DEVMODE too small [%s, needs %ld, got %d + %d]",
						  (char*)& pDM->dmDeviceName, dwNeeded, pDM->dmSize, pDM->dmDriverExtra);
		LOG_WIN32_EXCPT(msg.c_str());
		ClosePrinter(hPrinter);
		GlobalUnlock(hDevMode);
		UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
		return false;
	}
	
	// now get the printer driver to merge the data in the public section into its private part
	dwRet = DocumentPropertiesW(NULL,hPrinter, (wchar_t*)& pDM->dmDeviceName, pDM, pDM, DM_OUT_BUFFER | DM_IN_BUFFER);

	// g_free what needs be ...
	ClosePrinter(hPrinter);
	GlobalUnlock(hDevMode);

	UT_return_val_if_fail( dwRet == IDOK, false );
	return true;
}

DOCINFOW *GR_Win32Graphics::getDocInfo()
{
  DOCINFOW *di;
  
  di = (DOCINFOW*) UT_calloc(1, sizeof(DOCINFOW));
  memset( di, 0, sizeof(DOCINFOW) );
  di->cbSize = sizeof(DOCINFOW);
  di->lpszDocName = L"AbiWord Print Preview";
  di->lpszOutput = (LPWSTR) NULL;
  di->lpszDatatype = (LPWSTR) NULL;
  di->fwType = 0;
  
  return di;
}

/*
 *  Creates a HDC to the best printer to be used as a reference for CreateEnhMetaFile
 */
HDC GR_Win32Graphics::createbestmetafilehdc()
{
  DWORD neededsize;
  DWORD noprinters;
  LPPRINTER_INFO_5W printerinfo = NULL;
  int bestres = 0;
  HDC besthdc = 0;
  
  if (EnumPrintersW(PRINTER_ENUM_CONNECTIONS|PRINTER_ENUM_LOCAL, NULL, 5, NULL, 0,
		&neededsize, &noprinters)) 
  {
	printerinfo = (LPPRINTER_INFO_5W) malloc(neededsize);
  } else {
	return GetDC(NULL);
  }
  
  if (EnumPrintersW(PRINTER_ENUM_CONNECTIONS|PRINTER_ENUM_LOCAL, NULL, 5,
		   (LPBYTE)printerinfo, neededsize, &neededsize, &noprinters)) {
    // init best resolution for hdc=0, which is screen resolution:    
    HDC curhdc = GetDC(NULL);
    if (curhdc) {
      bestres = GetDeviceCaps(curhdc, LOGPIXELSX) + GetDeviceCaps(curhdc,
								  LOGPIXELSY);
      bestres = ReleaseDC(NULL, curhdc); 
    }
      
    for (UT_uint32 i = 0; i < noprinters; i++) {
      curhdc = CreateDCW(L"WINSPOOL", printerinfo[i].pPrinterName, NULL, NULL);
      if (curhdc) {
	int curres = GetDeviceCaps(curhdc, LOGPIXELSX) + GetDeviceCaps(curhdc,
								       LOGPIXELSY);
	if (curres > bestres) {
	  if (besthdc)
	    DeleteDC(besthdc);
	  bestres = curres;
	  besthdc = curhdc;
	} else {
	  DeleteDC(curhdc);
	}
      }
    }
  } else return GetDC(NULL);
  
  if (printerinfo) free(printerinfo);
  return besthdc;
}

void GR_Win32Graphics::getWidthAndHeightFromHWND(HWND h, int &width, int &height)
{
	RECT clientRect;
	GetClientRect(h, &clientRect);
	
	width = clientRect.right - clientRect.left;
	height = clientRect.bottom - clientRect.top;

	UT_ASSERT(clientRect.left == 0 && clientRect.top == 0);
}

void GR_Win32Graphics::_DeviceContext_MeasureBitBltCopySpeed(HDC sourceHdc, HDC destHdc, int width, int height)
{
	LARGE_INTEGER t1, t2, freq;
	
	QueryPerformanceCounter(&t1);
	BitBlt(destHdc, 0, 0, width, height, sourceHdc, 0, 0, SRCCOPY);
	QueryPerformanceCounter(&t2);
	
	QueryPerformanceFrequency(&freq);
#if DEBUG
	double blitSpeed = ((double)(t2.QuadPart - t1.QuadPart)) / ((double)freq.QuadPart);

	UT_DEBUGMSG(("ASFRENT: measured BitBlt speed: %lfs [client rectangle W = %d, H = %d]\n", 
			blitSpeed, width, height));
#endif
}

void GR_Win32Graphics::_DeviceContext_SwitchToBuffer()
{
	// get client area size
	int height(0), width(0);
	getWidthAndHeightFromHWND(m_hwnd, width, height);

	// set up the buffer
	m_bufferHdc = _DoubleBuffering_CreateBuffer(m_hdc, width, height);

	// copy the screen to the buffer
	BitBlt(m_bufferHdc, 0, 0, width, height, m_hdc, 0, 0, SRCCOPY);

	// save the current hdc & switch
	_HDCSwitchStack.push(new _HDCSwitchRecord(m_hdc));
	m_hdc = m_bufferHdc;
}

void GR_Win32Graphics::_DeviceContext_SwitchToScreen()
{
	_DeviceContext_RestorePrevHDCFromStack();

	// get client area size
	int height(0), width(0);
	getWidthAndHeightFromHWND(m_hwnd, width, height);
	
	// copy any modifications back to the screen
	BitBlt(m_hdc, 0, 0, width, height, m_bufferHdc, 0, 0, SRCCOPY);
	
	// free used resources
	_DoubleBuffering_ReleaseBuffer(m_bufferHdc);
}

void GR_Win32Graphics::_DeviceContext_RestorePrevHDCFromStack()
{
	_HDCSwitchRecord *switchRecord;
	_HDCSwitchStack.pop((void**)&switchRecord);
	m_hdc = switchRecord->oldHdc;
	delete switchRecord;
}

void GR_Win32Graphics::_DeviceContext_SuspendDrawing()
{
	// save the current hdc & switch them!
	_HDCSwitchStack.push(new _HDCSwitchRecord(m_hdc));
	m_hdc = m_dummyHdc;
}

void GR_Win32Graphics::_DeviceContext_ResumeDrawing()
{
	_DeviceContext_RestorePrevHDCFromStack();
}

void GR_Win32Graphics::_DoubleBuffering_SetUpDummyBuffer()
{
	m_dummyHdc = _DoubleBuffering_CreateBuffer(m_hdc, 0, 0);
}

void GR_Win32Graphics::_DoubleBuffering_ReleaseDummyBuffer()
{
	_DoubleBuffering_ReleaseBuffer(m_dummyHdc);
}

HDC GR_Win32Graphics::_DoubleBuffering_CreateBuffer(HDC compatibleWith, int width, int height)
{
	HDC resultingHdc = CreateCompatibleDC(compatibleWith);
	HBITMAP bufferBitmap = CreateCompatibleBitmap(compatibleWith, width, height);
	SelectObject(resultingHdc, bufferBitmap);
	return resultingHdc;
}

void GR_Win32Graphics::_DoubleBuffering_ReleaseBuffer(HDC hdc)
{
	DeleteObject(GetCurrentObject(hdc, OBJ_BITMAP));
	DeleteDC(hdc);
}
