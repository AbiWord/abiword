/* AbiWord
 * Copyright (C) 2004 Luca Padovani <lpadovan@cs.unibo.it>
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

#include "gr_Abi_GlyphArea.h"
#include "gr_Abi_RenderingContext.h"

GR_Abi_GlyphArea::GR_Abi_GlyphArea(GR_Font* pFont, UT_uint32 g)
  : m_pFont(pFont), m_glyphIndex(g)
{ }

GR_Abi_GlyphArea::~GR_Abi_GlyphArea()
{
  // is the font supposed to be freed by the Abi font manager?
}

BoundingBox
GR_Abi_GlyphArea::box() const
{
  return BoundingBox(); // ???
}

scaled
GR_Abi_GlyphArea::leftEdge() const
{
  return 0; // ???
}

scaled
GR_Abi_GlyphArea::rightEdge() const
{
  return 0; // ???
}

void
GR_Abi_GlyphArea::render(RenderingContext& c, const scaled& x, const scaled& y) const
{
  GR_Abi_RenderingContext& context = dynamic_cast<GR_Abi_RenderingContext&>(c);
  context.drawGlyph(x, y, m_pFont, m_glyphIndex);
}
