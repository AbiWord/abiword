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

#include <stdlib.h>
#include <string.h>
#include <X11/Xlib.h>
#include <stdio.h>

#include "xap_UnixApp.h"
#include "xap_UnixFontManager.h"
#include "xap_UnixFont.h"
#include "gr_UnixGraphics.h"
#include "gr_UnixImage.h"

#include <gdk/gdkprivate.h>

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_misc.h"
#include "ut_string.h"
#include "ut_dialogHelper.h"

#define DELETEP(p)	do { if (p) delete p; } while (0)
#define FREEP(p)	do { if (p) free(p); } while (0)

/*****************************************************************/

GR_UnixFont::GR_UnixFont(AP_UnixFont * pFont, UT_uint32 size)
{
	UT_ASSERT(pFont);
  
	m_hFont = pFont;
	m_pointSize = size;
}

GR_UnixFont::~GR_UnixFont(void)
{
	;
	// don't remove where our font points, since it's
	// owned by the font manager (we don't keep our own copy)
}

AP_UnixFont * GR_UnixFont::getUnixFont(void)
{
	UT_ASSERT(m_hFont);
	return m_hFont;
}

GdkFont * GR_UnixFont::getGdkFont(void)
{
	UT_ASSERT(m_hFont);
	return m_hFont->getGdkFont(m_pointSize);
}

GR_UNIXGraphics::GR_UNIXGraphics(GdkWindow * win, AP_UnixFontManager * fontManager)
{
	m_pWin = win;
	m_pFontManager = fontManager;
	m_pFont = NULL;
	m_pFontGUI = NULL;
	m_pGC = gdk_gc_new(m_pWin);
	m_pXORGC = gdk_gc_new(m_pWin);

	m_pColormap = gdk_rgb_get_cmap(); // = gdk_colormap_get_system();

	gdk_gc_set_function(m_pXORGC, GDK_XOR);

	GdkColor clrWhite;
	gdk_color_white(m_pColormap, &clrWhite);
	gdk_gc_set_foreground(m_pXORGC, &clrWhite);

	// I only want to set CAP_NOT_LAST, but the call takes all
	// arguments (and doesn't have a default value).  Set the
	// line attributes to not draw the last pixel.

	// We force the line width to be zero because the CAP_NOT_LAST
	// stuff does not seem to work correctly when the width is set
	// to one.
	
	gdk_gc_set_line_attributes(m_pGC,   0,GDK_LINE_SOLID,GDK_CAP_NOT_LAST,GDK_JOIN_MITER);
	gdk_gc_set_line_attributes(m_pXORGC,0,GDK_LINE_SOLID,GDK_CAP_NOT_LAST,GDK_JOIN_MITER);

	// Set GraphicsExposes so that XCopyArea() causes an expose on
	// obscured regions rather than just tiling in the default background.
	gdk_gc_set_exposures(m_pGC,1);
	gdk_gc_set_exposures(m_pXORGC,1);
	
	memset(m_aCharWidths, 0, 256 * sizeof(int));

	m_cursor = GR_CURSOR_INVALID;
	setCursor(GR_CURSOR_DEFAULT);
}

GR_UNIXGraphics::~GR_UNIXGraphics()
{
	DELETEP(m_pFontGUI);
}

UT_Bool GR_UNIXGraphics::queryProperties(GR_Graphics::Properties gp) const
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

void GR_UNIXGraphics::drawChars(const UT_UCSChar* pChars, int iCharOffset,
							 int iLength, UT_sint32 xoff, UT_sint32 yoff)
{
	GdkWindowPrivate *drawable_private;
	GdkFontPrivate *font_private;
	GdkGCPrivate *gc_private;

	drawable_private = (GdkWindowPrivate*) m_pWin;
	if (drawable_private->destroyed)
	{
		return;
	}
	gc_private = (GdkGCPrivate*) m_pGC;
	font_private = (GdkFontPrivate*) m_pFont->getGdkFont();

	XFontStruct *xfont = (XFontStruct *) font_private->xfont;
	XSetFont(drawable_private->xdisplay, gc_private->xgc, xfont->fid);

	/*
	  TODO -  We need to seriously look for a way to avoid this.
	  Doing a memory allocation on every draw is painful.
	*/

	XChar2b *pNChars = new XChar2b[iLength];

	for (int i = 0; i < iLength; i++)
    {
		pNChars[i].byte1 = pChars[i + iCharOffset] & 0xffff0000;
		pNChars[i].byte2 = pChars[i + iCharOffset] & 0x0000ffff;
    }
  
	XDrawString16 (drawable_private->xdisplay, drawable_private->xwindow,
				   gc_private->xgc, xoff, yoff + xfont->ascent, pNChars,
				   iLength);

	delete pNChars;

#if 1
	flush();
#endif	
}

void GR_UNIXGraphics::setFont(GR_Font* pFont)
{
	UT_ASSERT(pFont);

	GR_UnixFont * pUFont = static_cast<GR_UnixFont*> (pFont);
  
	if (m_pFont)
    {
		XFontStruct* pCurFont = (XFontStruct *)((GdkFontPrivate*)m_pFont->getGdkFont())->xfont;
		XFontStruct* pNewFont = (XFontStruct *)((GdkFontPrivate*)pUFont->getGdkFont())->xfont;

		UT_ASSERT(pCurFont && pNewFont);
		
		if (!pCurFont->fid == pNewFont->fid)
		{
			return;
		}
    }
		     
	m_pFont = pUFont;
  
	gdk_gc_set_font(m_pGC, m_pFont->getGdkFont());
	memset(m_aCharWidths, 0, 256 * sizeof(int));
}

UT_uint32 GR_UNIXGraphics::getFontHeight()
{
	UT_ASSERT(m_pFont);
	UT_ASSERT(m_pGC);

	GdkFontPrivate* prFont = (GdkFontPrivate*) m_pFont->getGdkFont();
	XFontStruct* pXFont = (XFontStruct *)prFont->xfont;

	return pXFont->ascent + pXFont->descent;
}

UT_uint32 GR_UNIXGraphics::measureString(const UT_UCSChar* s, int iOffset,
									  int num,  unsigned short* pWidths)
{
	UT_ASSERT(m_pFont);
	UT_ASSERT(m_pGC);
	UT_ASSERT(s);

	int charWidth = 0;

	GdkFont* pFont = m_pFont->getGdkFont();

	// TODO the following assignment looks suspicious...
	XFontStruct* pXFont = (XFontStruct *)(((GdkFontPrivate*)pFont)->xfont);
	
	for (int i = 0; i < num; i++)
    {
		UT_ASSERT(s[i + iOffset] < 256);
		unsigned char ch = s[i + iOffset];

		int len = m_aCharWidths[ch];
      
		if (!len)
		{
			XChar2b c2b;
			XCharStruct overall;
			int des, dir, asc;
			
			c2b.byte1 = s[i + iOffset] & 0xffff0000;
			c2b.byte2 = s[i + iOffset] & 0x0000ffff;

			XTextExtents16(pXFont, & c2b, 1, &dir, &asc, &des, &overall);

			len = overall.width;
			m_aCharWidths[ch] = len;
		}
      
		charWidth += len;
		pWidths[i] = len;
    }

  
	return charWidth;
}

UT_uint32 GR_UNIXGraphics::getResolution() const
{
	return 75;
}

void GR_UNIXGraphics::setColor(UT_RGBColor& clr)
{
	UT_ASSERT(m_pGC);
	GdkColor c;

	c.red = clr.m_red << 8;
	c.blue = clr.m_blu << 8;
	c.green = clr.m_grn << 8;
  
	gint ret = gdk_color_alloc(m_pColormap, &c);

	UT_ASSERT(ret == TRUE);
	
	GdkGCPrivate *pgc = (GdkGCPrivate*)m_pXORGC;
	GdkGCPrivate *pogc = (GdkGCPrivate*)m_pGC;

	gdk_gc_set_foreground(m_pGC, &c);

	/* Set up the XOR gc */
	XGCValues gcv;

	XGetGCValues(pogc->xdisplay, pogc->xgc, GCForeground, &gcv);

	gcv.foreground = c.pixel;
	
	gcv.function = GXxor;

	XChangeGC(pgc->xdisplay, pgc->xgc, GCForeground | GCFunction, &gcv);
}

GR_Font* GR_UNIXGraphics::getGUIFont(void)
{
	if (!m_pFontGUI)
	{
		// lazily grab this (once)
		AP_UnixFont * font = m_pFontManager->getDefaultFont();
		UT_ASSERT(font);
		
		m_pFontGUI = new GR_UnixFont(font, 10);
		UT_ASSERT(m_pFontGUI);
	}

	return m_pFontGUI;
}

GR_Font* GR_UNIXGraphics::findFont(const char* pszFontFamily, 
								const char* pszFontStyle, 
								const char* /*pszFontVariant*/, 
								const char* pszFontWeight, 
								const char* /*pszFontStretch*/, 
								const char* pszFontSize)
{
	UT_ASSERT(pszFontFamily);
	UT_ASSERT(pszFontStyle);
	UT_ASSERT(pszFontWeight);
	UT_ASSERT(pszFontSize);
	
	// convert styles to AP_UnixFont:: formats
	AP_UnixFont::style s;

	// this is kind of sloppy
	if (!UT_stricmp(pszFontStyle, "normal") &&
		!UT_stricmp(pszFontWeight, "normal"))
	{
		s = AP_UnixFont::STYLE_NORMAL;
	}
	else if (!UT_stricmp(pszFontStyle, "normal") &&
			 !UT_stricmp(pszFontWeight, "bold"))
	{
		s = AP_UnixFont::STYLE_BOLD;
	}
	else if (!UT_stricmp(pszFontStyle, "italic") &&
			 !UT_stricmp(pszFontWeight, "normal"))
	{
		s = AP_UnixFont::STYLE_ITALIC;
	}
	else if (!UT_stricmp(pszFontStyle, "italic") &&
			 !UT_stricmp(pszFontWeight, "bold"))
	{
		s = AP_UnixFont::STYLE_BOLD_ITALIC;
	}
	else
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}

	// Request the appropriate AP_UnixFont::, and bury it in an
	// instance of a GR_UnixFont:: with the correct size.
	AP_UnixFont * unixfont = m_pFontManager->getFont(pszFontFamily, s);	
	AP_UnixFont * item = NULL;
	if (unixfont)
	{
		// make a copy
		item = new AP_UnixFont(*unixfont);
	}
	else
	{
		// Oops!  We don't have that font here.  substitute something
		// we know we have (get smarter about this later)
		AP_UnixFont * fallback = m_pFontManager->getFont("Times New Roman", s);
		if (!fallback)
		{
			char message[1024];
			g_snprintf(message, 1024,
					   "AbiWord could not find its default fallback font\n"
					   "[%s], even though it was listed in a\n"
					   "valid font directory file ('fonts.dir') in a valid\n"
					   "directory in the font path.\n"
					   "\n"
					   "AbiWord cannot continue without this font.", pszFontFamily);
			messageBoxOK(message);
			exit(-1);
		}
		item = new AP_UnixFont(*fallback);
	}
	
	GR_UnixFont * pFont = new GR_UnixFont(item, convertDimension(pszFontSize));

	UT_ASSERT(pFont);

	return pFont;
}

UT_uint32 GR_UNIXGraphics::getFontAscent()
{
	UT_ASSERT(m_pFont);
	UT_ASSERT(m_pGC);

	GdkFontPrivate* prFont = (GdkFontPrivate*) m_pFont->getGdkFont();
	// TODO the following assignment looks suspicious...
	XFontStruct* pXFont = (XFontStruct *)prFont->xfont;

	return pXFont->ascent;
}

UT_uint32 GR_UNIXGraphics::getFontDescent()
{
	UT_ASSERT(m_pFont);
	UT_ASSERT(m_pGC);

	GdkFontPrivate* prFont = (GdkFontPrivate*) m_pFont->getGdkFont();
	XFontStruct* pXFont = (XFontStruct*) prFont->xfont;

	return pXFont->descent;
}

void GR_UNIXGraphics::drawLine(UT_sint32 x1, UT_sint32 y1,
							   UT_sint32 x2, UT_sint32 y2)
{
	// TODO set the line width according to m_iLineWidth
	gdk_draw_line(m_pWin, m_pGC, x1, y1, x2, y2);
}

void GR_UNIXGraphics::setLineWidth(UT_sint32 iLineWidth)
{
	m_iLineWidth = iLineWidth;
}

void GR_UNIXGraphics::xorLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2,
			    UT_sint32 y2)
{
	gdk_draw_line(m_pWin, m_pXORGC, x1, y1, x2, y2);
}

void GR_UNIXGraphics::polyLine(UT_Point * pts, UT_uint32 nPoints)
{
#if 0
	GdkPoint * points = (GdkPoint *)calloc(nPoints, sizeof(GdkPoint));
	UT_ASSERT(points);

	for (UT_uint32 i = 0; i < nPoints; i++)
	{
		points[i].x = pts[i].x;
		// It seems that Windows draws each pixel along the the Y axis
		// one pixel beyond where GDK draws it (even though both coordinate
		// systems start at 0,0 (?)).  Subtracting one clears this up so
		// that the poly line is in the correct place relative to where
		// the rest of GR_UNIXGraphics:: does things (drawing text, clearing
		// areas, etc.).
		points[i].y = pts[i].y - 1;	
	}

	gdk_draw_lines(m_pWin, m_pGC, points, nPoints);

	FREEP(points);
#else
	for (UT_uint32 k=1; k<nPoints; k++)
		drawLine(pts[k-1].x,pts[k-1].y, pts[k].x,pts[k].y);
#endif
}

void GR_UNIXGraphics::invertRect(const UT_Rect* pRect)
{
	UT_ASSERT(pRect);
	
	gdk_draw_rectangle(m_pWin, m_pXORGC, 1, pRect->left, pRect->top,
					   pRect->width, pRect->height);
}

void GR_UNIXGraphics::setClipRect(const UT_Rect* pRect)
{
	if (pRect)
	{
		GdkRectangle r;

		r.x = pRect->left;
		r.y = pRect->top;
		r.width = pRect->width;
		r.height = pRect->height;

		gdk_gc_set_clip_rectangle(m_pGC, &r);
		gdk_gc_set_clip_rectangle(m_pXORGC, &r);
	}
	else
	{
		gdk_gc_set_clip_rectangle(m_pGC, NULL);
		gdk_gc_set_clip_rectangle(m_pXORGC, NULL);
	}
}

void GR_UNIXGraphics::fillRect(UT_RGBColor& c, UT_Rect &r)
{
	fillRect(c,r.left,r.top,r.width,r.height);
}

void GR_UNIXGraphics::fillRect(UT_RGBColor& c, UT_sint32 x, UT_sint32 y,
							UT_sint32 w, UT_sint32 h)
{
	// save away the current color, and restore it after we fill the rect
	GdkGCValues gcValues;
	GdkColor oColor;

	memset(&oColor, 0, sizeof(GdkColor));
  
	gdk_gc_get_values(m_pGC, &gcValues);

	oColor.pixel = gcValues.foreground.pixel;

	// get the new color
	GdkColor nColor;

	nColor.red = c.m_red << 8;
	nColor.blue = c.m_blu << 8;
	nColor.green = c.m_grn << 8;

	gdk_color_alloc(m_pColormap, &nColor);
  
	gdk_gc_set_foreground(m_pGC, &nColor);

	gdk_draw_rectangle(m_pWin, m_pGC, 1, x, y, w, h);

	gdk_gc_set_foreground(m_pGC, &oColor);
}

void GR_UNIXGraphics::scroll(UT_sint32 dx, UT_sint32 dy)
{
	GdkWindowPrivate* pPWin = (GdkWindowPrivate*) m_pWin;

	UT_sint32 winWidth = pPWin->width;
	UT_sint32 winHeight = pPWin->height;
  
	if (dy > 0)
    {
		if (dy < winHeight)
			gdk_window_copy_area(m_pWin, m_pGC, 0, 0,
								 m_pWin, 0, dy, winWidth, winHeight - dy);
    }
	else if (dy < 0)
    {
		if (dy >= -winHeight)
			gdk_window_copy_area(m_pWin, m_pGC, 0, -dy,
								 m_pWin, 0, 0, winWidth, winHeight + dy);
    }

	if (dx > 0)
    {
		if (dx < winWidth)
			gdk_window_copy_area(m_pWin, m_pGC, 0, 0,
								 m_pWin, dx, 0, winWidth - dx, winHeight);
    }
	else if (dx < 0)
    {
		if (dx >= -winWidth)
			gdk_window_copy_area(m_pWin, m_pGC, -dx, 0,
								 m_pWin, 0, 0, winWidth + dx, winHeight);
    }
	
}

void GR_UNIXGraphics::scroll(UT_sint32 x_dest, UT_sint32 y_dest,
						  UT_sint32 x_src, UT_sint32 y_src,
						  UT_sint32 width, UT_sint32 height)
{
	gdk_window_copy_area(m_pWin, m_pGC, x_dest, y_dest,
						 m_pWin, x_src, y_src, width, height);
}

void GR_UNIXGraphics::clearArea(UT_sint32 x, UT_sint32 y,
							 UT_sint32 width, UT_sint32 height)
{
//	UT_DEBUGMSG(("ClearArea: %d %d %d %d\n", x, y, width, height));
	if (width > 0)
	{
#define TURBOSLOW	0

#if TURBOSLOW
		gdk_flush();
		usleep(TURBOSLOW);
		
		UT_RGBColor clr(255,0,0);
		fillRect(clr, x, y, width, height);
		gdk_flush();
		usleep(TURBOSLOW);
#endif
		
		UT_RGBColor clrWhite(255,255,255);
		fillRect(clrWhite, x, y, width, height);

#if TURBOSLOW		
		gdk_flush();
		usleep(TURBOSLOW);
#endif		
	}
}

UT_Bool GR_UNIXGraphics::startPrint(void)
{
	UT_ASSERT(0);
	return UT_FALSE;
}

UT_Bool GR_UNIXGraphics::startPage(const char * /*szPageLabel*/, UT_uint32 /*pageNumber*/,
								UT_Bool /*bPortrait*/, UT_uint32 /*iWidth*/, UT_uint32 /*iHeight*/)
{
	UT_ASSERT(0);
	return UT_FALSE;
}

UT_Bool GR_UNIXGraphics::endPrint(void)
{
	UT_ASSERT(0);
	return UT_FALSE;
}

GR_Image* GR_UNIXGraphics::createNewImage(const char* pszName, const UT_ByteBuf* pBBPNG, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight)
{
	GR_UnixImage* pImg = new GR_UnixImage(NULL, pszName);

	pImg->convertFromPNG(pBBPNG, iDisplayWidth, iDisplayHeight);

	return pImg;
}

void GR_UNIXGraphics::drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest)
{
	UT_ASSERT(pImg);
	
	GR_UnixImage * pUnixImage = static_cast<GR_UnixImage *>(pImg);

	Fatmap * image = pUnixImage->getData();

	UT_sint32 iImageWidth = pUnixImage->getDisplayWidth();
	UT_sint32 iImageHeight = pUnixImage->getDisplayHeight();

	gdk_draw_rgb_image(m_pWin,
					   m_pGC,
					   xDest,
					   yDest,
					   iImageWidth,
					   iImageHeight,
					   GDK_RGB_DITHER_NORMAL,
					   image->data,
					   image->width * 3); // This parameter is the total bytes across one row,
	                                      // which is pixels * 3 (we use 3 bytes per pixel).
}

void GR_UNIXGraphics::flush(void)
{
	gdk_flush();
}

void GR_UNIXGraphics::setCursor(GR_Graphics::Cursor c)
{
	if (m_cursor == c)
		return;
	
	m_cursor = c;
	
	enum GdkCursorType cursor_number;
	
	switch (c)
	{
	default:
	case GR_CURSOR_DEFAULT:
		cursor_number = GDK_TOP_LEFT_ARROW;
		break;
		
	case GR_CURSOR_IBEAM:
		cursor_number = GDK_XTERM;
		break;
	}

	GdkCursor * cursor = gdk_cursor_new(cursor_number);
	gdk_window_set_cursor(m_pWin, cursor);
	gdk_cursor_destroy(cursor);
}

GR_Graphics::Cursor GR_UNIXGraphics::getCursor(void) const
{
	return m_cursor;
}

