/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */ 


#include "ut_types.h"
#include "ut_assert.h"
#include "ev_Menu_Actions.h"
#include "ap_Menu_ActionSet.h"
#include "ap_Menu_Functions.h"
#include "ap_Menu_Id.h"
	
EV_Menu_ActionSet * AP_CreateMenuActionSet(void)
{
	// This should only be called once by the application.
	// Everyone should share the set we create.

	EV_Menu_ActionSet * pActionSet = new EV_Menu_ActionSet(AP_MENU_ID__BOGUS1__,
														   AP_MENU_ID__BOGUS2__);
	UT_ASSERT(pActionSet);

	// The following is a list of all menu id's that we define,
	// the actions that they should be bound to, and various
	// other small details.  This creates the ActionSet of all
	// possible menu actions.  Order here is not significant and
	// does not necessarily correspond to any actual menu.
	// Elsewhere we define one or more MenuLayouts using these
	// verbs....

#define _s(id,a,b,c,d,e,f) pActionSet->setAction(id,a,b,c,d,e,f)

	_s(AP_MENU_ID__BOGUS1__,		0,0,0,	NULL,				NULL,				NULL);

	_s(AP_MENU_ID_FILE,				1,0,0,	NULL,				NULL,				NULL);
	_s(AP_MENU_ID_FILE_NEW,			0,0,0,	"fileNew",			NULL,				NULL);
	_s(AP_MENU_ID_FILE_OPEN,		0,1,0,	"fileOpen",			NULL,				NULL);
	_s(AP_MENU_ID_FILE_SAVE,		0,0,0,	"fileSave",			ap_GetState_Save,	NULL);
	_s(AP_MENU_ID_FILE_SAVEAS,		0,1,0,	"fileSaveAs",		NULL,				NULL);

	// ... add others here ...
	
	_s(AP_MENU_ID__BOGUS2__,		0,0,0,	NULL,				NULL,				NULL);

#undef _s
	
	return pActionSet;
}
