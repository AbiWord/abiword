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
#include "ut_string.h"
#include "ut_Win32OS.h"

/*****************************************************************/

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

	m_cs = GR_Graphics::GR_COLORSPACE_COLOR;
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

GR_Font* GR_Win32Graphics::findFont(const char* pszFontFamily, 
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
	lf.lfCharSet = DEFAULT_CHARSET;

	UT_sint32 iHeight = convertDimension(pszFontSize);
	lf.lfHeight = -(iHeight);

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

	lf.lfOutPrecision = OUT_TT_ONLY_PRECIS;		// Choose only True Type fonts.
	lf.lfQuality = PROOF_QUALITY;

	HFONT hFont = CreateFontIndirect(&lf);
	UT_ASSERT(hFont);
	if (!hFont)
		return 0;

	return new GR_Win32Font(hFont);
}

void GR_Win32Graphics::drawChar(UT_UCSChar Char, UT_sint32 xoff, UT_sint32 yoff)
{
	SelectObject(m_hdc, m_pFont->getHFONT());
	SetTextAlign(m_hdc, TA_LEFT | TA_TOP);
	SetBkMode(m_hdc, TRANSPARENT);		// TODO: remember and reset?

	ExtTextOutW(m_hdc, xoff, yoff, 0, NULL, &Char, 1, NULL);
}

void GR_Win32Graphics::drawChars(const UT_UCSChar* pChars,
								 int iCharOffset, int iLength,
								 UT_sint32 xoff, UT_sint32 yoff)
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
		return;
	
	m_pFont = static_cast<GR_Win32Font*> (pFont);
	m_pFont->selectFontIntoDC(m_hdc);
}

UT_uint32 GR_Win32Graphics::getFontHeight()
{
	UT_ASSERT(m_pFont);
	return m_pFont->getFontHeight();
}

UT_uint32 GR_Win32Graphics::getFontAscent()
{
	UT_ASSERT(m_pFont);
	return m_pFont->getAscent();
}

UT_uint32 GR_Win32Graphics::getFontDescent()
{
	UT_ASSERT(m_pFont);
	return m_pFont->getDescent();
}

UT_uint32 GR_Win32Graphics::measureString(const UT_UCSChar* s, int iOffset, int num, unsigned short* pWidths)
{
	UT_ASSERT(m_pFont);
	return m_pFont->measureString(s,iOffset,num,pWidths);
}

UT_uint32 GR_Win32Graphics::_getResolution(void) const
{
	int result = GetDeviceCaps(m_hdc, LOGPIXELSY); // NOTE: assumes square pixels

	return result;
}

void GR_Win32Graphics::setColor(UT_RGBColor& clr)
{
	_setColor(RGB(clr.m_red, clr.m_grn, clr.m_blu));
}

void GR_Win32Graphics::_setColor(DWORD dwColor)
{
	m_clrCurrent = dwColor;
	SetTextColor(m_hdc, m_clrCurrent);
}

void GR_Win32Graphics::drawLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2, UT_sint32 y2)
{
	HPEN hPen = CreatePen(PS_SOLID, m_iLineWidth, m_clrCurrent);
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
	HPEN hPen = CreatePen(PS_SOLID, 1, m_clrCurrent);
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
	HPEN hPen = CreatePen(PS_SOLID, 1, m_clrCurrent);
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

GR_Image* GR_Win32Graphics::createNewImage(const char* pszName, const UT_ByteBuf* pBBPNG,
										   UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight)
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
							 xDest, yDest,
							 pImg->getDisplayWidth(), pImg->getDisplayHeight(),
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
	r.left = x;
	r.top = y;
	r.right = r.left + w;
	r.bottom = r.top + h;

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
										 UT_Bool * pbTrueType)
{
	// describe in generic terms the named font.
	
	// we borrow some code from GR_Win32Graphics::findFont()

	LOGFONT lf;
	memset(&lf, 0, sizeof(lf));
	TEXTMETRIC tm;
	memset(&tm,0,sizeof(tm));

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

UT_uint32 GR_Win32Font::measureString(const UT_UCSChar* s, int iOffset, int num, unsigned short* pWidths)
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
		
		iWidth = m_cw.getWidth(s[i+iOffset]);
		if (!iWidth)
		{
			m_cw.setCharWidthsOfRange(m_oldHDC,s[i+iOffset],s[i+iOffset]);
			iWidth = m_cw.getWidth(s[i+iOffset]);
			if (!iWidth)
				iWidth = m_defaultCharWidth;
		}
		
		iCharWidth += iWidth;
		if (pWidths)
			pWidths[i] = iWidth;
	}

	return iCharWidth;
}

GR_Win32Font::GR_Win32Font(HFONT hFont)
{
	m_hFont = hFont;
	m_oldHDC = 0;
}

void GR_Win32Font::selectFontIntoDC(HDC hdc)
{
	SelectObject(hdc, m_hFont);
	if (hdc == m_oldHDC)
		return;

	// invalidate cached info when we change hdc's.
	// this is probably unnecessary except when
	// sharing a font with screen and printer.
	// TODO consider changing our invalidate test
	// TODO to not invalidate if old and new are
	// TODO both on screen.
	
	m_oldHDC = hdc;

	m_cw.zeroWidths();


	// preload US-ASCII subset of Latin-1 as these
	// are rather frequently used.  we can demand
	// load the other half of latin-1 as well as
	// the rest of unicode.
	//m_cw.setCharWidthsOfRange(hdc,0,255);
	
	m_cw.setCharWidthsOfRange(hdc,0x20,0x7f);
	
	GetTextMetrics(hdc, &m_tm);

	UINT d = m_tm.tmDefaultChar;
	m_cw.setCharWidthsOfRange(hdc,d,d);
	m_defaultCharWidth = m_cw.getWidth(d);
}
