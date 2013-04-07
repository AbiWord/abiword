/* AbiWord
 * Copyright (C) 2005 Martin Sevior <msevior@physics.unimel.edu.au>
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

/*****************************************************************
******************************************************************
** IT IS IMPORTANT THAT THIS FILE ALLOW ITSELF TO BE INCLUDED
** MORE THAN ONE TIME.
******************************************************************
*****************************************************************/

BeginLayout(ContextEmbedLayoutT,EV_EMC_EMBED)

	BeginPopupMenu()
		MenuItem(AP_MENU_ID_FMT_EMBED)
		MenuItem(AP_MENU_ID_FILE_SAVEEMBED)
		Separator()
		MenuItem(AP_MENU_ID_EDIT_CUTEMBED)
		MenuItem(AP_MENU_ID_EDIT_COPYEMBED)
		MenuItem(AP_MENU_ID_EDIT_DELETEEMBED)
	EndPopupMenu()

EndLayout()

