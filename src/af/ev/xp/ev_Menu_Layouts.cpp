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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */
 


#include <stdlib.h>
#include "ev_Menu_Layouts.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_misc.h"

/*****************************************************************/

EV_Menu_LayoutItem::EV_Menu_LayoutItem(XAP_Menu_Id id, EV_Menu_LayoutFlags flags)
{
	m_id = id;
	m_flags = flags;
}

EV_Menu_LayoutItem::~EV_Menu_LayoutItem()
{
}

XAP_Menu_Id EV_Menu_LayoutItem::getMenuId() const
{
	return m_id;
}

EV_Menu_LayoutFlags EV_Menu_LayoutItem::getMenuLayoutFlags() const
{
	return m_flags;
}

/*****************************************************************/

#define max(a, b) ((a) < (b) ? (b) : (a))
#define min(a, b) ((a) < (b) ? (a) : (b))

EV_Menu_Layout::EV_Menu_Layout(const UT_String &stName, UT_uint32 nrLayoutItems)
	: m_stName(stName),
	  m_layoutTable(nrLayoutItems),
	  m_iMaxId(0)
{
	for (UT_uint32 i = 0; i < nrLayoutItems; i++)
		m_layoutTable.addItem(0);
}

EV_Menu_Layout::~EV_Menu_Layout()
{
	UT_VECTOR_PURGEALL(EV_Menu_LayoutItem *, m_layoutTable);
}

/*!
 * It adds a new item to the menu bar.
 * \param path says where should appear the new item
 * (syntax: "/File/Send by email", "/Insert/Autotext/Insert automatically", etc.
 * you got it.)
 *
 * \todo This operation is a slow dog.  I should use ut_list, but it doesn't exists yet.
 */
XAP_Menu_Id EV_Menu_Layout::addLayoutItem(const UT_String &/* path */, EV_Menu_LayoutFlags flags)
{
// todo
//	UT_Vector *items = simpleSplit(path);
	EV_Menu_LayoutItem *pItem = new EV_Menu_LayoutItem(m_iMaxId++, flags);
	// fixme: by now, I will just put the item in a random place.  Just for test purposes
	int pos = 30;

	m_layoutTable.insertItemAt(pItem, pos);

	return m_iMaxId;
}

bool EV_Menu_Layout::setLayoutItem(UT_uint32 indexLayoutItem, XAP_Menu_Id id, EV_Menu_LayoutFlags flags)
{
	UT_ASSERT(indexLayoutItem < m_layoutTable.getItemCount());
	m_iMaxId = max(m_iMaxId, id);
	void *old;
	m_layoutTable.setNthItem(indexLayoutItem, new EV_Menu_LayoutItem(id, flags), &old);
	EV_Menu_LayoutItem * pOld = static_cast<EV_Menu_LayoutItem *> (old);
	DELETEP(pOld);
	return (m_layoutTable[indexLayoutItem] != NULL);
}

EV_Menu_LayoutItem * EV_Menu_Layout::getLayoutItem(UT_uint32 indexLayoutItem) const
{
	UT_ASSERT(indexLayoutItem < m_layoutTable.getItemCount());
	return static_cast<EV_Menu_LayoutItem *> (m_layoutTable.getNthItem(indexLayoutItem));
}

const char * EV_Menu_Layout::getName() const
{
	return m_stName.c_str();
}

UT_uint32 EV_Menu_Layout::getLayoutItemCount() const
{
	return m_layoutTable.getItemCount();
}

