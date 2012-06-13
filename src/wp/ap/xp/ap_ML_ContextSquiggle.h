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

BeginLayout(ContextSquiggle,EV_EMC_MISSPELLEDTEXT)

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
	EndPopupMenu()

EndLayout()
