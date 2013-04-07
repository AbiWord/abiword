/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */
 


#include <stdlib.h>
#include "ev_Toolbar_Layouts.h"
#include "ut_assert.h"
#include "ut_string.h"

/*****************************************************************/

EV_Toolbar_LayoutItem::EV_Toolbar_LayoutItem(XAP_Toolbar_Id id, EV_Toolbar_LayoutFlags flags)
{
	m_id = id;
	m_flags = flags;
}

EV_Toolbar_LayoutItem::~EV_Toolbar_LayoutItem(void)
{
}

XAP_Toolbar_Id EV_Toolbar_LayoutItem::getToolbarId(void) const
{
	return m_id;
}

EV_Toolbar_LayoutFlags EV_Toolbar_LayoutItem::getToolbarLayoutFlags(void) const
{
	return m_flags;
}

/*****************************************************************/

EV_Toolbar_Layout::EV_Toolbar_Layout(const char * szName, UT_uint32 nrLayoutItems)
{
	UT_ASSERT(nrLayoutItems > 0);
	m_nrLayoutItems = nrLayoutItems;
	// TODO tis bad to call g_try_malloc/UT_calloc from a constructor, since we cannot report failure.
	// TODO move this allocation to somewhere else.
	m_layoutTable = static_cast<EV_Toolbar_LayoutItem **>(UT_calloc(nrLayoutItems,sizeof(EV_Toolbar_LayoutItem *)));
	UT_ASSERT(m_layoutTable);
	m_szName = g_strdup(szName);
}

EV_Toolbar_Layout::EV_Toolbar_Layout(EV_Toolbar_Layout * pTB)
{
	UT_ASSERT(pTB);
	m_nrLayoutItems = pTB->getLayoutItemCount();
	// TODO tis bad to call g_try_malloc/UT_calloc from a constructor, since we cannot report failure.
	// TODO move this allocation to somewhere else.
	m_layoutTable = static_cast<EV_Toolbar_LayoutItem **>(UT_calloc(m_nrLayoutItems,sizeof(EV_Toolbar_LayoutItem *)));
	UT_ASSERT(m_layoutTable);
	m_szName = g_strdup(pTB->getName());
	UT_uint32 i = 0;
	for(i=0; i < m_nrLayoutItems; i++)
	{
		EV_Toolbar_LayoutItem * pLayItem = pTB->getLayoutItem(i);
		m_layoutTable[i] = new EV_Toolbar_LayoutItem(pLayItem->getToolbarId(),
													 pLayItem->getToolbarLayoutFlags());
	}
}

EV_Toolbar_Layout::~EV_Toolbar_Layout(void)
{
	FREEP(m_szName);
	if (!m_layoutTable)
		return;
	for (UT_uint32 k=0; k<m_nrLayoutItems; k++)
		DELETEP(m_layoutTable[k]);
	g_free(m_layoutTable);
}

bool EV_Toolbar_Layout::setLayoutItem(UT_uint32 indexLayoutItem, XAP_Toolbar_Id id, EV_Toolbar_LayoutFlags flags)
{
	UT_ASSERT(indexLayoutItem < m_nrLayoutItems);
	DELETEP(m_layoutTable[indexLayoutItem]);
	m_layoutTable[indexLayoutItem] = new EV_Toolbar_LayoutItem(id,flags);
	return (m_layoutTable[indexLayoutItem] != NULL);
}

EV_Toolbar_LayoutItem * EV_Toolbar_Layout::getLayoutItem(UT_uint32 indexLayoutItem) const
{
	UT_ASSERT(indexLayoutItem < m_nrLayoutItems);
	return m_layoutTable[indexLayoutItem];
}

const char * EV_Toolbar_Layout::getName(void) const
{
	return m_szName;
}

UT_uint32 EV_Toolbar_Layout::getLayoutItemCount(void) const
{
	return m_nrLayoutItems;
}



