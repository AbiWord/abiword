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
