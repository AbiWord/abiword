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
#include "xap_Strings.h"

#ifdef HAVE_GNOME
#include "gr_UnixGnomeImage.h"
#endif

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_misc.h"
#include "ut_string.h"
#include "xap_UnixDialogHelper.h"
#include "ut_wctomb.h"
#include "xap_EncodingManager.h"
#include "ut_OverstrikingChars.h"
#ifdef USE_XFT
#include <X11/Xlib.h>
#endif

const char* GR_Graphics::findNearestFont(const char* pszFontFamily,
										 const char* pszFontStyle,
										 const char* pszFontVariant,
										 const char* pszFontWeight,
										 const char* pszFontStretch,
										 const char* pszFontSize)
{
#ifdef USE_XFT	
	XAP_UnixFont* pUnixFont = XAP_UnixFontManager::findNearestFont(pszFontFamily, pszFontStyle, pszFontVariant, pszFontWeight,
																   pszFontStretch, pszFontSize);
	return pUnixFont->getName();
#else
	return NULL;
#endif
}

#if (1 && (!defined(WITH_PANGO) || !defined(USE_XFT)))
#include <gdk/gdkprivate.h>
static bool isFontUnicode(GdkFont *font)
{
	if(!font)
	{
		UT_DEBUGMSG(("gr_UnixGraphics: isFontUnicode: font is NULL !!!\n"));
		return false;
	}

	GdkFontPrivate *font_private = (GdkFontPrivate*) font;
	XFontStruct *xfont = (XFontStruct *) font_private->xfont;

	return ((xfont->min_byte1 == 0) || (xfont->max_byte1 == 0));
}
#endif


//
// Below this size we use GDK fonts. Above it we use metric info.
//
#define MAX_ABI_GDK_FONT_SIZE 200
#define FALLBACK_FONT_SIZE 12

XAP_UnixFontHandle *	GR_UnixGraphics::s_pFontGUI = NULL;
UT_uint32 				GR_UnixGraphics::s_iInstanceCount = 0;

#ifndef WITH_PANGO
GR_UnixGraphics::GR_UnixGraphics(GdkWindow * win, XAP_UnixFontManager * fontManager, XAP_App * app)
#else
GR_UnixGraphics::GR_UnixGraphics(GdkWindow * win, XAP_App * app)
#endif
#ifdef USE_XFT
	: m_bLayoutUnits(false)
#endif
{
	m_pApp = app;
	m_pWin = win;
#ifndef WITH_PANGO
	m_pFontManager = fontManager;
	m_pFont = NULL;
#endif
	m_pSingleByteFont = NULL;
	m_pMultiByteFont = NULL;
	//m_pFontGUI = NULL;
	s_iInstanceCount++;
	m_pGC = gdk_gc_new(m_pWin);
	m_pXORGC = gdk_gc_new(m_pWin);

	m_pColormap = gdk_rgb_get_cmap(); // = gdk_colormap_get_system();
#ifdef USE_XFT
	m_pVisual = GDK_VISUAL_XVISUAL(gdk_window_get_visual(win));
	m_Drawable = GDK_WINDOW_XWINDOW(win);
	m_Colormap = GDK_COLORMAP_XCOLORMAP(m_pColormap);
	m_pXftFont = NULL;

	m_pXftDraw = XftDrawCreate(GDK_DISPLAY(), m_Drawable, m_pVisual, m_Colormap);
#endif

	gdk_gc_set_function(m_pXORGC, GDK_XOR);

 	GdkColor clrWhite;
	gdk_color_white(m_pColormap, &clrWhite);
	gdk_gc_set_foreground(m_pXORGC, &clrWhite);

 	GdkColor clrBlack;
	gdk_color_black(m_pColormap, &clrBlack);
	gdk_gc_set_foreground(m_pGC, &clrBlack);

#ifdef USE_XFT
	m_XftColor.color.red = clrBlack.red;
	m_XftColor.color.green = clrBlack.green;
	m_XftColor.color.blue = clrBlack.blue;
	m_XftColor.color.alpha = 0xffff;
	m_XftColor.pixel = clrBlack.pixel;
#endif

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

#if (!defined(WITH_PANGO) || !defined(USE_XFT))

	m_pFallBackFontHandle = new XAP_UnixFontHandle(m_pFontManager->getDefaultFont(), FALLBACK_FONT_SIZE);
#endif
}

GR_UnixGraphics::~GR_UnixGraphics()
{
	s_iInstanceCount--;
	if(!s_iInstanceCount)
		DELETEP(s_pFontGUI);

#ifdef USE_XFT
	/* WARNING: Don't use XftDrawDestroy.  XftDrawDestroy will also destroy the drawable */
	if (m_pXftDraw)
		free(m_pXftDraw);

#ifndef WITH_PANGO
	delete m_pFallBackFontHandle;
#endif
#endif
}

bool GR_UnixGraphics::queryProperties(GR_Graphics::Properties gp) const
{
	switch (gp)
	{
	case DGP_SCREEN:
	case DGP_OPAQUEOVERLAY:
		return true;
	case DGP_PAPER:
		return false;
	default:
		UT_ASSERT(0);
		return false;
	}
}

static GdkCapStyle mapCapStyle ( GR_Graphics::CapStyle in )
{
  switch ( in )
    {
    case GR_Graphics::CAP_ROUND :
      return GDK_CAP_ROUND ;
    case GR_Graphics::CAP_PROJECTING :
      return GDK_CAP_PROJECTING ;
    case GR_Graphics::CAP_BUTT :
    default:
      return GDK_CAP_BUTT ;
    }
}

static GdkLineStyle mapLineStyle ( GR_Graphics::LineStyle in )
{
  switch ( in )
    {
    case GR_Graphics::LINE_ON_OFF_DASH :
      return GDK_LINE_ON_OFF_DASH ;
    case GR_Graphics::LINE_DOUBLE_DASH :
      return GDK_LINE_DOUBLE_DASH ;
    case GR_Graphics::LINE_SOLID :
    default:
      return GDK_LINE_SOLID ;
    }
}

static GdkJoinStyle mapJoinStyle ( GR_Graphics::JoinStyle in )
{
  switch ( in )
    {
    case GR_Graphics::JOIN_ROUND :
      return GDK_JOIN_ROUND ;
    case GR_Graphics::JOIN_BEVEL :
      return GDK_JOIN_BEVEL ;
    case GR_Graphics::JOIN_MITER :
    default:
      return GDK_JOIN_MITER ;
    }
}

void GR_UnixGraphics::setLineProperties ( double inWidthPixels, 
					  GR_Graphics::JoinStyle inJoinStyle,
					  GR_Graphics::CapStyle inCapStyle,
					  GR_Graphics::LineStyle inLineStyle )
{
  gdk_gc_set_line_attributes ( m_pGC, (gint)inWidthPixels,
			       mapLineStyle ( inLineStyle ),
			       mapCapStyle ( inCapStyle ),
			       mapJoinStyle ( inJoinStyle ) ) ;
}

#if (!defined(WITH_PANGO) || !defined(USE_XFT))
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
		if (!w->wctomb(text,text_length,c)) {	\
		    w->wctomb_or_fallback(text,text_length,c);	\
		    fallback_used = 1;	\
		}	\
	}
#endif

#ifndef WITH_PANGO
void GR_UnixGraphics::drawGlyph(UT_uint32 Char, UT_sint32 xoff, UT_sint32 yoff)
{
#ifdef USE_XFT
	XftDrawGlyphs(m_pXftDraw, &m_XftColor, m_pXftFont, xoff, yoff + m_pXftFont->ascent, &Char, 1);
#else
	UT_UCSChar Wide_char = remapGlyph(Char, false);
	if(Wide_char == 0x200B || Wide_char == 0xFEFF) //zero width spaces
		return;

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
#endif
}

void GR_UnixGraphics::drawChars(const UT_UCSChar* pChars, int iCharOffset,
							 int iLength, UT_sint32 xoff, UT_sint32 yoff)
{
#ifdef USE_XFT
	XftDrawString32(m_pXftDraw, &m_XftColor, m_pXftFont, xoff, yoff + m_pXftFont->ascent,
					const_cast<XftChar32*> (pChars + iCharOffset), iLength);
#else
	if (!m_pFontManager)
		return;
	UT_ASSERT(m_pFont);
	WCTOMB_DECLS;
	GdkFont *font;
	UT_sint32 x;

	static bool bFontSizeWarning = true;

	// to be able to handle overstriking characters, we have to remember the width
	// of the previous character printed
	// NB: overstriking characters are only supported under UTF-8, since on 8-bit locales
	// these are typically handled by combination glyphs

	static UT_sint32 prevWidth = 0;
	UT_sint32 curX;
	UT_sint32 curWidth;

	const UT_UCSChar *pC;
  	for(pC=pChars+iCharOffset, x=xoff; pC<pChars+iCharOffset+iLength; ++pC)
	{
		UT_UCSChar actual = remapGlyph(*pC,false);
		if(actual == 0x200B || actual == 0xFEFF) //zero width spaces
			continue;

		font=XAP_EncodingManager::get_instance()->is_cjk_letter(actual)? m_pMultiByteFont: m_pSingleByteFont;

		if(!font)
		{
			// what now? this happens for instance when you set font size to 72
			// and zoom to 200; obviously gtk cannot create font that big
			// we do get the width right though, since we use the afm file for
			// such big sizes, so we should just use the default font here,
			// this will look really weird, but it is better then not drawing anything
			// and much better than crashing
			if(bFontSizeWarning)
			{
				XAP_App * pApp = XAP_App::getApp();
				UT_ASSERT(pApp);
				const XML_Char * msg = pApp->getStringSet()->getValue(XAP_STRING_ID_MSG_UnixFontSizeWarning);
				UT_ASSERT(msg);
				bFontSizeWarning = false;
				messageBoxOK(msg);
			}

			UT_DEBUGMSG(("gr_UnixGraphics::drawChars: no font to draw with, using default !!!\n"));
			UT_sint32 iSize = m_pFallBackFontHandle ? m_pFallBackFontHandle->getSize() : 0;
			UT_sint32 iMySize = FALLBACK_FONT_SIZE * getZoomPercentage() / 100;

			if(iSize != iMySize)
			{
				delete m_pFallBackFontHandle;
				m_pFallBackFontHandle = new XAP_UnixFontHandle(m_pFontManager->getDefaultFont(),iMySize);
			}

			setFont(m_pFallBackFontHandle);
			font=XAP_EncodingManager::get_instance()->is_cjk_letter(actual)? m_pMultiByteFont: m_pSingleByteFont;

			UT_ASSERT(font);
			if(!font)
				return;
		}

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

				switch(isOverstrikingChar(*pC))
				{
				case UT_NOT_OVERSTRIKING:
				default:
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
			}
			else
			{
				// not a unicode font; actual is guaranteed to be <=0xff
				// (this happens typically when drawing the interface)
				gchar gc = (gchar) actual;

				switch(isOverstrikingChar(*pC))
				{
				case UT_NOT_OVERSTRIKING:
				default:
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
#endif // USE_XFT
}

void GR_UnixGraphics::setFont(GR_Font * pFont)
{
	XAP_UnixFontHandle * pUFont = static_cast<XAP_UnixFontHandle *> (pFont);

	// Sometimes we ask gr_UnixGraphics to build big (*BIG*) fonts only to
	// get the linear metrics of the font (in the so called "layout units").
	// Xft is not able to open fonts so big, so if we are called with such
	// a font, then we don't even try to open it.
	// IMO the code should not create a big GR_Font to get the linear metrics,
	// but just ask for the metrics with float precision, for instance.
	// I'm just taking here the shortest path to get Xft working...

	// this is probably caching done on the wrong level
	// but it's currently faster to shortcut
	// than to call explodeGdkFonts
	// TODO: turn this off when our text runs get a bit smarter
	if(m_pFont && (pUFont->getUnixFont() == m_pFont->getUnixFont()) &&
	   (pUFont->getSize() == m_pFont->getSize()))
		return;

	m_pFont = pUFont;

#ifdef USE_XFT
	UT_uint32 size = pUFont->getSize();
	if (size < MAX_ABI_GDK_FONT_SIZE)
	{
		m_bLayoutUnits = false;
		m_pXftFont = m_pFont->getXftFont();
		m_XftFaceLocker = XftFaceLocker(m_pXftFont);

	}
	else
	{
		m_bLayoutUnits = true;
		m_pXftFont = NULL;
	}
#else
//
// Only use gdk fonts for Low resolution
//
	if(pUFont->getSize()< MAX_ABI_GDK_FONT_SIZE)
		m_pFont->explodeGdkFonts(m_pSingleByteFont,m_pMultiByteFont);
#endif
}

UT_uint32 GR_UnixGraphics::getFontHeight(GR_Font * fnt)
{
	return getFontAscent(fnt)+getFontDescent(fnt);
}

#ifdef USE_XFT
void GR_UnixGraphics::getCoverage(UT_Vector& coverage)
{
	m_pFont->getUnixFont()->getCoverage(coverage);
}
#else
void GR_UnixGraphics::getCoverage(UT_Vector& coverage)
{
	coverage.clear();
	coverage.push_back((void*) ' ');
	coverage.push_back((void*) (127 - ' '));
}
#endif

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

#ifdef USE_XFT
	if (m_bLayoutUnits)
	{
		xxx_UT_DEBUGMSG(("Using measureUnRemappedChar in layout units\n"));
		float width = m_pFont->getUnixFont()->measureUnRemappedChar(c, m_pFont->getSize());
		return (UT_uint32) (width + 0.5);
	}
	else
	{
		xxx_UT_DEBUGMSG(("Using measureUnRemappedChar in screen units\n"));
		XGlyphInfo extents;
		XftTextExtents32(GDK_DISPLAY(), m_pXftFont, const_cast<XftChar32*> (&c), 1, &extents);
		return extents.xOff;
	}

#else

	if(c == 0x200B || c == 0xFEFF) // 0-with spaces
		return 0;

	if (!m_pFontManager)
		return 0;

	UT_ASSERT(m_pFont);
	UT_ASSERT(m_pGC);

	GdkFont * font;
	UT_UCSChar Wide_char = c;
//
// Use GDK at Low resolutions, Metrics at high resolution. This saves tons
// of memory on the X server and speeds up things enormously.
//
	if(m_pFont->getSize() <  MAX_ABI_GDK_FONT_SIZE)
	{
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
//
// Use Metric info. From PS Graphics.
//
	else
	{
		double dsize = (double) m_pFont->getSize();
	    XAP_UnixFont *pEnglishFont;
		XAP_UnixFont *pChineseFont;
	    m_pFont->explodeUnixFonts(&pEnglishFont,&pChineseFont);
//
// The metrics are in 1/1000th's of an inch, we need to convert these to
// pixels.  Try this....
//
		double fFactor;
		fFactor = (double) 1.0/1000.0;
		if (XAP_EncodingManager::get_instance()->is_cjk_letter(c))
		{
			return (UT_uint32) ( fFactor * dsize * (double) pChineseFont->get_CJK_Width());
		}
		else
		{
			UT_uint32 width;
			width = (UT_uint32) (fFactor * dsize * (double) pEnglishFont->getCharWidth(c));
			return width;
		}
	}
#endif
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
		if(cChar == 0x200B || cChar == 0xFEFF)
			continue;

		width = gdk_char_width_wc (pFont, cChar);
		charWidth += width;
		if (pWidths)
			pWidths[i] = width;
    }

	return charWidth;
}
#endif

#else
void GR_UnixGraphics::_drawFT2Bitmap(UT_sint32 x, UT_sint32 y, FT_Bitmap * pBitmap) const
{
	// TODO: provide implementation ...
}

#endif // #ifndef WITH_PANGO


UT_uint32 GR_UnixGraphics::_getResolution(void) const
{
	// this is hard-coded at 96 for X now, since 75 (which
	// most X servers return when queried for a resolution)
	// makes for tiny fonts on modern resolutions.

	return 96;
}

void GR_UnixGraphics::setColor(const UT_RGBColor& clr)
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

#ifdef USE_XFT
	m_XftColor.color.red = c.red;
	m_XftColor.color.green = c.green;
	m_XftColor.color.blue = c.blue;
	m_XftColor.color.alpha = 0xffff;
	m_XftColor.pixel = c.pixel;
#endif
	
	/* Set up the XOR gc */
	gdk_gc_set_foreground(m_pXORGC, &c);
	gdk_gc_set_function(m_pXORGC, GDK_XOR);
}

GR_Font * GR_UnixGraphics::getGUIFont(void)
{
#ifndef WITH_PANGO
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
#endif
	// TODO provide PANGO implementation
	return s_pFontGUI;
}

#ifndef WITH_PANGO
#ifdef USE_XFT
/**
 * Finds a font which match the family, style, variant, weight and size
 * asked.  It will do a fuzzy match to find the font (using the aliases
 * found in fonts.conf
 */
GR_Font * GR_UnixGraphics::findFont(const char* pszFontFamily,
									const char* pszFontStyle,
									const char* pszFontVariant,
									const char* pszFontWeight,
									const char* pszFontStretch,
									const char* pszFontSize)
{
	XAP_UnixFont* pUnixFont = XAP_UnixFontManager::findNearestFont(pszFontFamily, pszFontStyle, pszFontVariant, pszFontWeight,
																   pszFontStretch, pszFontSize);

	// bury the pointer to our Unix font in a XAP_UnixFontHandle with the correct size.
	// This piece of code scales the FONT chosen at low resolution to that at high
	// resolution. This fixes bug 1632 and other non-WYSIWYG behaviour.
	UT_uint32 iSize = getAppropriateFontSizeFromString(pszFontSize);
	XAP_UnixFontHandle* pFont = new XAP_UnixFontHandle(pUnixFont, iSize);
	UT_ASSERT(pFont);

	return pFont;
}

#else

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
		// Oops!  We don't have that font here.
		// first try "Times New Roman", which should be sensible, and should
		// be there unless the user fidled with the installation
		unixfont = m_pFontManager->getFont("Times New Roman", s);

		// Oh well, see if there are any fonts at all, and if so
		// just take the first one ...
		if(!unixfont)
		{
				UT_Vector *	pVec = m_pFontManager->getAllFonts();
				if(pVec && pVec->getItemCount() > 0)
				{
					// get the first font we have
					unixfont = static_cast<XAP_UnixFont *>(pVec->getNthItem(0));
				}

				// free the returned vector
				DELETEP(pVec);

		}

		// this is really desperate, we do not seem to have any fonts
		// we cannot be blamed if we just give up

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

//
// This piece of code scales the FONT chosen at low resolution to that at high
// resolution. This fixes bug 1632 and other non-WYSIWYG behaviour.
//
	UT_uint32 iSize = getAppropriateFontSizeFromString(pszFontSize);
	XAP_UnixFontHandle * pFont = new XAP_UnixFontHandle(unixfont, iSize);
	UT_ASSERT(pFont);

	return pFont;
}
#endif

GR_Font* GR_UnixGraphics::getDefaultFont(UT_String& fontFamily)
{
	static XAP_UnixFontHandle fontHandle(m_pFontManager->getDefaultFont(), 12);
	fontFamily = fontHandle.getUnixFont()->getName();
	
	return &fontHandle;
}

UT_uint32 GR_UnixGraphics::getFontAscent(GR_Font * fnt)
{
	UT_ASSERT(fnt);
	UT_ASSERT(m_pGC);

	XAP_UnixFontHandle * hndl = static_cast<XAP_UnixFontHandle *>(fnt);
	
#ifdef USE_XFT
	return (UT_uint32) (hndl->getUnixFont()->getAscender(hndl->getSize()) + 0.5);
#else
//
// Use GDK at low resolution.
//
	if(hndl->getSize() < MAX_ABI_GDK_FONT_SIZE)
	{
		GdkFont* pFont = hndl->getGdkFont();
		GdkFont* pMatchFont=hndl->getMatchGdkFont();

		// with the incremental loader in place, this happens to be
		// the first place which tries to load fonts (because we need
		// to compute font ascents).  If they're not present, these
		// GdkFont guys will be NULL, so we would segfault.  Let's
		// quietly abort instead.

		if (!pFont || !pMatchFont)
			abort();

		return MAX(pFont->ascent, pMatchFont->ascent);
	}
//
// Use metrics info at higher resolution.
//
	else
	{
		XAP_UnixFont * pSingleByte = NULL;
		XAP_UnixFont * pMultiByte = NULL;
		hndl->explodeUnixFonts(&pSingleByte,&pMultiByte);

		// Some more crash protection.  Why not?
		if (!pSingleByte || !pSingleByte->getMetricsData())
			abort();

		GlobalFontInfo * gfsi = pSingleByte->getMetricsData()->gfi;
		UT_ASSERT(gfsi);
		UT_uint32 ascsingle = (UT_uint32) ( (double) gfsi->fontBBox.ury * (double) hndl->getSize() /1000.);
		UT_uint32 ascmulti = (UT_uint32) ( (double) pMultiByte->get_CJK_Ascent() * (double) hndl->getSize() /1000.);
		return MAX(ascsingle,ascmulti);
	}
#endif
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

#ifdef USE_XFT
	XAP_UnixFont* pFont = hndl->getUnixFont();
	return (UT_uint32) (pFont->getDescender(hndl->getSize()) + 0.5);
#else
//
// Use GDK at low resolution.
//
	if(hndl->getSize() <  MAX_ABI_GDK_FONT_SIZE )
	{
		GdkFont* pFont = hndl->getGdkFont();
		GdkFont* pMatchFont=hndl->getMatchGdkFont();
		return MAX(pFont->descent, pMatchFont->descent);
	}
//
// Use metrics info at higher resolution.
//
	else
	{
		XAP_UnixFont * pSingleByte = NULL;
		XAP_UnixFont * pMultiByte = NULL;
		hndl->explodeUnixFonts(&pSingleByte,&pMultiByte);
		GlobalFontInfo * gfsi = pSingleByte->getMetricsData()->gfi;
		UT_ASSERT(gfsi);
		UT_uint32 dsingle = (UT_uint32) ( -(double) gfsi->fontBBox.lly * (double) hndl->getSize() /1000.);
		UT_uint32 dmulti = (UT_uint32) ( (double) pMultiByte->get_CJK_Descent() * (double) hndl->getSize() /1000.);
		return MAX(dsingle,dmulti);
	}
#endif
}

UT_uint32 GR_UnixGraphics::getFontDescent()
{
	return getFontDescent(m_pFont);
}
#endif //#ifndef WITH_PANGO


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
#ifdef USE_XFT
		Region region;
		XPoint points[4];

		points[0].x = r.x;
		points[0].y = r.y;
			
		points[1].x = r.x + r.width;
		points[1].y = r.y;
			
		points[2].x = r.x + r.width;
		points[2].y = r.y + r.height;
			
		points[3].x = r.x;
		points[3].y = r.y + r.height;

		xxx_UT_DEBUGMSG(("Setting clipping rectangle: (%d, %d, %d, %d)\n", r.x, r.y, r.width, r.height));
		region = XPolygonRegion(points, 4, EvenOddRule);
		if (region)
		{
			XftDrawSetClip(m_pXftDraw, region);
			XDestroyRegion (region);
		}
#endif
	}
	else
	{
		gdk_gc_set_clip_rectangle(m_pGC, NULL);
		gdk_gc_set_clip_rectangle(m_pXORGC, NULL);

#ifdef USE_XFT
		xxx_UT_DEBUGMSG(("Setting clipping rectangle NULL\n"));
		XftDrawSetClip(m_pXftDraw, 0);
#endif
	}
}

void GR_UnixGraphics::fillRect(const UT_RGBColor& c, UT_sint32 x, UT_sint32 y,
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
		// We're moving right so left is reduced
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

	if (image == 0)
	{
		UT_DEBUGMSG(("Found no image data. This is probably SVG masquerading as a raster!\n"));
		return;
	}

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


	pImg = new GR_UnixGnomeImage(pszName,false);
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

	UT_ASSERT(image);

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

	//I have changed the shape of the arrow so get a consistent
	//behaviour in the bidi build; I think the new arrow is better
	//for the purpose anyway

	case GR_CURSOR_RIGHTARROW:
		cursor_number = GDK_SB_RIGHT_ARROW; //GDK_ARROW;
		break;

	case GR_CURSOR_LEFTARROW:
		cursor_number = GDK_SB_LEFT_ARROW; //GDK_LEFT_PTR;
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

	case GR_CURSOR_LINK:
		cursor_number = GDK_HAND2;
		break;

	case GR_CURSOR_WAIT:
		cursor_number = GDK_WATCH;
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

#ifndef WITH_PANGO
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
#endif
