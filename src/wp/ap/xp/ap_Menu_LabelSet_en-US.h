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

// We use the Win32 '&' character to denote a keyboard accelerator on a menu item.
// If your platform doesn't have a way to do accelerators or uses a different
// character, remove or change the '&' in your menu constructor code.

BeginSet(EnUS)

	MenuLabel(AP_MENU_ID__BOGUS1__,			NULL,NULL,NULL)

	//       (id,                       	szLabel,           	szToolTip,      szStatusMsg)

	MenuLabel(AP_MENU_ID_FILE,				"&File",			NULL, NULL)
	MenuLabel(AP_MENU_ID_FILE_NEW,			"&New", 			NULL, "Create a new document")	
	MenuLabel(AP_MENU_ID_FILE_OPEN,			"&Open",			NULL, "Open an existing document")
	MenuLabel(AP_MENU_ID_FILE_CLOSE,		"&Close", 			NULL, NULL)
	MenuLabel(AP_MENU_ID_FILE_SAVE,			"&Save", 			NULL, "Save the current document")
	MenuLabel(AP_MENU_ID_FILE_SAVEAS,		"Save &As", 		NULL, "Save the current document under a different name")
	MenuLabel(AP_MENU_ID_FILE_PAGESETUP,	"Page Set&up",		NULL, NULL)
	MenuLabel(AP_MENU_ID_FILE_PRINT,		"&Print",			NULL, NULL)
	MenuLabel(AP_MENU_ID_FILE_EXIT,			"E&xit", 			NULL, NULL)

	MenuLabel(AP_MENU_ID_EDIT,				"&Edit",			NULL, NULL)
	MenuLabel(AP_MENU_ID_EDIT_UNDO,			"&Undo",			NULL, NULL)
	MenuLabel(AP_MENU_ID_EDIT_REDO,			"&Redo",			NULL, NULL)
	MenuLabel(AP_MENU_ID_EDIT_CUT,			"Cu&t",				NULL, NULL)
	MenuLabel(AP_MENU_ID_EDIT_COPY,			"&Copy",			NULL, NULL)
	MenuLabel(AP_MENU_ID_EDIT_PASTE,		"&Paste",			NULL, NULL)
	MenuLabel(AP_MENU_ID_EDIT_CLEAR,		"Cle&ar",			NULL, NULL)
	MenuLabel(AP_MENU_ID_EDIT_SELECTALL,	"Select A&ll",		NULL, NULL)
	MenuLabel(AP_MENU_ID_EDIT_FIND,			"&Find",			NULL, NULL)
	MenuLabel(AP_MENU_ID_EDIT_REPLACE,		"R&eplace",			NULL, NULL)

	MenuLabel(AP_MENU_ID_FORMAT,			"F&ormat",			NULL, NULL)
	MenuLabel(AP_MENU_ID_FMT_FONT,			"&Font",			NULL, NULL)
	MenuLabel(AP_MENU_ID_FMT_PARAGRAPH,		"&Paragraph",		NULL, NULL)
	MenuLabel(AP_MENU_ID_FMT_TABS,			"&Tabs",			NULL, NULL)
	MenuLabel(AP_MENU_ID_FMT_BOLD,			"&Bold",			NULL, NULL)
	MenuLabel(AP_MENU_ID_FMT_ITALIC,		"&Italic",			NULL, NULL)
	MenuLabel(AP_MENU_ID_FMT_UNDERLINE,		"&Underline",		NULL, NULL)
	MenuLabel(AP_MENU_ID_FMT_STRIKE,		"Stri&ke",			NULL, NULL)

	MenuLabel(AP_MENU_ID_ALIGN,				"&Align",			NULL, NULL)
	MenuLabel(AP_MENU_ID_ALIGN_LEFT,		"&Left",			NULL, NULL)
	MenuLabel(AP_MENU_ID_ALIGN_CENTER,		"&Center",			NULL, NULL)
	MenuLabel(AP_MENU_ID_ALIGN_RIGHT,		"&Right",			NULL, NULL)
	MenuLabel(AP_MENU_ID_ALIGN_JUSTIFY,		"&Justify",			NULL, NULL)

	MenuLabel(AP_MENU_ID_HELP,				"&Help",			NULL, NULL)
	MenuLabel(AP_MENU_ID_HELP_READSRC,		"Read the source",	NULL, NULL)
	MenuLabel(AP_MENU_ID_HELP_FIXBUGS,		"Fix bugs",			NULL, NULL)

	MenuLabel(AP_MENU_ID_WINDOW,			"&Window",			NULL,	NULL)
	MenuLabel(AP_MENU_ID_WINDOW_NEW,		"&New Window",		NULL,	"Open another window for the document")
	MenuLabel(AP_MENU_ID_WINDOW_1,			"&1 %s",			NULL,	"Raise this window")
	MenuLabel(AP_MENU_ID_WINDOW_2,			"&2 %s",			NULL,	"Raise this window")
	MenuLabel(AP_MENU_ID_WINDOW_3,			"&3 %s",			NULL,	"Raise this window")
	MenuLabel(AP_MENU_ID_WINDOW_4,			"&4 %s",			NULL,	"Raise this window")
	MenuLabel(AP_MENU_ID_WINDOW_5,			"&5 %s",			NULL,	"Raise this window")
	MenuLabel(AP_MENU_ID_WINDOW_6,			"&6 %s",			NULL,	"Raise this window")
	MenuLabel(AP_MENU_ID_WINDOW_7,			"&7 %s",			NULL,	"Raise this window")
	MenuLabel(AP_MENU_ID_WINDOW_8,			"&8 %s",			NULL,	"Raise this window")
	MenuLabel(AP_MENU_ID_WINDOW_9,			"&9 %s",			NULL,	"Raise this window")
	MenuLabel(AP_MENU_ID_WINDOW_MORE,		"&More Windows",	NULL,	"Show full list of windows")

	// ... add others here ...

	MenuLabel(AP_MENU_ID__BOGUS2__,			NULL,NULL,NULL)

EndSet()
