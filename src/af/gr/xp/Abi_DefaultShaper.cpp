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

#include <MathView/ShapingResult.hh>
#include <MathView/MathGraphicDevice.hh>
#include <MathView/MathMLElement.hh>

#include "Abi_CharArea.h"
#include "Abi_DefaultShaper.h"

Abi_DefaultXpShaper::Abi_DefaultXpShaper()
{ }

Abi_DefaultXpShaper::~Abi_DefaultXpShaper()
{ }

void
Abi_DefaultXpShaper::registerShaper(const SmartPtr<class ShaperManager>&, unsigned)
{
  // normal characters are not registered because this shaper is supposed to
  // be the default one. It will be called anyway as soon as there's a
  // Unicode char that cannot be shaped otherwise
}

void
Abi_DefaultXpShaper::unregisterShaper(const SmartPtr<class ShaperManager>&, unsigned)
{
  // nothing to do
}

void
Abi_DefaultXpShaper::shape(const MathFormattingContext& ctxt, ShapingResult& result) const
{
  const unsigned n = result.chunkSize();
  assert(n > 0);
#if 0
  UT_UCS4Char* buffer = new UT_UCS4Char[n];
  for (unsigned i = 0; i < n; i++) buffer[i] = result.data()[i];
  result.pushArea(n, shapeString(ctxt, buffer, n));
  delete [] buffer;
#else
  UT_UCS4Char ch = result.thisChar();
  result.pushArea(1, shapeChar(ctxt, ch));
#endif
}

#if 0
AreaRef
Abi_DefaultXpShaper::shapeString(const MathFormattingContext& ctxt, const UT_UCS4Char* buffer, glong n) const
{
  assert(buffer);
  assert(false);
}
#endif

AreaRef
Abi_DefaultXpShaper::shapeChar(const MathFormattingContext& ctxt, UT_UCSChar ch) const
{
  return Abi_CharArea::create(graphics, graphics->getGUIFont(), ch);
}
