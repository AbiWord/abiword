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
#include "ev_Toolbar_Actions.h"
#include "xap_Toolbar_ActionSet.h"
#include "ap_Toolbar_Id.h"
#include "ap_Toolbar_Functions.h"
#include "xav_Listener.h"

/*****************************************************************/

EV_Toolbar_ActionSet * AP_CreateToolbarActionSet(void)
{
	// This should only be called once by the application.
	// Everyone should share the set we create.

	EV_Toolbar_ActionSet * pActionSet = new EV_Toolbar_ActionSet(AP_TOOLBAR_ID__BOGUS1__,
																 AP_TOOLBAR_ID__BOGUS2__);
	UT_ASSERT(pActionSet);

	// The following is a list of all toolbar id's that we define,
	// the actions that they should be bound to, and various
	// other small details.  This creates the ActionSet of all
	// possible toolbar actions.  Order here is not significant and
	// does not necessarily correspond to any actual toolbar.
	// Elsewhere we define one or more ToolbarLayouts using these
	// verbs....
	//
	// type         defines the kind of button or thing that should
	//              be created on the toolbar.
	//
	// szMethodName is the name of a "call-by-name" EditMethod that we will
	//              call when the toolbar item is selected.  if it is null, the
	//              toolbar item doesn't do anything (we set it null for spacers).
	//
	// mask         defines the mask-of-interest.  This describes what type of
	//              document changes that the item reflects (ie. dirty-state vs
	//              font style at the insertion point).  this allows us to short
	//              cut toolbar refreshes.
	//
	// pfnGetState  defines a function to be called to compute the state of
	//              the toolbar widget;  whether enabled/disabled,
	//              grayed/ungrayed, and for text or combo objects, the value
	//              of string.
	
#define _s(id,type,szMethodName,maskOfInterest,pfnGetState)		\
	pActionSet->setAction(id,type,szMethodName,maskOfInterest,pfnGetState)

	//( __id__,          			type,					szMethodName,	mask,				pfn);
	
	_s(AP_TOOLBAR_ID__BOGUS1__,		EV_TBIT_BOGUS,			NULL,			0,					NULL);

	_s(AP_TOOLBAR_ID_FILE_NEW,		EV_TBIT_PushButton,		"fileNew",		AV_CHG_NONE,		NULL);
	_s(AP_TOOLBAR_ID_FILE_OPEN,		EV_TBIT_PushButton,		"fileOpen",		AV_CHG_NONE,		NULL);
	_s(AP_TOOLBAR_ID_FILE_SAVE,		EV_TBIT_PushButton,		"fileSave",		AV_CHG_NONE,		NULL);
	_s(AP_TOOLBAR_ID_FILE_SAVEAS,	EV_TBIT_PushButton,		"fileSaveAs",	AV_CHG_NONE,		NULL);
	_s(AP_TOOLBAR_ID_FILE_PRINT,	EV_TBIT_PushButton,		"printTB",		AV_CHG_NONE,		NULL);

	_s(AP_TOOLBAR_ID_EDIT_UNDO,		EV_TBIT_PushButton,		"undo",			AV_CHG_DO,			ap_ToolbarGetState_Changes);
	_s(AP_TOOLBAR_ID_EDIT_REDO,		EV_TBIT_PushButton,		"redo",			AV_CHG_DO,			ap_ToolbarGetState_Changes);
	_s(AP_TOOLBAR_ID_EDIT_CUT,		EV_TBIT_PushButton,		"cut",			AV_CHG_EMPTYSEL,	ap_ToolbarGetState_Selection);
	_s(AP_TOOLBAR_ID_EDIT_COPY,		EV_TBIT_PushButton,		"copy",			AV_CHG_EMPTYSEL,	ap_ToolbarGetState_Selection);
	_s(AP_TOOLBAR_ID_EDIT_PASTE,	EV_TBIT_PushButton,		"paste",		AV_CHG_CLIPBOARD,	ap_ToolbarGetState_Clipboard);

	_s(AP_TOOLBAR_ID_FMT_FONT,		EV_TBIT_ComboBox,		"fontFamily",	AV_CHG_FMTCHAR,		ap_ToolbarGetState_CharFmt);
	_s(AP_TOOLBAR_ID_FMT_SIZE,		EV_TBIT_ComboBox,		"fontSize",		AV_CHG_FMTCHAR,		ap_ToolbarGetState_CharFmt);
	_s(AP_TOOLBAR_ID_FMT_BOLD,		EV_TBIT_ToggleButton,	"toggleBold",	AV_CHG_FMTCHAR,		ap_ToolbarGetState_CharFmt);
	_s(AP_TOOLBAR_ID_FMT_ITALIC,	EV_TBIT_ToggleButton,	"toggleItalic",	AV_CHG_FMTCHAR,		ap_ToolbarGetState_CharFmt);
	_s(AP_TOOLBAR_ID_FMT_UNDERLINE,	EV_TBIT_ToggleButton,	"toggleUline",	AV_CHG_FMTCHAR,		ap_ToolbarGetState_CharFmt);
	_s(AP_TOOLBAR_ID_FMT_STRIKE,	EV_TBIT_ToggleButton,	"toggleStrike",	AV_CHG_FMTCHAR,		ap_ToolbarGetState_CharFmt);

	_s(AP_TOOLBAR_ID_ALIGN_LEFT,	EV_TBIT_ToggleButton,	"alignLeft",	AV_CHG_FMTBLOCK,	ap_ToolbarGetState_BlockFmt);
	_s(AP_TOOLBAR_ID_ALIGN_CENTER,	EV_TBIT_ToggleButton,	"alignCenter",	AV_CHG_FMTBLOCK,	ap_ToolbarGetState_BlockFmt);
	_s(AP_TOOLBAR_ID_ALIGN_RIGHT,	EV_TBIT_ToggleButton,	"alignRight",	AV_CHG_FMTBLOCK,	ap_ToolbarGetState_BlockFmt);
	_s(AP_TOOLBAR_ID_ALIGN_JUSTIFY,	EV_TBIT_ToggleButton,	"alignJustify",	AV_CHG_FMTBLOCK,	ap_ToolbarGetState_BlockFmt);

	_s(AP_TOOLBAR_ID_PARA_0BEFORE,	EV_TBIT_ToggleButton,	"paraBefore0",	AV_CHG_FMTBLOCK,	ap_ToolbarGetState_BlockFmt);
	_s(AP_TOOLBAR_ID_PARA_12BEFORE,	EV_TBIT_ToggleButton,	"paraBefore12",	AV_CHG_FMTBLOCK,	ap_ToolbarGetState_BlockFmt);

	_s(AP_TOOLBAR_ID_SINGLE_SPACE,	EV_TBIT_ToggleButton,	"singleSpace",	AV_CHG_FMTBLOCK,	ap_ToolbarGetState_BlockFmt);
	_s(AP_TOOLBAR_ID_MIDDLE_SPACE,	EV_TBIT_ToggleButton,	"middleSpace",	AV_CHG_FMTBLOCK,	ap_ToolbarGetState_BlockFmt);
	_s(AP_TOOLBAR_ID_DOUBLE_SPACE,	EV_TBIT_ToggleButton,	"doubleSpace",	AV_CHG_FMTBLOCK,	ap_ToolbarGetState_BlockFmt);

	// ... add others here ...
	
	_s(AP_TOOLBAR_ID__BOGUS2__,		EV_TBIT_BOGUS,			NULL,			0,					NULL);

#undef _s
	
	return pActionSet;
}
