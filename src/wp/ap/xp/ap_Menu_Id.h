 
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

#ifndef AP_MENU_IDSET_H
#define AP_MENU_IDSET_H

/*****************************************************************/
/*****************************************************************/
/** This file defines the set of Id's used for all menu-related **/
/** things.  Each Id defines a conceptual unit which may be     **/
/** used on one or more menus or not at all.                    **/
/*****************************************************************/
/*****************************************************************/

/* the following Id's must start at zero and be contiguous */

typedef enum _Ap_Menu_Id
{
	AP_MENU_ID__BOGUS1__ = 0,			/* must be first */

	AP_MENU_ID_FILE,
	AP_MENU_ID_FILE_NEW,
	AP_MENU_ID_FILE_OPEN,
	AP_MENU_ID_FILE_SAVE,
	AP_MENU_ID_FILE_SAVEAS,

	/* ... add others here ... */

	AP_MENU_ID__BOGUS2__				/* must be last */

} AP_Menu_Id;

#endif /* AP_MENU_IDSET_H */
