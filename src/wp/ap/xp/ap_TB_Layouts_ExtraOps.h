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

BeginLayout(ExtraOps, AP_STRING_ID_TB_Extra, AP_PREF_KEY_ExtraBarVisible)
     ToolbarItem(AP_TOOLBAR_ID_FMT_HYPERLINK)
     ToolbarItem(AP_TOOLBAR_ID_FMT_BOOKMARK)
     Spacer ()

	ToolbarItem(AP_TOOLBAR_ID_FMT_OVERLINE)
	ToolbarItem(AP_TOOLBAR_ID_FMT_STRIKE)
	//ToolbarItem(AP_TOOLBAR_ID_FMT_TOPLINE)
	//ToolbarItem(AP_TOOLBAR_ID_FMT_BOTTOMLINE)

	Spacer()
	ToolbarItem(AP_TOOLBAR_ID_FMT_SUPERSCRIPT)
	ToolbarItem(AP_TOOLBAR_ID_FMT_SUBSCRIPT)
	ToolbarItem(AP_TOOLBAR_ID_INSERT_SYMBOL)

	//Spacer()
	//ToolbarItem(AP_TOOLBAR_ID_SCRIPT_PLAY)

	Spacer()
	ToolbarItem(AP_TOOLBAR_ID_PARA_0BEFORE)
	ToolbarItem(AP_TOOLBAR_ID_PARA_12BEFORE)

	Spacer()
	ToolbarItem(AP_TOOLBAR_ID_SINGLE_SPACE)
	ToolbarItem(AP_TOOLBAR_ID_MIDDLE_SPACE)
	ToolbarItem(AP_TOOLBAR_ID_DOUBLE_SPACE)

	Spacer()
	//ToolbarItem(AP_TOOLBAR_ID_FMT_DIRECTION)
	ToolbarItem(AP_TOOLBAR_ID_FMT_DIR_OVERRIDE_LTR)
	ToolbarItem(AP_TOOLBAR_ID_FMT_DIR_OVERRIDE_RTL)
	ToolbarItem(AP_TOOLBAR_ID_FMT_DOM_DIRECTION)

	Spacer()
	ToolbarItem(AP_TOOLBAR_ID_EDIT_HEADER)
	ToolbarItem(AP_TOOLBAR_ID_EDIT_FOOTER)

	Spacer()
	ToolbarItem(AP_TOOLBAR_ID_EDIT_REMOVEHEADER)
	ToolbarItem(AP_TOOLBAR_ID_EDIT_REMOVEFOOTER)

#ifndef TOOLKIT_WIN
	Spacer()
	ToolbarItem(AP_TOOLBAR_ID_SEMITEM_THIS)
	ToolbarItem(AP_TOOLBAR_ID_SEMITEM_NEXT)
	ToolbarItem(AP_TOOLBAR_ID_SEMITEM_PREV)
	ToolbarItem(AP_TOOLBAR_ID_SEMITEM_EDIT)
	ToolbarItem(AP_TOOLBAR_ID_SEMITEM_STYLESHEET_APPLY)

#endif

EndLayout()
