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


#include <windows.h>

#include "gr_Win32Graphics.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"

#define FREEP(p)	do { if (p) free(p); } while (0)

/*****************************************************************/

static UT_Bool __isWinNT(void)
{
	static UT_Bool bInitialized = UT_FALSE;
	static OSVERSIONINFO os;

	if (!bInitialized)
	{
		os.dwOSVersionInfoSize = sizeof(os);
		BOOL bSuccess = GetVersionEx(&os);
		UT_ASSERT(bSuccess);
		bInitialized = UT_TRUE;
	}

	return (os.dwPlatformId == VER_PLATFORM_WIN32_NT);
}


void Win32Graphics::_constructorCommonCode(HDC hdc)
{
	UT_ASSERT(hdc);

	m_hdc = hdc;
	m_hwnd = 0;
	m_bPrint = UT_FALSE;
	m_bStartPrint = UT_FALSE;
	m_bStartPage = UT_FALSE;
	m_pFont = NULL;
	memset(m_aABCs, 0, 256*sizeof(ABC));
	memset(m_aCharWidths, 0, 256*sizeof(int));
}

Win32Graphics::Win32Graphics(HDC hdc, HWND hwnd)
{
	_constructorCommonCode(hdc);
	m_hwnd = hwnd;
}

Win32Graphics::Win32Graphics(HDC hdc, const DOCINFO * pDocInfo)
{
	_constructorCommonCode(hdc);
	m_bPrint = UT_TRUE;
	m_pDocInfo = pDocInfo;
}

UT_Bool Win32Graphics::queryProperties(DG_Graphics::Properties gp) const
{
	switch (gp)
	{
	case DGP_SCREEN:
		return UT_TRUE;
	case DGP_PAPER:
		return UT_FALSE;
	default:
		UT_ASSERT(0);
		return UT_FALSE;
	}
}


Win32Font::Win32Font(HFONT hFont)
{
	m_hFont = hFont;
}

DG_Font* Win32Graphics::findFont(
		const char* pszFontFamily, 
		const char* pszFontStyle, 
		const char* pszFontVariant, 
		const char* pszFontWeight, 
		const char* pszFontStretch, 
		const char* pszFontSize)
{
	LOGFONT lf;

	/*
		TODO we need to fill out the LOGFONT object such that
		we'll get the closest possible matching font.  For
		now, we're hard-coding a hack.
	*/
	memset(&lf, 0, sizeof(lf));
	lf.lfCharSet = ANSI_CHARSET;

	UT_sint32 iHeight = convertDimension(pszFontSize);
	lf.lfHeight = -(iHeight);

	// TODO note that we don't support all those other ways of expressing weight.
	if (0 == stricmp(pszFontWeight, "bold"))
	{
		lf.lfWeight = 700;
	}

	// We squash oblique into italic
	if (0 == stricmp(pszFontStyle, "italic") || 0 == stricmp(pszFontStyle, "oblique"))
	{
		lf.lfItalic = TRUE;
	}

	// TODO note that we currently think pszFontFamily is a single name, not a list!
	// TODO why don't these generic family names work?!?
	if (0 == stricmp(pszFontFamily, "serif"))
	{
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_ROMAN;
	}
	else if (0 == stricmp(pszFontFamily, "sans-serif"))
	{
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_SWISS;
	}
	else if (0 == stricmp(pszFontFamily, "cursive"))
	{
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_SCRIPT;
	}
	else if (0 == stricmp(pszFontFamily, "fantasy"))
	{
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DECORATIVE;
	}
	else if (0 == stricmp(pszFontFamily, "monospace"))
	{
		lf.lfPitchAndFamily = DEFAULT_PITCH | FF_MODERN;
	}
	else
	{
		strcpy(lf.lfFaceName, pszFontFamily);
	}

	HFONT hFont = CreateFontIndirect(&lf);
	UT_ASSERT(hFont);	// TODO perhaps this should not be an assert.  Can the call fail without a coding error?
	return new Win32Font(hFont);
}

void Win32Graphics::drawChars(const UT_UCSChar* pChars, int iCharOffset, int iLength, UT_sint32 xoff, UT_sint32 yoff)
{
	UT_ASSERT(pChars);

	SelectObject(m_hdc, m_pFont->getHFONT());
	SetTextAlign(m_hdc, TA_LEFT | TA_TOP);
	SetBkMode(m_hdc, TRANSPARENT);		// TODO: remember and reset?

	ExtTextOutW(m_hdc, xoff, yoff, 0, NULL, pChars + iCharOffset, iLength, NULL);
	//TextOutW(m_hdc, xoff, yoff, pChars + iCharOffset, iLength);
}

void Win32Graphics::setFont(DG_Font* pFont)
{
	UT_ASSERT(pFont);	// TODO should we allow pFont == NULL?

	if (m_pFont == (static_cast<Win32Font*> (pFont)))
	{
		return;
	};
	
	m_pFont = static_cast<Win32Font*> (pFont);
	SelectObject(m_hdc, m_pFont->getHFONT());

#if 0
	if (0 == GetCharABCWidths(m_hdc, 0, 255, m_aABCs))
	{
		DWORD err = GetLastError();

		UT_ASSERT(err);
	}
#endif	

	UT_Bool bSuccess;
	if (__isWinNT())
		bSuccess = GetCharWidth32(m_hdc, 0, 255, m_aCharWidths);
	else
		bSuccess = GetCharWidth(m_hdc, 0, 255, m_aCharWidths);

	if (!bSuccess)
	{
		DWORD err = GetLastError();

		UT_ASSERT(err);
	}
}

HFONT Win32Font::getHFONT()
{
	return m_hFont;
}

UT_uint32 Win32Graphics::getFontHeight()
{
	UT_ASSERT(m_pFont);
	UT_ASSERT(m_hdc);

	TEXTMETRIC tm;

	GetTextMetrics(m_hdc, &tm);

	return tm.tmHeight;
}

UT_uint32 Win32Graphics::getFontAscent()
{
	UT_ASSERT(m_pFont);
	UT_ASSERT(m_hdc);

	TEXTMETRIC tm;

	GetTextMetrics(m_hdc, &tm);
	UT_ASSERT(tm.tmAscent > 0);

	return tm.tmAscent;
}

UT_uint32 Win32Graphics::getFontDescent()
{
	UT_ASSERT(m_pFont);
	UT_ASSERT(m_hdc);

	TEXTMETRIC tm;

	GetTextMetrics(m_hdc, &tm);

	return tm.tmDescent;
}

#if 0
UT_uint32 Win32Graphics::measureWidth(UT_UCSChar* s, int num)
{
	UT_ASSERT(m_pFont);
	UT_ASSERT(s);

	int i;

	/*
		This code calculates the width of the string in three distinct
		ways.  TODO eliminate two of them -- these are all here only
		for cross-checking the results.
	*/
	int iABCWidth = 0;
	for (i=0; i<num; i++)
	{
		UT_ASSERT(s[i] < 256);	// TODO we currently cannot deal with Unicode properly
		iABCWidth += m_aABCs[s[i]].abcA + m_aABCs[s[i]].abcB + m_aABCs[s[i]].abcC;
	}

	int iCharWidth = 0;
	for (i=0; i<num; i++)
	{
		UT_ASSERT(s[i] < 256);	// TODO we currently cannot deal with Unicode properly
		iCharWidth += m_aCharWidths[s[i]];
	}

	SIZE siz;
	SelectObject(m_hdc, m_pFont->getHFONT());
	GetTextExtentPoint32W(m_hdc, s, num, &siz);

	return siz.cx;
}
#endif

UT_uint32 Win32Graphics::measureString(const UT_UCSChar* s, int iOffset, int num, unsigned short* pWidths)
{
	UT_ASSERT(m_pFont);
	UT_ASSERT(s);

	int iCharWidth = 0;
	for (int i=0; i<num; i++)
	{
		// TODO should this assert be s[i+iOffset] ??
		UT_ASSERT(s[i] < 256);	// TODO we currently cannot deal with Unicode properly

		iCharWidth += m_aCharWidths[s[i + iOffset]];
		pWidths[i] = m_aCharWidths[s[i + iOffset]];
	}

	return iCharWidth;
}

UT_uint32 Win32Graphics::getResolution() const
{
	int result = GetDeviceCaps(m_hdc, LOGPIXELSY); // NOTE: assumes square pixels

	return result;
}

void Win32Graphics::setColor(UT_RGBColor& clr)
{
	SetTextColor(m_hdc, RGB(clr.m_red, clr.m_grn, clr.m_blu));
	m_clr = clr;
}

void Win32Graphics::drawLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2, UT_sint32 y2)
{
	// TODO do we really want the line width to ALWAYS be 1?
	HPEN hPen = CreatePen(PS_SOLID, 1, RGB(m_clr.m_red, m_clr.m_grn, m_clr.m_blu));
	HPEN hOldPen = (HPEN) SelectObject(m_hdc, hPen);

	MoveToEx(m_hdc, x1, y1, NULL);
	LineTo(m_hdc, x2, y2);

	(void) SelectObject(m_hdc, hOldPen);
	DeleteObject(hPen);
}

void Win32Graphics::xorLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2, UT_sint32 y2)
{
	// TODO do we really want the line width to ALWAYS be 1?
	HPEN hPen = CreatePen(PS_SOLID, 1, RGB(m_clr.m_red, m_clr.m_grn, m_clr.m_blu));
	int iROP = SetROP2(m_hdc, R2_XORPEN);
	HPEN hOldPen = (HPEN) SelectObject(m_hdc, hPen);

	MoveToEx(m_hdc, x1, y1, NULL);
	LineTo(m_hdc, x2, y2);

	(void) SelectObject(m_hdc, hOldPen);
	DeleteObject(hPen);
	(void) SetROP2(m_hdc, iROP);
}

void Win32Graphics::polyLine(UT_Point * pts, UT_uint32 nPoints)
{
	// TODO do we really want the line width to ALWAYS be 1?
	HPEN hPen = CreatePen(PS_SOLID, 1, RGB(m_clr.m_red, m_clr.m_grn, m_clr.m_blu));
	HPEN hOldPen = (HPEN) SelectObject(m_hdc, hPen);

	POINT * points = (POINT *)calloc(nPoints, sizeof(POINT));
	UT_ASSERT(points);

	for (UT_uint32 i = 0; i < nPoints; i++)
	{
		points[i].x = pts[i].x;
		points[i].y = pts[i].y;
	}

	Polyline(m_hdc, points, nPoints);

	(void) SelectObject(m_hdc, hOldPen);
	DeleteObject(hPen);
	FREEP(points);
}

void Win32Graphics::fillRect(UT_RGBColor& c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h)
{
	HBRUSH hBrush = CreateSolidBrush(RGB(c.m_red, c.m_grn, c.m_blu));

	RECT r;
	r.left = x;
	r.top = y;
	r.right = r.left + w;
	r.bottom = r.top + h;

	FillRect(m_hdc, &r, hBrush);
	DeleteObject(hBrush);
}

UT_Bool Win32Graphics::startPrint(void)
{
	UT_ASSERT(m_bPrint);
	UT_ASSERT(!m_bStartPrint);
	m_bStartPrint = ( StartDoc(m_hdc,m_pDocInfo) > 0 );
	
	return m_bStartPrint;
}

UT_Bool Win32Graphics::startPage(const char * szPageLabel, UT_uint32 pageNumber,
								 UT_Bool bPortrait, UT_uint32 iWidth, UT_uint32 iHeight)
{
	if (m_bStartPage)
		EndPage(m_hdc);

	m_bStartPage = UT_TRUE;
	return (StartPage(m_hdc) > 0);
}

UT_Bool Win32Graphics::endPrint(void)
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
void Win32Graphics::scroll(UT_sint32 dx, UT_sint32 dy)
{
	RECT rInvalid;
	
	ScrollWindowEx(m_hwnd, -dx, -dy, NULL, NULL, NULL, &rInvalid, 0);
//	HBRUSH hBrush = (HBRUSH) GetStockObject(WHITE_BRUSH);
//	FillRect(m_hdc, &rInvalid, hBrush);
}

void Win32Graphics::scroll(UT_sint32 x_dest, UT_sint32 y_dest,
						   UT_sint32 x_src, UT_sint32 y_src,
						   UT_sint32 width, UT_sint32 height)
{
	RECT r;
	r.left = x_src;
	r.top = y_src;
	r.right = x_src + width;
	r.bottom = y_src + height;
	ScrollWindowEx(m_hwnd, (x_dest - x_src), (y_dest - y_src),
				   &r, NULL, NULL, NULL, SW_ERASE);
}

void Win32Graphics::clearArea(UT_sint32 x, UT_sint32 y, UT_sint32 width, UT_sint32 height)
{
//	UT_ASSERT((x + width) < 800);
	
	HBRUSH hBrush = (HBRUSH) GetStockObject(WHITE_BRUSH);
	RECT r;
	r.left = x;
	r.top = y;
	r.right = r.left + width;
	r.bottom = r.top + height;
	
	FillRect(m_hdc, &r, hBrush);
}

void Win32Graphics::invertRect(const UT_Rect* pRect)
{
	RECT r;
	
	r.left = pRect->left;
	r.top = pRect->top;
	r.right = pRect->left + pRect->width;
	r.bottom = pRect->top + pRect->height;

	InvertRect(m_hdc, &r);
}

void Win32Graphics::setClipRect(const UT_Rect* pRect)
{
	int res;

	if (pRect)
	{
		// set the clip rectangle
		HRGN hrgn = CreateRectRgn(pRect->left,
								  pRect->top, 
								  pRect->left + pRect->width,
								  pRect->top + pRect->height);
		UT_ASSERT(hrgn);

		res = SelectClipRgn(m_hdc, hrgn);
	}
	else
	{
		// stop clipping
		res = SelectClipRgn(m_hdc, NULL);
	}

	UT_ASSERT(res != ERROR);
}
