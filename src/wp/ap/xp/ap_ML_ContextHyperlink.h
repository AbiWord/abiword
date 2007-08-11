/* AbiWord
 * Copyright (C) 1998,1999 AbiSource, Inc.
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

BeginLayout(ContextHyperlinkM,EV_EMC_HYPERLINKMISSPELLED)

	BeginPopupMenu()
		MenuItem(AP_MENU_ID_SPELL_SUGGEST_1)
		MenuItem(AP_MENU_ID_SPELL_SUGGEST_2)
		MenuItem(AP_MENU_ID_SPELL_SUGGEST_3)
		MenuItem(AP_MENU_ID_SPELL_SUGGEST_4)
		MenuItem(AP_MENU_ID_SPELL_SUGGEST_5)
		MenuItem(AP_MENU_ID_SPELL_SUGGEST_6)
		MenuItem(AP_MENU_ID_SPELL_SUGGEST_7)
		MenuItem(AP_MENU_ID_SPELL_SUGGEST_8)
		MenuItem(AP_MENU_ID_SPELL_SUGGEST_9)
		Separator()
		MenuItem(AP_MENU_ID_SPELL_IGNOREALL)
		MenuItem(AP_MENU_ID_SPELL_ADD)
		Separator()
		MenuItem(AP_MENU_ID_TOOLS_SPELL)
		Separator()
		MenuItem(AP_MENU_ID_INSERT_GOTO_HYPERLINK)
		MenuItem(AP_MENU_ID_INSERT_EDIT_HYPERLINK)
		MenuItem(AP_MENU_ID_EDIT_COPY_HYPERLINK_LOCATION)
		MenuItem(AP_MENU_ID_INSERT_DELETE_HYPERLINK)
	EndPopupMenu()

EndLayout()

BeginLayout(ContextHyperlinkT,EV_EMC_HYPERLINKTEXT)

	BeginPopupMenu()
		MenuItem(AP_MENU_ID_INSERT_GOTO_HYPERLINK)
		MenuItem(AP_MENU_ID_INSERT_EDIT_HYPERLINK)
		MenuItem(AP_MENU_ID_EDIT_COPY_HYPERLINK_LOCATION)
		MenuItem(AP_MENU_ID_INSERT_DELETE_HYPERLINK)
		Separator()
		MenuItem(AP_MENU_ID_EDIT_CUT)
		MenuItem(AP_MENU_ID_EDIT_COPY)
		MenuItem(AP_MENU_ID_EDIT_PASTE)
	    MenuItem(AP_MENU_ID_EDIT_PASTE_SPECIAL)
		Separator()
     BeginSubMenu(AP_MENU_ID_TABLE)
		MenuItem(AP_MENU_ID_TABLE_INSERTTABLE)
		MenuItem(AP_MENU_ID_TABLE_DELETETABLE)
		Separator()
		MenuItem(AP_MENU_ID_TABLE_INSERTROW)		
		MenuItem(AP_MENU_ID_TABLE_INSERTCOLUMN)
		MenuItem(AP_MENU_ID_TABLE_DELETEROW)
		MenuItem(AP_MENU_ID_TABLE_DELETECOLUMN)
		MenuItem(AP_MENU_ID_TABLE_MERGE_CELLS)
		Separator()
		MenuItem(AP_MENU_ID_TABLE_FORMAT)
     EndSubMenu()
		Separator()
		MenuItem(AP_MENU_ID_FMT_FONT)
	    MenuItem(AP_MENU_ID_FMT_LANGUAGE)
		MenuItem(AP_MENU_ID_FMT_PARAGRAPH)
	    MenuItem(AP_MENU_ID_FMT_BULLETS)
	EndPopupMenu()

EndLayout()

// RIVERA
BeginLayout(ContextAnnotationM,EV_EMC_ANNOTATIONMISSPELLED)

	BeginPopupMenu()
		MenuItem(AP_MENU_ID_SPELL_SUGGEST_1)
		MenuItem(AP_MENU_ID_SPELL_SUGGEST_2)
		MenuItem(AP_MENU_ID_SPELL_SUGGEST_3)
		MenuItem(AP_MENU_ID_SPELL_SUGGEST_4)
		MenuItem(AP_MENU_ID_SPELL_SUGGEST_5)
		MenuItem(AP_MENU_ID_SPELL_SUGGEST_6)
		MenuItem(AP_MENU_ID_SPELL_SUGGEST_7)
		MenuItem(AP_MENU_ID_SPELL_SUGGEST_8)
		MenuItem(AP_MENU_ID_SPELL_SUGGEST_9)
		Separator()
		MenuItem(AP_MENU_ID_SPELL_IGNOREALL)
		MenuItem(AP_MENU_ID_SPELL_ADD)
		Separator()
		MenuItem(AP_MENU_ID_TOOLS_SPELL)
		Separator()
		MenuItem(AP_MENU_ID_GOTO_ANNOTATION)
		MenuItem(AP_MENU_ID_EDIT_ANNOTATION)
		MenuItem(AP_MENU_ID_DELETE_ANNOTATION)
	EndPopupMenu()

EndLayout()

BeginLayout(ContextAnnotationT,EV_EMC_ANNOTATIONTEXT)

	BeginPopupMenu()
		MenuItem(AP_MENU_ID_GOTO_ANNOTATION)
		MenuItem(AP_MENU_ID_EDIT_ANNOTATION)
		MenuItem(AP_MENU_ID_DELETE_ANNOTATION)
		Separator()
		MenuItem(AP_MENU_ID_EDIT_CUT)
		MenuItem(AP_MENU_ID_EDIT_COPY)
		MenuItem(AP_MENU_ID_EDIT_PASTE)
	    MenuItem(AP_MENU_ID_EDIT_PASTE_SPECIAL)
		Separator()
     BeginSubMenu(AP_MENU_ID_TABLE)
		MenuItem(AP_MENU_ID_TABLE_INSERTTABLE)
		MenuItem(AP_MENU_ID_TABLE_DELETETABLE)
		Separator()
		MenuItem(AP_MENU_ID_TABLE_INSERTROW)		
		MenuItem(AP_MENU_ID_TABLE_INSERTCOLUMN)
		MenuItem(AP_MENU_ID_TABLE_DELETEROW)
		MenuItem(AP_MENU_ID_TABLE_DELETECOLUMN)
		MenuItem(AP_MENU_ID_TABLE_MERGE_CELLS)
		Separator()
		MenuItem(AP_MENU_ID_TABLE_FORMAT)
     EndSubMenu()
		Separator()
		MenuItem(AP_MENU_ID_FMT_FONT)
	    MenuItem(AP_MENU_ID_FMT_LANGUAGE)
		MenuItem(AP_MENU_ID_FMT_PARAGRAPH)
	    MenuItem(AP_MENU_ID_FMT_BULLETS)
	EndPopupMenu()

EndLayout()

