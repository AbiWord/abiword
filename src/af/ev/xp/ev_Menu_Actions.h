 
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

typedef const char * ( EV_GetMenuItemComputedLabel_Fn )(FV_View * pView, AP_Menu_Id id);
typedef const char * (*EV_GetMenuItemComputedLabel_pFn)(FV_View * pView, AP_Menu_Id id);
#define Defun_EV_GetMenuItemComputedLabel_Fn(fn) const char * fn(FV_View * pView, AP_Menu_Id id)

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
