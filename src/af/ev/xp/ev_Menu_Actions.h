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

#ifndef EV_MENU_ACTIONS_H
#define EV_MENU_ACTIONS_H

/****************************************************************
*****************************************************************
** This file defines a framework for the set of actions which
** may be bound to a menu Id.  This binding is independent of
** the actual menu containing the item and actual menu layout.
**
** For example, we may have "File|Open" on the unique menus
** for three different types of top-level windows and on a
** context menu, but they all do the same fire the same event
** (have the same binding).
**
** We create one EV_Menu_Action per menu-item per application.
**
** We create one EV_Menu_ActionSet per application.
**
*****************************************************************
****************************************************************/

#include "ut_types.h"
#include "ap_Menu_Id.h"
class AP_App;
class FV_View;

// TODO consider removing bHoldsSubMenu bit from this file.

/*****************************************************************/

typedef enum _ev_Menu_ItemState			/* values may be ORed */
{
	EV_MIS_ZERO		= 0x00,				/* nothing is turned on */
	EV_MIS_Gray		= 0x01,				/* item is or should be gray */
	EV_MIS_Toggled	= 0x02,				/* checkable item should be checked */

} EV_Menu_ItemState;

typedef EV_Menu_ItemState ( EV_GetMenuItemState_Fn )(FV_View * pView, AP_Menu_Id id);
typedef EV_Menu_ItemState (*EV_GetMenuItemState_pFn)(FV_View * pView, AP_Menu_Id id);
#define Defun_EV_GetMenuItemState_Fn(fn) EV_Menu_ItemState fn(FV_View * pView, AP_Menu_Id id)

// TODO decide if ...GetMenuItemComputedLabel... should take an AP_App or an FV_View.
// TODO for most-recently-used-file-list and window-history, we probably just need
// TODO the ap.  but for view-specific things (like toggles where we change the menu
// TODO item name rather than doing a checkmark), we need the view.

typedef const char * ( EV_GetMenuItemComputedLabel_Fn )(AP_App * pApp, AP_Menu_Id id);
typedef const char * (*EV_GetMenuItemComputedLabel_pFn)(AP_App * pApp, AP_Menu_Id id);
#define Defun_EV_GetMenuItemComputedLabel_Fn(fn) const char * fn(AP_App * pApp, AP_Menu_Id id)

/*****************************************************************/

class EV_Menu_Action
{
public:
	EV_Menu_Action(AP_Menu_Id id,
				   UT_Bool bHoldsSubMenu,
				   UT_Bool bRaisesDialog,
				   UT_Bool bCheckable,
				   const char * szMethodName,
				   EV_GetMenuItemState_pFn pfnGetState,
				   EV_GetMenuItemComputedLabel_pFn pfnGetLabel);
	~EV_Menu_Action(void);

	AP_Menu_Id						getMenuId(void) const;
	const char *					getDynamicLabel(AP_App * pApp) const;
	const char *					getMethodName(void) const;
	UT_Bool							raisesDialog(void) const;
	
protected:
	AP_Menu_Id						m_id;
	UT_Bool							m_bHoldsSubMenu;	/* is a PullRight */
	UT_Bool							m_bRaisesDialog;	/* does it raise a dialog */
	UT_Bool							m_bCheckable;		/* is it checkable */
	char *							m_szMethodName;		/* name of method to invoke */
	EV_GetMenuItemState_pFn			m_pfnGetState;		/* to get state on an activate */
	EV_GetMenuItemComputedLabel_pFn m_pfnGetLabel;		/* to get computed label (for things like window-list) */
};

/*****************************************************************/

class EV_Menu_ActionSet					/* a glorified array with bounds checking */
{
public:
	EV_Menu_ActionSet(AP_Menu_Id first, AP_Menu_Id last);
	~EV_Menu_ActionSet(void);

	UT_Bool				setAction(AP_Menu_Id id,
								  UT_Bool bHoldsSubMenu,
								  UT_Bool bRaisesDialog,
								  UT_Bool bCheckable,
								  const char * szMethodName,
								  EV_GetMenuItemState_pFn pfnGetState,
								  EV_GetMenuItemComputedLabel_pFn pfnGetLabel);
	EV_Menu_Action *	getAction(AP_Menu_Id id) const;

protected:
	EV_Menu_Action **	m_actionTable;
	AP_Menu_Id			m_first;
	AP_Menu_Id			m_last;
};

#endif /* EV_MENU_ACTIONS_H */
