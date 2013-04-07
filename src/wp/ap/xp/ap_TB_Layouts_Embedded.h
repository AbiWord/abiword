/* AbiWord
 * Copyright (C) 2007 Tomas Frydrych.
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

BeginLayout(Embedded, AP_STRING_ID_TB_Embedded, AP_PREF_KEY_StandardBarVisible)

	ToolbarItem(AP_TOOLBAR_ID_MENU)

	Spacer()
	ToolbarItem(AP_TOOLBAR_ID_FILE_NEW)
	ToolbarItem(AP_TOOLBAR_ID_FILE_OPEN)
	ToolbarItem(AP_TOOLBAR_ID_FILE_SAVE)

	Spacer()
	ToolbarItem(AP_TOOLBAR_ID_EDIT_CUT)
	ToolbarItem(AP_TOOLBAR_ID_EDIT_COPY)
	ToolbarItem(AP_TOOLBAR_ID_EDIT_PASTE)

	Spacer()
	ToolbarItem(AP_TOOLBAR_ID_EDIT_UNDO)
	ToolbarItem(AP_TOOLBAR_ID_EDIT_REDO)

	Spacer()
	ToolbarItem(AP_TOOLBAR_ID_FMT_BOLD)
	ToolbarItem(AP_TOOLBAR_ID_FMT_ITALIC)

EndLayout()
