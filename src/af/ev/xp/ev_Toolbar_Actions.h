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

#ifndef EV_TOOLBAR_ACTIONS_H
#define EV_TOOLBAR_ACTIONS_H

/****************************************************************
*****************************************************************
** This file defines a framework for the set of actions which
** may be bound to a Toolbar Id.  This binding is independent of
** the actual toolbar containing the item and actual toolbar layout.
**
** We create one EV_Toolbar_Action per toolbar-item per application.
**
** We create one EV_Toolbar_ActionSet per application.
**
*****************************************************************
****************************************************************/

#include "ut_types.h"
#include "ap_Toolbar_Id.h"
#include "av_Listener.h"
class AP_App;
class AV_View;
class EV_Toolbar_Label;

/*****************************************************************/

typedef enum _ev_Toolbar_ItemState			/* values may be ORed */
{
	EV_TIS_ZERO				= 0x00,
	EV_TIS_Gray				= 0x01,			/* should be grayed */
	EV_TIS_Toggled			= 0x02,			/* should be pressed down */
	EV_TIS_UseString		= 0x04,			/* should reference pszState */
	
} EV_Toolbar_ItemState;

typedef EV_Toolbar_ItemState ( EV_GetToolbarItemState_Fn )(AV_View * pAV_View, AP_Toolbar_Id id, const char ** pszState);
typedef EV_Toolbar_ItemState (*EV_GetToolbarItemState_pFn)(AV_View * pAV_View, AP_Toolbar_Id id, const char ** pszState);
#define Defun_EV_GetToolbarItemState_Fn(fn) EV_Toolbar_ItemState fn(AV_View * pAV_View, AP_Toolbar_Id id, const char ** pszState)

#define EV_TIS_ShouldBeGray(tis)		(((tis) & EV_TIS_Gray)!=0)
#define EV_TIS_ShouldBeToggled(tis)		(((tis) & EV_TIS_Toggled)!=0)
#define EV_TIS_ShouldUseString(tis)		(((tis) & EV_TIS_UseString)!=0)

/*****************************************************************/

typedef enum _ev_Toolbar_ItemType
{
	EV_TBIT_BOGUS			= 0,
	EV_TBIT_PushButton		= 1,			/* simple push to fire */
	EV_TBIT_ToggleButton	= 2,			/* push-on/push-off */
	EV_TBIT_EditText		= 3,			/* text entry field */
	EV_TBIT_DropDown		= 4,			/* list box w/no text entry */
	EV_TBIT_ComboBox		= 5,			/* list box w/ text entry */
	EV_TBIT_StaticLabel		= 6,			/* a static control */
	EV_TBIT_Spacer			= 7,			/* for extra space between buttons */
	
} EV_Toolbar_ItemType;

/*****************************************************************/

class EV_Toolbar_Action
{
public:
	EV_Toolbar_Action(AP_Toolbar_Id id,
					  EV_Toolbar_ItemType type,
					  const char * szMethodName,
					  AV_ChangeMask maskOfInterest,
					  EV_GetToolbarItemState_pFn pfnGetState);
	~EV_Toolbar_Action(void);

	AP_Toolbar_Id					getToolbarId(void) const;
	EV_Toolbar_ItemType				getItemType(void) const;
	const char *					getMethodName(void) const;
	AV_ChangeMask					getChangeMaskOfInterest(void) const;
	EV_Toolbar_ItemState			getToolbarItemState(AV_View * pView, const char ** pszState) const;
	
protected:
	AP_Toolbar_Id					m_id;
	EV_Toolbar_ItemType				m_type;
	char *							m_szMethodName;		/* name of method to invoke */

	AV_ChangeMask					m_maskOfInterest;
	EV_GetToolbarItemState_pFn		m_pfnGetState;
};

/*****************************************************************/

class EV_Toolbar_ActionSet				/* a glorified array with bounds checking */
{
public:
	EV_Toolbar_ActionSet(AP_Toolbar_Id first, AP_Toolbar_Id last);
	~EV_Toolbar_ActionSet(void);

	UT_Bool				setAction(AP_Toolbar_Id id,
								  EV_Toolbar_ItemType type,
								  const char * szMethodName,
								  AV_ChangeMask maskOfInterest,
								  EV_GetToolbarItemState_pFn pfnGetState);
	EV_Toolbar_Action *	getAction(AP_Toolbar_Id id) const;

protected:
	EV_Toolbar_Action **	m_actionTable;
	AP_Toolbar_Id			m_first;
	AP_Toolbar_Id			m_last;
};

#endif /* EV_TOOLBAR_ACTIONS_H */
