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
#include "ev_Menu_Labels.h"
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_misc.h"
#include "ut_debugmsg.h"

/*****************************************************************/

EV_Menu_LayoutItem::EV_Menu_LayoutItem(XAP_Menu_Id id, EV_Menu_LayoutFlags flags)
	: m_id(id),
	  m_flags(flags)
{
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
	UT_VECTOR_SPARSEPURGEALL(EV_Menu_LayoutItem *, m_layoutTable);
}

#if 0
/*!
 * It adds a new item to the menu bar.
 * \param path says where should appear the new item
 * (syntax: "/File/Send by email", "/Insert/Autotext/Insert automatically", etc.
 * you got it.)
 *
 * \todo This operation is a slow dog.  I should use ut_list, but it doesn't exists yet.
 */
XAP_Menu_Id EV_Menu_Layout::addLayoutItem(const UT_String &path,
										  const EV_Menu_LabelSet &labels,
										  EV_Menu_LayoutFlags flags)
{
	UT_Vector *names = simpleSplit(path);
	UT_ASSERT(names);
	int pos = searchPos(names, labels);

	EV_Menu_LayoutItem *pItem = new EV_Menu_LayoutItem(++m_iMaxId, flags);
	UT_ASSERT(pItem);
	UT_DEBUGMSG(("Creating EV_Menu_LayoutItem(%d) at pos [%d]\n", m_iMaxId, pos));
	m_layoutTable.insertItemAt(pItem, pos);

	delete names;
	return m_iMaxId;
}

int EV_Menu_Layout::searchPos(const UT_Vector *names, const EV_Menu_LabelSet &labels) const
{
	UT_ASSERT(names);
	int pos = 1;

	if (names->size() < 2)
		return 1;

	void *tmp = names->getNthItem(names->size() - 2);

	if (!tmp)
		return pos;

	UT_String *last_path = static_cast<UT_String *> (tmp);
	for (UT_uint32 i = 0; i < m_layoutTable.size(); ++i)
	{
		tmp = m_layoutTable[i];
		if (!tmp)
			continue;
		
		EV_Menu_LayoutItem *layout_item = static_cast<EV_Menu_LayoutItem *> (tmp);
		if (EV_MLF_BeginSubMenu == layout_item->getMenuLayoutFlags())
		{
			XAP_Menu_Id id = layout_item->getMenuId();
			const EV_Menu_Label *label = labels.getLabel(id);
			UT_ASSERT(label);

			if (*last_path == label->getMenuLabel())
			{
				pos = i + 1;
				break;
			}
		}
	}

	return pos;
}
#endif

void EV_Menu_Layout::addFakeLayoutItem(UT_uint32 indexLayoutItem, EV_Menu_LayoutFlags flags)
{
	UT_uint32 err = m_layoutTable.insertItemAt(new EV_Menu_LayoutItem(0, flags), indexLayoutItem);
	UT_ASSERT(!err);
}

XAP_Menu_Id EV_Menu_Layout::addLayoutItem(UT_uint32 indexLayoutItem, EV_Menu_LayoutFlags flags)
{
	UT_uint32 err = m_layoutTable.insertItemAt(new EV_Menu_LayoutItem(++m_iMaxId, flags), indexLayoutItem);

	if (err)
		return 0;
	else
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

UT_uint32 EV_Menu_Layout::getLayoutIndex(XAP_Menu_Id id) const
{
	UT_uint32 size_table = m_layoutTable.size();
	UT_uint32 index;

	for (index = 0; index < size_table; ++index)
	{
		if ((static_cast<EV_Menu_LayoutItem*> (m_layoutTable[index]))->getMenuId() == id)
			break;
	}

	return index < size_table ? index : 0;
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

