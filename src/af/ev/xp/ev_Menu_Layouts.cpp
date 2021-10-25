/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2021 Hubert Figuière
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

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_misc.h"
#include "ut_std_vector.h"
#include "ut_string.h"

#include "ev_Menu_Labels.h"
#include "ev_Menu_Layouts.h"

/*****************************************************************/

EV_Menu_LayoutItem::EV_Menu_LayoutItem(XAP_Menu_Id id, EV_Menu_LayoutFlags flags)
    : m_id(id)
    , m_flags(flags)
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

static inline XAP_Menu_Id private_max(XAP_Menu_Id a, XAP_Menu_Id b)
{
    return a < b ? b : a;
}

EV_Menu_Layout::EV_Menu_Layout(const std::string& stName, UT_uint32 nrLayoutItems)
    : m_stName(stName)
    , m_layoutTable(nrLayoutItems, nullptr)
    , m_iMaxId(0)
{
}

EV_Menu_Layout::~EV_Menu_Layout()
{
    UT_std_vector_sparsepurgeall(m_layoutTable);
}

void EV_Menu_Layout::addFakeLayoutItem(UT_uint32 indexLayoutItem, EV_Menu_LayoutFlags flags)
{
    m_layoutTable.insert(m_layoutTable.begin() + indexLayoutItem,
        new EV_Menu_LayoutItem(0, flags));
}

XAP_Menu_Id EV_Menu_Layout::addLayoutItem(UT_uint32 indexLayoutItem, EV_Menu_LayoutFlags flags)
{
    auto iter = m_layoutTable.emplace(m_layoutTable.begin() + indexLayoutItem,
        new EV_Menu_LayoutItem(++m_iMaxId, flags));

    if (iter == m_layoutTable.end()) {
        return 0;
    } else {
        return m_iMaxId;
    }
}

bool EV_Menu_Layout::setLayoutItem(UT_uint32 indexLayoutItem, XAP_Menu_Id id, EV_Menu_LayoutFlags flags)
{
    UT_ASSERT(indexLayoutItem < m_layoutTable.size());
    m_iMaxId = private_max(m_iMaxId, id);
    EV_Menu_LayoutItem* pOld = NULL;
    pOld = m_layoutTable[indexLayoutItem];
    m_layoutTable[indexLayoutItem] = new EV_Menu_LayoutItem(id, flags);
    DELETEP(pOld);
    return (m_layoutTable[indexLayoutItem] != NULL);
}

UT_uint32 EV_Menu_Layout::getLayoutIndex(XAP_Menu_Id id) const
{
    UT_sint32 size_table = m_layoutTable.size();
    UT_sint32 index;

    for (index = 0; index < size_table; ++index) {
        if ((m_layoutTable[index])->getMenuId() == id) {
            break;
        }
    }

    return ((index < size_table) ? index : 0);
}

EV_Menu_LayoutItem* EV_Menu_Layout::getLayoutItem(UT_uint32 indexLayoutItem) const
{
    UT_ASSERT(indexLayoutItem < m_layoutTable.size());
    return m_layoutTable[indexLayoutItem];
}

const std::string& EV_Menu_Layout::getName() const
{
    return m_stName;
}

UT_uint32 EV_Menu_Layout::getLayoutItemCount() const
{
    return m_layoutTable.size();
}
