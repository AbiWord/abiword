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
#include "ut_OverstrikingChars.h"

#include "xap_UnixApp.h"
#include "xap_Prefs.h"
#include "xap_App.h"

// the resolution that we report to the application (pixels per inch).
#define PS_RESOLUTION		360

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
	m_cs = GR_Graphics::GR_COLORSPACE_COLOR;	
	m_fm = fontManager;

	m_currentColor.m_red = 0;
	m_currentColor.m_grn = 0;
	m_currentColor.m_blu = 0;
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
   
	pImg->convertFromBuffer(pBB, tdu(iDisplayWidth), tdu(iDisplayHeight));

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
	PSFont*	hndl = static_cast<PSFont*> (fnt);
	// FIXME we should really be getting stuff fromt he font in layout units,
	// FIXME but we're not smart enough to do that yet
	// we call getDeviceResolution() to avoid zoom
	return static_cast<UT_uint32>(hndl->getUnixFont()->getAscender(hndl->getSize()) * getResolution() / getDeviceResolution() + 0.5);
}

UT_uint32 UnixNull_Graphics::getFontAscent()
{
	return getFontAscent(m_pCurrentFont);
}

UT_uint32 UnixNull_Graphics::getFontDescent(GR_Font * fnt)
{
	PSFont*				psfnt = static_cast<PSFont*> (fnt);
	// FIXME we should really be getting stuff fromt he font in layout units,
	// FIXME but we're not smart enough to do that yet
	return static_cast<UT_uint32>(psfnt->getUnixFont()->getDescender(psfnt->getSize()) * getResolution() / getDeviceResolution() + 0.5);
}

UT_uint32 UnixNull_Graphics::getFontDescent()
{
	return getFontDescent(m_pCurrentFont);
}

UT_uint32 UnixNull_Graphics::getFontHeight(GR_Font * fnt)
{
	return getFontAscent(fnt) + getFontDescent(fnt);
}

void UnixNull_Graphics::getCoverage(UT_Vector& converage)
{
}

UT_uint32 UnixNull_Graphics::getFontHeight()
{
	return getFontAscent(static_cast<GR_Font *>(m_pCurrentFont)) + getFontDescent(static_cast<GR_Font *>(m_pCurrentFont));
}
	
UT_uint32 UnixNull_Graphics::measureUnRemappedChar(const UT_UCSChar c)
{
  // FIXME we should really be getting stuff fromt he font in layout units,
  // FIXME but we're not smart enough to do that yet
  return static_cast<UT_uint32>(m_pCurrentFont->getUnixFont()->measureUnRemappedChar(c, m_pCurrentFont->getSize()) * getResolution() / getDeviceResolution());
}

UT_uint32 UnixNull_Graphics::getDeviceResolution(void) const
{
	return PS_RESOLUTION;
}

void UnixNull_Graphics::setColor(const UT_RGBColor& clr)
{
	if (m_currentColor == clr)
		return;

	// NOTE : we always set our color to something RGB, even if the color
	// NOTE : space is b&w or grayscale
	m_currentColor = clr;
}

void UnixNull_Graphics::getColor(UT_RGBColor& clr)
{
	clr = m_currentColor;
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

	UT_uint32 iSize = UT_convertToPoints(pszFontSize);
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

void UnixNull_Graphics::setPageSize(char* pageSizeName, UT_uint32 iwidth, UT_uint32 iheight)
{
}




