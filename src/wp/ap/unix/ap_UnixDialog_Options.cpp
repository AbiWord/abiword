/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003 Hubert Figuiere
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
#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"
#include "xap_Prefs.h"
#include "xap_Toolbar_Layouts.h"

#include "ap_Dialog_Id.h"
#include "ap_Prefs_SchemeIds.h"

#include "ap_Strings.h"

#include "ap_UnixDialog_Options.h"

/*****************************************************************/

#define WIDGET_MENU_OPTION_PTR		"menuoptionptr"
#define WIDGET_MENU_VALUE_TAG		"value"

#define WID(widget)			glade_xml_get_widget(xml,widget)

/*****************************************************************/

typedef struct TBData {
	const gchar *name;
	int visible;
} TBData;

XAP_Dialog * AP_UnixDialog_Options::static_constructor (XAP_DialogFactory * pFactory,
							XAP_Dialog_Id id)
{
	return new AP_UnixDialog_Options(pFactory,id);
}

AP_UnixDialog_Options::AP_UnixDialog_Options (XAP_DialogFactory * pDlgFactory,
					      XAP_Dialog_Id id)
  : AP_Dialog_Options(pDlgFactory, id), 
	m_extraPages(NULL)
{}

AP_UnixDialog_Options::~AP_UnixDialog_Options(void)
{
}

/*****************************************************************/

void AP_UnixDialog_Options::runModal(XAP_Frame * pFrame)
{
	// Build the window's widgets and arrange them
	GtkWidget *mainWindow = _constructWindow();
	UT_ASSERT(mainWindow);

	// save for use with event
	m_pFrame = pFrame;

	// Populate the window's data items
	_populateWindowData();

	// Don't destroy the dialog if the user pressed defaults or help
	gint response;
	do
	{
		response = abiRunModalDialog (GTK_DIALOG (mainWindow), pFrame,
										this, GTK_RESPONSE_CLOSE, FALSE);
	} while (response != GTK_RESPONSE_CLOSE && response != GTK_RESPONSE_DELETE_EVENT);

	// unhook extra pages
	GSList *item = m_extraPages;
	while (item) {

		const XAP_NotebookDialog::Page *p = static_cast<const XAP_NotebookDialog::Page*>(item->data);
		GtkWidget *page = GTK_WIDGET(p->widget);
		gint i;

		i = gtk_notebook_page_num(GTK_NOTEBOOK(m_notebook), page);
		if (i > -1) {
			gtk_notebook_remove_page(GTK_NOTEBOOK(m_notebook), i);
		}

		GSList *tmp = item;
		item = item->next;
		g_slist_free_1(tmp);
	}

	abiDestroyWidget (mainWindow);
}

///
/// All this color selection code is stolen from the ap_UnixDialog_Background
/// dialog
///
#define CTI(c, v) (unsigned char)(c[v] * 255.0)

/* static */ void AP_UnixDialog_Options::s_color_changed (GtkColorSelection *csel,
							  gpointer data)
{
  AP_UnixDialog_Options * dlg = static_cast<AP_UnixDialog_Options *>(data);
  UT_ASSERT(csel && dlg);

  char color[10];
  gdouble cur[4];

  gtk_color_selection_get_color (csel, cur);
  sprintf(color,"#%02x%02x%02x",CTI(cur, 0), CTI(cur, 1), CTI(cur, 2));
  strncpy(dlg->m_CurrentTransparentColor,static_cast<const gchar *>(color),9);

  UT_DEBUGMSG (("Changing Color [%s]\n", color));

  if (strcmp (dlg->m_CurrentTransparentColor, "#ffffff") == 0)
	gtk_widget_set_sensitive (dlg->m_buttonColSel_Defaults, FALSE);
  else
	gtk_widget_set_sensitive (dlg->m_buttonColSel_Defaults, TRUE);

  // Update document view throuh instant apply magic. Emitting the "clicked" signal will result in a loop and
  // many dialogs popping up. Hacky, because we directly call a callback.
  s_control_changed (dlg->m_pushbuttonNewTransparentColor, data);
}

#undef CTI

void AP_UnixDialog_Options::event_ChooseTransparentColor(void)
{
//
// Run the Background dialog over the options? No the title is wrong.
//
	GtkWidget *dlg;
	GtkWidget *colorsel;
	UT_UTF8String s;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	// get the path where our glade file is located
	XAP_UnixApp * pApp = static_cast<XAP_UnixApp*>(m_pApp);
	UT_String glade_path( pApp->getAbiSuiteAppGladeDir() );
	glade_path += "/ap_UnixDialog_Options_ColorSel.glade";

	GladeXML *xml = abiDialogNewFromXML(glade_path.c_str());
	if (!xml)
		return;

	dlg = WID ("ap_UnixDialog_Options_ColorSel");
	pSS->getValueUTF8 (AP_STRING_ID_DLG_Options_Label_ChooseForTransparent, s);
	abiDialogSetTitle (dlg, s.utf8_str());

	colorsel = WID ("csColorSel");

	// quiet hacky. Fetch defaults button from colsel glade file and store it inside
	// the main dialog, because we'll need this for sensitivity toggling
	m_buttonColSel_Defaults = WID ("btnDefaults");

	g_signal_connect (G_OBJECT (colorsel), "color-changed",
			  G_CALLBACK(s_color_changed),
			  static_cast<gpointer>(this));

	UT_RGBColor c;
	UT_parseColor(m_CurrentTransparentColor,c);

	gdouble currentColor[4] = { 0, 0, 0, 0 };
	currentColor[0] = (static_cast<gdouble>(c.m_red) / static_cast<gdouble>(255.0));
	currentColor[1] = (static_cast<gdouble>(c.m_grn) / static_cast<gdouble>(255.0));
	currentColor[2] = (static_cast<gdouble>(c.m_blu) / static_cast<gdouble>(255.0));

	gtk_color_selection_set_color (GTK_COLOR_SELECTION (colorsel), currentColor);

	// run into the gtk main loop for this window. If the reponse is 0, the user pressed Defaults.
	// Don't destroy it if he did so.
	while (!abiRunModalDialog(GTK_DIALOG (dlg), m_pFrame, this, GTK_RESPONSE_OK, FALSE)) {
		// Answer was 0, so reset color to default
		strncpy(m_CurrentTransparentColor,static_cast<const gchar *>("ffffff"),9);

		UT_parseColor(m_CurrentTransparentColor,c);
		gdouble currentColor[4] = { 0, 0, 0, 0 };
		currentColor[0] = (static_cast<gdouble>(c.m_red) / static_cast<gdouble>(255.0));
		currentColor[1] = (static_cast<gdouble>(c.m_grn) / static_cast<gdouble>(255.0));
		currentColor[2] = (static_cast<gdouble>(c.m_blu) / static_cast<gdouble>(255.0));

		gtk_color_selection_set_color (GTK_COLOR_SELECTION (colorsel), currentColor);
	}
//
// Finish up here after a close or window delete signal.
//
	abiDestroyWidget(dlg);
}

void AP_UnixDialog_Options::addPage (const XAP_NotebookDialog::Page *page)
{
	// the page stays owned by the factory
	m_extraPages = g_slist_prepend(m_extraPages, (gpointer) page);
}

/*****************************************************************/
#define CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(w)			\
        do {							\
	        g_signal_connect(G_OBJECT(w), "activate",	\
                G_CALLBACK(s_menu_item_activate),		\
                static_cast<gpointer>(this));			\
        } while (0)

void AP_UnixDialog_Options::_setupUnitMenu (GtkWidget *optionmenu, const XAP_StringSet *pSS)
{
	GtkWidget *menu;
	GtkWidget *menuitem;
	UT_UTF8String s;

	menu = gtk_menu_new ();

	// inches
	pSS->getValueUTF8 (XAP_STRING_ID_DLG_Unit_inch, s);
	menuitem = gtk_menu_item_new_with_label (s.utf8_str());
 	g_object_set_data (G_OBJECT (menuitem), WIDGET_MENU_OPTION_PTR, static_cast<gpointer>(optionmenu));
 	g_object_set_data (G_OBJECT (menuitem), WIDGET_MENU_VALUE_TAG,  GINT_TO_POINTER (DIM_IN));
 	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE (menuitem);
	gtk_menu_append (GTK_MENU (menu), menuitem);

	// cm
	pSS->getValueUTF8 (XAP_STRING_ID_DLG_Unit_cm, s);
	menuitem = gtk_menu_item_new_with_label (s.utf8_str());
 	g_object_set_data (G_OBJECT (menuitem), WIDGET_MENU_OPTION_PTR, static_cast<gpointer>(optionmenu));
 	g_object_set_data (G_OBJECT (menuitem), WIDGET_MENU_VALUE_TAG,  GINT_TO_POINTER (DIM_CM));
 	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE (menuitem);
	gtk_menu_append (GTK_MENU (menu), menuitem);

	// points
	pSS->getValueUTF8 (XAP_STRING_ID_DLG_Unit_points, s);
	menuitem = gtk_menu_item_new_with_label (s.utf8_str());
 	g_object_set_data (G_OBJECT (menuitem), WIDGET_MENU_OPTION_PTR, static_cast<gpointer>(optionmenu));
  	g_object_set_data (G_OBJECT (menuitem), WIDGET_MENU_VALUE_TAG,  GINT_TO_POINTER (DIM_PT));
  	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE (menuitem);
	gtk_menu_append (GTK_MENU (menu), menuitem);

	// picas
	pSS->getValueUTF8 (XAP_STRING_ID_DLG_Unit_pica, s);
	menuitem = gtk_menu_item_new_with_label (s.utf8_str());
  	g_object_set_data (G_OBJECT (menuitem), WIDGET_MENU_OPTION_PTR, static_cast<gpointer>(optionmenu));
  	g_object_set_data (G_OBJECT (menuitem), WIDGET_MENU_VALUE_TAG,  GINT_TO_POINTER (DIM_PI));
  	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE (menuitem);
  	gtk_menu_append (GTK_MENU (menu), menuitem);

	gtk_widget_show_all (menu);
	gtk_option_menu_set_menu (GTK_OPTION_MENU (optionmenu), menu);
}

void AP_UnixDialog_Options::_constructWindowContents (GladeXML *xml)
{
	const XAP_StringSet *pSS = m_pApp->getStringSet();
	//const UT_Vector & vec = m_pApp->getToolbarFactory()->getToolbarNames();

	GtkWidget *tmp;

	// Dialog

	m_windowMain = WID ("ap_UnixDialog_Options");

	m_notebook = WID ("ntbMain");
	GSList *item = m_extraPages;
	while (item) {

		const XAP_NotebookDialog::Page *p = static_cast<const XAP_NotebookDialog::Page*>(item->data);
		GtkWidget *label = gtk_label_new(p->title);
		GtkWidget *page = GTK_WIDGET(p->widget);
		
		gtk_notebook_append_page(GTK_NOTEBOOK(m_notebook), 
								 page, label);
		item = item->next;
	}

	m_buttonDefaults = WID ("btnDefaults");
	m_buttonClose = WID ("btnClose");


	// General

	tmp = WID ("lblGeneral");
	localizeLabel (tmp, pSS, AP_STRING_ID_DLG_Options_Label_General);

		// User Interface

		tmp = WID ("lblUserInterface");
		localizeLabelMarkup (tmp, pSS, AP_STRING_ID_DLG_Options_Label_UI);
		
		m_checkbuttonViewCursorBlink = WID ("chkCursorBlink");
		localizeButtonUnderline (m_checkbuttonViewCursorBlink, pSS,
					AP_STRING_ID_DLG_Options_Label_ViewCursorBlink);

		m_checkbuttonAllowCustomToolbars = WID ("chkCustomToolbars");
		localizeButtonUnderline (m_checkbuttonAllowCustomToolbars, pSS,
					 AP_STRING_ID_DLG_Options_Label_CheckAllowCustomToolbars);

		tmp = WID ("lblUnits");
		localizeLabelUnderline (tmp, pSS, AP_STRING_ID_DLG_Options_Label_ViewUnits);

		m_menuUnits = WID ("omUnits");
		_setupUnitMenu (m_menuUnits, pSS);

		m_pushbuttonNewTransparentColor = WID ("btnScreenColor");

		tmp = WID ("lblScreenColor");
		localizeLabelUnderline (tmp, pSS, AP_STRING_ID_DLG_Options_Label_ChooseForTransparent);

#if !defined(EMBEDDED_TARGET) || EMBEDDED_TARGET != EMBEDDED_TARGET_HILDON
		// Application Startup
		tmp = WID ("lblApplicationStartup");
		localizeLabelMarkup (tmp, pSS, AP_STRING_ID_DLG_Options_Label_AppStartup);
#endif

		m_checkbuttonAutoLoadPlugins = WID ("chkAutoLoadPlugins");
		localizeButtonUnderline (m_checkbuttonAutoLoadPlugins, pSS,
					 AP_STRING_ID_DLG_Options_Label_CheckAutoLoadPlugins);


	// Documents

	tmp = WID ("lblDocuments");
	localizeLabel (tmp, pSS, AP_STRING_ID_DLG_Options_Label_Documents);

		// Auto Save

		m_checkbuttonAutoSaveFile = WID ("chkAutoSave");
		localizeButtonMarkup (m_checkbuttonAutoSaveFile, pSS,
				      AP_STRING_ID_DLG_Options_Label_AutoSaveUnderline);

			m_tableAutoSaveFile = WID ("tblAutoSave");

			tmp = WID ("lblInterval");
			localizeLabelUnderline (tmp, pSS, AP_STRING_ID_DLG_Options_Label_AutoSaveInterval);

			m_textAutoSaveFilePeriod = WID ("spInterval");

			tmp = WID ("lblFileExt");
			localizeLabelUnderline (tmp, pSS, AP_STRING_ID_DLG_Options_Label_FileExtension);

			m_textAutoSaveFileExt = WID ("enFileExt");

			tmp = WID ("lblMinutes");
			localizeLabel (tmp, pSS, AP_STRING_ID_DLG_Options_Label_Minutes);

		// RTL Text Layout
		tmp = WID ("lblRTL");
		localizeLabelMarkup (tmp, pSS, AP_STRING_ID_DLG_Options_Label_BiDiOptions);

		m_checkbuttonOtherDirectionRtl = WID ("chkDefaultToRTL");
		localizeButtonUnderline (m_checkbuttonOtherDirectionRtl, pSS,
					 AP_STRING_ID_DLG_Options_Label_DirectionRtl);

	// Spell Checking

	tmp = WID ("lblSpellChecking");
	localizeLabel (tmp, pSS, AP_STRING_ID_DLG_Options_SpellCheckingTitle);

		// General

		tmp = WID ("lblSpellCheckingGeneral");
		localizeLabelMarkup (tmp, pSS, AP_STRING_ID_DLG_Options_Label_General);

		m_checkbuttonSpellCheckAsType = WID ("chkSpellCheckAsType");
		localizeButtonUnderline (m_checkbuttonSpellCheckAsType, pSS,
					 AP_STRING_ID_DLG_Options_Label_SpellCheckAsType);

		m_checkbuttonSpellHideErrors = WID ("chkHighlightMisspelledWords");
		localizeButtonUnderline (m_checkbuttonSpellHideErrors, pSS,
					 AP_STRING_ID_DLG_Options_Label_SpellHighlightMisspelledWords);

		// Ignore Words

		tmp = WID ("lblIgnoreWords");
		localizeLabelMarkup (tmp, pSS, AP_STRING_ID_DLG_Options_Label_SpellIgnoreWords);

		m_checkbuttonSpellUppercase = WID ("chkIgnoreUppercase");
		localizeButtonUnderline (m_checkbuttonSpellUppercase, pSS,
					 AP_STRING_ID_DLG_Options_Label_SpellUppercase);

		m_checkbuttonSpellNumbers = WID ("chkIgnoreNumbers");
		localizeButtonUnderline (m_checkbuttonSpellNumbers, pSS,
					 AP_STRING_ID_DLG_Options_Label_SpellNumbers);

		// Dictionaries

		tmp = WID ("lblDictionaries");
		localizeLabelMarkup (tmp, pSS, AP_STRING_ID_DLG_Options_Label_SpellDictionaries);

		m_checkbuttonSpellSuggest = WID ("chkAlwaysSuggest");
		localizeButtonUnderline (m_checkbuttonSpellSuggest, pSS,
					 AP_STRING_ID_DLG_Options_Label_SpellSuggest);

		m_checkbuttonSpellMainOnly = WID ("chkOnlySuggestFromMain");
		localizeButtonUnderline (m_checkbuttonSpellMainOnly, pSS,
					 AP_STRING_ID_DLG_Options_Label_SpellMainOnly);

		tmp = WID ("lblGrammar");
		localizeLabelMarkup (tmp, pSS, AP_STRING_ID_DLG_Options_Label_Grammar);

		m_checkbuttonGrammarCheck = WID ("chkGrammarCheck");
		localizeButtonUnderline (m_checkbuttonGrammarCheck, pSS,
					 AP_STRING_ID_DLG_Options_Label_GrammarCheck);

	//////////////////////////////////////////////////////////////////

//
// Have to reset the toolbar is the result of a toggle is to turn off 
// custom toolbars
//
	g_signal_connect (G_OBJECT (m_checkbuttonAllowCustomToolbars),
			  "toggled",
			  G_CALLBACK(s_toolbars_toggled),
			  static_cast<gpointer>(this));

	// to enable/disable other controls (hide errors)
	g_signal_connect (G_OBJECT (m_checkbuttonSpellCheckAsType),
			  "toggled",
			  G_CALLBACK(s_checkbutton_toggle),
			  static_cast<gpointer>(this));

	// to enable/disable the save 
	g_signal_connect (G_OBJECT (m_checkbuttonAutoSaveFile),
			  "toggled",
			  G_CALLBACK(s_auto_save_toggled),
			  static_cast<gpointer>(this));

	// set inital state
	g_signal_emit_by_name (G_OBJECT (m_checkbuttonAutoSaveFile), "toggled");


	// to choose another color for the screen
	g_signal_connect (G_OBJECT (m_pushbuttonNewTransparentColor),
			  "clicked",
			  G_CALLBACK (s_chooseTransparentColor),
			  static_cast<gpointer>(this));

	_setNotebookPageNum (0);
}

GtkWidget* AP_UnixDialog_Options::_constructWindow ()
{
	GtkWidget *mainWindow;
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	// get the path where our glade file is located
	XAP_UnixApp * pApp = static_cast<XAP_UnixApp*>(m_pApp);
	UT_String glade_path( pApp->getAbiSuiteAppGladeDir() );
#if defined(EMBEDDED_TARGET) && EMBEDDED_TARGET == EMBEDDED_TARGET_HILDON
	glade_path += "/ap_UnixHildonDialog_Options.glade";
#else
	glade_path += "/ap_UnixDialog_Options.glade";
#endif

	// Update member variables with the important widgets that
	// might need to be queried or altered later.

	// load the dialog from the glade file
	GladeXML *xml = abiDialogNewFromXML(glade_path.c_str());
	if (!xml)
		return NULL;

	_constructWindowContents(xml);

	// create the accelerators from &'s
	// createLabelAccelerators(mainWindow);

	mainWindow = glade_xml_get_widget(xml,"ap_UnixDialog_Options");

	// set the dialog title
	UT_UTF8String s;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Options_OptionsTitle, s);
	abiDialogSetTitle(mainWindow, s.utf8_str());

	// the control buttons
	g_signal_connect(G_OBJECT(m_buttonDefaults),
			   "clicked",
			   G_CALLBACK(s_defaults_clicked),
			   static_cast<gpointer>(this));


	// create user data tControl -> stored in widgets
	for ( int i = 0; i < id_last; i++ )
	{
		GtkWidget *w = _lookupWidget( static_cast<tControl>(i) );
		if (!(w && GTK_IS_WIDGET (w)))
		  continue;

		/* check to see if there is any data already stored there (note, will
		 * not work if 0's is stored in multiple places  */
		UT_ASSERT( g_object_get_data(G_OBJECT(w), "tControl" ) == NULL);

		g_object_set_data (G_OBJECT (w), "tControl", reinterpret_cast<gpointer>(i));
		if (GTK_IS_OPTION_MENU (w) || GTK_IS_ENTRY (w))
			g_signal_connect (G_OBJECT (w),
					  "changed",
					  G_CALLBACK (s_control_changed),
					  static_cast<gpointer>(this));
		else if (GTK_IS_TOGGLE_BUTTON (w))
			g_signal_connect (G_OBJECT (w),
					  "toggled",
					  G_CALLBACK (s_control_changed),
					  static_cast<gpointer>(this));
		else if (GTK_IS_SPIN_BUTTON (w))
			g_signal_connect (G_OBJECT (w),
					  "value-changed",
					  G_CALLBACK (s_control_changed),
					  static_cast<gpointer>(this));
	}
	
	return mainWindow;
}

GtkWidget *AP_UnixDialog_Options::_lookupWidget ( tControl id )
{
	switch (id)
	{
	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// spell
	case id_CHECK_SPELL_CHECK_AS_TYPE:
		return m_checkbuttonSpellCheckAsType;

	case id_CHECK_SPELL_HIDE_ERRORS:
		return m_checkbuttonSpellHideErrors;

	case id_CHECK_SPELL_SUGGEST:
		return m_checkbuttonSpellSuggest;

	case id_CHECK_SPELL_MAIN_ONLY:
		return m_checkbuttonSpellMainOnly;

	case id_CHECK_SPELL_UPPERCASE:
		return m_checkbuttonSpellUppercase;

	case id_CHECK_SPELL_NUMBERS:
		return m_checkbuttonSpellNumbers;

	case id_CHECK_GRAMMAR_CHECK:
		return m_checkbuttonGrammarCheck;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// other
	case id_CHECK_OTHER_DEFAULT_DIRECTION_RTL:
		return m_checkbuttonOtherDirectionRtl;

	case id_CHECK_AUTO_SAVE_FILE:
		return m_checkbuttonAutoSaveFile;

	case id_TEXT_AUTO_SAVE_FILE_EXT:
		return m_textAutoSaveFileExt;

	case id_TEXT_AUTO_SAVE_FILE_PERIOD:
		return m_textAutoSaveFilePeriod;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// view
	case id_LIST_VIEW_RULER_UNITS:
		return m_menuUnits;

	case id_CHECK_VIEW_CURSOR_BLINK:
		return m_checkbuttonViewCursorBlink;

	case id_CHECK_ALLOW_CUSTOM_TOOLBARS:
		return m_checkbuttonAllowCustomToolbars;

	case id_CHECK_AUTO_LOAD_PLUGINS:
		return m_checkbuttonAutoLoadPlugins;

	case id_PUSH_CHOOSE_COLOR_FOR_TRANSPARENT:
		return  m_pushbuttonNewTransparentColor;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// general

	case id_BUTTON_DEFAULTS:
		return m_buttonDefaults;

		// not implemented
	case id_CHECK_VIEW_SHOW_STATUS_BAR:
	case id_CHECK_VIEW_SHOW_RULER:
	case id_CHECK_VIEW_UNPRINTABLE:
	case id_CHECK_ENABLE_SMOOTH_SCROLLING:
	case id_CHECK_VIEW_ALL:
	case id_CHECK_VIEW_HIDDEN_TEXT:
	case id_COMBO_PREFS_SCHEME:
	case id_CHECK_PREFS_AUTO_SAVE:

	case id_BUTTON_SAVE:
	case id_BUTTON_APPLY:
	case id_BUTTON_CANCEL:
	case id_BUTTON_OK:
	  return 0;

	default:
		UT_ASSERT("Unknown Widget");
		return 0;
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return 0;
}

void AP_UnixDialog_Options::_controlEnable( tControl id, bool value )
{
	GtkWidget *w = _lookupWidget(id);

	if (w && GTK_IS_WIDGET (w))
	  gtk_widget_set_sensitive( w, value );
}


#define DEFINE_GET_SET_BOOL(button) \
bool     AP_UnixDialog_Options::_gather##button(void) {				\
	UT_ASSERT(m_checkbutton##button && GTK_IS_BUTTON(m_checkbutton##button)); \
	return gtk_toggle_button_get_active(								\
				GTK_TOGGLE_BUTTON(m_checkbutton##button) ); }			\
void        AP_UnixDialog_Options::_set##button(bool b) {	\
	UT_ASSERT(m_checkbutton##button && GTK_IS_BUTTON(m_checkbutton##button)); \
	gtk_toggle_button_set_active (										\
				GTK_TOGGLE_BUTTON(m_checkbutton##button), b ); }

#define DEFINE_GET_SET_TEXT(widget) \
char *		AP_UnixDialog_Options::_gather##widget() {				\
	UT_ASSERT(m_text##widget && GTK_IS_EDITABLE(m_text##widget));	\
	return gtk_editable_get_chars(GTK_EDITABLE(m_text##widget), 0, -1); }			\
\
void		AP_UnixDialog_Options::_set##widget(const char *t) {	\
	int pos = 0;													\
	UT_ASSERT(m_text##widget && GTK_IS_EDITABLE(m_text##widget));	\
	gtk_editable_delete_text(GTK_EDITABLE(m_text##widget), 0, -1);				\
	gtk_editable_insert_text(GTK_EDITABLE(m_text##widget), t, strlen(t), &pos);	\
}

DEFINE_GET_SET_BOOL(SpellCheckAsType)
DEFINE_GET_SET_BOOL(SpellHideErrors)
DEFINE_GET_SET_BOOL(SpellSuggest)
DEFINE_GET_SET_BOOL(SpellMainOnly)
DEFINE_GET_SET_BOOL(SpellUppercase)
DEFINE_GET_SET_BOOL(SpellNumbers)
DEFINE_GET_SET_BOOL(GrammarCheck)

DEFINE_GET_SET_BOOL(OtherDirectionRtl)

DEFINE_GET_SET_BOOL(AutoSaveFile)
	
// dummy implementations. XP pref backend isn't very smart.
#define DEFINE_GET_SET_BOOL_DUMMY(Bool)					\
bool	AP_UnixDialog_Options::_gather##Bool(void) {			\
		return m_bool##Bool;					\
	}								\
void	AP_UnixDialog_Options::_set##Bool(bool b) {			\
		m_bool##Bool = b;					\
	}


DEFINE_GET_SET_BOOL_DUMMY (EnableSmoothScrolling)
DEFINE_GET_SET_BOOL_DUMMY (PrefsAutoSave)
DEFINE_GET_SET_BOOL_DUMMY (ViewAll)
DEFINE_GET_SET_BOOL_DUMMY (ViewHiddenText)
DEFINE_GET_SET_BOOL_DUMMY (ViewShowRuler)
DEFINE_GET_SET_BOOL_DUMMY (ViewShowStatusBar)
DEFINE_GET_SET_BOOL_DUMMY (ViewUnprintable)

void AP_UnixDialog_Options::_gatherAutoSaveFileExt(UT_String &stRetVal)
{
	UT_ASSERT(m_textAutoSaveFileExt && GTK_IS_EDITABLE(m_textAutoSaveFileExt));
	char *tmp = gtk_editable_get_chars(GTK_EDITABLE(m_textAutoSaveFileExt), 0, -1);
	stRetVal = tmp;
	g_free(tmp);
}

void AP_UnixDialog_Options::_setAutoSaveFileExt(const UT_String &stExt)
{
	int pos = 0;
	UT_ASSERT(m_textAutoSaveFileExt && GTK_IS_EDITABLE(m_textAutoSaveFileExt));
	gtk_editable_delete_text(GTK_EDITABLE(m_textAutoSaveFileExt), 0, -1);
	gtk_editable_insert_text(GTK_EDITABLE(m_textAutoSaveFileExt), stExt.c_str(), stExt.size(), &pos);
}

void AP_UnixDialog_Options::_gatherAutoSaveFilePeriod(UT_String &stRetVal)
{
	UT_ASSERT(m_textAutoSaveFilePeriod && GTK_IS_SPIN_BUTTON(m_textAutoSaveFilePeriod));
	char nb[12];
	int val = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(m_textAutoSaveFilePeriod));
	g_snprintf(nb, 12, "%d", val);
	stRetVal = nb;
}

void AP_UnixDialog_Options::_setAutoSaveFilePeriod(const UT_String &stPeriod)
{
	UT_ASSERT(m_textAutoSaveFilePeriod && GTK_IS_EDITABLE(m_textAutoSaveFilePeriod));
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(m_textAutoSaveFilePeriod), atoi(stPeriod.c_str()));
}

UT_Dimension AP_UnixDialog_Options::_gatherViewRulerUnits(void)
{
	UT_ASSERT(m_menuUnits && GTK_IS_OPTION_MENU(m_menuUnits));
	return (UT_Dimension)(GPOINTER_TO_INT(g_object_get_data( G_OBJECT(m_menuUnits), WIDGET_MENU_VALUE_TAG )));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
// This function will lookup a option box by the value stored in the
//	user data under the key WIDGET_MENU_VALUE_TAG
//
typedef struct {
	int index;
	int found;
	gchar *key;
	gpointer data;
} search_data;

static void search_for_value ( GtkWidget *widget, gpointer _value )
{
	search_data *value = static_cast<search_data *>(_value);

	if ( !GTK_IS_MENU_ITEM(widget))
		return;

	value->index++;

	gint v = GPOINTER_TO_INT(g_object_get_data( G_OBJECT(widget), value->key ));
	if ( v == GPOINTER_TO_INT(value->data) )
	{
		// UT_DEBUGMSG(("search_for_value [%d]", static_cast<gint>(value->data) ));
		value->found = value->index;
	}
}

// returns -1 if not found
static int option_menu_set_by_key ( GtkWidget *option_menu, gpointer value, gchar *key )
{
	UT_ASSERT( option_menu && key && GTK_IS_OPTION_MENU(option_menu));

	// at least make sure the value will be restored by the _gather
	g_object_set_data( G_OBJECT(option_menu), key, value);

	// lookup for the key with the value of dim
	search_data data = { -1, -1, key, value };

	GtkWidget *menu = gtk_option_menu_get_menu( GTK_OPTION_MENU(option_menu));
	UT_ASSERT(menu&&GTK_IS_MENU(menu));

	// iterate through all the values
	gtk_container_forall ( GTK_CONTAINER(menu), search_for_value, static_cast<gpointer>(&data) );

	// if we found a value that matches, then say select it
	if ( data.found >= 0 )
	{
		gtk_option_menu_set_history( GTK_OPTION_MENU(option_menu), data.found );
		//UT_DEBUGMSG(("search found %d\n", data.found ));
	}
	else
		UT_DEBUGMSG(("%s:%f search NOT found (searched %d indexes)\n", __FILE__, __LINE__, data.index ));

	return data.found;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void    AP_UnixDialog_Options::_setViewRulerUnits(UT_Dimension dim)
{
	UT_ASSERT(m_menuUnits && GTK_IS_OPTION_MENU(m_menuUnits));

	int r = option_menu_set_by_key ( m_menuUnits, reinterpret_cast<gpointer>(dim), WIDGET_MENU_VALUE_TAG );
	
	if (r < 0)
		UT_DEBUGMSG(("option_menu_set_by_key failed\n"));
}

DEFINE_GET_SET_BOOL	(ViewCursorBlink)

DEFINE_GET_SET_BOOL (AllowCustomToolbars)
DEFINE_GET_SET_BOOL (AutoLoadPlugins)

#undef DEFINE_GET_SET_BOOL

int AP_UnixDialog_Options::_gatherNotebookPageNum(void)
{
	UT_ASSERT(m_notebook && GTK_IS_NOTEBOOK(m_notebook));
	return gtk_notebook_get_current_page( GTK_NOTEBOOK(m_notebook) );
}

void    AP_UnixDialog_Options::_setNotebookPageNum(int pn)
{
	UT_ASSERT(m_notebook && GTK_IS_NOTEBOOK(m_notebook));
	gtk_notebook_set_current_page( GTK_NOTEBOOK(m_notebook), pn );
}

/*****************************************************************/
//
// Reset custom toolbars
//
/* static */ void AP_UnixDialog_Options::s_toolbars_toggled(GtkWidget * widget, gpointer data )
{
	AP_UnixDialog_Options * dlg = static_cast<AP_UnixDialog_Options *>(data);
	UT_ASSERT(widget && dlg);
	if(dlg->isInitialPopulationHappenning())
	{
		return;
	}

//
// If the toolbar preference is now off, reset to default.
//
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)) == FALSE)
	{
		XAP_App * pApp = XAP_App::getApp();
		pApp->resetToolbarsToDefault();
	}
}

/*static*/ void AP_UnixDialog_Options::s_defaults_clicked( GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_Options * dlg = static_cast<AP_UnixDialog_Options *>(data);
	UT_ASSERT(widget && dlg);
	dlg->_event_SetDefaults();

#if 0
	// repopulate controls
	dlg->_populateWindowData();
#endif
}

/*static*/ void AP_UnixDialog_Options::s_control_changed (GtkWidget *widget, gpointer data)
{
	guint id;
	UT_DEBUGMSG (("Control changed\n"));
	AP_UnixDialog_Options *dlg = static_cast<AP_UnixDialog_Options *>(data);
	UT_ASSERT(widget && dlg);

	if(dlg->isInitialPopulationHappenning())
	{
		return;
	}

	id = GPOINTER_TO_INT (g_object_get_data (G_OBJECT(widget), "tControl"));
	dlg->_storeDataForControl (static_cast <tControl> (id));
}

/*static*/ void AP_UnixDialog_Options::s_chooseTransparentColor( GtkWidget *widget, gpointer data )
{
	AP_UnixDialog_Options * dlg = static_cast<AP_UnixDialog_Options *>(data);
	UT_ASSERT(widget && dlg);
	dlg->event_ChooseTransparentColor();
}


// these function will allow multiple widget to tie into the same logic
// function (at the AP level) to enable/disable stuff
/*static*/ void AP_UnixDialog_Options::s_checkbutton_toggle( GtkWidget *w, gpointer data )
{
	AP_UnixDialog_Options * dlg = static_cast<AP_UnixDialog_Options *>(data);
	UT_ASSERT(dlg);
	UT_ASSERT(w && GTK_IS_WIDGET(w));

	int i = GPOINTER_TO_INT(g_object_get_data( G_OBJECT(w), "tControl" ));
	UT_DEBUGMSG(("s_checkbutton_toggle: control id = %d\n", i));
	dlg->_enableDisableLogic( (AP_Dialog_Options::tControl) i );
}

/*static*/ void AP_UnixDialog_Options::s_auto_save_toggled (GtkToggleButton *togglebutton, gpointer data)
{
	AP_UnixDialog_Options * dlg = static_cast<AP_UnixDialog_Options *>(data);
	gboolean is_toggled;
	UT_ASSERT(dlg);

	is_toggled = gtk_toggle_button_get_active (togglebutton);
	gtk_widget_set_sensitive (dlg->m_tableAutoSaveFile, is_toggled);
}

/*static*/ gint AP_UnixDialog_Options::s_menu_item_activate(GtkWidget * widget, gpointer data )
{
	AP_UnixDialog_Options * dlg = static_cast<AP_UnixDialog_Options *>(data);

	UT_ASSERT(widget && dlg);

	GtkWidget *option_menu = static_cast<GtkWidget *>(g_object_get_data(G_OBJECT(widget),
												 WIDGET_MENU_OPTION_PTR));
	UT_ASSERT( option_menu && GTK_IS_OPTION_MENU(option_menu));

	gpointer p = g_object_get_data( G_OBJECT(widget),
												WIDGET_MENU_VALUE_TAG);

	g_object_set_data( G_OBJECT(option_menu), WIDGET_MENU_VALUE_TAG, p );

	//TODO: This code is now shared between RulerUnits and DefaultPaperSize
	//so anyone who wants to resurect this msg. needs to add a conditional
	//UT_DEBUGMSG(("s_menu_item_activate [%d %s]\n", p, UT_dimensionName( (UT_Dimension)(reinterpret_cast<UT_uint32>(p))) ) );

	return TRUE;
}

void AP_UnixDialog_Options::_storeWindowData(void)
{
	AP_Dialog_Options::_storeWindowData();
}
