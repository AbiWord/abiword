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

#pragma once

#include <string>
#include <vector>

#include "ut_types.h"
#include "xap_Types.h"

/*****************************************************************
******************************************************************
** This file defines the basis for defining a menu layout.  A
** menu layout describes the actual items and their ordering
** for a specific menu.  With this we can do things like have
** simple (novice) or complex (expert) menus for a window and/or
** have different menus for different types of windows (such as
** normal-view vs outline-view vs page-preview-view).
******************************************************************
*****************************************************************/

typedef enum _ev_Menu_LayoutFlags {
    EV_MLF_Normal,
    EV_MLF_BeginSubMenu,
    EV_MLF_EndSubMenu,
    EV_MLF_BeginPopupMenu,
    EV_MLF_EndPopupMenu,
    EV_MLF_Separator

} EV_Menu_LayoutFlags;

/*****************************************************************/

class ABI_EXPORT EV_Menu_LayoutItem
{
public:
    EV_Menu_LayoutItem(XAP_Menu_Id id, EV_Menu_LayoutFlags flags);
    ~EV_Menu_LayoutItem();

    XAP_Menu_Id getMenuId() const;
    EV_Menu_LayoutFlags getMenuLayoutFlags() const;

private:
    XAP_Menu_Id m_id;
    EV_Menu_LayoutFlags m_flags;
};

/*****************************************************************/

class EV_Menu_LabelSet;

class ABI_EXPORT EV_Menu_Layout /* a glorified array with bounds checking */
{
public:
    EV_Menu_Layout(const std::string& szName, UT_uint32 nrLayoutItems);
    ~EV_Menu_Layout();

    bool setLayoutItem(UT_uint32 indexLayoutItem, XAP_Menu_Id id, EV_Menu_LayoutFlags flags);
    XAP_Menu_Id addLayoutItem(UT_uint32 indexLayoutItem, EV_Menu_LayoutFlags flags);
    void addFakeLayoutItem(UT_uint32 indexLayoutItem, EV_Menu_LayoutFlags flags);
    EV_Menu_LayoutItem* getLayoutItem(UT_uint32 indexLayoutItem) const;
    UT_uint32 getLayoutIndex(XAP_Menu_Id id) const;
    const std::string& getName() const;
    UT_uint32 getLayoutItemCount() const;
    inline UT_uint32 size() const { return getLayoutItemCount(); }

private:
    std::string m_stName; /* the name of our layout (like "MainMenu") */
    std::vector<EV_Menu_LayoutItem*> m_layoutTable;
    XAP_Menu_Id m_iMaxId;
};
