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
#include "gr_Painter.h"
#include "gr_Abi_RenderingContext.h"


GR_Abi_RenderingContext::GR_Abi_RenderingContext(GR_Graphics* pGr)
  : m_pGraphics(pGr), m_pPainter(new GR_Painter(pGr))
{
  UT_ASSERT(m_pGraphics);
}

GR_Abi_RenderingContext::~GR_Abi_RenderingContext()
{
  DELETEP(m_pPainter);
}

void
GR_Abi_RenderingContext::setColor(const UT_RGBColor& c)
{ m_pGraphics->setColor(c); }

void
GR_Abi_RenderingContext::getColor(UT_RGBColor& c) const
{ m_pGraphics->getColor(c); }

void
GR_Abi_RenderingContext::getColor(RGBColor& c) const
{ 
  UT_RGBColor color;
  m_pGraphics->getColor(color);
  c = GR_Abi_RenderingContext::fromAbiColor(color);
}

void
GR_Abi_RenderingContext::fill(const UT_RGBColor& color, const scaled& x, const scaled& y, const BoundingBox& box) const
{
  xxx_UT_DEBUGMSG(("GR_Abi_RenderingContext::fill: color = %d %d %d at (%d,%d) box = [%d,%d,%d]\n",
		   color.m_red, color.m_grn, color.m_blu,
		   toAbiLayoutUnits(x), toAbiLayoutUnits(y),
		   toAbiLayoutUnits(box.width), toAbiLayoutUnits(box.height), toAbiLayoutUnits(box.depth)));
  m_pPainter->fillRect(color,
		       GR_Abi_RenderingContext::toAbiX(x),
		       GR_Abi_RenderingContext::toAbiY(y + box.height),
		       GR_Abi_RenderingContext::toAbiLayoutUnits(box.horizontalExtent()),
		       GR_Abi_RenderingContext::toAbiLayoutUnits(box.verticalExtent()));
}

void
GR_Abi_RenderingContext::fill(const scaled& x, const scaled& y, const BoundingBox& box) const
{
  UT_RGBColor color;
  getColor(color);
  fill(color, x, y, box);
}

void
GR_Abi_RenderingContext::drawGlyph(const scaled& x, const scaled& y, GR_Font* f, UT_uint32 glyph) const
{
  m_pGraphics->setFont(f);
  m_pPainter->drawGlyph(glyph,
			GR_Abi_RenderingContext::toAbiX(x),
			GR_Abi_RenderingContext::toAbiY(y));
}

void
GR_Abi_RenderingContext::drawChar(const scaled& x, const scaled& y, GR_Font* f, UT_UCS4Char c) const
{
  m_pGraphics->setFont(f);
  m_pPainter->drawChars(&c, 0, 1,
			GR_Abi_RenderingContext::toAbiX(x),
			GR_Abi_RenderingContext::toAbiY(y));
}

void
GR_Abi_RenderingContext::drawBox(const scaled& x, const scaled& y, const BoundingBox& box) const
{
  const UT_sint32 x0 = toAbiX(x);
  const UT_sint32 x1 = toAbiX(x + box.width);
  const UT_sint32 y0 = toAbiY(y);
  const UT_sint32 y1 = toAbiY(y + box.height);
  const UT_sint32 y2 = toAbiY(y - box.depth);
  m_pPainter->drawLine(x0, y0, x1, y0);
  m_pPainter->drawLine(x0, y1, x0, y2);
  m_pPainter->drawLine(x1, y1, x1, y2);
  m_pPainter->drawLine(x0, y1, x1, y1);
  m_pPainter->drawLine(x0, y2, x1, y2);
}
