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
#include <math.h>

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
#include <X11/Xlib.h>
#include <X11/Xft/Xft.h>

static UT_uint32 adobeDUni[/*202*/][2] =
	{
		{0x0020,0x0020},
		{0x0021,0x2701},
		{0x0022,0x2702},
		{0x0023,0x2703},
		{0x0024,0x2704},
		{0x0025,0x260E},
		{0x0026,0x2706},
		{0x0027,0x2707},
		{0x0028,0x2708},
		{0x0029,0x2709},
		{0x002A,0x261B},
		{0x002B,0x261E},
		{0x002C,0x270C},
		{0x002D,0x270D},
		{0x002E,0x270E},
		{0x002F,0x270F},
		{0x0030,0x2710},
		{0x0031,0x2711},
		{0x0032,0x2712},
		{0x0033,0x2713},
		{0x0034,0x2714},
		{0x0035,0x2715},
		{0x0036,0x2716},
		{0x0037,0x2717},
		{0x0038,0x2718},
		{0x0039,0x2719},
		{0x003A,0x271A},
		{0x003B,0x271B},
		{0x003C,0x271C},
		{0x003D,0x271D},
		{0x003E,0x271E},
		{0x003F,0x271F},
		{0x0040,0x2720},
		{0x0041,0x2721},
		{0x0042,0x2722},
		{0x0043,0x2723},
		{0x0044,0x2724},
		{0x0045,0x2725},
		{0x0046,0x2726},
		{0x0047,0x2727},
		{0x0048,0x2605},
		{0x0049,0x2729},
		{0x004A,0x272A},
		{0x004B,0x272B},
		{0x004C,0x272C},
		{0x004D,0x272D},
		{0x004E,0x272E},
		{0x004F,0x272F},
		{0x0050,0x2730},
		{0x0051,0x2731},
		{0x0052,0x2732},
		{0x0053,0x2733},
		{0x0054,0x2734},
		{0x0055,0x2735},
		{0x0056,0x2736},
		{0x0057,0x2737},
		{0x0058,0x2738},
		{0x0059,0x2739},
		{0x005A,0x273A},
		{0x005B,0x273B},
		{0x005C,0x273C},
		{0x005D,0x273D},
		{0x005E,0x273E},
		{0x005F,0x273F},
		{0x0060,0x2740},
		{0x0061,0x2741},
		{0x0062,0x2742},
		{0x0063,0x2743},
		{0x0064,0x2744},
		{0x0065,0x2745},
		{0x0066,0x2746},
		{0x0067,0x2747},
		{0x0068,0x2748},
		{0x0069,0x2749},
		{0x006A,0x274A},
		{0x006B,0x274B},
		{0x006C,0x25CF},
		{0x006D,0x274D},
		{0x006E,0x25A0},
		{0x006F,0x274F},
		{0x0070,0x2750},
		{0x0071,0x2751},
		{0x0072,0x2752},
		{0x0073,0x25B2},
		{0x0074,0x25BC},
		{0x0075,0x25C6},
		{0x0076,0x2756},
		{0x0077,0x25D7},
		{0x0078,0x2758},
		{0x0079,0x2759},
		{0x007A,0x275A},
		{0x007B,0x275B},
		{0x007C,0x275C},
		{0x007D,0x275D},
		{0x007E,0x275E},
		{0x0080,0xF8D7},
		{0x0081,0xF8D8},
		{0x0082,0xF8D9},
		{0x0083,0xF8DA},
		{0x0084,0xF8DB},
		{0x0085,0xF8DC},
		{0x0086,0xF8DD},
		{0x0087,0xF8DE},
		{0x0088,0xF8DF},
		{0x0089,0xF8E0},
		{0x008A,0xF8E1},
		{0x008B,0xF8E2},
		{0x008C,0xF8E3},
		{0x008D,0xF8E4},
		{0x00A1,0x2761},
		{0x00A2,0x2762},
		{0x00A3,0x2763},
		{0x00A4,0x2764},
		{0x00A5,0x2765},
		{0x00A6,0x2766},
		{0x00A7,0x2767},
		{0x00A8,0x2663},
		{0x00A9,0x2666},
		{0x00AA,0x2665},
		{0x00AB,0x2660},
		{0x00AC,0x2460},
		{0x00AD,0x2461},
		{0x00AE,0x2462},
		{0x00AF,0x2463},
		{0x00B0,0x2464},
		{0x00B1,0x2465},
		{0x00B2,0x2466},
		{0x00B3,0x2467},
		{0x00B4,0x2468},
		{0x00B5,0x2469},
		{0x00B6,0x2776},
		{0x00B7,0x2777},
		{0x00B8,0x2778},
		{0x00B9,0x2779},
		{0x00BA,0x277A},
		{0x00BB,0x277B},
		{0x00BC,0x277C},
		{0x00BD,0x277D},
		{0x00BE,0x277E},
		{0x00BF,0x277F},
		{0x00C0,0x2780},
		{0x00C1,0x2781},
		{0x00C2,0x2782},
		{0x00C3,0x2783},
		{0x00C4,0x2784},
		{0x00C5,0x2785},
		{0x00C6,0x2786},
		{0x00C7,0x2787},
		{0x00C8,0x2788},
		{0x00C9,0x2789},
		{0x00CA,0x278A},
		{0x00CB,0x278B},
		{0x00CC,0x278C},
		{0x00CD,0x278D},
		{0x00CE,0x278E},
		{0x00CF,0x278F},
		{0x00D0,0x2790},
		{0x00D1,0x2791},
		{0x00D2,0x2792},
		{0x00D3,0x2793},
		{0x00D4,0x2794},
		{0x00D5,0x2192},
		{0x00D6,0x2194},
		{0x00D7,0x2195},
		{0x00D8,0x2798},
		{0x00D9,0x2799},
		{0x00DA,0x279A},
		{0x00DB,0x279B},
		{0x00DC,0x279C},
		{0x00DD,0x279D},
		{0x00DE,0x279E},
		{0x00DF,0x279F},
		{0x00E0,0x27A0},
		{0x00E1,0x27A1},
		{0x00E2,0x27A2},
		{0x00E3,0x27A3},
		{0x00E4,0x27A4},
		{0x00E5,0x27A5},
		{0x00E6,0x27A6},
		{0x00E7,0x27A7},
		{0x00E8,0x27A8},
		{0x00E9,0x27A9},
		{0x00EA,0x27AA},
		{0x00EB,0x27AB},
		{0x00EC,0x27AC},
		{0x00ED,0x27AD},
		{0x00EE,0x27AE},
		{0x00EF,0x27AF},
		{0x00F1,0x27B1},
		{0x00F2,0x27B2},
		{0x00F3,0x27B3},
		{0x00F4,0x27B4},
		{0x00F5,0x27B5},
		{0x00F6,0x27B6},
		{0x00F7,0x27B7},
		{0x00F8,0x27B8},
		{0x00F9,0x27B9},
		{0x00FA,0x27BA},
		{0x00FB,0x27BB},
		{0x00FC,0x27BC},
		{0x00FD,0x27BD},
		{0x00FE,0x27BE},
		{255,100000}
	};

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

extern UT_uint32 adobeToUnicode(UT_uint32 iAdobe)
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
	UT_sint32 slow = static_cast<UT_sint32>(iAdobe) - 72;
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

extern UT_uint32 adobeDingbatsToUnicode(UT_uint32 iAdobe)
{
	UT_uint32 low = adobeDUni[0][0];
	UT_uint32 high = adobeDUni[202][0];
	if(iAdobe < low)
	{
		return iAdobe;
	}
	if(iAdobe > high)
	{
		return iAdobe;
	}
	UT_sint32 slow = static_cast<UT_sint32>(iAdobe) - 32;
	if(slow < 0)
	{ 
		slow = 0;
	}
	while(adobeDUni[slow][0] != iAdobe && slow < 255)
	{
		xxx_UT_DEBUGMSG(("char at %d is %d value %d \n",slow,adobeDUni[slow][0],adobeSUni[slow][1]));
		slow++;
	}
	xxx_UT_DEBUGMSG(("Input %d return %d \n",iAdobe,adobeDUni[slow][1]));
	if(slow > 255)
	{
		return iAdobe;
	}
	return adobeDUni[slow][1];
}

const char* GR_Graphics::findNearestFont(const char* pszFontFamily,
										 const char* pszFontStyle,
										 const char* pszFontVariant,
										 const char* pszFontWeight,
										 const char* pszFontStretch,
										 const char* pszFontSize)
{

	XAP_UnixFont* pUnixFont = XAP_UnixFontManager::pFontManager->findNearestFont(pszFontFamily, pszFontStyle, pszFontVariant, pszFontWeight,
																				 pszFontStretch, pszFontSize);
	return pUnixFont->getName();
}

//
// Below this size we use GDK fonts. Above it we use metric info.
//
#define MAX_ABI_GDK_FONT_SIZE 200
#define FALLBACK_FONT_SIZE 12

UT_uint32 				GR_UnixGraphics::s_iInstanceCount = 0;

#ifndef WITH_PANGO
GR_UnixGraphics::GR_UnixGraphics(GdkWindow * win, XAP_UnixFontManager * fontManager, XAP_App * app)
#else
	GR_UnixGraphics::GR_UnixGraphics(GdkWindow * win, XAP_App * app)
#endif
		:
         m_iLineWidth(tlu(1))
{
	m_pApp = app;
	m_pWin = win;
#ifndef WITH_PANGO
	m_pFontManager = fontManager;
	m_pFont = NULL;
#endif
	m_pSingleByteFont = NULL;
	m_pMultiByteFont = NULL;
	m_pFontGUI = NULL;
	s_iInstanceCount++;
	m_pColormap = gdk_rgb_get_cmap(); // = gdk_colormap_get_system();

	//
	// Martin's attempt to make double buffering work.with xft
	//
	m_iXoff = 0;
	m_iYoff = 0;
	GdkDrawable * realDraw;
	gdk_window_get_internal_paint_info (m_pWin, &realDraw,&m_iXoff,&m_iYoff);
    m_iXoff = tlu(m_iXoff); m_iYoff = tlu(m_iYoff);
	m_pGC = gdk_gc_new(realDraw);
	m_pXORGC = gdk_gc_new(realDraw);
	m_pVisual = GDK_VISUAL_XVISUAL( gdk_drawable_get_visual(realDraw));
	m_Drawable = gdk_x11_drawable_get_xid(realDraw);

	m_pXftFontL = NULL;
	m_pXftFontD = NULL;
	m_Colormap = GDK_COLORMAP_XCOLORMAP(m_pColormap);
	m_pXftDraw = XftDrawCreate(GDK_DISPLAY(), m_Drawable, m_pVisual, m_Colormap);
	gdk_gc_set_function(m_pXORGC, GDK_XOR);

 	GdkColor clrWhite;
	gdk_color_white(m_pColormap, &clrWhite);
	gdk_gc_set_foreground(m_pXORGC, &clrWhite);

 	GdkColor clrBlack;
	gdk_color_black(m_pColormap, &clrBlack);
	gdk_gc_set_foreground(m_pGC, &clrBlack);

	m_XftColor.color.red = clrBlack.red;
	m_XftColor.color.green = clrBlack.green;
	m_XftColor.color.blue = clrBlack.blue;
	m_XftColor.color.alpha = 0xffff;
	m_XftColor.pixel = clrBlack.pixel;

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
	DELETEP (m_pFontGUI);

	/* WARNING: Don't use XftDrawDestroy.  XftDrawDestroy will also destroy the drawable */
	if (m_pXftDraw)
		free(m_pXftDraw);

#ifndef WITH_PANGO
	delete m_pFallBackFontHandle;
#endif

	UT_VECTOR_PURGEALL(UT_Rect*, m_vSaveRect);

	// purge saved pixbufs
	for (UT_uint32 i = 0; i < m_vSaveRectBuf.size (); i++)
		{
			GdkPixbuf * pix = static_cast<GdkPixbuf *>(m_vSaveRectBuf.getNthItem (i));
			g_object_unref (G_OBJECT (pix));
		}

	gdk_gc_unref (m_pGC);
	gdk_gc_unref (m_pXORGC);
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

void GR_UnixGraphics::setZoomPercentage(UT_uint32 iZoom)
{
	DELETEP (m_pFontGUI);
	GR_Graphics::setZoomPercentage (iZoom); // chain up
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

static GdkLineStyle mapLineStyle ( GdkGC* pGC, GR_Graphics::LineStyle in )
{
	switch ( in )
    {
		case GR_Graphics::LINE_ON_OFF_DASH :
			return GDK_LINE_ON_OFF_DASH ;
		case GR_Graphics::LINE_DOUBLE_DASH :
			return GDK_LINE_DOUBLE_DASH ;
		case GR_Graphics::LINE_DOTTED:
			{
				gint8 dash_list[2] = { 1, 2 }; // MARCM: I chose a pattern of 1 pixel on, 2 pixels off for dotted lines
				gdk_gc_set_dashes(pGC, 0, dash_list, 2);
			}
			return GDK_LINE_ON_OFF_DASH;
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

void GR_UnixGraphics::setLineProperties ( double inWidth, 
										  GR_Graphics::JoinStyle inJoinStyle,
										  GR_Graphics::CapStyle inCapStyle,
										  GR_Graphics::LineStyle inLineStyle )
{
	gdk_gc_set_line_attributes ( m_pGC, static_cast<gint>(tduD(inWidth)),
								 mapLineStyle ( m_pGC, inLineStyle ),
								 mapCapStyle ( inCapStyle ),
								 mapJoinStyle ( inJoinStyle ) ) ;
	gdk_gc_set_line_attributes ( m_pXORGC, static_cast<gint>(tduD(inWidth)),
								 mapLineStyle ( m_pGC, inLineStyle ),
								 mapCapStyle ( inCapStyle ),
								 mapJoinStyle ( inJoinStyle ) ) ;
}

void GR_UnixGraphics::drawGlyph(UT_uint32 Char, UT_sint32 xoff, UT_sint32 yoff)
{
	UT_uint32 iChar = Char;
	if(m_bIsSymbol && iChar < 255  && iChar >= 32)
	{
		iChar = adobeToUnicode(Char);
		xxx_UT_DEBUGMSG(("DrawGlyph 1 remapped %d to %d \n",Char,iChar));
	}
	if(m_bIsDingbat && iChar < 255  && iChar >= 32)
	{
		iChar = adobeDingbatsToUnicode(Char);
		UT_DEBUGMSG(("DrawGlyph 1 remapped %d to %d \n",Char,iChar));
	}
	
	// FIXME ascent in wrong unit
	XftDrawGlyphs(m_pXftDraw, &m_XftColor, m_pXftFontD, tdu(xoff + m_iXoff), tdu(m_pXftFontL->ascent * getResolution() / s_getDeviceResolution() + yoff + m_iYoff), &iChar, 1);
}

void GR_UnixGraphics::drawChars(const UT_UCSChar* pChars, int iCharOffset,
				int iLength, UT_sint32 xoff, UT_sint32 yoff,
				int * pCharWidths)
{
	xxx_UT_DEBUGMSG(("UnixGraphics:drawChars: m_bIsSymbol %d iLength %d \n",m_bIsSymbol,iLength));
	if (iLength == 0)
		return;
	// FIXME shouldn't need to do this - plam
	yoff += m_pXftFontL->ascent * getResolution() / s_getDeviceResolution();
	if (!pCharWidths)
	{
		if(!m_bIsSymbol && !m_bIsDingbat)
		{
			XftDrawString32(m_pXftDraw, &m_XftColor, m_pXftFontD, tdu(xoff + m_iXoff), tdu(yoff + m_iYoff),
							const_cast<XftChar32*> (pChars + iCharOffset), iLength);
		}
		else if(m_bIsSymbol)
		{
			xxx_UT_DEBUGMSG(("Doing draw symbols length %d offset %d \n",iLength,iCharOffset));
			UT_uint32 * uChars = new UT_uint32[iLength];
			for(UT_uint32 i = static_cast<UT_uint32>(iCharOffset); i< static_cast<UT_uint32>(iLength); i++)
			{
				uChars[i] = static_cast<UT_uint32>(pChars[iCharOffset + i]);
				if(uChars[i] < 255 && uChars[i] >= 32)
				{
					uChars[i] = adobeToUnicode(uChars[i]);
					xxx_UT_DEBUGMSG(("drawchars: mapped %d to %d \n",pChars[i],uChars[i]));
				}
			}
			XftDrawString32(m_pXftDraw, &m_XftColor, m_pXftFontD, tdu(xoff + m_iXoff), tdu(yoff + m_iYoff),
							const_cast<XftChar32*> (uChars), iLength);
			delete [] uChars;
		}
		else if(m_bIsDingbat)
		{
			UT_DEBUGMSG(("Doing draw symbols length %d offset %d \n",iLength,iCharOffset));
			UT_uint32 * uChars = new UT_uint32[iLength];
			for(UT_uint32 i = static_cast<UT_uint32>(iCharOffset); i< static_cast<UT_uint32>(iLength); i++)
			{
				uChars[i] = static_cast<UT_uint32>(pChars[iCharOffset + i]);
				if(uChars[i] < 255 && uChars[i] >= 32)
				{
					uChars[i] = adobeDingbatsToUnicode(uChars[i]);
					UT_DEBUGMSG(("drawchars: mapped %d to %d \n",pChars[i],uChars[i]));
				}
			}
			XftDrawString32(m_pXftDraw, &m_XftColor, m_pXftFontD, tdu(xoff + m_iXoff), tdu(yoff + m_iYoff),
							const_cast<XftChar32*> (uChars), iLength);
			delete [] uChars;
		}
	}
	else
	{
		UT_uint32 uChar;
		XftCharSpec aCharSpec[256];
		XftCharSpec* pCharSpec = aCharSpec;
		UT_sint32 currentYoff = tdu(yoff);

		if (iLength > 256)
			pCharSpec = new XftCharSpec[iLength];

		UT_uint32 xPos = xoff;
		for (int i = 0; i < iLength; ++i)
		{
			uChar = static_cast<UT_uint32>(pChars[i + iCharOffset]);
			if(m_bIsSymbol && uChar < 255 && uChar >=32)
			{
				pCharSpec[i].ucs4 = static_cast<FT_UInt>(adobeToUnicode(uChar));
				xxx_UT_DEBUGMSG(("DrawGlyph 2 remapped %d to %d \n",uChar,pCharSpec[i].ucs4));
			}
			else if(m_bIsDingbat && uChar < 255 && uChar >=32)
			{
				pCharSpec[i].ucs4 = static_cast<FT_UInt>(adobeDingbatsToUnicode(uChar));
				UT_DEBUGMSG(("DrawGlyph 2 remapped %d to %d \n",uChar,pCharSpec[i].ucs4));

			}
			else
			{
				pCharSpec[i].ucs4 = static_cast<FT_UInt>(pChars[i + iCharOffset]);
			}
			// we may have to keep a remainder and tdu all through, if
			// it turns out that we don't fit in a short.
			// or we can keep it in an int array, then transfer to shorts.
			// that's probably better.
			pCharSpec[i].x = tdu(xPos);
			pCharSpec[i].y = currentYoff;
			if (i < iLength - 1) {
				xPos += pCharWidths[i];
			}
		}
		
		XftDrawCharSpec (m_pXftDraw, &m_XftColor, m_pXftFontD, pCharSpec, iLength);

		if (pCharSpec != aCharSpec)
			delete[] pCharSpec;
	}
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
	
	if(m_pFont && (pUFont->getUnixFont() == m_pFont->getUnixFont()) &&
	   (pUFont->getSize() == m_pFont->getSize()))
		return;
	m_bIsSymbol = false;
	m_bIsDingbat = false;

	m_pFont = pUFont;
	char * szUnixFontName = UT_strdup(m_pFont->getUnixFont()->getName());
	const char * szFontName = UT_lowerString(szUnixFontName);

	if (szFontName)
	{
		if(strstr(szFontName,"symbol") != NULL)
		{
			if(strstr(szFontName,"star") != NULL)
				m_bIsSymbol = false;
			else
				m_bIsSymbol = true;
		}
		if(strstr(szFontName,"dingbat") != NULL)
			m_bIsDingbat = true;
	}
	FREEP(szFontName);
	m_pXftFontL = m_pFont->getLayoutXftFont();
	m_pXftFontD = m_pFont->getDeviceXftFont(getZoomPercentage());
}

UT_uint32 GR_UnixGraphics::getFontHeight(GR_Font * fnt)
{
	return getFontAscent(fnt)+getFontDescent(fnt);
}

void GR_UnixGraphics::getCoverage(UT_Vector& coverage)
{
	m_pFont->getUnixFont()->getCoverage(coverage);
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

	UT_UCSChar newChar;
	double fWidth;

	if(!m_bIsSymbol && !m_bIsDingbat)
	{
		newChar = c;
	}
	else if(m_bIsSymbol)
	{
		newChar = static_cast<UT_UCSChar>(adobeToUnicode(c));
	}
	else
	{
		newChar = static_cast<UT_UCSChar>(adobeDingbatsToUnicode(c));
	}
	// FIXME we should really be getting stuff fromt he font in layout units,
	// FIXME but we're not smart enough to do that yet

	fWidth = m_pFont->measureUnRemappedChar(newChar, m_pFont->getSize())
		* ((double)getResolution() / (double)s_getDeviceResolution());
	return static_cast<UT_uint32>(rint(fWidth));
}

UT_uint32 GR_UnixGraphics::s_getDeviceResolution(void)
{
	// this is hard-coded at 96 for X now, since 72 (which
	// most X servers return when queried for a resolution)
	// makes for tiny fonts on modern resolutions.

	return 72;
}

UT_uint32 GR_UnixGraphics::getDeviceResolution(void) const
{
	return s_getDeviceResolution();
}

void GR_UnixGraphics::getColor(UT_RGBColor& clr)
{
	clr = m_curColor;
}

void GR_UnixGraphics::setColor(const UT_RGBColor& clr)
{
	UT_ASSERT(m_pGC);
	GdkColor c;

	if (m_curColor == clr)
		return;

	m_curColor = clr;
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

	m_XftColor.color.red = c.red;
	m_XftColor.color.green = c.green;
	m_XftColor.color.blue = c.blue;
	m_XftColor.color.alpha = 0xffff;
	m_XftColor.pixel = c.pixel;
	
	/* Set up the XOR gc */
	gdk_gc_set_foreground(m_pXORGC, &c);
	gdk_gc_set_function(m_pXORGC, GDK_XOR);
}

GR_Font * GR_UnixGraphics::getGUIFont(void)
{
#ifndef WITH_PANGO
	if (!m_pFontManager)
		return NULL;

	if (!m_pFontGUI)
	{

		// get the font resource
		GtkStyle *tempStyle = gtk_style_new();
		const char *guiFontName = pango_font_description_get_family(tempStyle->font_desc);
		XAP_UnixFont * font = static_cast<XAP_UnixFont *>(m_pFontManager->searchFont(guiFontName)); // this operation should never fail
		g_object_unref(G_OBJECT(tempStyle));
		UT_ASSERT(font);

		// bury it in a new font handle
		// Hardcoded GUI font size guaranteed to be 12 device units tall
		m_pFontGUI = new XAP_UnixFontHandle(font, static_cast<UT_uint32>(12*100.0/getZoomPercentage()));
		UT_ASSERT(m_pFontGUI);
	}
#endif
	// TODO provide PANGO implementation
	return m_pFontGUI;
}

#ifndef WITH_PANGO
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
	UT_uint32 iSize = static_cast<UT_uint32>(UT_convertToPoints(pszFontSize));
	XAP_UnixFontHandle* pFont = new XAP_UnixFontHandle(pUnixFont, iSize);
	UT_ASSERT(pFont);

	return pFont;
}

GR_Font* GR_UnixGraphics::getDefaultFont(UT_String& fontFamily)
{
	static XAP_UnixFontHandle fontHandle(m_pFontManager->getDefaultFont(), 12);
	fontFamily = fontHandle.getUnixFont()->getName();
	
	return &fontHandle;
}

UT_uint32 GR_UnixGraphics::getFontAscent(GR_Font * fnt)
{
	// should return a value in layout units, independent of the zoom.
	// ideally, the font should return something appropriate to us.

	UT_ASSERT(fnt);
	UT_ASSERT(m_pGC);

	XAP_UnixFontHandle * hndl = static_cast<XAP_UnixFontHandle *>(fnt);
	
	// FIXME we should really be getting stuff fromt he font in layout units,
	// FIXME but we're not smart enough to do that yet
    // we call s_getDeviceResolution() to avoid zoom
	return static_cast<UT_uint32>(hndl->getUnixFont()->getAscender(hndl->getSize()) * getResolution() / s_getDeviceResolution() + 0.5);
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

	XAP_UnixFont* pFont = hndl->getUnixFont();
	// FIXME we should really be getting stuff fromt he font in layout units,
	// FIXME but we're not smart enough to do that yet
	return static_cast<UT_uint32>(pFont->getDescender(hndl->getSize()) * getResolution() / getDeviceResolution() + 0.5);
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
	gdk_draw_line(m_pWin, m_pGC, tdu(x1), tdu(y1), tdu(x2), tdu(y2));
}

void GR_UnixGraphics::setLineWidth(UT_sint32 iLineWidth)
{
	m_iLineWidth = tdu(iLineWidth);

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
	gdk_draw_line(m_pWin, m_pXORGC, tdu(x1), tdu(y1), tdu(x2), tdu(y2));
}

void GR_UnixGraphics::polyLine(UT_Point * pts, UT_uint32 nPoints)
{
	GR_CaretDisabler caretDisabler(getCaret());

	// see bug #303 for what this is about

	GdkPoint * points = static_cast<GdkPoint *>(calloc(nPoints, sizeof(GdkPoint)));
	UT_ASSERT(points);

	for (UT_uint32 i = 0; i < nPoints; i++)
	{
		points[i].x = tdu(pts[i].x);
		// It seems that Windows draws each pixel along the the Y axis
		// one pixel beyond where GDK draws it (even though both coordinate
		// systems start at 0,0 (?)).  Subtracting one clears this up so
		// that the poly line is in the correct place relative to where
		// the rest of GR_UnixGraphics:: does things (drawing text, clearing
		// areas, etc.).
		points[i].y = tdu(pts[i].y) - 1;
	}

	gdk_draw_lines(m_pWin, m_pGC, points, nPoints);

	FREEP(points);
}

void GR_UnixGraphics::invertRect(const UT_Rect* pRect)
{
	GR_CaretDisabler caretDisabler(getCaret());
	UT_ASSERT(pRect);
	gdk_draw_rectangle(m_pWin, m_pXORGC, 1, tdu(pRect->left), tdu(pRect->top),
			   tdu(pRect->width), tdu(pRect->height));
}

void GR_UnixGraphics::setClipRect(const UT_Rect* pRect)
{
	m_pRect = pRect;
	if (pRect)
	{
		GdkRectangle r;

		r.x = tdu(pRect->left);
		r.y = tdu(pRect->top);
		r.width = tdu(pRect->width);
		r.height = tdu(pRect->height);

		gdk_gc_set_clip_rectangle(m_pGC, &r);
		gdk_gc_set_clip_rectangle(m_pXORGC, &r);
		Region region;
		XPoint points[4];
	
		points[0].x = r.x + tdu(m_iXoff);
		points[0].y = r.y + tdu(m_iYoff);
			
		points[1].x = r.x + r.width  + tdu(m_iXoff);
		points[1].y = r.y + tdu(m_iYoff);
			
		points[2].x = r.x + r.width  + tdu(m_iXoff);
		points[2].y = r.y + r.height + tdu(m_iYoff);
			
		points[3].x = r.x  + tdu(m_iXoff);
		points[3].y = r.y + r.height + tdu(m_iYoff);

		xxx_UT_DEBUGMSG(("Setting clipping rectangle: (%d, %d, %d, %d)\n", r.x, r.y, r.width, r.height));
		region = XPolygonRegion(points, 4, EvenOddRule);
		if (region)
		{
			XftDrawSetClip(m_pXftDraw, region);
			XDestroyRegion (region);
		}
	}
	else
	{
		gdk_gc_set_clip_rectangle(m_pGC, NULL);
		gdk_gc_set_clip_rectangle(m_pXORGC, NULL);

		xxx_UT_DEBUGMSG(("Setting clipping rectangle NULL\n"));
		XftDrawSetClip(m_pXftDraw, 0);
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

 	gdk_draw_rectangle(m_pWin, m_pGC, 1, tdu(x), tdu(y), tdu(w), tdu(h));

	gdk_gc_set_foreground(m_pGC, &oColor);
}

void GR_UnixGraphics::scroll(UT_sint32 dx, UT_sint32 dy)
{
	GR_CaretDisabler caretDisabler(getCaret());
	gdk_window_scroll(m_pWin,-tdu(dx),-tdu(dy));
}

void GR_UnixGraphics::scroll(UT_sint32 x_dest, UT_sint32 y_dest,
						  UT_sint32 x_src, UT_sint32 y_src,
						  UT_sint32 width, UT_sint32 height)
{
	GR_CaretDisabler caretDisabler(getCaret());
	gdk_window_copy_area(m_pWin, m_pGC, tdu(x_dest), tdu(y_dest),
			     m_pWin, tdu(x_src), tdu(y_src), tdu(width), tdu(height));
}

void GR_UnixGraphics::clearArea(UT_sint32 x, UT_sint32 y,
				UT_sint32 width, UT_sint32 height)
{
	GR_CaretDisabler caretDisabler(getCaret());

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

GR_Image* GR_UnixGraphics::createNewImage(const char* pszName, const UT_ByteBuf* pBB, UT_sint32 iWidth, UT_sint32 iHeight, GR_Image::GRType iType)
{
   	GR_Image* pImg = NULL;

	pImg = new GR_UnixImage(pszName);
	pImg->convertFromBuffer(pBB, tdu(iWidth), tdu(iHeight));
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

   	UT_sint32 iImageWidth = pUnixImage->getDisplayWidth();
   	UT_sint32 iImageHeight = pUnixImage->getDisplayHeight();

	xDest = tdu(xDest); yDest = tdu(yDest);

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
		cursor_number = GDK_LEFT_PTR;
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
		cursor_number = GDK_SB_V_DOUBLE_ARROW;
		break;

	case GR_CURSOR_VLINE_DRAG:
		cursor_number = GDK_SB_H_DOUBLE_ARROW;
		break;
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
	gdk_draw_rectangle(m_pWin, m_pGC, 1, tdu(x), tdu(y), tdu(w), tdu(h));
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
        points[i].x = tdu(pts[i].x);
        points[i].y = tdu(pts[i].y);
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

void GR_UnixGraphics::saveRectangle(UT_Rect & r, UT_uint32 iIndx)
{
	void * oldR = NULL;
	m_vSaveRect.setNthItem(iIndx, static_cast<void*>(new UT_Rect(r)),&oldR);
	if(oldR)
		delete static_cast<UT_Rect*>(oldR);

	void * oldC = NULL;
	GdkPixbuf * pix = gdk_pixbuf_get_from_drawable(NULL,
												   m_pWin,
												   NULL,
												   tdu(r.left), tdu(r.top), 0, 0,
												   tdu(r.width), tdu(r.height));
	m_vSaveRectBuf.setNthItem(iIndx, static_cast<void*>(pix), &oldC);

	if(oldC)
		g_object_unref (G_OBJECT (oldC));
}

void GR_UnixGraphics::restoreRectangle(UT_uint32 iIndx)
{
	UT_Rect * r = static_cast<UT_Rect*>(m_vSaveRect.getNthItem(iIndx));
	GdkPixbuf *p = static_cast<GdkPixbuf *>(m_vSaveRectBuf.getNthItem(iIndx));

	if (p && r)
		gdk_pixbuf_render_to_drawable(p,
									  m_pWin,
									  NULL, 
									  0, 0,
									  tdu(r->left), tdu(r->top),
									  -1, -1, GDK_RGB_DITHER_NONE, 0, 0);
}
