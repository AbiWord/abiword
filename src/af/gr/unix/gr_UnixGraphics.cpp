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

#include "gr_UnixGraphics.h"

#include <gdk/gdkprivate.h>

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_misc.h"
#include "ut_string.h"

UNIXFont::UNIXFont(GdkFont* pFont)
{
	UT_ASSERT(pFont);
  
	m_hFont = pFont;
	m_strFontName = NULL;
}

GdkFont *UNIXFont::getHFONT()
{
	return m_hFont;
}

UNIXGraphics::UNIXGraphics(GdkWindow* win)
{
	m_pWin = win;
	m_pFont = NULL;
	m_pGC = gdk_gc_new(m_pWin);
	m_pXORGC = gdk_gc_new(m_pWin);

	m_pColormap = gdk_colormap_get_system();
	
	gdk_gc_set_function(m_pXORGC, GDK_XOR);

	GdkColor clrWhite;
	gdk_color_white(m_pColormap, &clrWhite);
	gdk_gc_set_foreground(m_pXORGC, &clrWhite);

	// I only want to set CAP_NOT_LAST, but the call takes all
	// arguments (and doesn't have a default value).  Set the
	// line attributes to not draw the last pixel.
	gdk_gc_set_line_attributes(m_pGC,   1,GDK_LINE_SOLID,GDK_CAP_NOT_LAST,GDK_JOIN_MITER);
	gdk_gc_set_line_attributes(m_pXORGC,1,GDK_LINE_SOLID,GDK_CAP_NOT_LAST,GDK_JOIN_MITER);

	// Set GraphicsExposes so that XCopyArea() causes an expose on
	// obscured regions rather than just tiling in the default background.
	gdk_gc_set_exposures(m_pGC,1);
	gdk_gc_set_exposures(m_pXORGC,1);
	
	memset(m_aCharWidths, 0, 256 * sizeof(int));
}

UT_Bool UNIXGraphics::queryProperties(DG_Graphics::Properties gp) const
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

void UNIXGraphics::drawChars(const UT_UCSChar* pChars, int iCharOffset,
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
	font_private = (GdkFontPrivate*) m_pFont->getHFONT();

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
}

void UNIXGraphics::setFont(DG_Font* pFont)
{
	UT_ASSERT(pFont);

	UNIXFont* pUFont = static_cast<UNIXFont*> (pFont);
  
	if (m_pFont)
    {
		XFontStruct* pCurFont = (XFontStruct *)((GdkFontPrivate*)m_pFont->getHFONT())->xfont;
		XFontStruct* pNewFont = (XFontStruct *)((GdkFontPrivate*)pUFont->getHFONT())->xfont;

		if (pCurFont->fid == pNewFont->fid)
		{
			return;
		}
    }
		     
	m_pFont = pUFont;
  
	gdk_gc_set_font(m_pGC, m_pFont->getHFONT());
	memset(m_aCharWidths, 0, 256 * sizeof(int));
}

UT_uint32 UNIXGraphics::getFontHeight()
{
	UT_ASSERT(m_pFont);
	UT_ASSERT(m_pGC);

	GdkFontPrivate* prFont = (GdkFontPrivate*) m_pFont->getHFONT();
	XFontStruct* pXFont = (XFontStruct *)prFont->xfont;

	return pXFont->ascent + pXFont->descent;
}

UT_uint32 UNIXGraphics::measureString(const UT_UCSChar* s, int iOffset,
									  int num,  unsigned short* pWidths)
{
	UT_ASSERT(m_pFont);
	UT_ASSERT(m_pGC);
	UT_ASSERT(s);

	int charWidth = 0;

	GdkFont* pFont = m_pFont->getHFONT();

	// TODO the following assignment looks suspicious...
	XFontStruct* pXFont = (XFontStruct *)(((GdkFontPrivate*)pFont)->xfont);
	
	for (int i = 0; i < num; i++)
    {
		UT_ASSERT(s[i + iOffset] < 256);
		char ch = s[i + iOffset];

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

UT_uint32 UNIXGraphics::getResolution() const
{
	return 75;
}

void UNIXGraphics::setColor(UT_RGBColor& clr)
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

DG_Font* UNIXGraphics::findFont(const char* pszFontFamily, 
								const char* pszFontStyle, 
								const char* /*pszFontVariant*/, 
								const char* pszFontWeight, 
								const char* /*pszFontStretch*/, 
								const char* pszFontSize)
{
	UNIXFont* pFont = NULL;
	char xFontName[2048];

	char szFamily[256];

	char szWeight[32];
	char szWeight2[32];
	char szWeight3[32]; 	// HACK:  since it's almost always the weight
	                        // we have trouble matching, we use this for a second shot.
	
	char szSlant[8];
	char cSpacing = 'p';	// proportional
	
	// family copied unless it maches a generic-font type or Windows, then it's cast 
	if (!UT_stricmp(pszFontFamily, "times new roman") ||	// win font
		!UT_stricmp(pszFontFamily, "serif") ||				// generic
		!UT_stricmp(pszFontFamily, ""))						// our default font
	{
		strcpy(szFamily, "times");
		cSpacing = 'p';	// proportional
	}
	else if(!UT_stricmp(pszFontFamily, "courier new") ||	// win font
			!UT_stricmp(pszFontFamily, "monospace"))		// generic
	{
		strcpy(szFamily, "courier");
		cSpacing = 'm';	// monospace
	}
	else if (!UT_stricmp(pszFontFamily, "arial") ||			// win font
			 !UT_stricmp(pszFontFamily, "sans-serif"))		// generic
	{	
		strcpy(szFamily, "helvetica");
		cSpacing = 'p';	// proportional		
	}
	else
	{
		// Font "foo" we don't know anything about
		strcpy(szFamily, pszFontFamily);
		cSpacing = '*';	// match all
	}

	// weight 
	if (!UT_stricmp(pszFontWeight, "bold"))
	{
		strcpy(szWeight, "bold");
	}
	else
	{
		// Note that we order them normal, then medium, because fonts
		// that support both consider medium a heavier weight than
		// normal.

		strcpy(szWeight, "normal");		// matches most fonts
		strcpy(szWeight2, "regular");	// matches some odd fonts
		strcpy(szWeight3, "medium");	// matches other fonts
	}

	UT_sint32 height = convertDimension(pszFontSize);

	if (!UT_stricmp(pszFontStyle, "italic"))
	{
		strcpy(szSlant, "i");
	}
	else if (!UT_stricmp(pszFontStyle, "oblique"))
	{
		strcpy(szSlant, "o");
	}
	else
	{
		strcpy(szSlant, "r");
	}

	// First match try
	GdkFont * pgFont;
	{
		sprintf(xFontName, "-*-%s-%s-%s-normal-*-%ld-0-75-75-%c-0-*-*", szFamily, szWeight, szSlant, height, cSpacing);
		if (m_pFont && m_pFont->m_strFontName)
			if (!UT_strnicmp(m_pFont->m_strFontName, xFontName, strlen(m_pFont->m_strFontName)))
				return m_pFont;

		pgFont = gdk_font_load(xFontName);

		if (!pgFont)
		{
			// load failed, try again with compromise attributes (this should be made much more general)
			sprintf(xFontName, "-*-%s-%s-%s-normal-*-%ld-0-75-75-%c-0-*-*", szFamily, szWeight2, szSlant, height, cSpacing);
			if (m_pFont && m_pFont->m_strFontName)
				if (!UT_strnicmp(m_pFont->m_strFontName, xFontName, strlen(m_pFont->m_strFontName)))
					return m_pFont;

			pgFont = gdk_font_load(xFontName);
			
			if (!pgFont)
			{
				// TRY AGAIN!
				// load failed, try again with compromise attributes (this should be made much more general)
				sprintf(xFontName, "-*-%s-%s-%s-normal-*-%ld-0-75-75-%c-0-*-*", szFamily, szWeight3, szSlant, height, cSpacing);
				if (m_pFont && m_pFont->m_strFontName)
					if (!UT_strnicmp(m_pFont->m_strFontName, xFontName, strlen(m_pFont->m_strFontName)))
						return m_pFont;

				pgFont = gdk_font_load(xFontName);

				// we failed the third lookup!  Render with an ugly fixed font.
				// Note that we use the misc foundry font, since some workstations
				// could have other fixed fonts (like a JIS Japanese foundry or
				// the Sony foundry)
				if (!pgFont)
					pgFont = gdk_font_load("-misc-fixed-*-*-*--*-*-*-*-*-*-*-*");
			}
		}
	}
	
	UT_ASSERT(pgFont);

	pFont = new UNIXFont(pgFont);
  
	pFont->m_strFontName = new char[strlen(xFontName) + 1];
	strcpy(pFont->m_strFontName, xFontName);
  
	return pFont;
}

UT_uint32 UNIXGraphics::getFontAscent()
{
	UT_ASSERT(m_pFont);
	UT_ASSERT(m_pGC);

	GdkFontPrivate* prFont = (GdkFontPrivate*) m_pFont->getHFONT();
	// TODO the following assignment looks suspicious...
	XFontStruct* pXFont = (XFontStruct *)prFont->xfont;

	return pXFont->ascent;
}

UT_uint32 UNIXGraphics::getFontDescent()
{
	UT_ASSERT(m_pFont);
	UT_ASSERT(m_pGC);

	GdkFontPrivate* prFont = (GdkFontPrivate*) m_pFont->getHFONT();
	XFontStruct* pXFont = (XFontStruct*) prFont->xfont;

	return pXFont->descent;
}

void UNIXGraphics::drawLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2,
							UT_sint32 y2)
{
	gdk_draw_line(m_pWin, m_pGC, x1, y1, x2, y2);
}

void UNIXGraphics::xorLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2,
			    UT_sint32 y2)
{
	gdk_draw_line(m_pWin, m_pXORGC, x1, y1, x2, y2);
}

void UNIXGraphics::invertRect(const UT_Rect* pRect)
{
	UT_ASSERT(pRect);
	
	gdk_draw_rectangle(m_pWin, m_pXORGC, 1, pRect->left, pRect->top,
					   pRect->width, pRect->height);
}

void UNIXGraphics::setClipRect(const UT_Rect* pRect)
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

void UNIXGraphics::fillRect(UT_RGBColor& c, UT_sint32 x, UT_sint32 y,
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

void UNIXGraphics::scroll(UT_sint32 dx, UT_sint32 dy)
{
	GdkWindowPrivate* pPWin = (GdkWindowPrivate*) m_pWin;

	UT_sint32 winWidth = pPWin->width;
	UT_sint32 winHeight = pPWin->height;
  
	if (dy > 0)
    {
		if (dy < winHeight)
		{
			gdk_window_copy_area(m_pWin, m_pGC, 0, 0,
								 m_pWin, 0, dy, winWidth, winHeight - dy);
//			gdk_window_clear_area(m_pWin, 0, winHeight - dy, winWidth, dy);
		}
		else
		{
//			gdk_window_clear_area(m_pWin, 0, 0, winWidth, winHeight);
		}
    }
	else if (dy < 0)
    {
		if (dy >= -winHeight)
		{
			gdk_window_copy_area(m_pWin, m_pGC, 0, -dy, m_pWin, 0, 0, winWidth,
								 winHeight + dy);

//			gdk_window_clear_area(m_pWin, 0, 0, winWidth, -dy);
		}
		else
		{
//			gdk_window_clear_area(m_pWin, 0, 0, winWidth, winHeight);
		}
    }

	if (dx > 0)
    {
		if (dx < winWidth)
		{
			gdk_window_copy_area(m_pWin, m_pGC, 0, 0,
								 m_pWin, dx, 0, winWidth - dx, winHeight);
//			gdk_window_clear_area(m_pWin, winWidth - dx, 0, dx, winHeight);
		}
		else
		{
//			gdk_window_clear_area(m_pWin, 0, 0, winWidth, winHeight);
		}
    }
	else if (dx < 0)
    {
		if (dx >= -winWidth)
		{
			gdk_window_copy_area(m_pWin, m_pGC, -dx, 0, m_pWin, 0, 0, winWidth + dx,
								 winHeight);

//			gdk_window_clear_area(m_pWin, 0, 0, -dx, winHeight);
		}
		else
		{
//			gdk_window_clear_area(m_pWin, 0, 0, winWidth, winHeight);
		}
    }
	
}

void UNIXGraphics::clearArea(UT_sint32 x, UT_sint32 y,
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

UT_Bool UNIXGraphics::startPrint(void)
{
	UT_ASSERT(0);
	return UT_FALSE;
}

UT_Bool UNIXGraphics::startPage(const char * /*szPageLabel*/, UT_uint32 /*pageNumber*/,
								UT_Bool /*bPortrait*/, UT_uint32 /*iWidth*/, UT_uint32 /*iHeight*/)
{
	UT_ASSERT(0);
	return UT_FALSE;
}

UT_Bool UNIXGraphics::endPrint(void)
{
	UT_ASSERT(0);
	return UT_FALSE;
}

