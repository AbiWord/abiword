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


#include "ut_types.h"
#include "ut_assert.h"
#include "ev_Menu_Actions.h"
#include "ap_Menu_ActionSet.h"
#include "ap_Menu_Functions.h"
#include "ap_Menu_Id.h"
	
EV_Menu_ActionSet * AP_CreateMenuActionSet(void)
{
	// This should only be called once by the application.
	// Everyone should share the set we create.

	EV_Menu_ActionSet * pActionSet = new EV_Menu_ActionSet(AP_MENU_ID__BOGUS1__,
														   AP_MENU_ID__BOGUS2__);
	UT_ASSERT(pActionSet);

	// The following is a list of all menu id's that we define,
	// the actions that they should be bound to, and various
	// other small details.  This creates the ActionSet of all
	// possible menu actions.  Order here is not significant and
	// does not necessarily correspond to any actual menu.
	// Elsewhere we define one or more MenuLayouts using these
	// verbs....
	//
	// szMethodName is the name of a "call-by-name" EditMethod that we will
	//              call when the menu item is selected.  if it is null, the
	//              menu item doesn't do anything (we set it null for separators
	//              and the popup name).
	//
	// fnGetState   is a function pointer to be called to return the enabled/disabled
	//              and/or checked/unchecked state of the menu item.  this will be
	//              called on each item when the menu (bar) is activated (on Win32
	//              this is in response to an WM_INITMENU message).
	//
	// fnGetLabel   is a function pointer to compute a "dynamic menu item name" for
	//              the item.  it returns a string which will be stuffed into the
	//              menu as the label for this item.  the returned label will be
	//              decorated in platform-code to include the usual i-raise-a-dialog "..."
	//              and any other platform-specific decoration.  if this function returns
	//              a null string, we temporarily hide/remove this item from the layout.
	//              (this feature is used by the window list manager.)
	
#define _s(id,bHoldsSubMenu,bRaisesDialog,bCheckable,szMethodName,pfnGetState,pfnGetLabel)	\
	pActionSet->setAction(id,bHoldsSubMenu,bRaisesDialog,bCheckable,szMethodName,pfnGetState,pfnGetLabel)

	//( __id__,          bSub,bDlg,bCheck,  szMethodName,       fnGetState,         fnGetLabel)
	
	_s(AP_MENU_ID__BOGUS1__,		0,0,0,	NULL,				NULL,					NULL);

	_s(AP_MENU_ID_FILE,				1,0,0,	NULL,				NULL,					NULL);
	_s(AP_MENU_ID_FILE_NEW,			0,0,0,	"fileNew",			NULL,					NULL);
	_s(AP_MENU_ID_FILE_OPEN,		0,1,0,	"fileOpen",			NULL,					NULL);
	_s(AP_MENU_ID_FILE_SAVE,		0,0,0,	"fileSave",			NULL,					NULL);
	_s(AP_MENU_ID_FILE_SAVEAS,		0,1,0,	"fileSaveAs",		NULL,					NULL);
	_s(AP_MENU_ID_FILE_CLOSE,		0,0,0,	"closeWindow",		NULL,					NULL);
	_s(AP_MENU_ID_FILE_PAGESETUP,	0,1,0,	NULL,				NULL,					NULL);
	_s(AP_MENU_ID_FILE_PRINT,		0,1,0,	"print",			NULL,					NULL);
	_s(AP_MENU_ID_FILE_EXIT,		0,0,0,	"querySaveAndExit",	NULL,					NULL);

	_s(AP_MENU_ID_EDIT,				1,0,0,	NULL,				NULL,					NULL);
	_s(AP_MENU_ID_EDIT_UNDO,		0,0,0,	"undo",				ap_GetState_Changes,	NULL);
	_s(AP_MENU_ID_EDIT_REDO,		0,0,0,	"redo",				ap_GetState_Changes,	NULL);
	_s(AP_MENU_ID_EDIT_CUT,			0,0,0,	"cut",				ap_GetState_Selection,	NULL);
	_s(AP_MENU_ID_EDIT_COPY,		0,0,0,	"copy",				ap_GetState_Selection,	NULL);
	_s(AP_MENU_ID_EDIT_PASTE,		0,0,0,	"paste",			ap_GetState_Clipboard,	NULL);
	_s(AP_MENU_ID_EDIT_CLEAR,		0,0,0,	"delRight",			NULL,					NULL);
	_s(AP_MENU_ID_EDIT_SELECTALL,	0,0,0,	"selectAll",		NULL,					NULL);
	_s(AP_MENU_ID_EDIT_FIND,		0,1,0,	"find",				NULL,					NULL);
	_s(AP_MENU_ID_EDIT_REPLACE,		0,1,0,	"replace",			NULL,					NULL);

	_s(AP_MENU_ID_FORMAT,			1,0,0,	NULL,				NULL,					NULL);
	_s(AP_MENU_ID_FMT_FONT,			0,1,0,	"dlgFont",			NULL,					NULL);
	_s(AP_MENU_ID_FMT_PARAGRAPH,	0,1,0,	NULL,				NULL,					NULL);
	_s(AP_MENU_ID_FMT_TABS,			0,1,0,	NULL,				NULL,					NULL);
	_s(AP_MENU_ID_FMT_BOLD,			0,0,1,	"toggleBold",		ap_GetState_CharFmt,	NULL);
	_s(AP_MENU_ID_FMT_ITALIC,		0,0,1,	"toggleItalic",		ap_GetState_CharFmt,	NULL);
	_s(AP_MENU_ID_FMT_UNDERLINE,	0,0,1,	"toggleUline",		ap_GetState_CharFmt,	NULL);
	_s(AP_MENU_ID_FMT_STRIKE,		0,0,1,	"toggleStrike",		ap_GetState_CharFmt,	NULL);

	_s(AP_MENU_ID_ALIGN,			1,0,0,	NULL,				NULL,					NULL);
	_s(AP_MENU_ID_ALIGN_LEFT,		0,0,1,	"alignLeft",		ap_GetState_BlockFmt,	NULL);
	_s(AP_MENU_ID_ALIGN_CENTER,		0,0,1,	"alignCenter",		ap_GetState_BlockFmt,	NULL);
	_s(AP_MENU_ID_ALIGN_RIGHT,		0,0,1,	"alignRight",		ap_GetState_BlockFmt,	NULL);
	_s(AP_MENU_ID_ALIGN_JUSTIFY,	0,0,1,	"alignJustify",		ap_GetState_BlockFmt,	NULL);

	_s(AP_MENU_ID_WINDOW,			1,0,0,	NULL,				NULL,					NULL);
	_s(AP_MENU_ID_WINDOW_NEW,		0,0,0,	"newWindow",		NULL,					NULL);
	_s(AP_MENU_ID_WINDOW_1,			0,0,0,	"activateWindow_1",	ap_GetState_Window,		ap_GetLabel_Window);
	_s(AP_MENU_ID_WINDOW_2,			0,0,0,	"activateWindow_2",	ap_GetState_Window,		ap_GetLabel_Window);
	_s(AP_MENU_ID_WINDOW_3,			0,0,0,	"activateWindow_3",	ap_GetState_Window,		ap_GetLabel_Window);
	_s(AP_MENU_ID_WINDOW_4,			0,0,0,	"activateWindow_4",	ap_GetState_Window,		ap_GetLabel_Window);
	_s(AP_MENU_ID_WINDOW_5,			0,0,0,	"activateWindow_5",	ap_GetState_Window,		ap_GetLabel_Window);
	_s(AP_MENU_ID_WINDOW_6,			0,0,0,	"activateWindow_6",	ap_GetState_Window,		ap_GetLabel_Window);
	_s(AP_MENU_ID_WINDOW_7,			0,0,0,	"activateWindow_7",	ap_GetState_Window,		ap_GetLabel_Window);
	_s(AP_MENU_ID_WINDOW_8,			0,0,0,	"activateWindow_8",	ap_GetState_Window,		ap_GetLabel_Window);
	_s(AP_MENU_ID_WINDOW_9,			0,0,0,	"activateWindow_9",	ap_GetState_Window,		ap_GetLabel_Window);
	_s(AP_MENU_ID_WINDOW_MORE,		0,1,0,	"moreWindowsDlg",	NULL,					ap_GetLabel_WindowMore);
	
	_s(AP_MENU_ID_HELP,				1,0,0,	NULL,				NULL,					NULL);
	_s(AP_MENU_ID_HELP_READSRC,		0,0,0,	NULL,				NULL,					NULL);
	_s(AP_MENU_ID_HELP_FIXBUGS,		0,0,0,	NULL,				NULL,					NULL);

	// ... add others here ...
	
	_s(AP_MENU_ID__BOGUS2__,		0,0,0,	NULL,				NULL,					NULL);

#undef _s
	
	return pActionSet;
}
