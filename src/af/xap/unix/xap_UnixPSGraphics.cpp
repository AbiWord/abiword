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
#include <math.h>

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
#include "xap_EncodingManager.h"

#include "ut_OverstrikingChars.h"

#include "xap_Prefs.h"
#include "xap_App.h"

// the resolution that we report to the application (pixels per inch).
#define PS_RESOLUTION		72

/*
	How long line in the PS output we will allow;
	DSC3.0 limits to 256, but we may need to print a few extra
	characters after we reach the end of our buffer
*/
#define OUR_LINE_LIMIT		220			
#define LINE_BUFFER_SIZE   OUR_LINE_LIMIT + 30

static
UT_uint32 _scaleFont(PSFont * pFont, UT_uint32 units)
{
	XftFont* pXftFont = pFont->getUnixFont()->getLayoutXftFont(pFont->getSize());
	XftFaceLocker locker(pXftFont);

	UT_uint32 retval = units * pFont->getSize() / locker.getFace()->units_per_EM;
	xxx_UT_DEBUGMSG(("_scaleFont(%u) -> %u\n", units, retval));

	return retval;
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
	if (pNewFont == m_pCurrentFont) 
		return;

	UT_ASSERT(pFont);

	m_pCurrentFont = pNewFont;
	if (m_ps)
		_emit_SetFont();
}

UT_uint32 PS_Graphics::getFontAscent(GR_Font * fnt)
{
	PSFont*	hndl = static_cast<PSFont*> (fnt);
	// FIXME we should really be getting stuff fromt he font in layout units,
	// FIXME but we're not smart enough to do that yet
	// we call getDeviceResolution() to avoid zoom
	return static_cast<UT_uint32>(hndl->getUnixFont()->getAscender(hndl->getSize()) * getResolution() / getDeviceResolution() + 0.5);
}

UT_uint32 PS_Graphics::getFontAscent()
{
  return getFontAscent(m_pCurrentFont);
}

UT_uint32 PS_Graphics::getFontDescent(GR_Font * fnt)
{
	PSFont*	psfnt = static_cast<PSFont*> (fnt);
	// FIXME we should really be getting stuff fromt he font in layout units,
	// FIXME but we're not smart enough to do that yet
	return static_cast<UT_uint32>(psfnt->getUnixFont()->getDescender(psfnt->getSize()) * getResolution() / getDeviceResolution() + 0.5);
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
	return getFontAscent(fnt) + getFontDescent(fnt);
}

UT_uint32 PS_Graphics::getFontHeight()
{
	return getFontAscent(m_pCurrentFont) + getFontDescent(m_pCurrentFont);
}
	
UT_uint32 PS_Graphics::measureUnRemappedChar(const UT_UCSChar c)
{
  // FIXME we should really be getting stuff fromt he font in layout units,
  // FIXME but we're not smart enough to do that yet
  return static_cast<UT_uint32>(m_pCurrentFont->getUnixFont()->measureUnRemappedChar(c, m_pCurrentFont->getSize()) * getResolution() / getDeviceResolution());
}
#endif //#ifndef WITH_PANGO


UT_uint32 PS_Graphics::getDeviceResolution(void) const
{
	return PS_RESOLUTION;
}

void PS_Graphics::setColor(const UT_RGBColor& clr)
{
  if (m_currentColor == clr)
    return;

  m_currentColor = clr;

  if (m_bStartPage)
    _emit_SetColor();
}

void PS_Graphics::getColor(UT_RGBColor& clr)
{
	clr = m_currentColor;
}

GR_Font* PS_Graphics::getGUIFont()
{
	// getGUIFont is only used for drawing UI widgets, which does not apply on paper.
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return NULL;
}

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
	XAP_UnixFont* pUnixFont = XAP_UnixFontManager::pFontManager->findNearestFont(pszFontFamily, pszFontStyle, pszFontVariant, pszFontWeight,
																				 pszFontStretch, pszFontSize);

	UT_uint32 iSize = static_cast<UT_uint32>(UT_convertToPoints(pszFontSize));

	XAP_UnixFontHandle* item = new XAP_UnixFontHandle(pUnixFont, iSize);
	UT_ASSERT(item);

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
		if (!strcmp(psf->getUnixFont()->getPostscriptName().c_str(),pFont->getUnixFont()->getPostscriptName().c_str()) &&		
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
	if (!m_bStartPage)
		return;

	_drawCharsUTF8(pChars, iCharOffset, iLength, tdu(xoff), tdu(yoff), pCharWidths);
}

void PS_Graphics::_drawCharsOverstriking(const UT_UCSChar* pChars, UT_uint32 iCharOffset,
							 UT_uint32 iLength, UT_sint32 xoff, UT_sint32 yoff)
{
	const encoding_pair*  enc = 0;
	UT_AdobeEncoding* ae = 0;
	
	UT_ASSERT(m_pCurrentFont);

	xoff = tdu(xoff);
	yoff = tdu(yoff);

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

void PS_Graphics::_drawCharsUTF8(const UT_UCSChar* pChars, UT_uint32 iCharOffset,
							 UT_uint32 iLength, UT_sint32 xoff, UT_sint32 yoff, int * pCharWidths)
{
	xxx_UT_DEBUGMSG(("_drawCharsUTF8\n"));
	const encoding_pair*  enc = 0;
	UT_AdobeEncoding* ae = 0;
	UT_sint32 curwidth = 0;
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
	yoff += tdu (getFontAscent());

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
				xoff += curwidth;
				curwidth =0;
				m_ps->writeBytes(buf);
				pD = buf;
			}
			else if(!using_names)
			{
				xoff += curwidth;
				sprintf((char *) pD," %d %d MV ",xoff,yoff);
				curwidth =0;
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
			xoff += curwidth;
			curwidth = 0;
			pD += 4;
		}
		else
		{
			xxx_UT_DEBUGMSG(("char < 255\n"));

			if(!open_bracket)
			{
//
// mark this point as where the text needs to be drawn from.
//
				xoff += curwidth;
				curwidth =0;
				*pD++ = '(';
				open_bracket = true;
				using_names = false;
			}

			*pD++ = (char)currentChar;
		}
		curwidth += tdu (measureUnRemappedChar(currentChar));
		xxx_UT_DEBUGMSG((" width %d curwidth %d xoff %d curwidth+xoff %d char %c \n", measureUnRemappedChar(currentChar),curwidth, xoff,xoff+curwidth,(char) currentChar));
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
  if (!m_bStartPage)
    return;

	// TODO This is used for lines in the document, as well as underlines
	// TODO and strikes.
	m_bNeedStroked = true;

	// emit a change in line width
	_emit_SetLineWidth();
	UT_ASSERT(y1 < 400000);
	char buf[LINE_BUFFER_SIZE];
//	UT_sint32 nA = getFontAscent();
	g_snprintf(buf,sizeof (buf),"%d %d %d %d ML\n", tdu(x2), tdu(y2), tdu(x1), tdu(y1));
	m_ps->writeBytes(buf);
}

void PS_Graphics::setLineWidth(UT_sint32 iLineWidth)
{
	m_iLineWidth = tdu (iLineWidth);
}

static int
joinToPS (GR_Graphics::JoinStyle js)
{
  switch(js)
    {
    case GR_Graphics::JOIN_MITER: 
      return 0;
    case GR_Graphics::JOIN_ROUND: 
      return 1;
    case GR_Graphics::JOIN_BEVEL: 
      return 2;
    }

  return 1;
}

static int
capToPS (GR_Graphics::CapStyle cs)
{
  switch (cs)
    {
    case GR_Graphics::CAP_BUTT: 
      return 0;
    case GR_Graphics::CAP_ROUND: 
      return 1;
    case GR_Graphics::CAP_PROJECTING: 
      return 2;
    }

  return 1;
}

static UT_String
dashToPS (GR_Graphics::LineStyle ls)
{
  switch(ls)
    {
    case GR_Graphics::LINE_SOLID: 
      return "[] 0";
    case GR_Graphics::LINE_ON_OFF_DASH: 
      return "[1 1] 0";
    case GR_Graphics::LINE_DOUBLE_DASH: 
      return "[1 2] 0";
    case GR_Graphics::LINE_DOTTED: 
      UT_ASSERT(UT_TODO);
      return "[] 0";
    }

  return "[] 0";
}

void PS_Graphics::setLineProperties ( double inWidthPixels,
				      JoinStyle js,
				      CapStyle cs,
				      LineStyle ls)
{
  char buf[1024];
  
  g_snprintf(buf, sizeof(buf), "%d setlinecap\n", capToPS(cs));
  m_ps->writeBytes (buf);

  g_snprintf(buf, sizeof(buf), "%d setlinejoin\n", joinToPS(js));
  m_ps->writeBytes (buf);

  g_snprintf(buf, sizeof(buf), "%s setdash\n", dashToPS(ls).c_str());
  m_ps->writeBytes (buf);

#if 0
  // rounded up and most certainly ignored for now!!!
  m_iLineWidth = (UT_uint32)tdu(ceil(inWidthPixels));
  _emit_SetLineWidth ();
#endif
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
  if (!m_bStartPage)
    return;

  UT_RGBColor cl = m_currentColor;
  setColor(c);

  x = tdu(x);
  y = tdu(y);
  w = tdu(w);
  h = tdu(h);

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

	iWidth = tdu (iWidth); iHeight = tdu (iHeight);

	// TODO add other page-header comments here

	m_ps->formatComment("BeginPageSetup");

	g_snprintf(buf,sizeof (buf),"%d %d %d %s\n",iWidth,iHeight,PS_RESOLUTION,((bPortrait) ? "BPP" : "BPL"));
	m_ps->writeBytes(buf);
	
	// TODO add page-setup stuff here

	m_ps->formatComment("EndPageSetup");

	// Note, the actual PS for the page will be sent by the
	// individual drawing routings following the EndPageSetup.

	m_ps->writeBytes ("gsave\n");

#if 0
	g_snprintf(buf, sizeof(buf), "[1 0 0 1 0 %d]concat\n", iHeight*(getResolution()/100));
	m_ps->writeBytes(buf);
#endif

	// Note, need to reset font at the beginning of each page
	_emit_SetFont();	
	_emit_SetColor();

	return true;
}

bool PS_Graphics::_endPage(void)
{
	// emit stuff following each page

        m_ps->writeBytes ("grestore\n");

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
		if(bFontKeyword)
		{
			vec.addItem((void *) UT_strdup("font"));
			bFontKeyword = false;
		}
		
		// only include each font name once
		UT_String pName(psf->getUnixFont()->getPostscriptName());
		bool bFound = false;
		for(n = 0; n < vec.getItemCount(); n++)
		{
			if(!UT_strcmp(pName.c_str(), (const char *)vec.getNthItem(n)))
			{
				bFound = true;
				break;
			}
		}
		
		if(!bFound)
			vec.addItem(UT_strdup(pName.c_str()));
	}

	// TODO add any other resources here
	bool bEmbedFonts;
	XAP_App::getApp()->getPrefsValueBool((const XML_Char *)XAP_PREF_KEY_EmbedFontsInPS, &bEmbedFonts);
	if(bEmbedFonts)
	  m_ps->formatComment("DocumentSuppliedResources",&vec);
	else
	  m_ps->formatComment("DocumentNeededResources",&vec);

	UT_VECTOR_FREEALL(char*, vec);
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
	UT_DEBUGMSG(("bEmbedFonts: %d\n",bEmbedFonts));

    if(bEmbedFonts)
    {
    	for (k=0; k<kLimit; k++)
    	{
    		char buf[128];

    		PSFont * psf = (PSFont *) m_vecFontList.getNthItem(k);

    		// Instead of including the resources, we actually splat the fonts
    		// into the document.  This looks really slow... perhaps buffer line
    		// by line or in larger chunks the font data.
    		XAP_UnixFont * unixfont = psf->getUnixFont();

			UT_DEBUGMSG(("ps: Look at Embedding font number %d name %s \n",k,unixfont->getFontKey()));
		
    		bool match = false;
            const char * pName = unixfont->getFontKey();
    		for(size_t i = 0; i < vec.getItemCount(); ++i)
    		{
				if(!strcmp(pName,(const char*)vec.getNthItem(i)))
    			{

					UT_DEBUGMSG(("_ps: Font already emitted, forget it. \n"));
			    	match=true;
    				break;
    			}
    		}
    		if(match)
		    	continue;
            
            vec.addItem((void*)pName);
			UT_DEBUGMSG(("PS: Aboiut to embed font %s \n",pName));
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
			UT_String stName(psf->getUnixFont()->getPostscriptName());
    		g_snprintf(buf, sizeof (buf), "/%s findfont\n", stName.c_str());
    		m_ps->writeBytes(buf);

    		// exec the swapper macro
    		g_snprintf(buf, sizeof (buf), "/%s EXC\n", stName.c_str());
    		m_ps->writeBytes(buf);

    	}
    }
	else
	{
		// when not embeding fonts, we have to issue %IncludeResource statement
		bool bFontKeyword = true;
		const char* pFResource[2] = {"font", NULL};
		for (k=0; k<kLimit; k++)
		{
			PSFont * psf = (PSFont *)m_vecFontList.getNthItem(k);
			if(bFontKeyword)
			{
				vec.addItem((void *) UT_strdup("font"));
				bFontKeyword = false;
			}
            
			// only include each font name once
			UT_String pName(psf->getUnixFont()->getPostscriptName());
			bool bFound = false;
			for(n = 0; n < vec.getItemCount(); n++)
			{
				if(!UT_strcmp(pName.c_str(), (const char *)vec.getNthItem(n)))
				{
					bFound = true;
					break;
				}
			}
            
			if(!bFound)
			{
				vec.addItem((void*) UT_strdup(pName.c_str()));
				pFResource[1] = pName.c_str();
				m_ps->formatComment("IncludeResource", pFResource, 2);
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
		UT_String stName(psf->getUnixFont()->getPostscriptName());
		g_snprintf(buf,sizeof(buf),"  /F%d {%d /%s FSF setfont} bind def\n", k,
				psf->getSize(), stName.c_str());
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
  g_snprintf(buf, sizeof(buf), "%d setlinewidth\n", m_iLineWidth);
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

	const char * old_locale = setlocale(LC_NUMERIC,"C");
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
   
	//_UUD(iDisplayWidth);
	//_UUD(iDisplayHeight);

   	if (iType == GR_Image::GRT_Raster)
     		pImg = new PS_Image(pszName);
   	else if (iType == GR_Image::GRT_Vector)
     		pImg = new GR_VectorImage(pszName);
   
	pImg->convertFromBuffer(pBB, tdu(iDisplayWidth), tdu(iDisplayHeight));

	return pImg;
}

void PS_Graphics::drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest)
{
  if (!m_bStartPage)
    return;

  xDest = tdu(xDest); yDest = tdu(yDest);

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
	g_snprintf(buf, sizeof (buf), "%d %d translate\n", xDest, 0 - (m_iRasterPosition + 2 * iDestHeight));

	UT_DEBUGMSG(("DOM: Raster:%d | YDest: %d IDestHeight:%d\n", m_iRasterPosition, yDest, iDestHeight));

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
	g_snprintf(buf, sizeof (buf), "%d %d translate\n", xDest, 0 - (m_iRasterPosition + iDestHeight));
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

void PS_Graphics::_emit_SetFont(PSFont *pFont)
{
  if (m_bStartPage && pFont && pFont->getIndex () < m_vecFontList.size())
    {
      char buf[1024];
      g_snprintf(buf, 1024, "F%d\n", pFont->getIndex());
      m_ps->writeBytes(buf);
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
