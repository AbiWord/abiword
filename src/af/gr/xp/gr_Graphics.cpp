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
#include "xap_EncodingManager.h"
#include "xap_Prefs.h"
#include "gr_Graphics.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_units.h"
#include "ut_growbuf.h"
#include "ut_debugmsg.h"

#ifdef BIDI_ENABLED
#include "ut_OverstrikingChars.h"
#endif
// static class member initializations
bool GR_Graphics::m_bRemapGlyphsMasterSwitch = true;
bool GR_Graphics::m_bRemapGlyphsNoMatterWhat = false;
UT_UCSChar GR_Graphics::m_ucRemapGlyphsDefault = 0xB0;
UT_UCSChar *GR_Graphics::m_pRemapGlyphsTableSrc = 0;
UT_UCSChar *GR_Graphics::m_pRemapGlyphsTableDst = 0;
UT_uint32 GR_Graphics::m_iRemapGlyphsTableLen = 0;

XAP_PrefsScheme *GR_Graphics::m_pPrefsScheme = 0;
UT_uint32 GR_Graphics::m_uTick = 0;

UT_uint32 GR_Graphics::m_instanceCount = 0;

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
	m_bLayoutResolutionModeEnabled = false;
	m_bIsPortrait = true;

	m_instanceCount++;
}

GR_Graphics::~GR_Graphics()
{
	// need this so children can clean up

	m_instanceCount--;

	if(m_instanceCount == 0)
	{
		delete m_pRemapGlyphsTableSrc;
		delete m_pRemapGlyphsTableDst;
	}

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

	int stringWidth = 0, charWidth;
	for (int i = 0; i < num; i++)
    {
		UT_UCSChar currentChar = remapGlyph(s[i + iOffset], false);
#ifdef BIDI_ENABLED
		if(isOverstrikingChar(currentChar) == UT_NOT_OVERSTRIKING)
		{
			charWidth = measureUnRemappedChar(currentChar);
			stringWidth += charWidth;
		}
		else
			charWidth = 0;
#else
		charWidth = measureUnRemappedChar(currentChar);
		stringWidth += charWidth;
#endif
		if (pWidths) pWidths[i] = charWidth;
    }
	return stringWidth;
}

UT_UCSChar GR_Graphics::remapGlyph(const UT_UCSChar actual_, bool noMatterWhat)
{
	UT_UCSChar actual = actual_;

	// Here is how the remapGlyph works.

	// * If preference value RemapGlyphsMasterSwitch is not true, no
	// remapping is done.

	// * RemapGlyphsTable is a list of pairs of characters, with the
	// first of each pair being the actual character and the second of
	// each pair being the replacement.  Normally, a character will
	// not be remapped unless it is zero-width.  If it is listed as an
	// actual character in RemapGlyphsTable, then the affiliated
	// replacement character is returned.  If it is not listed in
	// RemapGlyphsTable, then the value of RemapGlyphsDefault is used,
	// though if the default value is null, it is not used (it is
	// tricky to specify null due to XML parsing).

	// * Remapping is not recursive/iterative/whatever.  So, if you
	// remap from a zero-width character to another zero-width
	// character, that's what you get.

	// * If this function is called with noMatterWhat true, or if the
	// preference value RemapGlyphsNoMatterWhat is true, the above algorithm
	// is changed a bit.  Characters listed as actual characters in the table
	// are given replacement values even if they are not zero-width, however
	// the default replacement character is used only if the actual character
	// really has zero-width.  (The intent of the function parameter is that
	// the calling function may have already determined that the character is
	// zero-width, and it may be arbitrarily expensive to check, so a
	// redundant check can be avoided.)  (The intent of the user preference
	// value is to just give a way to force the algorithm's hand "just in
	// case".)

	// * Because we are constantly checking those user preference values
	// (sometimes it might be 2-3 times per character rendered), we cache
	// their values as static members of the GR_Graphics class.  The table of
	// actual and replacement characters is split into two arrays of "src"
	// and "dst" characters, respectively.  If the table had an odd number of
	// characters, the extra dangling character is ignored.

	// * Linear searching is done looking for the actual character, so the
	// implementation is assuming that the RemapGlyphsTable is not too big.
	// Also, because null-terminated UTF8 and Unicode strings are all over
	// the place, you can remap a null character nor remap to a null
	// character.  In office productivity apps, this probably won't be a
	// problem.

	UT_UCSChar remap = actual;
	if (!m_pApp)
	{
		UT_DEBUGMSG(("GR_Graphics::remapGlyph() has no XAP_App*, glyphs not remapped\n"));
		return actual;
	}
	XAP_Prefs *p = m_pApp->getPrefs();
	UT_ASSERT(m_pApp->getPrefs());
	XAP_PrefsScheme *s = p->getCurrentScheme(false);
	UT_ASSERT(p->getCurrentScheme(false));
	UT_uint32 t = s->getTickCount();
	if (m_pPrefsScheme != s  ||  m_uTick != t)
	{
		// refresh cached preference values
		UT_DEBUGMSG(("GR_Graphics::remapGlyph() refreshing cached values\n"));
		m_pPrefsScheme = s;
		m_uTick = t;
		p->getPrefsValueBool((XML_Char*)XAP_PREF_KEY_RemapGlyphsMasterSwitch, &m_bRemapGlyphsMasterSwitch);
		p->getPrefsValueBool((XML_Char*)XAP_PREF_KEY_RemapGlyphsNoMatterWhat, &m_bRemapGlyphsNoMatterWhat);

		const XML_Char *table_utf8, *default_utf8;
		UT_GrowBuf gb;
		const UT_UCSChar *tbl;

		p->getPrefsValue((XML_Char*)XAP_PREF_KEY_RemapGlyphsDefault, &default_utf8);
		UT_ASSERT(p->getPrefsValue((XML_Char*)XAP_PREF_KEY_RemapGlyphsDefault, &default_utf8));
		UT_decodeUTF8string(default_utf8, UT_XML_strlen(default_utf8), &gb);
		m_ucRemapGlyphsDefault = *(gb.getPointer(0));  // might be null

		p->getPrefsValue((XML_Char*)XAP_PREF_KEY_RemapGlyphsTable, &table_utf8);
		UT_ASSERT(p->getPrefsValue((XML_Char*)XAP_PREF_KEY_RemapGlyphsTable, &table_utf8));
		gb.truncate(0);
		UT_decodeUTF8string(table_utf8, UT_XML_strlen(table_utf8), &gb);

		UT_uint32 doublelength;
		doublelength = gb.getLength();
		m_iRemapGlyphsTableLen = doublelength / 2;
		delete m_pRemapGlyphsTableSrc;
		delete m_pRemapGlyphsTableDst;
		m_pRemapGlyphsTableSrc = 0;
		m_pRemapGlyphsTableDst = 0;
		if (m_iRemapGlyphsTableLen)
		{
			m_pRemapGlyphsTableSrc = new UT_UCSChar[m_iRemapGlyphsTableLen];
			m_pRemapGlyphsTableDst = new UT_UCSChar[m_iRemapGlyphsTableLen];
			tbl = gb.getPointer(0);
			for (UT_uint32 tdex=0; tdex<m_iRemapGlyphsTableLen; ++tdex)
			{
				m_pRemapGlyphsTableSrc[tdex] = tbl[2 * tdex];
				m_pRemapGlyphsTableDst[tdex] = tbl[2 * tdex + 1];
				UT_DEBUGMSG(("RemapGlyphsTable[%d]  0x%04x -> 0x%04x\n", tdex, m_pRemapGlyphsTableSrc[tdex], m_pRemapGlyphsTableDst[tdex]));
			}
		}
	}
	if (!m_bRemapGlyphsMasterSwitch) return (actual);
	UT_uint32 width = 0xFFFF;
	if (noMatterWhat  ||  m_bRemapGlyphsNoMatterWhat  ||  !(width = measureUnRemappedChar(actual)))
	{
		bool try_default = true;
		for (UT_uint32 tdex=0; tdex<m_iRemapGlyphsTableLen; ++tdex)
		{
			/*compare with character in unicode, not in current locale*/
			if (actual_ == m_pRemapGlyphsTableSrc[tdex])
			{
				remap = m_pRemapGlyphsTableDst[tdex];
				try_default = false;
				break;
			}
		}
		if (try_default  &&  m_ucRemapGlyphsDefault)
		{
			if (width == 0xFFFF) width = measureUnRemappedChar(actual);
			if (!width) remap = m_ucRemapGlyphsDefault;
		}
	}
	if (remap != actual)
	{
		UT_DEBUGMSG(("GR_Graphics::remapGlyph( 0x%04X ) -> 0x%04X    tick: %d [%d]\n", actual, remap, t, s));
	}

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

	return UT_convertInchesToDimensionString( dim, dInches);
}

bool GR_Graphics::scaleDimensions(const char * szLeftIn, const char * szWidthIn,
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

	return true;
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

bool GR_Graphics::_PtInPolygon(UT_Point * pts,UT_uint32 nPoints,UT_sint32 x,UT_sint32 y)
{
    UT_uint32 i,j;
    bool bResult = false;
    for (i = 0,j = nPoints - 1;i < nPoints;j = i++){
        if ((((pts[i].y <= y) && (y < pts[j].y)) || ((pts[j].y <= y) && (y < pts[i].y))) &&
            (x < (pts[j].x - pts[i].x) * (y - pts[i].y) / (pts[j].y - pts[i].y) + pts[i].x))
        {
            bResult = !bResult;
        }
    }
    return (bResult);
}

void GR_Graphics::polygon(UT_RGBColor& c,UT_Point *pts,UT_uint32 nPoints)
{
    UT_sint32 minX,maxX,minY,maxY,x,y;
    minX = maxX = pts[0].x;
    minY = maxY = pts[0].y;
    for(UT_uint32 i = 0;i < nPoints - 1;i++){
        minX = UT_MIN(minX,pts[i].x);
        maxX = UT_MAX(maxX,pts[i].x);
        minY = UT_MIN(minY,pts[i].y);
        maxY = UT_MAX(maxY,pts[i].y);
    }
    for(x = minX;x <= maxX;x++){
        for(y = minY;y <= maxY;y++){
            if(_PtInPolygon(pts,nPoints,x,y)){
                fillRect(c,x,y,1,1);
            }
        }
    }
 }
