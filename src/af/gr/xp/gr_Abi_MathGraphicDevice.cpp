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

#include <MathView/MathMLElement.hh>
#include <MathView/ShaperManager.hh>
#include <MathView/SpaceShaper.hh>

#include "gr_Abi_AreaFactory.h"
#include "gr_Abi_MathGraphicDevice.h"
#include "gr_Abi_DefaultShaper.h"

GR_Abi_MathGraphicDevice::GR_Abi_MathGraphicDevice(GR_Graphics* pGr)
  : m_abiFactory(GR_Abi_AreaFactory::create())
{
  UT_ASSERT(pGr);

  setFactory(m_abiFactory);

  SmartPtr<GR_Abi_DefaultShaper> defaultShaper = GR_Abi_DefaultShaper::create();
  defaultShaper->setGraphics(pGr);
  getShaperManager()->registerShaper(defaultShaper);
  getShaperManager()->registerShaper(SpaceShaper::create());
}

GR_Abi_MathGraphicDevice::~GR_Abi_MathGraphicDevice()
{ }

