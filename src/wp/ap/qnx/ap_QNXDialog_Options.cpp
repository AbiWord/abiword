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
#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "gr_QNXGraphics.h"

#include "xap_App.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrameImpl.h"
#include "xap_Frame.h"
#include "xap_Prefs.h"

#include "ap_Dialog_Id.h"
#include "ap_Prefs_SchemeIds.h"

#include "ap_Strings.h"

#include "ap_QNXDialog_Options.h"
#include "ut_qnxHelper.h"

/*****************************************************************/
int AP_QNXDialog_Options::s_ok_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Options * dlg = (AP_QNXDialog_Options *)data;
	UT_ASSERT(dlg);
	dlg->event_OK();
	return Pt_CONTINUE;
}

int AP_QNXDialog_Options::s_cancel_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Options * dlg = (AP_QNXDialog_Options *)data;
	UT_ASSERT(widget && dlg);
	dlg->event_Cancel();
	return Pt_CONTINUE;
}

int AP_QNXDialog_Options::s_apply_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Options * dlg = (AP_QNXDialog_Options *)data;
	UT_ASSERT(widget && dlg);
	dlg->event_Apply();
	return Pt_CONTINUE;
}

int AP_QNXDialog_Options::s_delete_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Options * dlg = (AP_QNXDialog_Options *)data;
	UT_ASSERT(dlg);
	dlg->event_WindowDelete();
	return Pt_CONTINUE;
}

int AP_QNXDialog_Options::s_defaults_clicked(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Options * dlg = (AP_QNXDialog_Options *)data;
	UT_ASSERT(widget && dlg);
	dlg->_event_SetDefaults();
	return Pt_CONTINUE;
}


// these function will allow multiple widget to tie into the same logic
// function (at the AP level) to enable/disable stuff
int AP_QNXDialog_Options::s_checkbutton_toggle(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
#if 0
	AP_QNXDialog_Options * dlg = (AP_QNXDialog_Options *)data;
	UT_ASSERT(dlg);
	UT_ASSERT( w && GTK_IS_WIDGET(w));
	int i = (int) g_object_get_data( G_OBJECT(w), "tControl" );
	dlg->_enableDisableLogic( (AP_Dialog_Options::tControl) i );
#endif
	return Pt_CONTINUE;
}

int AP_QNXDialog_Options::s_menu_item_activate(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
#if 0
	AP_QNXDialog_Options * dlg = (AP_QNXDialog_Options *)data;

	UT_ASSERT(widget && dlg);

	PtWidget_t *option_menu = (PtWidget_t *)g_object_get_data(G_OBJECT(widget),
												 WIDGET_MENU_OPTION_PTR);
	UT_ASSERT( option_menu && GTK_IS_OPTION_MENU(option_menu));

	void * p = g_object_get_data( G_OBJECT(widget),
												WIDGET_MENU_VALUE_TAG);

	g_object_set_data( G_OBJECT(option_menu), WIDGET_MENU_VALUE_TAG, p );

	UT_DEBUGMSG(("s_menu_item_activate [%d %s]\n", p, UT_dimensionName( (UT_Dimension)((UT_uint32)p)) ) );
#endif
	return Pt_CONTINUE;
}


int AP_QNXDialog_Options::s_chooseTransparentColor(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Options * dlg = (AP_QNXDialog_Options *)data;
	UT_ASSERT(widget && dlg);
	dlg->event_ChooseTransparentColor();
	return Pt_CONTINUE;
}


int AP_QNXDialog_Options::s_allowTransparentColor(PtWidget_t *widget, void *data, PtCallbackInfo_t *info)
{
	AP_QNXDialog_Options * dlg = (AP_QNXDialog_Options *)data;
	UT_ASSERT(widget && dlg);
	dlg->event_AllowTransparentColor();
	return Pt_CONTINUE;
}

bool AP_QNXDialog_Options::_gatherAllowCustomToolbars(void)
{
	return false;
}

void AP_QNXDialog_Options::_setAllowCustomToolbars(const bool b)
{
}

// TODO:  Add these item to dialog and impliment
bool AP_QNXDialog_Options::_gatherAutoLoadPlugins(void)
{
	return false;
}

void AP_QNXDialog_Options::_setAutoLoadPlugins(const bool b)
{
}


/*****************************************************************/

#define TR(str) UT_XML_transNoAmpersands((str))

XAP_Dialog * AP_QNXDialog_Options::static_constructor(XAP_DialogFactory * pFactory,
                                                         XAP_Dialog_Id id)
{
    AP_QNXDialog_Options * p = new AP_QNXDialog_Options(pFactory,id);
    return p;
}

AP_QNXDialog_Options::AP_QNXDialog_Options(XAP_DialogFactory * pDlgFactory,
                                                 XAP_Dialog_Id id)
    : AP_Dialog_Options(pDlgFactory,id)
{
	/* DEBUG stuff */
	XAP_Prefs *prefs = m_pApp->getPrefs();
	UT_ASSERT(prefs);
	UT_DEBUGMSG(("AP_QNXDialog_Options::AP_QNXDialog_Options[%s:%d]\n", __FILE__, __LINE__));
	UT_DEBUGMSG(("    current pref : %s\n",
		prefs->getCurrentScheme()->getSchemeName()) );

	bool b;
	b = prefs->savePrefsFile();
	UT_DEBUGMSG(("    prefs saved (%d) in %s\n", b, prefs->getPrefsPathname() ));

	UT_uint32 i;
	XAP_PrefsScheme *ps;
	for ( i = 0; (ps = prefs->getNthScheme(i)) != 0; i++ ) {
		UT_DEBUGMSG(("    %d [%s]\n", i, ps->getSchemeName() ));

		gchar const *pszKey, *pszValue;
		for ( UT_uint32 j = 0; ps->getNthValue(j, &pszKey, &pszValue ); j++ ) {
			UT_DEBUGMSG(("        %x %-30s : %s\n", j, pszKey, pszValue ));
		}
	}
}

AP_QNXDialog_Options::~AP_QNXDialog_Options(void)
{
}

/*****************************************************************/

void AP_QNXDialog_Options::runModal(XAP_Frame * pFrame)
{
	XAP_QNXFrameImpl * pQNXFrameImpl = (XAP_QNXFrameImpl*)pFrame->getFrameImpl();
	PtWidget_t *parentWindow =	pQNXFrameImpl->getTopLevelWindow();	
	UT_ASSERT(parentWindow);

	PtSetParentWidget(parentWindow);
    PtWidget_t * mainWindow = _constructWindow();
    UT_ASSERT(mainWindow);

	// save for use with event
	m_pFrame = pFrame;

    // Populate the window's data items
    _populateWindowData();

    // Center our new dialog in its parent and make it a transient
    // so it won't get lost underneath
	UT_QNXCenterWindow(parentWindow, mainWindow);
	UT_QNXBlockWidget(parentWindow, 1);

	PtRealizeWidget(mainWindow);

one_more_time:
	int count;
	count = PtModalStart();
	done = 0;
	while(!done) {
		PtProcessEvent();
	}
	PtModalEnd(MODAL_END_ARG(count));

	switch ( m_answer ) {
	case AP_Dialog_Options::a_OK:
		_storeWindowData();
		break;

	case AP_Dialog_Options::a_APPLY:
		UT_DEBUGMSG(("Applying changes\n"));
		_storeWindowData();
		goto one_more_time;
		break;

	case AP_Dialog_Options::a_CANCEL:
		break;

	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		break;
	};

	UT_QNXBlockWidget(parentWindow, 0);
	PtDestroyWidget(mainWindow);
}

void AP_QNXDialog_Options::event_OK(void)
{
	if (!done++)
	    m_answer = AP_Dialog_Options::a_OK;
}

void AP_QNXDialog_Options::event_Cancel(void)
{
	if (!done++)
	    m_answer = AP_Dialog_Options::a_CANCEL;
}

void AP_QNXDialog_Options::event_Apply(void)
{
	if (!done++)
	    m_answer = AP_Dialog_Options::a_APPLY;
}

void AP_QNXDialog_Options::event_WindowDelete(void)
{
	if (!done++)
	    m_answer = AP_Dialog_Options::a_CANCEL;
}

void AP_QNXDialog_Options::event_AllowTransparentColor(void)
{
	strcpy(m_CurrentTransparentColor, (const gchar *)"ffffff");
}

void AP_QNXDialog_Options::event_ChooseTransparentColor(void)
{
	//Do the colour selection ...
}


void AP_QNXDialog_Options::initializeTransperentToggle(void)
{
	PtSetResource(m_wCheckWhiteTransparent, Pt_ARG_FLAGS, Pt_SET, Pt_SET);
}

/*****************************************************************/
PtWidget_t* AP_QNXDialog_Options::_constructWindow ()
{
	// for the internationalization
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	UT_UTF8String s;

	PtWidget_t *windowOptions;
	PtWidget_t *buttonSave;
	PtWidget_t *buttonDefaults;
	PtWidget_t *buttonApply;
	PtWidget_t *buttonOk;
	PtWidget_t *buttonCancel;
	PtWidget_t *notebook1;
	PtWidget_t *checkbuttonSpellHideErrors;
	PtWidget_t *checkbuttonSpellSuggest;
	PtWidget_t *checkbuttonSpellMainOnly;
	PtWidget_t *checkbuttonSpellUppercase;
	PtWidget_t *checkbuttonSpellNumbers;
	PtWidget_t *label4;
	PtWidget_t *checkbuttonSpellCheckAsType;
	PtWidget_t *buttonSpellIgnoreReset;
	PtWidget_t *buttonSpellIgnoreEdit;
	PtWidget_t *buttonSpellDictionary;
	PtWidget_t *label5;
	PtWidget_t *listSpellDicts;
	PtWidget_t *listSpellDicts_menu;
	PtWidget_t *checkbuttonPrefsAutoSave;
	PtWidget_t *label6;
	PtWidget_t *comboPrefsSchemes;
	PtWidget_t *checkbuttonViewRuler;
	PtWidget_t *labelUnits;
	PtWidget_t *listViewRulerUnit;
	PtWidget_t *checkbuttonViewCursorBlink;
	PtWidget_t *checkbuttonViewStatusBar;
	PtWidget_t *checkbuttonViewAll;
	PtWidget_t *checkbuttonViewHidden;
	PtWidget_t *checkbuttonViewUnprintable;
	PtWidget_t *checkbuttonOtherDirectionRtl;
	PtWidget_t *checkbuttonOtherHebrewContextGlyphs;




	PtArg_t args[10];
	const char *item;
	int		n;

#define WIN_WIDTH  450
#define WIN_HEIGHT 350
	n = 0;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Options_OptionsTitle,s);
	PtSetArg(&args[n++], Pt_ARG_WINDOW_TITLE, s.utf8_str(), 0);
	PtSetArg(&args[n++], Pt_ARG_WINDOW_RENDER_FLAGS, 0, ABI_MODAL_WINDOW_RENDER_FLAGS);
	PtSetArg(&args[n++], Pt_ARG_WINDOW_MANAGED_FLAGS, 0, ABI_MODAL_WINDOW_MANAGE_FLAGS);
	windowOptions = PtCreateWidget(PtWindow, NULL, n, args);
	SetupContextHelp(windowOptions,this);
	PtAddHotkeyHandler(windowOptions,Pk_F1,0,Pt_HOTKEY_SYM,this,OpenHelp);

	PtAddCallback(windowOptions, Pt_CB_WINDOW_CLOSING, s_delete_clicked, this);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_WIDTH, ABI_MODAL_MARGIN_SIZE, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_HEIGHT, ABI_MODAL_MARGIN_SIZE, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_HORZ_ALIGN, Pt_GROUP_HORZ_CENTER, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_SPACING_Y, 10, 0);
	PtWidget_t *vallgroup = PtCreateWidget(PtGroup, windowOptions, n, args);

	n = 0;
	PhPoint_t pos;
#define PANEL_WIDTH (WIN_WIDTH - 0)
#define PANEL_HEIGHT (WIN_HEIGHT - 50)
	pos.x = (WIN_WIDTH - PANEL_WIDTH) / 2;
	pos.y = (WIN_HEIGHT - PANEL_HEIGHT - 30) / 2;
	PtSetArg(&args[n++], Pt_ARG_POS, &pos, 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, PANEL_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_HEIGHT, PANEL_HEIGHT, 0);
	PtWidget_t *panelGroup = PtCreateWidget(PtPanelGroup, vallgroup, n, args);

#define TAB_WIDTH  (PANEL_WIDTH - 30)
#define TAB_HEIGHT (PANEL_HEIGHT - 50)
	/*** Spelling Tab ***/
	n = 0;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Options_TabLabel_Spelling,s);
PtSetArg(&args[n++], Pt_ARG_TITLE, TR(s.utf8_str()), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, TAB_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_HEIGHT, TAB_HEIGHT, 0);
	PtWidget_t *spellingTab = PtCreateWidget(PtPane, panelGroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_WIDTH, ABI_MODAL_MARGIN_SIZE, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_HEIGHT, ABI_MODAL_MARGIN_SIZE, 0);
	PtWidget_t *vspellgroup = PtCreateWidget(PtGroup, spellingTab, n, args);

	n = 0;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_SpellHideErrors,s);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, TR(s.utf8_str()), 0);
	checkbuttonSpellHideErrors = PtCreateWidget(PtToggleButton, vspellgroup, n, args);

	n = 0;
	pSS->getValueUTF8( AP_STRING_ID_DLG_Options_Label_SpellSuggest,s),
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, TR(s.utf8_str()), 0);
	checkbuttonSpellSuggest = PtCreateWidget(PtToggleButton, vspellgroup, n, args);

	n = 0;
	pSS->getValueUTF8( AP_STRING_ID_DLG_Options_Label_SpellMainOnly,s);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, TR(s.utf8_str()), 0);
	checkbuttonSpellMainOnly = PtCreateWidget(PtToggleButton, vspellgroup, n, args);

	n = 0;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_SpellUppercase,s);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, TR(s.utf8_str()), 0);
	checkbuttonSpellUppercase = PtCreateWidget(PtToggleButton, vspellgroup, n, args);

	n = 0;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_SpellNumbers,s);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, TR(s.utf8_str()), 0);
	checkbuttonSpellNumbers = PtCreateWidget(PtToggleButton, vspellgroup, n, args);

	n = 0;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_SpellCheckAsType,s);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, TR(s.utf8_str()), 0);
	checkbuttonSpellCheckAsType = PtCreateWidget(PtToggleButton, vspellgroup, n, args);

	/* Align these items horizontally */
	n = 0;
//	PtSetArg(&args[n++], Pt_ARG_WIDTH, TAB_WIDTH, 0);
	PtWidget_t *hcustomdict = PtCreateWidget(PtGroup, vspellgroup, n, args);

	n = 0;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_SpellCustomDict,s);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, TR(s.utf8_str()), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH,  3 * ABI_DEFAULT_BUTTON_WIDTH, 0);
	label5 = PtCreateWidget(PtLabel, hcustomdict, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH,  ABI_DEFAULT_BUTTON_WIDTH, 0);
	listSpellDicts = PtCreateWidget(PtComboBox, hcustomdict, n, args);
	const char *items1[] = { "custom.dic" };
	PtListAddItems(listSpellDicts, items1, 1, 0);
	UT_QNXComboSetPos(listSpellDicts, 1);

	n = 0;
	pSS->getValueUTF8( AP_STRING_ID_DLG_Options_Btn_CustomDict,s);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, TR(s.utf8_str()), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH,  ABI_DEFAULT_BUTTON_WIDTH, 0);
	buttonSpellDictionary = PtCreateWidget(PtButton, hcustomdict, n, args);

	/* Align these items horizontally */
	n = 0;
//	PtSetArg(&args[n++], Pt_ARG_WIDTH, TAB_WIDTH, 0);
	PtWidget_t *hignorewords = PtCreateWidget(PtGroup, vspellgroup, n, args);

	n = 0;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_SpellIgnoredWord,s);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, TR(s.utf8_str()), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH,  3 * ABI_DEFAULT_BUTTON_WIDTH, 0);
	label4 = PtCreateWidget(PtLabel, hignorewords, n, args);

	n = 0;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Btn_IgnoreReset,s);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, TR(s.utf8_str()), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH,  ABI_DEFAULT_BUTTON_WIDTH, 0);
	buttonSpellIgnoreReset = PtCreateWidget(PtButton, hignorewords, n, args);

	n = 0;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Btn_IgnoreEdit,s);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, TR(s.utf8_str()), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH,  ABI_DEFAULT_BUTTON_WIDTH, 0);
	buttonSpellIgnoreEdit = PtCreateWidget(PtButton, hignorewords, n, args);

	/*** Preferences Tab ***/
	n = 0;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Options_TabLabel_Preferences,s);
	PtSetArg(&args[n++], Pt_ARG_TITLE, TR(s.utf8_str()), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, TAB_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_HEIGHT, TAB_HEIGHT, 0);
	PtWidget_t *prefTab = PtCreateWidget(PtPane, panelGroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_WIDTH, ABI_MODAL_MARGIN_SIZE, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_HEIGHT, ABI_MODAL_MARGIN_SIZE, 0);
	PtWidget_t *vprefgroup = PtCreateWidget(PtGroup, prefTab, n, args);

	n = 0;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_PrefsAutoSave,s);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, TR(s.utf8_str()), 0);
	checkbuttonPrefsAutoSave = PtCreateWidget(PtToggleButton, vprefgroup, n, args);

 	/** Group these together horizontally **/
	n = 0;
//	PtSetArg(&args[n++], Pt_ARG_WIDTH, TAB_WIDTH, 0);
	PtWidget_t *hprefscheme = PtCreateWidget(PtGroup, vprefgroup, n, args);

	n = 0;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_PrefsCurrentScheme,s);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, TR(s.utf8_str()), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH,  3 * ABI_DEFAULT_BUTTON_WIDTH, 0);
	label6 = PtCreateWidget(PtLabel, hprefscheme, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH,  ABI_DEFAULT_BUTTON_WIDTH, 0);
	comboPrefsSchemes = PtCreateWidget(PtComboBox, hprefscheme, n, args);

	/*** View Tab ***/
	n = 0;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Options_TabLabel_View,s);
	PtSetArg(&args[n++], Pt_ARG_TITLE, TR(s.utf8_str()), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, TAB_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_HEIGHT, TAB_HEIGHT, 0);
	PtWidget_t *viewTab = PtCreateWidget(PtPane, panelGroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_WIDTH, ABI_MODAL_MARGIN_SIZE, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_HEIGHT, ABI_MODAL_MARGIN_SIZE, 0);
	PtWidget_t *vviewgroup = PtCreateWidget(PtGroup, viewTab, n, args);

	/** View View Show/Hide **/
	n = 0;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_ViewShowHide,s);
	PtSetArg(&args[n++], Pt_ARG_TITLE, TR(s.utf8_str()), 0);
	PtSetArg(&args[n++], Pt_ARG_CONTAINER_FLAGS, Pt_SHOW_TITLE | Pt_ETCH_TITLE_AREA,
												 Pt_SHOW_TITLE | Pt_ETCH_TITLE_AREA);
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
//	PtSetArg(&args[n++], Pt_ARG_WIDTH, TAB_WIDTH, 0);
//	PtSetArg(&args[n++], Pt_ARG_HEIGHT, TAB_HEIGHT / 2, 0);
	PtWidget_t *vshowgroup = PtCreateWidget (PtGroup, vviewgroup, n, args);

	n = 0;
//	PtSetArg(&args[n++], Pt_ARG_WIDTH, TAB_WIDTH, 0);
	PtWidget_t *hrulergroup = PtCreateWidget(PtGroup, vshowgroup, n, args);

	n = 0;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_ViewRuler,s);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, TR(s.utf8_str()), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH,  2 * ABI_DEFAULT_BUTTON_WIDTH, 0);
	checkbuttonViewRuler = PtCreateWidget(PtToggleButton, hrulergroup, n, args);

	n = 0;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_ViewUnits,s);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, TR(s.utf8_str()), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH,  ABI_DEFAULT_BUTTON_WIDTH, 0);
	labelUnits = PtCreateWidget(PtLabel, hrulergroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH,  ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_TEXT_FLAGS,Pt_FALSE,Pt_EDITABLE);
	listViewRulerUnit = PtCreateWidget(PtComboBox, hrulergroup, n, args);
	//Populate the list with the units and set the vector
    m_vecUnits.clear();
	pSS->getValueUTF8(XAP_STRING_ID_DLG_Unit_inch,s);
	item = s.utf8_str();
	PtListAddItems(listViewRulerUnit, &item, 1, 0);
    m_vecUnits.addItem((void *)DIM_IN);
	pSS->getValueUTF8(XAP_STRING_ID_DLG_Unit_cm,s);
	item = s.utf8_str();
	PtListAddItems(listViewRulerUnit, &item, 1, 0);
    m_vecUnits.addItem((void *)DIM_CM);
	pSS->getValueUTF8(XAP_STRING_ID_DLG_Unit_points,s);
	item = s.utf8_str();
	PtListAddItems(listViewRulerUnit, &item, 1, 0);
    m_vecUnits.addItem((void *)DIM_PT);
	pSS->getValueUTF8(XAP_STRING_ID_DLG_Unit_pica,s);
	item = s.utf8_str();
	PtListAddItems(listViewRulerUnit, &item, 1, 0);
    m_vecUnits.addItem((void *)DIM_PI);

	/*
	TODO: Page Size
	*/

	n = 0;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_ViewStatusBar,s);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, TR(s.utf8_str()), 0);
	checkbuttonViewStatusBar = PtCreateWidget(PtToggleButton, vshowgroup, n, args);

	n = 0;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_ViewCursorBlink,s);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, TR(s.utf8_str()), 0);
	checkbuttonViewCursorBlink = PtCreateWidget(PtToggleButton, vshowgroup, n, args);

	/** View View Frame **/
	n = 0;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_ViewViewFrame,s);
	PtSetArg(&args[n++], Pt_ARG_TITLE, TR(s.utf8_str()), 0);
	PtSetArg(&args[n++], Pt_ARG_CONTAINER_FLAGS, Pt_SHOW_TITLE | Pt_ETCH_TITLE_AREA,
												 Pt_SHOW_TITLE | Pt_ETCH_TITLE_AREA);
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
//	PtSetArg(&args[n++], Pt_ARG_WIDTH, TAB_WIDTH, 0);
//	PtSetArg(&args[n++], Pt_ARG_HEIGHT, TAB_HEIGHT / 2, 0);
	PtWidget_t *vviewviewgroup = PtCreateWidget (PtGroup, vviewgroup, n, args);

	n = 0;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_ViewAll,s);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, TR(s.utf8_str()), 0);
	checkbuttonViewAll = PtCreateWidget(PtToggleButton, vviewviewgroup, n, args);

	n = 0;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_ViewHiddenText,s);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, TR(s.utf8_str()), 0);
	checkbuttonViewHidden = PtCreateWidget(PtToggleButton, vviewviewgroup, n, args);

	n = 0;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_ViewUnprintable,s);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, TR(s.utf8_str()), 0);
	checkbuttonViewUnprintable = PtCreateWidget(PtToggleButton, vviewviewgroup, n, args);

	/*** Other/Misc Tab (smart quotes) ***/
	n = 0;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Options_TabLabel_Other,s);
	PtSetArg(&args[n++], Pt_ARG_TITLE, TR(s.utf8_str()), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH, TAB_WIDTH, 0);
	PtSetArg(&args[n++], Pt_ARG_HEIGHT, TAB_HEIGHT, 0);
	PtWidget_t *miscTab = PtCreateWidget(PtPane, panelGroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_ORIENTATION, Pt_GROUP_VERTICAL, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_WIDTH, ABI_MODAL_MARGIN_SIZE, 0);
	PtSetArg(&args[n++], Pt_ARG_MARGIN_HEIGHT, ABI_MODAL_MARGIN_SIZE, 0);
	PtWidget_t *vmiscgroup = PtCreateWidget(PtGroup, miscTab, n, args);

	/** SmartQuotes Enable **/
	n = 0;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_SmartQuotesEnable,s);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, TR(s.utf8_str()), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH,  2 * ABI_DEFAULT_BUTTON_WIDTH, 0);
	m_checkbuttonSmartQuotesEnable = PtCreateWidget(PtToggleButton, vmiscgroup, n, args);

	/** Autosave Options **/
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_SPACING_X, 5, 0);
	PtWidget_t *hautosavegroup = PtCreateWidget (PtGroup, vmiscgroup, n, args);

	n = 0;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_AutoSaveCurrent,s);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING,TR(s.utf8_str()), 0);
	m_checkbuttonAutoSaveFile = PtCreateWidget(PtToggleButton, hautosavegroup, n, args);

	n = 0;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_Minutes,s);
	PtSetArg(&args[n++], Pt_ARG_NUMERIC_SUFFIX,TR(s.utf8_str()), 0);
	PtSetArg(&args[n++], Pt_ARG_NUMERIC_MIN, 0, 0);
	PtSetArg(&args[n++], Pt_ARG_NUMERIC_MAX, 60, 0);
	m_wAutoSaveFilePeriod = PtCreateWidget(PtNumericInteger, hautosavegroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_SPACING_X, 5, 0);
	hautosavegroup = PtCreateWidget (PtGroup, vmiscgroup, n, args);

	n = 0;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_WithExtension,s);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING,TR(s.utf8_str()), 0);
	PtCreateWidget(PtLabel, hautosavegroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_WIDTH,  ABI_DEFAULT_BUTTON_WIDTH, 0);
	m_wAutoSaveFileExt = PtCreateWidget(PtText, hautosavegroup, n, args);

	/** Colour Selector Options **/
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_SPACING_X, 5, 0);
	PtSetArg(&args[n++], Pt_ARG_GROUP_FLAGS, Pt_GROUP_EQUAL_SIZE_VERTICAL, Pt_GROUP_EQUAL_SIZE_VERTICAL);
	PtWidget_t *hcolorgroup = PtCreateWidget (PtGroup, vmiscgroup, n, args);

	n = 0;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_ChooseForTransparent,s);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING,s.utf8_str(), 0);
	PtCreateWidget(PtLabel, hcolorgroup, n, args);

	n = 0;
	PtSetArg(&args[n++], Pt_ARG_FLAGS, Pt_TOGGLE, Pt_TOGGLE);
	PtSetArg(&args[n++], Pt_ARG_WIDTH,  ABI_DEFAULT_BUTTON_WIDTH, 0);
	m_wChooseColorForTransparent = PtCreateWidget(PtColorWell, hcolorgroup, n, args);
	//PtAddCallback(m_wCheckWhiteTransparent, Pt_CB_ACTIVATE, s_allowTransparentColor, this);

	n = 0;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_CheckWhiteForTransparent,s);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING,
TR(s.utf8_str()), 0);
	m_wCheckWhiteTransparent = PtCreateWidget(PtToggleButton, vmiscgroup, n, args);
	PtAddCallback(m_wCheckWhiteTransparent, Pt_CB_ACTIVATE, s_allowTransparentColor, this);

/*
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING,
TR(pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_BiDiOptions ).utf8_str(), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH,  2 * ABI_DEFAULT_BUTTON_WIDTH, 0);
	PtCreateWidget(PtGroup, vmiscgroup, n, args);
*/
	n = 0;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_DirectionRtl,s);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING,TR(s.utf8_str()), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH,  2 * ABI_DEFAULT_BUTTON_WIDTH, 0);
	checkbuttonOtherDirectionRtl = PtCreateWidget(PtToggleButton, vmiscgroup, n, args);

	n = 0;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Label_HebrewContextGlyphs,s);
	PtSetArg(&args[n++],Pt_ARG_TEXT_STRING,TR(s.utf8_str()),0);
	PtSetArg(&args[n++],Pt_ARG_WIDTH, 2* ABI_DEFAULT_BUTTON_WIDTH,0);
	checkbuttonOtherHebrewContextGlyphs=PtCreateWidget(PtToggleButton,vmiscgroup,n,args);


	/*** Create the horizontal button group ***/
	n = 0;
	PtSetArg(&args[n++], Pt_ARG_GROUP_SPACING_X, 5, 0);
	PtWidget_t *hbuttongroup = PtCreateWidget (PtGroup, vallgroup, n, args);

	n = 0;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Btn_Save,s);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, TR(s.utf8_str()), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH,  ABI_DEFAULT_BUTTON_WIDTH, 0);
	buttonSave = PtCreateWidget(PtButton, hbuttongroup, n, args);

	n = 0;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Btn_Apply,s);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, TR(s.utf8_str()), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH,  ABI_DEFAULT_BUTTON_WIDTH, 0);
	buttonApply = PtCreateWidget(PtButton, hbuttongroup, n, args);

	n = 0;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Options_Btn_Default,s);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, TR(s.utf8_str()), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH,  ABI_DEFAULT_BUTTON_WIDTH, 0);
	buttonDefaults = PtCreateWidget(PtButton, hbuttongroup, n, args);

	n = 0;
	pSS->getValueUTF8(XAP_STRING_ID_DLG_OK,s);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, TR(s.utf8_str()), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH,  ABI_DEFAULT_BUTTON_WIDTH, 0);
	buttonOk = PtCreateWidget(PtButton, hbuttongroup, n, args);

	n = 0;
	pSS->getValueUTF8(XAP_STRING_ID_DLG_Cancel,s);
	PtSetArg(&args[n++], Pt_ARG_TEXT_STRING, TR(s.utf8_str()), 0);
	PtSetArg(&args[n++], Pt_ARG_WIDTH,  ABI_DEFAULT_BUTTON_WIDTH, 0);
	buttonCancel = PtCreateWidget(PtButton, hbuttongroup, n, args);

	PtAddCallback(windowOptions, Pt_CB_WINDOW_CLOSING, s_delete_clicked, this);
	PtAddCallback(buttonOk, Pt_CB_ACTIVATE, s_ok_clicked, this);
	PtAddCallback(buttonCancel, Pt_CB_ACTIVATE, s_cancel_clicked, this);
	PtAddCallback(buttonDefaults, Pt_CB_ACTIVATE, s_defaults_clicked, this);
	PtAddCallback(buttonApply, Pt_CB_ACTIVATE, s_apply_clicked, this);

  m_windowMain = windowOptions;
  m_notebook = notebook1 = NULL;

  m_checkbuttonOtherDirectionRtl = checkbuttonOtherDirectionRtl;
  m_checkbuttonOtherHebrewContextGlyphs = checkbuttonOtherHebrewContextGlyphs;

  m_checkbuttonSpellCheckAsType	= checkbuttonSpellCheckAsType;
  m_checkbuttonSpellHideErrors	= checkbuttonSpellHideErrors;
  m_checkbuttonSpellSuggest		= checkbuttonSpellSuggest;
  m_checkbuttonSpellMainOnly		= checkbuttonSpellMainOnly;
  m_checkbuttonSpellUppercase		= checkbuttonSpellUppercase;
  m_checkbuttonSpellNumbers		= checkbuttonSpellNumbers;
  m_listSpellDicts				= listSpellDicts;
  m_listSpellDicts_menu			= listSpellDicts_menu = NULL;
  m_buttonSpellDictionary			= buttonSpellDictionary;
  m_buttonSpellIgnoreEdit			= buttonSpellIgnoreEdit;
  m_buttonSpellIgnoreReset		= buttonSpellIgnoreReset;

  m_checkbuttonPrefsAutoSave		= checkbuttonPrefsAutoSave;
  m_comboPrefsScheme				= comboPrefsSchemes;

  m_checkbuttonViewShowRuler		= checkbuttonViewRuler;
  m_listViewRulerUnits			= listViewRulerUnit;
  m_checkbuttonViewCursorBlink	= checkbuttonViewCursorBlink;
  m_checkbuttonViewShowStatusBar	= checkbuttonViewStatusBar;
  m_checkbuttonViewAll			= checkbuttonViewAll;
  m_checkbuttonViewHiddenText		= checkbuttonViewHidden;
  m_checkbuttonViewUnprintable	= checkbuttonViewUnprintable;

  m_buttonSave					= buttonSave;
  m_buttonDefaults				= buttonDefaults;
  m_buttonApply					= buttonApply;
  m_buttonOK						= buttonOk;
  m_buttonCancel					= buttonCancel;

    return windowOptions;
}

PtWidget_t *AP_QNXDialog_Options::_lookupWidget ( tControl id )
{
	switch (id)
	{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// spell
	case id_CHECK_SPELL_CHECK_AS_TYPE:
		return m_checkbuttonSpellCheckAsType;
		break;

	case id_CHECK_SPELL_HIDE_ERRORS:
		return m_checkbuttonSpellHideErrors;
		break;

	case id_CHECK_SPELL_SUGGEST:
		return m_checkbuttonSpellSuggest;
		break;

	case id_CHECK_SPELL_MAIN_ONLY:
		return m_checkbuttonSpellMainOnly;
		break;

	case id_CHECK_SPELL_UPPERCASE:
		return m_checkbuttonSpellUppercase;
		break;

	case id_CHECK_SPELL_NUMBERS:
		return m_checkbuttonSpellNumbers;
		break;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// smart quotes
	case id_CHECK_SMART_QUOTES_ENABLE:
		return m_checkbuttonSmartQuotesEnable;
		break;

/* TODO: Page Size
	case id_LIST_DEFAULT_PAGE_SIZE:
		return m_listDefaultPageSize;
*/
	case id_CHECK_AUTO_SAVE_FILE:
		return m_checkbuttonAutoSaveFile;

	case id_TEXT_AUTO_SAVE_FILE_EXT:
		return m_wAutoSaveFileExt;

	case id_TEXT_AUTO_SAVE_FILE_PERIOD:
		return m_wAutoSaveFilePeriod;

	case id_CHECK_COLOR_FOR_TRANSPARENT_IS_WHITE:
		return  m_wCheckWhiteTransparent;

	case id_PUSH_CHOOSE_COLOR_FOR_TRANSPARENT:
		return  m_wChooseColorForTransparent;

	case id_CHECK_OTHER_DEFAULT_DIRECTION_RTL:
		return m_checkbuttonOtherDirectionRtl;
	case id_CHECK_OTHER_HEBREW_CONTEXT_GLYPHS:
		return m_checkbuttonOtherHebrewContextGlyphs;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// prefs
	case id_CHECK_PREFS_AUTO_SAVE:
		return m_checkbuttonPrefsAutoSave;
		break;

	case id_COMBO_PREFS_SCHEME:
		return m_comboPrefsScheme;
		break;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// view
	case id_CHECK_VIEW_SHOW_RULER:
		return m_checkbuttonViewShowRuler;
		break;

	case id_LIST_VIEW_RULER_UNITS:
		return m_listViewRulerUnits;
		break;

	case id_CHECK_VIEW_CURSOR_BLINK:
		return m_checkbuttonViewCursorBlink;
		break;

	case id_CHECK_VIEW_SHOW_STATUS_BAR:
		return m_checkbuttonViewShowStatusBar;
		break;

	case id_CHECK_VIEW_ALL:
		return m_checkbuttonViewAll;
		break;

	case id_CHECK_VIEW_HIDDEN_TEXT:
		return m_checkbuttonViewHiddenText;
		break;

	case id_CHECK_VIEW_UNPRINTABLE:
		return m_checkbuttonViewUnprintable;
		break;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// general
	case id_BUTTON_SAVE:
		return m_buttonSave;
		break;

	case id_BUTTON_DEFAULTS:
		return m_buttonDefaults;
		break;

	case id_BUTTON_OK:
		return m_buttonOK;
		break;

	case id_BUTTON_CANCEL:
		return m_buttonCancel;
		break;

	case id_BUTTON_APPLY:
		return m_buttonApply;
		break;

	default:
		UT_ASSERT("Unknown Widget");
		return 0;
		break;
	}
}

void AP_QNXDialog_Options::_controlEnable( tControl id, bool value )
{
	PtWidget_t *w = _lookupWidget(id);
	UT_ASSERT( w );

	if (!w) {
		printf("BAD CAN;t find %d widget \n", id);
		return;
	}

	PtArg_t arg;
	PtSetArg(&arg, Pt_ARG_FLAGS,
				(value) ? (Pt_BLOCKED | Pt_GHOST) : 0,
				Pt_BLOCKED | Pt_GHOST|Pt_SET);
	PtSetResources(w, 1, &arg);
}


#define DEFINE_GET_SET_BOOL(button) \
bool     AP_QNXDialog_Options::_gather##button(void) {       \
	UT_ASSERT(m_checkbutton##button);                           \
	PtArg_t arg;                                                \
	int *flags = NULL;                                          \
	PtSetArg(&arg, Pt_ARG_FLAGS, &flags, 0);                    \
	PtGetResources(m_checkbutton##button, 1, &arg);             \
	return ((flags) ? (*flags & Pt_SET) == Pt_SET : 0); }       \
void        AP_QNXDialog_Options::_set##button(bool b) {	    \
	UT_ASSERT(m_checkbutton##button);                           \
	PtArg_t arg;                                                \
	PtSetArg(&arg, Pt_ARG_FLAGS, (b) ? Pt_SET : 0, Pt_SET);     \
	PtSetResources(m_checkbutton##button, 1, &arg); }

DEFINE_GET_SET_BOOL(SpellCheckAsType);
DEFINE_GET_SET_BOOL(SpellHideErrors);
DEFINE_GET_SET_BOOL(SpellSuggest);
DEFINE_GET_SET_BOOL(SpellMainOnly);
DEFINE_GET_SET_BOOL(SpellUppercase);
DEFINE_GET_SET_BOOL(SpellNumbers);
DEFINE_GET_SET_BOOL(SpellInternet);
DEFINE_GET_SET_BOOL(SmartQuotesEnable);
DEFINE_GET_SET_BOOL(PrefsAutoSave);
DEFINE_GET_SET_BOOL(ViewShowRuler);
DEFINE_GET_SET_BOOL(ViewShowStatusBar);

DEFINE_GET_SET_BOOL(OtherDirectionRtl);
DEFINE_GET_SET_BOOL(OtherHebrewContextGlyphs);

/* File save options related stuff */
DEFINE_GET_SET_BOOL(AutoSaveFile);

void AP_QNXDialog_Options::_gatherAutoSaveFileExt(UT_String &stRetVal)
{
	char *value;
	PtGetResource(m_wAutoSaveFileExt, Pt_ARG_TEXT_STRING, &value, 0);
    stRetVal = value;
}

void AP_QNXDialog_Options::_setAutoSaveFileExt(const UT_String &stExt)
{
	PtSetResource(m_wAutoSaveFileExt, Pt_ARG_TEXT_STRING, stExt.c_str(), 0);
}

void AP_QNXDialog_Options::_gatherAutoSaveFilePeriod(UT_String &stRetVal)
{
	int *value;
	char nb[12];
	PtGetResource(m_wAutoSaveFilePeriod, Pt_ARG_NUMERIC_VALUE, &value, 0);
	snprintf(nb, sizeof(nb), "%d", (value) ? *value : 1);
	stRetVal = nb;
}

void AP_QNXDialog_Options::_setAutoSaveFilePeriod(const UT_String &stPeriod)
{
	PtSetResource(m_wAutoSaveFilePeriod, Pt_ARG_NUMERIC_VALUE, atoi(stPeriod.c_str()), 0);
}

UT_Dimension AP_QNXDialog_Options::_gatherViewRulerUnits(void)
{
	int selection;
    selection = UT_QNXComboGetPos(m_listViewRulerUnits);

	UT_ASSERT(m_listViewRulerUnits && selection > 0);

 	return (UT_Dimension)((int)m_vecUnits.getNthItem(selection - 1));
}

void    AP_QNXDialog_Options::_setViewRulerUnits(UT_Dimension dim)
{
	UT_uint32 i;

	for(i = 0; i < m_vecUnits.getItemCount(); i++) {
		if((UT_Dimension)((int)m_vecUnits.getNthItem(i)) == dim) {
			UT_QNXComboSetPos(m_listViewRulerUnits, i+1);
			return;
		}
	}
	UT_ASSERT(0);
}

DEFINE_GET_SET_BOOL	(ViewCursorBlink);

DEFINE_GET_SET_BOOL	(ViewAll);
DEFINE_GET_SET_BOOL	(ViewHiddenText);
DEFINE_GET_SET_BOOL	(ViewUnprintable);

#undef DEFINE_GET_SET_BOOL

int AP_QNXDialog_Options::_gatherNotebookPageNum(void)
{
	UT_DEBUGMSG(("TODO: _gatherNotebookPageNum "));
	return 0;
}

void    AP_QNXDialog_Options::_setNotebookPageNum(int pn)
{
	UT_DEBUGMSG(("TODO: _gatherNotebookPageNum "));
}

fp_PageSize::Predefined AP_QNXDialog_Options::_gatherDefaultPageSize(void)
{
	// FIXME: replace this with *real* gui code
	return defaultPageSize;
}

void	AP_QNXDialog_Options::_setDefaultPageSize(fp_PageSize::Predefined pre)
{
	// FIXME: replace this with *real* gui code
	defaultPageSize = pre;
}
void AP_QNXDialog_Options::_setViewShowToolbar(UT_uint32 row, bool b)
{


}
bool AP_QNXDialog_Options::_gatherViewShowToolbar(UT_uint32 row)
{

}
