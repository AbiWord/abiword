 
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

#ifndef EV_MENU_LAYOUTS_H
#define EV_MENU_LAYOUTS_H

#include "ap_Menu_Id.h"
#include "ut_types.h"

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

typedef enum _ev_Menu_LayoutFlags
{
	EV_MLF_Normal,
	EV_MLF_BeginSubMenu,
	EV_MLF_EndSubMenu,
	EV_MLF_Separator

} EV_Menu_LayoutFlags;

/*****************************************************************/

class EV_Menu_LayoutItem
{
public:
	EV_Menu_LayoutItem(AP_Menu_Id id, EV_Menu_LayoutFlags flags);
	~EV_Menu_LayoutItem(void);

	AP_Menu_Id						getMenuId(void) const;
	EV_Menu_LayoutFlags				getMenuLayoutFlags(void) const;

protected:
	AP_Menu_Id						m_id;
	EV_Menu_LayoutFlags				m_flags;
};

/*****************************************************************/

class EV_Menu_Layout					/* a glorified array with bounds checking */
{
public:
	EV_Menu_Layout(const char * szName, UT_uint32 nrLayoutItems);
	~EV_Menu_Layout(void);

	UT_Bool					setLayoutItem(UT_uint32 indexLayoutItem, AP_Menu_Id id, EV_Menu_LayoutFlags flags);
	EV_Menu_LayoutItem *	getLayoutItem(UT_uint32 indexLayoutItem) const;
	const char *			getName(void) const;

protected:
	char *					m_szName;
	UT_uint32				m_nrLayoutItems;
	EV_Menu_LayoutItem **	m_layoutTable;
};

#endif /* EV_MENU_LAYOUTS_H */
