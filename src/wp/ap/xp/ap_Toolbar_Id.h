/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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


#ifndef AP_TOOLBAR_IDSET_H
#define AP_TOOLBAR_IDSET_H

#include "xap_Types.h"

/********************************************************************/
/********************************************************************/
/** This file defines the set of Id's used for all toolbar-related **/
/** things.  Each Id defines a conceptual unit which may be        **/
/** used on one or more toolbars or not at all.                    **/
/********************************************************************/
/********************************************************************/

/* the following Id's must start at zero and be contiguous */

#define toolbaritem(id)	 AP_TOOLBAR_ID_##id,

enum _Ap_Toolbar_Id
{
	AP_TOOLBAR_ID__BOGUS1__ = 0,	/* must be first */
#include "ap_Toolbar_Id_List.h"
	AP_TOOLBAR_ID__BOGUS2__		/* must be last */
};

#undef toolbaritem

#endif /* AP_TOOLBAR_IDSET_H */
