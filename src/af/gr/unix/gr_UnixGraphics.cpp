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

#include "ut_endian.h"
#include "xap_UnixApp.h"
#include "xap_UnixFontManager.h"
#include "xap_UnixFont.h"
#include "gr_UnixGraphics.h"
#include "gr_UnixImage.h"
#include "ut_sleep.h"
#include "xap_UnixFrame.h"

#ifdef HAVE_GNOME
#include <gnome.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include "gr_UnixGnomeImage.h"
#endif

#if 1
#include <gdk/gdkprivate.h>
static bool isFontUnicode(GdkFont *font)
{
	GdkFontPrivate *font_private = (GdkFontPrivate*) font;
	XFontStruct *xfont = (XFontStruct *) font_private->xfont;
	
	return ((xfont->min_byte1 == 0) || (xfont->max_byte1 == 0));
}
#endif

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_misc.h"
#include "ut_string.h"
#include "ut_dialogHelper.h"
#include "ut_wctomb.h"
#include "xap_EncodingManager.h"
#include "ut_OverstrikingChars.h"

XAP_UnixFontHandle *	GR_UnixGraphics::s_pFontGUI = NULL;
UT_uint32 				GR_UnixGraphics::s_iInstanceCount = 0;

GR_UnixGraphics::GR_UnixGraphics(GdkWindow * win, XAP_UnixFontManager * fontManager, XAP_App * app)
{
	m_pApp = app;
	m_pWin = win;
	m_pFontManager = fontManager;
	m_pFont = NULL;
	m_pSingleByteFont = NULL;
	m_pMultiByteFont = NULL;
	//m_pFontGUI = NULL;
	s_iInstanceCount++;
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
	s_iInstanceCount--;
	if(!s_iInstanceCount)
		DELETEP(s_pFontGUI);
}

bool GR_UnixGraphics::queryProperties(GR_Graphics::Properties gp) const
{
	switch (gp)
	{
	case DGP_SCREEN:
		return true;
	case DGP_PAPER:
		return false;
	default:
		UT_ASSERT(0);
		return false;
	}
}

/* let's cache this, since construction of UT_Wctomb is rather slow */
static UT_Wctomb* w = NULL;
static char text[MB_LEN_MAX];
static int text_length;
static bool fallback_used;

#define WCTOMB_DECLS \
	if (!w) {	\
	    w = new UT_Wctomb;	\
	} else	\
	    w->initialize();	
	    
#define CONVERT_TO_MBS(c)	\
    	if (c<=0xff) {	\
		/* this branch is to allow Lists to function */	\
		text[0] = (unsigned char)c;			\
		text_length = 1;				\
		fallback_used = 0;				\
	} else	{\
		fallback_used = 0;	\
		if (!w->wctomb(text,text_length,(wchar_t)c)) {	\
		    w->wctomb_or_fallback(text,text_length,(wchar_t)c);	\
		    fallback_used = 1;	\
		}	\
	}	


// HACK: I need more speed
void GR_UnixGraphics::drawChar(UT_UCSChar Char, UT_sint32 xoff, UT_sint32 yoff)
{
	UT_UCSChar Wide_char = remapGlyph(Char, false);
	GdkFont *font = XAP_EncodingManager::get_instance()->is_cjk_letter(Wide_char) ? m_pMultiByteFont : m_pSingleByteFont;

	if(XAP_EncodingManager::get_instance()->isUnicodeLocale())
	{
		/*  if the locale is unicode (i.e., utf-8) then we do not want
			to convert the UCS string to anything,
			gdk_draw_text can draw 16-bit string, if the font is
			a matrix; however, the byte ordering is interpreted as big-endian
		*/
		if(isFontUnicode(font))
		{
			LE2BE16((&Wide_char),(&Wide_char)) //declared in ut_endian.h
			gdk_draw_text(m_pWin,font,m_pGC,xoff,yoff+font->ascent,(gchar*)&Wide_char,2);
		}
		else
		{
			//non-unicode font, Wide char is guaranteed to be <= 0xff
			gchar gc = (gchar) Wide_char;
			gdk_draw_text(m_pWin,font,m_pGC,xoff,yoff+font->ascent,(gchar*)&gc,1);			//UT_DEBUGMSG(("drawChar: utf-8\n"));
		}
	}
	else
	{	
		WCTOMB_DECLS;
		CONVERT_TO_MBS(Wide_char);
		gdk_draw_text(m_pWin,font,m_pGC,xoff,yoff+font->ascent,text,text_length);
	}
}

void GR_UnixGraphics::drawChars(const UT_UCSChar* pChars, int iCharOffset,
							 int iLength, UT_sint32 xoff, UT_sint32 yoff)
{
	if (!m_pFontManager)
		return;
	UT_ASSERT(m_pFont);
	WCTOMB_DECLS;
	GdkFont *font;
	UT_sint32 x;

#ifdef BIDI_ENABLED	
	// to be able to handle overstriking characters, we have to remember the width
	// of the previous character printed
	// NB: overstriking characters are only supported under UTF-8, since on 8-bit locales
	// these are typically handled by combination glyphs

	static UT_sint32 prevWidth = 0;
	UT_sint32 curX;
	UT_sint32 curWidth;
#endif
	const UT_UCSChar *pC;
  	for(pC=pChars+iCharOffset, x=xoff; pC<pChars+iCharOffset+iLength; ++pC)
	{
		UT_UCSChar actual = remapGlyph(*pC,false);
		font=XAP_EncodingManager::get_instance()->is_cjk_letter(actual)? m_pMultiByteFont: m_pSingleByteFont;
		
		if(XAP_EncodingManager::get_instance()->isUnicodeLocale())
		{
			/*	if the locale is unicode (i.e., utf-8) then we do not want
				to convert the UCS string to anything,
				gdk_draw_text can draw 16-bit string, if the font is
				a matrix; however the string is interpreted as big-endian
			*/
			if(isFontUnicode(font))
			{
				//unicode font
				//UT_DEBUGMSG(("UnixGraphics::drawChars: utf-8\n"));
				UT_UCSChar beucs;
				LE2BE16((pC),(&beucs))  //declared in ut_endian.h
#ifdef BIDI_ENABLED
				switch(isOverstrikingChar(*pC))
				{
				case UT_NOT_OVERSTRIKING:
					curWidth = gdk_text_width(font, (gchar*)&beucs, 2);
					curX = x;
					break;
				case UT_OVERSTRIKING_RTL:
					curWidth = 0;
					curX = x;
					break;
				case UT_OVERSTRIKING_LTR:
					curWidth =  prevWidth;
					curX = x - prevWidth;
					break;
				}
			
				gdk_draw_text(m_pWin,font,m_pGC,curX,yoff+font->ascent,(gchar*)&beucs,2);
				x+=curWidth;
				prevWidth = curWidth;
#else
				gdk_draw_text(m_pWin,font,m_pGC,x,yoff+font->ascent,(gchar*)&beucs,2);
                x+=gdk_text_width(font, (gchar*)&beucs, 2);
#endif
			}
			else
			{
				// not a unicode font; actual is guaranteed to be <=0xff
				// (this happens typically when drawing the interface)
				gchar gc = (gchar) actual;
#ifdef BIDI_ENABLED				
				switch(isOverstrikingChar(*pC))
				{
				case UT_NOT_OVERSTRIKING:
					curWidth = gdk_text_width(font, (gchar*)&gc, 1);
					curX = x;
					break;
				case UT_OVERSTRIKING_RTL:
					curWidth = 0;
					curX = x;
					break;
				case UT_OVERSTRIKING_LTR:
					curWidth =  prevWidth;
					curX = x - prevWidth;
					break;
				}
				
				gdk_draw_text(m_pWin,font,m_pGC,curX,yoff+font->ascent,(gchar*)&gc,1);
				x += curWidth;
				prevWidth = curWidth;
#else
				gdk_draw_text(m_pWin,font,m_pGC,x,yoff+font->ascent,(gchar*)&gc,1);
                x += gdk_text_width(font, (gchar*)&gc, 1);
#endif
			}
		}
		else
		{
			CONVERT_TO_MBS(actual);
			gdk_draw_text(m_pWin,font,m_pGC,x,yoff+font->ascent,text,text_length);
			x+=gdk_text_width(font, text, text_length);
		}
	}
	flush();
}

void GR_UnixGraphics::setFont(GR_Font * pFont)
{
	XAP_UnixFontHandle * pUFont = static_cast<XAP_UnixFontHandle *> (pFont);
#if 1	
	// this is probably caching done on the wrong level
	// but it's currently faster to shortcut
	// than to call explodeGdkFonts
	// TODO: turn this off when our text runs get a bit smarter
	if(m_pFont && (pUFont->getUnixFont() == m_pFont->getUnixFont()) && 
	   (pUFont->getSize() == m_pFont->getSize()))
	  return;
#endif

	m_pFont = pUFont;
	m_pFont->explodeGdkFonts(m_pSingleByteFont,m_pMultiByteFont);
}

UT_uint32 GR_UnixGraphics::getFontHeight(GR_Font * fnt)
{
	return getFontAscent(fnt)+getFontDescent(fnt);
}

UT_uint32 GR_UnixGraphics::getFontHeight()
{
	if (!m_pFontManager)
		return 0;

	return getFontAscent()+getFontDescent();
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

	GdkFont * font;
	UT_UCSChar Wide_char = c;
		
	if(XAP_EncodingManager::get_instance()->isUnicodeLocale())
	{
		font = m_pSingleByteFont;
		
		if(isFontUnicode(font))
		{
			//this is a unicode font
			LE2BE16(&c,&Wide_char)
			return gdk_text_width(font, (gchar*) &Wide_char, 2);
		}
		else
		{
			//this is not a unicode font
			if(c > 0xff) //a non unicode font contains only 256 chars
				return 0;
			else
			{
				gchar gc = (gchar) c;
				return gdk_text_width(font, (gchar*)&gc, 1);
			}		
		}
	}
	else
	{
		WCTOMB_DECLS;
		CONVERT_TO_MBS(Wide_char);
		if (fallback_used)
			return 0;
		font = XAP_EncodingManager::get_instance()->is_cjk_letter(Wide_char) ? m_pMultiByteFont : m_pSingleByteFont;

		return gdk_text_width(font, text, text_length);
	}

}

#if 0
/*
    WARNING: this code doesn't support non-latin1 chars.
*/
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
		cChar = remapGlyph(s[i + iOffset], true);
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
	
	if (!s_pFontGUI)
	{
		// get the font resource
		//UT_DEBUGMSG(("GR_UnixGraphics::getGUIFont: getting default font\n"));
		XAP_UnixFont * font = (XAP_UnixFont *) m_pFontManager->getDefaultFont();
		UT_ASSERT(font);

		// bury it in a new font handle
		s_pFontGUI = new XAP_UnixFontHandle(font, 12); // Hardcoded GUI font size
		UT_ASSERT(s_pFontGUI);
	}

	return s_pFontGUI;
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
	if (!UT_strcmp(pszFontStyle, "normal") &&
		!UT_strcmp(pszFontWeight, "normal"))
	{
		s = XAP_UnixFont::STYLE_NORMAL;
	}
	else if (!UT_strcmp(pszFontStyle, "normal") &&
			 !UT_strcmp(pszFontWeight, "bold"))
	{
		s = XAP_UnixFont::STYLE_BOLD;
	}
	else if (!UT_strcmp(pszFontStyle, "italic") &&
			 !UT_strcmp(pszFontWeight, "normal"))
	{
		s = XAP_UnixFont::STYLE_ITALIC;
	}
	else if (!UT_strcmp(pszFontStyle, "italic") &&
			 !UT_strcmp(pszFontWeight, "bold"))
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

UT_uint32 GR_UnixGraphics::getFontAscent(GR_Font * fnt)
{
	UT_ASSERT(fnt);
	UT_ASSERT(m_pGC);

	XAP_UnixFontHandle * hndl = static_cast<XAP_UnixFontHandle *>(fnt);

	GdkFont* pFont = hndl->getGdkFont();
	GdkFont* pMatchFont=hndl->getMatchGdkFont();
	return MAX(pFont->ascent, pMatchFont->ascent);
}

UT_uint32 GR_UnixGraphics::getFontAscent()
{
	return getFontAscent(m_pFont);
}

UT_uint32 GR_UnixGraphics::getFontDescent(GR_Font * fnt)
{
	UT_ASSERT(fnt);
	UT_ASSERT(m_pGC);

	XAP_UnixFontHandle * hndl = static_cast<XAP_UnixFontHandle *>(fnt);

	GdkFont* pFont = hndl->getGdkFont();
	GdkFont* pMatchFont=hndl->getMatchGdkFont();
	return MAX(pFont->descent, pMatchFont->descent);
}

UT_uint32 GR_UnixGraphics::getFontDescent()
{
	return getFontDescent(m_pFont);
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
  // see bug #303 for what this is about
#if 1
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
//	if(pRect != NULL)
//		UT_ASSERT(m_pRect==NULL);
	m_pRect = pRect;
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
	UT_Rect exposeArea;
//
// Handle pending expose. The idea is that if there is an expose event that
// has not yet been painted, we expand the expose area to take account of the
// scroll we're about to make.
//
	while(isSpawnedRedraw())
	{
		UT_usleep(100); // 100 microseconds
	}
	setDontRedraw(true);
	while(isExposedAreaAccessed())
	{
		UT_usleep(10); // 10 microseconds
	}
//
// Don't let the repaint procede until after this adjustment
//
	setExposedAreaAccessed(true);
	exposeArea.top = getPendingRect()->top;
	exposeArea.left = getPendingRect()->left;
	exposeArea.width = getPendingRect()->width;
	exposeArea.height = getPendingRect()->height;
	xxx_UT_DEBUGMSG(("SEVIOR: before expand top %d left %d width %d height %d \n",exposeArea.top,exposeArea.left,exposeArea.width,exposeArea.height));
	if(dy < 0)
	{
		//
		// We're moving up so height is increased top is reduced.
		//
		exposeArea.height -= dy;
	}
	if(dy > 0)
	{
		exposeArea.top -= dy;
		exposeArea.height += dy;
	}
	if(dx < 0)
	{
		exposeArea.width -= dx;
	}
	if(dx > 0)
	{
		//
		// We're moving left so left is reduced
		//
		exposeArea.width += dx;
		exposeArea.left -= dx;
	}
	if(exposeArea.width > 100000 || exposeArea.height > 100000)
	{
		setPendingRect(0,0,0,0);
	}
	else
	{
		unionPendingRect(&exposeArea);
	}
	xxx_UT_DEBUGMSG(("SEVIOR: After expand top %d left %d width %d height %d \n",exposeArea.top,exposeArea.left,exposeArea.width,exposeArea.height));
	xxx_UT_DEBUGMSG(("SEVIOR: After Union top %d left %d width %d height %d \n",getPendingRect()->top,getPendingRect()->left,getPendingRect()->width,getPendingRect()->height));
//
// Merge into previous areas
//
	setDoMerge(true);
	setExposedAreaAccessed(false);

	xxx_UT_DEBUGMSG(("SEVIOR: Scrolling \n"));
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
	setDontRedraw(false);
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

bool GR_UnixGraphics::startPrint(void)
{
	UT_ASSERT(0);
	return false;
}

bool GR_UnixGraphics::startPage(const char * /*szPageLabel*/, UT_uint32 /*pageNumber*/,
								bool /*bPortrait*/, UT_uint32 /*iWidth*/, UT_uint32 /*iHeight*/)
{
	UT_ASSERT(0);
	return false;
}

bool GR_UnixGraphics::endPrint(void)
{
	UT_ASSERT(0);
	return false;
}

#if !defined(HAVE_GNOME)

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

#else

// gdk-pixbuf based routines

GR_Image* GR_UnixGraphics::createNewImage(const char* pszName, const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight, GR_Image::GRType iType)
{
   	GR_Image* pImg = NULL;
	
	pImg = new GR_UnixGnomeImage(pszName);
	pImg->convertFromBuffer(pBB, iDisplayWidth, iDisplayHeight);
   	return pImg;
}

// a bit of voodoo since i'm not entirely sure what the
// alpha_threshold param means. I know it takes values 0 <= threshold <= 255
// and that values < than the alpha threshold are painted as 0s
// this seems to work for me, so I'm happy - Dom
#define ABI_ALPHA_THRESHOLD 100

void GR_UnixGraphics::drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest)
{
	UT_ASSERT(pImg);

   	GR_UnixGnomeImage * pUnixImage = static_cast<GR_UnixGnomeImage *>(pImg);

	GdkPixbuf * image = pUnixImage->getData();

   	UT_sint32 iImageWidth = pUnixImage->getDisplayWidth();
   	UT_sint32 iImageHeight = pUnixImage->getDisplayHeight();
	
	if (gdk_pixbuf_get_has_alpha (image))
		gdk_pixbuf_render_to_drawable_alpha (image, m_pWin,
											 0, 0,
											 xDest, yDest,
											 iImageWidth, iImageHeight,
											 GDK_PIXBUF_ALPHA_BILEVEL, 
											 ABI_ALPHA_THRESHOLD,
											 GDK_RGB_DITHER_NORMAL,
											 0, 0); 
	else
		gdk_pixbuf_render_to_drawable (image, m_pWin, m_pGC,
									   0, 0,
									   xDest, yDest,
									   iImageWidth, iImageHeight,
									   GDK_RGB_DITHER_NORMAL,
									   0, 0);
}

#endif /* HAVE_GNOME */

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
		
	case GR_CURSOR_LEFTRIGHT:
		cursor_number = GDK_SB_H_DOUBLE_ARROW;
		break;

	case GR_CURSOR_UPDOWN:
		cursor_number = GDK_SB_V_DOUBLE_ARROW;
		break;

	case GR_CURSOR_EXCHANGE:
		cursor_number = GDK_EXCHANGE;
		break;

	case GR_CURSOR_GRAB:
		cursor_number = GDK_HAND1;
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
										 bool * pbTrueType)
{
	// describe in generic terms the named font.

	// Note: most of the unix font handling code is in abi/src/af/xap/unix
	// Note: rather than in the graphics class.  i'm not sure this matters,
	// Note: but it is just different....

	// TODO add code to map the given font name into one of the
	// TODO enums in GR_Font and set *pff and *pft.

	*pff = FF_Unknown;
	*pfp = FP_Unknown;
	*pbTrueType = true;
}
