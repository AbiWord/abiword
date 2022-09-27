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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

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
	UT_return_val_if_fail (pActionSet, nullptr);

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

#define _s(id,bHoldsSubMenu,bRaisesDialog,bCheckable,bRadio,szMethodName,pfnGetState,pfnGetLabel)	\
	pActionSet->setAction(id,bHoldsSubMenu,bRaisesDialog,bCheckable,bRadio,szMethodName,pfnGetState,pfnGetLabel)

	//( __id__,          bSub,bDlg,bCheck,bRadio,  szMethodName,       fnGetState,         fnGetLabel)

	_s(AP_MENU_ID__BOGUS1__,		0,0,0,0,	nullptr,				nullptr,					nullptr);

	_s(AP_MENU_ID_FILE,				1,0,0,0,	nullptr,				nullptr,					nullptr);
	_s(AP_MENU_ID_FILE_NEW,			0,0,0,0,	"fileNew",			nullptr,					nullptr);
	_s(AP_MENU_ID_FILE_NEW_USING_TEMPLATE,			0,1,0,0,	"fileNewUsingTemplate",			nullptr,					nullptr);
	_s(AP_MENU_ID_FILE_OPEN,		0,1,0,0,	"fileOpen",			nullptr,					nullptr);
	_s(AP_MENU_ID_FILE_IMPORTSTYLES,		0,1,0,0,	"importStyles",	nullptr,					nullptr);
	_s(AP_MENU_ID_FILE_SAVE,		0,0,0,0,	"fileSave",			ap_GetState_Changes,					nullptr);
	_s(AP_MENU_ID_FILE_SAVEAS,		0,1,0,0,	"fileSaveAs",		nullptr,					nullptr);
	_s(AP_MENU_ID_FILE_SAVEIMAGE,		0,1,0,0,	"fileSaveImage",		nullptr,					nullptr);
	_s(AP_MENU_ID_FILE_SAVE_TEMPLATE, 0,1,0,0, "fileSaveTemplate", nullptr, nullptr);
	_s(AP_MENU_ID_FILE_IMPORT, 0,1,0,0, "fileImport", nullptr, nullptr);
	_s(AP_MENU_ID_FILE_EXPORT, 0,1,0,0, "fileExport", nullptr, nullptr);
	_s(AP_MENU_ID_FILE_PROPERTIES, 0,1,0,0, "dlgMetaData", nullptr, nullptr);
	_s(AP_MENU_ID_FILE_CLOSE,		0,0,0,0,	"closeWindow",		nullptr,					nullptr);
	_s(AP_MENU_ID_FILE_PAGESETUP,	0,1,0,0,	"pageSetup",		nullptr,					nullptr);
#if TOOLKIT_GTK_ALL
	_s(AP_MENU_ID_FILE_PRINT,  0,1,0,0,	"cairoPrint",nullptr,nullptr);
	_s(AP_MENU_ID_FILE_PRINT_PREVIEW, 0,1,0,0, "cairoPrintPreview", nullptr, nullptr);
	_s(AP_MENU_ID_FILE_PRINT_DIRECTLY, 0,1,0,0, "cairoPrintDirectly", nullptr, nullptr);
#else
	_s(AP_MENU_ID_FILE_PRINT,		0,1,0,0,	"print",			nullptr,					nullptr);
	_s(AP_MENU_ID_FILE_PRINT_PREVIEW, 0,1,0,0, "printPreview", nullptr, nullptr);
#endif
	_s(AP_MENU_ID_FILE_RECENT,	1,0,0,0,	nullptr, ap_GetState_Recent,nullptr);
	_s(AP_MENU_ID_FILE_RECENT_1,	0,0,0,0,	"openRecent_1",		nullptr,					ap_GetLabel_Recent);
	_s(AP_MENU_ID_FILE_RECENT_2,	0,0,0,0,	"openRecent_2",		nullptr,					ap_GetLabel_Recent);
	_s(AP_MENU_ID_FILE_RECENT_3,	0,0,0,0,	"openRecent_3",		nullptr,					ap_GetLabel_Recent);
	_s(AP_MENU_ID_FILE_RECENT_4,	0,0,0,0,	"openRecent_4",		nullptr,					ap_GetLabel_Recent);
	_s(AP_MENU_ID_FILE_RECENT_5,	0,0,0,0,	"openRecent_5",		nullptr,					ap_GetLabel_Recent);
	_s(AP_MENU_ID_FILE_RECENT_6,	0,0,0,0,	"openRecent_6",		nullptr,					ap_GetLabel_Recent);
	_s(AP_MENU_ID_FILE_RECENT_7,	0,0,0,0,	"openRecent_7",		nullptr,					ap_GetLabel_Recent);
	_s(AP_MENU_ID_FILE_RECENT_8,	0,0,0,0,	"openRecent_8",		nullptr,					ap_GetLabel_Recent);
	_s(AP_MENU_ID_FILE_RECENT_9,	0,0,0,0,	"openRecent_9",		nullptr,					ap_GetLabel_Recent);
	_s(AP_MENU_ID_FILE_REVERT, 0,0,0,0, "fileRevert", ap_GetState_Changes, nullptr);
	_s(AP_MENU_ID_FILE_EXIT,		0,0,0,0,	"querySaveAndExit",	nullptr,					nullptr);
	_s(AP_MENU_ID_FILE_SAVEEMBED,		0,1,0,0,	"fileSaveEmbed",	nullptr,					nullptr);
	_s(AP_MENU_ID_OPEN_TEMPLATE, 0,1,0,0, "openTemplate", nullptr, nullptr);

	_s(AP_MENU_ID_EDIT,				1,0,0,0,	nullptr,				nullptr,					nullptr);
	_s(AP_MENU_ID_EDIT_UNDO,		0,0,0,0,	"undo",				ap_GetState_Changes,	nullptr);
	_s(AP_MENU_ID_EDIT_REDO,		0,0,0,0,	"redo",				ap_GetState_Changes,	nullptr);
	_s(AP_MENU_ID_EDIT_CUT,			0,0,0,0,	"cut",				ap_GetState_Selection,	nullptr);
	_s(AP_MENU_ID_EDIT_LATEXEQUATION,			0,1,0,0,	"editLatexAtPos",				ap_GetState_Selection,	nullptr);
	_s(AP_MENU_ID_EDIT_COPY,		0,0,0,0,	"copy",				ap_GetState_Selection,	nullptr);
	_s(AP_MENU_ID_EDIT_PASTE,		0,0,0,0,	"paste",			ap_GetState_Clipboard,	nullptr);
	_s(AP_MENU_ID_EDIT_PASTE_SPECIAL,0,0,0,0, "pasteSpecial",     ap_GetState_Clipboard,  nullptr);
	_s(AP_MENU_ID_EDIT_CLEAR,		0,0,0,0,	"delRight",			nullptr,					nullptr);
	_s(AP_MENU_ID_EDIT_SELECTALL,	0,0,0,0,	"selectAll",		nullptr,					nullptr);
	_s(AP_MENU_ID_EDIT_FIND,		0,1,0,0,	"find",				nullptr,					nullptr);
	_s(AP_MENU_ID_EDIT_REPLACE,		0,1,0,0,	"replace",			nullptr,					nullptr);
	_s(AP_MENU_ID_EDIT_GOTO,		0,1,0,0,	"go",				nullptr,					nullptr);
	_s(AP_MENU_ID_EDIT_EDITHEADER,		0,0,0,0,	"editHeader",	ap_GetState_Changes,					nullptr);
	_s(AP_MENU_ID_EDIT_EDITFOOTER,		0,0,0,0,	"editFooter",	ap_GetState_Changes,					nullptr);
	_s(AP_MENU_ID_EDIT_REMOVEHEADER,		0,0,0,0,"removeHeader",ap_GetState_Changes	,					nullptr);
	_s(AP_MENU_ID_EDIT_REMOVEFOOTER,		0,0,0,0,	"removeFooter",ap_GetState_Changes,					nullptr);
	_s(AP_MENU_ID_EDIT_DELETEFRAME,		0,0,0,0,	"deleteFrame", nullptr,nullptr);
	_s(AP_MENU_ID_EDIT_DELETEIMAGE,		0,0,0,0,	"deleteFrame", nullptr,nullptr);
	_s(AP_MENU_ID_FMT_POSIMAGE,		0,1,0,0,	"dlgFmtPosImage", nullptr,nullptr);
	_s(AP_MENU_ID_EDIT_CUTIMAGE,		0,0,0,0,	"cutFrame", nullptr,nullptr);
	_s(AP_MENU_ID_EDIT_COPYIMAGE,		0,0,0,0,	"copyFrame", nullptr,nullptr);
	_s(AP_MENU_ID_EDIT_CUT_FRAME,		0,0,0,0,	"cutFrame", nullptr,nullptr);
	_s(AP_MENU_ID_EDIT_COPY_FRAME,		0,0,0,0,	"copyFrame",nullptr,nullptr);
	_s(AP_MENU_ID_EDIT_SELECT_FRAME,	0,0,0,0,	"selectFrame",nullptr,nullptr);
	_s(AP_MENU_ID_EDIT_CUTEMBED,	0,0,0,0,	"cut",nullptr,nullptr);
	_s(AP_MENU_ID_EDIT_COPYEMBED,	0,0,0,0,	"copy",nullptr,nullptr);
	_s(AP_MENU_ID_EDIT_DELETEEMBED,	0,0,0,0,	"delLeft",nullptr,nullptr);

	_s(AP_MENU_ID_VIEW,				1,0,0,0,	nullptr,				nullptr,					nullptr);
	_s(AP_MENU_ID_VIEW_NORMAL, 0,0,0,1, "viewNormalLayout", ap_GetState_View, nullptr);
	_s(AP_MENU_ID_VIEW_WEB,    0,0,0,1, "viewWebLayout", ap_GetState_View, nullptr);
	_s(AP_MENU_ID_VIEW_PRINT,  0,0,0,1, "viewPrintLayout", ap_GetState_View, nullptr);
	_s(AP_MENU_ID_VIEW_TOOLBARS,	1,0,0,0,	nullptr,				nullptr,					nullptr);
	_s(AP_MENU_ID_VIEW_TB_1,		0,0,1,0,	"viewTB1",			ap_GetState_View,		ap_GetLabel_Toolbar);
	_s(AP_MENU_ID_VIEW_TB_2,		0,0,1,0,	"viewTB2",			ap_GetState_View,		ap_GetLabel_Toolbar);
	_s(AP_MENU_ID_VIEW_TB_3,		0,0,1,0,	"viewTB3",			ap_GetState_View,		ap_GetLabel_Toolbar);
	_s(AP_MENU_ID_VIEW_TB_4,		0,0,1,0,	"viewTB4",			ap_GetState_View,		ap_GetLabel_Toolbar);
	_s(AP_MENU_ID_VIEW_LOCK_TB_LAYOUT, 0,0,1,0,	"lockToolbarLayout", ap_GetState_View,			nullptr);
	_s(AP_MENU_ID_VIEW_DEFAULT_TB_LAYOUT, 0,0,0,0,	"defaultToolbarLayout", ap_GetState_View,		nullptr);
	_s(AP_MENU_ID_VIEW_RULER,	0,0,1,0,	"viewRuler",		ap_GetState_View,		nullptr);
	_s(AP_MENU_ID_VIEW_STATUSBAR,	0,0,1,0,	"viewStatus",		ap_GetState_View,		nullptr);
	_s(AP_MENU_ID_VIEW_LOCKSTYLES,	0,0,1,0,	"viewLockStyles",   ap_GetState_View,		nullptr);
	_s(AP_MENU_ID_VIEW_SHOWPARA,	0,0,1,0,	"viewPara",			ap_GetState_View,		nullptr);
	_s(AP_MENU_ID_VIEW_FULLSCREEN, 0,0,1,0, "viewFullScreen", ap_GetState_View, nullptr);
	_s(AP_MENU_ID_VIEW_ZOOM_MENU, 1,0,0,0, nullptr, nullptr, nullptr);
	_s(AP_MENU_ID_VIEW_ZOOM,		0,1,0,0,	"dlgZoom",			nullptr,					nullptr);
	_s(AP_MENU_ID_VIEW_ZOOM_200, 0,0,0,1, "zoom200", ap_GetState_Zoom, nullptr);
	_s(AP_MENU_ID_VIEW_ZOOM_100, 0,0,0,1, "zoom100", ap_GetState_Zoom, nullptr);
	_s(AP_MENU_ID_VIEW_ZOOM_75, 0,0,0,1, "zoom75", ap_GetState_Zoom, nullptr);
	_s(AP_MENU_ID_VIEW_ZOOM_50, 0,0,0,1, "zoom50", ap_GetState_Zoom, nullptr);
	_s(AP_MENU_ID_VIEW_ZOOM_WHOLE, 0,0,0,1, "zoomWhole", ap_GetState_Zoom, nullptr);
	_s(AP_MENU_ID_VIEW_ZOOM_WIDTH, 0,0,0,1, "zoomWidth", ap_GetState_Zoom, nullptr);

	_s(AP_MENU_ID_INSERT,			1,0,0,0,	nullptr,				nullptr,					nullptr);
	_s(AP_MENU_ID_INSERT_BREAK,		0,1,0,0,	"insBreak",			ap_GetState_BreakOK,					nullptr);
	_s(AP_MENU_ID_INSERT_BOOKMARK,  0,1,0,0,	"insertBookmark",	ap_GetState_BookmarkOK,					nullptr);
	_s(AP_MENU_ID_INSERT_XMLID,     0,1,0,0,	"insertXMLID"  ,	ap_GetState_xmlidOK,					nullptr);
	_s(AP_MENU_ID_INSERT_HYPERLINK, 0,1,0,0,	"insertHyperlink",	ap_GetState_HyperlinkOK,					nullptr);
	_s(AP_MENU_ID_INSERT_GOTO_HYPERLINK, 0,1,0,0,	"hyperlinkJumpPos",	ap_GetState_HyperlinkOK,					nullptr);
	_s(AP_MENU_ID_INSERT_EDIT_HYPERLINK, 0,1,0,0,	"insertHyperlink",	ap_GetState_HyperlinkOK,					nullptr);
	_s(AP_MENU_ID_EDIT_COPY_HYPERLINK_LOCATION, 0,0,0,0, "hyperlinkCopyLocation", nullptr, nullptr);
	_s(AP_MENU_ID_INSERT_DELETE_HYPERLINK, 0,0,0,0,	"deleteHyperlink",	nullptr,			nullptr);
	_s(AP_MENU_ID_INSERT_PAGENO,	0,1,0,0,	"insPageNo",		nullptr,					nullptr);
	_s(AP_MENU_ID_INSERT_DATETIME,	0,1,0,0,	"insDateTime",		nullptr,					nullptr);
	_s(AP_MENU_ID_INSERT_FIELD,		0,1,0,0,	"insField",			nullptr,					nullptr);
	_s(AP_MENU_ID_INSERT_TEXTBOX,		0,0,0,0,	"insTextBox",   ap_GetState_InsTextBox,					nullptr);
	_s(AP_MENU_ID_INSERT_MAILMERGE,		0,1,0,0,	"insMailMerge",			nullptr,					nullptr);
	_s(AP_MENU_ID_INSERT_FILE, 0,1,0,0, "insFile", nullptr, nullptr);
	_s(AP_MENU_ID_INSERT_SYMBOL,	0,1,0,0,	"insSymbol",		nullptr,					nullptr);
	_s(AP_MENU_ID_INSERT_TABLEOFCONTENTS,	0,0,0,0,	"insTOC",	ap_GetState_TOCOK, nullptr);
	_s(AP_MENU_ID_INSERT_FOOTNOTE,	0,0,0,0,	"insFootnote",		ap_GetState_InFootnote,					nullptr);
	_s(AP_MENU_ID_INSERT_ENDNOTE,	0,0,0,0,	"insEndnote",		ap_GetState_InFootnote,					nullptr);
	_s(AP_MENU_ID_INSERT_HEADER,		0,0,0,0,	"editHeader",	ap_GetState_Changes,					nullptr);
	_s(AP_MENU_ID_INSERT_FOOTER,		0,0,0,0,	"editFooter",	ap_GetState_Changes,					nullptr);
	_s(AP_MENU_ID_INSERT_CLIPART,   0,1,0,0,  "insertClipart",    nullptr,                   nullptr);
	_s(AP_MENU_ID_INSERT_GRAPHIC,	0,1,0,0,	"fileInsertGraphic",nullptr,					nullptr);

	_s(AP_MENU_ID_INSERT_DIRECTIONMARKER,  1,0,0,0, nullptr, nullptr, nullptr);
	_s(AP_MENU_ID_INSERT_DIRECTIONMARKER_LRM,0,0,0,0, "insertLRM", nullptr, nullptr);
	_s(AP_MENU_ID_INSERT_DIRECTIONMARKER_RLM,0,0,0,0, "insertRLM", nullptr, nullptr);

	_s(AP_MENU_ID_FORMAT,			1,0,0,0,	nullptr,				nullptr,					nullptr);
	_s(AP_MENU_ID_FMT,			1,0,0,0,	nullptr,				nullptr,					nullptr);
	_s(AP_MENU_ID_FMT_LANGUAGE,			0,1,0,0,	"dlgLanguage",	ap_GetState_StylesLocked,					nullptr);
	_s(AP_MENU_ID_FMT_FONT,			0,1,0,0,	"dlgFont",	ap_GetState_StylesLocked,	nullptr);
	_s(AP_MENU_ID_FMT_PARAGRAPH,	0,1,0,0,	"dlgParagraph",		ap_GetState_StylesLocked,	nullptr);
	_s(AP_MENU_ID_FMT_BULLETS,		0,1,0,0,	"dlgBullets",	ap_GetState_Lists,	nullptr);
	_s(AP_MENU_ID_FMT_TABLE,		0,1,0,0, "formatTable", ap_GetState_PointOrAnchorInTable, nullptr);
	_s(AP_MENU_ID_FMT_DOCUMENT, 0,1,0,0, "pageSetup", nullptr, nullptr);
	_s(AP_MENU_ID_FMT_BORDERS,		0,1,0,0,	"dlgBorders",		nullptr,					nullptr);
	_s(AP_MENU_ID_FMT_COLUMNS,		0,1,0,0,	"dlgColumns",		ap_GetState_ColumnsActive,					nullptr);
	_s(AP_MENU_ID_FMT_BACKGROUND, 1,0,0,0, nullptr, nullptr, nullptr);
	_s(AP_MENU_ID_FMT_BACKGROUND_PAGE_COLOR, 0,1,0,0, "dlgBackground", nullptr, nullptr);
	_s(AP_MENU_ID_FMT_BACKGROUND_PAGE_IMAGE, 0,1,0,0, "fileInsertPageBackgroundGraphic", nullptr, nullptr);
	_s(AP_MENU_ID_FMT_HDRFTR,     0,1,0,0, "dlgHdrFtr", ap_GetState_FmtHdrFtr, nullptr);
	_s(AP_MENU_ID_FMT_TABLEOFCONTENTS, 0,1,0,0, "formatTOC", ap_GetState_InTOC, nullptr);
	_s(AP_MENU_ID_FMT_FOOTNOTES,     0,1,0,0, "formatFootnotes", nullptr, nullptr);
	_s(AP_MENU_ID_FMT_IMAGE, 0,1,0,0, "dlgFmtImage", ap_GetState_InImage, nullptr);
	_s(AP_MENU_ID_CONTEXT_IMAGE, 0,1,0,0, "dlgFmtImageCtxt", ap_GetState_InImage, nullptr);
	_s(AP_MENU_ID_FMT_SETPOSIMAGE, 0,0,0,0, "setPosImage", ap_GetState_SetPosImage, nullptr);
	_s(AP_MENU_ID_FMT_STYLE_DEFINE,		0,1,0,0,	"dlgStyle",			nullptr,					nullptr);
	_s(AP_MENU_ID_FMT_STYLE,		1,0,0,0,	nullptr,			nullptr,					nullptr);
	_s(AP_MENU_ID_FMT_STYLIST,		0,1,0,0,	"dlgStylist",	nullptr,					nullptr);
	_s(AP_MENU_ID_FMT_TABS,			0,1,0,0,	"dlgTabs",			nullptr,					nullptr);
	_s(AP_MENU_ID_FMT_BOLD,			0,0,1,0,	"toggleBold",		ap_GetState_CharFmt,	nullptr);
	_s(AP_MENU_ID_FMT_ITALIC,		0,0,1,0,	"toggleItalic",		ap_GetState_CharFmt,	nullptr);
	_s(AP_MENU_ID_FMT_UNDERLINE,	0,0,1,0,	"toggleUline",		ap_GetState_CharFmt,	nullptr);
	_s(AP_MENU_ID_FMT_OVERLINE,	0,0,1,0,	"toggleOline",		ap_GetState_CharFmt,	nullptr);
	_s(AP_MENU_ID_FMT_STRIKE,		0,0,1,0,	"toggleStrike",		ap_GetState_CharFmt,	nullptr);
	_s(AP_MENU_ID_FMT_TOPLINE,		0,0,1,0,	"toggleTopline",		ap_GetState_CharFmt,	nullptr);
	_s(AP_MENU_ID_FMT_BOTTOMLINE,		0,0,1,0,	"toggleBottomline",		ap_GetState_CharFmt,	nullptr);
	_s(AP_MENU_ID_FMT_SUPERSCRIPT,		0,0,1,0,	"toggleSuper",	ap_GetState_CharFmt,	nullptr);
	_s(AP_MENU_ID_FMT_SUBSCRIPT,		0,0,1,0,	"toggleSub",	ap_GetState_CharFmt,	nullptr);
	_s(AP_MENU_ID_FMT_TOGGLECASE,           0,1,0,0,  "dlgToggleCase", ap_GetState_SomethingSelected, nullptr);
	_s(AP_MENU_ID_FMT_FRAME,           0,1,0,0,  "dlgFormatFrame", ap_GetState_InFrame, nullptr);

	_s(AP_MENU_ID_FMT_DIRECTION,  1,0,0,0, nullptr, nullptr, nullptr);
	_s(AP_MENU_ID_FMT_DIRECTION_DD_RTL,0,0,1,0, "toggleDomDirection", ap_GetState_BlockFmt, nullptr);
	_s(AP_MENU_ID_FMT_DIRECTION_SD_RTL,0,0,1,0, "toggleDomDirectionSect", ap_GetState_SectFmt, nullptr);
	_s(AP_MENU_ID_FMT_DIRECTION_DOCD_RTL,0,0,1,0, "toggleDomDirectionDoc", ap_GetState_DocFmt, nullptr);
	_s(AP_MENU_ID_FMT_DIRECTION_DO_LTR,0,0,1,0, "toggleDirOverrideLTR", ap_GetState_CharFmt, nullptr);
	_s(AP_MENU_ID_FMT_DIRECTION_DO_RTL,0,0,1,0, "toggleDirOverrideRTL", ap_GetState_CharFmt, nullptr);
	_s(AP_MENU_ID_FMT_EMBED,0,1,0,0, "editEmbed", nullptr, nullptr);

	_s(AP_MENU_ID_ALIGN,			1,0,0,0,	nullptr,				ap_GetState_StylesLocked,	nullptr);
	_s(AP_MENU_ID_ALIGN_LEFT,		0,0,0,1,	"alignLeft",		ap_GetState_BlockFmt,	nullptr);
	_s(AP_MENU_ID_ALIGN_CENTER,		0,0,0,1,	"alignCenter",		ap_GetState_BlockFmt,	nullptr);
	_s(AP_MENU_ID_ALIGN_RIGHT,		0,0,0,1,	"alignRight",		ap_GetState_BlockFmt,	nullptr);
	_s(AP_MENU_ID_ALIGN_JUSTIFY,	0,0,0,1,	"alignJustify",		ap_GetState_BlockFmt,	nullptr);

	_s(AP_MENU_ID_TOOLS,			1,0,0,0,	nullptr,				nullptr,					nullptr);
#ifdef ENABLE_SPELL
	_s(AP_MENU_ID_TOOLS_SPELLING,	        1,0,0,0,	nullptr,				nullptr,				       nullptr);
	_s(AP_MENU_ID_TOOLS_SPELL,	        0,1,0,0,	"dlgSpell",		ap_GetState_Spelling,					nullptr);
	_s(AP_MENU_ID_TOOLS_SPELLPREFS, 0,1,0,0, "dlgSpellPrefs", nullptr, nullptr);
	_s(AP_MENU_ID_TOOLS_AUTOSPELL,          0,0,1,0,  "toggleAutoSpell",      ap_GetState_Prefs, nullptr);
#endif
	_s(AP_MENU_ID_TOOLS_LANGUAGE, 1,0,0,0, nullptr, nullptr, nullptr);
	_s(AP_MENU_ID_TOOLS_WORDCOUNT,		0,1,0,0,	"dlgWordCount",			nullptr,					nullptr);
	_s(AP_MENU_ID_TOOLS_PLUGINS, 0,1,0,0, "dlgPlugins", nullptr, nullptr);
	_s(AP_MENU_ID_TOOLS_OPTIONS,		0,1,0,0,	"dlgOptions",		nullptr,					nullptr);
	_s(AP_MENU_ID_TOOLS_SCRIPTS,	0,1,0,0,	"scriptPlay", ap_GetState_ScriptsActive, nullptr);
	_s(AP_MENU_ID_TOOLS_MAILMERGE,	0,1,0,0,	"mailMerge", ap_GetState_MailMerge, nullptr);
	_s(AP_MENU_ID_TOOLS_REVISIONS,  1,0,0,0,  nullptr,               nullptr,                   nullptr);
	_s(AP_MENU_ID_TOOLS_REVISIONS_MARK, 0,0,1,0, "toggleMarkRevisions", ap_GetState_MarkRevisionsCheck,nullptr);
	_s(AP_MENU_ID_TOOLS_REVISIONS_NEW_REVISION, 0,1,0,0, "startNewRevision", ap_GetState_MarkRevisions,nullptr);
	_s(AP_MENU_ID_TOOLS_REVISIONS_SHOW, 0,0,1,0, "toggleShowRevisions", ap_GetState_ShowRevisions,nullptr);
	_s(AP_MENU_ID_TOOLS_REVISIONS_SHOW_AFTERPREV, 0,0,1,0, "toggleShowRevisionsAfterPrevious", ap_GetState_ShowRevisionsAfterPrev,nullptr);
	_s(AP_MENU_ID_TOOLS_REVISIONS_SHOW_AFTER, 0,0,1,0, "toggleShowRevisionsAfter", ap_GetState_ShowRevisionsAfter,nullptr);
	_s(AP_MENU_ID_TOOLS_REVISIONS_SHOW_BEFORE, 0,0,1,0, "toggleShowRevisionsBefore", ap_GetState_ShowRevisionsBefore,nullptr);

	_s(AP_MENU_ID_TOOLS_REVISIONS_ACCEPT_REVISION, 0,0,0,0, "revisionAccept", ap_GetState_RevisionPresent,nullptr);
	_s(AP_MENU_ID_TOOLS_REVISIONS_REJECT_REVISION, 0,0,0,0, "revisionReject", ap_GetState_RevisionPresent,nullptr);
	_s(AP_MENU_ID_TOOLS_REVISIONS_PURGE, 0,0,0,0, "purgeAllRevisions", ap_GetState_HasRevisions,nullptr);
	_s(AP_MENU_ID_TOOLS_REVISIONS_COMPARE_DOCUMENTS, 0,1,0,0, "revisionCompareDocuments",nullptr,nullptr);
	_s(AP_MENU_ID_TOOLS_REVISIONS_AUTO, 0,0,1,0, "toggleAutoRevision",ap_GetState_AutoRevision,nullptr);
	_s(AP_MENU_ID_TOOLS_REVISIONS_FIND_NEXT, 0,0,0,0, "revisionFindNext", ap_GetState_HasRevisions,nullptr);
	_s(AP_MENU_ID_TOOLS_REVISIONS_FIND_PREV, 0,0,0,0, "revisionFindPrev", ap_GetState_HasRevisions,nullptr);

	// RIVERA
	_s(AP_MENU_ID_TOOLS_ANNOTATIONS,				1,0,0,0,	nullptr,						nullptr,								nullptr);
	_s(AP_MENU_ID_TOOLS_ANNOTATIONS_INSERT,			0,1,0,0,	"insAnnotation",			ap_GetState_InAnnotation,			nullptr);
	_s(AP_MENU_ID_TOOLS_ANNOTATIONS_INSERT_FROMSEL,	0,1,0,0,	"insAnnotationFromSel",		ap_GetState_InAnnotation,			nullptr);
	_s(AP_MENU_ID_TOOLS_ANNOTATIONS_TOGGLE_DISPLAY,	0,0,1,0,	"toggleDisplayAnnotations",	ap_GetState_ToggleAnnotations,		nullptr);

	_s(AP_MENU_ID_GOTO_ANNOTATION,					0,0,0,0,	"hyperlinkJumpPos",			ap_GetState_AnnotationJumpOK,			nullptr);
	_s(AP_MENU_ID_EDIT_ANNOTATION,					0,1,0,0,	"editAnnotation",			ap_GetState_HyperlinkOK,			nullptr);
	_s(AP_MENU_ID_DELETE_ANNOTATION,				0,0,0,0,	"deleteHyperlink",			nullptr,								nullptr);

	_s(AP_MENU_ID_RDFANCHOR_EDIT_TRIPLES,   0,0,0,0, "rdfAnchorEditTriples", nullptr, nullptr);
	_s(AP_MENU_ID_RDFANCHOR_QUERY,         0,0,0,0, "rdfAnchorQuery", nullptr, nullptr);
	_s(AP_MENU_ID_RDFANCHOR_EDITSEMITEM,      0,0,0,0, "rdfAnchorEditSemanticItem", ap_GetState_haveSemItems, nullptr);
	_s(AP_MENU_ID_RDFANCHOR_EXPORTSEMITEM,    0,0,0,0, "rdfAnchorExportSemanticItem", ap_GetState_haveSemItems, nullptr);
	_s(AP_MENU_ID_RDFANCHOR_SELECTTHISREFTOSEMITEM,    0,0,0,0, "rdfAnchorSelectThisReferenceToSemanticItem", ap_GetState_haveSemItems, nullptr);
	_s(AP_MENU_ID_RDFANCHOR_SELECTNEXTREFTOSEMITEM,    0,0,0,0, "rdfAnchorSelectNextReferenceToSemanticItem", ap_GetState_haveSemItems, nullptr);
	_s(AP_MENU_ID_RDFANCHOR_SELECTPREVREFTOSEMITEM,    0,0,0,0, "rdfAnchorSelectPrevReferenceToSemanticItem", ap_GetState_haveSemItems, nullptr);



	_s(AP_MENU_ID_TABLE,1,0,0,0,nullptr,nullptr,nullptr);
	_s(AP_MENU_ID_TABLE_INSERT,1,0,0,0, nullptr, nullptr, nullptr);
	_s(AP_MENU_ID_TABLE_INSERT_TABLE,0,1,0,0, "insertTable",ap_GetState_TableOK, nullptr);
	_s(AP_MENU_ID_TABLE_INSERTTABLE,0,1,0,0, "insertTable",ap_GetState_TableOK , nullptr);
	_s(AP_MENU_ID_TABLE_INSERT_COLUMNS_BEFORE,0,0,0,0, "insertColsBefore", ap_GetState_InTable, nullptr);
	_s(AP_MENU_ID_TABLE_INSERT_COLUMNS_AFTER,0,0,0,0, "insertColsAfter", ap_GetState_InTable, nullptr);
	_s(AP_MENU_ID_TABLE_INSERTCOLUMN,0,0,0,0, "insertColsAfter", ap_GetState_InTable, nullptr);
	_s(AP_MENU_ID_TABLE_INSERT_ROWS_BEFORE,0,0,0,0,"insertRowsBefore", ap_GetState_InTable, nullptr);
	_s(AP_MENU_ID_TABLE_INSERT_ROWS_AFTER,0,0,0,0,"insertRowsAfter", ap_GetState_InTable, nullptr);
	_s(AP_MENU_ID_TABLE_INSERTROW,0,0,0,0,"insertRowsAfter", ap_GetState_InTable, nullptr);
	_s(AP_MENU_ID_TABLE_INSERT_CELLS,0,0,0,0, nullptr, ap_GetState_InTable, nullptr);
	_s(AP_MENU_ID_TABLE_INSERT_SUMROWS,0,0,0,0, "insertSumRows", ap_GetState_InTable, nullptr);
	_s(AP_MENU_ID_TABLE_INSERT_SUMCOLS,0,0,0,0, "insertSumCols", ap_GetState_InTable, nullptr);
	_s(AP_MENU_ID_TABLE_DELETE,1,0,0,0, nullptr, ap_GetState_PointOrAnchorInTable, nullptr);
	_s(AP_MENU_ID_TABLE_DELETE_TABLE,0,0,0,0, "deleteTable", ap_GetState_PointOrAnchorInTable, nullptr);
	_s(AP_MENU_ID_TABLE_DELETETABLE,0,0,0,0, "deleteTable", ap_GetState_PointOrAnchorInTable, nullptr);
	_s(AP_MENU_ID_TABLE_DELETE_COLUMNS,0,0,0,0, "deleteColumns", ap_GetState_InTable, nullptr);
	_s(AP_MENU_ID_TABLE_DELETECOLUMN,0,0,0,0, "deleteColumns", ap_GetState_InTable, nullptr);
	_s(AP_MENU_ID_TABLE_DELETE_ROWS,0,0,0,0, "deleteRows", ap_GetState_InTable, nullptr);
	_s(AP_MENU_ID_TABLE_DELETEROW,0,0,0,0, "deleteRows", ap_GetState_InTable, nullptr);
	_s(AP_MENU_ID_TABLE_DELETE_CELLS,0,0,0,0, "deleteCell", ap_GetState_InTable, nullptr);
	_s(AP_MENU_ID_TABLE_TEXTTOTABLE_ALL,1,0,0,0, "textToTable", ap_GetState_TextToTableOK, nullptr);
	_s(AP_MENU_ID_TABLE_TEXTTOTABLE_COMMAS,1,0,0,0, "textToTableCommas", ap_GetState_TextToTableOK, nullptr);
	_s(AP_MENU_ID_TABLE_TEXTTOTABLE_SPACES,1,0,0,0, "textToTableSpaces", ap_GetState_TextToTableOK, nullptr);
	_s(AP_MENU_ID_TABLE_TEXTTOTABLE_TABS,1,0,0,0, "textToTableTabs", ap_GetState_TextToTableOK, nullptr);
	_s(AP_MENU_ID_TABLE_SORTROWSASCEND,1,0,0,0, "sortRowsAscend", ap_GetState_InTable, nullptr);
	_s(AP_MENU_ID_TABLE_SORTROWSDESCEND,1,0,0,0, "sortRowsDescend", ap_GetState_InTable, nullptr);
	_s(AP_MENU_ID_TABLE_SORTCOLSASCEND,1,0,0,0, "sortColsAscend", ap_GetState_InTable, nullptr);
	_s(AP_MENU_ID_TABLE_SORTCOLSDESCEND,1,0,0,0, "sortColsDescend", ap_GetState_InTable, nullptr);
	_s(AP_MENU_ID_TABLE_TABLETOTEXTCOMMAS,1,0,0,0, "tableToTextCommas", ap_GetState_PointOrAnchorInTable, nullptr);
	_s(AP_MENU_ID_TABLE_TABLETOTEXTTABS,1,0,0,0, "tableToTextTabs", ap_GetState_PointOrAnchorInTable, nullptr);
	_s(AP_MENU_ID_TABLE_TABLETOTEXTCOMMASTABS,1,0,0,0, "tableToTextCommasTabs", ap_GetState_PointOrAnchorInTable, nullptr);
	_s(AP_MENU_ID_TABLE_SELECT,1,0,0,0, nullptr, ap_GetState_InTable, nullptr);
	_s(AP_MENU_ID_TABLE_SELECT_TABLE,0,0,0,0, "selectTable", ap_GetState_InTable, nullptr);
	_s(AP_MENU_ID_TABLE_SELECT_COLUMN,0,0,0,0, "selectColumn", ap_GetState_InTable, nullptr);
	_s(AP_MENU_ID_TABLE_SELECT_ROW,0,0,0,0, "selectRow", ap_GetState_InTable, nullptr);
	_s(AP_MENU_ID_TABLE_SELECT_CELL,0,0,0,0, "selectCell", ap_GetState_InTable, nullptr);
	_s(AP_MENU_ID_TABLE_MERGE_CELLS,0,1,0,0, "mergeCells", ap_GetState_InTable, nullptr);
	_s(AP_MENU_ID_TABLE_SPLIT_CELLS,0,1,0,0, "splitCells", ap_GetState_InTableMerged, nullptr);
	_s(AP_MENU_ID_TABLE_SPLIT_TABLE,0,0,0,0, nullptr, ap_GetState_AlwaysDisabled, nullptr);
	_s(AP_MENU_ID_TABLE_FORMAT,0,1,0,0, "formatTable", ap_GetState_PointOrAnchorInTable, nullptr);
	_s(AP_MENU_ID_TABLE_AUTOFIT,0,0,0,0, "autoFitTable", ap_GetState_PointInTable, nullptr);
	_s(AP_MENU_ID_TABLE_TABLETOTEXT,0,0,0,0, nullptr, ap_GetState_PointInTable, nullptr);
	_s(AP_MENU_ID_TABLE_HEADING_ROWS_REPEAT,0,0,0,0, nullptr, ap_GetState_PointInTable, nullptr);
	_s(AP_MENU_ID_TABLE_HEADING_ROWS_REPEAT_THIS,0,0,0,0, "repeatThisRow", ap_GetState_PointInTable, nullptr);
	_s(AP_MENU_ID_TABLE_HEADING_ROWS_REPEAT_REMOVE,0,0,0,0, "removeThisRowRepeat", ap_GetState_InTableIsRepeat, nullptr);
	_s(AP_MENU_ID_TABLE_SORT,0,0,0,0, nullptr, ap_GetState_PointOrAnchorInTable, nullptr);
	_s(AP_MENU_ID_TABLE_TEXTTOTABLE,0,0,0,0, nullptr, ap_GetState_TextToTableOK, nullptr);


	_s(AP_MENU_ID_RDF,1,0,0,0,nullptr,nullptr,nullptr);
	_s(AP_MENU_ID_RDF_HIGHLIGHT, 0,0,1,0, "toggleRDFAnchorHighlight",	ap_GetState_ToggleRDFAnchorHighlight,		nullptr);
	_s(AP_MENU_ID_RDF_QUERY, 0,1,0,0, "rdfQuery", nullptr, nullptr);
	_s(AP_MENU_ID_RDF_EDITOR, 0,1,0,0, "rdfEditor", nullptr, nullptr);
	_s(AP_MENU_ID_RDF_QUERY_XMLIDS, 0,1,0,0, "rdfQueryXMLIDs", nullptr, nullptr);
    _s(AP_MENU_ID_RDF_SEMITEM,              1,0,0,0,nullptr,nullptr,nullptr);
    _s(AP_MENU_ID_RDF_SEMITEM_CREATE,       1,0,0,0,nullptr,nullptr,nullptr);
	_s(AP_MENU_ID_RDF_SEMITEM_CREATEREF,    0,1,0,0,"rdfInsertRef",ap_GetState_RDF_Query,nullptr);
    _s(AP_MENU_ID_RDF_SEMITEM_NEW,          1,0,0,0,nullptr,nullptr,nullptr);
    _s(AP_MENU_ID_RDF_SEMITEM_NEW_CONTACT,  0,0,0,0,"rdfInsertNewContact",nullptr,nullptr);
    _s(AP_MENU_ID_RDF_SEMITEM_NEW_CONTACT_FROM_FILE,  0,1,0,0,"rdfInsertNewContactFromFile",ap_GetState_RDF_Contact,nullptr);
#ifdef DEBUG
	_s(AP_MENU_ID_RDF_ADV,1,0,0,0,nullptr,nullptr,nullptr);
	_s(AP_MENU_ID_RDF_ADV_DUMP_FOR_POINT,0,0,0,0,"dumpRDFForPoint",nullptr,nullptr);
	_s(AP_MENU_ID_RDF_ADV_DUMP_OBJECTS  ,0,0,0,0,"dumpRDFObjects",nullptr,nullptr);
	_s(AP_MENU_ID_RDF_ADV_TEST          ,0,0,0,0,"rdfTest",nullptr,nullptr);
	_s(AP_MENU_ID_RDF_ADV_PLAY          ,0,0,0,0,"rdfPlay",nullptr,nullptr);
#endif
    _s(AP_MENU_ID_RDF_SEMITEM_RELATION,          1,0,0,0,nullptr,nullptr,nullptr);
    _s(AP_MENU_ID_RDF_SEMITEM_SET_AS_SOURCE,     0,0,0,0,"rdfSemitemSetAsSource",nullptr,nullptr);
    _s(AP_MENU_ID_RDF_SEMITEM_RELATED_TO_SOURCE, 1,0,0,0,nullptr,nullptr,nullptr);
    _s(AP_MENU_ID_RDF_SEMITEM_RELATED_TO_SOURCE_FOAFKNOWS, 0,0,0,0,"rdfSemitemRelatedToSourceFoafKnows",nullptr,nullptr);
    _s(AP_MENU_ID_RDF_SEMITEM_FIND_RELATED,           1,0,0,0,nullptr,nullptr,nullptr);
    _s(AP_MENU_ID_RDF_SEMITEM_FIND_RELATED_FOAFKNOWS, 0,0,0,0,"rdfSemitemFindRelatedFoafKnows",nullptr,nullptr);
    _s(AP_MENU_ID_RDF_SEMITEM_STYLESHEET,          1,0,0,0,nullptr,nullptr,nullptr);
    _s(AP_MENU_ID_RDF_SEMITEM_STYLESHEET_APPLY,       0,0,0,0,"rdfApplyCurrentStyleSheet",nullptr,nullptr);
    _s(AP_MENU_ID_RDF_SEMITEM_STYLESHEET_DISASSOCIATE,  0,0,0,0,"rdfDisassocateCurrentStyleSheet",nullptr,nullptr);
    _s(AP_MENU_ID_RDF_SEMITEM_STYLESHEET_CONTACT,  1,0,0,0,nullptr,nullptr,nullptr);
    _s(AP_MENU_ID_RDF_SEMITEM_STYLESHEET_CONTACT_NAME,                0,0,0,0,"rdfApplyStylesheetContactName",nullptr,nullptr);
    _s(AP_MENU_ID_RDF_SEMITEM_STYLESHEET_CONTACT_NICK,                0,0,0,0,"rdfApplyStylesheetContactNick",nullptr,nullptr);
    _s(AP_MENU_ID_RDF_SEMITEM_STYLESHEET_CONTACT_NAME_PHONE,          0,0,0,0,"rdfApplyStylesheetContactNamePhone",nullptr,nullptr);
    _s(AP_MENU_ID_RDF_SEMITEM_STYLESHEET_CONTACT_NICK_PHONE,          0,0,0,0,"rdfApplyStylesheetContactNickPhone",nullptr,nullptr);
    _s(AP_MENU_ID_RDF_SEMITEM_STYLESHEET_CONTACT_NAME_HOMEPAGE_PHONE, 0,0,0,0,"rdfApplyStylesheetContactNameHomepagePhone",nullptr,nullptr);
    _s(AP_MENU_ID_RDF_SEMITEM_STYLESHEET_EVENT,                        1,0,0,0,nullptr,nullptr,nullptr);
    _s(AP_MENU_ID_RDF_SEMITEM_STYLESHEET_EVENT_NAME,                   0,0,0,0,"rdfApplyStylesheetEventName",nullptr,nullptr);
    _s(AP_MENU_ID_RDF_SEMITEM_STYLESHEET_EVENT_SUMMARY,                0,0,0,0,"rdfApplyStylesheetEventSummary",nullptr,nullptr);
    _s(AP_MENU_ID_RDF_SEMITEM_STYLESHEET_EVENT_SUMMARY_LOCATION,       0,0,0,0,"rdfApplyStylesheetEventSummaryLocation",nullptr,nullptr);
    _s(AP_MENU_ID_RDF_SEMITEM_STYLESHEET_EVENT_SUMMARY_LOCATION_TIMES, 0,0,0,0,"rdfApplyStylesheetEventSummaryLocationTimes",nullptr,nullptr);
    _s(AP_MENU_ID_RDF_SEMITEM_STYLESHEET_EVENT_SUMMARY_TIMES,          0,0,0,0,"rdfApplyStylesheetEventSummaryTimes",nullptr,nullptr);
    _s(AP_MENU_ID_RDF_SEMITEM_STYLESHEET_LOCATION,         1,0,0,0,nullptr,nullptr,nullptr);
    _s(AP_MENU_ID_RDF_SEMITEM_STYLESHEET_LOCATION_NAME,    0,0,0,0,"rdfApplyStylesheetLocationName",nullptr,nullptr);
    _s(AP_MENU_ID_RDF_SEMITEM_STYLESHEET_LOCATION_NAME_LATLONG, 0,0,0,0,"rdfApplyStylesheetLocationLatLong",nullptr,nullptr);
    _s(AP_MENU_ID_RDF_SEMITEM_STYLESHEET_SETTINGS,         0,1,0,0,"rdfStylesheetSettings",nullptr,nullptr);

	_s(AP_MENU_ID_CONTEXT_REVISIONS_ACCEPT_REVISION, 0,0,0,0, "revisionAccept", ap_GetState_RevisionPresent,nullptr);
	_s(AP_MENU_ID_CONTEXT_REVISIONS_REJECT_REVISION, 0,0,0,0, "revisionReject", ap_GetState_RevisionPresent, nullptr);
	_s(AP_MENU_ID_CONTEXT_REVISIONS_FIND_NEXT, 0,0,0,0, "revisionFindNext", ap_GetState_HasRevisions,nullptr);
	_s(AP_MENU_ID_CONTEXT_REVISIONS_FIND_PREV, 0,0,0,0, "revisionFindPrev", ap_GetState_HasRevisions,nullptr);
	_s(AP_MENU_ID_TOOLS_REVISIONS_SET_VIEW_LEVEL, 0,1,0,0, "revisionSetViewLevel", ap_GetState_RevisionsSelectLevel, nullptr);
	_s(AP_MENU_ID_TOOLS_HISTORY,  1,0,0,0,  nullptr,               nullptr,                   nullptr);
	_s(AP_MENU_ID_TOOLS_HISTORY_SHOW, 0,1,0,0, "history", ap_GetState_History, nullptr);
	_s(AP_MENU_ID_TOOLS_HISTORY_PURGE, 0,0,0,0, /* wrong: */"purgeAllRevisions"/* should be: purgeAllHistory */, ap_GetState_HasRevisions, nullptr);

	_s(AP_MENU_ID_WINDOW,			1,0,0,0,	nullptr,				nullptr,					nullptr);
	_s(AP_MENU_ID_WINDOW_NEW,		0,0,0,0,	"newWindow",		nullptr,					nullptr);
	_s(AP_MENU_ID_WINDOW_1,			0,0,0,0,	"activateWindow_1",	ap_GetState_Window,		ap_GetLabel_Window);
	_s(AP_MENU_ID_WINDOW_2,			0,0,0,0,	"activateWindow_2",	ap_GetState_Window,		ap_GetLabel_Window);
	_s(AP_MENU_ID_WINDOW_3,			0,0,0,0,	"activateWindow_3",	ap_GetState_Window,		ap_GetLabel_Window);
	_s(AP_MENU_ID_WINDOW_4,			0,0,0,0,	"activateWindow_4",	ap_GetState_Window,		ap_GetLabel_Window);
	_s(AP_MENU_ID_WINDOW_5,			0,0,0,0,	"activateWindow_5",	ap_GetState_Window,		ap_GetLabel_Window);
	_s(AP_MENU_ID_WINDOW_6,			0,0,0,0,	"activateWindow_6",	ap_GetState_Window,		ap_GetLabel_Window);
	_s(AP_MENU_ID_WINDOW_7,			0,0,0,0,	"activateWindow_7",	ap_GetState_Window,		ap_GetLabel_Window);
	_s(AP_MENU_ID_WINDOW_8,			0,0,0,0,	"activateWindow_8",	ap_GetState_Window,		ap_GetLabel_Window);
	_s(AP_MENU_ID_WINDOW_9,			0,0,0,0,	"activateWindow_9",	ap_GetState_Window,		ap_GetLabel_Window);
	_s(AP_MENU_ID_WINDOW_MORE,		0,1,0,0,	"dlgMoreWindows",	nullptr,					ap_GetLabel_WindowMore);

	_s(AP_MENU_ID_WEB_SAVEASWEB, 0,1,0,0, "fileSaveAsWeb", nullptr, nullptr);
	_s(AP_MENU_ID_WEB_WEBPREVIEW, 0,0,0,0, "filePreviewWeb", nullptr, nullptr);

	_s(AP_MENU_ID_HELP,				1,0,0,0,	nullptr,				nullptr,					nullptr);
	_s(AP_MENU_ID_HELP_CONTENTS,		0,0,0,0,	"helpContents",			nullptr,					ap_GetLabel_Contents);
	_s(AP_MENU_ID_HELP_INTRO,		0,0,0,0,	"helpIntro",			nullptr,					ap_GetLabel_Intro);
	_s(AP_MENU_ID_HELP_CHECKVER,		0,0,0,0,	"helpCheckVer",			nullptr,					ap_GetLabel_Checkver);
	_s(AP_MENU_ID_HELP_SEARCH,		0,0,0,0,	"helpSearch",			nullptr,					ap_GetLabel_Search);
	_s(AP_MENU_ID_HELP_ABOUT,		0,1,0,0,	"dlgAbout",			nullptr,					ap_GetLabel_About);
	_s(AP_MENU_ID_HELP_CREDITS, 0,0,0,0, "helpCredits", nullptr, nullptr);
	_s(AP_MENU_ID_HELP_REPORT_BUG, 0,0,0,0, "helpReportBug", nullptr, nullptr);

#ifdef ENABLE_SPELL
	_s(AP_MENU_ID_SPELL_SUGGEST_1,	0,0,0,0,	"spellSuggest_1",	ap_GetState_Suggest,	ap_GetLabel_Suggest);
	_s(AP_MENU_ID_SPELL_SUGGEST_2,	0,0,0,0,	"spellSuggest_2",	ap_GetState_Suggest,	ap_GetLabel_Suggest);
	_s(AP_MENU_ID_SPELL_SUGGEST_3,	0,0,0,0,	"spellSuggest_3",	ap_GetState_Suggest,	ap_GetLabel_Suggest);
	_s(AP_MENU_ID_SPELL_SUGGEST_4,	0,0,0,0,	"spellSuggest_4",	ap_GetState_Suggest,	ap_GetLabel_Suggest);
	_s(AP_MENU_ID_SPELL_SUGGEST_5,	0,0,0,0,	"spellSuggest_5",	ap_GetState_Suggest,	ap_GetLabel_Suggest);
	_s(AP_MENU_ID_SPELL_SUGGEST_6,	0,0,0,0,	"spellSuggest_6",	ap_GetState_Suggest,	ap_GetLabel_Suggest);
	_s(AP_MENU_ID_SPELL_SUGGEST_7,	0,0,0,0,	"spellSuggest_7",	ap_GetState_Suggest,	ap_GetLabel_Suggest);
	_s(AP_MENU_ID_SPELL_SUGGEST_8,	0,0,0,0,	"spellSuggest_8",	ap_GetState_Suggest,	ap_GetLabel_Suggest);
	_s(AP_MENU_ID_SPELL_SUGGEST_9,	0,0,0,0,	"spellSuggest_9",	ap_GetState_Suggest,	ap_GetLabel_Suggest);
	_s(AP_MENU_ID_SPELL_IGNOREALL,	0,0,0,0,	"spellIgnoreAll",	nullptr,					nullptr);
	_s(AP_MENU_ID_SPELL_ADD,		0,0,0,0,	"spellAdd",			nullptr,					nullptr);
#endif
	// ... add others here ...

	_s(AP_MENU_ID__BOGUS2__,		0,0,0,0,	nullptr,				nullptr,					nullptr);

#undef _s

	return pActionSet;
}
