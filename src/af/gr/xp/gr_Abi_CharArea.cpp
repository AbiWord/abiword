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

GR_Abi_CharArea::GR_Abi_CharArea(GR_Graphics* graphics, GR_Font* f, const scaled& size, UT_UCS4Char c)
  : m_pFont(f), m_ch(c)
{
#if 0
  UT_ASSERT(graphics);
  graphics->setFont(m_pFont);
  m_box = BoundingBox(GR_Abi_RenderingContext::fromAbiLayoutUnits(graphics->measureUnRemappedChar(m_ch)),
		      GR_Abi_RenderingContext::fromAbiLayoutUnits(graphics->getFontAscent()),
		      GR_Abi_RenderingContext::fromAbiLayoutUnits(graphics->getFontDescent()));
#else
  UT_ASSERT(f);
  UT_Rect glyphRect;
  graphics->setFont(m_pFont);
  f->glyphBox(c, glyphRect);
  xxx_UT_DEBUGMSG(("getting glyph %d\n glyphBox [left=%d,width=%d,top=%d,height=%d]\n measureUnremappedChar=%d\n measureUnremappedCharForCache=%d\n tdu=%d\n size=%d\n", c,
		   glyphRect.left, glyphRect.width, glyphRect.top, glyphRect.height,
		   graphics->measureUnRemappedChar(m_ch),
		   f->measureUnremappedCharForCache(m_ch),
		   graphics->tdu(f->measureUnremappedCharForCache(m_ch)),
		   size.getValue()));

  const float logicalWidth = f->measureUnremappedCharForCache(' ');
  const float deviceWidth = graphics->measureUnRemappedChar(' ');

  xxx_UT_DEBUGMSG(("LUCA: computing scaling ratio = %f\n", deviceWidth / logicalWidth));

  m_box = BoundingBox(GR_Abi_RenderingContext::fromAbiLayoutUnits(glyphRect.width * deviceWidth / logicalWidth),
		      GR_Abi_RenderingContext::fromAbiLayoutUnits(glyphRect.top * deviceWidth / logicalWidth),
		      GR_Abi_RenderingContext::fromAbiLayoutUnits(glyphRect.height - glyphRect.top) * deviceWidth / logicalWidth);
#endif
}

GR_Abi_CharArea::~GR_Abi_CharArea()
{ }

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
  xxx_UT_DEBUGMSG(("LUCA: rendering glyph at %d,%d height=%d\n", x.getValue(), y.getValue(),
		   m_box.height.getValue()));
  context.drawChar(x, y, m_pFont, m_ch);
  //context.drawBox(x, y, m_box);
}
