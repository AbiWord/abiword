/* AbiWord
 * Copyright (C) 2005 Martin Sevior <msevior@physics.unimelb.edu.au>
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

#include "gr_Abi_EmbedManager.h"
#include "gr_Graphics.h"
#include "xad_Document.h"

GR_EmbedView::GR_EmbedView(AD_Document * pDoc, UT_uint32 api )
  : m_pDoc(pDoc),
    m_iAPI(api)
{
}

GR_Abi_EmbedManager::GR_Abi_EmbedManager(GR_Graphics* pG)
  : m_pG(pG) 
{
  m_vecSnapshots.clear();
}

GR_Abi_EmbedManager::~GR_Abi_EmbedManager()
{ 
  UT_VECTOR_PURGEALL(GR_EmbedView *, m_vecSnapshots);
}

GR_Graphics * GR_Abi_EmbedManager::getGraphics(void)
{
  return m_pG;
}

void GR_Abi_EmbedManager::setGraphics(GR_Graphics * pG)
{
  m_pG = pG;
}

GR_Abi_EmbedManager * GR_Abi_EmbedManager::create(GR_Graphics * pG)
{
  return static_cast<GR_Abi_EmbedManager *>(new GR_Abi_EmbedManager(pG));
}

const char * GR_Abi_EmbedManager::getObjectType(void) const
{
  return "default";
}

void GR_Abi_EmbedManager::initialize(void)
{
}

void GR_Abi_EmbedManager::setDefaultFontSize(UT_sint32, UT_sint32 )
{
}

UT_sint32 GR_Abi_EmbedManager::makeEmbedView(AD_Document * pDoc, UT_uint32  api)
{
  GR_EmbedView * pEmV= new GR_EmbedView(pDoc,api);
  m_vecSnapshots.addItem(pEmV);
  UT_sint32 iNew = static_cast<UT_sint32>(m_vecSnapshots.getItemCount());
  return iNew;
}

void GR_Abi_EmbedManager::makeSnapShot(UT_sint32)
{
}

bool GR_Abi_EmbedManager::isDefault(void)
{
  return true;
}

bool GR_Abi_EmbedManager::modify(UT_sint32 uid)
{
  return false;
}

void GR_Abi_EmbedManager::initializeEmbedView(UT_sint32 uid)
{
}

void GR_Abi_EmbedManager::loadEmbedData(UT_sint32 )
{
}

UT_sint32 GR_Abi_EmbedManager::getWidth(UT_sint32 uid)
{
  return uid;
}


UT_sint32 GR_Abi_EmbedManager::getAscent(UT_sint32 uid)
{
  return uid;
}


UT_sint32 GR_Abi_EmbedManager::getDescent(UT_sint32 uid)
{
  return uid;
}

void GR_Abi_EmbedManager::setColor(UT_sint32 , UT_RGBColor )
{
}

void GR_Abi_EmbedManager::render(UT_sint32 , UT_sint32 , UT_sint32 )
{
}

void GR_Abi_EmbedManager::releaseEmbedView(UT_sint32)
{
  
}
