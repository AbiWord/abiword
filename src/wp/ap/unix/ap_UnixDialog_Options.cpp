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
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "ut_dialogHelper.h"

#include "gr_UnixGraphics.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"
#include "xap_Prefs.h"

#include "ap_Dialog_Id.h"
#include "ap_Prefs_SchemeIds.h"

#include "ap_Strings.h"

#include "ap_UnixDialog_Options.h"

/*****************************************************************/

#define WIDGET_MENU_OPTION_PTR		"menuoptionptr"
#define WIDGET_MENU_VALUE_TAG		"value"

/*****************************************************************/

#define _(a) a

XAP_Dialog * AP_UnixDialog_Options::static_constructor(XAP_DialogFactory * pFactory,
                                                         XAP_Dialog_Id id)
{
    AP_UnixDialog_Options * p = new AP_UnixDialog_Options(pFactory,id);
    return p;
}

AP_UnixDialog_Options::AP_UnixDialog_Options(XAP_DialogFactory * pDlgFactory,
                                                 XAP_Dialog_Id id)
    : AP_Dialog_Options(pDlgFactory,id)
{
#if 0
	/* DEBUG stuff */
	XAP_Prefs *prefs = m_pApp->getPrefs();
	UT_ASSERT(prefs);
	UT_DEBUGMSG(("AP_UnixDialog_Options::AP_UnixDialog_Options[%s:%d]\n", __FILE__, __LINE__));
	UT_DEBUGMSG(("    current pref : %s\n",
		prefs->getCurrentScheme()->getSchemeName()) );

	UT_Bool b = prefs->savePrefsFile();
	UT_DEBUGMSG(("    prefs saved (%d) in %s\n", b, prefs->getPrefsPathname() ));

	UT_uint32 i;
	XAP_PrefsScheme *ps;
	for ( i = 0; (ps = prefs->getNthScheme(i)) != 0; i++ ) {
		UT_DEBUGMSG(("    %d [%s]\n", i, ps->getSchemeName() ));
	
		XML_Char const *pszKey, *pszValue;
		for ( UT_uint32 j = 0; ps->getNthValue(j, &pszKey, &pszValue ); j++ ) {
			UT_DEBUGMSG(("        %x %-30s : %s\n", j, pszKey, pszValue ));
		}
	}
#endif
}

AP_UnixDialog_Options::~AP_UnixDialog_Options(void)
{
}

/*****************************************************************/

void AP_UnixDialog_Options::runModal(XAP_Frame * pFrame)
{
    // Build the window's widgets and arrange them
    GtkWidget * mainWindow = _constructWindow();
    UT_ASSERT(mainWindow);

	connectFocus(GTK_WIDGET(mainWindow),pFrame);
	// save for use with event
	m_pFrame = pFrame;

    // Populate the window's data items
    _populateWindowData();

    // To center the dialog, we need the frame of its parent.
    XAP_UnixFrame * pUnixFrame = static_cast<XAP_UnixFrame *>(pFrame);
    UT_ASSERT(pUnixFrame);
    
    // Get the GtkWindow of the parent frame
    GtkWidget * parentWindow = pUnixFrame->getTopLevelWindow();
    UT_ASSERT(parentWindow);
    
    // Center our new dialog in its parent and make it a transient
    // so it won't get lost underneath
    centerDialog(parentWindow, mainWindow);
    gtk_window_set_transient_for(GTK_WINDOW(mainWindow), GTK_WINDOW(parentWindow));

    // Show the top level dialog,
    gtk_widget_show(mainWindow);

    // Make it modal, and stick it up top
    gtk_grab_add(mainWindow);

    // Run into the GTK event loop for this window.
	do {
		gtk_main();

		switch ( m_answer )
		{
		case AP_Dialog_Options::a_OK:
			_storeWindowData();
			break;

		case AP_Dialog_Options::a_APPLY:
			UT_DEBUGMSG(("Applying changes\n"));
			_storeWindowData();
			break;

		case AP_Dialog_Options::a_CANCEL:
			break;

		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			break;
		};

	} while ( m_answer == AP_Dialog_Options::a_APPLY );	
	
	gtk_widget_destroy(mainWindow);
}

void AP_UnixDialog_Options::event_OK(void)
{
    m_answer = AP_Dialog_Options::a_OK;
    gtk_main_quit();
}

void AP_UnixDialog_Options::event_Cancel(void)
{
    m_answer = AP_Dialog_Options::a_CANCEL;
    gtk_main_quit();
}

void AP_UnixDialog_Options::event_Apply(void)
{
    m_answer = AP_Dialog_Options::a_APPLY;
    gtk_main_quit();
}

void AP_UnixDialog_Options::event_WindowDelete(void)
{
    m_answer = AP_Dialog_Options::a_CANCEL;    
    gtk_main_quit();
}

/*****************************************************************/
#define CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(w)				\
        do {												\
	        gtk_signal_connect(GTK_OBJECT(w), "activate",	\
                GTK_SIGNAL_FUNC(s_menu_item_activate),		\
                (gpointer) this);							\
        } while (0)

GtkWidget* AP_UnixDialog_Options::_constructWindowContents ()
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	GtkWidget *windowOptions;
	GtkWidget *notebook1;

	GtkWidget *tableSpell;
	GtkWidget *checkbuttonSpellHideErrors;
	GtkWidget *checkbuttonSpellSuggest;
	GtkWidget *checkbuttonSpellMainOnly;
	GtkWidget *checkbuttonSpellUppercase;
	GtkWidget *checkbuttonSpellNumbers;
	GtkWidget *checkbuttonSpellInternet;
	GtkWidget *label4;
	GtkWidget *checkbuttonSpellCheckAsType;
	GtkWidget *buttonSpellIgnoreReset;
	GtkWidget *buttonSpellIgnoreEdit;
	GtkWidget *buttonSpellDictionary;
	GtkWidget *label5;
	GtkWidget *listSpellDicts;
	GtkWidget *listSpellDicts_menu;
	GtkWidget *glade_menuitem;
	GtkWidget *labelSpell;

	GtkWidget *tableOther;
	GtkWidget *checkbuttonSmartQuotesEnable;
	GtkWidget *labelSmartQuotes;

	GtkWidget *tablePreferences;
	GtkWidget *checkbuttonPrefsAutoSave;
	GtkWidget *label6;
	GtkWidget *comboPrefsSchemes;
	GtkWidget *comboPrefsSchemesEdit;
	GtkWidget *labelPreferences;

	GtkWidget *hboxView;
	GtkWidget *vbox4;
	GtkWidget *frame2;
	GtkWidget *vbox7;
	GtkWidget *hbox10;
	GtkWidget *checkbuttonViewRuler;
	GtkWidget *labelUnits;
	GtkWidget *listViewRulerUnit;
	GtkWidget *listViewRulerUnit_menu;
	GtkWidget *checkbuttonViewCursorBlink;
	GtkWidget *checkbuttonViewStandard;
	GtkWidget *checkbuttonViewFormat;
	GtkWidget *checkbuttonViewExtra;
	GtkWidget *frameViewStuff;
	GtkWidget *vbox6;
	GtkWidget *checkbuttonViewAll;
	GtkWidget *checkbuttonViewHidden;
	GtkWidget *checkbuttonViewUnprintable;
	GtkWidget *labelView;

	windowOptions = m_windowMain;
	notebook1 = gtk_notebook_new ();
	gtk_widget_ref (notebook1);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "notebook1", notebook1,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (notebook1);

	// SPELL ////////////////////////////////////////////////////////////////
	tableSpell = gtk_table_new (9, 3, FALSE);
	gtk_widget_ref (tableSpell);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "tableSpell", tableSpell,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (tableSpell);
	gtk_container_add (GTK_CONTAINER (notebook1), tableSpell);
	gtk_container_set_border_width (GTK_CONTAINER (tableSpell), 10);

	checkbuttonSpellHideErrors = gtk_check_button_new_with_label
		(pSS->getValue( AP_STRING_ID_DLG_Options_Label_SpellHideErrors ));
	gtk_widget_ref (checkbuttonSpellHideErrors);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "checkbuttonSpellHideErrors", checkbuttonSpellHideErrors,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (checkbuttonSpellHideErrors);
	gtk_table_attach (GTK_TABLE (tableSpell), checkbuttonSpellHideErrors, 0, 3, 1, 2,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	checkbuttonSpellSuggest = gtk_check_button_new_with_label(pSS->getValue(AP_STRING_ID_DLG_Options_Label_SpellSuggest));
	gtk_widget_ref (checkbuttonSpellSuggest);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "checkbuttonSpellSuggest", checkbuttonSpellSuggest,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (checkbuttonSpellSuggest);
	gtk_table_attach (GTK_TABLE (tableSpell), checkbuttonSpellSuggest, 0, 3, 2, 3,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	checkbuttonSpellMainOnly = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Label_SpellMainOnly));
	gtk_widget_ref (checkbuttonSpellMainOnly);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "checkbuttonSpellMainOnly", checkbuttonSpellMainOnly,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (checkbuttonSpellMainOnly);
	gtk_table_attach (GTK_TABLE (tableSpell), checkbuttonSpellMainOnly, 0, 3, 3, 4,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	checkbuttonSpellUppercase = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Label_SpellUppercase));
	gtk_widget_ref (checkbuttonSpellUppercase);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "checkbuttonSpellUppercase", checkbuttonSpellUppercase,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (checkbuttonSpellUppercase);
	gtk_table_attach (GTK_TABLE (tableSpell), checkbuttonSpellUppercase, 0, 3, 4, 5,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	checkbuttonSpellNumbers = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Label_SpellNumbers));
	gtk_widget_ref (checkbuttonSpellNumbers);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "checkbuttonSpellNumbers", checkbuttonSpellNumbers,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (checkbuttonSpellNumbers);
	gtk_table_attach (GTK_TABLE (tableSpell), checkbuttonSpellNumbers, 0, 3, 5, 6,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	checkbuttonSpellInternet = gtk_check_button_new_with_label ( pSS->getValue(AP_STRING_ID_DLG_Options_Label_SpellInternet));
	gtk_widget_ref (checkbuttonSpellInternet);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "checkbuttonSpellInternet", checkbuttonSpellInternet,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (checkbuttonSpellInternet);
	gtk_table_attach (GTK_TABLE (tableSpell), checkbuttonSpellInternet, 0, 3, 6, 7,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	label4 = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Options_Label_SpellIgnoredWord));
	gtk_widget_ref (label4);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "label4", label4,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (label4);
	gtk_table_attach (GTK_TABLE (tableSpell), label4, 0, 1, 8, 9,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (label4), GTK_JUSTIFY_LEFT);

	checkbuttonSpellCheckAsType = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Label_SpellCheckAsType));
	gtk_widget_ref (checkbuttonSpellCheckAsType);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "checkbuttonSpellCheckAsType", checkbuttonSpellCheckAsType,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (checkbuttonSpellCheckAsType);
	gtk_table_attach (GTK_TABLE (tableSpell), checkbuttonSpellCheckAsType, 0, 3, 0, 1,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	buttonSpellIgnoreReset = gtk_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Btn_IgnoreReset));
	gtk_widget_ref (buttonSpellIgnoreReset);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "buttonSpellIgnoreReset", buttonSpellIgnoreReset,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (buttonSpellIgnoreReset);
	gtk_table_attach (GTK_TABLE (tableSpell), buttonSpellIgnoreReset, 1, 2, 8, 9,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 3);

	buttonSpellIgnoreEdit = gtk_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Btn_IgnoreEdit));
	gtk_widget_ref (buttonSpellIgnoreEdit);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "buttonSpellIgnoreEdit", buttonSpellIgnoreEdit,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (buttonSpellIgnoreEdit);
	gtk_table_attach (GTK_TABLE (tableSpell), buttonSpellIgnoreEdit, 2, 3, 8, 9,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 4, 0);

	buttonSpellDictionary = gtk_button_new_with_label (pSS->getValue( AP_STRING_ID_DLG_Options_Btn_CustomDict));
	gtk_widget_ref (buttonSpellDictionary);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "buttonSpellDictionary", buttonSpellDictionary,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (buttonSpellDictionary);
	gtk_table_attach (GTK_TABLE (tableSpell), buttonSpellDictionary, 2, 3, 7, 8,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 4, 0);

	label5 = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Options_Label_SpellCustomDict));
	gtk_widget_ref (label5);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "label5", label5,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (label5);
	gtk_table_attach (GTK_TABLE (tableSpell), label5, 0, 1, 7, 8,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);
	gtk_label_set_justify (GTK_LABEL (label5), GTK_JUSTIFY_LEFT);

	listSpellDicts = gtk_option_menu_new ();
	gtk_widget_ref (listSpellDicts);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "listSpellDicts", listSpellDicts,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (listSpellDicts);
	gtk_table_attach (GTK_TABLE (tableSpell), listSpellDicts, 1, 2, 7, 8,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);
	listSpellDicts_menu = gtk_menu_new ();
	glade_menuitem = gtk_menu_item_new_with_label ("custom.dic");	// TODO - get from prefs / var
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (listSpellDicts_menu), glade_menuitem);
	gtk_option_menu_set_menu (GTK_OPTION_MENU (listSpellDicts), listSpellDicts_menu);

	labelSpell = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Options_TabLabel_Spelling));
	gtk_widget_ref (labelSpell);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "labelSpell", labelSpell,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (labelSpell);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 0), labelSpell);

	// PREFERENCES ////////////////////////////////////////////////////////////////
	tablePreferences = gtk_table_new (2, 3, FALSE);
	gtk_widget_ref (tablePreferences);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "tablePreferences", tablePreferences,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (tablePreferences);
	gtk_container_add (GTK_CONTAINER (notebook1), tablePreferences);
	gtk_container_set_border_width (GTK_CONTAINER (tablePreferences), 10);

	checkbuttonPrefsAutoSave = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Label_PrefsAutoSave));
	gtk_widget_ref (checkbuttonPrefsAutoSave);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "checkbuttonPrefsAutoSave", checkbuttonPrefsAutoSave,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (checkbuttonPrefsAutoSave);
	gtk_table_attach (GTK_TABLE (tablePreferences), checkbuttonPrefsAutoSave, 0, 3, 0, 1,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	label6 = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Options_Label_PrefsCurrentScheme));
	gtk_widget_ref (label6);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "label6", label6,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (label6);
	gtk_table_attach (GTK_TABLE (tablePreferences), label6, 0, 1, 1, 2,
	                  (GtkAttachOptions) (0),
	                  (GtkAttachOptions) (0), 0, 0);

	comboPrefsSchemes = gtk_combo_new ();
	gtk_widget_ref (comboPrefsSchemes);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "comboPrefsSchemes", comboPrefsSchemes,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (comboPrefsSchemes);
	gtk_table_attach (GTK_TABLE (tablePreferences), comboPrefsSchemes, 2, 3, 1, 2,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	comboPrefsSchemesEdit = GTK_COMBO (comboPrefsSchemes)->entry;
	gtk_widget_ref (comboPrefsSchemesEdit);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "comboPrefsSchemesEdit", comboPrefsSchemesEdit,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (comboPrefsSchemesEdit);

	labelPreferences = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Options_TabLabel_Preferences));
	gtk_widget_ref (labelPreferences);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "labelPreferences", labelPreferences,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (labelPreferences);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 1), labelPreferences);

	hboxView = gtk_hbox_new (FALSE, 0);
	gtk_widget_ref (hboxView);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "hboxView", hboxView,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (hboxView);
	gtk_container_add (GTK_CONTAINER (notebook1), hboxView);
	gtk_container_set_border_width (GTK_CONTAINER (hboxView), 10);

	vbox4 = gtk_vbox_new (FALSE, 10);
	gtk_widget_ref (vbox4);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "vbox4", vbox4,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (vbox4);
	gtk_box_pack_start (GTK_BOX (hboxView), vbox4, TRUE, TRUE, 5);

	frame2 = gtk_frame_new (pSS->getValue(AP_STRING_ID_DLG_Options_Label_ViewShowHide));
	gtk_widget_ref (frame2);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "frame2", frame2,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (frame2);
	gtk_box_pack_start (GTK_BOX (vbox4), frame2, FALSE, FALSE, 0);

	vbox7 = gtk_vbox_new (FALSE, 0);
	gtk_widget_ref (vbox7);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "vbox7", vbox7,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (vbox7);
	gtk_container_add (GTK_CONTAINER (frame2), vbox7);

	hbox10 = gtk_hbox_new (FALSE, 0);
	gtk_widget_ref (hbox10);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "hbox10", hbox10,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (hbox10);
	gtk_box_pack_start (GTK_BOX (vbox7), hbox10, FALSE, FALSE, 0);

	checkbuttonViewRuler = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Label_ViewRuler));
	gtk_widget_ref (checkbuttonViewRuler);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "checkbuttonViewRuler", checkbuttonViewRuler,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (checkbuttonViewRuler);
	gtk_box_pack_start (GTK_BOX (hbox10), checkbuttonViewRuler, FALSE, FALSE, 0);

	labelUnits = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Options_Label_ViewUnits));
	gtk_widget_ref (labelUnits);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "labelUnits", labelUnits,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (labelUnits);
	gtk_box_pack_start (GTK_BOX (hbox10), labelUnits, TRUE, TRUE, 0);
	gtk_label_set_justify (GTK_LABEL (labelUnits), GTK_JUSTIFY_RIGHT);

	listViewRulerUnit = gtk_option_menu_new ();
	gtk_widget_ref (listViewRulerUnit);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "listViewRulerUnit", listViewRulerUnit,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (listViewRulerUnit);
	gtk_box_pack_start (GTK_BOX (hbox10), listViewRulerUnit, FALSE, FALSE, 0);
	listViewRulerUnit_menu = gtk_menu_new ();
	glade_menuitem = gtk_menu_item_new_with_label (_("inch"));		// TODO
 	/**/ gtk_object_set_data(GTK_OBJECT(glade_menuitem), WIDGET_MENU_OPTION_PTR, (gpointer) listViewRulerUnit );
 	/**/ gtk_object_set_data(GTK_OBJECT(glade_menuitem), WIDGET_MENU_VALUE_TAG,  (gpointer) DIM_IN );	
 	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(glade_menuitem);
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (listViewRulerUnit_menu), glade_menuitem);
 	
 	// glade_menuitem = gtk_menu_item_new_with_label (_("mm"));
 	// /**/ gtk_object_set_data(GTK_OBJECT(glade_menuitem), WIDGET_MENU_OPTION_PTR, (gpointer) listViewRulerUnit );
 	// /**/ gtk_object_set_data(GTK_OBJECT(glade_menuitem), WIDGET_MENU_VALUE_TAG,  (gpointer) DIM_CM );	
 	// CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(glade_menuitem);
 	// gtk_widget_show (glade_menuitem);
 	// gtk_menu_append (GTK_MENU (listViewRulerUnit_menu), glade_menuitem);
 
	glade_menuitem = gtk_menu_item_new_with_label (_("cm"));		// TODO
 	/**/ gtk_object_set_data(GTK_OBJECT(glade_menuitem), WIDGET_MENU_OPTION_PTR, (gpointer) listViewRulerUnit );
 	/**/ gtk_object_set_data(GTK_OBJECT(glade_menuitem), WIDGET_MENU_VALUE_TAG,  (gpointer) DIM_CM );	
 	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(glade_menuitem);
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (listViewRulerUnit_menu), glade_menuitem);
 
 	// glade_menuitem = gtk_menu_item_new_with_label (_("twips"));
 	// gtk_widget_show (glade_menuitem);
 	// /**/ gtk_object_set_data(GTK_OBJECT(glade_menuitem), WIDGET_MENU_OPTION_PTR, (gpointer) listViewRulerUnit );
 	// /**/ gtk_object_set_data(GTK_OBJECT(glade_menuitem), WIDGET_MENU_VALUE_TAG,  (gpointer)  );	
 	// CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(glade_menuitem);
 	// gtk_menu_append (GTK_MENU (listViewRulerUnit_menu), glade_menuitem);
 
	glade_menuitem = gtk_menu_item_new_with_label (_("points"));	// TODO
 	/**/ gtk_object_set_data(GTK_OBJECT(glade_menuitem), WIDGET_MENU_OPTION_PTR, (gpointer) listViewRulerUnit );
  	/**/ gtk_object_set_data(GTK_OBJECT(glade_menuitem), WIDGET_MENU_VALUE_TAG,  (gpointer) DIM_PT );	
  	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(glade_menuitem);
	gtk_widget_show (glade_menuitem);
	gtk_menu_append (GTK_MENU (listViewRulerUnit_menu), glade_menuitem);
  
  	glade_menuitem = gtk_menu_item_new_with_label (_("pico"));
  	/**/ gtk_object_set_data(GTK_OBJECT(glade_menuitem), WIDGET_MENU_OPTION_PTR, (gpointer) listViewRulerUnit );
  	/**/ gtk_object_set_data(GTK_OBJECT(glade_menuitem), WIDGET_MENU_VALUE_TAG,  (gpointer) DIM_PI  );	
  	CONNECT_MENU_ITEM_SIGNAL_ACTIVATE(glade_menuitem);
  	gtk_widget_show (glade_menuitem);
  	gtk_menu_append (GTK_MENU (listViewRulerUnit_menu), glade_menuitem);
  
	gtk_option_menu_set_menu (GTK_OPTION_MENU (listViewRulerUnit), listViewRulerUnit_menu);

	checkbuttonViewStandard = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Label_ViewStandardTB));
	gtk_widget_show (checkbuttonViewStandard);
	gtk_box_pack_start (GTK_BOX (vbox7), checkbuttonViewStandard, FALSE, FALSE, 0);

	checkbuttonViewFormat = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Label_ViewFormatTB));
	gtk_widget_show (checkbuttonViewFormat);
	gtk_box_pack_start (GTK_BOX (vbox7), checkbuttonViewFormat, FALSE, FALSE, 0);

	checkbuttonViewExtra = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Label_ViewExtraTB));
	gtk_widget_show (checkbuttonViewExtra);
	gtk_box_pack_start (GTK_BOX (vbox7), checkbuttonViewExtra, FALSE, FALSE, 0);

	checkbuttonViewCursorBlink = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Label_ViewCursorBlink));
	gtk_widget_ref (checkbuttonViewCursorBlink);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "checkbuttonViewCursorBlink", checkbuttonViewCursorBlink,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (checkbuttonViewCursorBlink);
	gtk_box_pack_start (GTK_BOX (vbox7), checkbuttonViewCursorBlink, FALSE, FALSE, 0);

	frameViewStuff = gtk_frame_new (pSS->getValue(AP_STRING_ID_DLG_Options_Label_ViewViewFrame));
	gtk_widget_ref (frameViewStuff);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "frameViewStuff", frameViewStuff,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (frameViewStuff);
	gtk_box_pack_start (GTK_BOX (vbox4), frameViewStuff, FALSE, FALSE, 0);

	vbox6 = gtk_vbox_new (FALSE, 0);
	gtk_widget_ref (vbox6);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "vbox6", vbox6,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (vbox6);
	gtk_container_add (GTK_CONTAINER (frameViewStuff), vbox6);

	checkbuttonViewAll = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Label_ViewAll));
	gtk_widget_ref (checkbuttonViewAll);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "checkbuttonViewAll", checkbuttonViewAll,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (checkbuttonViewAll);
	gtk_box_pack_start (GTK_BOX (vbox6), checkbuttonViewAll, FALSE, FALSE, 0);

	checkbuttonViewHidden = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Label_ViewHiddenText));
	gtk_widget_ref (checkbuttonViewHidden);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "checkbuttonViewHidden", checkbuttonViewHidden,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (checkbuttonViewHidden);
	gtk_box_pack_start (GTK_BOX (vbox6), checkbuttonViewHidden, FALSE, FALSE, 0);

	checkbuttonViewUnprintable = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Label_ViewUnprintable));
	gtk_widget_ref (checkbuttonViewUnprintable);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "checkbuttonViewUnprintable", checkbuttonViewUnprintable,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (checkbuttonViewUnprintable);
	gtk_box_pack_start (GTK_BOX (vbox6), checkbuttonViewUnprintable, FALSE, FALSE, 0);

	labelView = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Options_TabLabel_View));
	gtk_widget_ref (labelView);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "labelView", labelView,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (labelView);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 2), labelView);

    //////////////////////////////////////////////////////////////////////
	// END: glade stuff


	// OTHER STUFF (including SMART QUOTES) //////////////////////////////
	tableOther = gtk_table_new (2, 3, FALSE);
	gtk_widget_ref (tableOther);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "tableOther", tableOther,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (tableOther);
	gtk_container_add (GTK_CONTAINER (notebook1), tableOther);
	gtk_container_set_border_width (GTK_CONTAINER (tableOther), 10);

	checkbuttonSmartQuotesEnable = gtk_check_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Options_Label_SmartQuotesEnable));
	gtk_widget_ref (checkbuttonSmartQuotesEnable);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "checkbuttonSmartQuotesEnable", checkbuttonSmartQuotesEnable,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (checkbuttonSmartQuotesEnable);
	gtk_table_attach (GTK_TABLE (tableOther), checkbuttonSmartQuotesEnable, 0, 3, 0, 1,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
	                  (GtkAttachOptions) (0), 0, 0);

	labelSmartQuotes = gtk_label_new (pSS->getValue(AP_STRING_ID_DLG_Options_TabLabel_Other));
	gtk_widget_ref (labelSmartQuotes);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "labelSmartQuotes", labelSmartQuotes,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (labelSmartQuotes);
	gtk_notebook_set_tab_label (GTK_NOTEBOOK (notebook1), gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook1), 3), labelSmartQuotes);

	//////////////////////////////////////////////////////////////////

	m_notebook = notebook1;

    m_checkbuttonSpellCheckAsType	= checkbuttonSpellCheckAsType;
    m_checkbuttonSpellHideErrors	= checkbuttonSpellHideErrors;
    m_checkbuttonSpellSuggest		= checkbuttonSpellSuggest;
    m_checkbuttonSpellMainOnly		= checkbuttonSpellMainOnly;
    m_checkbuttonSpellUppercase		= checkbuttonSpellUppercase;
    m_checkbuttonSpellNumbers		= checkbuttonSpellNumbers;
    m_checkbuttonSpellInternet		= checkbuttonSpellInternet;
	m_listSpellDicts				= listSpellDicts;
	m_listSpellDicts_menu			= listSpellDicts_menu;
	m_buttonSpellDictionary			= buttonSpellDictionary;
	m_buttonSpellIgnoreEdit			= buttonSpellIgnoreEdit;
	m_buttonSpellIgnoreReset		= buttonSpellIgnoreReset;

    m_checkbuttonSmartQuotesEnable	= checkbuttonSmartQuotesEnable;

    m_checkbuttonPrefsAutoSave		= checkbuttonPrefsAutoSave;
	m_comboPrefsScheme				= comboPrefsSchemes;

    m_checkbuttonViewShowRuler		= checkbuttonViewRuler;
    m_listViewRulerUnits			= listViewRulerUnit;
    m_checkbuttonViewCursorBlink	= checkbuttonViewCursorBlink;
    m_checkbuttonViewShowStandardBar	= checkbuttonViewStandard;
    m_checkbuttonViewShowFormatBar	= checkbuttonViewFormat;
    m_checkbuttonViewShowExtraBar	= checkbuttonViewExtra;
    m_checkbuttonViewAll			= checkbuttonViewAll;
    m_checkbuttonViewHiddenText		= checkbuttonViewHidden;
    m_checkbuttonViewUnprintable	= checkbuttonViewUnprintable;


    gtk_signal_connect(GTK_OBJECT(buttonSpellIgnoreEdit),
                       "clicked",
                       GTK_SIGNAL_FUNC(s_ignore_edit_clicked),
                       (gpointer) this);

    gtk_signal_connect(GTK_OBJECT(buttonSpellIgnoreReset),
                       "clicked",
                       GTK_SIGNAL_FUNC(s_ignore_reset_clicked),
                       (gpointer) this);

    gtk_signal_connect(GTK_OBJECT(buttonSpellDictionary),
                       "clicked",
                       GTK_SIGNAL_FUNC(s_dict_edit_clicked),
                       (gpointer) this);

	// to enable/disable other controls (hide errors)
	gtk_signal_connect(GTK_OBJECT(checkbuttonSpellCheckAsType),
						"toggled",
                       GTK_SIGNAL_FUNC(s_checkbutton_toggle),
                       (gpointer) this);

	return notebook1;
}

GtkWidget* AP_UnixDialog_Options::_constructWindow ()
{
    //////////////////////////////////////////////////////////////////////
	// BEGIN: glade stuff (interface.c)

	// for the internationalization	
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	GtkWidget *table2;
	GtkWidget *windowOptions;
	GtkWidget *hbuttonbox2;
	GtkWidget *buttonSave;
	GtkWidget *buttonDefaults;
	GtkWidget *buttonApply;
	GtkWidget *buttonOk;
	GtkWidget *buttonCancel;

	windowOptions = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_object_set_data (GTK_OBJECT (windowOptions), "windowOptions", windowOptions);
	gtk_window_set_title (GTK_WINDOW (windowOptions),
		pSS->getValue(AP_STRING_ID_DLG_Options_OptionsTitle) );

	table2 = gtk_table_new (2, 1, FALSE);
	gtk_widget_ref (table2);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "table2", table2,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (table2);
	gtk_container_add (GTK_CONTAINER (windowOptions), table2);

	hbuttonbox2 = gtk_hbutton_box_new ();
	gtk_widget_ref (hbuttonbox2);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "hbuttonbox2", hbuttonbox2,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (hbuttonbox2);
	gtk_table_attach (GTK_TABLE (table2), hbuttonbox2, 0, 1, 1, 2,
	                  (GtkAttachOptions) (GTK_FILL),
	                  (GtkAttachOptions) (GTK_FILL), 0, 0);
	gtk_button_box_set_layout (GTK_BUTTON_BOX (hbuttonbox2), GTK_BUTTONBOX_END);
	gtk_button_box_set_spacing (GTK_BUTTON_BOX (hbuttonbox2), 10);

	buttonSave = gtk_button_new_with_label ( pSS->getValue(AP_STRING_ID_DLG_Options_Btn_Save) );
	gtk_widget_ref (buttonSave);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "buttonSave", buttonSave,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (buttonSave);
	gtk_container_add (GTK_CONTAINER (hbuttonbox2), buttonSave);
	GTK_WIDGET_SET_FLAGS (buttonSave, GTK_CAN_DEFAULT);

	buttonApply = gtk_button_new_with_label ( 
							pSS->getValue(AP_STRING_ID_DLG_Options_Btn_Apply ));
	gtk_widget_ref (buttonApply);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "buttonApply", buttonApply,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (buttonApply);
	gtk_container_add (GTK_CONTAINER (hbuttonbox2), buttonApply);
	GTK_WIDGET_SET_FLAGS (buttonApply, GTK_CAN_DEFAULT);

	buttonDefaults = gtk_button_new_with_label ( 
							pSS->getValue(AP_STRING_ID_DLG_Options_Btn_Default ));
	gtk_widget_ref (buttonDefaults);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "buttonDefaults", buttonDefaults,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (buttonDefaults);
	gtk_container_add (GTK_CONTAINER (hbuttonbox2), buttonDefaults);
	GTK_WIDGET_SET_FLAGS (buttonDefaults, GTK_CAN_DEFAULT);

	buttonOk = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_OK));
	gtk_widget_ref (buttonOk);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "buttonOk", buttonOk,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (buttonOk);
	gtk_container_add (GTK_CONTAINER (hbuttonbox2), buttonOk);
	GTK_WIDGET_SET_FLAGS (buttonOk, GTK_CAN_DEFAULT);

	buttonCancel = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_Cancel));
	gtk_widget_ref (buttonCancel);
	gtk_object_set_data_full (GTK_OBJECT (windowOptions), "buttonCancel", buttonCancel,
	                          (GtkDestroyNotify) gtk_widget_unref);
	gtk_widget_show (buttonCancel);
	gtk_container_add (GTK_CONTAINER (hbuttonbox2), buttonCancel);
	GTK_WIDGET_SET_FLAGS (buttonCancel, GTK_CAN_DEFAULT);


    // the catch-alls
    gtk_signal_connect_after(GTK_OBJECT(windowOptions),
                             "delete_event",
                             GTK_SIGNAL_FUNC(s_delete_clicked),
                             (gpointer) this);


    gtk_signal_connect_after(GTK_OBJECT(windowOptions),
                             "destroy",
                             NULL,
                             NULL);

    //////////////////////////////////////////////////////////////////////
    // the control buttons
    gtk_signal_connect(GTK_OBJECT(buttonOk),
                       "clicked",
                       GTK_SIGNAL_FUNC(s_ok_clicked),
                       (gpointer) this);
    
    gtk_signal_connect(GTK_OBJECT(buttonCancel),
                       "clicked",
                       GTK_SIGNAL_FUNC(s_cancel_clicked),
                       (gpointer) this);

    gtk_signal_connect(GTK_OBJECT(buttonDefaults),
                       "clicked",
                       GTK_SIGNAL_FUNC(s_defaults_clicked),
                       (gpointer) this);

    gtk_signal_connect(GTK_OBJECT(buttonApply),
                       "clicked",
                       GTK_SIGNAL_FUNC(s_apply_clicked),
                       (gpointer) this);


    // Update member variables with the important widgets that
    // might need to be queried or altered later.

    m_windowMain = windowOptions;

    _constructWindowContents();
	gtk_table_attach (GTK_TABLE (table2), m_notebook, 0, 1, 0, 1,
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL),
	                  (GtkAttachOptions) (GTK_EXPAND | GTK_FILL), 10, 7);

    m_buttonSave					= buttonSave;
    m_buttonDefaults				= buttonDefaults;
    m_buttonApply					= buttonApply;
    m_buttonOK						= buttonOk;
    m_buttonCancel					= buttonCancel;

	// create the accelerators from &'s
	createLabelAccelerators(windowOptions);

	// create user data tControl -> stored in widgets 
	for ( int i = 0; i < id_last; i++ )
	{
		GtkWidget *w = _lookupWidget( (tControl)i );
		UT_ASSERT( w && GTK_IS_WIDGET(w) );

		/* check to see if there is any data already stored there (note, will
		 * not work if 0's is stored in multiple places  */
		UT_ASSERT( gtk_object_get_data(GTK_OBJECT(w), "tControl" ) == NULL);

		gtk_object_set_data( GTK_OBJECT(w), "tControl", (gpointer) i );
	}

    return windowOptions;
}

GtkWidget *AP_UnixDialog_Options::_lookupWidget ( tControl id )
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

	case id_CHECK_SPELL_INTERNET:
		return m_checkbuttonSpellInternet;
		break;

	case id_LIST_DICTIONARY:
		return m_listSpellDicts;
		break;

	case id_BUTTON_DICTIONARY_EDIT:
		return m_buttonSpellDictionary;
		break;

	case id_BUTTON_IGNORE_RESET:
		return m_buttonSpellIgnoreReset;
		break;

	case id_BUTTON_IGNORE_EDIT:
		return m_buttonSpellIgnoreEdit;
		break;

	// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// smart quotes
	case id_CHECK_SMART_QUOTES_ENABLE:
		return m_checkbuttonSmartQuotesEnable;
		break;

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

	case id_CHECK_VIEW_SHOW_STANDARD_TOOLBAR:
		return m_checkbuttonViewShowStandardBar;
		break;

	case id_CHECK_VIEW_SHOW_FORMAT_TOOLBAR:
		return m_checkbuttonViewShowFormatBar;
		break;

	case id_CHECK_VIEW_SHOW_EXTRA_TOOLBAR:
		return m_checkbuttonViewShowExtraBar;
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

void AP_UnixDialog_Options::_controlEnable( tControl id, UT_Bool value )
{
	GtkWidget *w = _lookupWidget(id);
	UT_ASSERT( w && GTK_IS_WIDGET(w) );
	gtk_widget_set_sensitive( w, value );
}


#define DEFINE_GET_SET_BOOL(button) \
UT_Bool     AP_UnixDialog_Options::_gather##button(void) {				\
	UT_ASSERT(m_checkbutton##button && GTK_IS_BUTTON(m_checkbutton##button)); \
	return gtk_toggle_button_get_active(								\
				GTK_TOGGLE_BUTTON(m_checkbutton##button) ); }			\
void        AP_UnixDialog_Options::_set##button(UT_Bool b) {	\
	UT_ASSERT(m_checkbutton##button && GTK_IS_BUTTON(m_checkbutton##button)); \
	gtk_toggle_button_set_active (										\
				GTK_TOGGLE_BUTTON(m_checkbutton##button), b ); }

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
DEFINE_GET_SET_BOOL(ViewShowStandardBar);
DEFINE_GET_SET_BOOL(ViewShowFormatBar);
DEFINE_GET_SET_BOOL(ViewShowExtraBar);

UT_Dimension AP_UnixDialog_Options::_gatherViewRulerUnits(void) 
{				
	UT_ASSERT(m_listViewRulerUnits && GTK_IS_OPTION_MENU(m_listViewRulerUnits)); 
	return (UT_Dimension)((gint)gtk_object_get_data( GTK_OBJECT(m_listViewRulerUnits), WIDGET_MENU_VALUE_TAG )); 
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
	search_data *value = (search_data *)_value;

	if ( !GTK_IS_MENU_ITEM(widget))
		return;

	value->index++;

	gint v = (gint) gtk_object_get_data( GTK_OBJECT(widget), value->key );
	if ( v == (gint)value->data )
	{
		// UT_DEBUGMSG(("search_for_value [%d]", (gint) value->data ));
		value->found = value->index;
	}
}

// returns -1 if not found
int option_menu_set_by_key ( GtkWidget *option_menu, gpointer value, gchar *key )
{
	UT_ASSERT( option_menu && key && GTK_IS_OPTION_MENU(option_menu));

	// lookup for the key with the value of dim
	search_data data = { -1, -1, key, value };

	GtkWidget *menu = gtk_option_menu_get_menu( GTK_OPTION_MENU(option_menu));
	UT_ASSERT(menu&&GTK_IS_MENU(menu));

	// iterate through all the values
	gtk_container_forall ( GTK_CONTAINER(menu), search_for_value, (gpointer) &data );

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

void    AP_UnixDialog_Options::_setViewRulerUnits(UT_Dimension dim) 
{	
	UT_ASSERT(m_listViewRulerUnits && GTK_IS_OPTION_MENU(m_listViewRulerUnits)); 

	int r = option_menu_set_by_key ( m_listViewRulerUnits, (gpointer)dim, WIDGET_MENU_VALUE_TAG ); 
	UT_ASSERT( r != -1 );
}

DEFINE_GET_SET_BOOL	(ViewCursorBlink);

DEFINE_GET_SET_BOOL	(ViewAll);
DEFINE_GET_SET_BOOL	(ViewHiddenText);
DEFINE_GET_SET_BOOL	(ViewUnprintable);

#undef DEFINE_GET_SET_BOOL

int AP_UnixDialog_Options::_gatherNotebookPageNum(void) 
{				
	UT_ASSERT(m_notebook && GTK_IS_NOTEBOOK(m_notebook)); 
	return gtk_notebook_get_current_page( GTK_NOTEBOOK(m_notebook) ); 
}			

void    AP_UnixDialog_Options::_setNotebookPageNum(int pn) 
{	
	UT_ASSERT(m_notebook && GTK_IS_NOTEBOOK(m_notebook)); 
	gtk_notebook_set_page( GTK_NOTEBOOK(m_notebook), pn ); 
}

/*****************************************************************/

// sample callback function
/*static*/ void AP_UnixDialog_Options::s_ok_clicked(GtkWidget * /*widget*/, gpointer data)
{ 
	AP_UnixDialog_Options * dlg = (AP_UnixDialog_Options *)data;
	UT_ASSERT(dlg); 
	dlg->event_OK(); 
}

/*static*/ void AP_UnixDialog_Options::s_cancel_clicked(GtkWidget * widget, gpointer data )
{ 
	AP_UnixDialog_Options * dlg = (AP_UnixDialog_Options *)data;
	UT_ASSERT(widget && dlg); 
	dlg->event_Cancel(); 
}

/*static*/ void AP_UnixDialog_Options::s_apply_clicked(GtkWidget * widget, gpointer data )
{ 
	AP_UnixDialog_Options * dlg = (AP_UnixDialog_Options *)data;
	UT_ASSERT(widget && dlg); 
	dlg->event_Apply(); 
}

/*static*/ void AP_UnixDialog_Options::s_delete_clicked(GtkWidget * /* widget */, GdkEvent * /*event*/, gpointer data )
{ 
	AP_UnixDialog_Options * dlg = (AP_UnixDialog_Options *)data;
	UT_ASSERT(dlg); 
	UT_DEBUGMSG(("AP_UnixDialog_Options::s_delete_clicked\n"));
	dlg->event_WindowDelete(); 
}


/*static*/ void AP_UnixDialog_Options::s_ignore_reset_clicked( GtkWidget * /* widget */, gpointer  data )
{ 
	AP_UnixDialog_Options * dlg = (AP_UnixDialog_Options *)data;
	UT_ASSERT(dlg); 
	dlg->_event_IgnoreReset(); 
}

/*static*/ void AP_UnixDialog_Options::s_ignore_edit_clicked( GtkWidget * /* widget */, gpointer  data )
{ 
	AP_UnixDialog_Options * dlg = (AP_UnixDialog_Options *)data;
	UT_ASSERT(dlg); 
	dlg->_event_IgnoreEdit(); 
}

/*static*/ void AP_UnixDialog_Options::s_dict_edit_clicked( GtkWidget * /* widget */, gpointer  data )
{ 
	AP_UnixDialog_Options * dlg = (AP_UnixDialog_Options *)data;
	UT_ASSERT(dlg); 
	dlg->_event_DictionaryEdit(); 
}

/*static*/ void AP_UnixDialog_Options::s_defaults_clicked( GtkWidget *widget, gpointer data )
{ 
	AP_UnixDialog_Options * dlg = (AP_UnixDialog_Options *)data;
	UT_ASSERT(widget && dlg); 
	dlg->_event_SetDefaults(); 
}


// these function will allow multiple widget to tie into the same logic
// function (at the AP level) to enable/disable stuff
/*static*/ void AP_UnixDialog_Options::s_checkbutton_toggle( GtkWidget *w, gpointer data )
{ 
	AP_UnixDialog_Options * dlg = (AP_UnixDialog_Options *)data;
	UT_ASSERT(dlg); 
	UT_ASSERT( w && GTK_IS_WIDGET(w));
	int i = (int) gtk_object_get_data( GTK_OBJECT(w), "tControl" );
	dlg->_enableDisableLogic( (AP_Dialog_Options::tControl) i );
}

/*static*/ gint AP_UnixDialog_Options::s_menu_item_activate(GtkWidget * widget, gpointer data )
{
	AP_UnixDialog_Options * dlg = (AP_UnixDialog_Options *)data;

	UT_ASSERT(widget && dlg);

	GtkWidget *option_menu = (GtkWidget *)gtk_object_get_data(GTK_OBJECT(widget),
												 WIDGET_MENU_OPTION_PTR);
	UT_ASSERT( option_menu && GTK_IS_OPTION_MENU(option_menu));

	gpointer p = gtk_object_get_data( GTK_OBJECT(widget),
												WIDGET_MENU_VALUE_TAG);

	gtk_object_set_data( GTK_OBJECT(option_menu), WIDGET_MENU_VALUE_TAG, p );

	UT_DEBUGMSG(("s_menu_item_activate [%d %s]\n", p, UT_dimensionName( (UT_Dimension)((UT_uint32)p)) ) );

	return TRUE;
}

