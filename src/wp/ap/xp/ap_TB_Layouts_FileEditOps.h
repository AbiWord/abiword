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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */


/*****************************************************************
******************************************************************
** IT IS IMPORTANT THAT THIS FILE ALLOW ITSELF TO BE INCLUDED
** MORE THAN ONE TIME.
******************************************************************
*****************************************************************/

BeginLayout(FileEditOps)

	ToolbarItem(AP_TOOLBAR_ID_FILE_NEW)
	ToolbarItem(AP_TOOLBAR_ID_FILE_OPEN)
	ToolbarItem(AP_TOOLBAR_ID_FILE_SAVE)
	ToolbarItem(AP_TOOLBAR_ID_FILE_SAVEAS)

	Spacer()
	ToolbarItem(AP_TOOLBAR_ID_FILE_PRINT)

	Spacer()
	ToolbarItem(AP_TOOLBAR_ID_EDIT_UNDO)
	ToolbarItem(AP_TOOLBAR_ID_EDIT_REDO)

	Spacer()
	ToolbarItem(AP_TOOLBAR_ID_EDIT_CUT)
	ToolbarItem(AP_TOOLBAR_ID_EDIT_COPY)
	ToolbarItem(AP_TOOLBAR_ID_EDIT_PASTE)

	Spacer()
	ToolbarItem(AP_TOOLBAR_ID_1COLUMN)
	ToolbarItem(AP_TOOLBAR_ID_2COLUMN)
	ToolbarItem(AP_TOOLBAR_ID_3COLUMN)

	Spacer()
	ToolbarItem(AP_TOOLBAR_ID_ZOOM)
EndLayout()
