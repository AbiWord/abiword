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

#include <ctype.h>
#include <math.h>
#include <string.h>

#include "xap_App.h"
#include "xap_Prefs.h"
#include "gr_Graphics.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_units.h"
#include "ut_debugmsg.h"

GR_Font::GR_Font() 
{
}

GR_Font::~GR_Font() 
{
	// need this so children can clean up
}

GR_Graphics::GR_Graphics()
{
	m_pApp = 0;
	m_iZoomPercentage = 100;
	m_bLayoutResolutionModeEnabled = UT_FALSE;
}

GR_Graphics::~GR_Graphics()
{
	// need this so children can clean up
}

void GR_Graphics::drawChar(UT_UCSChar Char, UT_sint32 xoff, UT_sint32 yoff)
{
}

UT_uint32 GR_Graphics::getMaxCharacterWidth(const UT_UCSChar*s, UT_uint32 Length)
{
	unsigned short *pWidths = new unsigned short[Length];

	measureString(s, 0, Length, pWidths);

	UT_uint32 MaxWidth = 0;

	for(UT_uint32 i = 0; i < Length; i++)
	{
		if(pWidths[i] > MaxWidth)
			MaxWidth = pWidths[i];
	}

	delete pWidths;

	return MaxWidth;

}

UT_uint32 GR_Graphics::measureString(const UT_UCSChar* s, int iOffset,
										 int num,  unsigned short* pWidths)
{
	// Generic base class version defined in terms of measureUnRemappedChar().
	// Platform versions can roll their own if it makes a performance difference.
	UT_ASSERT(s);

	int charWidth = 0, width;
	
	for (int i = 0; i < num; i++)
    {
		UT_UCSChar currentChar = s[i + iOffset];
		
		width = measureUnRemappedChar(currentChar);
		if (width == 0)
		{
			xxx_UT_DEBUGMSG(("GR_Graphics::measureString remapping 0x%04X\n", currentChar));
			currentChar = remapGlyph(currentChar, UT_TRUE);
			width = measureUnRemappedChar(currentChar);
		}
		
		charWidth += width;
		if (pWidths)
			pWidths[i] = width;
    }
  
	return charWidth;
}

UT_UCSChar GR_Graphics::remapGlyph(const UT_UCSChar actual, UT_Bool noMatterWhat)
{
	// This routine is for remapping zero-width characters, which
	// represent undefined glyphs in the font, into something useful
	// and visible.  This is most useful for the smart quote
	// characters, which are specially treated.  Other zero-width
	// characters are mapped to a generic filler character.
	if (!m_pApp)
	{
		UT_DEBUGMSG(("m_pApp in GR_Graphics object is null, glyphs not remapped\n"));
		return actual;
	}
	UT_UCSChar remap = actual;
	unsigned short w = 1;
	
	w = measureUnRemappedChar(actual);
	if (noMatterWhat  ||  w == 0)
	{
		switch (actual)
		{
		case UCS_LQUOTE:     remap = '`';  break;
		case UCS_RQUOTE:     remap = '\''; break;
		case UCS_RDBLQUOTE:  remap = '"'; break;
		case UCS_LDBLQUOTE:  remap = '"'; break;
		default:             remap = (w ? actual : 0xB0); break;
		}
	}
	XAP_Prefs *p = m_pApp->getPrefs();
	XAP_PrefsScheme *s = p->getCurrentScheme(UT_FALSE);
	UT_uint32 t = s->getTickCount();
	if (remap != actual) UT_DEBUGMSG(("remapGlyph  0x%04X -> 0x%04X   %d %d  tick: %d [%d]\n", actual, remap, noMatterWhat, w, t, s));
	return remap;
}

void GR_Graphics::setZoomPercentage(UT_uint32 iZoom)
{
	UT_ASSERT(iZoom > 0);
	
	m_iZoomPercentage = iZoom;
}

UT_uint32 GR_Graphics::getZoomPercentage(void) const
{
	return m_iZoomPercentage;
}

UT_uint32 GR_Graphics::getResolution(void) const
{
	return _getResolution() * m_iZoomPercentage / 100;
}

UT_sint32 GR_Graphics::convertDimension(const char * s) const
{
	double dInches = UT_convertToInches(s);
	double dResolution;
	if(m_bLayoutResolutionModeEnabled)
		{
		dResolution = UT_LAYOUT_UNITS;
		}
	else
		{
		dResolution = getResolution();		// NOTE: assumes square pixels/dpi/etc.
		}

	return (UT_sint32) (dInches * dResolution);
}

const char * GR_Graphics::invertDimension(UT_Dimension dim, double dValue) const
{
	// return pointer to static buffer -- use it quickly.
	
	double dResolution;
	if(m_bLayoutResolutionModeEnabled)
		{
		dResolution = UT_LAYOUT_UNITS;
		}
	else
		{
		dResolution = getResolution();		// NOTE: assumes square pixels/dpi/etc.
		}

	double dInches = dValue / dResolution;

	return UT_convertToDimensionString( dim, dInches);
}

UT_Bool GR_Graphics::scaleDimensions(const char * szLeftIn, const char * szWidthIn,
									 UT_uint32 iWidthAvail,
									 UT_sint32 * piLeft, UT_uint32 * piWidth) const
{
	/* Scale the given left-offset and width using the width available.
	** Compute the actual left-offset and actual width used.
	** We allow the given left-offset to be a number.
	** We allow the given width to be a number or "*"; where "*" indicates
	** we take all remaining space available.
	**
	** NOTE: This routine can also be used for vertical calculations.
	*/

	UT_ASSERT(szLeftIn);
	UT_ASSERT(szWidthIn);

	UT_sint32 iLeft = convertDimension(szLeftIn);
	UT_uint32 iWidth;

	if (szWidthIn[0] == '*')
		iWidth = iWidthAvail - iLeft;
	else
		iWidth = convertDimension(szWidthIn);

	if (piLeft)
		*piLeft = iLeft;
	if (piWidth)
		*piWidth = iWidth;

	return UT_TRUE;
}

void GR_Graphics::flush(void)
{
	// default implementation does nothing
}

void GR_Graphics::drawImage(GR_Image* pImg, UT_sint32 xDest, UT_sint32 yDest)
{
   if (pImg)
     pImg->render(this, xDest, yDest);
}

GR_Image* GR_Graphics::createNewImage(const char* pszName, const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight, GR_Image::GRType iType)
{
   GR_VectorImage * vectorImage = NULL;
   
   if (iType == GR_Image::GRT_Unknown) {
      if (GR_Image::getBufferType(pBB) == GR_Image::GRT_Vector)
	vectorImage = new GR_VectorImage(pszName);
   }
   else if (iType == GR_Image::GRT_Vector) {
      vectorImage = new GR_VectorImage(pszName);
   }
   
   if (vectorImage) {
      vectorImage->convertFromBuffer(pBB, iDisplayWidth, iDisplayHeight);
   }
   
   return vectorImage;
}
