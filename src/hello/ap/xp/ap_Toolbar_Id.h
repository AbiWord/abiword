/* AbiHello
 * Copyright (C) 1999 AbiSource, Inc.
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

enum _Ap_Toolbar_Id
{
	AP_TOOLBAR_ID__BOGUS1__ = 0,			/* must be first */

	AP_TOOLBAR_ID_FILE_CLOSE,

	/* ... add others here ... */

	AP_TOOLBAR_ID__BOGUS2__				/* must be last */

};

#endif /* AP_TOOLBAR_IDSET_H */
