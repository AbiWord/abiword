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

#ifndef __Abi_DefaultXpShaper_h__
#define __Abi_DefaultXpShaper_h__

#include <MathView/Shaper.hh>

#include "ut_types.h" // for UT_UCS4Char
#include "gr_Graphics.h"

class Abi_DefaultXpShaper : public Shaper
{
protected:
  Abi_DefaultXpShaper(void);
  virtual ~Abi_DefaultXpShaper();

public:
  static SmartPtr<Abi_DefaultXpShaper> create(void)
  { return new Abi_DefaultXpShaper(); }

  virtual void registerShaper(const SmartPtr<class ShaperManager>&, unsigned);
  virtual void unregisterShaper(const SmartPtr<class ShaperManager>&, unsigned);
  virtual void shape(const class MathFormattingContext&, class ShapingResult&) const;

  void setGraphics(class GR_Graphics*);
  class GR_Graphics* getGraphics(void) const { return graphics; }

protected:
  AreaRef shapeChar(const class MathFormattingContext&, UT_UCS4Char) const;
  class GR_Graphics* graphics;
};

#endif // __Abi_DefaultXpShaper_h__
