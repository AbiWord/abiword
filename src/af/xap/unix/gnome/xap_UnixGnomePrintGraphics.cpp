/* -*- Mode: C; tab-width: 4; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/* AbiSource Application Framework
 * Copyright (C) 2001,2002 AbiSource, Inc.
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
#include <ctype.h>
#include <glib.h>

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_misc.h"
#include "xap_Strings.h"
#include "xap_UnixGnomePrintGraphics.h"
#include "xap_EncodingManager.h"
#include "gr_UnixGnomeImage.h"
#include "ut_string_class.h"
#include "xap_UnixDialogHelper.h"

#include <libgnomeprint/gnome-print-master-preview.h>

/***********************************************************************/
/*      This file provides an interface into Gnome Print               */
/***********************************************************************/

#define OUR_LINE_LIMIT          200 /* FIXME:
				       UGLY UGLY UGLY, but we need to fix the PrintPrivew
				       show bug. Test it with something like 7 when to see
				       what is the problem with PP. The show operator in
				       print preview does not move th currentpoint. Chema */

// the resolution that we report to the application (pixels per inch).
#define GPG_RESOLUTION		7200
#define GPG_DEFAULT_FONT        "Nimbus Roman No9 L"

/*************************************************************************/
// Map font specic encoding to Unicode for gnome-print
/*************************************************************************/
struct _encUnicode
{
        UT_uint32 fontcode; // Font specific code
		UT_uint32 unicode;  // True unicode value
};

/***********************************************************************/
/*      map abi's fonts to gnome fonts, or at least try to             */
/***********************************************************************/

struct _fontMapping 
{
        char *abi;   // what abiword calls a font's name
        char *gnome; // what gnome refers to it as (or a close substitute)
};


// mapping of the fonts that abi ships with
// to what gnome-font ships with
/* The ones with ?? have not been verified. (Chema) */
static struct _fontMapping fontMappingTable[] = 
{
		{"Arial",                  "Nimbus Sans L"}, // Arial is a MS name for Helvetica, //v
		{"Bitstream",              "Bitstream Charter"}, // ?? // v
		{"Bookman",                "URW Bookman L"}, // v
		{"Century Schoolbook",     "Century Schoolbook L"}, //v
		{"Courier",                "Nimbus Mono L"}, //v
		{"Courier New",            "Nimbus Mono L"}, /* ??? not really. (I think) */ //v
		{"Dingbats",               "Dingbats"}, // v
		{"Goth",                   "URW Gothic L"}, // v
		{"Helvetic",               "Nimbus Sans L"}, // v
		{"Helvetica",              "Nimbus Sans L"}, // v
		{"Nimbus Mono",            "Nimbus Mono L"}, // v
		{"Nimbus Roman",           "Nimbus Roman No9 L"}, //v
		{"Nimbus Sans",            "Nimbus Sans L"}, // v
		{"Nimbus Sans Condensed",  "Nimbus Sans L"}, // v This name is Nimbus Sans L * Condensed!!
		{"Palladio",               "URW Palladio L"}, // v
		{"Standard Symbols",       "Standard Symbols L"}, // v
		{"Symbol",                 "Standard Symbols L"}, // v
		{"Times",                  "Nimbus Roman No9 L"}, // v
		{"Times New Roman",        "Nimbus Roman No9 L"}, //v
		{"*",                      GPG_DEFAULT_FONT}
};

#define TableSize	((sizeof(fontMappingTable)/sizeof(fontMappingTable[0])))

static char * mapFontName(const char *name)
{
		unsigned int idx = 0;
		// if we're passed crap, default to some normal font
		if(!name || !*name)
				{
						return GPG_DEFAULT_FONT;
				}
		UT_uint32 k;
		for (k=0; k<TableSize; k++)
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

/*!
 * This is for sorting the unicode vectors
\params p1 pointer to the pointer of the _encUnicode struct
\params p2 pointer to the pointer of the _encUnicode struct
\returns -1 if p1 < p2, 0 if p1 = p2, +ve if p1 > p2
*/
static UT_sint32 compareCodes( const void * p1, const void * p2)
{
		_encUnicode ** sP1 = (_encUnicode **) const_cast<void *>(p1); 
		_encUnicode ** sP2 = (_encUnicode **) const_cast<void *>(p2);
		return (UT_sint32) (*sP1)->fontcode - (UT_sint32) (*sP2)->fontcode;
}

static bool isItalic(XAP_UnixFont::style s)
{
        return ((s == XAP_UnixFont::STYLE_ITALIC) || 
				(s == XAP_UnixFont::STYLE_BOLD_ITALIC));
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
		xxx_UT_DEBUGMSG(("Looking for %s returned %s \n",intended,what));
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
		double size         = (double)pFont->getSize() * _scale_factor_get ();

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

XAP_UnixGnomePrintGraphics::~XAP_UnixGnomePrintGraphics()
{
		// unref the resource
		if(m_pCurrentFont != NULL && GNOME_IS_FONT(m_pCurrentFont))
				gnome_font_unref(m_pCurrentFont);
		UT_VECTOR_PURGEALL(_encUnicode *, m_vecEncDingbats);
		UT_VECTOR_PURGEALL(_encUnicode *, m_vecEncSymbol);
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

XAP_UnixGnomePrintGraphics::XAP_UnixGnomePrintGraphics(GnomePrintContext *gpc,
													   XAP_UnixFontManager * fontManager,
													   XAP_App *pApp)
{
		m_gpm = NULL;
		m_gpc = gpc;
		m_paper = NULL;

		m_bIsPreview   = false;
		m_fm           = fontManager;
		m_bStartPrint  = false;
		m_bStartPage   = false;
		m_pCurrentFont = NULL;
		m_pCurrentPSFont = NULL;
		if(m_vecEncSymbol.getItemCount()>0)
				m_vecEncSymbol.clear();
		if(m_vecEncDingbats.getItemCount()>0)
				m_vecEncDingbats.clear();
		
		m_cs = GR_Graphics::GR_COLORSPACE_COLOR;
		
		m_currentColor.m_red = 0;
		m_currentColor.m_grn = 0;
		m_currentColor.m_blu = 0;
		//
		// Now I need to load in the translations to unicode for Dingbats and Symbols.
		//
		loadUnicodeData();
}

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

	xxx_UT_DEBUGMSG(("DOM: mapping '%s' returned '%s'\n", pageSize, mapPageSize (pageSize)));
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
	m_pCurrentPSFont = NULL;
	if(m_vecEncSymbol.getItemCount()>0)
			m_vecEncSymbol.clear();
	if(m_vecEncDingbats.getItemCount()>0)
			m_vecEncDingbats.clear();

	m_cs = GR_Graphics::GR_COLORSPACE_COLOR;

	m_currentColor.m_red = 0;
	m_currentColor.m_grn = 0;
	m_currentColor.m_blu = 0;
//
// Now I need to load in the translations to unicode for Dingbats and Symbols.
//
	loadUnicodeData();
}

bool XAP_UnixGnomePrintGraphics::loadUnicodeData(void)
{
	UT_String SymbolName(XAP_App::getApp()->getAbiSuiteLibDir());
	SymbolName += "/fonts/symbol.e2u";
	char * szName = const_cast<char *>(SymbolName.c_str());
	FILE * ef = fopen(szName, "r");
	char buff[128];
	UT_sint32 i;
	UT_uint32 code,unicode;
	if(!ef)
	{
			UT_DEBUGMSG(("Cannot load Symbol Unicode value file \n"));
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return false;
	}
	else
	{
			//
			// Load the file
			//skip comments and empty lines
			while(fgets(buff,128,ef))
					if((*buff != '#')&&(*buff != '\n'))
							break;
			UT_sint32 nLines = atoi(buff);
			for(i=0; i< nLines;i++)
			{
					if(!fgets(buff,128,ef))
					{
							UT_VECTOR_PURGEALL(_encUnicode *, m_vecEncSymbol);
							UT_DEBUGMSG(("Invalid Unicode translation file %s \n",szName));
							UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
							fclose(ef);
							return false;
					}
					
					if((*buff != '#')&&(*buff != '\n'))
					{
							buff[strlen(buff)-1]= 0; //remove '\n'
							xxx_UT_DEBUGMSG(("Reading line %s \n",buff));
							char * comma = strchr(buff, ',');
							if(!comma)
							{
									UT_VECTOR_PURGEALL(_encUnicode *, m_vecEncSymbol);
									UT_DEBUGMSG(("Invalid Unicode translation file %s \n",szName));
									UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
									fclose(ef);
									return false;
							}
							*comma = 0;
							sscanf(comma + 1, "%x", &unicode);
							sscanf(buff,"%x",&code);
							_encUnicode * p = new _encUnicode;
							p->fontcode = code;
							p->unicode = unicode;
							m_vecEncSymbol.addItem((void *) p);
					}
			}
			fclose(ef);
	}
//
// Load the Dingbat encoding file
//
	SymbolName = XAP_App::getApp()->getAbiSuiteLibDir();
	SymbolName += "/fonts/dingbats.e2u";
	szName = const_cast<char *>(SymbolName.c_str());
	ef = fopen(szName, "r");
	if(!ef)
	{
			UT_DEBUGMSG(("Cannot load Dingbats Unicode value file %s \n",szName));
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return false;
	}
	else
	{
			//
			// Load the file
			//skip comments and empty lines
			while(fgets(buff,128,ef))
					if((*buff != '#')&&(*buff != '\n'))
							break;
			UT_sint32 nLines = atoi(buff);
			for(i=0; i< nLines;i++)
			{
					if(!fgets(buff,128,ef))
					{
							UT_VECTOR_PURGEALL(_encUnicode *, m_vecEncDingbats);
							UT_DEBUGMSG(("Invalid Unicode translation file %s \n",szName));
							UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
							fclose(ef);
							return false;
					}
					
					if((*buff != '#')&&(*buff != '\n'))
					{
							buff[strlen(buff)-1]= 0; //remove '\n'
							xxx_UT_DEBUGMSG(("Reading line %s \n",buff));
							char * comma = strchr(buff, ',');
							if(!comma)
							{
									UT_VECTOR_PURGEALL(_encUnicode *, m_vecEncDingbats);
									UT_DEBUGMSG(("Invalid Unicode translation file %s \n",szName));
									UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
									fclose(ef);
									return false;
							}
							*comma = 0;
							sscanf(comma + 1, "%x", &unicode);
							sscanf(buff,"%x",&code);
							_encUnicode * p = new _encUnicode;
							p->fontcode = code;
							p->unicode = unicode;
							m_vecEncDingbats.addItem((void *) p);
					}
			}
			fclose(ef);
	}
	m_vecEncSymbol.qsort(compareCodes);
	m_vecEncDingbats.qsort(compareCodes);
	return true;
}

UT_uint32 XAP_UnixGnomePrintGraphics::getUnicodeForSymbol(UT_uint32 code)
{
//
// Do a binary search later
//
	UT_sint32 nCode = m_vecEncSymbol.getItemCount();
	_encUnicode *p = NULL;
	UT_sint32 i;
	for(i=0; i< nCode; i++)
	{	
		p = static_cast<_encUnicode *>( m_vecEncSymbol.getNthItem(i));
		if(p->fontcode == code)
				break;
	}
	if(nCode == i || p->fontcode != code)
			return 0;
	return p->unicode;
}


UT_uint32 XAP_UnixGnomePrintGraphics::getUnicodeForDingbats(UT_uint32 code)
{
//
// Do a binary search later
//
	UT_sint32 nCode = m_vecEncSymbol.getItemCount();
	_encUnicode *p = NULL;
	UT_sint32 i;
	for(i=0; i< nCode; i++)
	{	
		p = static_cast<_encUnicode *>( m_vecEncDingbats.getNthItem(i));
		if(p->fontcode >= code)
				break;
	}
	if(i == nCode || p->fontcode != code)
			return 0;
	return p->unicode;
}


UT_uint32 XAP_UnixGnomePrintGraphics::measureUnRemappedChar(const UT_UCSChar c)
{
#ifdef USE_XFT
	UT_ASSERT(m_pCurrentFont);
	XAP_UnixFont* pUFont = m_pCurrentPSFont->getUnixFont();
	UT_sint32 iSize = m_pCurrentPSFont->getSize();
	return (UT_uint32) (pUFont->measureUnRemappedChar(c, iSize) + 0.5);
#else
	
	UT_uint32 width = 0;
	
	UT_ASSERT(m_pCurrentFont);

	UT_UCSChar cc = c; 
	//
	// If the font is in cache we're doing layout calculations so use exactly
	// the same calculations as the screen uses.
	//
	XAP_UnixFont * pUFont = m_pCurrentPSFont->getUnixFont();
	UT_sint32 iSize = m_pCurrentPSFont->getSize();
	xxx_UT_DEBUGMSG(("SEVIOR: Gnome Looking for font of size %d in the cache \n",iSize));
	if(pUFont->isSizeInCache(iSize))
	{
			XAP_UnixFontHandle * pHndl = new XAP_UnixFontHandle(pUFont, iSize);
			GdkFont* font = pHndl->getGdkFont();
			gchar gc = (gchar) c;
			width = gdk_text_width(font, (gchar*)&gc, 1);
			delete pHndl;
			xxx_UT_DEBUGMSG(("SEVIOR: Got Width %d from gdk_font \n",width));
			return width;
	}
	else
	{
			width = (UT_uint32) ((double) m_pCurrentPSFont->getCharWidth(cc) * (double) m_pCurrentPSFont->getSize() / 1000.);
			xxx_UT_DEBUGMSG(("SEVIOR: Got Width %d from PS font \n",width));
	}
	return width;
#endif
}

void XAP_UnixGnomePrintGraphics::drawGlyph (UT_uint32 Char, UT_sint32 xoff, UT_sint32 yoff)
{
		// TODO
		UT_ASSERT(UT_NOT_IMPLEMENTED);
		return;
}

void XAP_UnixGnomePrintGraphics::getCoverage (UT_Vector& coverage)
{
		// TODO
		UT_ASSERT(UT_NOT_IMPLEMENTED);
		return;
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

	// push a graphics state & save it. then set the font
	gnome_print_gsave ( m_gpc ) ;
	gnome_print_setfont (m_gpc, m_pCurrentFont);

	gnome_print_moveto(m_gpc, _scale_x_dir(xoff), _scale_y_dir(yoff));

	pEnd = pChars + iCharOffset + iLength;
	
	for (pS = pChars + iCharOffset; pS < pEnd; pS += OUR_LINE_LIMIT) {
			const UT_UCSChar * pB = 0;
   
			pD = buf;
			for (pB = pS; (pB < pS + OUR_LINE_LIMIT) && (pB < pEnd); pB++) {
					UT_UCSChar currentChar = *pB; // normal, default case

					if(m_bisSymbol)
					{
							//
							// Convert to Unicode..
							//
							//currentChar = remapGlyph(*pB, *pB >= 256 ? true : false);
							currentChar = (UT_UCSChar) getUnicodeForSymbol((UT_uint32) currentChar);
					}
					else if(m_bisDingbats)
					{
							//
							// Convert to Unicode..
							//

							//currentChar = remapGlyph(*pB, *pB >= 256 ? true : false);
							currentChar = (UT_UCSChar) getUnicodeForDingbats((UT_uint32) currentChar);
					}
					pD += unichar_to_utf8 (currentChar, pD);
			}
			gnome_print_show_sized (m_gpc, (gchar *) buf, pD - buf);
	}

	// pop the graphics state
	gnome_print_grestore ( m_gpc ) ;
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
	//
	// Need this psFont to get the scales correct
	//
	PSFont * psFont = (static_cast<PSFont*> (pFont));
	m_pCurrentPSFont = psFont;
	//
	// We have to see if we must do fancy encoding to unicode for gnomeprint
	//
	UT_String pszFname(psFont->getUnixFont()->getName());
	UT_sint32 i;
	for(i=0; pszFname[i] != 0; i++)
	{
			pszFname[i] = tolower(pszFname[i]);
	}
	if(strstr(pszFname.c_str(),"symbol") != 0)
	{
			m_bisSymbol = true;
	}
	else if (strstr(pszFname.c_str(),"dingbats") !=0 )
	{
			m_bisDingbats = true;
	}
	else
	{
			m_bisSymbol = false;
			m_bisDingbats = false;
	}

	// TODO: We *must* be smarter about this, maybe a hash
	// TODO: of PSFonts -> GnomeFonts

	if(m_pCurrentFont && GNOME_IS_FONT(m_pCurrentFont))
			gnome_font_unref(m_pCurrentFont);

	m_pCurrentFont = _allocGnomeFont(psFont);

	// we delay gnome_print_setfont until we draw characters. this fixes
	// a bug with regard to font sizes < 10 pts in gnome-print version
	// <= 0.35 at the cost of a slightly larger PS output file
}

bool XAP_UnixGnomePrintGraphics::queryProperties(GR_Graphics::Properties gp) const
{
	switch (gp)
	{
	case DGP_SCREEN:
	case DGP_OPAQUEOVERLAY:
		return false;
	case DGP_PAPER:
		return true;
	default:
		UT_ASSERT(0);
		return false;
	}
}

void XAP_UnixGnomePrintGraphics::getColor(UT_RGBColor& clr)
{
	return m_currentColor;
}

void XAP_UnixGnomePrintGraphics::setColor(const UT_RGBColor& clr)
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

	double red = (double)m_currentColor.m_red / 255.0;
	double green = (double)m_currentColor.m_grn / 255.0;
	double blue = (double)m_currentColor.m_blu / 255.0;
	UT_DEBUGMSG(("SEVIOR: Current red = %f Current green = %f current blue =%f \n",red,green,blue));
	gint ret = gnome_print_setrgbcolor(m_gpc,red,green,blue);
	UT_DEBUGMSG(("SEVIOR: Return value from set = %d \n",ret));

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
											   UT_sint32 xDest, 
											   UT_sint32 yDest, bool rgb)
{

//
// One might think that I would have to adjust these sizes because of the
// scaling by a factor of 36 for print images. Actually I don't think I do.
// gnome-print scales all images to fit in whatever sized piece of screen/paper
// you've got.
//    
	UT_sint32 iDestWidth  = pImg->getDisplayWidth();
	UT_sint32 iDestHeight = pImg->getDisplayHeight();

	UT_DEBUGMSG(("SEVIOR: Draw any image iDestWidth %d iDestHeight %d \n",iDestWidth,iDestHeight));
	GR_UnixGnomeImage * pImage = static_cast<GR_UnixGnomeImage *>(pImg);
	GdkPixbuf * image = pImage->getData();
	UT_ASSERT(image);

	gnome_print_gsave(m_gpc);
	gnome_print_translate(m_gpc,
						  _scale_x_dir(xDest),
						  _scale_y_dir(yDest + iDestHeight));
	gnome_print_scale(m_gpc,
			  ((double) iDestWidth)  * _scale_factor_get (),
			  ((double) iDestHeight) * _scale_factor_get ());

	gnome_print_pixbuf (m_gpc, image);
	
	gnome_print_grestore(m_gpc);
}

void XAP_UnixGnomePrintGraphics::drawImage(GR_Image* pImg, UT_sint32 xDest, 
										   UT_sint32 yDest)
{
	UT_DEBUGMSG(("SEVIOR: drawing image in gnome-print image type = %d \n", pImg->getType()));
   	if (pImg->getType() != GR_Image::GRT_Raster) 
	{
		UT_DEBUGMSG(("SEVIOR: Rendering non-raster image in gnome-print \n"));
	    pImg->render(this, xDest, yDest);
	    return;
	}

	UT_DEBUGMSG(("SEVIOR: drawing raster image in gnome-print \n"));
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
     		pImg = new GR_UnixGnomeImage(pszName,iType,true);
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
		if ( !m_gpm )
				return true ;

        gnome_print_beginpage(m_gpc, szPageLabel);
		_setup_rotation ();
		return true;
}

bool XAP_UnixGnomePrintGraphics::_endPage(void)
{
	if(m_bNeedStroked)
	  gnome_print_stroke(m_gpc);

	if ( !m_gpm )
			return true ;

	gnome_print_showpage(m_gpc);
	return true;
}

bool XAP_UnixGnomePrintGraphics::_endDocument(void)
{
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
	
	g_object_unref(G_OBJECT(m_gpm));
	return true;
}

UT_uint32 XAP_UnixGnomePrintGraphics::_getResolution(void) const
{
        return GPG_RESOLUTION;
}

void XAP_UnixGnomePrintGraphics::fillRect(const UT_RGBColor& c, 
										  UT_sint32 x, UT_sint32 y, 
										  UT_sint32 w, UT_sint32 h)
{
		// draw background color
		gnome_print_setrgbcolor(m_gpc,
								((double) c.m_red) / 255.0,
								((double) c.m_grn) / 255.0,
								((double) c.m_blu) / 255.0);

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

void XAP_UnixGnomePrintGraphics::fillRect(const UT_RGBColor& c, UT_Rect & r)
{
		fillRect(c, r.left, r.top, r.width, r.height);
}

void XAP_UnixGnomePrintGraphics::setClipRect(const UT_Rect* pRect)
{
		// TODO: gnome_print_clip()
		// useful for clipping images and other objects
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
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return GR_CURSOR_INVALID;
}

void XAP_UnixGnomePrintGraphics::xorLine(UT_sint32, UT_sint32, UT_sint32, 
										 UT_sint32)
{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

void XAP_UnixGnomePrintGraphics::polyLine(UT_Point * /* pts */, 
										  UT_uint32 /* nPoints */)
{
        // only used by us for printing red squiggly lines
        // in the spell-checker
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

void XAP_UnixGnomePrintGraphics::invertRect(const UT_Rect* /*pRect*/)
{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
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
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

void XAP_UnixGnomePrintGraphics::fillRect(GR_Color3D c, UT_Rect &r)
{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}


/***********************************************************************/
/*                                 Done                                */
/***********************************************************************/

//
// This code will return the identical size to that used for the layout
//
UT_uint32 XAP_UnixGnomePrintGraphics::getFontAscent(GR_Font *fnt)
{
#ifdef USE_XFT
	PSFont * psfnt = static_cast<PSFont *>(fnt);
	XAP_UnixFont* pUFont = psfnt->getUnixFont();
	UT_sint32 iSize = psfnt->getSize();
	return (UT_uint32) (pUFont->getAscender(iSize) + 0.5);
#else
	UT_uint32 asc;

#if 0
	GnomeFont * gfnt = _allocGnomeFont(static_cast<PSFont*>(fnt));
	const GnomeFontFace *face;
	const ArtDRect *bbox;
	
	face = gnome_font_get_face (gfnt);
	bbox = 	gnome_font_face_get_stdbbox (face);
	asc = (gint) (bbox->y1 * gnome_font_get_size (gfnt) / 10);
	gnome_font_unref (gfnt);
#endif

  PSFont * psfnt = static_cast<PSFont *>(fnt);

  XAP_UnixFont * pUFont = psfnt->getUnixFont();
  UT_sint32 iSize = psfnt->getSize();
  if(pUFont->isSizeInCache(iSize))
  {
		  XAP_UnixFontHandle * pHndl = new XAP_UnixFontHandle(pUFont, iSize);
		  GdkFont* pFont = pHndl->getGdkFont();
		  GdkFont* pMatchFont=pHndl->getMatchGdkFont();
		  asc = MAX(pFont->ascent, pMatchFont->ascent);
		  delete pHndl;
  }
  else
  {
		  GlobalFontInfo * gfi = psfnt->getMetricsData()->gfi;
		  UT_ASSERT(gfi);
		  asc = (UT_uint32) ( (double) gfi->fontBBox.ury * (double) psfnt->getSize() /1000.);
  }
  return asc;
#endif
}

/* This function does not expect in return the font ascent,
   it expects the font bbox.ury. Chema */
UT_uint32 XAP_UnixGnomePrintGraphics::getFontAscent()
{
	UT_uint32 asc;
	asc = getFontAscent(static_cast<GR_Font *>(m_pCurrentPSFont));
	return asc;
}

UT_uint32 XAP_UnixGnomePrintGraphics::getFontDescent(GR_Font *fnt)
{
#ifdef USE_XFT
	PSFont * psfnt = static_cast<PSFont *>(fnt);
	XAP_UnixFont* pUFont = psfnt->getUnixFont();
	UT_sint32 iSize = psfnt->getSize();
	return (UT_uint32) (pUFont->getAscender(iSize) + 0.5);
#else
	UT_uint32 des;

#if 0
	GnomeFont * gfnt = _allocGnomeFont(static_cast<PSFont*>(fnt));
	const GnomeFontFace *face;
    const ArtDRect *bbox;

	face = gnome_font_get_face (gfnt);
	bbox = 	gnome_font_face_get_stdbbox (face);

	des = (gint) (bbox->y0 * gnome_font_get_size (gfnt) / 10);
	des *= -1;
	gnome_font_unref (gfnt);
#endif

	PSFont * psfnt = static_cast<PSFont *>(fnt);

	XAP_UnixFont * pUFont = psfnt->getUnixFont();
	UT_sint32 iSize = psfnt->getSize();
	if(pUFont->isSizeInCache(iSize))
	{
		XAP_UnixFontHandle * pHndl = new XAP_UnixFontHandle(pUFont, iSize);
		GdkFont* pFont = pHndl->getGdkFont();
		GdkFont* pMatchFont=pHndl->getMatchGdkFont();
		des = MAX(pFont->descent, pMatchFont->descent);
		delete pHndl;
	}
	else
	{
		GlobalFontInfo * gfi = psfnt->getMetricsData()->gfi;
		UT_ASSERT(gfi);
		des = (UT_uint32) ((double) -(gfi->fontBBox.lly) * (double) psfnt->getSize() / 1000.);
	}
	return des;
#endif
}

UT_uint32 XAP_UnixGnomePrintGraphics::getFontDescent()
{
	UT_uint32 des;
#if 0
	const GnomeFontFace *face;
	const ArtDRect *bbox;
	GnomeFont *font;

	font = m_pCurrentFont;
		
        UT_ASSERT(GNOME_IS_FONT (font));
	face = gnome_font_get_face (font);
        UT_ASSERT(GNOME_IS_FONT_FACE (face));
	bbox = 	gnome_font_face_get_stdbbox (face);

	des = (gint) (bbox->y0 * gnome_font_get_size (font) / 10);
	des *= -1;
#endif

	des = getFontDescent(static_cast<GR_Font *>(m_pCurrentPSFont));
	return des;
}

UT_uint32 XAP_UnixGnomePrintGraphics::getFontHeight()
{
	UT_ASSERT(m_pCurrentFont);
	UT_sint32 height = getFontAscent() + getFontDescent();
	return height;
}

UT_uint32 XAP_UnixGnomePrintGraphics::getFontHeight(GR_Font *fnt)
{
	UT_sint32 height = getFontAscent(fnt) + getFontDescent(fnt);
	return height;
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
	XAP_UnixFontHandle * item = NULL;
	
//
// This piece of code scales the FONT chosen at low resolution to that at high
// resolution. This fixes bug 1632 and other non-WYSIWYG behaviour.
//
	UT_uint32 iSize = getAppropriateFontSizeFromString(pszFontSize);
	if (unixfont)
	{
		// end up burying a pointer to the unix font, but first we need a handle
		item = new XAP_UnixFontHandle(unixfont,iSize);
	}
	else
	{
		// Oops!  We don't have that font here.  substitute something
		// we know we have (get smarter about this later)
		g_print ("Get times new roman !!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n"
				 "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
		item = new XAP_UnixFontHandle(m_fm->getFont("Times New Roman", s),iSize);
	}
	xxx_UT_DEBUGMSG(("Using Gnome-Print PS Font Size %d \n",iSize));
	PSFont * pFont = new PSFont(item->getUnixFont(), iSize);
    delete item;
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
