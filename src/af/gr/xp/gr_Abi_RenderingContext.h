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

#ifndef __gr_Abi_RenderingContext_h__
#define __gr_Abi_RenderingContext_h__

#include <MathView/scaled.hh>
#include <MathView/BoundingBox.hh>
#include <MathView/RGBColor.hh>
#include <MathView/RenderingContext.hh>

#include "ut_misc.h" // for UT_RGBColor

class GR_Abi_RenderingContext : public RenderingContext
{
public:
  GR_Abi_RenderingContext(class GR_Graphics*);
  virtual ~GR_Abi_RenderingContext();

  class GR_Graphics* getGraphics(void) const { return m_pGraphics; }

  void setColor(const RGBColor& c) { setColor(toAbiColor(c)); }
  void setColor(const UT_RGBColor&);
  // the return value is passed as an argument so that
  // we can use overloading
  void getColor(RGBColor&) const;
  void getColor(UT_RGBColor&) const;

  void fill(const scaled&, const scaled&, const BoundingBox&) const;
  void fill(const UT_RGBColor&, const scaled&, const scaled&, const BoundingBox&) const;
  void fill(const RGBColor& c, const scaled& x, const scaled& y, const BoundingBox& box) const
  { fill(toAbiColor(c), x, y, box); }

  void drawGlyph(const scaled&, const scaled&, class GR_Font*, UT_uint32) const;
  void drawChar(const scaled&, const scaled&, class GR_Font*, UT_UCS4Char) const;

  static int toAbiPixels(const scaled& s)
  { return round(s * (72.27 / 72.0)).toInt(); }
  static scaled fromAbiPixels(int s)
  { return scaled(s * (72.0 / 72.27)); }

  static int toAbiX(const scaled& x)
  { return toAbiPixels(x); }
  static int toAbiY(const scaled& y)
  { return toAbiPixels(-y); }

  static scaled fromAbiX(int x)
  { return fromAbiPixels(x); }
  static scaled fromAbiY(int y)
  { return fromAbiPixels(-y); }

  static RGBColor fromAbiColor(const UT_RGBColor& c)
  { return RGBColor(c.m_red, c.m_grn, c.m_blu, c.m_bIsTransparent); }
  static UT_RGBColor toAbiColor(const RGBColor& c)
  { return UT_RGBColor(c.red, c.green, c.blue, c.transparent); }

private:
  class GR_Graphics* m_pGraphics;
  class GR_Painter* m_pPainter;
};

#endif // __gr_Abi_RenderingContext_h__
