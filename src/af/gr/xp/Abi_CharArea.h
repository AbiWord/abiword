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

#ifndef __Abi_CharArea_h__
#define __Abi_CharArea_h__

#include <MathView/GlyphArea.hh>

#include "ut_types.h" // for UT_UCS4Char

class Abi_CharArea : public GlyphArea
{
protected:
  Abi_CharArea(class GR_Graphics*, class GR_Font*, UT_UCS4Char);
  virtual ~Abi_CharArea();

public:
  static SmartPtr<Abi_CharArea> create(GR_Graphics* g, GR_Font* font, UT_UCS4Char glyph)
  { return new Abi_CharArea(g, font, glyph); }

  virtual BoundingBox box(void) const;
  virtual scaled leftEdge(void) const;
  virtual scaled rightEdge(void) const;
  virtual void render(class RenderingContext&, const scaled&, const scaled&) const;

private:
  class GR_Font* font;
  UT_UCS4Char ch;
  BoundingBox bbox;
};

#endif // __Abi_CharArea_h__
