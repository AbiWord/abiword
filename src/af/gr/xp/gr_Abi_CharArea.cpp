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

#include "gr_Graphics.h"
#include "gr_Abi_CharArea.h"
#include "gr_Abi_RenderingContext.h"

GR_Abi_CharArea::GR_Abi_CharArea(GR_Graphics* graphics, GR_Font* f, UT_UCS4Char c)
  : m_pFont(f), m_ch(c)
{
  assert(graphics);
  graphics->setFont(m_pFont);
  m_box = BoundingBox(GR_Abi_RenderingContext::fromAbiPixels(graphics->measureUnRemappedChar(m_ch)),
		      GR_Abi_RenderingContext::fromAbiPixels(graphics->getFontAscent()),
		      GR_Abi_RenderingContext::fromAbiPixels(graphics->getFontDescent()));
}

GR_Abi_CharArea::~GR_Abi_CharArea()
{
  // is the font supposed to be freed by the Abi font manager?
}

BoundingBox
GR_Abi_CharArea::box() const
{
  return m_box;
}

scaled
GR_Abi_CharArea::leftEdge() const
{
  return 0;
}

scaled
GR_Abi_CharArea::rightEdge() const
{
  return m_box.width;
}

void
GR_Abi_CharArea::render(RenderingContext& c, const scaled& x, const scaled& y) const
{
  GR_Abi_RenderingContext& context = dynamic_cast<GR_Abi_RenderingContext&>(c);
  context.drawChar(x, y, m_pFont, m_ch);
}
