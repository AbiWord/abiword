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

#include <stdlib.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_string.h"

#include "gr_CharWidthsCache.h"
#include "xap_UnixPSFont.h"

#ifndef FT_ENCODING_ADOBE_CUSTOM
#define FT_ENCODING_ADOBE_CUSTOM ft_encoding_adobe_custom
#endif
/*
  This class is much like the UnixGraphics class in
  abi/src/wp/gr/unix/gr_UnixGraphics.h.  Why?  Because it's
  a light wrapper around the cross-application XAP_UnixFont
  class, exposing appropriate methods to the type of GC
  that needs access to the font resource (which itself
  handles keeping X fonts in sync with files on the local
  machine and their metrics).

  It's not pretty, and these wrappers should probably go
  away if someone gets around to making GR_Font basically
  XAP_UnixFont, but at a cross platform class where there
  is no Windows implementation.
*/

PSFont::PSFont(XAP_UnixFont * pFont, UT_uint32 size)
{
	UT_ASSERT(pFont);
  
	m_hFont = pFont;
	m_pointSize = size;
	m_index = 0;
}

PSFont::~PSFont(void)
{
}

XAP_UnixFont * PSFont::getUnixFont(void)
{
	UT_ASSERT(m_hFont);
	return m_hFont;
}

const UT_String & PSFont::hashKey(void) const
{
	m_hashKey = m_hFont->getFontKey();
	return m_hashKey;
}

UT_sint32 PSFont::measureUnremappedCharForCache(UT_UCSChar cChar) const
{
	UT_sint32 width;
	XftFaceLocker locker(m_hFont->getLayoutXftFont(GR_CharWidthsCache::CACHE_FONT_SIZE));
	FT_Face pFace = locker.getFace();
	if(m_hFont->isDingbat())
	{
		FT_Select_Charmap(pFace,FT_ENCODING_ADOBE_CUSTOM);
	}		
	FT_UInt glyph_index = FT_Get_Char_Index(pFace, cChar);
	FT_Error error =
		FT_Load_Glyph(pFace, glyph_index,
					FT_LOAD_LINEAR_DESIGN |
					FT_LOAD_IGNORE_TRANSFORM |
					FT_LOAD_NO_BITMAP | FT_LOAD_NO_SCALE);
	if (error) {
		return 0;
	}

	width = pFace->glyph->linearHoriAdvance;
	return width;
}


//
// UT_Rect of glyph in Logical units.
// rec.left = bearing Left (distance from origin to start)
// rec.width = widht of the glyph
// rec.top = distance from the origin to the top of the glyph
// rec.height = total height of the glyph
//
bool  PSFont::glyphBox(UT_UCS4Char glyph_index, UT_Rect & rec, GR_Graphics * pG)
{
	XftFaceLocker locker(m_hFont->getLayoutXftFont(GR_CharWidthsCache::CACHE_FONT_SIZE));
	FT_Face pFace = locker.getFace();

	FT_Error error =
		FT_Load_Glyph(pFace, glyph_index,
					FT_LOAD_LINEAR_DESIGN |
					FT_LOAD_IGNORE_TRANSFORM |
					FT_LOAD_NO_BITMAP | FT_LOAD_NO_SCALE);
	if (error) {
		return false;
	}
	rec.left = pFace->glyph->metrics.horiBearingX;
	rec.width = pFace->glyph->metrics.width;
	rec.top = pFace->glyph->metrics.horiBearingY;
	rec.height = pFace->glyph->metrics.height;


	return true;
}

float PSFont::measureUnRemappedChar(const UT_UCSChar c, UT_uint32 iSize) const
{
	XftFaceLocker locker(m_hFont->getLayoutXftFont(12));
	float width =
		fontPoints2float(iSize, locker.getFace(), getCharWidthFromCache(c));

	return width;
}

bool PSFont::doesGlyphExist(UT_UCS4Char g)
{
	return m_hFont->doesGlyphExist(g);
}
