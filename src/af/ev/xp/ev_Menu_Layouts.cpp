 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
*/

#include <stdlib.h>
#include "ev_Menu_Layouts.h"
#include "ut_assert.h"
#include "ut_string.h"

#define FREEP(p)	do { if (p) free(p); } while (0)

/*****************************************************************/

EV_Menu_LayoutItem::EV_Menu_LayoutItem(AP_Menu_Id id, EV_Menu_LayoutFlags flags)
{
	m_id = id;
	m_flags = flags;
}

EV_Menu_LayoutItem::~EV_Menu_LayoutItem(void)
{
}

AP_Menu_Id EV_Menu_LayoutItem::getMenuId(void) const
{
	return m_id;
}

EV_Menu_LayoutFlags EV_Menu_LayoutItem::getMenuLayoutFlags(void) const
{
	return m_flags;
}

/*****************************************************************/

EV_Menu_Layout::EV_Menu_Layout(const char * szName, UT_uint32 nrLayoutItems)
{
	UT_ASSERT(nrLayoutItems > 0);
	m_nrLayoutItems = nrLayoutItems;
	// TODO tis bad to call malloc/calloc from a constructor, since we cannot report failure.
	// TODO move this allocation to somewhere else.
	m_layoutTable = (EV_Menu_LayoutItem **)calloc(nrLayoutItems,sizeof(EV_Menu_LayoutItem *));
	UT_ASSERT(m_layoutTable);
	UT_cloneString(m_szName,szName);
}

EV_Menu_Layout::~EV_Menu_Layout(void)
{
	if (!m_layoutTable)
		return;
	for (UT_uint32 k=0; k<m_nrLayoutItems; k++)
		FREEP(m_layoutTable[k]);
	free(m_layoutTable);
}

UT_Bool EV_Menu_Layout::setLayoutItem(UT_uint32 indexLayoutItem, AP_Menu_Id id, EV_Menu_LayoutFlags flags)
{
	UT_ASSERT(indexLayoutItem < m_nrLayoutItems);
	FREEP(m_layoutTable[indexLayoutItem]);
	m_layoutTable[indexLayoutItem] = new EV_Menu_LayoutItem(id,flags);
	return (m_layoutTable[indexLayoutItem] != NULL);
}

EV_Menu_LayoutItem * EV_Menu_Layout::getLayoutItem(UT_uint32 indexLayoutItem) const
{
	UT_ASSERT(indexLayoutItem < m_nrLayoutItems);
	return m_layoutTable[indexLayoutItem];
}

const char * EV_Menu_Layout::getName(void) const
{
	return m_szName;
}
