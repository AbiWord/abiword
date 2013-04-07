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

BeginLayout(ContextText,EV_EMC_TEXT)

	BeginPopupMenu()
		MenuItem(AP_MENU_ID_EDIT_CUT)
		MenuItem(AP_MENU_ID_EDIT_COPY)
		MenuItem(AP_MENU_ID_EDIT_PASTE)
   	        MenuItem(AP_MENU_ID_EDIT_PASTE_SPECIAL)
		Separator()
	BeginSubMenu(AP_MENU_ID_TABLE)
		MenuItem(AP_MENU_ID_TABLE_INSERTTABLE)
		MenuItem(AP_MENU_ID_TABLE_DELETETABLE)
		Separator()
		BeginSubMenu(AP_MENU_ID_INSERT)
			MenuItem(AP_MENU_ID_TABLE_INSERT_COLUMNS_BEFORE)
			MenuItem(AP_MENU_ID_TABLE_INSERT_COLUMNS_AFTER)
			MenuItem(AP_MENU_ID_TABLE_INSERT_ROWS_BEFORE)
			MenuItem(AP_MENU_ID_TABLE_INSERT_ROWS_AFTER)
		EndSubMenu()
		MenuItem(AP_MENU_ID_TABLE_DELETEROW)
		MenuItem(AP_MENU_ID_TABLE_DELETECOLUMN)
		MenuItem(AP_MENU_ID_TABLE_MERGE_CELLS)
		MenuItem(AP_MENU_ID_TABLE_SPLIT_CELLS)
		Separator()
		MenuItem(AP_MENU_ID_TABLE_FORMAT)
	EndSubMenu()
// RIVERA
	BeginSubMenu(AP_MENU_ID_TOOLS_ANNOTATIONS)
		MenuItem(AP_MENU_ID_TOOLS_ANNOTATIONS_INSERT)
		MenuItem(AP_MENU_ID_TOOLS_ANNOTATIONS_INSERT_FROMSEL)
//		Separator()
//		MenuItem(AP_MENU_ID_TOOLS_ANNOTATIONS_TOGGLE_DISPLAY)
	EndSubMenu()
	MenuItem(AP_MENU_ID_INSERT_HYPERLINK)
		Separator()
		MenuItem(AP_MENU_ID_FMT_FONT)
	    MenuItem(AP_MENU_ID_FMT_LANGUAGE)
		MenuItem(AP_MENU_ID_FMT_PARAGRAPH)
	    MenuItem(AP_MENU_ID_FMT_BULLETS)
	EndPopupMenu()

EndLayout()
