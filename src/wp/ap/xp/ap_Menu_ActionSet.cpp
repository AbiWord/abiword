 
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
