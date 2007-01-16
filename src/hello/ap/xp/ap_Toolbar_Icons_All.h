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


/*****************************************************************
******************************************************************
** IT IS IMPORTANT THAT THIS FILE ALLOW ITSELF TO BE INCLUDED
** MORE THAN ONE TIME.
******************************************************************
*****************************************************************/

/*****************************************************************
******************************************************************
** FOR EACH ICON YOU ADD, ADD IT TO BOTH SECTIONS OF THIS FILE. **
******************************************************************
*****************************************************************/

#ifndef AP_TOOLBAR_ICONS_ALL_H

#	define AP_TOOLBAR_ICONS_ALL_H

	// Include each toolbar icon that we want to build.

#	include "tb_close.xpm"

	
	// ... add new icons here (don't forget to add below the #else) ...

#else

	// Declare each toolbar icon that we are building.
	
	DefineToolbarIcon(tb_close_xpm)

	// ... also add new icons here ...

#endif
