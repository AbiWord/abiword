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
#include "xap_Menu_ActionSet.h"
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
	_s(AP_MENU_ID_FILE_NEW,			0,1,0,	"fileNew",			NULL,					NULL);
	_s(AP_MENU_ID_FILE_OPEN,		0,1,0,	"fileOpen",			NULL,					NULL);
	_s(AP_MENU_ID_FILE_SAVE,		0,0,0,	"fileSave",			ap_GetState_Changes,					NULL);
	_s(AP_MENU_ID_FILE_SAVEAS,		0,1,0,	"fileSaveAs",		NULL,					NULL);
	_s(AP_MENU_ID_FILE_SAVEIMAGE,		0,1,0,	"fileSaveImage",		NULL,					NULL);
	_s(AP_MENU_ID_FILE_SAVE_TEMPLATE, 0,1,0, "fileSaveTemplate", NULL, NULL);
	_s(AP_MENU_ID_FILE_IMPORT, 0,1,0, "fileImport", NULL, NULL);
	_s(AP_MENU_ID_FILE_EXPORT, 0,1,0, "fileExport", NULL, NULL);
	_s(AP_MENU_ID_FILE_PROPERTIES, 0,1,0, "dlgMetaData", NULL, NULL);
	_s(AP_MENU_ID_FILE_CLOSE,		0,0,0,	"closeWindow",		NULL,					NULL);
	_s(AP_MENU_ID_FILE_PAGESETUP,	0,1,0,	"pageSetup",		NULL,					NULL);
	_s(AP_MENU_ID_FILE_PRINT,		0,1,0,	"print",			NULL,					NULL);
#ifdef HAVE_GNOME_DIRECT_PRINT
	_s(AP_MENU_ID_FILE_PRINT_DIRECTLY,		0,1,0,	"printDirectly",			NULL,					NULL);
#endif
	_s(AP_MENU_ID_FILE_PRINT_PREVIEW, 0,1,0, "printPreview", NULL, NULL);
	_s(AP_MENU_ID_FILE_RECENT_1,	0,0,0,	"openRecent_1",		NULL,					ap_GetLabel_Recent);
	_s(AP_MENU_ID_FILE_RECENT_2,	0,0,0,	"openRecent_2",		NULL,					ap_GetLabel_Recent);
	_s(AP_MENU_ID_FILE_RECENT_3,	0,0,0,	"openRecent_3",		NULL,					ap_GetLabel_Recent);
	_s(AP_MENU_ID_FILE_RECENT_4,	0,0,0,	"openRecent_4",		NULL,					ap_GetLabel_Recent);
	_s(AP_MENU_ID_FILE_RECENT_5,	0,0,0,	"openRecent_5",		NULL,					ap_GetLabel_Recent);
	_s(AP_MENU_ID_FILE_RECENT_6,	0,0,0,	"openRecent_6",		NULL,					ap_GetLabel_Recent);
	_s(AP_MENU_ID_FILE_RECENT_7,	0,0,0,	"openRecent_7",		NULL,					ap_GetLabel_Recent);
	_s(AP_MENU_ID_FILE_RECENT_8,	0,0,0,	"openRecent_8",		NULL,					ap_GetLabel_Recent);
	_s(AP_MENU_ID_FILE_RECENT_9,	0,0,0,	"openRecent_9",		NULL,					ap_GetLabel_Recent);
	_s(AP_MENU_ID_FILE_REVERT, 0,0,0, "fileRevert", ap_GetState_Changes, NULL);
	_s(AP_MENU_ID_FILE_EXIT,		0,0,0,	"querySaveAndExit",	NULL,					NULL);
	_s(AP_MENU_ID_OPEN_TEMPLATE, 0,1,0, "openTemplate", NULL, NULL);

	_s(AP_MENU_ID_EDIT,				1,0,0,	NULL,				NULL,					NULL);
	_s(AP_MENU_ID_EDIT_UNDO,		0,0,0,	"undo",				ap_GetState_Changes,	NULL);
	_s(AP_MENU_ID_EDIT_REDO,		0,0,0,	"redo",				ap_GetState_Changes,	NULL);
	_s(AP_MENU_ID_EDIT_CUT,			0,0,0,	"cut",				ap_GetState_Selection,	NULL);
	_s(AP_MENU_ID_EDIT_COPY,		0,0,0,	"copy",				ap_GetState_Selection,	NULL);
	_s(AP_MENU_ID_EDIT_PASTE,		0,0,0,	"paste",			ap_GetState_Clipboard,	NULL);
	_s(AP_MENU_ID_EDIT_PASTE_SPECIAL,0,0,0, "pasteSpecial",     ap_GetState_Clipboard,  NULL);
	_s(AP_MENU_ID_EDIT_CLEAR,		0,0,0,	"delRight",			NULL,					NULL);
	_s(AP_MENU_ID_EDIT_SELECTALL,	0,0,0,	"selectAll",		NULL,					NULL);
	_s(AP_MENU_ID_EDIT_FIND,		0,1,0,	"find",				NULL,					NULL);
	_s(AP_MENU_ID_EDIT_REPLACE,		0,1,0,	"replace",			NULL,					NULL);
	_s(AP_MENU_ID_EDIT_GOTO,		0,1,0,	"go",				NULL,					NULL);
	_s(AP_MENU_ID_EDIT_EDITHEADER,		0,0,0,	"editHeader",	ap_GetState_Changes,					NULL);
	_s(AP_MENU_ID_EDIT_EDITFOOTER,		0,0,0,	"editFooter",	ap_GetState_Changes,					NULL);
	_s(AP_MENU_ID_EDIT_REMOVEHEADER,		0,0,0,"removeHeader",ap_GetState_Changes	,					NULL);
	_s(AP_MENU_ID_EDIT_REMOVEFOOTER,		0,0,0,	"removeFooter",ap_GetState_Changes,					NULL);

	_s(AP_MENU_ID_VIEW,				1,0,0,	NULL,				NULL,					NULL);
	_s(AP_MENU_ID_VIEW_NORMAL, 0,0,1, "viewNormalLayout", ap_GetState_View, NULL);
	_s(AP_MENU_ID_VIEW_WEB,    0,0,1, "viewWebLayout", ap_GetState_View, NULL);
	_s(AP_MENU_ID_VIEW_PRINT,  0,0,1, "viewPrintLayout", ap_GetState_View, NULL);
	_s(AP_MENU_ID_VIEW_TOOLBARS,	1,0,0,	NULL,				NULL,					NULL);
	_s(AP_MENU_ID_VIEW_TB_STD,		0,0,1,	"viewStd",			ap_GetState_View,		NULL);
	_s(AP_MENU_ID_VIEW_TB_FORMAT,	0,0,1,	"viewFormat",		ap_GetState_View,		NULL);
	_s(AP_MENU_ID_VIEW_TB_EXTRA,	0,0,1,	"viewExtra",		ap_GetState_View,		NULL);
	_s(AP_MENU_ID_VIEW_RULER,		0,0,1,	"viewRuler",		ap_GetState_View,		NULL);
	_s(AP_MENU_ID_VIEW_STATUSBAR,	0,0,1,	"viewStatus",		ap_GetState_View,		NULL);
	_s(AP_MENU_ID_VIEW_SHOWPARA,	0,0,1,	"viewPara",			ap_GetState_View,		NULL);
//	_s(AP_MENU_ID_VIEW_HEADFOOT,	0,0,1,	"viewHeadFoot",		ap_GetState_View,		NULL);
	_s(AP_MENU_ID_VIEW_FULLSCREEN, 0,0,1, "viewFullScreen", ap_GetState_View, NULL);
	_s(AP_MENU_ID_VIEW_ZOOM_MENU, 1,0,0, NULL, NULL, NULL);
	_s(AP_MENU_ID_VIEW_ZOOM,		0,1,0,	"dlgZoom",			NULL,					NULL);
	_s(AP_MENU_ID_VIEW_ZOOM_200, 0,0,0, "zoom200", NULL, NULL);
	_s(AP_MENU_ID_VIEW_ZOOM_100, 0,0,0, "zoom100", NULL, NULL);
	_s(AP_MENU_ID_VIEW_ZOOM_75, 0,0,0, "zoom75", NULL, NULL);
	_s(AP_MENU_ID_VIEW_ZOOM_50, 0,0,0, "zoom50", NULL, NULL);
	_s(AP_MENU_ID_VIEW_ZOOM_WHOLE, 0,0,0, "zoomWhole", NULL, NULL);
	_s(AP_MENU_ID_VIEW_ZOOM_WIDTH, 0,0,0, "zoomWidth", NULL, NULL);

	_s(AP_MENU_ID_INSERT,			1,0,0,	NULL,				NULL,					NULL);
	_s(AP_MENU_ID_INSERT_BREAK,		0,1,0,	"insBreak",			NULL,					NULL);
	_s(AP_MENU_ID_INSERT_BOOKMARK,  0,1,0,	"insertBookmark",	NULL,					NULL);
	_s(AP_MENU_ID_INSERT_HYPERLINK, 0,1,0,	"insertHyperlink",	ap_GetState_SomethingSelected,					NULL);
	_s(AP_MENU_ID_INSERT_DELETE_HYPERLINK, 0,0,0,	"deleteHyperlink",	NULL,			NULL);
	_s(AP_MENU_ID_INSERT_PAGENO,	0,1,0,	"insPageNo",		NULL,					NULL);
	_s(AP_MENU_ID_INSERT_DATETIME,	0,1,0,	"insDateTime",		NULL,					NULL);
	_s(AP_MENU_ID_INSERT_FIELD,		0,1,0,	"insField",			NULL,					NULL);
	_s(AP_MENU_ID_INSERT_FILE, 0,1,0, "insFile", NULL, NULL);
	_s(AP_MENU_ID_INSERT_SYMBOL,	0,1,0,	"insSymbol",		NULL,					NULL);
	_s(AP_MENU_ID_INSERT_ENDNOTE,	0,0,0,	"insEndnote",		NULL,					NULL);
#ifdef HAVE_GNOME
	_s(AP_MENU_ID_INSERT_PICTURE,   1,0,0,  NULL, NULL, NULL);
#else
	_s(AP_MENU_ID_INSERT_PICTURE,   0,1,0,  "fileInsertGraphic",NULL,                   NULL);
#endif
	_s(AP_MENU_ID_INSERT_CLIPART,   0,1,0,  "insertClipart",    NULL,                   NULL);
	_s(AP_MENU_ID_INSERT_GRAPHIC,	0,1,0,	"fileInsertGraphic",NULL,					NULL);
	_s(AP_MENU_ID_FORMAT,			1,0,0,	NULL,				NULL,					NULL);
	_s(AP_MENU_ID_FMT,			1,0,0,	NULL,				NULL,					NULL);
	_s(AP_MENU_ID_FMT_LANGUAGE,			0,1,0,	"dlgLanguage",	/*ap_GetState_Selection*/NULL,					NULL);
	_s(AP_MENU_ID_FMT_FONT,			0,1,0,	"dlgFont",	ap_GetState_StylesLocked,	NULL);
	_s(AP_MENU_ID_FMT_PARAGRAPH,	0,1,0,	"dlgParagraph",		ap_GetState_StylesLocked,	NULL);
	_s(AP_MENU_ID_FMT_BULLETS,		0,1,0,	"dlgBullets",	ap_GetState_StylesLocked,	NULL);
	_s(AP_MENU_ID_FMT_DOCUMENT, 0,1,0, "pageSetup", NULL, NULL);
	_s(AP_MENU_ID_FMT_BORDERS,		0,1,0,	"dlgBorders",		NULL,					NULL);
	_s(AP_MENU_ID_FMT_COLUMNS,		0,1,0,	"dlgColumns",		ap_GetState_ColumnsActive,					NULL);
	_s(AP_MENU_ID_FMT_BACKGROUND, 0,1,0, "dlgBackground", NULL, NULL);
	_s(AP_MENU_ID_FMT_HDRFTR,     0,1,0, "dlgHdrFtr", NULL, NULL);
	_s(AP_MENU_ID_FMT_IMAGE, 0,1,0, "dlgFmtImage", NULL, NULL);
	_s(AP_MENU_ID_FMT_STYLE,		0,1,0,	"dlgStyle",			NULL,					NULL);
	_s(AP_MENU_ID_FMT_TABS,			0,1,0,	"dlgTabs",			NULL,					NULL);
	_s(AP_MENU_ID_FMT_BOLD,			0,0,1,	"toggleBold",		ap_GetState_CharFmt,	NULL);
	_s(AP_MENU_ID_FMT_ITALIC,		0,0,1,	"toggleItalic",		ap_GetState_CharFmt,	NULL);
	_s(AP_MENU_ID_FMT_UNDERLINE,	0,0,1,	"toggleUline",		ap_GetState_CharFmt,	NULL);
	_s(AP_MENU_ID_FMT_OVERLINE,	0,0,1,	"toggleOline",		ap_GetState_CharFmt,	NULL);
	_s(AP_MENU_ID_FMT_STRIKE,		0,0,1,	"toggleStrike",		ap_GetState_CharFmt,	NULL);
	_s(AP_MENU_ID_FMT_TOPLINE,		0,0,1,	"toggleTopline",		ap_GetState_CharFmt,	NULL);
	_s(AP_MENU_ID_FMT_BOTTOMLINE,		0,0,1,	"toggleBottomline",		ap_GetState_CharFmt,	NULL);
	_s(AP_MENU_ID_FMT_SUPERSCRIPT,		0,0,1,	"toggleSuper",	ap_GetState_CharFmt,	NULL);
	_s(AP_MENU_ID_FMT_SUBSCRIPT,		0,0,1,	"toggleSub",	ap_GetState_CharFmt,	NULL);
	_s(AP_MENU_ID_FMT_TOGGLECASE,           0,1,0,  "dlgToggleCase", ap_GetState_SomethingSelected, NULL);

	_s(AP_MENU_ID_ALIGN,			1,0,0,	NULL,				ap_GetState_StylesLocked,	NULL);
	_s(AP_MENU_ID_ALIGN_LEFT,		0,0,1,	"alignLeft",		ap_GetState_BlockFmt,	NULL);
	_s(AP_MENU_ID_ALIGN_CENTER,		0,0,1,	"alignCenter",		ap_GetState_BlockFmt,	NULL);
	_s(AP_MENU_ID_ALIGN_RIGHT,		0,0,1,	"alignRight",		ap_GetState_BlockFmt,	NULL);
	_s(AP_MENU_ID_ALIGN_JUSTIFY,	0,0,1,	"alignJustify",		ap_GetState_BlockFmt,	NULL);

	_s(AP_MENU_ID_TOOLS,			1,0,0,	NULL,				NULL,					NULL);
	_s(AP_MENU_ID_TOOLS_SPELLING,	        1,0,0,	NULL,				NULL,				       NULL);
	_s(AP_MENU_ID_TOOLS_SPELL,	        0,1,0,	"dlgSpell",		ap_GetState_Spelling,					NULL);
	_s(AP_MENU_ID_TOOLS_SPELLPREFS, 0,1,0, "dlgSpellPrefs", NULL, NULL);
	_s(AP_MENU_ID_TOOLS_AUTOSPELL,          0,0,1,  "toggleAutoSpell",      ap_GetState_Prefs, NULL);
	_s(AP_MENU_ID_TOOLS_LANGUAGE, 1,0,0, NULL, NULL, NULL);
	_s(AP_MENU_ID_TOOLS_WORDCOUNT,		0,1,0,	"dlgWordCount",			NULL,					NULL);
	_s(AP_MENU_ID_TOOLS_PLUGINS, 0,1,0, "dlgPlugins", NULL, NULL);
	_s(AP_MENU_ID_TOOLS_OPTIONS,		0,1,0,	"dlgOptions",		NULL,					NULL);
	_s(AP_MENU_ID_TOOLS_SCRIPTS,	1,0,0,	"scriptPlay", ap_GetState_ScriptsActive, NULL);

	_s(AP_MENU_ID_TOOLS_REVISIONS,  1,0,0,  NULL,               NULL,                   NULL);
	_s(AP_MENU_ID_TOOLS_REVISIONS_MARK, 0,0,1, "toggleMarkRevisions", ap_GetState_MarkRevisions,NULL);
	_s(AP_MENU_ID_TOOLS_REVISIONS_ACCEPT_REVISION, 0,0,0, "revisionAccept", ap_GetState_RevisionPresent,NULL);
	_s(AP_MENU_ID_TOOLS_REVISIONS_REJECT_REVISION, 0,0,0, "revisionReject", ap_GetState_RevisionPresent,NULL);
	_s(AP_MENU_ID_TABLE,1,0,0,NULL,NULL,NULL);
    _s(AP_MENU_ID_TABLE_INSERT,1,0,0, NULL, NULL, NULL);
    _s(AP_MENU_ID_TABLE_INSERT_TABLE,0,0,0, NULL, NULL, NULL);
    _s(AP_MENU_ID_TABLE_INSERT_COLUMNS_LEFT,0,0,0, NULL, NULL, NULL);
    _s(AP_MENU_ID_TABLE_INSERT_COLUMNS_RIGHT,0,0,0, NULL, NULL, NULL);
    _s(AP_MENU_ID_TABLE_INSERT_ROWS_ABOVE,0,0,0, NULL, NULL, NULL);
    _s(AP_MENU_ID_TABLE_INSERT_ROWS_BELOW,0,0,0, NULL, NULL, NULL);
    _s(AP_MENU_ID_TABLE_INSERT_CELLS,0,0,0, NULL, NULL, NULL);
    _s(AP_MENU_ID_TABLE_DELETE,1,0,0, NULL, NULL, NULL);
    _s(AP_MENU_ID_TABLE_DELETE_TABLE,0,0,0, NULL, NULL, NULL);
    _s(AP_MENU_ID_TABLE_DELETE_COLUMNS,0,0,0, NULL, NULL, NULL);
    _s(AP_MENU_ID_TABLE_DELETE_ROWS,0,0,0, NULL, NULL, NULL);
    _s(AP_MENU_ID_TABLE_DELETE_CELLS,0,0,0, NULL, NULL, NULL);
    _s(AP_MENU_ID_TABLE_SELECT,1,0,0, NULL, NULL, NULL);
    _s(AP_MENU_ID_TABLE_SELECT_TABLE,0,0,0, NULL, NULL, NULL);
    _s(AP_MENU_ID_TABLE_SELECT_COLUMN,0,0,0, NULL, NULL, NULL);
    _s(AP_MENU_ID_TABLE_SELECT_ROW,0,0,0, NULL, NULL, NULL);
    _s(AP_MENU_ID_TABLE_SELECT_CELL,0,0,0, NULL, NULL, NULL);
    _s(AP_MENU_ID_TABLE_MERGE_CELLS,0,0,0, NULL, NULL, NULL);
    _s(AP_MENU_ID_TABLE_SPLIT_CELLS,0,0,0, NULL, NULL, NULL);
    _s(AP_MENU_ID_TABLE_SPLIT_TABLE,0,0,0, NULL, NULL, NULL);
    _s(AP_MENU_ID_TABLE_FORMAT,0,0,0, NULL, NULL, NULL);
    _s(AP_MENU_ID_TABLE_AUTOFIT,0,0,0, NULL, NULL, NULL);
    _s(AP_MENU_ID_TABLE_HEADING_ROWS_REPEAT,0,0,0, NULL, NULL, NULL);
    _s(AP_MENU_ID_TABLE_SORT,0,0,0, NULL, NULL, NULL);





	_s(AP_MENU_ID_CONTEXT_REVISIONS_ACCEPT_REVISION, 0,0,0, "revisionAccept", ap_GetState_RevisionPresent,NULL);
	_s(AP_MENU_ID_CONTEXT_REVISIONS_REJECT_REVISION, 0,0,0, "revisionReject", ap_GetState_RevisionPresent, NULL);
	_s(AP_MENU_ID_TOOLS_REVISIONS_SET_VIEW_LEVEL, 0,1,0, "revisionSetViewLevel", NULL, NULL);

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
	_s(AP_MENU_ID_WINDOW_MORE,		0,1,0,	"dlgMoreWindows",	NULL,					ap_GetLabel_WindowMore);

	_s(AP_MENU_ID_WEB, 1,0,0, NULL, NULL, NULL);
	_s(AP_MENU_ID_WEB_SAVEASWEB, 0,1,0, "fileSaveAsWeb", NULL, NULL);
	_s(AP_MENU_ID_WEB_WEBPREVIEW, 0,1,0, "filePreviewWeb", NULL, NULL);

	_s(AP_MENU_ID_HELP,				1,0,0,	NULL,				NULL,					NULL);
	_s(AP_MENU_ID_HELP_CONTENTS,		0,0,0,	"helpContents",			NULL,					ap_GetLabel_Contents);
	_s(AP_MENU_ID_HELP_INDEX,		0,0,0,	"helpIndex",			NULL,					ap_GetLabel_Index);
	_s(AP_MENU_ID_HELP_CHECKVER,		0,0,0,	"helpCheckVer",			NULL,					ap_GetLabel_Checkver);
	_s(AP_MENU_ID_HELP_SEARCH,		0,0,0,	"helpSearch",			NULL,					ap_GetLabel_Search);
	_s(AP_MENU_ID_HELP_ABOUT,		0,1,0,	"dlgAbout",			NULL,					ap_GetLabel_About);
	_s(AP_MENU_ID_HELP_ABOUTOS,		0,0,0,	"helpAboutOS",			NULL,					ap_GetLabel_AboutOS);
	_s(AP_MENU_ID_HELP_ABOUT_GNU, 0,0,0, "helpAboutGnu", NULL, NULL);
	_s(AP_MENU_ID_HELP_ABOUT_GNOMEOFFICE, 0,0,0, "helpAboutGnomeOffice", NULL, NULL);
	_s(AP_MENU_ID_HELP_CREDITS, 0,0,0, "helpCredits", NULL, NULL);
	_s(AP_MENU_ID_HELP_REPORT_BUG, 0,0,0, "helpReportBug", NULL, NULL);

	_s(AP_MENU_ID_SPELL_SUGGEST_1,	0,0,0,	"spellSuggest_1",	ap_GetState_Suggest,	ap_GetLabel_Suggest);
	_s(AP_MENU_ID_SPELL_SUGGEST_2,	0,0,0,	"spellSuggest_2",	ap_GetState_Suggest,	ap_GetLabel_Suggest);
	_s(AP_MENU_ID_SPELL_SUGGEST_3,	0,0,0,	"spellSuggest_3",	ap_GetState_Suggest,	ap_GetLabel_Suggest);
	_s(AP_MENU_ID_SPELL_SUGGEST_4,	0,0,0,	"spellSuggest_4",	ap_GetState_Suggest,	ap_GetLabel_Suggest);
	_s(AP_MENU_ID_SPELL_SUGGEST_5,	0,0,0,	"spellSuggest_5",	ap_GetState_Suggest,	ap_GetLabel_Suggest);
	_s(AP_MENU_ID_SPELL_SUGGEST_6,	0,0,0,	"spellSuggest_6",	ap_GetState_Suggest,	ap_GetLabel_Suggest);
	_s(AP_MENU_ID_SPELL_SUGGEST_7,	0,0,0,	"spellSuggest_7",	ap_GetState_Suggest,	ap_GetLabel_Suggest);
	_s(AP_MENU_ID_SPELL_SUGGEST_8,	0,0,0,	"spellSuggest_8",	ap_GetState_Suggest,	ap_GetLabel_Suggest);
	_s(AP_MENU_ID_SPELL_SUGGEST_9,	0,0,0,	"spellSuggest_9",	ap_GetState_Suggest,	ap_GetLabel_Suggest);
	_s(AP_MENU_ID_SPELL_IGNOREALL,	0,0,0,	"spellIgnoreAll",	NULL,					NULL);
	_s(AP_MENU_ID_SPELL_ADD,		0,0,0,	"spellAdd",			NULL,					NULL);

	_s(AP_MENU_ID_INSERT_AUTOTEXT,  1,0,0, NULL, NULL, NULL);
	_s(AP_MENU_ID_AUTOTEXT_ATTN, 1,0,0, NULL, NULL, NULL);
	_s(AP_MENU_ID_AUTOTEXT_CLOSING, 1,0,0, NULL, NULL, NULL);
	_s(AP_MENU_ID_AUTOTEXT_MAIL, 1,0,0, NULL, NULL, NULL);
	_s(AP_MENU_ID_AUTOTEXT_REFERENCE, 1,0,0, NULL, NULL, NULL);
	_s(AP_MENU_ID_AUTOTEXT_SALUTATION, 1,0,0, NULL, NULL, NULL);
	_s(AP_MENU_ID_AUTOTEXT_SUBJECT, 1,0,0, NULL, NULL, NULL);
	_s(AP_MENU_ID_AUTOTEXT_EMAIL, 1,0,0, NULL, NULL, NULL);

	_s(AP_MENU_ID_AUTOTEXT_ATTN_1, 0,0,0, "insAutotext_attn_1", NULL, ap_GetLabel_Autotext);
	_s(AP_MENU_ID_AUTOTEXT_ATTN_2, 0,0,0, "insAutotext_attn_2", NULL, ap_GetLabel_Autotext);

	_s(AP_MENU_ID_AUTOTEXT_CLOSING_1, 0,0,0, "insAutotext_closing_1", NULL, ap_GetLabel_Autotext);
	_s(AP_MENU_ID_AUTOTEXT_CLOSING_2, 0,0,0, "insAutotext_closing_2", NULL, ap_GetLabel_Autotext);
	_s(AP_MENU_ID_AUTOTEXT_CLOSING_3, 0,0,0, "insAutotext_closing_3", NULL, ap_GetLabel_Autotext);
	_s(AP_MENU_ID_AUTOTEXT_CLOSING_4, 0,0,0, "insAutotext_closing_4", NULL, ap_GetLabel_Autotext);
	_s(AP_MENU_ID_AUTOTEXT_CLOSING_5, 0,0,0, "insAutotext_closing_5", NULL, ap_GetLabel_Autotext);
	_s(AP_MENU_ID_AUTOTEXT_CLOSING_6, 0,0,0, "insAutotext_closing_6", NULL, ap_GetLabel_Autotext);
	_s(AP_MENU_ID_AUTOTEXT_CLOSING_7, 0,0,0, "insAutotext_closing_7", NULL, ap_GetLabel_Autotext);
	_s(AP_MENU_ID_AUTOTEXT_CLOSING_8, 0,0,0, "insAutotext_closing_8", NULL, ap_GetLabel_Autotext);
	_s(AP_MENU_ID_AUTOTEXT_CLOSING_9, 0,0,0, "insAutotext_closing_9", NULL, ap_GetLabel_Autotext);
	_s(AP_MENU_ID_AUTOTEXT_CLOSING_10, 0,0,0, "insAutotext_closing_10", NULL, ap_GetLabel_Autotext);
	_s(AP_MENU_ID_AUTOTEXT_CLOSING_11, 0,0,0, "insAutotext_closing_11", NULL, ap_GetLabel_Autotext);
	_s(AP_MENU_ID_AUTOTEXT_CLOSING_12, 0,0,0, "insAutotext_closing_12", NULL, ap_GetLabel_Autotext);

	_s(AP_MENU_ID_AUTOTEXT_MAIL_1, 0,0,0, "insAutotext_mail_1", NULL, ap_GetLabel_Autotext);
	_s(AP_MENU_ID_AUTOTEXT_MAIL_2, 0,0,0, "insAutotext_mail_2", NULL, ap_GetLabel_Autotext);
	_s(AP_MENU_ID_AUTOTEXT_MAIL_3, 0,0,0, "insAutotext_mail_3", NULL, ap_GetLabel_Autotext);
	_s(AP_MENU_ID_AUTOTEXT_MAIL_4, 0,0,0, "insAutotext_mail_4", NULL, ap_GetLabel_Autotext);
	_s(AP_MENU_ID_AUTOTEXT_MAIL_5, 0,0,0, "insAutotext_mail_5", NULL, ap_GetLabel_Autotext);
	_s(AP_MENU_ID_AUTOTEXT_MAIL_6, 0,0,0, "insAutotext_mail_6", NULL, ap_GetLabel_Autotext);
	_s(AP_MENU_ID_AUTOTEXT_MAIL_7, 0,0,0, "insAutotext_mail_7", NULL, ap_GetLabel_Autotext);
	_s(AP_MENU_ID_AUTOTEXT_MAIL_8, 0,0,0, "insAutotext_mail_8", NULL, ap_GetLabel_Autotext);

	_s(AP_MENU_ID_AUTOTEXT_REFERENCE_1, 0,0,0, "insAutotext_reference_1", NULL, ap_GetLabel_Autotext);
	_s(AP_MENU_ID_AUTOTEXT_REFERENCE_2, 0,0,0, "insAutotext_reference_2", NULL, ap_GetLabel_Autotext);
	_s(AP_MENU_ID_AUTOTEXT_REFERENCE_3, 0,0,0, "insAutotext_reference_3", NULL, ap_GetLabel_Autotext);

	_s(AP_MENU_ID_AUTOTEXT_SALUTATION_1, 0,0,0, "insAutotext_salutation_1", NULL, ap_GetLabel_Autotext);
	_s(AP_MENU_ID_AUTOTEXT_SALUTATION_2, 0,0,0, "insAutotext_salutation_2", NULL, ap_GetLabel_Autotext);
	_s(AP_MENU_ID_AUTOTEXT_SALUTATION_3, 0,0,0, "insAutotext_salutation_3", NULL, ap_GetLabel_Autotext);
	_s(AP_MENU_ID_AUTOTEXT_SALUTATION_4, 0,0,0, "insAutotext_salutation_4", NULL, ap_GetLabel_Autotext);

	_s(AP_MENU_ID_AUTOTEXT_SUBJECT_1, 0,0,0, "insAutotext_subject_1", NULL, ap_GetLabel_Autotext);

	_s(AP_MENU_ID_AUTOTEXT_EMAIL_1, 0,0,0, "insAutotext_email_1", NULL, ap_GetLabel_Autotext);
	_s(AP_MENU_ID_AUTOTEXT_EMAIL_2, 0,0,0, "insAutotext_email_2", NULL, ap_GetLabel_Autotext);
	_s(AP_MENU_ID_AUTOTEXT_EMAIL_3, 0,0,0, "insAutotext_email_3", NULL, ap_GetLabel_Autotext);
	_s(AP_MENU_ID_AUTOTEXT_EMAIL_4, 0,0,0, "insAutotext_email_4", NULL, ap_GetLabel_Autotext);
	_s(AP_MENU_ID_AUTOTEXT_EMAIL_5, 0,0,0, "insAutotext_email_5", NULL, ap_GetLabel_Autotext);
	_s(AP_MENU_ID_AUTOTEXT_EMAIL_6, 0,0,0, "insAutotext_email_6", NULL, ap_GetLabel_Autotext);

	// ... add others here ...

	_s(AP_MENU_ID__BOGUS2__,		0,0,0,	NULL,				NULL,					NULL);

#undef _s

	return pActionSet;
}
