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

/*
  Platform-specific ifdefs are bad, but the one below may be worth
  doing anyway.  It basically allows us to have ONE set of menu
  labels, even though the Windows version of same needs ampersands
  inserted for the purpose of marking the mneumonic underlined
  letters.
*/

#undef AMPERSAND
#ifdef WIN32
#define		AMPERSAND	"&"
#else
#define		AMPERSAND	""
#endif

BeginSet(EnUS)

	MenuLabel(AP_MENU_ID__BOGUS1__,		NULL,NULL,NULL)

	MenuLabel(AP_MENU_ID_FILE,			AMPERSAND "File", NULL, NULL)
	MenuLabel(AP_MENU_ID_FILE_NEW,		AMPERSAND "New", NULL, "Create a new document")	
	MenuLabel(AP_MENU_ID_FILE_OPEN,		AMPERSAND "Open", NULL, "Open an existing document")
	MenuLabel(AP_MENU_ID_FILE_CLOSE,	AMPERSAND "Close", NULL, NULL)
	MenuLabel(AP_MENU_ID_FILE_SAVE,		AMPERSAND "Save", NULL, "Save the current document")
	MenuLabel(AP_MENU_ID_FILE_SAVEAS,	"Save " AMPERSAND "As", NULL, "Save the current document under a different name")
	MenuLabel(AP_MENU_ID_FILE_PAGESETUP,	"Page Set" AMPERSAND "up", NULL, NULL)
	MenuLabel(AP_MENU_ID_FILE_PRINT,	AMPERSAND "Print", NULL, NULL)
	MenuLabel(AP_MENU_ID_FILE_EXIT,		"E" AMPERSAND "xit", NULL, NULL)

	MenuLabel(AP_MENU_ID_EDIT,			AMPERSAND "Edit", NULL, NULL)
	MenuLabel(AP_MENU_ID_EDIT_UNDO,		AMPERSAND "Undo", NULL, NULL)
	MenuLabel(AP_MENU_ID_EDIT_REDO,		AMPERSAND "Redo", NULL, NULL)
	MenuLabel(AP_MENU_ID_EDIT_CUT,		"Cu" AMPERSAND "t", NULL, NULL)
	MenuLabel(AP_MENU_ID_EDIT_COPY,		AMPERSAND "Copy", NULL, NULL)
	MenuLabel(AP_MENU_ID_EDIT_PASTE,	AMPERSAND "Paste", NULL, NULL)
	MenuLabel(AP_MENU_ID_EDIT_CLEAR,	"Cle" AMPERSAND "ar", NULL, NULL)
	MenuLabel(AP_MENU_ID_EDIT_SELECTALL,	"Select A" AMPERSAND "ll", NULL, NULL)
	MenuLabel(AP_MENU_ID_EDIT_FIND,		AMPERSAND "Find", NULL, NULL)
	MenuLabel(AP_MENU_ID_EDIT_REPLACE,	"R" AMPERSAND "eplace", NULL, NULL)

	MenuLabel(AP_MENU_ID_FORMAT,		"F" AMPERSAND "ormat", NULL, NULL)
	MenuLabel(AP_MENU_ID_FMT_FONT,		AMPERSAND "Font", NULL, NULL)
	MenuLabel(AP_MENU_ID_FMT_PARAGRAPH,	AMPERSAND "Paragraph", NULL, NULL)
	MenuLabel(AP_MENU_ID_FMT_TABS,		AMPERSAND "Tabs", NULL, NULL)
	MenuLabel(AP_MENU_ID_FMT_BOLD,		AMPERSAND "Bold", NULL, NULL)
	MenuLabel(AP_MENU_ID_FMT_ITALIC,	AMPERSAND "Italic", NULL, NULL)
	MenuLabel(AP_MENU_ID_FMT_UNDERLINE,	AMPERSAND "Underline", NULL, NULL)
	MenuLabel(AP_MENU_ID_FMT_STRIKE,	"Stri" AMPERSAND "ke", NULL, NULL)

	MenuLabel(AP_MENU_ID_ALIGN,			AMPERSAND "Align", NULL, NULL)
	MenuLabel(AP_MENU_ID_ALIGN_LEFT,	AMPERSAND "Left", NULL, NULL)
	MenuLabel(AP_MENU_ID_ALIGN_CENTER,	AMPERSAND "Center", NULL, NULL)
	MenuLabel(AP_MENU_ID_ALIGN_RIGHT,	AMPERSAND "Right", NULL, NULL)
	MenuLabel(AP_MENU_ID_ALIGN_JUSTIFY,	AMPERSAND "Justify", NULL, NULL)

	MenuLabel(AP_MENU_ID_HELP,			AMPERSAND "Help", NULL, NULL)
	MenuLabel(AP_MENU_ID_HELP_READSRC,	"Read the source", NULL, NULL)
	MenuLabel(AP_MENU_ID_HELP_FIXBUGS,	"Fix bugs", NULL, NULL)

	// ... add others here ...

	MenuLabel(AP_MENU_ID__BOGUS2__,		NULL,NULL,NULL)

EndSet()
