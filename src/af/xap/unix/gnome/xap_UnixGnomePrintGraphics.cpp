/* -*- Mode: C; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* AbiSource Application Framework
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

#include <stdio.h>
#include <string.h>
#include <glib.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_misc.h"
#include "xap_Strings.h"
#include "xap_UnixGnomePrintGraphics.h"
#include "xap_UnixPSImage.h"

#include <libgnomeprint/gnome-print-master-preview.h>

// the resolution that we report to the application (pixels per inch).
#define OUR_LINE_LIMIT          200 /* FIXME:
				       UGLY UGLY UGLY, but we need to fix the PrintPrivew
				       show bug. Test it with something like 7 when to see
				       what is the problem with PP. The show operator in
				       print preview does not move th currentpoint. Chema */
#define GPG_RESOLUTION		7200
#define GPG_DEFAULT_FONT        "NimbusSanL"

/***********************************************************************/
/*      map abi's fonts to gnome fonts, or at least try to             */
/***********************************************************************/

typedef struct _fontMapping 
{
        char *abi;   // what abiword calls a font's name
        char *gnome; // what gnome refers to it as (or a close substitute)
};

// mapping of the fonts that abi ships with
// to what gnome-font ships with
/* The ones with __ have not been verified. (Chema) */
static struct _fontMapping fontMappingTable[] = 
{
        {"Arial",                  "__AvantGarde"},
	{"Bitstream",              "__Palatino"},
	{"Bookman",                "ITC Bookman"},
	{"Courier",                "Courier"},
	{"Courier New",            "Courier"}, /* ??? not really. (I think)*/
	{"Century Schoolbook",     "Century Schoolbook L"},
	{"Dingbats",               "Dingbats"},
	{"Goth",                   "URW Gothic L"},
	{"Helvetica",              "Helvetica"},
	{"Helvetic",               "Helvetica"},
	{"Nimbus Sans",            "Nimbus Sans L"},
	{"Nimbus Sans Condensed",  "__NimbusSanL"},
	{"Nimbus Roman",           "Nimbus Roman No9 L"},
	{"Nimbus Mono",            "Nimbus Mono L"},
	{"Palladio",               "URW Palladio L"},
	{"Symbol",                 "Symbol"},
	{"Standard Symbols",       "Standard Symbols L"},
	{"Times",                  "Times"},
	{"Times New Roman",        "Times"},
	{"*",                      GPG_DEFAULT_FONT}
};

#define TableSize	((sizeof(fontMappingTable)/sizeof(fontMappingTable[0])))

static char * mapFontName(const char *name)
{
        unsigned int idx = 0;

	// if we're passed crap, default to some normal font
	if(!name || !*name)
	  {
	        UT_DEBUGMSG(("Dom: mapFontName: null name, returning default\n"));
	        return GPG_DEFAULT_FONT;
	  }

	for (unsigned int k=0; k<TableSize; k++)
	  {
		if (fontMappingTable[k].abi[0] == '*')
			idx = k;
		else if (!UT_strnicmp(fontMappingTable[k].abi,name, 
				      strlen(fontMappingTable[k].abi)))
		  {
		    idx = k;
		    break;
		  }
	  }

	// return the gnome mapping
	return fontMappingTable[idx].gnome;
}

static UT_Bool isItalic(XAP_UnixFont::style s)
{
        return ((s == XAP_UnixFont::STYLE_ITALIC) || (s == XAP_UnixFont::STYLE_BOLD_ITALIC));
}

static GnomeFontWeight getGnomeFontWeight(XAP_UnixFont::style s)
{
        GnomeFontWeight w = GNOME_FONT_BOOK;
	switch((int)s)
	  {
	  case XAP_UnixFont::STYLE_BOLD_ITALIC:
	  case XAP_UnixFont::STYLE_BOLD:
	    w = GNOME_FONT_BOLD;
	  default:
	    break;
	  }

	return w;
}

#define FONT_NAMES_MATCH(gf, n) (!strcasecmp(gnome_font_get_ps_name((gf)), n))

GnomeFont * XAP_UnixGnomePrintGraphics::_allocGnomeFont(PSFont* pFont)
{
        XAP_UnixFont *uf          = pFont->getUnixFont();
	XAP_UnixFont::style style = uf->getStyle();
	char *abi_name            = (char*)uf->getName();
	
	double size         = pFont->getSize() * _scale_factor_get ();
	GnomeFontWeight gfw = getGnomeFontWeight(style);
	UT_Bool italic      = isItalic(style);
	
	// first try to directly allocate abi's name
	// this is good for fonts not in my table, and
	// fonts installed by the user in both Abi and
	// using gnome-font-install

	GnomeFont *tmp = NULL;

	/* gnome_font_new_closest always returns a font. 
	   So you will get helvetica if not found */
#if 0
	tmp      = gnome_font_new_closest(abi_name, gfw, italic, size);

	// assert that the fonts match
	if(FONT_NAMES_MATCH(tmp, abi_name))
		return tmp;

	UT_DEBUGMSG(("Dom: unreffing gnome font: ('%s','%s')\n", 
		     gnome_font_get_ps_name(tmp), abi_name));

	// else we got something we didn't ask for
	// unref the gnome-font and try again
	gnome_font_unref(tmp);
#endif

	char *fontname      = mapFontName(abi_name);
	tmp = gnome_font_new_closest(fontname, gfw, italic, size);

#if 0
	if(!FONT_NAMES_MATCH(tmp, fontname))
	  {
	    // ok, that failed. maybe put up a messagebox reporting our
	    // failure? Anyway, try to allocate our default font
	    // with their given size,weight, and italic settings
	    gnome_font_unref(tmp);

	    UT_DEBUGMSG(("Couldn't allocate %s, defaulting to '%s'\n", fontname, GPG_DEFAULT_FONT));
	    tmp = gnome_font_new_closest (GPG_DEFAULT_FONT, gfw, italic, size);
	  }
#endif
	
	return tmp;
}

/***********************************************************************/
/*      This file provides an interface into Gnome Print               */
/***********************************************************************/

XAP_UnixGnomePrintGraphics::~XAP_UnixGnomePrintGraphics()
{
        // nothing
}

XAP_UnixGnomePrintGraphics::XAP_UnixGnomePrintGraphics(GnomePrintMaster *gpm,
						       XAP_UnixFontManager * fontManager,
						       XAP_App *pApp,
						       UT_Bool isPreview)
{
        m_pApp         = pApp;
	m_gpm          = gpm;
	m_gpc          = gnome_print_master_get_context(gpm);

	const GnomePaper * paper = gnome_paper_with_name("US-Letter");
	
	// probably not what we want, but don't be picky
	if(!paper)
		paper = gnome_paper_with_name(gnome_paper_name_default());
	UT_ASSERT(paper);

	m_paper = paper;
	gnome_print_master_set_paper(m_gpm, paper);

	m_bIsPreview   = isPreview;
	m_fm           = fontManager;
	m_bStartPrint  = UT_FALSE;
	m_bStartPage   = UT_FALSE;
	m_pCurrentFont = NULL;

	m_cs = GR_Graphics::GR_COLORSPACE_COLOR;

	m_currentColor.m_red = 0;
	m_currentColor.m_grn = 0;
	m_currentColor.m_blu = 0;
}

UT_uint32 XAP_UnixGnomePrintGraphics::measureUnRemappedChar(const UT_UCSChar c)
{
	int size;
	
        UT_ASSERT(m_pCurrentFont);
	if (c >= 256)
		return 0;

	/* Use get glyph width intead ... Chema */
	size = (int) ( _scale_factor_get_inverse () *
		       gnome_font_get_width_string_n (m_pCurrentFont, (const char *)&c, 1) );
		
	return size;
}

void XAP_UnixGnomePrintGraphics::drawChars(const UT_UCSChar* pChars, 
				   int iCharOffset, int iLength,
				   UT_sint32 xoff, UT_sint32 yoff)
{
	UT_ASSERT(m_pCurrentFont);

	// The GR classes are expected to take yoff as the upper-left of
	// each glyph.  PostScript interprets the yoff as the baseline,
	// which doesn't match this expectation.  Adding the ascent of the
	// font will bring it back to the correct position.
	yoff += getFontAscent();

	// unsigned buffer holds Latin-1 data to character code 255
	unsigned char buf[OUR_LINE_LIMIT*2];
	unsigned char * pD = buf;

	const UT_UCSChar * pS = pChars+iCharOffset;
	const UT_UCSChar * pEnd = pS+iLength;
	UT_UCSChar currentChar;

	gnome_print_moveto(m_gpc, _scale_x_dir(xoff), _scale_y_dir(yoff));

	while (pS<pEnd)
	{
#if 1 /* FIXME: I don't fully like this ... Chema. */
		if (pD-buf > OUR_LINE_LIMIT)
		{
			*pD++ = 0;
			gnome_print_show(m_gpc, (const gchar *)buf);
			pD = buf;
		}
#endif	

		// TODO deal with Unicode issues.
		currentChar = remapGlyph(*pS, *pS >= 256 ? UT_TRUE : UT_FALSE);
		switch (currentChar)
		{
		default:		*pD++ = (unsigned char)currentChar; 	break;
		}
		pS++;
	}
	*pD++ = 0;

	gnome_print_show(m_gpc, (const gchar *)buf);
}

void XAP_UnixGnomePrintGraphics::drawLine(UT_sint32 x1, UT_sint32 y1,
				  UT_sint32 x2, UT_sint32 y2)
{
#if 0
	/* Chema: how about we just set this in setLineWidth() - Dom */
	double real_width;
	real_width  = ((double) m_iLineWidth * _scale_factor_get ());
	
	UT_DEBUGMSG(("Dom: drawLine (%f,%f) (%f, %f) width- %f\n", 
		     _scale_x_dir(x1), _scale_y_dir(y1),
		     _scale_x_dir(x2), _scale_y_dir(y2), m_iLineWidth));
#endif

        gnome_print_setlinewidth(m_gpc, m_dLineWidth); 
	gnome_print_moveto(m_gpc, _scale_x_dir(x1), _scale_y_dir(y1));
	gnome_print_lineto(m_gpc, _scale_x_dir(x2), _scale_y_dir(y2));
	gnome_print_stroke(m_gpc);

	/* Dom: you don't wanna close the path, you only need to
	   stroke it [ hhhuu hhuiuuu hhuu. Hey beavis, yes said "stroke it" ]*/
}

void XAP_UnixGnomePrintGraphics::setFont(GR_Font* pFont)
{
	UT_ASSERT(pFont);
	PSFont * psFont = (static_cast<PSFont*> (pFont));

	/* FIXME, don't call allocGnomeFont for every font
	   request. Keep a hash of psfonts->GnomeFonts.
	   Ref them when we ask for them. And unref them
	   when we stop using them. Chema */
	m_pCurrentFont = _allocGnomeFont(psFont);
        gnome_print_setfont (m_gpc, m_pCurrentFont);
}

UT_Bool XAP_UnixGnomePrintGraphics::queryProperties(GR_Graphics::Properties gp) const
{
	switch (gp)
	{
	case DGP_SCREEN:
		return UT_FALSE;
	case DGP_PAPER:
		return UT_TRUE;
	default:
		UT_ASSERT(0);
		return UT_FALSE;
	}
}

void XAP_UnixGnomePrintGraphics::setColor(UT_RGBColor& clr)
{
	if (clr.m_red == m_currentColor.m_red &&
	    clr.m_grn == m_currentColor.m_grn &&
	    clr.m_blu == m_currentColor.m_blu)
		return;
	
	// NOTE : we always set our color to something RGB, even if the color
	// NOTE : space is b&w or grayscale
	m_currentColor.m_red = clr.m_red;
	m_currentColor.m_grn = clr.m_grn;
	m_currentColor.m_blu = clr.m_blu;

	UT_DEBUGMSG(("Dom: setColor\n"));

	gnome_print_setrgbcolor(m_gpc,
				(int)(m_currentColor.m_red / 255),
				(int)(m_currentColor.m_grn / 255),
				(int)(m_currentColor.m_blu / 255));
	
}

void XAP_UnixGnomePrintGraphics::setLineWidth(UT_sint32 iLineWidth)
{
 	m_dLineWidth = (double)((double)iLineWidth * _scale_factor_get());
}

UT_Bool XAP_UnixGnomePrintGraphics::startPrint(void)
{
        UT_ASSERT(!m_bStartPrint);
	m_bStartPrint = UT_TRUE;
	return _startDocument();
}

UT_Bool XAP_UnixGnomePrintGraphics::startPage(const char * szPageLabel)
{
	if (m_bStartPage)
	  _endPage();
	m_bStartPage = UT_TRUE;
	m_bNeedStroked = UT_FALSE;
	return _startPage(szPageLabel);
}

UT_Bool XAP_UnixGnomePrintGraphics::startPage (const char *szPageLabel, 
				       unsigned int, unsigned char, 
				       unsigned int, unsigned int)
{
        return startPage(szPageLabel);
}

UT_Bool XAP_UnixGnomePrintGraphics::endPrint()
{
	if (m_bStartPage)
	  _endPage();
	return _endDocument();
}

void XAP_UnixGnomePrintGraphics::setColorSpace(GR_Graphics::ColorSpace c)
{
	m_cs = c;
}

GR_Graphics::ColorSpace XAP_UnixGnomePrintGraphics::getColorSpace(void) const
{
	return m_cs;
}


void XAP_UnixGnomePrintGraphics::drawAnyImage (GR_Image* pImg, 
				       UT_sint32 xDest, UT_sint32 yDest, UT_Bool rgb)
{
	UT_sint32 iDestWidth  = pImg->getDisplayWidth();
	UT_sint32 iDestHeight = pImg->getDisplayHeight();
	
	PS_Image * pPSImage = static_cast<PS_Image *>(pImg);

	PSFatmap * image = pPSImage->getData();

	UT_ASSERT(image && image->data);

	gnome_print_gsave(m_gpc);
	gnome_print_translate(m_gpc,
			      _scale_x_dir(xDest),
			      _scale_y_dir(yDest + pImg->getDisplayHeight()));
	gnome_print_scale(m_gpc,
			  ((double) iDestWidth)  * _scale_factor_get (),
			  ((double) iDestHeight) * _scale_factor_get ());

	/* TODO: we horribly crash (including the PS version) 
	   TODO: if the image we try to print has alpha
	   TODO: channels in it, like lots of pngs do. Detect 
	   TODO: this and call gnome_print_rgbaimage instead */
	if (rgb)
		gnome_print_rgbimage(m_gpc, (const gchar*)image->data, image->width, 
				     image->height, (UT_sint32) (image->width) * 3);
	else
		gnome_print_grayimage(m_gpc, (const gchar*)image->data, image->width, 
				      image->height, (UT_sint32) (image->width) * 1);
	gnome_print_grestore(m_gpc);
}

void XAP_UnixGnomePrintGraphics::drawImage(GR_Image* pImg, UT_sint32 xDest, 
				   UT_sint32 yDest)
{
   	if (pImg->getType() != GR_Image::GRT_Raster) {
	   pImg->render(this, xDest, yDest);
	   return;
	}
   
   	switch(m_cs)
     	{
       	case GR_Graphics::GR_COLORSPACE_COLOR:
		drawAnyImage(pImg, xDest, yDest, UT_TRUE);
		break;
      	case GR_Graphics::GR_COLORSPACE_GRAYSCALE:
		drawAnyImage(pImg, xDest, yDest, UT_FALSE);
		break;
      	case GR_Graphics::GR_COLORSPACE_BW:
		drawAnyImage(pImg, xDest, yDest, UT_FALSE);
		break;
      	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
     }
}

GR_Image* XAP_UnixGnomePrintGraphics::createNewImage(const char* pszName, 
					     const UT_ByteBuf* pBB, 
					     UT_sint32 iDisplayWidth,
					     UT_sint32 iDisplayHeight, 
					     GR_Image::GRType iType)
{
	GR_Image* pImg = NULL;
   
   	if (iType == GR_Image::GRT_Raster)
     		pImg = new PS_Image(pszName);
   	else if (iType == GR_Image::GRT_Vector)
     		pImg = new GR_VectorImage(pszName);
   
	pImg->convertFromBuffer(pBB, iDisplayWidth, iDisplayHeight);

	return pImg;
}

/***********************************************************************/
/*                          Protected things                           */
/***********************************************************************/
UT_Bool	XAP_UnixGnomePrintGraphics::_startDocument(void)
{
        return UT_TRUE;
}

UT_Bool XAP_UnixGnomePrintGraphics::_startPage(const char * szPageLabel)
{
        gnome_print_beginpage(m_gpc, szPageLabel);
	return UT_TRUE;
}

UT_Bool XAP_UnixGnomePrintGraphics::_endPage(void)
{
	if(m_bNeedStroked)
	  gnome_print_stroke(m_gpc);

	gnome_print_showpage(m_gpc);
	return UT_TRUE;
}

UT_Bool XAP_UnixGnomePrintGraphics::_endDocument(void)
{
	// bonobo version, no gnome-print-master
	if(!m_gpm)
	  return UT_TRUE;

        gnome_print_master_close(m_gpm);

	if(!m_bIsPreview)
	  {
	    gnome_print_master_print(m_gpm);
	  }
	else
	  {
	    GnomePrintMasterPreview *preview;
	    const XAP_StringSet * pSS = m_pApp->getStringSet();

	    // TODO: translate me
	    preview = gnome_print_master_preview_new(m_gpm, pSS->getValue(XAP_STRING_ID_DLG_UP_PrintPreviewTitle));
	    gtk_widget_show(GTK_WIDGET(preview));
	  }
	
	gtk_object_unref(GTK_OBJECT(m_gpm));
	return UT_TRUE;
}

UT_uint32 XAP_UnixGnomePrintGraphics::_getResolution(void) const
{
        return GPG_RESOLUTION;
}


/***********************************************************************/
/*                    Things that souldn't happen                      */
/***********************************************************************/
void XAP_UnixGnomePrintGraphics::setCursor(GR_Graphics::Cursor)
{
        // nada
}

GR_Graphics::Cursor XAP_UnixGnomePrintGraphics::getCursor(void) const
{
	return GR_CURSOR_INVALID;
}

void XAP_UnixGnomePrintGraphics::xorLine(UT_sint32, UT_sint32, UT_sint32, 
				 UT_sint32)
{
}

void XAP_UnixGnomePrintGraphics::polyLine(UT_Point * /* pts */, 
				  UT_uint32 /* nPoints */)
{
        // only used by us for printing red squiggly lines
        // in the spell-checker
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

void XAP_UnixGnomePrintGraphics::fillRect(UT_RGBColor& /*c*/, UT_sint32 /*x*/, 
				  UT_sint32 /*y*/, UT_sint32 /*w*/, 
				  UT_sint32 /*h*/)
{
        // nada
}

void XAP_UnixGnomePrintGraphics::fillRect(UT_RGBColor& /*c*/, UT_Rect & /*r*/)
{
        // nada
}

void XAP_UnixGnomePrintGraphics::invertRect(const UT_Rect* /*pRect*/)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

void XAP_UnixGnomePrintGraphics::setClipRect(const UT_Rect* /*pRect*/)
{
        // can ps print this?
}

void XAP_UnixGnomePrintGraphics::clearArea(UT_sint32 /*x*/, UT_sint32 /*y*/,
				   UT_sint32 /*width*/, UT_sint32 /*height*/)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

void XAP_UnixGnomePrintGraphics::scroll(UT_sint32, UT_sint32)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

void XAP_UnixGnomePrintGraphics::scroll(UT_sint32 /* x_dest */,
				UT_sint32 /* y_dest */,
				UT_sint32 /* x_src */,
				UT_sint32 /* y_src */,
				UT_sint32 /* width */,
				UT_sint32 /* height */)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

UT_RGBColor * XAP_UnixGnomePrintGraphics::getColor3D(GR_Color3D /*c*/)
{
        UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return NULL;
}

void XAP_UnixGnomePrintGraphics::setColor3D(GR_Color3D /*c*/)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

void XAP_UnixGnomePrintGraphics::fillRect(GR_Color3D /*c*/, UT_sint32 /*x*/, UT_sint32 /*y*/, UT_sint32 /*w*/, UT_sint32 /*h*/)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

void XAP_UnixGnomePrintGraphics::fillRect(GR_Color3D /*c*/, UT_Rect & /*r*/)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

GR_Font* XAP_UnixGnomePrintGraphics::getGUIFont()
{
        UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return NULL;
}

/***********************************************************************/
/*                                 Done                                */
/***********************************************************************/

/* This is somewhat broken. :-) 
   this function does not expect in return the font ascent,
   it expects the font bbox.ury. Chema */
UT_uint32 XAP_UnixGnomePrintGraphics::getFontAscent()
{
	const GnomeFontFace *face;
	const ArtDRect *bbox;
	GnomeFont *font;
	UT_uint32 asc;

	font = m_pCurrentFont;
		
        UT_ASSERT(GNOME_IS_FONT (font));
	face = gnome_font_get_face (font);
        UT_ASSERT(GNOME_IS_FONT_FACE (face));
	bbox = 	gnome_font_face_get_stdbbox (face);

	asc = (gint) (bbox->y1 * gnome_font_get_size (font) / 10);
	/* Hach so that we can compare the non-gnome output with
	   this one */
	asc = (gint) (((double) 915.0) * gnome_font_get_size (font) / 10);

//	UT_DEBUGMSG(("Font \"Ascent\" %i [%g, %g]\n", asc, bbox->y1,
//		     gnome_font_get_size (font)));

	return asc;
}

UT_uint32 XAP_UnixGnomePrintGraphics::getFontDescent()
{
	const GnomeFontFace *face;
	const ArtDRect *bbox;
	GnomeFont *font;
	UT_uint32 des;

	font = m_pCurrentFont;
		
        UT_ASSERT(GNOME_IS_FONT (font));
	face = gnome_font_get_face (font);
        UT_ASSERT(GNOME_IS_FONT_FACE (face));
	bbox = 	gnome_font_face_get_stdbbox (face);

	des = (gint) (bbox->y0 * gnome_font_get_size (font) / 10);
	des *= -1;

//	UT_DEBUGMSG(("Font \"Descent\" %i [%g, %g]\n", des, bbox->y0,
//		     gnome_font_get_size (font)));

	return des;
}

UT_uint32 XAP_UnixGnomePrintGraphics::getFontHeight()
{
	UT_ASSERT(m_pCurrentFont);

	UT_uint32 height = getFontAscent() - getFontDescent();
	return height;
}

GR_Font* XAP_UnixGnomePrintGraphics::findFont(const char* pszFontFamily, 
				      const char* pszFontStyle, 
				      const char* pszFontVariant, 
				      const char* pszFontWeight, 
				      const char* pszFontStretch, 
				      const char* pszFontSize)
{
//      UT_DEBUGMSG(("Dom: findFont\n"));
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

	// Request the appropriate XAP_UnixFont::, and bury it in an
	// instance of a UnixFont:: with the correct size.
	XAP_UnixFont * unixfont = m_fm->getFont(pszFontFamily, s);
	XAP_UnixFont * item = NULL;
	if (unixfont)
	{
		// make a copy
		item = new XAP_UnixFont(*unixfont);
	}
	else
	{
		// Oops!  We don't have that font here.  substitute something
		// we know we have (get smarter about this later)
		g_print ("Get times new roman !!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
			 "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!1\n");
		item = new XAP_UnixFont(*m_fm->getFont("Times New Roman", s));
	}
	
	PSFont * pFont = new PSFont(item, convertDimension(pszFontSize));

	return pFont;
}

/***********************************************************************/
/*                Private Scaling Conversion Routines                  */
/***********************************************************************/

double XAP_UnixGnomePrintGraphics::_scale_factor_get (void)
{
	return ((double) 72.0 / (double) GPG_RESOLUTION);
}

double XAP_UnixGnomePrintGraphics::_scale_factor_get_inverse (void)
{
	return ((double) GPG_RESOLUTION / (double) 72.0);
}

double XAP_UnixGnomePrintGraphics::_scale_x_dir (int x)
{
        double d = 0.0;
	
	d  =  (double) x;
	d *= _scale_factor_get ();
	
	return d;
}

double XAP_UnixGnomePrintGraphics::_scale_y_dir (int y)
{
	double d = 0.0;
	static double page_length = 0.0;
	
	if (page_length == 0.0)
		{
#if 0
			if (m_paper)
				{
					page_length = 96 * gnome_paper_psheight (m_paper);
					UT_DEBUGMSG(("Dom: %f %f\n", page_length, 
						     11 * (double) GPG_RESOLUTION));
				}
			else
#else
				{
					/* FIXME: Hardcode US-Letter for now */
					page_length = 11 * (double) GPG_RESOLUTION;      
				}
#endif
		}
	
	d  =  page_length - (double) y;
	d *= _scale_factor_get ();
	
	return d;
}
