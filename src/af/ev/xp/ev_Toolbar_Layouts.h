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
 


#ifndef EV_TOOLBAR_LAYOUTS_H
#define EV_TOOLBAR_LAYOUTS_H

#include "ap_Toolbar_Id.h"
#include "ut_types.h"

/*****************************************************************
******************************************************************
** This file defines the basis for defining a toolbar layout.  A
** toolbar layout describes the actual items and their ordering
** for a specific toolbar.  With this we can do things like have
** simple (novice) or complex (expert) toolbars for a window and/or
** have different toolbars for different types of windows (such as
** normal-view vs outline-view vs page-preview-view).
******************************************************************
*****************************************************************/

typedef enum _ev_Toolbar_LayoutFlags
{
	EV_TLF_Normal,
	EV_TLF_Spacer

} EV_Toolbar_LayoutFlags;

/*****************************************************************/

class EV_Toolbar_LayoutItem
{
public:
	EV_Toolbar_LayoutItem(AP_Toolbar_Id id, EV_Toolbar_LayoutFlags flags);
	~EV_Toolbar_LayoutItem(void);

	AP_Toolbar_Id				getToolbarId(void) const;
	EV_Toolbar_LayoutFlags		getToolbarLayoutFlags(void) const;

protected:
	AP_Toolbar_Id				m_id;
	EV_Toolbar_LayoutFlags		m_flags;
};

/*****************************************************************/

class EV_Toolbar_Layout					/* a glorified array with bounds checking */
{
public:
	EV_Toolbar_Layout(const char * szName, UT_uint32 nrLayoutItems);
	~EV_Toolbar_Layout(void);

	UT_Bool						setLayoutItem(UT_uint32 indexLayoutItem,
											  AP_Toolbar_Id id, EV_Toolbar_LayoutFlags flags);
	EV_Toolbar_LayoutItem *		getLayoutItem(UT_uint32 indexLayoutItem) const;
	const char *				getName(void) const;
	UT_uint32					getLayoutItemCount(void) const;

protected:
	char *						m_szName;			/* the name of our layout (like "MainToolbar") */
	UT_uint32					m_nrLayoutItems;
	EV_Toolbar_LayoutItem **	m_layoutTable;
};

#endif /* EV_TOOLBAR_LAYOUTS_H */
