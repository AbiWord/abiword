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


/*****************************************************************
******************************************************************
** IT IS IMPORTANT THAT THIS FILE ALLOW ITSELF TO BE INCLUDED
** MORE THAN ONE TIME.
******************************************************************
*****************************************************************/

BeginLayout(SimpleOps, AP_STRING_ID_TB_Simple, AP_PREF_KEY_SimpleBarVisible)

	ToolbarItem(AP_TOOLBAR_ID_FMT_CHOOSE)

	Spacer()
	ToolbarItem(AP_TOOLBAR_ID_FMT_BOLD)
	ToolbarItem(AP_TOOLBAR_ID_FMT_ITALIC)
	ToolbarItem(AP_TOOLBAR_ID_FMT_UNDERLINE)

	Spacer()
	ToolbarItem(AP_TOOLBAR_ID_ALIGN_LEFT)
	ToolbarItem(AP_TOOLBAR_ID_ALIGN_CENTER)
	ToolbarItem(AP_TOOLBAR_ID_ALIGN_RIGHT)
	ToolbarItem(AP_TOOLBAR_ID_ALIGN_JUSTIFY)

	Spacer()
	ToolbarItem(AP_TOOLBAR_ID_UNINDENT)
	ToolbarItem(AP_TOOLBAR_ID_INDENT)

	Spacer()
	ToolbarItem(AP_TOOLBAR_ID_EDIT_UNDO)
	ToolbarItem(AP_TOOLBAR_ID_EDIT_REDO)

	Spacer()
	ToolbarItem(AP_TOOLBAR_ID_VIEW_FULL_SCREEN)

EndLayout()
