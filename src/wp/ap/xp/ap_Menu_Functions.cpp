 
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
#include "ap_Menu_Id.h"
#include "ap_Menu_Functions.h"
#include "ev_Menu_Actions.h"


Defun_EV_GetMenuItemState_Fn(ap_GetState_Save)
{
	UT_ASSERT(pView);
	UT_ASSERT(id == AP_MENU_ID_FILE_SAVE);

	EV_Menu_ItemState s = EV_MIS_ZERO;

	// TODO if (file doesn't have a filename)
	// TODO     s |= EV_MIS_Gray;

	return s;
}

