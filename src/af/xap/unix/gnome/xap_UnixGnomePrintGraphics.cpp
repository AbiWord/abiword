/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 8 -*- */
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
#include "xap_EncodingManager.h"

#include <libgnomeprint/gnome-print-master-preview.h>

#define OUR_LINE_LIMIT          200 /* FIXME:
				       UGLY UGLY UGLY, but we need to fix the PrintPrivew
				       show bug. Test it with something like 7 when to see
				       what is the problem with PP. The show operator in
				       print preview does not move th currentpoint. Chema */

// the resolution that we report to the application (pixels per inch).
#define GPG_RESOLUTION		7200
#define GPG_DEFAULT_FONT        "Nimbus Roman No9 L"

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
/* The ones with ?? have not been verified. (Chema) */
static struct _fontMapping fontMappingTable[] = 
{
	{"Arial",                  "Helvetica"}, // Arial is a MS name for Helvetica, so I've been told
	{"Bitstream",              "Palatino"}, // ??
	{"Bookman",                "URW Bookman L"},
	{"Century Schoolbook",     "Century Schoolbook L"},
	{"Courier",                "Courier"},
	{"Courier New",            "Courier"}, /* ??? not really. (I think) */
	{"Dingbats",               "Dingbats"},
	{"Goth",                   "URW Gothic L"},
	{"Helvetic",               "Helvetica"},
	{"Helvetica",              "Helvetica"},
	{"Nimbus Mono",            "Nimbus Mono L"},
	{"Nimbus Roman",           "Nimbus Roman No9 L"},
	{"Nimbus Sans",            "Nimbus Sans L"},
	{"Nimbus Sans Condensed",  "Nimbus Sans L"}, // ??
	{"Palladio",               "URW Palladio L"},
	{"Standard Symbols",       "Standard Symbols L"},
	{"Symbol",                 "Standard Symbols L"}, // ?? (Symbol?)
	{"Times",                  "Times"},
	{"Times New Roman",        "Nimbus Roman No9 L"},
	{"*",                      GPG_DEFAULT_FONT}
};

#define TableSize	((sizeof(fontMappingTable)/sizeof(fontMappingTable[0])))

static char * mapFontName(const char *name)
{
        unsigned int idx = 0;

	// if we're passed crap, default to some normal font
	if(!name || !*name)
	  {
	        xxx_UT_DEBUGMSG(("Dom: mapFontName: null name, returning default\n"));
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

#undef TableSize

static bool isItalic(XAP_UnixFont::style s)
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

#define DEFAULT_GNOME_FONT "Helvetica"
static
gboolean fonts_match(GnomeFont *tmp, const gchar * intended)
{
		/* this is a hack - gnome_font_new_closest() returns Helvetica
		   when it gets confused */

		if(!g_strcasecmp(intended, DEFAULT_GNOME_FONT))
				return TRUE; // asked for and got helvetica

		const gchar * what = gnome_font_get_name(tmp);
		xxx_UT_DEBUGMSG(("DOM: intended - '%s' what - '%s'\n", intended, what));

		if(!g_strcasecmp(what, DEFAULT_GNOME_FONT))
				return FALSE; // asked for something and got helvetica instead
		return TRUE;
}
#undef DEFAULT_GNOME_FONT

GnomeFont * XAP_UnixGnomePrintGraphics::_allocGnomeFont(PSFont* pFont)
{
        XAP_UnixFont *uf          = pFont->getUnixFont();
	XAP_UnixFont::style style = uf->getStyle();
	char *abi_name            = (char*)uf->getName();
	
	GnomeFontWeight gfw = getGnomeFontWeight(style);
	bool italic      = isItalic(style);
	
	// ok, this is the ugliest hack of the year, so I'll take it one step
	// at a time

	// add 0.1 so 11.99 gets rounded up to 12
	double size         = (double)pFont->getSize() * _scale_factor_get () + 0.1;
	
	// test for oddness, if odd, subtract 1
	// why? abi allows odd point fonts for at least 
	// 9 and 11 points. gnome print does not, so we
	// scale it down. *ugly*
	if((int)size % 2 != 0)
			size -= 1.0;
			
	// first try to directly allocate abi's name
	// this is good for fonts not in my table, and
	// fonts installed by the user in both Abi and
	// using gnome-font-install

	GnomeFont *tmp = NULL;

	/* gnome_font_new_closest always returns a font. 
	   So you will get helvetica if not found */

	tmp      = gnome_font_new_closest(abi_name, gfw, italic, size);

	// assert that the fonts match
	if(tmp && fonts_match(tmp, abi_name))
			return tmp;

	xxx_UT_DEBUGMSG(("Dom: unreffing gnome font: ('%s','%s')\n", 
		     gnome_font_get_ps_name(tmp), abi_name));

	// else we got something we didn't ask for
	// unref the gnome-font and try again
	gnome_font_unref(tmp);

	char *fontname      = mapFontName(abi_name);
	tmp = gnome_font_new_closest(fontname, gfw, italic, size);

	return tmp;
}

/***********************************************************************/
/*      This file provides an interface into Gnome Print               */
/***********************************************************************/

XAP_UnixGnomePrintGraphics::~XAP_UnixGnomePrintGraphics()
{
		// unref the resource
		if(m_pCurrentFont != NULL && GNOME_IS_FONT(m_pCurrentFont))
				gnome_font_unref(m_pCurrentFont);
}

struct pageSizeMapping {
		char * abi;
		char * gnome;
};

static struct pageSizeMapping paperMap[] = {
		{"Letter", "US-Letter"},
		{"Legal",  "US-Legal"},
		{"Folio",  "Executive"}, // i think that this is correct
		{NULL,     NULL}
};

#define PaperTableSize	((sizeof(paperMap)/sizeof(paperMap[0])))

// This method maps Abi's paper sizes to their gnome-print
// equivalents if there is a mapping. If @sz is null,
// return the default name. If @sz is not in our map, return @sz
static const
char * mapPageSize (const char * sz)
{
		if (!sz && !*sz)
				return gnome_paper_name_default ();

		for (int i = 0; i < (int)PaperTableSize; i++)
				{
						if (paperMap[i].abi && !g_strcasecmp (sz, paperMap [i].abi))
								return paperMap[i].gnome;
				}

		return sz;
}

#undef PaperTableSize

XAP_UnixGnomePrintGraphics::XAP_UnixGnomePrintGraphics(GnomePrintMaster *gpm,
													   const char * pageSize,
						       XAP_UnixFontManager * fontManager,
						       XAP_App *pApp,
						       bool isPreview)
{
        m_pApp         = pApp;
	m_gpm          = gpm;
	m_gpc          = gnome_print_master_get_context(gpm);

	// TODO: be more robust about this
	const GnomePaper * paper = gnome_paper_with_name(mapPageSize (pageSize));

	UT_DEBUGMSG(("DOM: mapping '%s' returned '%s'\n", pageSize, mapPageSize (pageSize)));
	if (!paper)
			paper = gnome_paper_with_name (gnome_paper_name_default ());

	UT_ASSERT(paper);

	m_paper = paper;
	gnome_print_master_set_paper(m_gpm, paper);

	m_bIsPreview   = isPreview;
	m_fm           = fontManager;
	m_bStartPrint  = false;
	m_bStartPage   = false;
	m_pCurrentFont = NULL;

	m_cs = GR_Graphics::GR_COLORSPACE_COLOR;

	m_currentColor.m_red = 0;
	m_currentColor.m_grn = 0;
	m_currentColor.m_blu = 0;
}

UT_uint32 XAP_UnixGnomePrintGraphics::measureUnRemappedChar(const UT_UCSChar c)
{
	int size = 0;
	
        UT_ASSERT(m_pCurrentFont);
	if (c >= 256)
		return 0;

#if 1
	unsigned char uc = c;
	size = (int) ( _scale_factor_get_inverse () *
		       gnome_font_get_width_string_n (m_pCurrentFont, (const char *)&uc, 1) );
#else
	size = (int) (_scale_factor_get_inverse () * gnome_font_get_glyph_width(m_pCurrentFont, (int)c));
#endif
	
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
	guchar buf[OUR_LINE_LIMIT*6];
	guchar * pD;
	const UT_UCSChar * pS;
	const UT_UCSChar * pEnd;

	xxx_UT_DEBUGMSG(("DOM: drawChars (x: %d) (y: %d)\n", xoff, yoff));

	gnome_print_moveto(m_gpc, _scale_x_dir(xoff), _scale_y_dir(yoff));

	pEnd = pChars + iCharOffset + iLength;
	
	for (pS = pChars + iCharOffset; pS < pEnd; pS += OUR_LINE_LIMIT) {
			const UT_UCSChar * pB;
			UT_UCSChar currentChar;

			pD = buf;
			for (pB = pS; (pB < pS + OUR_LINE_LIMIT) && (pB < pEnd); pB++) {
					currentChar = remapGlyph(*pB, *pB >= 256 ? true : false);
					currentChar = currentChar <= 0xff ? currentChar : XAP_EncodingManager::instance->UToNative(currentChar);
					pD += unichar_to_utf8 (currentChar, pD);
			}
			gnome_print_show_sized (m_gpc, (gchar *) buf, pD - buf);
	}
}

void XAP_UnixGnomePrintGraphics::drawLine(UT_sint32 x1, UT_sint32 y1,
				  UT_sint32 x2, UT_sint32 y2)
{
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

	// TODO: We *must* be smarter about this, maybe a hash
	// TODO: of PSFonts -> GnomeFonts

	if(m_pCurrentFont && GNOME_IS_FONT(m_pCurrentFont))
			gnome_font_unref(m_pCurrentFont);

	m_pCurrentFont = _allocGnomeFont(psFont);

#if 0
	XAP_UnixFont *uf          = static_cast<PSFont*>(pFont)->getUnixFont();
	xxx_UT_DEBUGMSG(("Dom: setting font:\n"
				 "\tsize returned: %f (requested %f)\n"
				 "\tname returned: %s (requested %s)\n", 
				 gnome_font_get_size(m_pCurrentFont),
				 (double)static_cast<PSFont*>(pFont)->getSize() * _scale_factor_get(), 
				 gnome_font_get_name(m_pCurrentFont), uf->getName()));
#endif

	gnome_print_setfont (m_gpc, m_pCurrentFont);
}

bool XAP_UnixGnomePrintGraphics::queryProperties(GR_Graphics::Properties gp) const
{
	switch (gp)
	{
	case DGP_SCREEN:
		return false;
	case DGP_PAPER:
		return true;
	default:
		UT_ASSERT(0);
		return false;
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

	xxx_UT_DEBUGMSG(("Dom: setColor\n"));

	gnome_print_setrgbcolor(m_gpc,
				(int)(m_currentColor.m_red / 255),
				(int)(m_currentColor.m_grn / 255),
				(int)(m_currentColor.m_blu / 255));
	
}

void XAP_UnixGnomePrintGraphics::setLineWidth(UT_sint32 iLineWidth)
{
 	m_dLineWidth = (double)((double)iLineWidth * _scale_factor_get());
}

bool XAP_UnixGnomePrintGraphics::startPrint(void)
{
        UT_ASSERT(!m_bStartPrint);
	m_bStartPrint = true;
	return _startDocument();
}

bool XAP_UnixGnomePrintGraphics::startPage(const char * szPageLabel)
{
	if (m_bStartPage)
	  _endPage();
	m_bStartPage = true;
	m_bNeedStroked = false;
	return _startPage(szPageLabel);
}

bool XAP_UnixGnomePrintGraphics::startPage (const char *szPageLabel, 
											UT_uint32, bool,
											UT_uint32, UT_uint32)
{
        return startPage(szPageLabel);
}

bool XAP_UnixGnomePrintGraphics::endPrint()
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
				       UT_sint32 xDest, UT_sint32 yDest, bool rgb)
{
	UT_sint32 iDestWidth  = pImg->getDisplayWidth();
	UT_sint32 iDestHeight = pImg->getDisplayHeight();
	
	PS_Image * pPSImage = static_cast<PS_Image *>(pImg);

	PSFatmap * image = pPSImage->getData();

#if 0
	xxx_UT_DEBUGMSG(("DOM: image data:\n"
				 "\tiDestWidth: %d\n"
				 "\tiDestHeight: %d\n"
				 "\twidth: %d\n"
				 "\theight: %d\n"
				 "\tRGB: %d\n"
				 "\tTranslated: (%d, %d)\n"
				 "\tScaled = (iDestWidth, iDestHeight)\n"
				 "\tyDest: %d displayHeight: %d\n",
				 (int)(iDestWidth*_scale_factor_get()), // iDestWidth
				 (int)(iDestHeight*_scale_factor_get()), // iDestHeight
				 image->width, // width
				 image->height, // height
				 rgb, // rgb
				 (int)_scale_x_dir(xDest), // translated 1
				 (int)_scale_y_dir(yDest + pImg->getDisplayHeight()), // translated 2
				 yDest, pImg->getDisplayHeight()));
#endif

	UT_ASSERT(image && image->data);

	gnome_print_gsave(m_gpc);
	gnome_print_translate(m_gpc,
						  _scale_x_dir(xDest),
						  _scale_y_dir(yDest + pImg->getDisplayHeight()));
	gnome_print_scale(m_gpc,
			  ((double) iDestWidth)  * _scale_factor_get (),
			  ((double) iDestHeight) * _scale_factor_get ());
				 
	/* 
	 * TODO: one day support the alpha channel internally and then call
	 * gnome_print_rgbaimage()
	 */
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
		drawAnyImage(pImg, xDest, yDest, true);
		break;
      	case GR_Graphics::GR_COLORSPACE_GRAYSCALE:
		drawAnyImage(pImg, xDest, yDest, false);
		break;
      	case GR_Graphics::GR_COLORSPACE_BW:
		drawAnyImage(pImg, xDest, yDest, false);
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
bool	XAP_UnixGnomePrintGraphics::_startDocument(void)
{
        return true;
}

bool XAP_UnixGnomePrintGraphics::_startPage(const char * szPageLabel)
{
		xxx_UT_DEBUGMSG(("DOM: startPage\n"));
        gnome_print_beginpage(m_gpc, szPageLabel);
		_setup_rotation ();
		return true;
}

bool XAP_UnixGnomePrintGraphics::_endPage(void)
{
		xxx_UT_DEBUGMSG(("DOM: endPage\n"));

	if(m_bNeedStroked)
	  gnome_print_stroke(m_gpc);

	gnome_print_showpage(m_gpc);
	return true;
}

bool XAP_UnixGnomePrintGraphics::_endDocument(void)
{

		xxx_UT_DEBUGMSG(("DOM: endDocument\n"));
		// bonobo version, we'd don't own the context
		// or the master, just return
	if(!m_gpm)
	  return true;

        gnome_print_master_close(m_gpm);

	if(!m_bIsPreview)
	  {
	    gnome_print_master_print(m_gpm);
	  }
	else
	  {
	    GnomePrintMasterPreview *preview;
	    const XAP_StringSet * pSS = m_pApp->getStringSet();

	    preview = gnome_print_master_preview_new_with_orientation (m_gpm, 
																   pSS->getValue(XAP_STRING_ID_DLG_UP_PrintPreviewTitle), 
																   !isPortrait());
	    gtk_widget_show(GTK_WIDGET(preview));
	  }
	
	gtk_object_unref(GTK_OBJECT(m_gpm));
	return true;
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

void XAP_UnixGnomePrintGraphics::fillRect(UT_RGBColor& c, UT_sint32 x, 
										  UT_sint32 y, UT_sint32 w, 
										  UT_sint32 h)
{
		// draw background color
		gnome_print_setrgbcolor(m_gpc,
								(int)(c.m_red / 255),
								(int)(c.m_grn / 255),
								(int)(c.m_blu / 255));

		// adjust for the text's height
		//y += getFontDescent () + getFontHeight();
		
		/* Mirror gdk which excludes the far point */
#if 0
		w -= (int)_scale_x_dir (1);
		h -= (int)_scale_y_dir (1);
#endif

		xxx_UT_DEBUGMSG(("DOM: (w: %d) (h: %d) (x: %d) (y: %d)\n",
						 w, h, x, y));

		// Lauris says to do this: 
		// newpath + moveto + lineto + lineto + lineto + lineto + closepath + fill
		gnome_print_newpath (m_gpc);
		gnome_print_moveto (m_gpc, _scale_x_dir(x),   _scale_y_dir(y));		
		gnome_print_lineto (m_gpc, _scale_x_dir(x+w), _scale_y_dir(y));
		gnome_print_lineto (m_gpc, _scale_x_dir(x+w), _scale_y_dir(y+h));
		gnome_print_lineto (m_gpc, _scale_x_dir(x),   _scale_y_dir(y+h));
		gnome_print_lineto (m_gpc, _scale_x_dir(x),   _scale_y_dir(y));
		gnome_print_closepath (m_gpc);
		gnome_print_fill (m_gpc);

		// reset color to its original state
		gnome_print_setrgbcolor(m_gpc,
								(int)(m_currentColor.m_red / 255),
								(int)(m_currentColor.m_grn / 255),
								(int)(m_currentColor.m_blu / 255));
}

void XAP_UnixGnomePrintGraphics::fillRect(UT_RGBColor& c, UT_Rect & r)
{
		fillRect(c, r.left, r.top, r.width, r.height);
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

GR_Font* XAP_UnixGnomePrintGraphics::getGUIFont()
{
        UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return NULL;
}

void XAP_UnixGnomePrintGraphics::fillRect(GR_Color3D c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h)
{
		// nada
		xxx_UT_DEBUGMSG(("DOM: FILLRECT3D\n"));
}

void XAP_UnixGnomePrintGraphics::fillRect(GR_Color3D c, UT_Rect &r)
{
		// nada
		xxx_UT_DEBUGMSG(("DOM: FILLRECT3D\n"));
}


/***********************************************************************/
/*                                 Done                                */
/***********************************************************************/

UT_uint32 XAP_UnixGnomePrintGraphics::getFontAscent(GR_Font *fnt)
{
	GnomeFont * gfnt = _allocGnomeFont(static_cast<PSFont*>(fnt));
	const GnomeFontFace *face;
	const ArtDRect *bbox;
	UT_uint32 asc;

	face = gnome_font_get_face (gfnt);
	bbox = 	gnome_font_face_get_stdbbox (face);
	asc = (gint) (bbox->y1 * gnome_font_get_size (gfnt) / 10);
	gnome_font_unref (gfnt);

	return asc;
}

/* This function does not expect in return the font ascent,
   it expects the font bbox.ury. Chema */
UT_uint32 XAP_UnixGnomePrintGraphics::getFontAscent()
{
	const GnomeFontFace *face;
	const ArtDRect *bbox;
	GnomeFont *font;
	UT_uint32 asc;

	font = m_pCurrentFont;
	face = gnome_font_get_face (font);
	bbox = 	gnome_font_face_get_stdbbox (face);
	asc = (gint) (bbox->y1 * gnome_font_get_size (font) / 10);

	return asc;
}

UT_uint32 XAP_UnixGnomePrintGraphics::getFontDescent(GR_Font *fnt)
{
	GnomeFont * gfnt = _allocGnomeFont(static_cast<PSFont*>(fnt));
	const GnomeFontFace *face;
	const ArtDRect *bbox;
	UT_uint32 des;

	face = gnome_font_get_face (gfnt);
	bbox = 	gnome_font_face_get_stdbbox (face);

	des = (gint) (bbox->y0 * gnome_font_get_size (gfnt) / 10);
	des *= -1;
	gnome_font_unref (gfnt);
	return des;
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

	return des;
}

UT_uint32 XAP_UnixGnomePrintGraphics::getFontHeight()
{
	UT_ASSERT(m_pCurrentFont);

	return getFontAscent() - getFontDescent();
}

UT_uint32 XAP_UnixGnomePrintGraphics::getFontHeight(GR_Font *fnt)
{
		return getFontAscent(fnt) - getFontDescent(fnt);
}

GR_Font* XAP_UnixGnomePrintGraphics::findFont(const char* pszFontFamily, 
											  const char* pszFontStyle, 
											  const char* /* pszFontVariant */,
											  const char* pszFontWeight, 
											  const char* /* pszFontStretch */,
											  const char* pszFontSize)
{
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
				 "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		item = new XAP_UnixFont(*m_fm->getFont("Times New Roman", s));
	}
	
	PSFont * pFont = new PSFont(item, convertDimension(pszFontSize));
	UT_ASSERT(pFont);

	return pFont;
}

/***********************************************************************/
/*                Private Scaling Conversion Routines                  */
/***********************************************************************/

void XAP_UnixGnomePrintGraphics::_setup_rotation (void)
{
	double affine [6];

	// do nothing for this default case
	if (isPortrait ())
		return;

	// we have to apply an affine to the print context for
	// each page in order to print in landscape mode
	art_affine_rotate (affine, 90.0);
	gnome_print_concat (m_gpc, affine);

	art_affine_translate (affine, 0, - _get_height ());
	gnome_print_concat (m_gpc, affine);
}

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

double XAP_UnixGnomePrintGraphics::_get_height (void)
{
	if (m_paper)
			{
					if (isPortrait ())
							return gnome_paper_psheight (m_paper);
					else
							return gnome_paper_pswidth (m_paper);
			}
	else
			{
					/* FIXME: Hardcode US-Letter values as a standby/fallback for now */
					if (isPortrait ())
							return 11.0;
					else
							return 8.5;
			}
}

double XAP_UnixGnomePrintGraphics::_scale_y_dir (int y)
{
	double d = 0.0;
	double page_length = 0.0;
	
	page_length = _get_height ();

	/* This one is obscure:
	 * Our drawChars and drawLine functions recieve yDests relative to the
	 * page that they're on. drawImg doesn't. We need to properly
	 * correct for that
	 */

#if 0	
	d  =  page_length - (double) y;
	d *= _scale_factor_get();
#else
	d = page_length - (double)((int)(y * _scale_factor_get()) % (int)page_length);
#endif

	return d;
}
