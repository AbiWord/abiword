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
#include "ut_debugmsg.h"
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
	_s(AP_TOOLBAR_ID_FILE_PRINT_PREVIEW, EV_TBIT_PushButton, "printPreview", AV_CHG_NONE, NULL);
	_s(AP_TOOLBAR_ID_SPELLCHECK, EV_TBIT_PushButton, "dlgSpell", AV_CHG_NONE, NULL);
	_s(AP_TOOLBAR_ID_COLOR_FORE, EV_TBIT_ColorFore, "colorForeTB", AV_CHG_NONE, NULL);
	_s(AP_TOOLBAR_ID_COLOR_BACK, EV_TBIT_ColorBack, "colorBackTB", AV_CHG_NONE, NULL);
	_s(AP_TOOLBAR_ID_EDIT_UNDO,		EV_TBIT_PushButton,		"undo",			AV_CHG_DO,			ap_ToolbarGetState_Changes);
	_s(AP_TOOLBAR_ID_EDIT_REDO,		EV_TBIT_PushButton,		"redo",			AV_CHG_DO,			ap_ToolbarGetState_Changes);
	_s(AP_TOOLBAR_ID_EDIT_CUT,		EV_TBIT_PushButton,		"cut",			AV_CHG_EMPTYSEL,	ap_ToolbarGetState_Selection);
	_s(AP_TOOLBAR_ID_EDIT_COPY,		EV_TBIT_PushButton,		"copy",			AV_CHG_EMPTYSEL,	ap_ToolbarGetState_Selection);
	_s(AP_TOOLBAR_ID_EDIT_PASTE,	EV_TBIT_PushButton,		"paste",		AV_CHG_CLIPBOARD,	ap_ToolbarGetState_Clipboard);

	_s(AP_TOOLBAR_ID_FMT_STYLE,		EV_TBIT_ComboBox,		"style",		AV_CHG_FMTSTYLE,	ap_ToolbarGetState_Style);
	_s(AP_TOOLBAR_ID_FMT_FONT,		EV_TBIT_ComboBox,		"fontFamily",	AV_CHG_FMTCHAR,		ap_ToolbarGetState_CharFmt);
	_s(AP_TOOLBAR_ID_FMT_SIZE,		EV_TBIT_ComboBox,		"fontSize",		AV_CHG_FMTCHAR,		ap_ToolbarGetState_CharFmt);
	_s(AP_TOOLBAR_ID_FMT_BOLD,		EV_TBIT_ToggleButton,	"toggleBold",	AV_CHG_FMTCHAR,		ap_ToolbarGetState_CharFmt);
	_s(AP_TOOLBAR_ID_FMT_ITALIC,	EV_TBIT_ToggleButton,	"toggleItalic",	AV_CHG_FMTCHAR,		ap_ToolbarGetState_CharFmt);
	_s(AP_TOOLBAR_ID_FMT_UNDERLINE,	EV_TBIT_ToggleButton,	"toggleUline",	AV_CHG_FMTCHAR,		ap_ToolbarGetState_CharFmt);
	_s(AP_TOOLBAR_ID_FMT_OVERLINE,	EV_TBIT_ToggleButton,	"toggleOline",	AV_CHG_FMTCHAR,		ap_ToolbarGetState_CharFmt);
	_s(AP_TOOLBAR_ID_FMT_STRIKE,	EV_TBIT_ToggleButton,	"toggleStrike",	AV_CHG_FMTCHAR,		ap_ToolbarGetState_CharFmt);

	_s(AP_TOOLBAR_ID_INDENT, EV_TBIT_PushButton, "toggleIndent", AV_CHG_NONE,	NULL);
	_s(AP_TOOLBAR_ID_UNINDENT, EV_TBIT_PushButton, "toggleUnIndent", AV_CHG_NONE,		NULL);

	_s(AP_TOOLBAR_ID_FMT_SUPERSCRIPT,	EV_TBIT_ToggleButton,	"toggleSuper",	AV_CHG_FMTCHAR,		ap_ToolbarGetState_CharFmt);
	_s(AP_TOOLBAR_ID_FMT_SUBSCRIPT,		EV_TBIT_ToggleButton,	"toggleSub",	AV_CHG_FMTCHAR,		ap_ToolbarGetState_CharFmt);	
	_s(AP_TOOLBAR_ID_INSERT_SYMBOL,		EV_TBIT_PushButton,	"insSymbol",	AV_CHG_NONE,		NULL);

	_s(AP_TOOLBAR_ID_ALIGN_LEFT,	EV_TBIT_GroupButton,	"alignLeft",	AV_CHG_FMTBLOCK,	ap_ToolbarGetState_BlockFmt);
	_s(AP_TOOLBAR_ID_ALIGN_CENTER,	EV_TBIT_GroupButton,	"alignCenter",	AV_CHG_FMTBLOCK,	ap_ToolbarGetState_BlockFmt);
	_s(AP_TOOLBAR_ID_ALIGN_RIGHT,	EV_TBIT_GroupButton,	"alignRight",	AV_CHG_FMTBLOCK,	ap_ToolbarGetState_BlockFmt);
	_s(AP_TOOLBAR_ID_ALIGN_JUSTIFY,	EV_TBIT_GroupButton,	"alignJustify",	AV_CHG_FMTBLOCK,	ap_ToolbarGetState_BlockFmt);

	_s(AP_TOOLBAR_ID_PARA_0BEFORE,	EV_TBIT_GroupButton,	"paraBefore0",	AV_CHG_FMTBLOCK,	ap_ToolbarGetState_BlockFmt);
	_s(AP_TOOLBAR_ID_PARA_12BEFORE,	EV_TBIT_GroupButton,	"paraBefore12",	AV_CHG_FMTBLOCK,	ap_ToolbarGetState_BlockFmt);

	_s(AP_TOOLBAR_ID_SINGLE_SPACE,	EV_TBIT_GroupButton,	"singleSpace",	AV_CHG_FMTBLOCK,	ap_ToolbarGetState_BlockFmt);
	_s(AP_TOOLBAR_ID_MIDDLE_SPACE,	EV_TBIT_GroupButton,	"middleSpace",	AV_CHG_FMTBLOCK,	ap_ToolbarGetState_BlockFmt);
	_s(AP_TOOLBAR_ID_DOUBLE_SPACE,	EV_TBIT_GroupButton,	"doubleSpace",	AV_CHG_FMTBLOCK,	ap_ToolbarGetState_BlockFmt);

	_s(AP_TOOLBAR_ID_1COLUMN,		EV_TBIT_GroupButton,	"sectColumns1",	AV_CHG_FMTSECTION,	ap_ToolbarGetState_SectionFmt);
	_s(AP_TOOLBAR_ID_2COLUMN,		EV_TBIT_GroupButton,	"sectColumns2",	AV_CHG_FMTSECTION,	ap_ToolbarGetState_SectionFmt);
	_s(AP_TOOLBAR_ID_3COLUMN,		EV_TBIT_GroupButton,	"sectColumns3",	AV_CHG_FMTSECTION,	ap_ToolbarGetState_SectionFmt);

	// AV_CHG_WINDOWSIZE, below, doesn't seem right.  TODO
	
	_s(AP_TOOLBAR_ID_ZOOM,			EV_TBIT_ComboBox,		"zoom",			AV_CHG_WINDOWSIZE,	ap_ToolbarGetState_Zoom);
	_s(AP_TOOLBAR_ID_LISTS_BULLETS,		EV_TBIT_ToggleButton,		"doBullets",		AV_CHG_ALL,	ap_ToolbarGetState_Bullets	);
	_s(AP_TOOLBAR_ID_LISTS_NUMBERS,		EV_TBIT_ToggleButton,		"doNumbers",		AV_CHG_ALL,	ap_ToolbarGetState_Numbers	);
	
	// ... add others here ...
	
	_s(AP_TOOLBAR_ID__BOGUS2__,		EV_TBIT_BOGUS,			NULL,			0,					NULL);

#undef _s
	
	return pActionSet;
}


