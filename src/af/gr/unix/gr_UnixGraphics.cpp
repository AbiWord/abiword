/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
#include "xap_Strings.h"

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
#include <X11/Xft/Xft.h>
#endif

/* XPM */
static char * cursor_select_vline_xpm[] = {
"24 24 2 1",
" 	c None",
".	c #000000",
"                        ",
"                        ",
"        ..   ..         ",
"        ..   ..         ",
"        ..   ..         ",
"        ..   ..         ",
"        ..   ..         ",
"        ..   ..         ",
"        ..   ..         ",
"        ..   ..         ",
"        ..   ..         ",
"     .....   .....      ",
"     .....   .....      ",
"        ..   ..         ",
"        ..   ..         ",
"        ..   ..         ",
"        ..   ..         ",
"        ..   ..         ",
"        ..   ..         ",
"        ..   ..         ",
"        ..   ..         ",
"                        ",
"                        ",
"                        "};

/* XPM */
static char * cursor_select_hline_xpm[] = {
"24 24 2 1",
" 	c None",
".	c #000000",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        ",
"           ..           ",
"           ..           ",
"           ..           ",
"   ..................   ",
"   ..................   ",
"                        ",
"                        ",
"                        ",
"   ..................   ",
"   ..................   ",
"           ..           ",
"           ..           ",
"           ..           ",
"                        ",
"                        ",
"                        ",
"                        ",
"                        "};



static UT_uint32 adobeSUni[/*185*/][2] =
	{
		{32,32},
		{33,33},
		{34,8704},
		{35,35},
		{36,8707},
		{37,37},
		{38,38},
		{39,8715},
		{40,40},
		{41,41},
		{42,8727},
		{43,43},
		{44,44},
		{45,8722},
		{46,46},
		{47,47},
		{48,48},
		{49,49},
		{50,50},
		{51,51},
		{52,52},
		{53,53},
		{54,54},
		{55,55},
		{56,56},
		{57,57},
		{58,58},
		{59,59},
		{60,60},
		{61,61},
		{62,62},
		{63,63},
		{64,8773},
		{65,913},
		{66,914},
		{67,935},
		{68,8710},
		{69,917},
		{70,934},
		{71,915},
		{72,919},
		{73,921},
		{74,977},
		{75,922},
		{76,923},
		{77,924},
		{78,925},
		{79,927},
		{80,928},
		{81,920},
		{82,929},
		{83,931},
		{84,932},
		{85,933},
		{86,962},
		{87,8486},
		{88,926},
		{89,936},
		{90,918},
		{91,91},
		{92,8756},
		{93,93},
		{94,8869},
		{95,95},
		{96,63717},
		{97,945},
		{98,946},
		{99,967},
		{100,948},
		{101,949},
		{102,966},
		{103,947},
		{104,951},
		{105,953},
		{106,981},
		{107,954},
		{108,955},
		{109,181},
		{110,957},
		{111,959},
		{112,960},
		{113,952},
		{114,961},
		{115,963},
		{116,964},
		{117,965},
		{119,969},
		{120,958},
		{121,968},
		{122,950},
		{123,123},
		{124,124},
		{125,125},
		{126,8764},
		{163,8804},
		{164,8260},
		{165,8734},
		{166,402},
		{167,9827},
		{168,9830},
		{169,9829},
		{170,9824},
		{171,8596},
		{172,8592},
		{173,8593},
		{174,8594},
		{175,8595},
		{176,176},
		{177,177},
		{179,8805},
		{180,215},
		{181,8733},
		{182,8706},
		{183,8226},
		{184,247},
		{185,8800},
		{186,8801},
		{187,8776},
		{188,8230},
		{189,63718},
		{190,63719},
		{191,8629},
		{192,8501},
		{193,8465},
		{194,8476},
		{195,8472},
		{196,8855},
		{197,8853},
		{198,8709},
		{199,8745},
		{200,8746},
		{201,8835},
		{202,8839},
		{203,8836},
		{204,8834},
		{205,8838},
		{206,8712},
		{207,8713},
		{208,8736},
		{209,8711},
		{210,0},
		{211,63193},
		{212,63195},
		{213,8719},
		{214,8730},
		{215,8901},
		{216,172},
		{217,8743},
		{218,8744},
		{219,8660},
		{220,8656},
		{221,8657},
		{222,8658},
		{223,8659},
		{224,9674},
		{225,9001},
		{226,0},
		{227,63721},
		{228,63722},
		{229,8721},
		{230,63723},
		{231,63724},
		{232,63725},
		{233,63726},
		{234,63727},
		{235,63728},
		{236,63729},
		{237,63730},
		{238,0},
		{239,63732},
		{241,9002},
		{242,8747},
		{243,8992},
		{244,63733},
		{245,8993},
		{246,63734},
		{247,63735},
		{248,63736},
		{249,63737},
		{250,63738},
		{251,63739},
		{252,63740},
		{253,63741},
		{254,63742},
		{255,100000}
	};

static UT_uint32 adobeToUnicode(UT_uint32 iAdobe)
{
	UT_uint32 low = adobeSUni[0][0];
	UT_uint32 high = adobeSUni[183][0];
	if(iAdobe < low)
	{
		return iAdobe;
	}
	if(iAdobe > high)
	{
		return iAdobe;
	}
	UT_sint32 slow = (UT_sint32) iAdobe - 72;
	if(slow < 0)
	{ 
		slow = 0;
	}
	while(adobeSUni[slow][0] != iAdobe && slow < 255)
	{
		xxx_UT_DEBUGMSG(("char at %d is %d value %d \n",slow,adobeSUni[slow][0],adobeSUni[slow][1]));
		slow++;
	}
	xxx_UT_DEBUGMSG(("Input %d return %d \n",iAdobe,adobeSUni[slow][1]));
	if(slow > 255)
	{
		return iAdobe;
	}
	return adobeSUni[slow][1];
}

const char* GR_Graphics::findNearestFont(const char* pszFontFamily,
										 const char* pszFontStyle,
										 const char* pszFontVariant,
										 const char* pszFontWeight,
										 const char* pszFontStretch,
										 const char* pszFontSize)
{

#ifdef USE_XFT 
	XAP_UnixFont* pUnixFont = XAP_UnixFontManager::pFontManager->findNearestFont(pszFontFamily, pszFontStyle, pszFontVariant, pszFontWeight,
																				 pszFontStretch, pszFontSize);
	return pUnixFont->getName();
#else
	return NULL;
#endif
}

#if (!defined(WITH_PANGO) || !defined(USE_XFT))
#include <gdk/gdkprivate.h>
static bool isFontUnicode(GdkFont *font)
{
       if(!font)
       {
               UT_DEBUGMSG(("gr_UnixGraphics: isFontUnicode: font is NULL !!!\n"));
               return false;
       }

       return false ;
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
		:
#if (!defined(WITH_PANGO) || !defined(USE_XFT))
         m_wctomb(new UT_Wctomb),
#endif
#ifdef USE_XFT
         m_bLayoutUnits(false),
#endif
		 m_saveBuf(NULL), m_saveRect(NULL)
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
	m_pColormap = gdk_rgb_get_cmap(); // = gdk_colormap_get_system();

#ifdef USE_XFT
	//
	// Martin's attempt to make double buffering work.with xft
	//
	m_iXoff = 0;
	m_iYoff = 0;
	GdkDrawable * realDraw;
	gdk_window_get_internal_paint_info (m_pWin, &realDraw,&m_iXoff,&m_iYoff);
	m_pGC = gdk_gc_new(realDraw);
	m_pXORGC = gdk_gc_new(realDraw);
	m_pVisual = GDK_VISUAL_XVISUAL( gdk_drawable_get_visual(realDraw));
	m_Drawable = gdk_x11_drawable_get_xid(realDraw);



	m_pXftFont = NULL;
	m_Colormap = GDK_COLORMAP_XCOLORMAP(m_pColormap);
	m_pXftDraw = XftDrawCreate(GDK_DISPLAY(), m_Drawable, m_pVisual, m_Colormap);
#else
	m_pGC = gdk_gc_new(m_pWin);
	m_pXORGC = gdk_gc_new(m_pWin);
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
	m_bIsSymbol = false;
	m_bIsDingbat = false;

#ifndef WITH_PANGO
	if (m_pFontManager)
		m_pFallBackFontHandle = new XAP_UnixFontHandle(m_pFontManager->getDefaultFont(),
													   FALLBACK_FONT_SIZE);
	else
		m_pFallBackFontHandle = NULL;
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

#if (!defined(WITH_PANGO) || !defined(USE_XFT))
	DELETEP(m_wctomb);
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
	gdk_gc_set_line_attributes ( m_pXORGC, (gint)inWidthPixels,
								 mapLineStyle ( inLineStyle ),
								 mapCapStyle ( inCapStyle ),
								 mapJoinStyle ( inJoinStyle ) ) ;
}

#if (!defined(WITH_PANGO) || !defined(USE_XFT))

#define WCTOMB_DECLS m_wctomb->initialize()

#define CONVERT_TO_MBS(c)	\
    	if (c<=0xff) {	\
		/* this branch is to allow Lists to function */	\
		m_text[0] = (unsigned char)c;			\
		m_text_length = 1;				\
		m_fallback_used = 0;				\
	} else	{\
		m_fallback_used = 0;	\
		if (!m_wctomb->wctomb(m_text,m_text_length,c)) {	\
		    m_wctomb->wctomb_or_fallback(m_text,m_text_length,c);	\
		    m_fallback_used = 1;	\
		}	\
	}
#endif

#ifndef WITH_PANGO
void GR_UnixGraphics::drawGlyph(UT_uint32 Char, UT_sint32 xoff, UT_sint32 yoff)
{
	_UUD(xoff);
	_UUD(yoff);

#ifdef USE_XFT
	UT_uint32 iChar = Char;
	if(m_bIsSymbol && iChar < 255  && iChar >= 32)
	{
		iChar = adobeToUnicode(Char);
		UT_DEBUGMSG(("DrawGlyph remapped %d to %d \n",Char,iChar));
	}
	XftDrawGlyphs(m_pXftDraw, &m_XftColor, m_pXftFont, xoff + m_iXoff, yoff + m_pXftFont->ascent + m_iYoff, &iChar, 1);
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
			gdk_draw_text(m_pWin,font,m_pGC,xoff,yoff+font->ascent,(gchar*)&gc,1);
		}
	}
	else
	{
		WCTOMB_DECLS;
		CONVERT_TO_MBS(Wide_char);
		gdk_draw_text(m_pWin,font,m_pGC,xoff,yoff+font->ascent,m_text,m_text_length);
	}
#endif
}

void GR_UnixGraphics::drawChars(const UT_UCSChar* pChars, int iCharOffset,
				int iLength, UT_sint32 xoff, UT_sint32 yoff,
				int * pCharWidths)
{
  _UUD(xoff);
  _UUD(yoff);
  
#ifdef USE_XFT
	if (iLength == 0)
		return;
	
	yoff += m_pXftFont->ascent;
	if(m_bIsSymbol && pCharWidths == NULL)
	{
		UT_DEBUGMSG(("FIXME: Put some code here!!! \n"));
	}
	if (!pCharWidths)
	{
		if(!m_bIsSymbol)
		{
			XftDrawString32(m_pXftDraw, &m_XftColor, m_pXftFont, xoff + m_iXoff, yoff + m_iYoff,
							const_cast<XftChar32*> (pChars + iCharOffset), iLength);
		}
		else
		{
			UT_uint32 * uChars = new UT_uint32[iLength];
			for(UT_uint32 i = (UT_uint32) iCharOffset; i< (UT_uint32) iLength; i++)
			{
				uChars[i] = (UT_uint32) pChars[i];
				if(uChars[i] < 255 && uChars[i] >= 32)
				{
					uChars[i] = adobeToUnicode(uChars[i]);
					UT_DEBUGMSG(("drawchars: mapped %d to %d \n",pChars[i],uChars[i]));
				}
			}
			XftDrawString32(m_pXftDraw, &m_XftColor, m_pXftFont, xoff + m_iXoff, yoff + m_iYoff,
							const_cast<XftChar32*> (uChars + iCharOffset), iLength);
			delete [] uChars;
		}
	}
	else
	{
		XftCharSpec aCharSpec[256];
		XftCharSpec* pCharSpec = aCharSpec;
		
		if (iLength > 256)
			pCharSpec = new XftCharSpec[iLength];

		pCharSpec[0].ucs4 = (FT_UInt) pChars[iCharOffset];
		UT_uint32 uChar = (UT_uint32) pCharSpec[0].ucs4;
		pCharSpec[0].x = xoff;
		pCharSpec[0].y = yoff;
		if(m_bIsSymbol && uChar < 255 && uChar >=32)
		{
			pCharSpec[0].ucs4 = (FT_UInt) adobeToUnicode(uChar);
			UT_DEBUGMSG(("DrawGlyph remapped %d to %d \n",uChar,pCharSpec[0].ucs4));
		}
		for (int i = 1; i < iLength; ++i)
		{
			uChar = (UT_uint32) pCharSpec[i].ucs4;
			if(m_bIsSymbol && uChar < 255 && uChar >=32)
			{
				pCharSpec[i].ucs4 = (FT_UInt) adobeToUnicode(uChar);
			}
			pCharSpec[i].ucs4 = (FT_UInt) pChars[i + iCharOffset];
			pCharSpec[i].x = (short) (pCharSpec[i - 1].x + pCharWidths[i - 1]);
			pCharSpec[i].y = yoff;
		}
		
		XftDrawCharSpec (m_pXftDraw, &m_XftColor, m_pXftFont, pCharSpec, iLength);

		if (pCharSpec != aCharSpec)
			delete[] pCharSpec;
	}
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
	GR_CaretDisabler caretDisabler(getCaret());

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
				const XML_Char * msg = pApp->getStringSet()->getValueUTF8(XAP_STRING_ID_MSG_UnixFontSizeWarning).c_str();
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

				switch(UT_OVERSTRIKING_DIR & UT_isOverstrikingChar(*pC))
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

				switch(UT_OVERSTRIKING_DIR & UT_isOverstrikingChar(*pC))
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
			gdk_draw_text(m_pWin,font,m_pGC,x,yoff+font->ascent,m_text,m_text_length);
			x+=gdk_text_width(font, m_text, m_text_length);
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

	// this probably is not safe. It was observed in the win32 build that
	// identity of font pointer does not imply identity of font, i.e.,
	// code like this
	// 
	//   f1 = new GR_Font();
	//   delete f1;
	//   f2 = new GR_Font(); /* different font altogether */
	//
	//   can result in f1 == f2 and since the allocation and
	//   deallocation of fonts happens outside of the graphics class,
	//   the chached m_pFont could well be pointing to
	//   a different font than intended (or something completely
	//   different. I am not sure whether this is or is not the case
	//   on Unix, really depends on where the font pointer comes from,
	//   so I will not meddle with this, but it needs to be
	//   investigated by someone who knows better -- Tomas
	
	m_bIsSymbol = false;
	m_bIsDingbat = false;
	if(m_pFont && (pUFont->getUnixFont() == m_pFont->getUnixFont()) &&
	   (pUFont->getSize() == m_pFont->getSize()))
		return;

	m_pFont = pUFont;
	char * szUnixFontName = UT_strdup(m_pFont->getUnixFont()->getName());
	const char * szFontName = UT_lowerString(szUnixFontName);

	if (szFontName)
	{
		if(strstr(szFontName,"symbol") != NULL)
		{
			m_bIsSymbol = true;
			if(strstr(szFontName,"star") != NULL)
			{
				m_bIsSymbol = false;
			}
			UT_DEBUGMSG(("UnixGraphics: Found Symbol font \n"));
		}
		if(strstr(szFontName,"dingbat") != NULL)
		{
			m_bIsDingbat = true;
		}
	}
#ifdef USE_XFT
	UT_uint32 size = pUFont->getSize();
	if (szFontName)
	{
		if(strstr(szFontName,"Symbol") != NULL)
		{
			m_bIsSymbol = true;
			UT_DEBUGMSG(("unixGraphics: Found Symbol Font! \n"));
		}
	}
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
	float width;
	if (m_bLayoutUnits)
	{
		xxx_UT_DEBUGMSG(("Using measureUnRemappedChar in layout units\n"));
		if(!m_bIsSymbol)
		{
			width = m_pFont->getUnixFont()->measureUnRemappedChar(c, m_pFont->getSize());
		}
		else
		{
			width = m_pFont->getUnixFont()->measureUnRemappedChar((UT_UCSChar) adobeToUnicode(c), m_pFont->getSize());
		}
		return (UT_uint32) _UL((width + 0.5));
	}
	else
	{
		xxx_UT_DEBUGMSG(("Using measureUnRemappedChar in screen units\n"));
		XGlyphInfo extents;
		UT_UCSChar cc = c;
		if(m_bIsSymbol)
		{
			cc = adobeToUnicode((UT_uint32) cc);
		}
		XftTextExtents32(GDK_DISPLAY(), m_pXftFont, static_cast<XftChar32*> (&cc), 1, &extents);
		return _UL(extents.xOff);
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
					return _UL(gdk_text_width(font, (gchar*) &Wide_char, 2));
			}
			else
			{
				//this is not a unicode font
				if(c > 0xff) //a non unicode font contains only 256 chars
					return 0;
				else
				{
					gchar gc = (gchar) c;
					return _UL(gdk_text_width(font, (gchar*)&gc, 1));
				}
			}
		}
		else
		{
			WCTOMB_DECLS;
			CONVERT_TO_MBS(Wide_char);
			if (m_fallback_used)
				return 0;
			font = XAP_EncodingManager::get_instance()->is_cjk_letter(Wide_char) ? m_pMultiByteFont : m_pSingleByteFont;

			return _UL(gdk_text_width(font, m_text, m_text_length));
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
		double fFactor = (double) 1.0/1000.0;
		if (XAP_EncodingManager::get_instance()->is_cjk_letter(c))
		{
			return (UT_uint32) _UL((fFactor * dsize * (double) pChineseFont->get_CJK_Width()));
		}
		else
		{
			UT_uint32 width;
			width = (UT_uint32) _UL((fFactor * dsize * (double) pEnglishFont->getCharWidth(c)));
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

	return _UL(96);
}

void GR_UnixGraphics::getColor(UT_RGBColor& clr)
{
	clr = m_curColor;
}

void GR_UnixGraphics::setColor(const UT_RGBColor& clr)
{
	UT_ASSERT(m_pGC);
	GdkColor c;

	c.red = clr.m_red << 8;
	c.blue = clr.m_blu << 8;
	c.green = clr.m_grn << 8;

	m_curColor = clr;
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
	XAP_UnixFont* pUnixFont = m_pFontManager->findNearestFont(pszFontFamily, pszFontStyle, pszFontVariant, pszFontWeight,
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
	GR_CaretDisabler caretDisabler(getCaret());
	// TODO set the line width according to m_iLineWidth
	gdk_draw_line(m_pWin, m_pGC, _UD(x1), _UD(y1), _UD(x2), _UD(y2));
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
	GR_CaretDisabler caretDisabler(getCaret());
	gdk_draw_line(m_pWin, m_pXORGC, _UD(x1), _UD(y1), _UD(x2), _UD(y2));
}

void GR_UnixGraphics::polyLine(UT_Point * pts, UT_uint32 nPoints)
{
	GR_CaretDisabler caretDisabler(getCaret());

	// see bug #303 for what this is about

	GdkPoint * points = (GdkPoint *)calloc(nPoints, sizeof(GdkPoint));
	UT_ASSERT(points);

	for (UT_uint32 i = 0; i < nPoints; i++)
	{
		points[i].x = _UD(pts[i].x);
		// It seems that Windows draws each pixel along the the Y axis
		// one pixel beyond where GDK draws it (even though both coordinate
		// systems start at 0,0 (?)).  Subtracting one clears this up so
		// that the poly line is in the correct place relative to where
		// the rest of GR_UnixGraphics:: does things (drawing text, clearing
		// areas, etc.).
		points[i].y = _UD(pts[i].y - 1);
	}

	gdk_draw_lines(m_pWin, m_pGC, points, nPoints);

	FREEP(points);
}

void GR_UnixGraphics::invertRect(const UT_Rect* pRect)
{
	GR_CaretDisabler caretDisabler(getCaret());
	UT_ASSERT(pRect);
	gdk_draw_rectangle(m_pWin, m_pXORGC, 1, _UD(pRect->left), _UD(pRect->top),
			   _UD(pRect->width), _UD(pRect->height));
}

void GR_UnixGraphics::setClipRect(const UT_Rect* pRect)
{
	m_pRect = pRect;
	if (pRect)
	{
		GdkRectangle r;

		r.x = _UD(pRect->left);
		r.y = _UD(pRect->top);
		r.width = _UD(pRect->width);
		r.height = _UD(pRect->height);

		gdk_gc_set_clip_rectangle(m_pGC, &r);
		gdk_gc_set_clip_rectangle(m_pXORGC, &r);
#ifdef USE_XFT
		Region region;
		XPoint points[4];
		points[0].x = r.x + m_iXoff;
		points[0].y = r.y - r.height + m_iYoff;
			
		points[1].x = r.x + r.width  + m_iXoff;
		points[1].y = r.y - r.height  + m_iYoff;
			
		points[2].x = r.x + r.width  + m_iXoff;
		points[2].y = r.y + r.height + m_iYoff;
			
		points[3].x = r.x  + m_iXoff;
		points[3].y = r.y + r.height + m_iYoff;

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
	GR_CaretDisabler caretDisabler(getCaret());
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

 	gdk_draw_rectangle(m_pWin, m_pGC, 1, _UD(x), _UD(y), _UD(w), _UD(h));

	gdk_gc_set_foreground(m_pGC, &oColor);
}

void GR_UnixGraphics::scroll(UT_sint32 dx, UT_sint32 dy)
{
	GR_CaretDisabler caretDisabler(getCaret());
	gdk_window_scroll(m_pWin,-dx,-dy);
}

void GR_UnixGraphics::scroll(UT_sint32 x_dest, UT_sint32 y_dest,
						  UT_sint32 x_src, UT_sint32 y_src,
						  UT_sint32 width, UT_sint32 height)
{
	GR_CaretDisabler caretDisabler(getCaret());
	gdk_window_copy_area(m_pWin, m_pGC, _UD(x_dest), _UD(y_dest),
			     _UD(m_pWin), _UD(x_src), _UD(y_src), _UD(width), _UD(height));
}

void GR_UnixGraphics::clearArea(UT_sint32 x, UT_sint32 y,
				UT_sint32 width, UT_sint32 height)
{
	GR_CaretDisabler caretDisabler(getCaret());

	_UUD(x);
	_UUD(y);
	_UUD(width);
	_UUD(height);

	if (width > 0)
	{
		static const UT_RGBColor clrWhite(255,255,255);
		fillRect(clrWhite, x, y, width, height);
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

// gdk-pixbuf based routines

GR_Image* GR_UnixGraphics::createNewImage(const char* pszName, const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight, GR_Image::GRType iType)
{
   	GR_Image* pImg = NULL;

	pImg = new GR_UnixImage(pszName,false);
	pImg->convertFromBuffer(pBB, _UD(iDisplayWidth), _UD(iDisplayHeight));
   	return pImg;
}

// a bit of voodoo since i'm not entirely sure what the
// alpha_threshold param means. I know it takes values 0 <= threshold <= 255
// and that values < than the alpha threshold are painted as 0s
// this seems to work for me, so I'm happy - Dom
#define ABI_ALPHA_THRESHOLD 100

void GR_UnixGraphics::drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest)
{
	GR_CaretDisabler caretDisabler(getCaret());
	UT_ASSERT(pImg);

   	GR_UnixImage * pUnixImage = static_cast<GR_UnixImage *>(pImg);

	GdkPixbuf * image = pUnixImage->getData();
	UT_return_if_fail(image);

	_UUD(xDest);
	_UUD(yDest);
   	UT_sint32 iImageWidth = _UD(pUnixImage->getDisplayWidth());
   	UT_sint32 iImageHeight = _UD(pUnixImage->getDisplayHeight());

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

	case GR_CURSOR_HLINE_DRAG:
		{
#if 0
			GdkPixmap * source = NULL;
			GdkPixmap * mask = NULL;
			GdkColor  fg;
			GdkColor  bg;
			gdk_color_black	 (m_pColormap,&fg);
			gdk_color_white	 (m_pColormap,&bg);
			source	= gdk_pixmap_colormap_create_from_xpm_d(m_pWin,NULL,
														&mask, NULL,
														(char ** )cursor_select_hline_xpm);
			GDK_IS_PIXMAP(source);
			GDK_IS_PIXMAP(mask);
			UT_DEBUGMSG(("setCursor: source = %x \n",source));
			GdkCursor* cursor_new = gdk_cursor_new_from_pixmap(source,mask,&fg,
															   &bg,12,12);
			gdk_window_set_cursor(m_pWin, cursor_new);
			//			gdk_cursor_destroy(cursor_new);
			// gdk_pixmap_unref(source);
			// gdk_bitmap_unref(mask);
			return;
#else
		cursor_number = GDK_SB_V_DOUBLE_ARROW;
		break;

#endif
		}

	case GR_CURSOR_VLINE_DRAG:
		{
#if 0
			GdkPixmap * source = NULL;
			GdkPixmap * mask = NULL;
			GdkColor fg;
			GdkColor  bg;
			gdk_color_black	 (m_pColormap,&fg);
			gdk_color_white	 (m_pColormap,&bg);
			source = gdk_pixmap_colormap_create_from_xpm_d(m_pWin,NULL,
														&mask, NULL,
														(char ** )cursor_select_vline_xpm);
			GdkCursor* cursor_new = gdk_cursor_new_from_pixmap(source,mask,&fg,
															   &bg,12,12);
			gdk_window_set_cursor(m_pWin, cursor_new);
			//	gdk_cursor_destroy(cursor_new);
			// gdk_pixmap_unref(source);
			// gdk_bitmap_unref(mask);
			return;
#else
		cursor_number = GDK_SB_H_DOUBLE_ARROW;
		break;

#endif
		}
	}

	GdkCursor * cursor = gdk_cursor_new(cursor_number);
	gdk_window_set_cursor(m_pWin, cursor);
	gdk_cursor_destroy(cursor);
}

void GR_UnixGraphics::createPixmapFromXPM( char ** pXPM,GdkPixmap *source,
										   GdkBitmap * mask)
{
	source
		= gdk_pixmap_colormap_create_from_xpm_d(m_pWin,NULL,
							&mask, NULL,
							pXPM);
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
	m_3dColors[CLR3D_BevelUp]    = pStyle->light[GTK_STATE_NORMAL];
	m_3dColors[CLR3D_BevelDown]  = pStyle->dark[GTK_STATE_NORMAL];
	m_3dColors[CLR3D_Highlight]  = pStyle->bg[GTK_STATE_PRELIGHT];
}

void GR_UnixGraphics::fillRect(GR_Color3D c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h)
{
	GR_CaretDisabler caretDisabler(getCaret());
	UT_ASSERT(c < COUNT_3D_COLORS);
	gdk_gc_set_foreground(m_pGC, &m_3dColors[c]);
	gdk_draw_rectangle(m_pWin, m_pGC, 1, _UD(x), _UD(y), _UD(w), _UD(h));
}

void GR_UnixGraphics::fillRect(GR_Color3D c, UT_Rect &r)
{
	UT_ASSERT(c < COUNT_3D_COLORS);
	fillRect(c,r.left,r.top,r.width,r.height);
}

void GR_UnixGraphics::polygon(UT_RGBColor& c,UT_Point *pts,UT_uint32 nPoints)
{
	GR_CaretDisabler caretDisabler(getCaret());
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
        points[i].x = _UD(pts[i].x);
        points[i].y = _UD(pts[i].y);
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

void GR_UnixGraphics::saveRectangle(UT_Rect & r)
{
  if (m_saveBuf)
    {
      g_object_unref(m_saveBuf); 
      m_saveBuf = NULL;
    }
  DELETEP(m_saveRect);

  if (r.top < 0 || r.height <= 0 || r.width <= 0.)
    return;
  
  m_saveBuf = gdk_pixbuf_get_from_drawable(m_saveBuf,
					   m_pWin,
					   NULL,
					   r.left, r.top, 0, 0,
					   r.width, r.height);
  m_saveRect = new UT_Rect(r);
}

void GR_UnixGraphics::restoreRectangle()
{
	if (m_saveBuf && m_saveRect)
		gdk_pixbuf_render_to_drawable(m_saveBuf,
									  m_pWin,
									  NULL, 
									  0, 0,
									  m_saveRect->left, m_saveRect->top,
									  -1, -1, GDK_RGB_DITHER_NONE, 0, 0);
}
