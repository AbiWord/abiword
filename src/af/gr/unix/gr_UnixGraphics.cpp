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

GR_UnixGraphics::GR_UnixGraphics(GdkWindow * win, XAP_UnixFontManager * fontManager, XAP_App * app)
{
	m_pApp = app;
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
	
	m_cs = GR_Graphics::GR_COLORSPACE_COLOR;
	m_cursor = GR_CURSOR_INVALID;
	setCursor(GR_CURSOR_DEFAULT);
}

GR_UnixGraphics::~GR_UnixGraphics()
{
	DELETEP(m_pFontGUI);
}

UT_Bool GR_UnixGraphics::queryProperties(GR_Graphics::Properties gp) const
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

// HACK: I need more speed
void GR_UnixGraphics::drawChar(UT_UCSChar Char, UT_sint32 xoff, UT_sint32 yoff)
{
	GdkWChar Wide_char = remapGlyph(Char, UT_FALSE);
	
	GdkFont *font = m_pFont->getGdkFont();
	gdk_draw_text_wc (m_pWin, font, m_pGC,
				   xoff, yoff + font->ascent, &Wide_char, 1);
}

void GR_UnixGraphics::drawChars(const UT_UCSChar* pChars, int iCharOffset,
							 int iLength, UT_sint32 xoff, UT_sint32 yoff)
{
	if (!m_pFontManager)
		return;

	UT_ASSERT(m_pFont);

	GdkFont *font = m_pFont->getGdkFont();
	// Blargh... GDK wants strings in 32 bits, we use 16 internally
	GdkWChar *pNChars, utb[150];  // arbitrary biggish size for utb
	if ((unsigned)iLength < (sizeof(utb) / sizeof(utb[0])))
	{
		// avoid new/delete overhead for most cases via ubiquitous temp buf
		pNChars = utb;
	}
	else
	{
		pNChars = new GdkWChar[iLength];
	}
	for (int i = 0; i < iLength; i++)
	{
		pNChars[i] = remapGlyph(pChars[i + iCharOffset], UT_FALSE);
	}
	// Use "wide-char" function
	gdk_draw_text_wc(m_pWin, font, m_pGC, xoff, yoff + font->ascent, pNChars, iLength);
	if (pNChars != utb) delete pNChars;

	flush();
}

void GR_UnixGraphics::setFont(GR_Font * pFont)
{
	if (!m_pFontManager)
		return;

	UT_ASSERT(pFont);

	XAP_UnixFontHandle * pUFont = static_cast<XAP_UnixFontHandle *> (pFont);

	// want to only call this once, if possible, on a new font
	GdkFont * newGdkFont = pUFont->getGdkFont();
	UT_ASSERT(newGdkFont);

#if 0
	if (m_pFont && gdk_font_equal (newGdkFont, m_pFont->getGdkFont()))
	{
		return;
	}
#endif

	m_pFont = pUFont;
  
	gdk_gc_set_font(m_pGC, newGdkFont);
}

UT_uint32 GR_UnixGraphics::getFontHeight()
{
	if (!m_pFontManager)
		return 0;

	UT_ASSERT(m_pFont);

	GdkFont* font = m_pFont->getGdkFont();
	return font->ascent + font->descent;
}

UT_uint32 GR_UnixGraphics::measureUnRemappedChar(const UT_UCSChar c)
{
	// measureString() could be defined in terms of measureUnRemappedChar()
	// but its not (for presumed performance reasons).  Also, a difference
	// is that measureString() uses remapping to get past zero-width
	// character cells.
	if (!m_pFontManager)
		return 0;

	UT_ASSERT(m_pFont);
	UT_ASSERT(m_pGC);

	int width;
	GdkWChar cChar = c;

	GdkFont* pFont = m_pFont->getGdkFont();
	width = gdk_char_width_wc (pFont, cChar);
	return width;
}
#if 0
UT_uint32 GR_UnixGraphics::measureString(const UT_UCSChar* s, int iOffset,
										 int num,  unsigned short* pWidths)
{
	// on X11, we do not use the aCharWidths[] or the GR_CharWidths
	// cacheing mechanism -- because, XTextExtents16() provides a
	// local copy (in the client library) of all that information
	// unlike XQueryText...() which cause a round trip to the XServer.
	// and i'm tired of having semi-bogus local caches which are more
	// trouble (and cost more cycles) to maintain than they save.... -- jeff
	
	if (!m_pFontManager)
		return 0;

	UT_ASSERT(m_pFont);
	UT_ASSERT(m_pGC);
	UT_ASSERT(s);

	int charWidth = 0, width;
	GdkWChar cChar;

	GdkFont* pFont = m_pFont->getGdkFont();
	
	for (int i = 0; i < num; i++)
    {
		cChar = remapGlyph(s[i + iOffset], UT_TRUE);
		width = gdk_char_width_wc (pFont, cChar);
		charWidth += width;
		if (pWidths)
			pWidths[i] = width;
    }
  
	return charWidth;
}
#endif
UT_uint32 GR_UnixGraphics::_getResolution(void) const
{
	// this is hard-coded at 100 for X now, since 75 (which
	// most X servers return when queried for a resolution)
	// makes for tiny fonts on modern resolutions.

	return 100;
}

void GR_UnixGraphics::setColor(UT_RGBColor& clr)
{
	UT_ASSERT(m_pGC);
	GdkColor c;

	c.red = clr.m_red << 8;
	c.blue = clr.m_blu << 8;
	c.green = clr.m_grn << 8;

	_setColor(c);
}

void GR_UnixGraphics::_setColor(GdkColor & c)
{
	gint ret = gdk_color_alloc(m_pColormap, &c);

	UT_ASSERT(ret == TRUE);

	gdk_gc_set_foreground(m_pGC, &c);

	/* Set up the XOR gc */
	gdk_gc_set_foreground(m_pXORGC, &c);
	gdk_gc_set_function(m_pXORGC, GDK_XOR);
}

GR_Font * GR_UnixGraphics::getGUIFont(void)
{
	if (!m_pFontManager)
		return NULL;
	
	if (!m_pFontGUI)
	{
		// get the font resource
		XAP_UnixFont * font = (XAP_UnixFont *) m_pFontManager->getDefaultFont();
		UT_ASSERT(font);

		// bury it in a new font handle
		m_pFontGUI = new XAP_UnixFontHandle(font, 12); // Hardcoded GUI font size
		UT_ASSERT(m_pFontGUI);
	}

	return m_pFontGUI;
}

GR_Font * GR_UnixGraphics::findFont(const char* pszFontFamily, 
									const char* pszFontStyle, 
									const char* /*pszFontVariant*/, 
									const char* pszFontWeight, 
									const char* /*pszFontStretch*/, 
									const char* pszFontSize)
{
	if (!m_pFontManager)
		return NULL;

	UT_ASSERT(pszFontFamily);
	UT_ASSERT(pszFontStyle);
	UT_ASSERT(pszFontWeight);
	UT_ASSERT(pszFontSize);
	
	// convert styles to XAP_UnixFont:: formats
	XAP_UnixFont::style s = XAP_UnixFont::STYLE_NORMAL;

	// this is kind of sloppy
	if (!UT_stricmp(pszFontStyle, "normal") &&
		!UT_stricmp(pszFontWeight, "normal"))
	{
		s = XAP_UnixFont::STYLE_NORMAL;
	}
	else if (!UT_stricmp(pszFontStyle, "normal") &&
			 !UT_stricmp(pszFontWeight, "bold"))
	{
		s = XAP_UnixFont::STYLE_BOLD;
	}
	else if (!UT_stricmp(pszFontStyle, "italic") &&
			 !UT_stricmp(pszFontWeight, "normal"))
	{
		s = XAP_UnixFont::STYLE_ITALIC;
	}
	else if (!UT_stricmp(pszFontStyle, "italic") &&
			 !UT_stricmp(pszFontWeight, "bold"))
	{
		s = XAP_UnixFont::STYLE_BOLD_ITALIC;
	}
	else
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}

	// Request the appropriate XAP_UnixFont
	XAP_UnixFont * unixfont = m_pFontManager->getFont(pszFontFamily, s);	

	if (!unixfont)
	{
		// Oops!  We don't have that font here.  substitute something
		// we know we have (get smarter about this later)
		unixfont = m_pFontManager->getFont("Times New Roman", s);
		if (!unixfont)
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
			exit(1);
		}
		
	}

	// bury the pointer to our Unix font in a XAP_UnixFontHandle with the correct size.
	XAP_UnixFontHandle * pFont = new XAP_UnixFontHandle(unixfont, convertDimension(pszFontSize));
	UT_ASSERT(pFont);

	return pFont;
}

UT_uint32 GR_UnixGraphics::getFontAscent()
{
	if (!m_pFontManager)
		return 0;

	UT_ASSERT(m_pFont);
	UT_ASSERT(m_pGC);

	GdkFont* pFont = m_pFont->getGdkFont();

	return pFont->ascent;
}

UT_uint32 GR_UnixGraphics::getFontDescent()
{
	if (!m_pFontManager)
		return 0;
  
	UT_ASSERT(m_pFont);
	UT_ASSERT(m_pGC);

	GdkFont* pFont = m_pFont->getGdkFont();

	return pFont->descent;
}

void GR_UnixGraphics::drawLine(UT_sint32 x1, UT_sint32 y1,
							   UT_sint32 x2, UT_sint32 y2)
{
	// TODO set the line width according to m_iLineWidth
	gdk_draw_line(m_pWin, m_pGC, x1, y1, x2, y2);
}

void GR_UnixGraphics::setLineWidth(UT_sint32 iLineWidth)
{
	m_iLineWidth = iLineWidth;

	// Get the current values of the line attributes

	GdkGCValues cur_line_att;
        gdk_gc_get_values(m_pGC, &cur_line_att);
        GdkLineStyle cur_line_style = cur_line_att.line_style;
        GdkCapStyle   cur_cap_style = cur_line_att.cap_style;
        GdkJoinStyle  cur_join_style = cur_line_att.join_style;

	// Set the new line width

        gdk_gc_set_line_attributes(m_pGC,m_iLineWidth,cur_line_style,cur_cap_style,cur_join_style);

}

void GR_UnixGraphics::xorLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2,
			    UT_sint32 y2)
{
	gdk_draw_line(m_pWin, m_pXORGC, x1, y1, x2, y2);
}

void GR_UnixGraphics::polyLine(UT_Point * pts, UT_uint32 nPoints)
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
		// the rest of GR_UnixGraphics:: does things (drawing text, clearing
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

void GR_UnixGraphics::invertRect(const UT_Rect* pRect)
{
	UT_ASSERT(pRect);
	
	gdk_draw_rectangle(m_pWin, m_pXORGC, 1, pRect->left, pRect->top,
					   pRect->width, pRect->height);
}

void GR_UnixGraphics::setClipRect(const UT_Rect* pRect)
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

void GR_UnixGraphics::fillRect(UT_RGBColor& c, UT_Rect &r)
{
	fillRect(c,r.left,r.top,r.width,r.height);
}

void GR_UnixGraphics::fillRect(UT_RGBColor& c, UT_sint32 x, UT_sint32 y,
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

void GR_UnixGraphics::scroll(UT_sint32 dx, UT_sint32 dy)
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

void GR_UnixGraphics::scroll(UT_sint32 x_dest, UT_sint32 y_dest,
						  UT_sint32 x_src, UT_sint32 y_src,
						  UT_sint32 width, UT_sint32 height)
{
	gdk_window_copy_area(m_pWin, m_pGC, x_dest, y_dest,
						 m_pWin, x_src, y_src, width, height);
}

void GR_UnixGraphics::clearArea(UT_sint32 x, UT_sint32 y,
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

UT_Bool GR_UnixGraphics::startPrint(void)
{
	UT_ASSERT(0);
	return UT_FALSE;
}

UT_Bool GR_UnixGraphics::startPage(const char * /*szPageLabel*/, UT_uint32 /*pageNumber*/,
								UT_Bool /*bPortrait*/, UT_uint32 /*iWidth*/, UT_uint32 /*iHeight*/)
{
	UT_ASSERT(0);
	return UT_FALSE;
}

UT_Bool GR_UnixGraphics::endPrint(void)
{
	UT_ASSERT(0);
	return UT_FALSE;
}

GR_Image* GR_UnixGraphics::createNewImage(const char* pszName, const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight, GR_Image::GRType iType)
{
   	GR_Image* pImg = NULL;
   	if (iType == GR_Image::GRT_Raster)
   		pImg = new GR_UnixImage(pszName);
   	else
	   	pImg = new GR_VectorImage(pszName);
	     
	pImg->convertFromBuffer(pBB, iDisplayWidth, iDisplayHeight);
   	return pImg;
}

void GR_UnixGraphics::drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest)
{
	UT_ASSERT(pImg);

   	if (pImg->getType() != GR_Image::GRT_Raster) {
	   	pImg->render(this, xDest, yDest);
	   	return;
	}
	   
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

void GR_UnixGraphics::flush(void)
{
	gdk_flush();
}

void GR_UnixGraphics::setColorSpace(GR_Graphics::ColorSpace /* c */)
{
	// we only use ONE color space here now (GdkRGB's space)
	// and we don't let people change that on us.
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

GR_Graphics::ColorSpace GR_UnixGraphics::getColorSpace(void) const
{
	return m_cs;
}

void GR_UnixGraphics::setCursor(GR_Graphics::Cursor c)
{
	if (m_cursor == c)
		return;
	
	m_cursor = c;
	
	GdkCursorType cursor_number;
	
	switch (c)
	{
	default:
		UT_ASSERT(UT_NOT_IMPLEMENTED);
		/*FALLTHRU*/
	case GR_CURSOR_DEFAULT:
		cursor_number = GDK_TOP_LEFT_ARROW;
		break;
		
	case GR_CURSOR_IBEAM:
		cursor_number = GDK_XTERM;
		break;

	case GR_CURSOR_RIGHTARROW:
		cursor_number = GDK_ARROW;
		break;
		
	case GR_CURSOR_IMAGE:
		cursor_number = GDK_FLEUR;
		break;
		
	case GR_CURSOR_IMAGESIZE_NW:
		cursor_number = GDK_TOP_LEFT_CORNER;
		break;
		
	case GR_CURSOR_IMAGESIZE_N:
		cursor_number = GDK_TOP_SIDE;
		break;
		
	case GR_CURSOR_IMAGESIZE_NE:
		cursor_number = GDK_TOP_RIGHT_CORNER;
		break;
		
	case GR_CURSOR_IMAGESIZE_E:
		cursor_number = GDK_RIGHT_SIDE;
		break;
		
	case GR_CURSOR_IMAGESIZE_SE:
		cursor_number = GDK_BOTTOM_RIGHT_CORNER;
		break;
		
	case GR_CURSOR_IMAGESIZE_S:
		cursor_number = GDK_BOTTOM_SIDE;
		break;
		
	case GR_CURSOR_IMAGESIZE_SW:
		cursor_number = GDK_BOTTOM_LEFT_CORNER;
		break;
		
	case GR_CURSOR_IMAGESIZE_W:
		cursor_number = GDK_LEFT_SIDE;
		break;
	}

	GdkCursor * cursor = gdk_cursor_new(cursor_number);
	gdk_window_set_cursor(m_pWin, cursor);
	gdk_cursor_destroy(cursor);
}

GR_Graphics::Cursor GR_UnixGraphics::getCursor(void) const
{
	return m_cursor;
}

void GR_UnixGraphics::setColor3D(GR_Color3D c)
{
	UT_ASSERT(c < COUNT_3D_COLORS);
	_setColor(m_3dColors[c]);
}

void GR_UnixGraphics::init3dColors(GtkStyle * pStyle)
{
	m_3dColors[CLR3D_Foreground] = pStyle->fg[GTK_STATE_NORMAL];
	m_3dColors[CLR3D_Background] = pStyle->bg[GTK_STATE_NORMAL];
	m_3dColors[CLR3D_BevelUp] = pStyle->light[GTK_STATE_NORMAL];
	m_3dColors[CLR3D_BevelDown] = pStyle->dark[GTK_STATE_NORMAL];
	m_3dColors[CLR3D_Highlight] = pStyle->bg[GTK_STATE_PRELIGHT];
}

void GR_UnixGraphics::fillRect(GR_Color3D c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h)
{
	UT_ASSERT(c < COUNT_3D_COLORS);
	gdk_gc_set_foreground(m_pGC, &m_3dColors[c]);
	gdk_draw_rectangle(m_pWin, m_pGC, 1, x, y, w, h);
}

void GR_UnixGraphics::fillRect(GR_Color3D c, UT_Rect &r)
{
	UT_ASSERT(c < COUNT_3D_COLORS);
	fillRect(c,r.left,r.top,r.width,r.height);
}

void GR_UnixGraphics::polygon(UT_RGBColor& c,UT_Point *pts,UT_uint32 nPoints)
{
	// save away the current color, and restore it after we draw the polygon
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

	GdkPoint* points = new GdkPoint[nPoints];
    UT_ASSERT(points);

    for (UT_uint32 i = 0;i < nPoints;i++){
        points[i].x = pts[i].x;
        points[i].y = pts[i].y;
    }
	gdk_draw_polygon(m_pWin, m_pGC, 1, points, nPoints);
	delete[] points;

	gdk_gc_set_foreground(m_pGC, &oColor);
}

//////////////////////////////////////////////////////////////////
// This is a static method in the GR_Font base class implemented
// in platform code.
//////////////////////////////////////////////////////////////////

void GR_Font::s_getGenericFontProperties(const char * /*szFontName*/,
										 FontFamilyEnum * pff,
										 FontPitchEnum * pfp,
										 UT_Bool * pbTrueType)
{
	// describe in generic terms the named font.

	// Note: most of the unix font handling code is in abi/src/af/xap/unix
	// Note: rather than in the graphics class.  i'm not sure this matters,
	// Note: but it is just different....

	// TODO add code to map the given font name into one of the
	// TODO enums in GR_Font and set *pff and *pft.

	*pff = FF_Unknown;
	*pfp = FP_Unknown;
	*pbTrueType = UT_TRUE;
}
