/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2002 Dom Lachowicz
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

BeginLayout(TableOps, AP_STRING_ID_TB_Table, AP_PREF_KEY_TableBarVisible)

	ToolbarItem(AP_TOOLBAR_ID_INSERT_TABLE)
	ToolbarItem(AP_TOOLBAR_ID_ADD_ROW)
	ToolbarItem(AP_TOOLBAR_ID_ADD_COLUMN)

	Spacer()

	ToolbarItem(AP_TOOLBAR_ID_DELETE_ROW)
	ToolbarItem(AP_TOOLBAR_ID_DELETE_COLUMN)

	Spacer()

	ToolbarItem(AP_TOOLBAR_ID_MERGE_CELLS)
	ToolbarItem(AP_TOOLBAR_ID_SPLIT_CELLS)

EndLayout()
