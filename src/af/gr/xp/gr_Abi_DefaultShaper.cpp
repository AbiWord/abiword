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

#include <cassert>

#include <MathView/ShapingResult.hh>
#include <MathView/MathGraphicDevice.hh>
#include <MathView/MathMLElement.hh>

#include "gr_Abi_AreaFactory.h"
#include "gr_Abi_DefaultShaper.h"

GR_Abi_DefaultShaper::GR_Abi_DefaultShaper()
{ }

GR_Abi_DefaultShaper::~GR_Abi_DefaultShaper()
{ }

void
GR_Abi_DefaultShaper::registerShaper(const SmartPtr<class ShaperManager>&, unsigned)
{
  // normal characters are not registered because this shaper is supposed to
  // be the default one. It will be called anyway as soon as there's a
  // Unicode char that cannot be shaped otherwise
}

void
GR_Abi_DefaultShaper::unregisterShaper(const SmartPtr<class ShaperManager>&, unsigned)
{
  // nothing to do
}

void
GR_Abi_DefaultShaper::shape(const MathFormattingContext& ctxt, ShapingResult& result) const
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
GR_Abi_DefaultShaper::shapeString(const MathFormattingContext& ctxt, const UT_UCS4Char* buffer, glong n) const
{
  assert(buffer);
  assert(false);
}
#endif

AreaRef
GR_Abi_DefaultShaper::shapeChar(const MathFormattingContext& ctxt, UT_UCSChar ch) const
{
  SmartPtr<GR_Abi_AreaFactory> factory = smart_cast<GR_Abi_AreaFactory>(ctxt.getDevice()->getFactory());
  // do NOT use getGUIFont but find the appropriate font depending on the
  // font size and font variant
  return factory->charArea(m_pGraphics, m_pGraphics->getGUIFont(), ch);
}
