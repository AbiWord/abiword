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

#include "gr_MathView_config.h"

#include <cassert>

#include <MathView/MathMLElement.hh>

#include "gr_Abi_AreaFactory.h"
#include "gr_Abi_MathGraphicDevice.h"

GR_Abi_MathGraphicDevice::GR_Abi_MathGraphicDevice()
  : m_abiFactory(GR_Abi_AreaFactory::create())
{
  setFactory(m_abiFactory);

#if 0
  GObjectPtr<PangoContext> context = gtk_widget_create_pango_context(widget);
  SmartPtr<GR_Abi_DefaultPangoShaper> defaultPangoShaper = GR_Abi_DefaultPangoShaper::create();
  defaultPangoShaper->setPangoContext(context);
  getShaperManager()->registerShaper(defaultPangoShaper);

  getShaperManager()->registerShaper(SpaceShaper::create());

#if 1
  SmartPtr<GR_Abi_PangoShaper> pangoShaper = GR_Abi_PangoShaper::create();
  pangoShaper->setPangoContext(context);
  getShaperManager()->registerShaper(pangoShaper);
#else
  getShaperManager()->registerShaper(GR_Abi_AdobeShaper::create());
#endif
#endif
}

GR_Abi_MathGraphicDevice::~GR_Abi_MathGraphicDevice()
{ }

