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
#include <MathView/ShaperManager.hh>
#include <MathView/MathGraphicDevice.hh>
#include <MathView/MathMLElement.hh>
#include <MathView/MathVariantMap.hh>

#include "gr_Abi_AreaFactory.h"
#include "gr_Abi_DefaultShaper.h"

#define NORMAL_INDEX 0
#define MAPPED_BASE_INDEX 1

GR_Abi_DefaultShaper::GR_Abi_DefaultShaper()
{ }

GR_Abi_DefaultShaper::~GR_Abi_DefaultShaper()
{ }

void
GR_Abi_DefaultShaper::registerShaper(const SmartPtr<ShaperManager>& sm, unsigned shaperId)
{
  // normal characters are not registered because this shaper is supposed to
  // be the default one. It will be called anyway as soon as there's a
  // Unicode char that cannot be shaped otherwise

  // Variant characters however are mapped so that it is possible to
  // recover their "normal" Unicode number in the BMP and the appropriate
  // AbiWord properties can be applied when asking for a font
  for (unsigned i = NORMAL_VARIANT; i <= MONOSPACE_VARIANT; i++)
    {
      for (Char16 ch = 0x21; ch < 0x80; ch++)
	{
	  Char32 vch = mapMathVariant(MathVariant(i), ch);
	  if (vch != ch)
	    sm->registerChar(vch, GlyphSpec(shaperId, MAPPED_BASE_INDEX + i - NORMAL_VARIANT, ch));
	}
    }

}

void
GR_Abi_DefaultShaper::unregisterShaper(const SmartPtr<class ShaperManager>&, unsigned)
{
  // nothing to do
}

const GR_Abi_DefaultShaper::AbiTextProperties&
GR_Abi_DefaultShaper::getTextProperties(MathVariant variant)
{
  assert(variant >= NORMAL_VARIANT && variant <= MONOSPACE_VARIANT);
  static const AbiTextProperties variantDesc[] =
    {
      { NORMAL_VARIANT, "serif", "normal", "normal" },
      { BOLD_VARIANT, "serif", "normal", "bold" },
      { ITALIC_VARIANT, "serif", "italic", "normal" },
      { BOLD_ITALIC_VARIANT, "serif", "italic", "bold" }, 
      { DOUBLE_STRUCK_VARIANT, "sans-serif", "normal", "bold" },
      { BOLD_FRAKTUR_VARIANT, "serif", "normal", "bold" },
      { SCRIPT_VARIANT, "sans-serif", "normal", "normal" },
      { BOLD_SCRIPT_VARIANT, "sans-serif", "normal", "bold" },
      { FRAKTUR_VARIANT, "serif", "normal", "bold" },
      { SANS_SERIF_VARIANT, "sans-serif", "normal", "normal" },
      { BOLD_SANS_SERIF_VARIANT, "sans-serif", "normal", "bold" },
      { SANS_SERIF_ITALIC_VARIANT, "sans-serif", "italic", "normal" },
      { SANS_SERIF_BOLD_ITALIC_VARIANT, "sans-serif", "italic", "normal" },
      { MONOSPACE_VARIANT, "monospace", "normal", "normal" }
    };
  return variantDesc[variant - NORMAL_VARIANT];
}

void
GR_Abi_DefaultShaper::shape(const MathFormattingContext& ctxt, ShapingResult& result) const
{
  const GlyphSpec spec = result.getSpec();
  if (spec.getFontId() == NORMAL_INDEX)
    result.pushArea(1, shapeChar(NORMAL_VARIANT, ctxt, result.thisChar()));
  else
    result.pushArea(1, shapeChar(ctxt.getVariant(), ctxt, spec.getGlyphId()));
}

AreaRef
GR_Abi_DefaultShaper::shapeChar(MathVariant variant, const MathFormattingContext& ctxt, UT_UCS4Char ch) const
{
  // the "variant" parameter overrides the variant value in the formatting context

  static char fontSize[128];
  sprintf(fontSize, "%dpt", static_cast<int>(ctxt.getSize().toFloat() + 0.5f));

  const AbiTextProperties& props = getTextProperties(variant);
  GR_Font* font = m_pGraphics->findFont(props.family, props.style, 0, props.weight, 0, fontSize);
  assert(font);

  SmartPtr<GR_Abi_AreaFactory> factory = smart_cast<GR_Abi_AreaFactory>(ctxt.getDevice()->getFactory());
  // do NOT use getGUIFont but find the appropriate font depending on the
  // font size and font variant
  return factory->charArea(m_pGraphics, font, ch);
}
