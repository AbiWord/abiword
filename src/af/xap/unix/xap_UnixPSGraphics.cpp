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
#include <locale.h>

#ifdef USE_XFT
#include <freetype/ftsnames.h>
#include <freetype/ttnameid.h>
#endif

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_misc.h"
#include "ut_wctomb.h"
#include "xap_UnixDialogHelper.h"
#include "ut_endian.h"

#include "xap_UnixPSGenerate.h"
#include "xap_UnixPSGraphics.h"
#include "xap_UnixPSFont.h"
#include "xap_UnixPSImage.h"

#include "xap_UnixFont.h"
#include "xap_UnixFontManager.h"
#include "xap_UnixFontXLFD.h"
#include "xap_EncodingManager.h"

#ifdef BIDI_ENABLED
#include "ut_OverstrikingChars.h"
#endif

#include "xap_Prefs.h"
#include "xap_App.h"

// the resolution that we report to the application (pixels per inch).
#define PS_RESOLUTION		7200

/*
	How long line in the PS output we will allow;
	DSC3.0 limits to 256, but we may need to print a few extra
	characters after we reach the end of our buffer
*/
#define OUR_LINE_LIMIT		220			
#define LINE_BUFFER_SIZE   OUR_LINE_LIMIT + 30

#if (1 && (!defined(WITH_PANGO) || !defined(USE_XFT)))
#include <gdk/gdkprivate.h>
static bool isFontUnicode(GdkFont *font)
{
//	GdkFontPrivate *font_private = (GdkFontPrivate*) font;
//	XFontStruct *xfont = (XFontStruct *) font_private->xfont;

//	return ((xfont->min_byte1 == 0) || (xfont->max_byte1 == 0));
	return false;
}
#endif

static
UT_uint32 _scaleFont(PSFont * pFont, UT_uint32 units)
{
#ifdef USE_XFT
	XftFont* pXftFont = pFont->getUnixFont()->getXftFont(pFont->getSize());
	XftFaceLocker locker(pXftFont);

	UT_uint32 retval = units * pFont->getSize() / locker.getFace()->units_per_EM;
	UT_DEBUGMSG(("_scaleFont(%u) -> %u\n", units, retval));

	return retval;
#else
	return (units * pFont->getSize() / 1000); 
#endif
}

/*****************************************************************
******************************************************************
** This file contains the Interface between the application and
** the PostScript generation code.
******************************************************************
*****************************************************************/

PS_Graphics::PS_Graphics(const char * szFilename,
						 const char * szTitle,
						 const char * szSoftwareNameAndVersion,
						 XAP_UnixFontManager * fontManager,						 
						 bool	  bIsFile,
						 XAP_App *pApp)
{
	UT_ASSERT(szFilename && *szFilename);
	m_pApp = pApp;
	m_szFilename = szFilename;
	m_szTitle = szTitle;
	m_szSoftwareNameAndVersion = szSoftwareNameAndVersion;
	m_pCurrentFont = 0;
	m_bStartPrint = false;
	m_bStartPage = false;
	m_bNeedStroked = false;
	m_bIsFile = bIsFile;
	m_ps = 0;

	m_cs = GR_Graphics::GR_COLORSPACE_COLOR;
	
	m_fm = fontManager;

	m_currentColor.m_red = 0;
	m_currentColor.m_grn = 0;
	m_currentColor.m_blu = 0;
	m_iPageCount = 0;
}

PS_Graphics::~PS_Graphics()
{
	// TODO free stuff
	// UT_VECTOR_PURGEALL(PSFont*, m_vecFontList);
	FREEP(m_szPageSizeName);
}

bool PS_Graphics::queryProperties(GR_Graphics::Properties gp) const
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

#ifndef WITH_PANGO
void PS_Graphics::setFont(GR_Font* pFont)
{
	UT_ASSERT(pFont);
	PSFont * pNewFont = (static_cast<PSFont*> (pFont));

	// TODO Not always what we want, i.e., start of a new page.
	// TODO I added a call directly to _startPage to call _emit_SetFont();
	// TODO I would rather do it all here.
	if (pNewFont == m_pCurrentFont ) 
		return;

	UT_ASSERT(pFont);

	m_pCurrentFont = pNewFont;
	if (m_ps)
		_emit_SetFont();
}

UT_uint32 PS_Graphics::getFontAscent(GR_Font * fnt)
{
#ifdef USE_XFT
	PSFont*				psfnt = static_cast<PSFont*> (fnt);
	return (UT_uint32) (psfnt->getUnixFont()->getAscender(psfnt->getSize()) + 0.5);
#else
  PSFont * psfnt = static_cast<PSFont *>(fnt);
  PSFont *pEnglishFont;
  PSFont *pChineseFont;
  _explodePSFonts(psfnt, pEnglishFont,pChineseFont);
  UT_ASSERT(pEnglishFont && pChineseFont);
  int e,c;
  GlobalFontInfo * gfi = pEnglishFont->getMetricsData()->gfi;
  UT_ASSERT(gfi);

//#if 0
//  e = _scaleFont(psfnt, gfi->ascender);
  //#if 0
  //  e = _scaleFont(psfnt, gfi->fontBBox.ury);
  //#endif
    

  XAP_UnixFont * pUFont = psfnt->getUnixFont();
  UT_sint32 iSize = psfnt->getSize();
  if(pUFont->isSizeInCache(iSize))
  {
	XAP_UnixFontHandle * pHndl = new XAP_UnixFontHandle(pUFont, iSize);
	GdkFont* pFont = pHndl->getGdkFont();
	GdkFont* pMatchFont=pHndl->getMatchGdkFont();
	e = MAX(pFont->ascent, pMatchFont->ascent);
	delete pHndl;
  }
  else
  {
	e = _scaleFont(psfnt, gfi->fontBBox.ury);
  }

  c= pChineseFont == pEnglishFont ? 0 :_scaleFont(psfnt, pChineseFont->getUnixFont()->get_CJK_Ascent());
  return MAX(e,c);
#endif
}

UT_uint32 PS_Graphics::getFontAscent()
{
  return getFontAscent(m_pCurrentFont);
}

UT_uint32 PS_Graphics::getFontDescent(GR_Font * fnt)
{
#ifdef USE_XFT
	PSFont*				psfnt = static_cast<PSFont*> (fnt);
	return (UT_uint32) (psfnt->getUnixFont()->getDescender(psfnt->getSize()) + 0.5);
#else
  PSFont * psfnt = static_cast<PSFont *>(fnt);
  PSFont *pEnglishFont;
  PSFont *pChineseFont;
  _explodePSFonts(psfnt, pEnglishFont,pChineseFont);
  UT_ASSERT(pEnglishFont && pChineseFont);
  int e,c;
  GlobalFontInfo * gfi = pEnglishFont->getMetricsData()->gfi;
  UT_ASSERT(gfi);

  XAP_UnixFont * pUFont = psfnt->getUnixFont();
  UT_sint32 iSize = psfnt->getSize();
  if(pUFont->isSizeInCache(iSize))
  {
	XAP_UnixFontHandle * pHndl = new XAP_UnixFontHandle(pUFont, iSize);
	GdkFont* pFont = pHndl->getGdkFont();
	GdkFont* pMatchFont=pHndl->getMatchGdkFont();
	e = MAX(pFont->descent, pMatchFont->descent);
	delete pHndl;
  }
  else
  {
	e = _scaleFont(psfnt, -(gfi->fontBBox.lly));
  }

  c= pChineseFont == pEnglishFont ? 0 :_scaleFont(psfnt, pChineseFont->getUnixFont()->get_CJK_Descent());
  return MAX(e,c);
#endif
}

UT_uint32 PS_Graphics::getFontDescent()
{
  return getFontDescent(m_pCurrentFont);
}


void PS_Graphics::getCoverage(UT_Vector& coverage)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}


UT_uint32 PS_Graphics::getFontHeight(GR_Font * fnt)
{
	UT_sint32 height = getFontAscent(fnt) + getFontDescent(fnt);

#if 0
	UT_DEBUGMSG(("SEVIOR: Font height in PS-print = %d \n",height));
	PSFont * pFont = static_cast<PSFont *>(fnt);
	XAP_UnixFont *uf          = pFont->getUnixFont();
	char *abi_name            = (char*)uf->getName();
	UT_DEBUGMSG(("SEVIOR: Font size in PS-print = %d Font Name %s \n",pFont->getSize(),abi_name));
#endif

	return height;
}

UT_uint32 PS_Graphics::getFontHeight()
{
	UT_sint32 height = getFontAscent(m_pCurrentFont) + getFontDescent(m_pCurrentFont);
	return height;
}
	
UT_uint32 PS_Graphics::measureUnRemappedChar(const UT_UCSChar c)
{
#ifdef USE_XFT
 	UT_ASSERT(m_pCurrentFont);
	return (UT_uint32) (m_pCurrentFont->getUnixFont()->measureUnRemappedChar(c, m_pCurrentFont->getSize()) + 0.5);
#else
 	UT_ASSERT(m_pCurrentFont);
//
// If the font is in cache we're doing layout calculations so use exactly
// the same calculations as the screen uses.
//
	XAP_UnixFont * pUFont = m_pCurrentFont->getUnixFont();
	UT_sint32 iSize = m_pCurrentFont->getSize();

	PSFont *pEnglishFont;
	PSFont *pChineseFont;
	_explodePSFonts(m_pCurrentFont, pEnglishFont,pChineseFont);
	
	if (XAP_EncodingManager::get_instance()->is_cjk_letter(c))
	    return _scale(pChineseFont->getUnixFont()->get_CJK_Width());
	else
	{	
		if(pUFont->isSizeInCache(iSize))
		{
			UT_UCSChar Wide_char = c;
			
			XAP_UnixFontHandle * pHndl = new XAP_UnixFontHandle(pUFont, iSize);
			GdkFont* font = pHndl->getGdkFont();
			
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

			delete pHndl;
		}
		else
		{
			UT_uint32 width =_scale(pEnglishFont->getCharWidth(c));
			return width;
		}
	}
#endif
}
#if 0
/*
    WARNING: this code doesn't support non-latin1 chars.
*/
UT_uint32 PS_Graphics::measureString(const UT_UCSChar* s, int iOffset, 
									int num, unsigned short* pWidths)
{
	UT_ASSERT(m_pCurrentFont);
	UT_ASSERT(s);
	const UT_UCSChar * p = s+iOffset;
	const UT_uint16 * cwi = m_pCurrentFont->getUniWidths();

	UT_uint32 iCharWidth = 0;
	for (int k=0; k<num; k++)
	{
		//UT_ASSERT(p[k] < 256);			// TODO deal with Unicode
	
    	register int x;
		UT_UCSChar currentChar;
		currentChar = remapGlyph(p[k], false);
		x = (currentChar < 256 ? _scale(cwi[currentChar]) : 0;
		
		iCharWidth += x;
		pWidths[k] = x;
	}
		
	return iCharWidth;
}
#endif
#endif //#ifndef WITH_PANGO


UT_uint32 PS_Graphics::_getResolution(void) const
{
	return PS_RESOLUTION;
}

void PS_Graphics::setColor(const UT_RGBColor& clr)
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

	_emit_SetColor();
}

void PS_Graphics::getColor(UT_RGBColor& clr)
{
	return m_currentColor;
}

GR_Font* PS_Graphics::getGUIFont()
{
	// getGUIFont is only used for drawing UI widgets, which does not apply on paper.
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

	return NULL;
}

#ifdef USE_XFT
/**
 * Finds a font which match the family, style, variant, weight and size
 * asked.  It will do a fuzzy match to find the font (using the aliases
 * found in fonts.conf
 */
GR_Font * PS_Graphics::findFont(const char* pszFontFamily,
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

GR_Font* PS_Graphics::findFont(const char* pszFontFamily, 
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
		// Make a handle on the unixfont.
		item = new XAP_UnixFontHandle(unixfont,iSize);
	}
	else
	{
		// Oops!  We don't have that font here.  substitute something
		// we know we have (get smarter about this later)
		item = new XAP_UnixFontHandle(m_fm->getFont("Times New Roman", s),iSize);
	}

	xxx_UT_DEBUGMSG(("SEVIOR: Using PS Font Size %d \n",iSize));
	PSFont * pFont = new PSFont(item->getUnixFont(), iSize);
	UT_ASSERT(pFont);
    delete item;

	// Here we do something different from gr_UnixGraphics::setFont().
	// The PostScript context keeps a simple vector of all its fonts,
	// so that it can run through them and dump them into a document.
	UT_uint32 k, count;
	for (k = 0, count = m_vecFontList.getItemCount(); k < count; k++)
	{
		PSFont * psf = (PSFont *) m_vecFontList.getNthItem(k);
		UT_ASSERT(psf);
		// is this good enough for a match?
		if (!strcmp(psf->getUnixFont()->getFontKey(),pFont->getUnixFont()->getFontKey()) &&		
			psf->getSize() == pFont->getSize())
		{
			// don't return the one in the vector, even though
			// it matches, but return the copy, since they're
			// disposable outside our realm.
			pFont->setIndex(psf->getIndex());
			return pFont;
		}
	}

	// wasn't already there, add it
	m_vecFontList.addItem((void *) pFont);

	// it's always the last in the list
	UT_uint32 n = m_vecFontList.getItemCount() - 1;
	pFont->setIndex(n);
	
	return pFont;
}
#endif // USE_XFT

#ifndef WITH_PANGO
void PS_Graphics::drawGlyph(UT_uint32 Char, UT_sint32 xoff, UT_sint32 yoff)
{
	/* we should not reach this point */
	UT_ASSERT(false);
}

void PS_Graphics::drawChars(const UT_UCSChar* pChars, int iCharOffset,
							int iLength, UT_sint32 xoff, UT_sint32 yoff,
							int * pCharWidths)
{
	PSFont *pEnglishFont;
	PSFont *pChineseFont;
	_explodePSFonts(m_pCurrentFont, pEnglishFont,pChineseFont);
	UT_ASSERT(pEnglishFont && pChineseFont);

	const UT_UCSChar *pS;
	const UT_UCSChar *pE=pChars+iCharOffset;
	const UT_UCSChar *pEnd=pChars+iCharOffset+iLength;
	UT_sint32 xS;
	do
	{
		for(pS=pE,xS=xoff; pE<pEnd && XAP_EncodingManager::get_instance()->is_cjk_letter(*pE); ++pE)
			xoff+=_scale(pChineseFont->getUnixFont()->get_CJK_Width());
		if(pE>pS)
		{
			_emit_SetFont(pChineseFont);
			_drawCharsCJK(pS,0,pE-pS,xS,yoff);
		}
#ifdef BIDI_ENABLED
		bool font_emitted = false;
		for(pS=pE,xS=xoff; pE<pEnd && !XAP_EncodingManager::get_instance()->is_cjk_letter(*pE) && (UT_isOverstrikingChar(*pE) == UT_NOT_OVERSTRIKING); ++pE)
#else
		for(pS=pE,xS=xoff; pE<pEnd && !XAP_EncodingManager::get_instance()->is_cjk_letter(*pE); ++pE)
#endif
		xoff += _scale(pEnglishFont->getCharWidth(remapGlyph(*pE,/**pS > 0xff*/0)));
		if(pE>pS)
		{
			_emit_SetFont(pEnglishFont);
#ifdef BIDI_ENABLED
			font_emitted = true;
#endif

			UT_DEBUGMSG(("ARRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRRR\n"));
			
			// What??!!  That means that if we don't use an unicode locale,
			// we can not print accented characters!  Why??
#ifndef USE_XFT
			if(XAP_EncodingManager::get_instance()->isUnicodeLocale())
				_drawCharsUTF8(pS,0,pE-pS,xS,yoff);
			else
				_drawCharsNonCJK(pS,0,pE-pS,xS,yoff);
#else
			// in the Xft code I always use the UTF8 variant (that basically works)
			_drawCharsUTF8(pS, 0, pE - pS, xS, yoff);
#endif
		}
#ifdef BIDI_ENABLED
		for(pS=pE,xS=xoff; pE<pEnd && !XAP_EncodingManager::get_instance()->is_cjk_letter(*pE) && (UT_isOverstrikingChar(*pE) != UT_NOT_OVERSTRIKING); ++pE)
			;
		if(pE>pS)
		{
			if(!font_emitted)
				_emit_SetFont(pEnglishFont);
			_drawCharsOverstriking(pS,0,pE-pS,xS,yoff);
		}
#endif
	}
  	while(pE<pEnd);
}

#ifdef BIDI_ENABLED
void PS_Graphics::_drawCharsOverstriking(const UT_UCSChar* pChars, UT_uint32 iCharOffset,
							 UT_uint32 iLength, UT_sint32 xoff, UT_sint32 yoff)
{
	const encoding_pair*  enc = 0;
	UT_AdobeEncoding* ae = 0;
	
	UT_ASSERT(m_pCurrentFont);

	enc = m_pCurrentFont->getUnixFont()->loadEncodingFile();
	if(enc)
	{
		ae = new UT_AdobeEncoding(enc, m_pCurrentFont->getUnixFont()->getEncodingTableSize());
	}
	else
	{
		UT_DEBUGMSG(("UnixPS_Graphics: no encoding available!\n"));
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
	
	// The GR classes are expected to take yoff as the upper-left of
	// each glyph.  PostScript interprets the yoff as the baseline,
	// which doesn't match this expectation.  Adding the ascent of the
	// font will bring it back to the correct position.
	yoff += getFontAscent();

	// unsigned buffer holds Latin-1 data to character code 255
	char buf[LINE_BUFFER_SIZE]; //some extra space at the end
	char * pD = buf;

	const UT_UCSChar * pS = pChars+iCharOffset;
	const UT_UCSChar * pEnd = pS+iLength;
	UT_UCSChar currentChar;

	//when printing 8-bit chars we enclose them in brackets, but 16-bit
	//chars must be printed by name without brackets
	
	bool open_bracket = false;
	bool using_names = false;
	
	for(; pS < pEnd; pS++)
	{
		if (pD-buf > OUR_LINE_LIMIT)
		{
			if(!using_names)
				*pD++ = '\\';
			*pD++ = '\n';
			*pD++ = 0;
			m_ps->writeBytes(buf);
			pD = buf;
		}

		currentChar = remapGlyph(*pS, false);
		if(currentChar > 255)
		{
			if(open_bracket)
			{
				open_bracket = false;
				sprintf((char *) pD,") %d %d MS\n",xoff,yoff);
				m_ps->writeBytes(buf);
				pD = buf;
			}
			else if(!using_names)
			{
				sprintf((char *) pD," %d %d MV ",xoff,yoff);
				pD = buf + strlen(buf);
				using_names = true;
			}

			// we don't properly handle double mappings:
			// http://partners.adobe.com/asn/developer/type/unicodegn.html#4
			const char * glyph = ae->ucsToAdobe(currentChar);
			// ' /glyph GS '
			if(pD - buf + strlen(glyph) + 6 > OUR_LINE_LIMIT)
			{
				//*pD++ = '\\';
				*pD++ = '\n';
				*pD++ = 0;
				m_ps->writeBytes(buf);
				pD = buf;
			}
				
			*pD++ = ' ';
			*pD++ = '/';
			strcpy(pD, (const char*)glyph);
			pD += strlen(glyph);
			strcpy(pD, " GS ");
			pD += 4;
		}
		else    // currentChar < 255
		{
			if(!open_bracket)
			{
				*pD++ = '(';
				open_bracket = true;
				using_names = false;
			}
			
		    switch (currentChar)
		    {
				case 0x08:		*pD++ = '\\';	*pD++ = 'b';	break;
				case UCS_TAB:	*pD++ = '\\';	*pD++ = 't';	break;
				case UCS_LF:	*pD++ = '\\';	*pD++ = 'n';	break;
				case UCS_FF:	*pD++ = '\\';	*pD++ = 'f';	break;
				case UCS_CR:	*pD++ = '\\';	*pD++ = 'r';	break;
				case '\\':		*pD++ = '\\';	*pD++ = '\\';	break;
				case '(':		*pD++ = '\\';	*pD++ = '(';	break;
				case ')':		*pD++ = '\\';	*pD++ = ')';	break;
				default:		*pD++ = (char)currentChar; 	break;
	    	}
		}
	}
	if(open_bracket)
	{
		*pD++ = ')';
		sprintf((char *) pD," %d %d MS\n",xoff,yoff);
	}
	else
	{
		*pD++ = '\n';
		*pD++ = 0;
	}
			
	m_ps->writeBytes(buf);
	if(ae)
		delete ae;
	
}
#endif
							 
static UT_Wctomb* pWctomb = NULL;

void PS_Graphics::_drawCharsCJK(const UT_UCSChar* pChars, UT_uint32 iCharOffset,
							 UT_uint32 iLength, UT_sint32 xoff, UT_sint32 yoff)
{
	UT_DEBUGMSG(("_drawCharsCJK\n"));
	if (!pWctomb)
	    pWctomb = new UT_Wctomb;
	else
	    pWctomb->initialize();
	
	UT_ASSERT(m_pCurrentFont);

	// The GR classes are expected to take yoff as the upper-left of
	// each glyph.  PostScript interprets the yoff as the baseline,
	// which doesn't match this expectation.  Adding the ascent of the
	// font will bring it back to the correct position.
	yoff += getFontAscent();

	// unsigned buffer holds Latin-1 data to character code 255
	char buf[LINE_BUFFER_SIZE];
	char * pD = buf;

	const UT_UCSChar * pS = pChars+iCharOffset;
	const UT_UCSChar * pEnd = pS+iLength;
	char _bytes[30];
	int _bytes_len;

	//when printing 8-bit chars we enclose them in brackets, but 16-bit
	//chars must be printed by name without brackets
	
	bool open_bracket = false;
	bool using_names = false;
	while (pS<pEnd)
	{
		if (pD-buf > OUR_LINE_LIMIT)
		{
			if(!using_names)
				*pD++ = '\\';
			*pD++ = '\n';
			*pD++ = 0;
			m_ps->writeBytes(buf);
			pD = buf;
		}

		if(!open_bracket)
		{
			*pD++ = '(';
			open_bracket = true;
			using_names = false;
		}
		pWctomb->wctomb_or_fallback(_bytes,_bytes_len,*pS);
		if (pD+_bytes_len-buf > OUR_LINE_LIMIT)
		{
			if(!using_names)
			    *pD++ = '\\';
		    *pD++ = '\n';
		    *pD++ = 0;
		    m_ps->writeBytes(buf);
		    pD = buf;
		}
		for(int i = 0; i < _bytes_len; ++i)
		{
			char c = _bytes[i];
			switch (c)
			{
				case '\\': *pD++ = '\\'; *pD++ = '\\'; break;
				case '(' : *pD++ = '\\'; *pD++ = '(';  break;
				case ')' : *pD++ = '\\'; *pD++ = ')';  break;
				default:   *pD++ = (char)c;     break;
			}
		}
		pS++;
	}
	if(open_bracket)
	{
		*pD++ = ')';
		sprintf((char *) pD," %d %d MS\n",xoff,yoff);
	}
	else
	{
		*pD++ = '\n';
		*pD++ = 0;
	}
			
	m_ps->writeBytes(buf);
}

void PS_Graphics::_drawCharsNonCJK(const UT_UCSChar* pChars, UT_uint32 iCharOffset,
							 UT_uint32 iLength, UT_sint32 xoff, UT_sint32 yoff)
{
	UT_DEBUGMSG(("_drawCharsNonCJK\n"));
	if (!pWctomb)
	    pWctomb = new UT_Wctomb;
	else
	    pWctomb->initialize();
	
	UT_ASSERT(m_pCurrentFont);

	
	// The GR classes are expected to take yoff as the upper-left of
	// each glyph.  PostScript interprets the yoff as the baseline,
	// which doesn't match this expectation.  Adding the ascent of the
	// font will bring it back to the correct position.
	yoff += getFontAscent();

	// unsigned buffer holds Latin-1 data to character code 255
	char buf[LINE_BUFFER_SIZE];
	char * pD = buf;

	const UT_UCSChar * pS = pChars+iCharOffset;
	const UT_UCSChar * pEnd = pS+iLength;
	UT_UCSChar currentChar;

	//when printing 8-bit chars we enclose them in brackets, but 16-bit
	//chars must be printed by name without brackets
	
	bool open_bracket = false;
	bool using_names = false;
	while (pS<pEnd)
	{
		if (pD-buf > OUR_LINE_LIMIT)
		{
			if(!using_names)
				*pD++ = '\\';
			*pD++ = '\n';
			*pD++ = 0;
			m_ps->writeBytes(buf);
			pD = buf;
		}

		// TODO deal with Unicode issues.
		if(!open_bracket)
		{
			*pD++ = '(';
			open_bracket = true;
			using_names = false;
		}
	    currentChar = remapGlyph(*pS, *pS >= 256 ? true : false);
	    currentChar = currentChar <= 0xff ? currentChar : XAP_EncodingManager::get_instance()->UToNative(currentChar);
	    switch (currentChar)
	    {
			case 0x08:		*pD++ = '\\';	*pD++ = 'b';	break;
			case UCS_TAB:	*pD++ = '\\';	*pD++ = 't';	break;
			case UCS_LF:	*pD++ = '\\';	*pD++ = 'n';	break;
			case UCS_FF:	*pD++ = '\\';	*pD++ = 'f';	break;
			case UCS_CR:	*pD++ = '\\';	*pD++ = 'r';	break;
			case '\\':		*pD++ = '\\';	*pD++ = '\\';	break;
			case '(':		*pD++ = '\\';	*pD++ = '(';	break;
			case ')':		*pD++ = '\\';	*pD++ = ')';	break;
			default:		*pD++ = (char)currentChar; 	break;
	    }
		pS++;
	}
	if(open_bracket)
	{
		*pD++ = ')';
		sprintf((char *) pD," %d %d MS\n",xoff,yoff);
	}
	else
	{
		*pD++ = '\n';
		*pD++ = 0;
	}
			
	m_ps->writeBytes(buf);
}

void PS_Graphics::_drawCharsUTF8(const UT_UCSChar* pChars, UT_uint32 iCharOffset,
							 UT_uint32 iLength, UT_sint32 xoff, UT_sint32 yoff)
{
	UT_DEBUGMSG(("_drawCharsUTF8\n"));
	const encoding_pair*  enc = 0;
	UT_AdobeEncoding* ae = 0;
	
	UT_ASSERT(m_pCurrentFont);

	enc = m_pCurrentFont->getUnixFont()->loadEncodingFile();
	if(enc)
		ae = new UT_AdobeEncoding(enc, m_pCurrentFont->getUnixFont()->getEncodingTableSize());
	else
	{
		UT_DEBUGMSG(("UnixPS_Graphics: no encoding available!\n"));
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
	
	// The GR classes are expected to take yoff as the upper-left of
	// each glyph.  PostScript interprets the yoff as the baseline,
	// which doesn't match this expectation.  Adding the ascent of the
	// font will bring it back to the correct position.
	yoff += getFontAscent();

	// unsigned buffer holds Latin-1 data to character code 255
	char buf[LINE_BUFFER_SIZE];
	char * pD = buf;

	const UT_UCSChar * pS = pChars+iCharOffset;
	const UT_UCSChar * pEnd = pS+iLength;
	UT_UCSChar currentChar;

	//when printing 8-bit chars we enclose them in brackets, but 16-bit
	//chars must be printed by name without brackets
	
	bool open_bracket = false;
	bool using_names = false;
	while (pS<pEnd)
	{
		if (pD-buf > OUR_LINE_LIMIT)
		{
			if(!using_names)
				*pD++ = '\\';
			*pD++ = '\n';
			*pD++ = 0;
			m_ps->writeBytes(buf);
			pD = buf;
		}

		currentChar = remapGlyph(*pS, false);
		if (!((currentChar >= 'a' && currentChar <= 'z') || (currentChar >= 'A' && currentChar <= 'Z') ||
			  currentChar == ' '))
		{
			// if currentChar is not an english character, we will have to write it
			// using parentheses
			if(open_bracket)
			{
				open_bracket = false;
				sprintf((char *) pD,") %d %d MS\n",xoff,yoff);
				m_ps->writeBytes(buf);
				pD = buf;
			}
			else if(!using_names)
			{
				sprintf((char *) pD," %d %d MV ",xoff,yoff);
				pD = buf + strlen(buf);
				using_names = true;
			}
				
			// we don't properly handle double mappings:
			// http://partners.adobe.com/asn/developer/type/unicodegn.html#4
			const char * glyph = ae->ucsToAdobe(currentChar);

			// ' /glyph GS '
			if(pD - buf + strlen(glyph) + 6 > OUR_LINE_LIMIT)
			{
				//*pD++ = '\\';
				*pD++ = '\n';
				*pD++ = 0;
				m_ps->writeBytes(buf);
				pD = buf;
			}
				
			*pD++ = ' ';
			*pD++ = '/';
			strcpy(pD, (const char*)glyph);
			pD += strlen(glyph);
			strcpy(pD, " GS ");
			pD += 4;
		}
		else
		{
			UT_DEBUGMSG(("char < 255\n"));

			if(!open_bracket)
			{
				*pD++ = '(';
				open_bracket = true;
				using_names = false;
			}

#if 0			
		    switch (currentChar)
		    {
				case 0x08:		*pD++ = '\\';	*pD++ = 'b';	break;
				case UCS_TAB:	*pD++ = '\\';	*pD++ = 't';	break;
				case UCS_LF:	*pD++ = '\\';	*pD++ = 'n';	break;
				case UCS_FF:	*pD++ = '\\';	*pD++ = 'f';	break;
				case UCS_CR:	*pD++ = '\\';	*pD++ = 'r';	break;
				case '\\':		*pD++ = '\\';	*pD++ = '\\';	break;
				case '(':		*pD++ = '\\';	*pD++ = '(';	break;
				case ')':		*pD++ = '\\';	*pD++ = ')';	break;
				default:		*pD++ = (char)currentChar; 	break;
	    	}
#else
			*pD++ = (char)currentChar;
#endif
		}
		pS++;
	}
	if(open_bracket)
	{
		*pD++ = ')';
		sprintf((char *) pD," %d %d MS\n",xoff,yoff);
	}
	else
	{
		*pD++ = '\n';
		*pD++ = 0;
	}
			
	m_ps->writeBytes(buf);
	if(ae)
		delete ae;
}
#endif // #ifndef WITH_PANGO

void PS_Graphics::drawLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2, UT_sint32 y2)
{
	// TODO This is used for lines in the document, as well as underlines
	// TODO and strikes.
	m_bNeedStroked = true;

	// emit a change in line width
	_emit_SetLineWidth();
	
	char buf[LINE_BUFFER_SIZE];
//	UT_sint32 nA = getFontAscent();
	g_snprintf(buf,sizeof (buf),"%d %d %d %d ML\n", x2, y2, x1, y1);
	m_ps->writeBytes(buf);
}

void PS_Graphics::setLineWidth(UT_sint32 iLineWidth)
{
	m_iLineWidth = iLineWidth;
}

void PS_Graphics::xorLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32)
{
}

void PS_Graphics::polyLine(UT_Point * /* pts */, UT_uint32 /* nPoints */)
{
	// polyLine is only used for drawing spell-check squiggles, which does not apply on paper.
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

void PS_Graphics::fillRect(const UT_RGBColor& c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h)
{
  UT_RGBColor cl = m_currentColor;
  setColor(c);

  char buf[256];
  buf[0] = 0;

  sprintf(buf, "%d %d MT\n%d %d LT\n%d %d LT\n%d %d LT\n%d %d LT\nCP\nF\n",
	  x,y,
	  x+w, y,
	  x+w, y+h,
	  x, y+h,
	  x, y
	  );
  m_ps->writeBytes(buf);

  setColor(cl);
}

void PS_Graphics::invertRect(const UT_Rect* /*pRect*/)
{
	// invertRect is only used for drawing the selection region, which does not apply on paper.
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

void PS_Graphics::setClipRect(const UT_Rect* /*pRect*/)
{
    // setClipRect is used for clipping images, even when printing to
    // PostScript.  Just ignore these for now.

    // TODO : Can PostScript clip regions?  Does it make sense for our images?
}

void PS_Graphics::clearArea(UT_sint32 /*x*/, UT_sint32 /*y*/,
							UT_sint32 /*width*/, UT_sint32 /*height*/)
{
	// clearArea is only used for re-drawing edited regions, which does not apply on paper.
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

}

void PS_Graphics::scroll(UT_sint32, UT_sint32)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

void PS_Graphics::scroll(UT_sint32 /* x_dest */,
						 UT_sint32 /* y_dest */,
						 UT_sint32 /* x_src */,
						 UT_sint32 /* y_src */,
						 UT_sint32 /* width */,
						 UT_sint32 /* height */)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

bool PS_Graphics::startPrint(void)
{
	UT_ASSERT(!m_bStartPrint);
	m_bStartPrint = true;
	m_ps = new ps_Generate(m_szFilename);
	if (!m_ps)
		return false;

	return _startDocument();
}

bool PS_Graphics::startPage(const char * szPageLabel, UT_uint32 pageNumber,
							   bool bPortrait, UT_uint32 iWidth, UT_uint32 iHeight)
{
	if (m_bStartPage)
		_endPage();
	m_bStartPage = true;
	m_bNeedStroked = false;
	return _startPage(szPageLabel,pageNumber,bPortrait,iWidth,iHeight);
}

bool PS_Graphics::endPrint(void)
{
	if (m_bStartPage)
		_endPage();
	return _endDocument();
}

/*****************************************************************/
/*****************************************************************/

UT_uint32 PS_Graphics::_scale(UT_uint32 units) const
{
	// we are given a value from the AFM file which are
	// expressed in 1/1000ths of the scaled font.
	// return the number of pixels at our resolution.
	// It's 1/2048 for TrueType fonts.  The Xft code does the right thing.
  
	return _scaleFont(m_pCurrentFont, units);
}

bool PS_Graphics::_startDocument(void)
{
	// open the file and begin writing the prolog
	// return false if an error occurs.

	//////////////////////////////////////////////////////////////////
	// DSC3.0/Prolog/Header
	//////////////////////////////////////////////////////////////////
	
	if (!m_ps->openFile(m_bIsFile))
		return false;

	// we use the argv-version of these so that we get PS-escaping on
	// the strings (which will probably have spaces in them).
	
	if (m_szSoftwareNameAndVersion && *m_szSoftwareNameAndVersion)
		m_ps->formatComment("Creator",&m_szSoftwareNameAndVersion,1);
	if (m_szTitle && *m_szTitle)
		m_ps->formatComment("Title",&m_szTitle,1);
	m_ps->formatComment("Orientation", isPortrait() ? "Portrait" : "Landscape");

	m_ps->formatComment("Pages",m_iPageCount);
	
	//TODO: emit iWidth and iHeight in BoundingBox somehow (what's a 
	//factor between them and PS units (that are 1/72 of inch IIRC)
	m_ps->formatComment("DocumentPaperSizes",m_szPageSizeName);

	_emit_DocumentNeededResources();
	
	// TODO add other header-comments here
	
	m_ps->formatComment("EndComments");

	///////////////////////////////////////////////////////////////////
	// DSC3.0/Prolog/ProcedureDefinitions
	///////////////////////////////////////////////////////////////////
		
	m_ps->formatComment("BeginProlog");

	_emit_PrologMacros();
	_emit_FontMacros();
	
	// TODO add rest of prolog

	m_ps->formatComment("EndProlog");

	///////////////////////////////////////////////////////////////////
	// DSC3.0/Script/DocumentSetup
	///////////////////////////////////////////////////////////////////

	m_ps->formatComment("BeginSetup");

	_emit_IncludeResource();
	
	// TODO add other setup stuff

	m_ps->formatComment("EndSetup");
	return true;
}

bool PS_Graphics::_startPage(const char * szPageLabel, UT_uint32 pageNumber,
								bool bPortrait, UT_uint32 iWidth, UT_uint32 iHeight)
{
	// emit stuff prior to each page
	
	char buf[1024];
	g_snprintf(buf, sizeof(buf),"%d",pageNumber);

	const char * argv[2] = { buf, buf };
	m_ps->formatComment("Page",argv,2);

	// TODO add other page-header comments here

	m_ps->formatComment("BeginPageSetup");

	g_snprintf(buf,sizeof (buf),"%d %d %d %s\n",iWidth,iHeight,PS_RESOLUTION,((bPortrait) ? "BPP" : "BPL"));
	m_ps->writeBytes(buf);
	
	// TODO add page-setup stuff here

	m_ps->formatComment("EndPageSetup");

	// Note, the actual PS for the page will be sent by the
	// individual drawing routings following the EndPageSetup.

	// Note, need to reset font at the beginning of each page
	_emit_SetFont();

	return true;
}

bool PS_Graphics::_endPage(void)
{
	// emit stuff following each page

	m_ps->formatComment("PageTrailer");

	if(m_bNeedStroked)
		m_ps->writeBytes("stroke\n");
	m_ps->writeBytes("EP\n");

	// TODO add any page-trailer stuff here
	// TODO (this inludes an atend's that we used in the page header)

	// Note, either the next page or the document-trailer will follow this.
	return true;
}

bool PS_Graphics::_endDocument(void)
{
	// emit the document trailer

	m_ps->formatComment("Trailer");
	
	// TODO add any trailer stuff here
	// TODO (this includes an atend's that we used in the document header)

	bool bStatus = m_ps->formatComment("EOF");

	m_ps->closeFile();
	delete m_ps;
	m_ps = 0;

	return bStatus;
}

void PS_Graphics::_emit_DocumentNeededResources(void)
{
	UT_Vector vec;
	UT_uint32 k,n;
	UT_uint32 kLimit = m_vecFontList.getItemCount();

    bool bFontKeyword = true;
	for (k=0; k<kLimit; k++)
	{
		PSFont * psf = (PSFont *)m_vecFontList.getNthItem(k);
		if(!psf->getUnixFont()->is_CJK_font())
        {
            if(bFontKeyword)
            {
#ifdef USE_XFT
        		vec.addItem((void *) UT_strdup("font"));
#else
        		vec.addItem((void *) "font");
#endif
                bFontKeyword = false;
            }
            
            // only include each font name once
#ifdef USE_XFT
			UT_String pName(psf->getUnixFont()->getPostscriptName());
			UT_DEBUGMSG(("pName: %s\n", pName.c_str()));
#else	
            const char * pName = psf->getMetricsData()->gfi->fontName;
#endif
            bool bFound = false;
            for(n = 0; n < vec.getItemCount(); n++)
            {
#ifdef USE_XFT
                if(!UT_strcmp(pName.c_str(), (const char *)vec.getNthItem(n)))
#else
                if(!UT_strcmp(pName, (const char *)vec.getNthItem(n)))
#endif
                {
                    bFound = true;
                    break;
                }
            }
            
            if(!bFound)
#ifdef USE_XFT
    		    vec.addItem(UT_strdup(pName.c_str()));
#else
    		    vec.addItem((void*) pName);
#endif
        }
	}

	// TODO add any other resources here
	bool bEmbedFonts;
	XAP_App::getApp()->getPrefsValueBool((const XML_Char *)XAP_PREF_KEY_EmbedFontsInPS, &bEmbedFonts);
	if(bEmbedFonts)
        m_ps->formatComment("DocumentSuppliedResources",&vec);
    else
    	m_ps->formatComment("DocumentNeededResources",&vec);

#ifdef USE_XFT
	UT_VECTOR_FREEALL(char*, vec);
#endif
}

void PS_Graphics::_emit_IncludeResource(void)
{
   	UT_Vector vec;
   	UT_uint32 k,n;
   	UT_uint32 kLimit = m_vecFontList.getItemCount();

	// we want to have a checkbox in the Preferences that would allow
	// to disable splating of the fonts into the output, since people who
	// use Ghostscript my simply register their fonts with GS and do not
	// need them in the document
	bool bEmbedFonts;
	XAP_App::getApp()->getPrefsValueBool((const XML_Char *)XAP_PREF_KEY_EmbedFontsInPS, &bEmbedFonts);
	//UT_DEBUGMSG(("bEmbedFonts: %d\n",bEmbedFonts));

    if(bEmbedFonts)
    {
    	for (k=0; k<kLimit; k++)
    	{
    		char buf[128];

    		PSFont * psf = (PSFont *) m_vecFontList.getNthItem(k);
		
    		// m_ps->formatComment("IncludeResource",psf->getMetricsData()->gfi->fontName);

    		// Instead of including the resources, we actually splat the fonts
    		// into the document.  This looks really slow... perhaps buffer line
    		// by line or in larger chunks the font data.
    		XAP_UnixFont * unixfont = psf->getUnixFont();
		
    		if(unixfont->is_CJK_font())
		      continue;
    		bool match = false;
            const char * pName = unixfont->getFontKey();
    		for(size_t i = 0; i < vec.getItemCount(); ++i)
    		{
				if(!strcmp(pName,(const char*)vec.getNthItem(i)))
    			{
			    	match=true;
    				break;
    			}
    		}
    		if(match)
		    	continue;
            
            vec.addItem((void*)pName);

    		// Make sure the font file will open, maybe it disappeared...
			UT_ASSERT(m_ps);
			if (!unixfont->embedInto(*m_ps))
    		{
				UT_String message("Font data file [");
				message += unixfont->getFontfile();
				message += "cannot be opened for reading!\n"
					"Did it disappear on us?  AbiWord can't print without\n"
					"this file; your PostScript might be missing this resource.";

    			// we don't have any frame info, so we use the non-parented dialog box
    			messageBoxOK(message.c_str());
    			return;
    		}
		
    		// NOTE : here's an internationalization process step.  If the font
    		// NOTE : encoding is NOT "iso8859", we do not emit this macro.
    		// NOTE : this keeps fonts like Standard Symbols, and really
    		// NOTE : any other encoding, from being mangled.  however, it's
    		// NOTE : not intended to guarantee that these other encodings
    		// NOTE : actually work.  that requires more design work.

    		// write findfont
#ifdef USE_XFT
			UT_String stName(psf->getUnixFont()->getPostscriptName());
    		g_snprintf(buf, sizeof (buf), "/%s findfont\n", stName.c_str());
#else
    		g_snprintf(buf, sizeof (buf), "/%s findfont\n", psf->getMetricsData()->gfi->fontName);
#endif
    		m_ps->writeBytes(buf);

#ifndef USE_XFT
    		// Fetch an XLFD object from the font
    		XAP_UnixFontXLFD myXLFD(unixfont->getXLFD());

    		// Compare with iso8859, and emit LAT for that font
    		if (!UT_stricmp(myXLFD.getRegistry(), "iso8859") && !UT_strcmp(myXLFD.getEncoding(), "1"))
				m_ps->writeBytes("LAT\n");
#endif

    		// exec the swapper macro
#ifdef USE_XFT
    		g_snprintf(buf, sizeof (buf), "/%s EXC\n", stName.c_str());
#else
			g_snprintf(buf, sizeof (buf), "/%s EXC\n", psf->getMetricsData()->gfi->fontName);
#endif
    		m_ps->writeBytes(buf);

    	}
    }
	else
	{
		// when not embeding fonts, we have to issue %%IncludeResource statement
		bool bFontKeyword = true;
		const char* pFResource[2] = {"font", NULL};
		for (k=0; k<kLimit; k++)
		{
			PSFont * psf = (PSFont *)m_vecFontList.getNthItem(k);
			if(!psf->getUnixFont()->is_CJK_font())
			{
				if(bFontKeyword)
				{
					vec.addItem((void *) UT_strdup("font"));
					bFontKeyword = false;
				}
            
				// only include each font name once
#ifdef USE_XFT
				UT_String pName(psf->getUnixFont()->getPostscriptName());
#else
				const char * pName = psf->getMetricsData()->gfi->fontName;
#endif
				bool bFound = false;
				for(n = 0; n < vec.getItemCount(); n++)
				{
#ifdef USE_XFT
					if(!UT_strcmp(pName.c_str(), (const char *)vec.getNthItem(n)))
#else
					if(!UT_strcmp(pName, (const char *)vec.getNthItem(n)))
#endif
					{
						bFound = true;
						break;
					}
				}
            
#ifdef USE_XFT
				if(!bFound)
				{
					vec.addItem((void*) UT_strdup(pName.c_str()));
					pFResource[1] = pName.c_str();
					m_ps->formatComment("IncludeResource", pFResource, 2);
				}
#else
				if(!bFound)
				{
					vec.addItem((void*) UT_strdup(pName));
					pFResource[1] = pName;
					m_ps->formatComment("IncludeResource", pFResource, 2);
				}
#endif
			}
		}

		UT_VECTOR_FREEALL(char*, vec);
	}
        
	// TODO add any other IncludeResource's here
}


void PS_Graphics::_emit_PrologMacros(void)
{
	static const char * t[] = {
		"/FSF {findfont exch scalefont} bind def",			// Find & scale a font. <size> /<fontname> FSD
		"/MS  {neg moveto show} bind def",					// Move and draw. (<string>) <x> <y> MS
		"/GS  {glyphshow} bind def",						// show glyph by name
		"/MV  {neg moveto} bind def",						// Move only
		"/BP  {gsave 72 exch div dup scale} bind def",		// Begin Page. <res> BP
		"/SZ  {/HH exch def /WW exch def} bind def",		// Page Size.  <w> <h> SZ
		"/BPP {BP SZ 0 HH translate} bind def",				// Begin Portrait Page.  <w> <h> <res> BPP
		"/BPL {BP SZ 90 rotate} bind def",					// Begin Landscape Page. <w> <h> <res> BPP
		"/EP  {grestore showpage} bind def",				// EP
		"/ML  {neg moveto neg lineto stroke} bind def",		// Move and Line. <x2> <y2> <x1> <y1> ML
		"/LAT {dup length dict copy dup begin",				// Change encoding vector for current font to Latin1
		"       {1 index /FID ne {def} {pop pop} ifelse} forall",
		"       /Encoding ISOLatin1Encoding 256 array copy def",
		"       Encoding 39 /quotesingle put",              // hack for 2313
		"       Encoding 45 /hyphen put",
		"       currentdict",
		"       end} bind def",
		"/EXC {exch definefont pop} bind def",				// Exchange font entry
		"/CP {closepath} bind def",
		"/F {fill} bind def",
		"/LT {neg lineto} bind def",
		"/MT {neg moveto} bind def"
	};

	char buf[1024];
	
	for (unsigned int k=0; k<NrElements(t); k++)
	{
		g_snprintf(buf,sizeof(buf),"  %s\n",t[k]);
		m_ps->writeBytes(buf);
	}
}

void PS_Graphics::_emit_FontMacros(void)
{
	char buf[1024];
	UT_uint32 k;
	UT_uint32 kLimit = m_vecFontList.getItemCount();

	for (k=0; k<kLimit; k++)
	{
		PSFont * psf = (PSFont *)m_vecFontList.getNthItem(k);
#ifdef USE_XFT
		UT_String stName(psf->getUnixFont()->getPostscriptName());
		g_snprintf(buf,sizeof(buf),"  /F%d {%d /%s FSF setfont} bind def\n", k,
				psf->getSize(), stName.c_str());
#else
		g_snprintf(buf,sizeof(buf),"  /F%d {%d /%s FSF setfont} bind def\n", k,
				psf->getSize(), psf->getUnixFont()->is_CJK_font() ?
					psf->getUnixFont()->getFontfile() : 
					psf->getMetricsData()->gfi->fontName);
#endif
		m_ps->writeBytes(buf);
	}
}


void PS_Graphics::_emit_SetFont(void)
{
        _emit_SetFont(m_pCurrentFont);
}

void PS_Graphics::_emit_SetLineWidth(void)
{
	char buf[1024];
	g_snprintf(buf, sizeof(buf), " %d setlinewidth\n", m_iLineWidth);
	m_ps->writeBytes(buf);
}

void PS_Graphics::_emit_SetColor(void)
{
	// NOTE : Depending on the colorspace the user has selected, we emit
	// NOTE : the correct space declarations.

	// We're printing 8 digits of color... do we want to
	// be any more precise, or perhaps less?  8 was a
	// completely arbitrary decision on my part.  :)

	char buf[128];
	// used for any averaging
	unsigned char newclr;

	char * old_locale = setlocale(LC_NUMERIC,"C");
	switch(m_cs)
	{
	case GR_Graphics::GR_COLORSPACE_COLOR:
        g_snprintf(buf, sizeof (buf), "%.8f %.8f %.8f setrgbcolor\n",
                ((float) m_currentColor.m_red / (float) 255.0),
                ((float) m_currentColor.m_grn / (float) 255.0),
                ((float) m_currentColor.m_blu / (float) 255.0));
		break;
	case GR_Graphics::GR_COLORSPACE_GRAYSCALE:
		newclr = (unsigned char) (( (float) 0.30 * m_currentColor.m_red +
									(float) 0.59 * m_currentColor.m_grn +
									(float) 0.11 * m_currentColor.m_blu ));
		g_snprintf(buf, sizeof(buf), "%.8f setgray\n", (float) newclr / (float) 255.0);
		break;
	case GR_Graphics::GR_COLORSPACE_BW:
		// Black & White is a special case of the Gray color space where
		// all colors are 0 (black) and all absence of color is 1 (white)

        if (m_currentColor.m_red + m_currentColor.m_grn + m_currentColor.m_blu > 3*250) // yay, arbitrary threshold!
			g_snprintf(buf,sizeof(buf),"1 setgray\n");
         else		
			 g_snprintf(buf,sizeof(buf),"0 setgray\n");
		break;
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}
	setlocale(LC_NUMERIC,old_locale); // restore original locale
	
	m_ps->writeBytes(buf);
}

 GR_Image* PS_Graphics::createNewImage(const char* pszName, const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight, GR_Image::GRType iType)
{
	GR_Image* pImg = NULL;
   
   	if (iType == GR_Image::GRT_Raster)
     		pImg = new PS_Image(pszName);
   	else if (iType == GR_Image::GRT_Vector)
     		pImg = new GR_VectorImage(pszName);
   
	pImg->convertFromBuffer(pBB, iDisplayWidth, iDisplayHeight);

	return pImg;
}

void PS_Graphics::drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest)
{
   	if (pImg->getType() != GR_Image::GRT_Raster) {
	   pImg->render(this, xDest, yDest);
	   return;
	}
   
   	switch(m_cs)
     	{
       	case GR_Graphics::GR_COLORSPACE_COLOR:
		drawRGBImage(pImg, xDest, yDest);
		break;
      	case GR_Graphics::GR_COLORSPACE_GRAYSCALE:
		drawGrayImage(pImg, xDest, yDest);
		break;
      	case GR_Graphics::GR_COLORSPACE_BW:
		drawBWImage(pImg, xDest, yDest);
		break;
      	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
     }
}
	
void PS_Graphics::drawRGBImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest)
{
	UT_ASSERT(pImg);

	UT_sint32 iDestWidth = pImg->getDisplayWidth();
	UT_sint32 iDestHeight = pImg->getDisplayHeight();
	
	PS_Image * pPSImage = static_cast<PS_Image *>(pImg);

	PSFatmap * image = pPSImage->getData();

	UT_ASSERT(image && image->data);

	// preface with the sizing, position, and scale data
	char buf[128];

	// remember all the context information 
	g_snprintf(buf, sizeof(buf), "gsave\n");
	m_ps->writeBytes(buf);	

	// this is the number of bytes in a "row" of image data, which
	// is image->width times 3 bytes per pixel
	g_snprintf(buf, sizeof(buf),"/rowdata %d string def\n", image->width * 3);
	m_ps->writeBytes(buf);
	
	// translate for quadrant 2, so Y values are negative; land us at
	// lower left of image (baseline), which is twice the height
	g_snprintf(buf, sizeof (buf), "%d %d translate\n", xDest, m_iRasterPosition - yDest  - iDestHeight);
	m_ps->writeBytes(buf);

	g_snprintf(buf, sizeof (buf),"%d %d scale\n", iDestWidth, iDestHeight);
	m_ps->writeBytes(buf);

	// use true image source data dimensions for matrix 
	g_snprintf(buf, sizeof (buf),"%d %d 8 [%d 0 0 %d 0 %d]\n", image->width, image->height,
			image->width, image->height * -1, image->height);
	m_ps->writeBytes(buf);
	
	g_snprintf(buf, sizeof (buf),"{currentfile\n  rowdata readhexstring pop}\nfalse 3\ncolorimage\n");
	m_ps->writeBytes(buf);
	
	// "image" is full of 24 bit data; throw the data specifications
	// and then the raw data (in hexadeicmal) into the file.
	UT_Byte * start = image->data;
	UT_Byte * cursor = NULL;
	UT_Byte * end = start + image->width * image->height * 3; // 3 bytes per pixel
	UT_Byte hexbuf[3];
	int col;
	for (cursor = start, col = 0 ; cursor < end; cursor++, col++)
	{
		// fetch a byte and convert to hex
		g_snprintf((char *) hexbuf, sizeof (hexbuf), "%.2X", *cursor);
		m_ps->writeBytes(hexbuf, 2);

		if (col == 40) // 2 chars per round, 80 columns total
		{
			col = -1;
			g_snprintf((char *) hexbuf, sizeof(hexbuf), "\n");
			m_ps->writeBytes(hexbuf, 1);
		}
	}
	g_snprintf(buf, sizeof(buf), "\n");
	m_ps->writeBytes(buf);

	// recall all that great info
	g_snprintf(buf, sizeof(buf),"grestore\n");
	m_ps->writeBytes(buf);
	
}

void PS_Graphics::drawGrayImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest)
{
	UT_ASSERT(pImg);

	UT_sint32 iDestWidth = pImg->getDisplayWidth();
	UT_sint32 iDestHeight = pImg->getDisplayHeight();
	
	PS_Image * pPSImage = static_cast<PS_Image *>(pImg);

	PSFatmap * image = pPSImage->getData();

	UT_ASSERT(image && image->data);

	// preface with the sizing, position, and scale data
	char buf[128];

	// remember all the context information 
	g_snprintf(buf, sizeof(buf),"gsave\n");
	m_ps->writeBytes(buf);	

	// this is the number of bytes in a "row" of image data, which
	// is image->width in a grayscale image
	g_snprintf(buf, sizeof(buf),"/rowdata %d string def\n", image->width);
	m_ps->writeBytes(buf);
	
	// translate for quadrant 2, so Y values are negative; land us at
	// lower left of image (baseline), which is twice the height
	g_snprintf(buf, sizeof(buf),"%d %d translate\n", xDest, m_iRasterPosition - yDest  - iDestHeight);
	m_ps->writeBytes(buf);

	g_snprintf(buf, sizeof(buf),"%d %d scale\n", iDestWidth, iDestHeight);
	m_ps->writeBytes(buf);

	// use true image source data dimensions for matrix 
	g_snprintf(buf, sizeof(buf),"%d %d 8 [%d 0 0 %d 0 %d]\n", image->width, image->height,
			image->width, image->height * -1, image->height);
	m_ps->writeBytes(buf);

	g_snprintf(buf, sizeof(buf),"{currentfile\n  rowdata readhexstring pop}\nimage\n");
	// color image does:
	// sprintf(buf, "{currentfile\n  rowdata readhexstring pop}\nfalse 3\ncolorimage\n");
	m_ps->writeBytes(buf);
	
	UT_Byte * start = image->data;
	UT_Byte * cursor = NULL;
	// 3 bytes per pixel in original RGB data
	UT_Byte * end = start + image->width * image->height * 3;

	UT_Byte hexbuf[3];
	
	cursor = start;
	UT_uint16 col = 0;
	while (cursor < end)
	{
		/*
		  This is a pretty tight loop, for speed.  We're taking
		  each sample and averaging the R, G, and B values and
		  throwing that (in hex) to the output file.
		*/

		// TODO : Balance these colors!  I don't like the output
		// TODO : I get from a simple average or adding the YIQ
		// TODO : weights.  Look at Netscape for something better.
		
#if 0
		// We can use the Y channel from the YIQ spec, which weights
		// the R, G, and B channels to be perceptually more balanced.
		g_snprintf((char *) hexbuf, sizeof(hexbuf), "%.2X", ( (UT_Byte) ( ( (float) (*cursor++) * (float) (0.299) +
															   (float) (*cursor++) * (float) (0.587) +
															   (float) (*cursor++) * (float) (0.114) ) ) ));
#endif
		g_snprintf((char *) hexbuf, sizeof(hexbuf), "%.2X", ( (UT_Byte) ( ( (float) (*cursor++) * (float) (1) +
															   (float) (*cursor++) * (float) (1) +
															   (float) (*cursor++) * (float) (1) ) /
															 (float) (3.0) )) );

		m_ps->writeBytes(hexbuf, 2);
		if (col == 38)
		{
			m_ps->writeByte((UT_Byte) '\n');
			col = 0;
		}
		else
			col++;
	}
	g_snprintf(buf, sizeof(buf),"\n");
	m_ps->writeBytes(buf);

	// recall all that great info
	g_snprintf(buf, sizeof(buf),"grestore\n");
	m_ps->writeBytes(buf);
	
}
void PS_Graphics::drawBWImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest)
{
	// TODO : Someone do dithering?  Until, we just call grayscale.
	// TODO : The alternative is to half each color (set a threshold at
	// TODO : 50% for each channel), but that would be really ugly.
	drawGrayImage(pImg, xDest, yDest);
}

void PS_Graphics::setColorSpace(GR_Graphics::ColorSpace c)
{
	m_cs = c;
}

GR_Graphics::ColorSpace PS_Graphics::getColorSpace(void) const
{
	return m_cs;
}

void PS_Graphics::setCursor(GR_Graphics::Cursor)
{
	return;
}

GR_Graphics::Cursor PS_Graphics::getCursor(void) const
{
	return GR_CURSOR_INVALID;
}

void PS_Graphics::setColor3D(GR_Color3D /*c*/)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

UT_RGBColor * PS_Graphics::getColor3D(GR_Color3D /*c*/)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return NULL;
}

void PS_Graphics::fillRect(GR_Color3D /*c*/, UT_sint32 /*x*/, UT_sint32 /*y*/, UT_sint32 /*w*/, UT_sint32 /*h*/)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

void PS_Graphics::fillRect(GR_Color3D /*c*/, UT_Rect & /*r*/)
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}


PSFont *PS_Graphics::_findMatchPSFontCJK(PSFont * pFont)
{
  if (!XAP_EncodingManager::get_instance()->cjk_locale())
	return pFont;
  UT_uint32 k, count;
  int s;
  switch(pFont->getUnixFont()->getStyle())
	{
	case XAP_UnixFont::STYLE_NORMAL:
	  s=0;
	  break;
	case XAP_UnixFont::STYLE_BOLD:
	  s=1;
	  break;
	case XAP_UnixFont::STYLE_ITALIC:
	  s=2;
	  break;
	case XAP_UnixFont::STYLE_BOLD_ITALIC:
	  s=3;
	  break;
	default:
	  s=0;
	}
  for (k = 0, count = m_vecFontList.getItemCount(); k < count; k++)
	{
	  PSFont * psf = (PSFont *) m_vecFontList.getNthItem(k);
	  UT_ASSERT(psf);
	  if (!strcmp(psf->getUnixFont()->getFontKey(),
			(pFont->getUnixFont()->is_CJK_font() ? 
			    XAP_UnixFont::s_defaultNonCJKFont[s] : 
			    XAP_UnixFont::s_defaultCJKFont[s])->getFontKey()) &&
		  psf->getSize() == pFont->getSize())
		return psf;
	}  
  PSFont * p = new PSFont(pFont->getUnixFont()->is_CJK_font() ? 
          XAP_UnixFont::s_defaultNonCJKFont[s] : 
	  XAP_UnixFont::s_defaultCJKFont[s] , pFont->getSize() );
  UT_ASSERT(p);
  m_vecFontList.addItem((void *) p);
  p->setIndex(m_vecFontList.getItemCount() - 1);
  return p;
};

void PS_Graphics::_emit_SetFont(PSFont *pFont)
{
  if ( pFont )
    {
      char buf[1024];
      g_snprintf(buf, 1024, "F%d\n", pFont->getIndex());
      m_ps->writeBytes(buf);
    }
};




void PS_Graphics::_explodePSFonts(PSFont *current, PSFont*& pEnglishFont,PSFont*& pChineseFont)
{
  if (current->getUnixFont()->is_CJK_font())
	{
	  pChineseFont=current;
	  pEnglishFont=_findMatchPSFontCJK(pChineseFont);
	}
  else
	{
	  pEnglishFont=current;
	  pChineseFont=_findMatchPSFontCJK(pEnglishFont);
	}
}

void PS_Graphics::setPageSize(char* pageSizeName, UT_uint32 iwidth, UT_uint32 iheight)
{
	m_szPageSizeName = UT_strdup(pageSizeName);
	m_iWidth = iwidth; 
	m_iHeight = iheight;
}

#ifdef WITH_PANGO

/*!
  This is a dummy function that should never be called, since the PS graphics ovewrited the
  whole drawPangoGlyphString function (not implemented yet)
 */
void PS_Graphics::_drawFT2Bitmap(UT_sint32 x, UT_sint32 y, FT_Bitmap * pBitmap) const
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
}

#endif
