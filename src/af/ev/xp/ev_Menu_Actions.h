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
#include "xap_Types.h"
#include "ut_vector.h"

class XAP_App;
class XAP_Frame;
class AV_View;
class EV_Menu_Label;


// TODO consider removing bHoldsSubMenu bit from this file.

/*****************************************************************/

typedef enum _ev_Menu_ItemState			/* values may be ORed */
{
	EV_MIS_ZERO		= 0x00,				/* nothing is turned on */
	EV_MIS_Gray		= 0x01,				/* item is or should be gray */
	EV_MIS_Toggled 	= 0x02,				/* checkable item should be checked */
	EV_MIS_Bold		= 0x04				/* item is or should be bold */

} EV_Menu_ItemState;

typedef EV_Menu_ItemState ( EV_GetMenuItemState_Fn )(AV_View * pView, XAP_Menu_Id id);
typedef EV_Menu_ItemState (*EV_GetMenuItemState_pFn)(AV_View * pView, XAP_Menu_Id id);
#define Defun_EV_GetMenuItemState_Fn(fn) EV_Menu_ItemState fn(AV_View * pAV_View, XAP_Menu_Id id)

// TODO decide if ...GetMenuItemComputedLabel... should take an XAP_App or an AV_View.
// TODO for most-recently-used-file-list and window-history, we probably just need
// TODO the ap.  but for view-specific things (like toggles where we change the menu
// TODO item name rather than doing a checkmark), we need the view.

// for now, current (quick) compromise is to pass the XAP_Frame, 
// because you can get to either of them easily from there -- pcr

typedef const char * ( EV_GetMenuItemComputedLabel_Fn )(XAP_Frame * pFrame, const EV_Menu_Label * pLabel, XAP_Menu_Id id);
typedef const char * (*EV_GetMenuItemComputedLabel_pFn)(XAP_Frame * pFrame, const EV_Menu_Label * pLabel, XAP_Menu_Id id);
#define Defun_EV_GetMenuItemComputedLabel_Fn(fn) const char * fn(XAP_Frame * pFrame, const EV_Menu_Label * pLabel, XAP_Menu_Id id)

/*****************************************************************/

class EV_Menu_Action
{
public:
	EV_Menu_Action(XAP_Menu_Id id,
				   bool bHoldsSubMenu,
				   bool bRaisesDialog,
				   bool bCheckable,
				   const char * szMethodName,
				   EV_GetMenuItemState_pFn pfnGetState,
				   EV_GetMenuItemComputedLabel_pFn pfnGetLabel);
	~EV_Menu_Action();

	XAP_Menu_Id						getMenuId() const;
	bool							hasDynamicLabel() const;
	const char *					getDynamicLabel(XAP_Frame * pFrame, const EV_Menu_Label * pLabel) const;
	const char *					getMethodName() const;
	bool							hasGetStateFunction() const;
	EV_Menu_ItemState				getMenuItemState(AV_View * pView) const;
	bool							raisesDialog() const;
	bool							isCheckable() const;
	
private:
	XAP_Menu_Id						m_id;
	bool							m_bHoldsSubMenu;	/* is a PullRight */
	bool							m_bRaisesDialog;	/* does it raise a dialog */
	bool							m_bCheckable;		/* is it checkable */
	char *							m_szMethodName;		/* name of method to invoke */
	EV_GetMenuItemState_pFn			m_pfnGetState;		/* to get state on an activate */
	EV_GetMenuItemComputedLabel_pFn m_pfnGetLabel;		/* to get computed label (for things like window-list) */
};

/*****************************************************************/

class EV_Menu_ActionSet					/* a glorified array with bounds checking */
{
public:
	EV_Menu_ActionSet(XAP_Menu_Id first, XAP_Menu_Id last);
	~EV_Menu_ActionSet();

	bool				setAction(XAP_Menu_Id id,
								  bool bHoldsSubMenu,
								  bool bRaisesDialog,
								  bool bCheckable,
								  const char * szMethodName,
								  EV_GetMenuItemState_pFn pfnGetState,
								  EV_GetMenuItemComputedLabel_pFn pfnGetLabel);
	bool				addAction(EV_Menu_Action *pAction);

	EV_Menu_Action *	getAction(XAP_Menu_Id id) const;

private:
	UT_Vector			m_actionTable;
	XAP_Menu_Id			m_first;
};

#endif /* EV_MENU_ACTIONS_H */
