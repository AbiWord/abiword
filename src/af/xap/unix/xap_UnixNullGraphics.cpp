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

#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_misc.h"
#include "ut_wctomb.h"
#include "xap_UnixDialogHelper.h"
#include "ut_endian.h"

#include "xap_UnixNullGraphics.h"
#include "xap_UnixPSFont.h"
#include "xap_UnixPSImage.h"

#include "xap_UnixFont.h"
#include "xap_UnixFontManager.h"
#include "xap_UnixFontXLFD.h"
#include "xap_EncodingManager.h"

#ifdef BIDI_ENABLED
#include "ut_OverstrikingChars.h"
#endif

#include "xap_UnixApp.h"
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

static
UT_uint32 _scaleFont(PSFont * pFont, UT_uint32 units)
{
  return (units * pFont->getSize() / 1000); 
}

/*****************************************************************
******************************************************************
** This is a null graphics class to enable document editting with
** no GUI display. Much of the code was cut and pasted from 
** PS_Graphics so that no x resources
** are used in doing font calculations.
******************************************************************
*****************************************************************/

UnixNull_Graphics::UnixNull_Graphics( XAP_UnixFontManager * fontManager,
							 XAP_App *pApp)
{
	m_pApp = pApp;
	m_pCurrentFont = 0;
	m_bStartPrint = false;
	m_bStartPage = false;
	m_bNeedStroked = false;
	m_bIsFile = false;
	m_ps = 0;

	m_cs = GR_Graphics::GR_COLORSPACE_COLOR;
	
	m_fm = fontManager;

	m_currentColor.m_red = 0;
	m_currentColor.m_grn = 0;
	m_currentColor.m_blu = 0;
	m_iPageCount = 0;
}

UnixNull_Graphics::~UnixNull_Graphics()
{
	// TODO free stuff
}

 GR_Image* UnixNull_Graphics::createNewImage(const char* pszName, const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight, GR_Image::GRType iType)
{
	GR_Image* pImg = NULL;
   
   	if (iType == GR_Image::GRT_Raster)
     		pImg = new PS_Image(pszName);
   	else if (iType == GR_Image::GRT_Vector)
     		pImg = new GR_VectorImage(pszName);
   
	pImg->convertFromBuffer(pBB, iDisplayWidth, iDisplayHeight);

	return pImg;
}


bool UnixNull_Graphics::queryProperties(GR_Graphics::Properties gp) const
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

void UnixNull_Graphics::setFont(GR_Font* pFont)
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
}

UT_uint32 UnixNull_Graphics::getFontAscent(GR_Font * fnt)
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

UT_uint32 UnixNull_Graphics::getFontAscent()
{
	return getFontAscent(m_pCurrentFont);
}

UT_uint32 UnixNull_Graphics::getFontDescent(GR_Font * fnt)
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

UT_uint32 UnixNull_Graphics::getFontDescent()
{
	return getFontDescent(m_pCurrentFont);
}

UT_uint32 UnixNull_Graphics::getFontHeight(GR_Font * fnt)
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

void UnixNull_Graphics::getCoverage(UT_Vector& converage)
{
}

UT_uint32 UnixNull_Graphics::getFontHeight()
{
	UT_sint32 height = getFontAscent((GR_Font *)m_pCurrentFont) + getFontDescent((GR_Font *) m_pCurrentFont);
	return height;
}
	
UT_uint32 UnixNull_Graphics::measureUnRemappedChar(const UT_UCSChar c)
{
 	UT_ASSERT(m_pCurrentFont);
//
// If the font is in cache we're doing layout calculations so use exactly
// the same calculations as the screen uses.
//

	PSFont *pEnglishFont;
	PSFont *pChineseFont;
	_explodePSFonts(m_pCurrentFont, pEnglishFont,pChineseFont);
	
	if (XAP_EncodingManager::get_instance()->is_cjk_letter(c))
	    return _scale(pChineseFont->getUnixFont()->get_CJK_Width());
	else
	{	
		UT_uint32 width =_scale(pEnglishFont->getCharWidth(c));
		return width;
	}
}

UT_uint32 UnixNull_Graphics::_getResolution(void) const
{
	return PS_RESOLUTION;
}

void UnixNull_Graphics::setColor(const UT_RGBColor& clr)
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

}

GR_Font* UnixNull_Graphics::getGUIFont()
{
	// getGUIFont is only used for drawing UI widgets, which does not apply on paper.
//
// Better put something here...

	return NULL;
}

GR_Font* UnixNull_Graphics::findFont(const char* pszFontFamily, 
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

void UnixNull_Graphics::drawGlyph(UT_uint32 Char, UT_sint32 xoff, UT_sint32 yoff)
{
}

#ifdef WITH_PANGO
void UnixNull_Graphics::_drawFT2Bitmap(UT_sint32 x, UT_sint32 y, FT_Bitmap * pBitmap) const
{
}
#endif

void UnixNull_Graphics::drawChars(const UT_UCSChar* pChars, int iCharOffset,
								  int iLength, UT_sint32 xoff, UT_sint32 yoff,
								  int * pCharWidths)
{
}

void UnixNull_Graphics::drawLine(UT_sint32 x1, UT_sint32 y1, UT_sint32 x2, UT_sint32 y2)
{
}

void UnixNull_Graphics::setLineWidth(UT_sint32 iLineWidth)
{
	m_iLineWidth = iLineWidth;
}

void UnixNull_Graphics::xorLine(UT_sint32, UT_sint32, UT_sint32, UT_sint32)
{
}

void UnixNull_Graphics::polyLine(UT_Point * /* pts */, UT_uint32 /* nPoints */)
{
}

void UnixNull_Graphics::fillRect(const UT_RGBColor& c, UT_sint32 x, UT_sint32 y, UT_sint32 w, UT_sint32 h)
{
}

void UnixNull_Graphics::invertRect(const UT_Rect* /*pRect*/)
{
}

void UnixNull_Graphics::setClipRect(const UT_Rect* r)
{
//
// might have to put something here?
//
	m_pRect = r;
}

void UnixNull_Graphics::clearArea(UT_sint32 /*x*/, UT_sint32 /*y*/,
							UT_sint32 /*width*/, UT_sint32 /*height*/)
{

}

void UnixNull_Graphics::scroll(UT_sint32, UT_sint32)
{

}

void UnixNull_Graphics::scroll(UT_sint32 /* x_dest */,
						 UT_sint32 /* y_dest */,
						 UT_sint32 /* x_src */,
						 UT_sint32 /* y_src */,
						 UT_sint32 /* width */,
						 UT_sint32 /* height */)
{

}

bool UnixNull_Graphics::startPrint(void)
{
	return true;
}

bool UnixNull_Graphics::startPage(const char * szPageLabel, UT_uint32 pageNumber,
							   bool bPortrait, UT_uint32 iWidth, UT_uint32 iHeight)
{
	return true;
}

bool UnixNull_Graphics::endPrint(void)
{
	return true;
}

/*****************************************************************/
/*****************************************************************/

UT_uint32 UnixNull_Graphics::_scale(UT_uint32 units) const
{
	// we are given a value from the AFM file which are
	// expressed in 1/1000ths of the scaled font.
	// return the number of pixels at our resolution.

  
	return _scaleFont(m_pCurrentFont, units);
}

void UnixNull_Graphics::drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest)
{
}
	
void UnixNull_Graphics::drawRGBImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest)
{
}

void UnixNull_Graphics::drawGrayImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest)
{
	
}
void UnixNull_Graphics::drawBWImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest)
{
}

void UnixNull_Graphics::setColorSpace(GR_Graphics::ColorSpace c)
{
	m_cs = c;
}

GR_Graphics::ColorSpace UnixNull_Graphics::getColorSpace(void) const
{
	return m_cs;
}

void UnixNull_Graphics::setCursor(GR_Graphics::Cursor c)
{
	if (m_cursor == c)
		return;
	m_cursor = c;
	return;
}

GR_Graphics::Cursor UnixNull_Graphics::getCursor(void) const
{
	return m_cursor;
}

void UnixNull_Graphics::setColor3D(GR_Color3D /*c*/)
{

}

UT_RGBColor * UnixNull_Graphics::getColor3D(GR_Color3D /*c*/)
{
	return NULL;
}

void UnixNull_Graphics::fillRect(GR_Color3D /*c*/, UT_sint32 /*x*/, UT_sint32 /*y*/, UT_sint32 /*w*/, UT_sint32 /*h*/)
{
}

void UnixNull_Graphics::fillRect(GR_Color3D /*c*/, UT_Rect & /*r*/)
{
}


PSFont *UnixNull_Graphics::_findMatchPSFontCJK(PSFont * pFont)
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

void UnixNull_Graphics::_explodePSFonts(PSFont *current, PSFont*& pEnglishFont,PSFont*& pChineseFont)
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

void UnixNull_Graphics::setPageSize(char* pageSizeName, UT_uint32 iwidth, UT_uint32 iheight)
{
	m_szPageSizeName = UT_strdup(pageSizeName);
	m_iWidth = iwidth; 
	m_iHeight = iheight;
}




