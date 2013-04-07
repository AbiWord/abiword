/* AbiWord
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */


#ifndef AP_MENU_IDSET_H
#define AP_MENU_IDSET_H

#include "xap_Types.h"

/*****************************************************************/
/*****************************************************************/
/** This file defines the set of Id's used for all menu-related **/
/** things.  Each Id defines a conceptual unit which may be     **/
/** used on one or more menus or not at all.                    **/
/*****************************************************************/
/*****************************************************************/

/* the following Id's must start at zero and be contiguous */

#define menuitem(id)			AP_MENU_ID_##id,

enum _Ap_Menu_Id
{
	AP_MENU_ID__BOGUS1__,
#include "ap_Menu_Id_List.h"
	AP_MENU_ID__BOGUS2__
};


#undef menuitem

#endif /* AP_MENU_IDSET_H */
