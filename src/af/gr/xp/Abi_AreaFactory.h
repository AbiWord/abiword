// Copyright (C) 2000-2003, Luca Padovani <luca.padovani@cs.unibo.it>.
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
// http://helm.cs.unibo.it/mml-widget, or send a mail to
// <luca.padovani@cs.unibo.it>

#ifndef __Abi_AreaFactory_h__
#define __Abi_AreaFactory_h__

#include <MathView/AreaFactory.hh>

#include "Abi_ColorArea.h"
//#include "Abi_BackgroundArea.h"
#include "Abi_InkArea.h"
#include "Abi_GlyphArea.h"

class Abi_AreaFactory : public AreaFactory
{
protected:
  Abi_AreaFactory(void) { }
  virtual ~Abi_AreaFactory() { }

public:
  static SmartPtr<Abi_AreaFactory> create(void)
  { return new Abi_AreaFactory(); }

  // redefined methods

  virtual SmartPtr<ColorArea> color(const AreaRef& area, const RGBColor& color) const
  { return Abi_ColorArea::create(area, color); }
  virtual SmartPtr<InkArea> ink(const AreaRef& area) const
  { return Abi_InkArea::create(area); }
#if 0
  virtual AreaRef background(const AreaRef& area, const RGBColor& color) const
  { return Abi_BackgroundArea::create(area, color); }
#endif

  // new methods

  virtual SmartPtr<Abi_GlyphArea> glyph(class GR_Font* f, UT_uint32 glyph_idx) const
  { return Abi_GlyphArea::create(f, glyph_idx); }
};

#endif // __Abi_AreaFactory_h__
