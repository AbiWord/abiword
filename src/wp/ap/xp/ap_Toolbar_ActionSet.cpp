/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "ap_Features.h"

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
	UT_return_val_if_fail (pActionSet, NULL);

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
	_s(AP_TOOLBAR_ID_FILE_SAVE,		EV_TBIT_PushButton,		"fileSave",		AV_CHG_ALL,		ap_ToolbarGetState_Changes);
	_s(AP_TOOLBAR_ID_FILE_SAVEAS,	EV_TBIT_PushButton,		"fileSaveAs",	AV_CHG_NONE,		NULL);
#if TOOLKIT_GTK
	_s(AP_TOOLBAR_ID_FILE_PRINT,	EV_TBIT_PushButton,		"cairoPrint",		AV_CHG_NONE,		NULL);
	_s(AP_TOOLBAR_ID_FILE_PRINT_PREVIEW, EV_TBIT_PushButton,	"cairoPrintPreview", AV_CHG_NONE,	NULL);
#else
	_s(AP_TOOLBAR_ID_FILE_PRINT,	EV_TBIT_PushButton,		"printTB",		AV_CHG_NONE,		NULL);
	_s(AP_TOOLBAR_ID_FILE_PRINT_PREVIEW, EV_TBIT_PushButton,	"printPreview", AV_CHG_NONE,	NULL);
#endif
	// AV_CHG_ALL doesn't seem right here. TODO!
#ifdef ENABLE_SPELL
	_s(AP_TOOLBAR_ID_SPELLCHECK,	EV_TBIT_PushButton,		"dlgSpell",		AV_CHG_ALL,		ap_ToolbarGetState_Spelling);
#endif
	_s(AP_TOOLBAR_ID_IMG,			EV_TBIT_PushButton,		"fileInsertGraphic", AV_CHG_NONE,	NULL);
	_s(AP_TOOLBAR_ID_HELP,			EV_TBIT_PushButton,		"helpContents",	AV_CHG_NONE,		NULL);
	// This changes as a document property only, is there a less frequent action than AV_CHG_FRAMEDATA?
	_s(AP_TOOLBAR_ID_COLOR_FORE,	EV_TBIT_ColorFore,		"colorForeTB",	AV_CHG_FRAMEDATA,	ap_ToolbarGetState_StylesLocked);
	_s(AP_TOOLBAR_ID_COLOR_BACK,	EV_TBIT_ColorBack,		"colorBackTB",	AV_CHG_FRAMEDATA,	ap_ToolbarGetState_StylesLocked);
	_s(AP_TOOLBAR_ID_EDIT_UNDO,		EV_TBIT_PushButton,		"undo",			AV_CHG_ALL,			ap_ToolbarGetState_Changes);
	_s(AP_TOOLBAR_ID_EDIT_REDO,		EV_TBIT_PushButton,		"redo",			AV_CHG_ALL,			ap_ToolbarGetState_Changes);
	_s(AP_TOOLBAR_ID_EDIT_CUT,		EV_TBIT_PushButton,		"cut",			AV_CHG_ALL,	ap_ToolbarGetState_Selection);
	_s(AP_TOOLBAR_ID_EDIT_COPY,		EV_TBIT_PushButton,		"copy",			AV_CHG_ALL,	ap_ToolbarGetState_Selection);
	_s(AP_TOOLBAR_ID_EDIT_PASTE,	EV_TBIT_PushButton,		"paste",		AV_CHG_CLIPBOARD,	ap_ToolbarGetState_Clipboard);

	_s(AP_TOOLBAR_ID_EDIT_HEADER,	EV_TBIT_PushButton,		"editHeader",		AV_CHG_NONE,	NULL);
	_s(AP_TOOLBAR_ID_EDIT_FOOTER,	EV_TBIT_PushButton,		"editFooter",		AV_CHG_NONE,	NULL);
	_s(AP_TOOLBAR_ID_EDIT_REMOVEHEADER,	EV_TBIT_PushButton,		"removeHeader",		AV_CHG_MOTION,	ap_ToolbarGetState_HdrFtr);
	_s(AP_TOOLBAR_ID_EDIT_REMOVEFOOTER,	EV_TBIT_PushButton,		"removeFooter",		AV_CHG_MOTION,	ap_ToolbarGetState_HdrFtr);

	_s(AP_TOOLBAR_ID_FMT_STYLE,		EV_TBIT_ComboBox,		"style",		AV_CHG_FMTSTYLE | AV_CHG_MOTION,	ap_ToolbarGetState_Style);
	_s(AP_TOOLBAR_ID_FMT_FONT,		EV_TBIT_ComboBox,		"fontFamily",	AV_CHG_FMTCHAR | AV_CHG_MOTION,		ap_ToolbarGetState_CharFmt);
	
#if XAP_SIMPLE_TOOLBAR
	_s(AP_TOOLBAR_ID_FMT_CHOOSE,	EV_TBIT_PushButton,		"dlgFont",		AV_CHG_NONE,		NULL);	
	_s(AP_TOOLBAR_ID_VIEW_FULL_SCREEN,	EV_TBIT_PushButton,		"viewFullScreen",		AV_CHG_NONE,		NULL);	
#endif	
	
	_s(AP_TOOLBAR_ID_FMT_SIZE,		EV_TBIT_ComboBox,		"fontSize",		AV_CHG_FMTCHAR | AV_CHG_MOTION,		ap_ToolbarGetState_CharFmt);
	_s(AP_TOOLBAR_ID_FMT_BOLD,		EV_TBIT_ToggleButton,	"toggleBold",	AV_CHG_FMTCHAR | AV_CHG_MOTION,		ap_ToolbarGetState_CharFmt);
	_s(AP_TOOLBAR_ID_FMT_ITALIC,	EV_TBIT_ToggleButton,	"toggleItalic",	AV_CHG_FMTCHAR | AV_CHG_MOTION,		ap_ToolbarGetState_CharFmt);
	_s(AP_TOOLBAR_ID_FMT_UNDERLINE,	EV_TBIT_ToggleButton,	"toggleUline",	AV_CHG_FMTCHAR | AV_CHG_MOTION,		ap_ToolbarGetState_CharFmt);
	_s(AP_TOOLBAR_ID_FMT_OVERLINE,	EV_TBIT_ToggleButton,	"toggleOline",	AV_CHG_FMTCHAR | AV_CHG_MOTION,		ap_ToolbarGetState_CharFmt);
	_s(AP_TOOLBAR_ID_FMT_STRIKE,	EV_TBIT_ToggleButton,	"toggleStrike",	AV_CHG_FMTCHAR | AV_CHG_MOTION,		ap_ToolbarGetState_CharFmt);
	_s(AP_TOOLBAR_ID_FMT_TOPLINE,	EV_TBIT_ToggleButton,	"toggleTopline",	AV_CHG_FMTCHAR | AV_CHG_MOTION,		ap_ToolbarGetState_CharFmt);
	_s(AP_TOOLBAR_ID_FMT_BOTTOMLINE,	EV_TBIT_ToggleButton,	"toggleBottomline",	AV_CHG_FMTCHAR | AV_CHG_MOTION,		ap_ToolbarGetState_CharFmt);

	_s(AP_TOOLBAR_ID_INDENT,		EV_TBIT_PushButton,		"toggleIndent",		AV_CHG_FMTBLOCK | AV_CHG_MOTION,		ap_ToolbarGetState_Indents);
	_s(AP_TOOLBAR_ID_UNINDENT,		EV_TBIT_PushButton,		"toggleUnIndent",	AV_CHG_FMTBLOCK | AV_CHG_MOTION,	ap_ToolbarGetState_Indents);

	_s(AP_TOOLBAR_ID_FMT_SUPERSCRIPT,	EV_TBIT_ToggleButton,	"toggleSuper",	AV_CHG_FMTCHAR | AV_CHG_MOTION,	ap_ToolbarGetState_CharFmt);
	_s(AP_TOOLBAR_ID_FMT_SUBSCRIPT,		EV_TBIT_ToggleButton,	"toggleSub",	AV_CHG_FMTCHAR | AV_CHG_MOTION,	ap_ToolbarGetState_CharFmt);
	_s(AP_TOOLBAR_ID_INSERT_SYMBOL,		EV_TBIT_PushButton,	"insSymbol",	AV_CHG_NONE,		NULL);

	_s(AP_TOOLBAR_ID_ALIGN_LEFT,	EV_TBIT_GroupButton,	"alignLeft",	AV_CHG_FMTBLOCK | AV_CHG_MOTION,	ap_ToolbarGetState_BlockFmt);
	_s(AP_TOOLBAR_ID_ALIGN_CENTER,	EV_TBIT_GroupButton,	"alignCenter",	AV_CHG_FMTBLOCK | AV_CHG_MOTION,	ap_ToolbarGetState_BlockFmt);
	_s(AP_TOOLBAR_ID_ALIGN_RIGHT,	EV_TBIT_GroupButton,	"alignRight",	AV_CHG_FMTBLOCK | AV_CHG_MOTION,	ap_ToolbarGetState_BlockFmt);
	_s(AP_TOOLBAR_ID_ALIGN_JUSTIFY,	EV_TBIT_GroupButton,	"alignJustify",	AV_CHG_FMTBLOCK | AV_CHG_MOTION,	ap_ToolbarGetState_BlockFmt);

	_s(AP_TOOLBAR_ID_PARA_0BEFORE,	EV_TBIT_GroupButton,	"paraBefore0",	AV_CHG_FMTBLOCK | AV_CHG_MOTION,	ap_ToolbarGetState_BlockFmt);
	_s(AP_TOOLBAR_ID_PARA_12BEFORE,	EV_TBIT_GroupButton,	"paraBefore12",	AV_CHG_FMTBLOCK | AV_CHG_MOTION,	ap_ToolbarGetState_BlockFmt);

	_s(AP_TOOLBAR_ID_SINGLE_SPACE,	EV_TBIT_GroupButton,	"singleSpace",	AV_CHG_FMTBLOCK | AV_CHG_MOTION,	ap_ToolbarGetState_BlockFmt);
	_s(AP_TOOLBAR_ID_MIDDLE_SPACE,	EV_TBIT_GroupButton,	"middleSpace",	AV_CHG_FMTBLOCK | AV_CHG_MOTION,	ap_ToolbarGetState_BlockFmt);
	_s(AP_TOOLBAR_ID_DOUBLE_SPACE,	EV_TBIT_GroupButton,	"doubleSpace",	AV_CHG_FMTBLOCK | AV_CHG_MOTION,	ap_ToolbarGetState_BlockFmt);

	_s(AP_TOOLBAR_ID_1COLUMN,		EV_TBIT_GroupButton,	"sectColumns1",	AV_CHG_ALL,	ap_ToolbarGetState_SectionFmt);
	_s(AP_TOOLBAR_ID_2COLUMN,		EV_TBIT_GroupButton,	"sectColumns2",	AV_CHG_ALL,	ap_ToolbarGetState_SectionFmt);
	_s(AP_TOOLBAR_ID_3COLUMN,		EV_TBIT_GroupButton,	"sectColumns3",	AV_CHG_ALL,	ap_ToolbarGetState_SectionFmt);

	_s(AP_TOOLBAR_ID_VIEW_SHOWPARA,	EV_TBIT_ToggleButton,	"viewPara",		AV_CHG_ALL,	ap_ToolbarGetState_View);

	// AV_CHG_WINDOWSIZE, below, doesn't seem right.  TODO
	_s(AP_TOOLBAR_ID_ZOOM,				EV_TBIT_ComboBox,		"zoom",		AV_CHG_WINDOWSIZE,	ap_ToolbarGetState_Zoom);

	// AV_CHG_ALL, below, doesn't seem right.  TODO
	_s(AP_TOOLBAR_ID_LISTS_BULLETS,		EV_TBIT_ToggleButton,	"doBullets",	AV_CHG_ALL,		ap_ToolbarGetState_Bullets	);
	_s(AP_TOOLBAR_ID_LISTS_NUMBERS,		EV_TBIT_ToggleButton,	"doNumbers",	AV_CHG_ALL,		ap_ToolbarGetState_Numbers	);

	_s(AP_TOOLBAR_ID_FMT_HYPERLINK, EV_TBIT_PushButton, "insertHyperlink", AV_CHG_ALL,  ap_ToolbarGetState_HyperlinkOK);
	_s(AP_TOOLBAR_ID_FMT_BOOKMARK, EV_TBIT_PushButton, "insertBookmark", AV_CHG_ALL, ap_ToolbarGetState_BookmarkOK);

	_s(AP_TOOLBAR_ID_SCRIPT_PLAY,	EV_TBIT_PushButton,	"scriptPlay",		AV_CHG_ALL,			ap_ToolbarGetState_ScriptsActive);
	_s(AP_TOOLBAR_ID_FMTPAINTER, EV_TBIT_PushButton, "formatPainter",
	   AV_CHG_ALL, ap_ToolbarGetState_Clipboard);

	// ... add others here ...
	_s(AP_TOOLBAR_ID_FMT_DIR_OVERRIDE_LTR,	EV_TBIT_ToggleButton,	"toggleDirOverrideLTR",	AV_CHG_FMTCHAR | AV_CHG_DIRECTIONMODE | AV_CHG_MOTION,		ap_ToolbarGetState_CharFmt);
	_s(AP_TOOLBAR_ID_FMT_DIR_OVERRIDE_RTL,	EV_TBIT_ToggleButton,	"toggleDirOverrideRTL",	AV_CHG_FMTCHAR | AV_CHG_DIRECTIONMODE | AV_CHG_MOTION,		ap_ToolbarGetState_CharFmt);
	_s(AP_TOOLBAR_ID_FMT_DOM_DIRECTION,	EV_TBIT_ToggleButton,	"toggleDomDirection",	AV_CHG_FMTBLOCK | AV_CHG_FMTSECTION | AV_CHG_MOTION,		ap_ToolbarGetState_BlockFmt);


	_s(AP_TOOLBAR_ID_INSERT_TABLE, EV_TBIT_PushButton, "insertTable", AV_CHG_ALL, ap_ToolbarGetState_TableOK);
	_s(AP_TOOLBAR_ID_ADD_ROW, EV_TBIT_PushButton, "insertRowsAfter", AV_CHG_ALL, ap_ToolbarGetState_Table);
	_s(AP_TOOLBAR_ID_ADD_COLUMN, EV_TBIT_PushButton, "insertColsAfter", AV_CHG_ALL, ap_ToolbarGetState_Table);
	_s(AP_TOOLBAR_ID_DELETE_ROW, EV_TBIT_PushButton, "deleteRows", AV_CHG_ALL, ap_ToolbarGetState_Table);
	_s(AP_TOOLBAR_ID_DELETE_COLUMN, EV_TBIT_PushButton, "deleteColumns", AV_CHG_ALL, ap_ToolbarGetState_Table);
	_s(AP_TOOLBAR_ID_MERGE_CELLS, EV_TBIT_PushButton, "mergeCells", AV_CHG_ALL, ap_ToolbarGetState_Table);
	_s(AP_TOOLBAR_ID_SPLIT_CELLS, EV_TBIT_PushButton, "splitCells", AV_CHG_ALL, ap_ToolbarGetState_TableMerged);

	_s(AP_TOOLBAR_ID_MERGELEFT,  EV_TBIT_PushButton, "mergeCells", AV_CHG_ALL, ap_ToolbarGetState_Table); // FIXME
	_s(AP_TOOLBAR_ID_MERGERIGHT, EV_TBIT_PushButton, "mergeCells", AV_CHG_ALL, ap_ToolbarGetState_Table); // FIXME
	_s(AP_TOOLBAR_ID_MERGEABOVE, EV_TBIT_PushButton, "mergeCells", AV_CHG_ALL, ap_ToolbarGetState_Table); // FIXME
	_s(AP_TOOLBAR_ID_MERGEBELOW, EV_TBIT_PushButton, "mergeCells", AV_CHG_ALL, ap_ToolbarGetState_Table); // FIXME

#ifdef ENABLE_MENUBUTTON
	_s(AP_TOOLBAR_ID_MENU, EV_TBIT_MenuButton, NULL, AV_CHG_NONE, NULL);
#endif
	_s(AP_TOOLBAR_ID__BOGUS2__,		EV_TBIT_BOGUS,			NULL,			0,					NULL);
#undef _s

	return pActionSet;
}


