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

// TODO decide if we want menu items to have ToolTips ??

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
	MenuLabel(AP_MENU_ID_FILE_RECENT_1,		"&1 %s",			NULL, "Open this document")
	MenuLabel(AP_MENU_ID_FILE_RECENT_2,		"&2 %s",			NULL, "Open this document")
	MenuLabel(AP_MENU_ID_FILE_RECENT_3,		"&3 %s",			NULL, "Open this document")
	MenuLabel(AP_MENU_ID_FILE_RECENT_4,		"&4 %s",			NULL, "Open this document")
	MenuLabel(AP_MENU_ID_FILE_RECENT_5,		"&5 %s",			NULL, "Open this document")
	MenuLabel(AP_MENU_ID_FILE_RECENT_6,		"&6 %s",			NULL, "Open this document")
	MenuLabel(AP_MENU_ID_FILE_RECENT_7,		"&7 %s",			NULL, "Open this document")
	MenuLabel(AP_MENU_ID_FILE_RECENT_8,		"&8 %s",			NULL, "Open this document")
	MenuLabel(AP_MENU_ID_FILE_RECENT_9,		"&9 %s",			NULL, "Open this document")
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
	MenuLabel(AP_MENU_ID_EDIT_GOTO,			"&Go To",			NULL, NULL)
	MenuLabel(AP_MENU_ID_EDIT_OPTIONS,		"&Options",			NULL, NULL)
	
	MenuLabel(AP_MENU_ID_VIEW,				"&View",			NULL, NULL)
	MenuLabel(AP_MENU_ID_VIEW_TOOLBARS,		"&Toolbars",		NULL, NULL)
	MenuLabel(AP_MENU_ID_VIEW_TB_STD,		"&Standard",		NULL, NULL)
	MenuLabel(AP_MENU_ID_VIEW_TB_FORMAT,	"&Formatting",		NULL, NULL)
	MenuLabel(AP_MENU_ID_VIEW_RULER,		"&Ruler",			NULL, NULL)
	MenuLabel(AP_MENU_ID_VIEW_STATUSBAR,	"&Status Bar",		NULL, NULL)
	MenuLabel(AP_MENU_ID_VIEW_SHOWPARA,		"Show Para&graphs",	NULL, NULL)
	MenuLabel(AP_MENU_ID_VIEW_HEADFOOT,		"&Header and Footer",	NULL, NULL)
	MenuLabel(AP_MENU_ID_VIEW_ZOOM,			"&Zoom",			NULL, NULL)

	MenuLabel(AP_MENU_ID_INSERT,			"&Insert",			NULL, NULL)
	MenuLabel(AP_MENU_ID_INSERT_BREAK,		"&Break",			NULL, NULL)
	MenuLabel(AP_MENU_ID_INSERT_PAGENO,		"Page N&umbers",	NULL, NULL)
	MenuLabel(AP_MENU_ID_INSERT_DATETIME,	"Date and &Time",	NULL, NULL)
	MenuLabel(AP_MENU_ID_INSERT_FIELD,		"&Field",			NULL, NULL)
	MenuLabel(AP_MENU_ID_INSERT_SYMBOL,		"&Symbol",			NULL, NULL)
	MenuLabel(AP_MENU_ID_INSERT_IMAGE,		"&Picture",			NULL, NULL)

	MenuLabel(AP_MENU_ID_FORMAT,			"F&ormat",			NULL, NULL)
	MenuLabel(AP_MENU_ID_FMT_FONT,			"&Font",			NULL, NULL)
	MenuLabel(AP_MENU_ID_FMT_PARAGRAPH,		"&Paragraph",		NULL, NULL)
	MenuLabel(AP_MENU_ID_FMT_BULLETS,		"Bullets and &Numbering",	NULL, NULL)
	MenuLabel(AP_MENU_ID_FMT_BORDERS,		"Bor&ders and Shading",		NULL, NULL)
	MenuLabel(AP_MENU_ID_FMT_COLUMNS,		"&Columns",			NULL, NULL)
	MenuLabel(AP_MENU_ID_FMT_STYLE,			"St&yle",			NULL, NULL)
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

	MenuLabel(AP_MENU_ID_HELP,				"&Help",			NULL, NULL)
	MenuLabel(AP_MENU_ID_HELP_ABOUT,		"&About %s",		NULL, NULL)

	// ... add others here ...

	MenuLabel(AP_MENU_ID__BOGUS2__,			NULL,NULL,NULL)

EndSet()
