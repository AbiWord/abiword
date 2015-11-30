/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2003, 2009 Hubert Figuiere
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

#include "ap_Features.h"

#include "ut_types.h"
#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "xap_UnixDialogHelper.h"
#include "xap_GtkComboBoxHelpers.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"
#include "xap_Prefs.h"
#include "xap_Toolbar_Layouts.h"
#include "xap_EncodingManager.h"

#include "ap_Dialog_Id.h"
#include "ap_Prefs_SchemeIds.h"

#include "ap_Strings.h"

#include "ap_UnixDialog_Options.h"

#if !defined(ENABLE_SPELL) && !defined(_DISABLE_GRAMMAR)
#define _DISABLE_GRAMMAR
#endif

/*****************************************************************/

#define WID(widget)   GTK_WIDGET(gtk_builder_get_object(builder, widget))

/*****************************************************************/


XAP_Dialog * AP_UnixDialog_Options::static_constructor ( XAP_DialogFactory * pFactory,
        XAP_Dialog_Id id )
{
    return new AP_UnixDialog_Options ( pFactory,id );
}

AP_UnixDialog_Options::AP_UnixDialog_Options ( XAP_DialogFactory * pDlgFactory,
        XAP_Dialog_Id id )
        : AP_Dialog_Options ( pDlgFactory, id ),
        m_extraPages ( NULL )
{}

AP_UnixDialog_Options::~AP_UnixDialog_Options ( void )
{
}

/*****************************************************************/

void AP_UnixDialog_Options::runModal ( XAP_Frame * pFrame )
{
    // Build the window's widgets and arrange them
    GtkWidget *mainWindow = _constructWindow();
    UT_ASSERT ( mainWindow );

    // save for use with event
    m_pFrame = pFrame;

    // Populate the window's data items
    _populateWindowData();

    // Don't destroy the dialog if the user pressed defaults or help
    gint response;
    do
    {
        response = abiRunModalDialog ( GTK_DIALOG ( mainWindow ), pFrame,
                                       this, GTK_RESPONSE_CLOSE, FALSE );
    } while ( response != GTK_RESPONSE_CLOSE && response != GTK_RESPONSE_DELETE_EVENT );

    // unhook extra pages
    GSList *item = m_extraPages;
    while ( item ) {

        const XAP_NotebookDialog::Page *p = static_cast<const XAP_NotebookDialog::Page*> ( item->data );
        GtkWidget *page = GTK_WIDGET ( p->widget );
        gint i;

        i = gtk_notebook_page_num ( GTK_NOTEBOOK ( m_notebook ), page );
        if ( i > -1 ) {
            gtk_notebook_remove_page ( GTK_NOTEBOOK ( m_notebook ), i );
        }

        GSList *tmp = item;
        item = item->next;
        g_slist_free_1 ( tmp );
    }

    abiDestroyWidget ( mainWindow );
}

///
/// All this color selection code is stolen from the ap_UnixDialog_Background
/// dialog
///
void AP_UnixDialog_Options::s_real_color_changed(GdkRGBA & gdkcolor, AP_UnixDialog_Options * dlg)
{

	UT_RGBColor * rgbcolor = UT_UnixGdkColorToRGBColor(gdkcolor);
	UT_HashColor hash_color;
    strncpy ( dlg->m_CurrentTransparentColor, hash_color.setColor(*rgbcolor), 9 );
	
    UT_DEBUGMSG ( ( "Changing Color [%s]\n", hash_color.c_str() ) );
	delete rgbcolor;

    if ( strcmp ( dlg->m_CurrentTransparentColor, "#ffffff" ) == 0 )
        gtk_widget_set_sensitive ( dlg->m_buttonColSel_Defaults, FALSE );
    else
        gtk_widget_set_sensitive ( dlg->m_buttonColSel_Defaults, TRUE );

    // Update document view through instant apply magic. Emitting the "clicked" 
	// signal will result in a loop and
    // many dialogs popping up. Hacky, because we directly call a callback.
    s_control_changed ( dlg->m_pushbuttonNewTransparentColor, dlg );
}

void AP_UnixDialog_Options::s_color_changed ( GtkColorChooser *csel,
                                              GdkRGBA         *color,
                                              gpointer data )
{
    AP_UnixDialog_Options * dlg = static_cast<AP_UnixDialog_Options *> ( data );
    UT_ASSERT ( csel && dlg );

    UT_DEBUGMSG(("s_color_changed\n"));
    s_real_color_changed(*color, dlg);
}


void AP_UnixDialog_Options::event_ChooseTransparentColor ( void )
{
    GtkWidget *dlg;

//
// Run the Background dialog over the options? No the title is wrong.
//
    GtkWidget *colorsel;
	std::string s;

    const XAP_StringSet * pSS = m_pApp->getStringSet();

    GtkBuilder * builder = newDialogBuilder("ap_UnixDialog_Options_ColorSel.ui");

    dlg = WID ( "ap_UnixDialog_Options_ColorSel" );
    pSS->getValueUTF8 ( AP_STRING_ID_DLG_Options_Label_ChooseForTransparent, s );
    abiDialogSetTitle ( dlg, "%s", s.c_str() );

    colorsel = WID ( "csColorSel" );

    // quiet hacky. Fetch defaults button from colsel GtkBuilder UI file and store it inside
    // the main dialog, because we'll need this for sensitivity toggling
    m_buttonColSel_Defaults = WID ( "btnDefaults" );

    g_signal_connect ( G_OBJECT ( colorsel ), "color-activated",
                       G_CALLBACK ( s_color_changed ),
                       static_cast<gpointer> ( this ) );

    UT_RGBColor c;
    UT_parseColor ( m_CurrentTransparentColor,c );
	GdkRGBA *gcolor = UT_UnixRGBColorToGdkRGBA(c);

    gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(colorsel), gcolor);
	gdk_rgba_free(gcolor);

    // run into the gtk main loop for this window. If the reponse is 0, the user pressed Defaults.
    // Don't destroy it if he did so.
    while (!abiRunModalDialog(GTK_DIALOG(dlg), m_pFrame, this,
                                          GTK_RESPONSE_OK, FALSE)) {
        // Answer was 0, so reset color to default
        strncpy(m_CurrentTransparentColor, "ffffff", 9);

        UT_parseColor (m_CurrentTransparentColor, c);
        gcolor = UT_UnixRGBColorToGdkRGBA(c);
        gtk_color_chooser_set_rgba(GTK_COLOR_CHOOSER(colorsel), gcolor);
        gdk_rgba_free(gcolor);
    }

    GdkRGBA cc;
    gtk_color_chooser_get_rgba(GTK_COLOR_CHOOSER(colorsel), &cc);
    s_real_color_changed(cc, this);
//
// Finish up here after a close or window delete signal.
//
    abiDestroyWidget ( dlg );

	g_object_unref((GObject*)(builder));
}

void AP_UnixDialog_Options::addPage ( const XAP_NotebookDialog::Page *page )
{
    // the page stays owned by the factory
    m_extraPages = g_slist_prepend ( m_extraPages, ( gpointer ) page );
}

/*****************************************************************/

void AP_UnixDialog_Options::_setupUnitMenu ( GtkWidget *optionmenu, const XAP_StringSet *pSS )
{
	GtkComboBox *combo = GTK_COMBO_BOX(optionmenu);
	UnitMenuContent content;
	_getUnitMenuContent(pSS, content);
	XAP_makeGtkComboBoxText(combo, G_TYPE_INT);
	
	for(UnitMenuContent::const_iterator iter = content.begin();
		iter != content.end(); ++iter) {
		XAP_appendComboBoxTextAndInt(combo, iter->first.c_str(), iter->second);
	}
	gtk_combo_box_set_active(combo, 0);
}

void AP_UnixDialog_Options::_constructWindowContents ( GtkBuilder * builder )
{
    const XAP_StringSet *pSS = m_pApp->getStringSet();
    //const UT_Vector & vec = m_pApp->getToolbarFactory()->getToolbarNames();

    GtkWidget *tmp;

    // Dialog

    m_windowMain = WID ( "ap_UnixDialog_Options" );

    m_notebook = WID ( "ntbMain" );
    GSList *item = m_extraPages;
    while ( item ) {

        const XAP_NotebookDialog::Page *p = static_cast<const XAP_NotebookDialog::Page*> ( item->data );
        GtkWidget *label = gtk_label_new ( p->title );
        GtkWidget *page = GTK_WIDGET ( p->widget );

        gtk_notebook_append_page ( GTK_NOTEBOOK ( m_notebook ),
                                   page, label );
        item = item->next;
    }

    m_buttonDefaults = WID ( "btnDefaults" );
    m_buttonClose = WID ( "btnClose" );


    // General

    tmp = WID ( "lblGeneral" );
    localizeLabel ( tmp, pSS, AP_STRING_ID_DLG_Options_Label_General );

    // User Interface

    tmp = WID ( "lblUserInterface" );
    localizeLabelMarkup ( tmp, pSS, AP_STRING_ID_DLG_Options_Label_UI );

    tmp = WID ( "lblUnits" );
    localizeLabelUnderline ( tmp, pSS, AP_STRING_ID_DLG_Options_Label_ViewUnits );

    m_menuUnits = WID ( "omUnits" );
    _setupUnitMenu ( m_menuUnits, pSS );

    m_pushbuttonNewTransparentColor = WID ( "btnScreenColor" );

    tmp = WID ( "lblScreenColor" );
    localizeLabelUnderline ( tmp, pSS, AP_STRING_ID_DLG_Options_Label_ChooseForTransparent );

    m_checkbuttonEnableOverwrite = WID ( "btnOverwrite" );
    localizeButtonUnderline ( m_checkbuttonEnableOverwrite, pSS,
                              AP_STRING_ID_DLG_Options_Label_EnableOverwrite );    

    // Application Startup
    tmp = WID ( "lblApplicationStartup" );
    localizeLabelMarkup ( tmp, pSS, AP_STRING_ID_DLG_Options_Label_AppStartup );

    m_checkbuttonAutoLoadPlugins = WID ( "chkAutoLoadPlugins" );
    localizeButtonUnderline ( m_checkbuttonAutoLoadPlugins, pSS,
                              AP_STRING_ID_DLG_Options_Label_CheckAutoLoadPlugins );


    // Documents

    tmp = WID ( "lblDocuments" );
    localizeLabel ( tmp, pSS, AP_STRING_ID_DLG_Options_Label_Documents );

    // Auto Save

    m_checkbuttonAutoSaveFile = WID ( "chkAutoSave" );
    localizeButtonMarkup ( m_checkbuttonAutoSaveFile, pSS,
                           AP_STRING_ID_DLG_Options_Label_AutoSaveUnderline );

    m_tableAutoSaveFile = WID ( "tblAutoSave" );

    tmp = WID ( "lblInterval" );
    localizeLabelUnderline ( tmp, pSS, AP_STRING_ID_DLG_Options_Label_AutoSaveInterval );

    m_textAutoSaveFilePeriod = WID ( "spInterval" );

    tmp = WID ( "lblFileExt" );
    localizeLabelUnderline ( tmp, pSS, AP_STRING_ID_DLG_Options_Label_FileExtension );

    m_textAutoSaveFileExt = WID ( "enFileExt" );

    tmp = WID ( "lblMinutes" );
    localizeLabel ( tmp, pSS, AP_STRING_ID_DLG_Options_Label_Minutes );

    // RTL Text Layout
    tmp = WID ( "lblRTL" );
    localizeLabelMarkup ( tmp, pSS, AP_STRING_ID_DLG_Options_Label_BiDiOptions );

    m_checkbuttonOtherDirectionRtl = WID ( "chkDefaultToRTL" );
    localizeButtonUnderline ( m_checkbuttonOtherDirectionRtl, pSS,
                              AP_STRING_ID_DLG_Options_Label_DirectionRtl );

#if ENABLE_SPELL
    // Spell Checking

    tmp = WID ( "lblSpellChecking" );
    localizeLabel ( tmp, pSS, AP_STRING_ID_DLG_Options_SpellCheckingTitle );

    // General

    tmp = WID ( "lblSpellCheckingGeneral" );
    localizeLabelMarkup ( tmp, pSS, AP_STRING_ID_DLG_Options_Label_General );

    m_checkbuttonSpellCheckAsType = WID ( "chkSpellCheckAsType" );
    localizeButtonUnderline ( m_checkbuttonSpellCheckAsType, pSS,
                              AP_STRING_ID_DLG_Options_Label_SpellCheckAsType );

    // to enable/disable other controls (hide errors)
    g_signal_connect ( G_OBJECT ( m_checkbuttonSpellCheckAsType ),
                       "toggled",
                       G_CALLBACK ( s_checkbutton_toggle ),
                       static_cast<gpointer> ( this ) );

    m_checkbuttonSpellHideErrors = WID ( "chkHighlightMisspelledWords" );
    localizeButtonUnderline ( m_checkbuttonSpellHideErrors, pSS,
                              AP_STRING_ID_DLG_Options_Label_SpellHighlightMisspelledWords );

    // Ignore Words

    tmp = WID ( "lblIgnoreWords" );
    localizeLabelMarkup ( tmp, pSS, AP_STRING_ID_DLG_Options_Label_SpellIgnoreWords );

    m_checkbuttonSpellUppercase = WID ( "chkIgnoreUppercase" );
    localizeButtonUnderline ( m_checkbuttonSpellUppercase, pSS,
                              AP_STRING_ID_DLG_Options_Label_SpellUppercase );

    m_checkbuttonSpellNumbers = WID ( "chkIgnoreNumbers" );
    localizeButtonUnderline ( m_checkbuttonSpellNumbers, pSS,
                              AP_STRING_ID_DLG_Options_Label_SpellNumbers );

    // Dictionaries
    tmp = WID ( "lblDictionaries" );
    localizeLabelMarkup ( tmp, pSS, AP_STRING_ID_DLG_Options_Label_SpellDictionaries );

    m_checkbuttonSpellSuggest = WID ( "chkAlwaysSuggest" );
    localizeButtonUnderline ( m_checkbuttonSpellSuggest, pSS,
                              AP_STRING_ID_DLG_Options_Label_SpellSuggest );

    m_checkbuttonSpellMainOnly = WID ( "chkOnlySuggestFromMain" );
    localizeButtonUnderline ( m_checkbuttonSpellMainOnly, pSS,
                              AP_STRING_ID_DLG_Options_Label_SpellMainOnly );

#ifdef _DISABLE_GRAMMAR
    // remove anything related to grammar.
    tmp = WID ( "tableGrammar" );
    gtk_widget_destroy( tmp );
    m_checkbuttonGrammarCheck = NULL;
#else
    tmp = WID ( "lblGrammar" );
    localizeLabelMarkup ( tmp, pSS, AP_STRING_ID_DLG_Options_Label_Grammar );

    m_checkbuttonGrammarCheck = WID ( "chkGrammarCheck" );
    localizeButtonUnderline ( m_checkbuttonGrammarCheck, pSS,
                              AP_STRING_ID_DLG_Options_Label_GrammarCheck );
#endif /// _DISABLE_GRAMMAR

#else
    m_checkbuttonSpellCheckAsType = NULL;
    m_checkbuttonSpellHideErrors = NULL;
    m_checkbuttonSpellUppercase = NULL;
    m_checkbuttonSpellNumbers = NULL;
    m_checkbuttonSpellSuggest = NULL;
    m_checkbuttonSpellMainOnly = NULL;
    m_checkbuttonGrammarCheck = NULL;
    gtk_notebook_remove_page((GtkNotebook*)m_notebook, 2);
#endif
    // Smart Quotes

    tmp = WID ( "lblSmartQuotes" );
    localizeLabel ( tmp, pSS, AP_STRING_ID_DLG_Options_TabLabel_SmartQuotes );

    m_checkbuttonSmartQuotes = WID ( "chkSmartQuotes" );
    localizeButtonUnderline ( m_checkbuttonSmartQuotes, pSS,
                              AP_STRING_ID_DLG_Options_Label_SmartQuotes );

    m_checkbuttonCustomSmartQuotes = WID ( "chkCustomQuoteStyle" );
    localizeButtonUnderline ( m_checkbuttonCustomSmartQuotes, pSS,
                              AP_STRING_ID_DLG_Options_Label_CustomSmartQuotes );

    tmp = WID ( "lblOuterQuoteStyle" );
    localizeLabelUnderline ( tmp, pSS, AP_STRING_ID_DLG_Options_Label_OuterQuoteStyle );

    tmp = WID ( "lblInnerQuoteStyle" );
    localizeLabelUnderline ( tmp, pSS, AP_STRING_ID_DLG_Options_Label_InnerQuoteStyle );

    m_omOuterQuoteStyle = WID ( "omOuterQuoteStyle" );
    m_omInnerQuoteStyle = WID ( "omInnerQuoteStyle" );

    _setupSmartQuotesCombos(m_omOuterQuoteStyle);
    _setupSmartQuotesCombos(m_omInnerQuoteStyle);

    //////////////////////////////////////////////////////////////////


    // to enable/disable other smart quote widgets
    g_signal_connect ( G_OBJECT ( m_checkbuttonSmartQuotes ),
                       "toggled",
                       G_CALLBACK ( s_checkbutton_toggle ),
                       static_cast<gpointer> ( this ) );

    // to enable/disable custom smart quote combos and labels
    g_signal_connect ( G_OBJECT ( m_checkbuttonCustomSmartQuotes ),
                       "toggled",
                       G_CALLBACK ( s_checkbutton_toggle ),
                       static_cast<gpointer> ( this ) );

    // to enable/disable the save
    g_signal_connect ( G_OBJECT ( m_checkbuttonAutoSaveFile ),
                       "toggled",
                       G_CALLBACK ( s_auto_save_toggled ),
                       static_cast<gpointer> ( this ) );

    // set inital state
    g_signal_emit_by_name ( G_OBJECT ( m_checkbuttonAutoSaveFile ), "toggled" );


    // to choose another color for the screen
    g_signal_connect ( G_OBJECT ( m_pushbuttonNewTransparentColor ),
                       "clicked",
                       G_CALLBACK ( s_chooseTransparentColor ),
                       static_cast<gpointer> ( this ) );

    _setNotebookPageNum ( 0 );
}

GtkWidget* AP_UnixDialog_Options::_constructWindow ()
{
    GtkWidget *mainWindow;
    const XAP_StringSet * pSS = m_pApp->getStringSet();

    GtkBuilder * builder = newDialogBuilder("ap_UnixDialog_Options.ui");

    // Update member variables with the important widgets that
    // might need to be queried or altered later.

    _constructWindowContents ( builder );

    mainWindow = GTK_WIDGET(gtk_builder_get_object(builder, "ap_UnixDialog_Options"));
    UT_ASSERT(mainWindow);

    // set the dialog title
    std::string s;
    pSS->getValueUTF8(AP_STRING_ID_DLG_Options_OptionsTitle, s);
    abiDialogSetTitle(mainWindow, "%s", s.c_str());

    // the control buttons
    g_signal_connect ( G_OBJECT ( m_buttonDefaults ),
                       "clicked",
                       G_CALLBACK ( s_defaults_clicked ),
                       static_cast<gpointer> ( this ) );


    // create user data tControl -> stored in widgets
    for ( int i = 0; i < id_last; i++ )
    {
        GtkWidget *w = _lookupWidget ( static_cast<tControl> ( i ) );
        if ( ! ( w && GTK_IS_WIDGET ( w ) ) )
            continue;

        /* check to see if there is any data already stored there (note, will
         * not work if 0's is stored in multiple places  */
        UT_ASSERT ( g_object_get_data ( G_OBJECT ( w ), "tControl" ) == NULL );

        g_object_set_data ( G_OBJECT ( w ), "tControl", reinterpret_cast<gpointer> ( i ) );
        if ( GTK_IS_COMBO_BOX ( w ) || GTK_IS_ENTRY ( w ) )
            g_signal_connect ( G_OBJECT ( w ),
                               "changed",
                               G_CALLBACK ( s_control_changed ),
                               static_cast<gpointer> ( this ) );
        else if ( GTK_IS_TOGGLE_BUTTON ( w ) )
            g_signal_connect ( G_OBJECT ( w ),
                               "toggled",
                               G_CALLBACK ( s_control_changed ),
                               static_cast<gpointer> ( this ) );
        else if ( GTK_IS_SPIN_BUTTON ( w ) )
            g_signal_connect ( G_OBJECT ( w ),
                               "value-changed",
                               G_CALLBACK ( s_control_changed ),
                               static_cast<gpointer> ( this ) );
    }

	g_object_unref(G_OBJECT(builder));

    return mainWindow;
}

GtkWidget *AP_UnixDialog_Options::_lookupWidget ( tControl id )
{
    switch ( id )
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
            // Smart quotes

        case id_CHECK_SMART_QUOTES_ENABLE:
            return m_checkbuttonSmartQuotes;

        case id_CHECK_CUSTOM_SMART_QUOTES:
            return m_checkbuttonCustomSmartQuotes;
            
        case id_LIST_VIEW_OUTER_QUOTE_STYLE:
            return m_omOuterQuoteStyle;
            
        case id_LIST_VIEW_INNER_QUOTE_STYLE:
            return m_omInnerQuoteStyle;

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

        case id_CHECK_AUTO_LOAD_PLUGINS:
            return m_checkbuttonAutoLoadPlugins;

        case id_PUSH_CHOOSE_COLOR_FOR_TRANSPARENT:
            return  m_pushbuttonNewTransparentColor;
        case id_CHECK_ENABLE_OVERWRITE:
            return m_checkbuttonEnableOverwrite;

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
            UT_ASSERT ( "Unknown Widget" );
            return 0;
    }

    UT_ASSERT ( UT_SHOULD_NOT_HAPPEN );
    return 0;
}

void AP_UnixDialog_Options::_controlEnable ( tControl id, bool value )
{
    GtkWidget *w = _lookupWidget ( id );

    if ( w && GTK_IS_WIDGET ( w ) )
        gtk_widget_set_sensitive ( w, value );
}


#define DEFINE_GET_SET_BOOL(button) \
    bool     AP_UnixDialog_Options::_gather##button(void) {    \
        UT_ASSERT(m_checkbutton##button && GTK_IS_BUTTON(m_checkbutton##button)); \
        return gtk_toggle_button_get_active(        \
                GTK_TOGGLE_BUTTON(m_checkbutton##button) ); }   \
    void        AP_UnixDialog_Options::_set##button(bool b) { \
        UT_ASSERT(m_checkbutton##button && GTK_IS_BUTTON(m_checkbutton##button)); \
        gtk_toggle_button_set_active (          \
                                                GTK_TOGGLE_BUTTON(m_checkbutton##button), b ); }

#define DEFINE_GET_SET_BOOL_D(button) \
    bool     AP_UnixDialog_Options::_gather##button(void) {    \
                return false; }   \
    void        AP_UnixDialog_Options::_set##button(bool) { \
               }

#define DEFINE_GET_SET_TEXT(widget) \
    char *  AP_UnixDialog_Options::_gather##widget() {    \
        UT_ASSERT(m_text##widget && GTK_IS_EDITABLE(m_text##widget)); \
        return gtk_editable_get_chars(GTK_EDITABLE(m_text##widget), 0, -1); }   \
    \
    void  AP_UnixDialog_Options::_set##widget(const char *t) { \
        int pos = 0;             \
        UT_ASSERT(m_text##widget && GTK_IS_EDITABLE(m_text##widget)); \
        gtk_editable_delete_text(GTK_EDITABLE(m_text##widget), 0, -1);    \
        gtk_editable_insert_text(GTK_EDITABLE(m_text##widget), t, strlen(t), &pos); \
    }

#ifdef ENABLE_SPELL
DEFINE_GET_SET_BOOL ( SpellCheckAsType )
DEFINE_GET_SET_BOOL ( SpellHideErrors )
DEFINE_GET_SET_BOOL ( SpellSuggest )
DEFINE_GET_SET_BOOL ( SpellMainOnly )
DEFINE_GET_SET_BOOL ( SpellUppercase )
DEFINE_GET_SET_BOOL ( SpellNumbers )
#else
DEFINE_GET_SET_BOOL_D ( SpellCheckAsType )
DEFINE_GET_SET_BOOL_D ( SpellHideErrors )
DEFINE_GET_SET_BOOL_D ( SpellSuggest )
DEFINE_GET_SET_BOOL_D ( SpellMainOnly )
DEFINE_GET_SET_BOOL_D ( SpellUppercase )
DEFINE_GET_SET_BOOL_D ( SpellNumbers )
#endif
#ifndef _DISABLE_GRAMMAR
DEFINE_GET_SET_BOOL ( GrammarCheck )
#else
// TODO FIX this hack I do this to avoid the assert.
bool     AP_UnixDialog_Options::_gatherGrammarCheck(void) 
{
    return false;
}
void        AP_UnixDialog_Options::_setGrammarCheck(bool) 
{
}
#endif
DEFINE_GET_SET_BOOL ( SmartQuotes )
DEFINE_GET_SET_BOOL ( CustomSmartQuotes )

DEFINE_GET_SET_BOOL ( OtherDirectionRtl )

DEFINE_GET_SET_BOOL ( AutoSaveFile )
DEFINE_GET_SET_BOOL ( EnableOverwrite )

// dummy implementations. XP pref backend isn't very smart.
#define DEFINE_GET_SET_BOOL_DUMMY(Bool)     \
    bool AP_UnixDialog_Options::_gather##Bool(void) {   \
        return m_bool##Bool;     \
    }        \
    void AP_UnixDialog_Options::_set##Bool(bool b) {   \
        m_bool##Bool = b;     \
    }


DEFINE_GET_SET_BOOL_DUMMY ( EnableSmoothScrolling )
DEFINE_GET_SET_BOOL_DUMMY ( PrefsAutoSave )
DEFINE_GET_SET_BOOL_DUMMY ( ViewAll )
DEFINE_GET_SET_BOOL_DUMMY ( ViewHiddenText )
DEFINE_GET_SET_BOOL_DUMMY ( ViewShowRuler )
DEFINE_GET_SET_BOOL_DUMMY ( ViewShowStatusBar )
DEFINE_GET_SET_BOOL_DUMMY ( ViewUnprintable )

void AP_UnixDialog_Options::_gatherAutoSaveFileExt ( UT_String &stRetVal )
{
    UT_ASSERT ( m_textAutoSaveFileExt && GTK_IS_EDITABLE ( m_textAutoSaveFileExt ) );
    char *tmp = gtk_editable_get_chars ( GTK_EDITABLE ( m_textAutoSaveFileExt ), 0, -1 );
    stRetVal = tmp;
    g_free ( tmp );
}

void AP_UnixDialog_Options::_setAutoSaveFileExt ( const UT_String &stExt )
{
    int pos = 0;
    UT_ASSERT ( m_textAutoSaveFileExt && GTK_IS_EDITABLE ( m_textAutoSaveFileExt ) );
    gtk_editable_delete_text ( GTK_EDITABLE ( m_textAutoSaveFileExt ), 0, -1 );
    gtk_editable_insert_text ( GTK_EDITABLE ( m_textAutoSaveFileExt ), stExt.c_str(), stExt.size(), &pos );
}

void AP_UnixDialog_Options::_gatherAutoSaveFilePeriod ( UT_String &stRetVal )
{
    UT_ASSERT ( m_textAutoSaveFilePeriod && GTK_IS_SPIN_BUTTON ( m_textAutoSaveFilePeriod ) );
    char nb[12];
    int val = gtk_spin_button_get_value_as_int ( GTK_SPIN_BUTTON ( m_textAutoSaveFilePeriod ) );
    g_snprintf ( nb, 12, "%d", val );
    stRetVal = nb;
}

void AP_UnixDialog_Options::_setAutoSaveFilePeriod ( const UT_String &stPeriod )
{
    UT_ASSERT ( m_textAutoSaveFilePeriod && GTK_IS_EDITABLE ( m_textAutoSaveFilePeriod ) );
    gtk_spin_button_set_value ( GTK_SPIN_BUTTON ( m_textAutoSaveFilePeriod ), atoi ( stPeriod.c_str() ) );
}

UT_Dimension AP_UnixDialog_Options::_gatherViewRulerUnits ( void )
{
    UT_ASSERT ( m_menuUnits && GTK_IS_COMBO_BOX ( m_menuUnits ) );
	return ( UT_Dimension ) XAP_comboBoxGetActiveInt(GTK_COMBO_BOX(m_menuUnits));
}

gint AP_UnixDialog_Options::_gatherOuterQuoteStyle ( void )
{
    UT_ASSERT ( m_omOuterQuoteStyle && GTK_IS_COMBO_BOX( m_omOuterQuoteStyle ) );
	return XAP_comboBoxGetActiveInt(GTK_COMBO_BOX(m_omOuterQuoteStyle));
}


gint AP_UnixDialog_Options::_gatherInnerQuoteStyle ( void )
{
    UT_ASSERT ( m_omInnerQuoteStyle && GTK_IS_COMBO_BOX ( m_omInnerQuoteStyle ) );
	return XAP_comboBoxGetActiveInt(GTK_COMBO_BOX(m_omInnerQuoteStyle));
}


// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void AP_UnixDialog_Options::_setViewRulerUnits ( UT_Dimension dim )
{
    UT_ASSERT ( m_menuUnits && GTK_COMBO_BOX ( m_menuUnits ) );

	XAP_comboBoxSetActiveFromIntCol(GTK_COMBO_BOX(m_menuUnits), 1, dim);
}
void AP_UnixDialog_Options::_setOuterQuoteStyle ( gint nIndex )
{
    UT_ASSERT ( m_omOuterQuoteStyle && GTK_COMBO_BOX ( m_omOuterQuoteStyle ) );
	XAP_comboBoxSetActiveFromIntCol(GTK_COMBO_BOX(m_omOuterQuoteStyle), 1, nIndex);
}

void AP_UnixDialog_Options::_setInnerQuoteStyle ( gint nIndex )
{
    UT_ASSERT ( m_omInnerQuoteStyle && GTK_COMBO_BOX ( m_omInnerQuoteStyle ) );
	XAP_comboBoxSetActiveFromIntCol(GTK_COMBO_BOX(m_omInnerQuoteStyle), 1, nIndex);
}

DEFINE_GET_SET_BOOL ( AutoLoadPlugins )

#undef DEFINE_GET_SET_BOOL


int AP_UnixDialog_Options::_gatherNotebookPageNum ( void )
{
    UT_ASSERT ( m_notebook && GTK_IS_NOTEBOOK ( m_notebook ) );
    return gtk_notebook_get_current_page ( GTK_NOTEBOOK ( m_notebook ) );
}

void AP_UnixDialog_Options::_setNotebookPageNum ( int pn )
{
    UT_ASSERT ( m_notebook && GTK_IS_NOTEBOOK ( m_notebook ) );
    gtk_notebook_set_current_page ( GTK_NOTEBOOK ( m_notebook ), pn );
}

/*static*/ void AP_UnixDialog_Options::s_defaults_clicked ( GtkWidget *widget, gpointer data )
{
    AP_UnixDialog_Options * dlg = static_cast<AP_UnixDialog_Options *> ( data );
    UT_UNUSED ( widget );
    UT_ASSERT ( widget && dlg );
    dlg->_event_SetDefaults();

#if 0
    // repopulate controls
    dlg->_populateWindowData();
#endif
}

/*static*/ void AP_UnixDialog_Options::s_control_changed ( GtkWidget *widget, gpointer data )
{
    guint id;
    UT_DEBUGMSG ( ( "Control changed\n" ) );
    AP_UnixDialog_Options *dlg = static_cast<AP_UnixDialog_Options *> ( data );
    UT_ASSERT ( widget && dlg );

    if ( dlg->isInitialPopulationHappenning() )
    {
        return;
    }

    id = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( widget ), "tControl" ) );
    dlg->_storeDataForControl ( static_cast <tControl> ( id ) );
}

/*static*/ void AP_UnixDialog_Options::s_chooseTransparentColor ( GtkWidget *widget, gpointer data )
{
    AP_UnixDialog_Options * dlg = static_cast<AP_UnixDialog_Options *> ( data );
    UT_UNUSED ( widget );
    UT_ASSERT ( widget && dlg );
    dlg->event_ChooseTransparentColor();
}


// These functions will allow multiple widgets to tie into the
// same logic functions (at the AP level) to enable/disable stuff.
/*static*/ void AP_UnixDialog_Options::s_checkbutton_toggle ( GtkWidget *w, gpointer data )
{
    AP_UnixDialog_Options * dlg = static_cast<AP_UnixDialog_Options *> ( data );
    UT_ASSERT ( dlg );
    UT_ASSERT ( w && GTK_IS_WIDGET ( w ) );

    int i = GPOINTER_TO_INT ( g_object_get_data ( G_OBJECT ( w ), "tControl" ) );
    UT_DEBUGMSG ( ( "s_checkbutton_toggle: control id = %d\n", i ) );
    dlg->_enableDisableLogic ( ( AP_Dialog_Options::tControl ) i );
}

/*static*/ void AP_UnixDialog_Options::s_auto_save_toggled ( GtkToggleButton *togglebutton, gpointer data )
{
    AP_UnixDialog_Options * dlg = static_cast<AP_UnixDialog_Options *> ( data );
    gboolean is_toggled;
    UT_ASSERT ( dlg );

    is_toggled = gtk_toggle_button_get_active ( togglebutton );
    gtk_widget_set_sensitive ( dlg->m_tableAutoSaveFile, is_toggled );
}


void AP_UnixDialog_Options::_storeWindowData ( void )
{
    AP_Dialog_Options::_storeWindowData();
}

void AP_UnixDialog_Options::_setupSmartQuotesCombos(  GtkWidget *optionmenu  )
{
	GtkComboBox * combo = GTK_COMBO_BOX(optionmenu);

	XAP_makeGtkComboBoxText(combo, G_TYPE_INT);

    UT_UCSChar wszDisplayString[4];
	for (size_t i = 0; XAP_EncodingManager::smartQuoteStyles[i].leftQuote != (UT_UCSChar)0; ++i)
	{
		wszDisplayString[0] = XAP_EncodingManager::smartQuoteStyles[i].leftQuote;
		wszDisplayString[1] = (gunichar)'O';
		wszDisplayString[2] = XAP_EncodingManager::smartQuoteStyles[i].rightQuote;
		wszDisplayString[3] = (gunichar)0;
        gchar* szDisplayStringUTF8 = g_ucs4_to_utf8 ( wszDisplayString, -1, NULL, NULL, NULL );
		XAP_appendComboBoxTextAndInt(combo, szDisplayStringUTF8, i);
        g_free ( szDisplayStringUTF8 );
	}
	gtk_combo_box_set_active(combo, 0);
}
