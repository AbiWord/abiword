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
#include "gr_Win32Image.h"

#include "ut_debugmsg.h"
#include "ut_assert.h"

#define DELETEP(p)	do { if (p) delete p; } while (0)
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


void GR_Win32Graphics::_constructorCommonCode(HDC hdc)
{
	UT_ASSERT(hdc);

	m_hdc = hdc;
	m_hwnd = 0;
	m_iLineWidth = 0;	// default to a hairline
	m_bPrint = UT_FALSE;
	m_bStartPrint = UT_FALSE;
	m_bStartPage = UT_FALSE;
	m_pFont = NULL;
	m_pFontGUI = NULL;
	memset(m_aABCs, 0, 256*sizeof(ABC));
	memset(m_aCharWidths, 0, 256*sizeof(int));
	m_defaultCharWidth = 0;

	m_cursor = GR_CURSOR_INVALID;
	setCursor(GR_CURSOR_DEFAULT);
}

GR_Win32Graphics::GR_Win32Graphics(HDC hdc, HWND hwnd)
{
	_constructorCommonCode(hdc);
	m_hwnd = hwnd;
}

GR_Win32Graphics::GR_Win32Graphics(HDC hdc, const DOCINFO * pDocInfo)
{
	_constructorCommonCode(hdc);
	m_bPrint = UT_TRUE;
	m_pDocInfo = pDocInfo;
}

GR_Win32Graphics::~GR_Win32Graphics()
{
	DELETEP(m_pFontGUI);
}

UT_Bool GR_Win32Graphics::queryProperties(GR_Graphics::Properties gp) const
{
	switch (gp)
	{
	case DGP_SCREEN:
		return !m_bPrint;
	case DGP_PAPER:
		return m_bPrint;
	default:
		UT_ASSERT(0);
		return UT_FALSE;
	}
}

GR_Win32Font::GR_Win32Font(HFONT hFont)
{
	m_hFont = hFont;
}

GR_Font* GR_Win32Graphics::getGUIFont(void)
{
	if (!m_pFontGUI)
	{
		// lazily grab this (once)
		HFONT f = (HFONT) GetStockObject(DEFAULT_GUI_FONT);
		m_pFontGUI = new GR_Win32Font(f);
		UT_ASSERT(m_pFontGUI);
	}

	return m_pFontGUI;
}

GR_Font* GR_Win32Graphics::findFont(
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

	// TODO -- remove this block entirely, since oblique is no longer a valid style
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
	return new GR_Win32Font(hFont);
}

void GR_Win32Graphics::drawChars(const UT_UCSChar* pChars, int iCharOffset, int iLength, UT_sint32 xoff, UT_sint32 yoff)
{
	UT_ASSERT(pChars);

	SelectObject(m_hdc, m_pFont->getHFONT());
	SetTextAlign(m_hdc, TA_LEFT | TA_TOP);
	SetBkMode(m_hdc, TRANSPARENT);		// TODO: remember and reset?

	ExtTextOutW(m_hdc, xoff, yoff, 0, NULL, pChars + iCharOffset, iLength, NULL);
	//TextOutW(m_hdc, xoff, yoff, pChars + iCharOffset, iLength);
}

void GR_Win32Graphics::setFont(GR_Font* pFont)
{
	UT_ASSERT(pFont);	// TODO should we allow pFont == NULL?

	if (m_pFont == (static_cast<GR_Win32Font*> (pFont)))
	{
		return;
	};
	
	m_pFont = static_cast<GR_Win32Font*> (pFont);
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

	// also get the width of the default (slug) character

	TEXTMETRIC tm;

	GetTextMetrics(m_hdc, &tm);

	UINT d = tm.tmDefaultChar;
	UT_ASSERT(d<256);	// if this is always true, we could save a lookup

	if (__isWinNT())
		bSuccess = GetCharWidth32(m_hdc, d, d, &m_defaultCharWidth);
	else
		bSuccess = GetCharWidth(m_hdc, d, d, &m_defaultCharWidth);

	if (!bSuccess)
	{
		DWORD err = GetLastError();

		UT_ASSERT(err);
	}
}

HFONT GR_Win32Font::getHFONT()
{
	return m_hFont;
}

UT_uint32 GR_Win32Graphics::getFontHeight()
{
	UT_ASSERT(m_pFont);
	UT_ASSERT(m_hdc);

	TEXTMETRIC tm;

	GetTextMetrics(m_hdc, &tm);

	return tm.tmHeight;
}

UT_uint32 GR_Win32Graphics::getFontAscent()
{
	UT_ASSERT(m_pFont);
	UT_ASSERT(m_hdc);

	TEXTMETRIC tm;

	GetTextMetrics(m_hdc, &tm);
	UT_ASSERT(tm.tmAscent > 0);

	return tm.tmAscent;
}

UT_uint32 GR_Win32Graphics::getFontDescent()
{
	UT_ASSERT(m_pFont);
	UT_ASSERT(m_hdc);

	TEXTMETRIC tm;

	GetTextMetrics(m_hdc, &tm);

	return tm.tmDescent;
}

#if 0
UT_uint32 GR_Win32Graphics::measureWidth(UT_UCSChar* s, int num)
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

UT_uint32 GR_Win32Graphics::measureString(const UT_UCSChar* s, int iOffset, int num, unsigned short* pWidths)
{
	UT_ASSERT(m_pFont);
	UT_ASSERT(s);

	int iCharWidth = 0;
	int iWidth;
	for (int i=0; i<num; i++)
	{
//		UT_ASSERT(s[i+iOffset] < 256);	
		
		// TODO we currently cannot deal with Unicode properly

		if (s[i+iOffset] < 256)
			iWidth = m_aCharWidths[s[i + iOffset]];
		else 
			iWidth = m_defaultCharWidth;

		iCharWidth += iWidth;
		pWidths[i] = iWidth;
	}

	return iCharWidth;
}

UT_uint32 GR_Win32Graphics::getResolution() const
{
	int result = GetDeviceCaps(m_hdc, LOGPIXELSY); // NOTE: assumes square pixels

	return result;
}

void GR_Win32Graphics::setColor(UT_RGBColor& clr)
{
	SetTextColor(m_hdc, RGB(clr.m_red, clr.m_grn, clr.m_blu));
	m_clr = clr;
}

void GR_Win32Graphics::drawLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2, UT_sint32 y2)
{
	HPEN hPen = CreatePen(PS_SOLID, m_iLineWidth, RGB(m_clr.m_red, m_clr.m_grn, m_clr.m_blu));
	HPEN hOldPen = (HPEN) SelectObject(m_hdc, hPen);

	MoveToEx(m_hdc, x1, y1, NULL);
	LineTo(m_hdc, x2, y2);

	(void) SelectObject(m_hdc, hOldPen);
	DeleteObject(hPen);
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
	HPEN hPen = CreatePen(PS_SOLID, 1, RGB(m_clr.m_red, m_clr.m_grn, m_clr.m_blu));
	int iROP = SetROP2(m_hdc, R2_XORPEN);
	HPEN hOldPen = (HPEN) SelectObject(m_hdc, hPen);

	MoveToEx(m_hdc, x1, y1, NULL);
	LineTo(m_hdc, x2, y2);

	(void) SelectObject(m_hdc, hOldPen);
	DeleteObject(hPen);
	(void) SetROP2(m_hdc, iROP);
}

void GR_Win32Graphics::polyLine(UT_Point * pts, UT_uint32 nPoints)
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

void GR_Win32Graphics::fillRect(UT_RGBColor& c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h)
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

void GR_Win32Graphics::fillRect(UT_RGBColor& c, UT_Rect &r)
{
	fillRect(c,r.left,r.top,r.width,r.height);
}

UT_Bool GR_Win32Graphics::startPrint(void)
{
	UT_ASSERT(m_bPrint);
	UT_ASSERT(!m_bStartPrint);
	m_bStartPrint = ( StartDoc(m_hdc,m_pDocInfo) > 0 );
	
	return m_bStartPrint;
}

UT_Bool GR_Win32Graphics::startPage(const char * szPageLabel, UT_uint32 pageNumber,
								 UT_Bool bPortrait, UT_uint32 iWidth, UT_uint32 iHeight)
{
	if (m_bStartPage)
		EndPage(m_hdc);

	m_bStartPage = UT_TRUE;
	return (StartPage(m_hdc) > 0);
}

UT_Bool GR_Win32Graphics::endPrint(void)
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
	RECT rInvalid;
	
	ScrollWindowEx(m_hwnd, -dx, -dy, NULL, NULL, NULL, &rInvalid, 0);
//	HBRUSH hBrush = (HBRUSH) GetStockObject(WHITE_BRUSH);
//	FillRect(m_hdc, &rInvalid, hBrush);
}

void GR_Win32Graphics::scroll(UT_sint32 x_dest, UT_sint32 y_dest,
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

void GR_Win32Graphics::clearArea(UT_sint32 x, UT_sint32 y, UT_sint32 width, UT_sint32 height)
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

void GR_Win32Graphics::invertRect(const UT_Rect* pRect)
{
	RECT r;
	
	r.left = pRect->left;
	r.top = pRect->top;
	r.right = pRect->left + pRect->width;
	r.bottom = pRect->top + pRect->height;

	InvertRect(m_hdc, &r);
}

void GR_Win32Graphics::setClipRect(const UT_Rect* pRect)
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

GR_Image* GR_Win32Graphics::createNewImage(const char* pszName, const UT_ByteBuf* pBBPNG, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight)
{
	GR_Win32Image* pImg = new GR_Win32Image(NULL, pszName);

	pImg->convertFromPNG(pBBPNG, iDisplayWidth, iDisplayHeight);

	return pImg;
}

void GR_Win32Graphics::drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest)
{
	UT_ASSERT(pImg);
	
	GR_Win32Image* pWin32Img = static_cast<GR_Win32Image*>(pImg);

	BITMAPINFO* pDIB = pWin32Img->getDIB();

	UT_uint32 iSizeOfColorData = pDIB->bmiHeader.biClrUsed * sizeof(RGBQUAD);
	UT_Byte* pBits = ((unsigned char*) pDIB) + pDIB->bmiHeader.biSize + iSizeOfColorData;
	
	int iRes = StretchDIBits(m_hdc,
							 xDest,
							 yDest,
							 pImg->getDisplayWidth(),
							 pImg->getDisplayHeight(),
							 0,
							 0,
							 pDIB->bmiHeader.biWidth,
							 pDIB->bmiHeader.biHeight,
							 pBits,
							 pDIB,
							 DIB_RGB_COLORS,
							 SRCCOPY
							 );
	
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

	LPCTSTR cursor_name;
	
	switch (m_cursor)
	{
	default:
		UT_ASSERT(UT_NOT_IMPLEMENTED);
		/*FALLTHRU*/
	case GR_CURSOR_DEFAULT:
		cursor_name = IDC_ARROW;		// top-left arrow
		break;
		
	case GR_CURSOR_IBEAM:
		cursor_name = IDC_IBEAM;
		break;

	case GR_CURSOR_RIGHTARROW:
		cursor_name = IDC_ARROW;		// TODO change this
		break;
		
	case GR_CURSOR_IMAGE:
		cursor_name = IDC_SIZEALL;
		break;
		
	case GR_CURSOR_IMAGESIZE_NW:
	case GR_CURSOR_IMAGESIZE_SE:
		cursor_name = IDC_SIZENWSE;
		break;
		
	case GR_CURSOR_IMAGESIZE_N:
	case GR_CURSOR_IMAGESIZE_S:
		cursor_name = IDC_SIZENS;
		break;
		
	case GR_CURSOR_IMAGESIZE_NE:
	case GR_CURSOR_IMAGESIZE_SW:
		cursor_name = IDC_SIZENESW;
		break;
		
	case GR_CURSOR_IMAGESIZE_E:
	case GR_CURSOR_IMAGESIZE_W:
		cursor_name = IDC_SIZEWE;
		break;
	}

	HCURSOR hCursor = LoadCursor(NULL,cursor_name);
	SetCursor(hCursor);
}

