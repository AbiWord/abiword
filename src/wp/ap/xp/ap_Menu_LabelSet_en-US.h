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

	MenuLabel(AP_MENU_ID__BOGUS1__,			NULL,				NULL)

	//       (id,                       	szLabel,           	szStatusMsg)

	MenuLabel(AP_MENU_ID_FILE,				"&File",			NULL)
	MenuLabel(AP_MENU_ID_FILE_NEW,			"&New", 			"Create a new document")	
	MenuLabel(AP_MENU_ID_FILE_OPEN,			"&Open",			"Open an existing document")
	MenuLabel(AP_MENU_ID_FILE_CLOSE,		"&Close", 			NULL)
	MenuLabel(AP_MENU_ID_FILE_SAVE,			"&Save", 			"Save the current document")
	MenuLabel(AP_MENU_ID_FILE_SAVEAS,		"Save &As", 		"Save the current document under a different name")
	MenuLabel(AP_MENU_ID_FILE_PAGESETUP,	"Page Set&up",		NULL)
	MenuLabel(AP_MENU_ID_FILE_PRINT,		"&Print",			NULL)
	MenuLabel(AP_MENU_ID_FILE_RECENT_1,		"&1 %s",			"Open this document")
	MenuLabel(AP_MENU_ID_FILE_RECENT_2,		"&2 %s",			"Open this document")
	MenuLabel(AP_MENU_ID_FILE_RECENT_3,		"&3 %s",			"Open this document")
	MenuLabel(AP_MENU_ID_FILE_RECENT_4,		"&4 %s",			"Open this document")
	MenuLabel(AP_MENU_ID_FILE_RECENT_5,		"&5 %s",			"Open this document")
	MenuLabel(AP_MENU_ID_FILE_RECENT_6,		"&6 %s",			"Open this document")
	MenuLabel(AP_MENU_ID_FILE_RECENT_7,		"&7 %s",			"Open this document")
	MenuLabel(AP_MENU_ID_FILE_RECENT_8,		"&8 %s",			"Open this document")
	MenuLabel(AP_MENU_ID_FILE_RECENT_9,		"&9 %s",			"Open this document")
	MenuLabel(AP_MENU_ID_FILE_EXIT,			"E&xit", 			NULL)

	MenuLabel(AP_MENU_ID_EDIT,				"&Edit",			NULL)
	MenuLabel(AP_MENU_ID_EDIT_UNDO,			"&Undo",			NULL)
	MenuLabel(AP_MENU_ID_EDIT_REDO,			"&Redo",			NULL)
	MenuLabel(AP_MENU_ID_EDIT_CUT,			"Cu&t",				NULL)
	MenuLabel(AP_MENU_ID_EDIT_COPY,			"&Copy",			NULL)
	MenuLabel(AP_MENU_ID_EDIT_PASTE,		"&Paste",			NULL)
	MenuLabel(AP_MENU_ID_EDIT_CLEAR,		"Cle&ar",			NULL)
	MenuLabel(AP_MENU_ID_EDIT_SELECTALL,	"Select A&ll",		NULL)
	MenuLabel(AP_MENU_ID_EDIT_FIND,			"&Find",			NULL)
	MenuLabel(AP_MENU_ID_EDIT_REPLACE,		"R&eplace",			NULL)
	MenuLabel(AP_MENU_ID_EDIT_GOTO,			"&Go To",			NULL)
	MenuLabel(AP_MENU_ID_EDIT_OPTIONS,		"&Options",			NULL)
	
	MenuLabel(AP_MENU_ID_VIEW,				"&View",			NULL)
	MenuLabel(AP_MENU_ID_VIEW_TOOLBARS,		"&Toolbars",		NULL)
	MenuLabel(AP_MENU_ID_VIEW_TB_STD,		"&Standard",		NULL)
	MenuLabel(AP_MENU_ID_VIEW_TB_FORMAT,	"&Formatting",		NULL)
	MenuLabel(AP_MENU_ID_VIEW_RULER,		"&Ruler",			NULL)
	MenuLabel(AP_MENU_ID_VIEW_STATUSBAR,	"&Status Bar",		NULL)
	MenuLabel(AP_MENU_ID_VIEW_SHOWPARA,		"Show Para&graphs",	NULL)
	MenuLabel(AP_MENU_ID_VIEW_HEADFOOT,		"&Header and Footer",	NULL)
	MenuLabel(AP_MENU_ID_VIEW_ZOOM,			"&Zoom",			NULL)

	MenuLabel(AP_MENU_ID_INSERT,			"&Insert",			NULL)
	MenuLabel(AP_MENU_ID_INSERT_BREAK,		"&Break",			NULL)
	MenuLabel(AP_MENU_ID_INSERT_PAGENO,		"Page N&umbers",	NULL)
	MenuLabel(AP_MENU_ID_INSERT_DATETIME,	"Date and &Time",	NULL)
	MenuLabel(AP_MENU_ID_INSERT_FIELD,		"&Field",			NULL)
	MenuLabel(AP_MENU_ID_INSERT_SYMBOL,		"&Symbol",			NULL)
	MenuLabel(AP_MENU_ID_INSERT_IMAGE,		"&Picture",			NULL)

	MenuLabel(AP_MENU_ID_FORMAT,			"F&ormat",			NULL)
	MenuLabel(AP_MENU_ID_FMT_FONT,			"&Font",			NULL)
	MenuLabel(AP_MENU_ID_FMT_PARAGRAPH,		"&Paragraph",		NULL)
	MenuLabel(AP_MENU_ID_FMT_BULLETS,		"Bullets and &Numbering",	NULL)
	MenuLabel(AP_MENU_ID_FMT_BORDERS,		"Bor&ders and Shading",		NULL)
	MenuLabel(AP_MENU_ID_FMT_COLUMNS,		"&Columns",			NULL)
	MenuLabel(AP_MENU_ID_FMT_STYLE,			"St&yle",			NULL)
	MenuLabel(AP_MENU_ID_FMT_TABS,			"&Tabs",			NULL)
	MenuLabel(AP_MENU_ID_FMT_BOLD,			"&Bold",			NULL)
	MenuLabel(AP_MENU_ID_FMT_ITALIC,		"&Italic",			NULL)
	MenuLabel(AP_MENU_ID_FMT_UNDERLINE,		"&Underline",		NULL)
	MenuLabel(AP_MENU_ID_FMT_STRIKE,		"Stri&ke",			NULL)

	MenuLabel(AP_MENU_ID_ALIGN,				"&Align",			NULL)
	MenuLabel(AP_MENU_ID_ALIGN_LEFT,		"&Left",			NULL)
	MenuLabel(AP_MENU_ID_ALIGN_CENTER,		"&Center",			NULL)
	MenuLabel(AP_MENU_ID_ALIGN_RIGHT,		"&Right",			NULL)
	MenuLabel(AP_MENU_ID_ALIGN_JUSTIFY,		"&Justify",			NULL)

	MenuLabel(AP_MENU_ID_WINDOW,			"&Window",			NULL)
	MenuLabel(AP_MENU_ID_WINDOW_NEW,		"&New Window",		"Open another window for the document")
	MenuLabel(AP_MENU_ID_WINDOW_1,			"&1 %s",			"Raise this window")
	MenuLabel(AP_MENU_ID_WINDOW_2,			"&2 %s",			"Raise this window")
	MenuLabel(AP_MENU_ID_WINDOW_3,			"&3 %s",			"Raise this window")
	MenuLabel(AP_MENU_ID_WINDOW_4,			"&4 %s",			"Raise this window")
	MenuLabel(AP_MENU_ID_WINDOW_5,			"&5 %s",			"Raise this window")
	MenuLabel(AP_MENU_ID_WINDOW_6,			"&6 %s",			"Raise this window")
	MenuLabel(AP_MENU_ID_WINDOW_7,			"&7 %s",			"Raise this window")
	MenuLabel(AP_MENU_ID_WINDOW_8,			"&8 %s",			"Raise this window")
	MenuLabel(AP_MENU_ID_WINDOW_9,			"&9 %s",			"Raise this window")
	MenuLabel(AP_MENU_ID_WINDOW_MORE,		"&More Windows",	"Show full list of windows")

	MenuLabel(AP_MENU_ID_HELP,				"&Help",			NULL)
	MenuLabel(AP_MENU_ID_HELP_ABOUT,		"&About %s",		NULL)

	// ... add others here ...

	MenuLabel(AP_MENU_ID__BOGUS2__,			NULL,				NULL)

EndSet()
