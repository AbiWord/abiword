// Copyright (C) 2000-2004, Luca Padovani <luca.padovani@cs.unibo.it>.
//
// This file is part of GtkMathView, a Gtk widget for MathML.
// 
// GtkMathView is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// GtkMathView is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with GtkMathView; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
// 
// For details, see the GtkMathView World-Wide-Web page,
// http://helm.cs.unibo.it/mml-widget/, or send a mail to
// <lpadovan@cs.unibo.it>

#include "MathView_config.h"

#include <cassert>

#include "gr_Graphics.h"
#include "gr_Painter.h"

#include "Abi_RenderingContext.h"

Abi_RenderingContext::Abi_RenderingContext(GR_Graphics* g)
  : graphics(g), painter(new GR_Painter(g))
{
  assert(graphics);
}

Abi_RenderingContext::~Abi_RenderingContext()
{
  delete painter;
}

void
Abi_RenderingContext::setColor(const UT_RGBColor& c)
{ graphics->setColor(c); }

void
Abi_RenderingContext::getColor(UT_RGBColor& c) const
{ graphics->getColor(c); }

void
Abi_RenderingContext::getColor(RGBColor& c) const
{ 
  UT_RGBColor color;
  graphics->getColor(color);
  c = Abi_RenderingContext::fromAbiColor(color);
}

void
Abi_RenderingContext::fill(const UT_RGBColor& color, const scaled& x, const scaled& y, const BoundingBox& box) const
{
  painter->fillRect(color,
		    Abi_RenderingContext::toAbiX(x),
		    Abi_RenderingContext::toAbiY(y + box.height),
		    Abi_RenderingContext::toAbiPixels(box.horizontalExtent()),
		    Abi_RenderingContext::toAbiPixels(box.verticalExtent()));
}

void
Abi_RenderingContext::fill(const scaled& x, const scaled& y, const BoundingBox& box) const
{
  UT_RGBColor color;
  getColor(color);
  fill(color, x, y, box);
}

void
Abi_RenderingContext::drawGlyph(const scaled& x, const scaled& y, GR_Font* f, UT_uint32 glyph) const
{
  graphics->setFont(f);
  painter->drawGlyph(glyph,
		     Abi_RenderingContext::toAbiX(x),
		     Abi_RenderingContext::toAbiY(y));
}

void
Abi_RenderingContext::drawChar(const scaled& x, const scaled& y, GR_Font* f, UT_UCS4Char c) const
{
  graphics->setFont(f);
  painter->drawChars(&c, 0, 1,
		     Abi_RenderingContext::toAbiX(x),
		     Abi_RenderingContext::toAbiY(y));
}
