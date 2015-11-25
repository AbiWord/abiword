/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2005 Robert Staudinger <robsta@stereolyzer.net>
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

#include "ap_Features.h"

#include "ut_types.h"
#include "ut_string.h"
#include "ut_std_string.h"
#include "ut_units.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
// FIXME: remove the following when dimensions are localized properly
#include "ut_locale.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "xap_UnixDialogHelper.h"

#include "fl_BlockLayout.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"
#include "xap_Prefs.h"

#include "ap_Dialog_Id.h"
#include "ap_Prefs_SchemeIds.h"

#include "ap_Strings.h"

#include "ap_UnixDialog_Tab.h"
#include "xap_Gtk2Compat.h"

//! Column indices for list-store
enum {
	COLUMN_TAB = 0,
	NUM_COLUMNS
};

//! Event dispatcher for default tab width
static void
AP_UnixDialog_Tab__onDefaultTabChanged (GtkSpinButton *widget,
										gpointer	   data) 
{
	AP_UnixDialog_Tab *dlg = static_cast<AP_UnixDialog_Tab*>(data);
	dlg->onDefaultTabChanged (gtk_spin_button_get_value (GTK_SPIN_BUTTON (widget)));
}

//! Event dispatcher for default tab width
static gboolean
AP_UnixDialog_Tab__onDefaultTabFocusOut (GtkWidget 	   * /*widget*/,
										 GdkEventFocus *event,
										 gpointer 		data)
{	
	if (event->type == GDK_FOCUS_CHANGE) {
		AP_UnixDialog_Tab *dlg = static_cast<AP_UnixDialog_Tab*>(data);
		dlg->onDefaultTabFocusOut ();
	}

	return FALSE;
}

//! Event dispatcher for tab list. 
static void
AP_UnixDialog_Tab__onTabSelected (GtkTreeSelection * /*selection*/,
								  gpointer 			data) 
{
	AP_UnixDialog_Tab *dlg = static_cast<AP_UnixDialog_Tab*>(data);
	dlg->onTabSelected ();
}

//! Event dispatcher for position spinner.
static void
AP_UnixDialog_Tab__onPositionChanged (GtkSpinButton *widget,
									  gpointer		 data) 
{
	AP_UnixDialog_Tab *dlg = static_cast<AP_UnixDialog_Tab*>(data);
	dlg->onPositionChanged (gtk_spin_button_get_value (GTK_SPIN_BUTTON (widget)));
}

//! Event dispatcher for position spinner.
static gboolean
AP_UnixDialog_Tab__onPositionFocusOut (GtkWidget 	 * /*widget*/,
									   GdkEventFocus *event,
									   gpointer 	  data)
{
	xxx_UT_DEBUGMSG (("onPositionFocusOut() '%d' \n", event->type));
	
	if (event->type == GDK_FOCUS_CHANGE) {
		AP_UnixDialog_Tab *dlg = static_cast<AP_UnixDialog_Tab*>(data);
		dlg->onPositionFocusOut ();
	}

	return FALSE;
}

//! Event dispatcher for alignment popup-menu.
static void
AP_UnixDialog_Tab__onAlignmentChanged (GtkComboBox * /*widget*/,
									   gpointer 	data) 
{
	AP_UnixDialog_Tab *dlg = static_cast<AP_UnixDialog_Tab*>(data);
	dlg->onAlignmentChanged ();
}

//! Event dispatcher for leader popup-menu.
static void
AP_UnixDialog_Tab__onLeaderChanged (GtkComboBox * /*widget*/,
									   gpointer  data) 
{
	AP_UnixDialog_Tab *dlg = static_cast<AP_UnixDialog_Tab*>(data);
	dlg->onLeaderChanged ();
}

//! Event dispatcher for button "Add".
static void
AP_UnixDialog_Tab__onAddTab (GtkButton * /*widget*/,
							 gpointer	data) 
{
	AP_UnixDialog_Tab *dlg = static_cast<AP_UnixDialog_Tab*>(data);
	dlg->onAddTab ();
}

//! Event dispatcher for button "Delete".
static void
AP_UnixDialog_Tab__onDeleteTab (GtkButton * /*widget*/,
								gpointer   data) 
{
	AP_UnixDialog_Tab *dlg = static_cast<AP_UnixDialog_Tab*>(data);
	dlg->onDeleteTab ();
}

//! Callback for closing window via window manager.
static gboolean
AP_UnixDialog_Tab__onCloseWindow (GtkWidget * /*widget*/,
								  GdkEvent  * /*event*/,
								  gpointer   data)
{
	AP_UnixDialog_Tab *dlg = static_cast<AP_UnixDialog_Tab*>(data);
	gtk_dialog_response (GTK_DIALOG (dlg->getWindow ()), GTK_RESPONSE_CLOSE);
	return TRUE;
}



//! Abi style static ctor
XAP_Dialog * 
AP_UnixDialog_Tab::static_constructor (XAP_DialogFactory *pDlgFactory,
									   XAP_Dialog_Id 	  id)
{
    AP_UnixDialog_Tab * p = new AP_UnixDialog_Tab (pDlgFactory, id);
    return p;
}

//! Ctor.
AP_UnixDialog_Tab::AP_UnixDialog_Tab (XAP_DialogFactory *pDlgFactory,
									  XAP_Dialog_Id 	 id)
  : AP_Dialog_Tab  (pDlgFactory, id),
	m_pBuilder(NULL),
	m_wDialog	   (NULL),
	m_sbDefaultTab (NULL),
	m_exUserTabs   (NULL),
	m_lvTabs	   (NULL),
	m_btDelete	   (NULL),
	m_sbPosition   (NULL),
	m_cobAlignment (NULL),
	m_cobLeader	   (NULL)
{
}

//! Dtor.
AP_UnixDialog_Tab::~AP_UnixDialog_Tab (void)
{
	gint i;

	for (i = 0; i < __FL_TAB_MAX; i++) {
		if (m_AlignmentMapping[i] != NULL)
			g_free (m_AlignmentMapping[i]);
			m_AlignmentMapping[i] = NULL;
	}

	for (i = 0; i < __FL_LEADER_MAX; i++) {
		if (m_LeaderMapping[i] != NULL)
			g_free (m_LeaderMapping[i]);
			m_LeaderMapping[i] = NULL;
	}

	if (m_pBuilder)
		g_object_unref(G_OBJECT(m_pBuilder));
}



//! Abi style run method.
void AP_UnixDialog_Tab::runModal (XAP_Frame *pFrame)
{
    // Build the window's widgets and arrange them
    m_wDialog = _constructWindow ();
    UT_return_if_fail (m_wDialog);

    // save for use with event
    m_pFrame = pFrame;

    // Populate the window's data items
    _populateWindowData ();

    abiRunModalDialog (GTK_DIALOG (m_wDialog), pFrame, this, GTK_RESPONSE_CLOSE, false);

	// TODO save state of expander

	// GtkWidget *wDialog = m_wDialog;
	// m_wDialog = NULL;
    // gtk_widget_destroy (wDialog);
    gtk_widget_destroy (m_wDialog);
	m_wDialog = NULL;
}

GtkWidget * 
AP_UnixDialog_Tab::_constructWindow ()
{
	// load the dialog from the UI file
	m_pBuilder = newDialogBuilder("ap_UnixDialog_Tab.ui");
	GtkWidget *wDialog = GTK_WIDGET(gtk_builder_get_object(m_pBuilder, "ap_UnixDialog_Tab"));
	m_exUserTabs = GTK_WIDGET(gtk_builder_get_object(m_pBuilder, "exUserTabs"));

	// localise	
	std::string s;
	const XAP_StringSet *pSS = m_pApp->getStringSet ();
	pSS->getValueUTF8 (AP_STRING_ID_DLG_Tab_TabTitle, s);
	gtk_window_set_title (GTK_WINDOW (wDialog), s.c_str());	
	
	localizeLabel (GTK_WIDGET(gtk_builder_get_object(m_pBuilder, "lbDefaultTab")), pSS, AP_STRING_ID_DLG_Tab_Label_DefaultTS);
	localizeLabelMarkup (GTK_WIDGET(gtk_builder_get_object(m_pBuilder, "lbUserTabs")), pSS, AP_STRING_ID_DLG_Tab_Label_Existing);
	localizeLabel (GTK_WIDGET(gtk_builder_get_object(m_pBuilder, "lbPosition")), pSS, AP_STRING_ID_DLG_Tab_Label_Position);
	localizeLabel (GTK_WIDGET(gtk_builder_get_object(m_pBuilder, "lbAlignment")), pSS, AP_STRING_ID_DLG_Tab_Label_Alignment);
	localizeLabel (GTK_WIDGET(gtk_builder_get_object(m_pBuilder, "lbLeader")), pSS, AP_STRING_ID_DLG_Tab_Label_Leader);


	// initialise

	m_sbDefaultTab = GTK_WIDGET(gtk_builder_get_object(m_pBuilder, "sbDefaultTab"));
	gtk_spin_button_set_digits (GTK_SPIN_BUTTON (m_sbDefaultTab), UT_getDimensionPrecisicion (m_dim));
 	// FIXME set max that fits on page gtk_spin_button_set_range (GTK_SPIN_BUTTON (m_sbDefaultTab),	0, ...);

	m_btDelete = GTK_WIDGET(gtk_builder_get_object(m_pBuilder, "btDelete"));

	m_sbPosition = GTK_WIDGET(gtk_builder_get_object(m_pBuilder, "sbPosition"));
	gtk_spin_button_set_digits (GTK_SPIN_BUTTON (m_sbPosition), UT_getDimensionPrecisicion (m_dim));
 	// FIXME set max that fits on page gtk_spin_button_set_range (GTK_SPIN_BUTTON (m_sbPosition),	0, ...);

	GtkWidget *tblNew = GTK_WIDGET(gtk_builder_get_object(m_pBuilder, "tblNew"));

	m_cobAlignment = gtk_combo_box_text_new ();
	gtk_widget_show (m_cobAlignment);
	gtk_table_attach (GTK_TABLE (tblNew), m_cobAlignment, 
					  1, 2, 1, 2,
					  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), GTK_EXPAND, 
					  0, 0);


	gchar *trans = NULL;

	// placeholder so we stick to the enum's ordering
	// does not show up in UI
	pSS->getValueUTF8 (AP_STRING_ID_DLG_Tab_Radio_NoAlign, s);
	UT_XML_cloneNoAmpersands(trans, s.c_str());
	m_AlignmentMapping[FL_TAB_NONE] = trans;

	pSS->getValueUTF8 (AP_STRING_ID_DLG_Tab_Radio_Left, s);
	UT_XML_cloneNoAmpersands(trans, s.c_str());
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (m_cobAlignment), trans);
	m_AlignmentMapping[FL_TAB_LEFT] = trans;

	pSS->getValueUTF8 (AP_STRING_ID_DLG_Tab_Radio_Center, s);
	UT_XML_cloneNoAmpersands(trans, s.c_str());
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (m_cobAlignment), trans);
	m_AlignmentMapping[FL_TAB_CENTER] = trans;

	pSS->getValueUTF8 (AP_STRING_ID_DLG_Tab_Radio_Right, s);
	UT_XML_cloneNoAmpersands(trans, s.c_str());
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (m_cobAlignment), trans);
	m_AlignmentMapping[FL_TAB_RIGHT] = trans;

	pSS->getValueUTF8 (AP_STRING_ID_DLG_Tab_Radio_Decimal, s);
	UT_XML_cloneNoAmpersands(trans, s.c_str());
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (m_cobAlignment), trans);
	m_AlignmentMapping[FL_TAB_DECIMAL] = trans;

	pSS->getValueUTF8 (AP_STRING_ID_DLG_Tab_Radio_Bar, s);
	UT_XML_cloneNoAmpersands(trans, s.c_str());
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (m_cobAlignment), trans);
	m_AlignmentMapping[FL_TAB_BAR] = trans;


	m_cobLeader = gtk_combo_box_text_new ();
	gtk_widget_show (m_cobLeader);
	gtk_table_attach (GTK_TABLE (tblNew), m_cobLeader, 
					  1, 2, 2, 3,
					  (GtkAttachOptions)(GTK_EXPAND | GTK_FILL), GTK_EXPAND, 
					  0, 0);

	pSS->getValueUTF8 (AP_STRING_ID_DLG_Tab_Radio_None, s);
	UT_XML_cloneNoAmpersands(trans, s.c_str());
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (m_cobLeader), trans);
	m_LeaderMapping[FL_LEADER_NONE] = trans;

	pSS->getValueUTF8 (AP_STRING_ID_DLG_Tab_Radio_Dot, s);
	UT_XML_cloneNoAmpersands(trans, s.c_str());
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (m_cobLeader), trans);
	m_LeaderMapping[FL_LEADER_DOT] = trans;

	pSS->getValueUTF8 (AP_STRING_ID_DLG_Tab_Radio_Dash, s);
	UT_XML_cloneNoAmpersands(trans, s.c_str());
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (m_cobLeader), trans);
	m_LeaderMapping[FL_LEADER_HYPHEN] = trans;

	pSS->getValueUTF8 (AP_STRING_ID_DLG_Tab_Radio_Underline, s);
	UT_XML_cloneNoAmpersands(trans, s.c_str());
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (m_cobLeader), trans);
	m_LeaderMapping[FL_LEADER_UNDERLINE] = trans;
	

	// liststore and -view
	m_lvTabs = GTK_WIDGET(gtk_builder_get_object(m_pBuilder, "lvTabs"));
	GtkListStore *store = gtk_list_store_new (NUM_COLUMNS, G_TYPE_STRING);
	gtk_tree_view_set_model (GTK_TREE_VIEW (m_lvTabs), GTK_TREE_MODEL (store));
	g_object_unref (G_OBJECT (store));

	// column
	GtkCellRenderer *renderer = NULL;
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (m_lvTabs),
												-1, "Name", renderer,
												"text", COLUMN_TAB,
												NULL);
	GtkTreeViewColumn *column = gtk_tree_view_get_column (GTK_TREE_VIEW (m_lvTabs), 0);
	gtk_tree_view_column_set_sort_column_id (column, COLUMN_TAB);

	// FIXME not implemented dialog before move to GtkBuilder
	m_LeaderMapping[FL_LEADER_THICKLINE] = NULL;
	m_LeaderMapping[FL_LEADER_EQUALSIGN] = NULL;

	_connectSignals (m_pBuilder);

	return wDialog;
}

//! Connect callbacks.
void
AP_UnixDialog_Tab::_connectSignals (GtkBuilder *builder)
{
    m_hSigDefaultTabChanged = g_signal_connect (m_sbDefaultTab, 
					  "value-changed", 
					  G_CALLBACK (AP_UnixDialog_Tab__onDefaultTabChanged), 
					  (gpointer)this);
	
	g_signal_connect (m_sbDefaultTab, 
					  "focus-out-event", 
					  G_CALLBACK (AP_UnixDialog_Tab__onDefaultTabFocusOut), 
					  (gpointer)this);
	

    m_tsSelection = gtk_tree_view_get_selection(GTK_TREE_VIEW (m_lvTabs));
    m_hTabSelected = g_signal_connect (m_tsSelection, 
					  "changed", 
					  G_CALLBACK (AP_UnixDialog_Tab__onTabSelected), 
					  (gpointer)this);


    m_hSigPositionChanged = g_signal_connect (m_sbPosition, 
					  "value-changed", 
					  G_CALLBACK (AP_UnixDialog_Tab__onPositionChanged), 
					  (gpointer)this);

	g_signal_connect (m_sbPosition, 
					  "focus-out-event", 
					  G_CALLBACK (AP_UnixDialog_Tab__onPositionFocusOut), 
					  (gpointer)this);


    m_hSigAlignmentChanged = g_signal_connect (m_cobAlignment, 
					  "changed", 
					  G_CALLBACK (AP_UnixDialog_Tab__onAlignmentChanged), 
					  (gpointer)this);
	
    m_hSigLeaderChanged = g_signal_connect (m_cobLeader, 
					  "changed", 
					  G_CALLBACK (AP_UnixDialog_Tab__onLeaderChanged), 
					  (gpointer)this);	

    g_signal_connect (GTK_WIDGET(gtk_builder_get_object(builder, "btAdd")), 
					  "clicked", 
					  G_CALLBACK (AP_UnixDialog_Tab__onAddTab), 
					  (gpointer)this);

    g_signal_connect (m_btDelete, 
					  "clicked", 
					  G_CALLBACK (AP_UnixDialog_Tab__onDeleteTab), 
					  (gpointer)this);

    g_signal_connect (GTK_WIDGET(gtk_builder_get_object(builder, "ap_UnixDialog_Tab")), 
					  "delete-event", 
					  G_CALLBACK (AP_UnixDialog_Tab__onCloseWindow), 
					  (gpointer)this);
}

/*!
* Auto-apply changed tab width.
* \param value width.
*/
void 
AP_UnixDialog_Tab::onDefaultTabChanged (double value)
{ 
	UT_Dimension dim = _getDimension ();
	const gchar *text = UT_formatDimensionString (dim, value);

	UT_DEBUGMSG(("onDefaultTabChanged() %s\n", text));

	gtk_entry_set_text (GTK_ENTRY (m_sbDefaultTab), text);

    _storeWindowData ();
}

//! Validate tab width, apply if ok.
void 
AP_UnixDialog_Tab::onDefaultTabFocusOut ()
{
	const gchar *text = gtk_entry_get_text (GTK_ENTRY (m_sbDefaultTab));
	UT_LocaleTransactor t(LC_NUMERIC, "C"); // FIXME: remove when we support localized dimensions
	if (UT_isValidDimensionString (text)) {
		// set
		float pos = strtof(text, NULL);
		if (pos != gtk_spin_button_get_value (GTK_SPIN_BUTTON (m_sbDefaultTab))) {

			UT_Dimension dim = UT_determineDimension(text, _getDimension ());
			UT_DEBUGMSG (("onDefaultTabFocusOut() %d/%d (%s/%f)\n", dim, _getDimension (), text, pos));
			if (dim != _getDimension ()) {
				pos = UT_convertDimensions(pos, dim, _getDimension ());
			}

			text = UT_formatDimensionString (dim, pos);
			UT_DEBUGMSG (("onDefaultTabFocusOut() '%s'\n", text));

			g_signal_handler_block (G_OBJECT (m_sbDefaultTab), m_hSigDefaultTabChanged);
			gtk_spin_button_set_value (GTK_SPIN_BUTTON (m_sbDefaultTab), pos);
			gtk_entry_set_text (GTK_ENTRY (m_sbDefaultTab), text);
			g_signal_handler_unblock (G_OBJECT (m_sbDefaultTab), m_hSigDefaultTabChanged);
		}
	}
	else {
		// reset
		UT_Dimension dim = _getDimension ();
		gdouble value = gtk_spin_button_get_value (GTK_SPIN_BUTTON (m_sbDefaultTab));
		const gchar *dimtext = UT_formatDimensionString (dim, value);

		g_signal_handler_block (G_OBJECT (m_sbDefaultTab), m_hSigDefaultTabChanged);
		gtk_entry_set_text (GTK_ENTRY (m_sbDefaultTab), dimtext);
		g_signal_handler_unblock (G_OBJECT (m_sbDefaultTab), m_hSigDefaultTabChanged);

	    _storeWindowData ();
	}

}

//! Add tab and apply to paragraph.
void 
AP_UnixDialog_Tab::onAddTab ()
{ 
	// Add tab one position after the last one
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (m_lvTabs));
	GtkTreeIter iter;
	char *value; 
	float pos, max = 0;
	if (gtk_tree_model_get_iter_first(model, &iter)) {
		do {
			gtk_tree_model_get (model, &iter, 0, &value, -1);
			pos = strtof (value, NULL);
			free(value);
			if (pos > max)
				max = pos;
		} while (gtk_tree_model_iter_next(model, &iter));
	}
 
	pos = gtk_spin_button_get_value (GTK_SPIN_BUTTON (m_sbDefaultTab));
	max += pos;
	std::string text = UT_formatDimensionString(_getDimension (), max);
	UT_DEBUGMSG (("onAddTab() '%s' (%f/%f)\n", text.c_str(), pos, max));
 
 	// set defaults

	g_signal_handler_block (G_OBJECT (m_sbPosition), m_hSigPositionChanged);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (m_sbPosition), pos);
	gtk_entry_set_text (GTK_ENTRY (m_sbPosition), text.c_str ());
	g_signal_handler_unblock (G_OBJECT (m_sbPosition), m_hSigPositionChanged);
	
	g_signal_handler_block (G_OBJECT (m_cobAlignment), m_hSigAlignmentChanged);
	gtk_combo_box_set_active (GTK_COMBO_BOX (m_cobAlignment), 0);
	g_signal_handler_unblock (G_OBJECT (m_cobAlignment), m_hSigAlignmentChanged);

	g_signal_handler_block (G_OBJECT (m_cobLeader), m_hSigLeaderChanged);
	gtk_combo_box_set_active (GTK_COMBO_BOX (m_cobLeader), 0);
	g_signal_handler_unblock (G_OBJECT (m_cobLeader), m_hSigLeaderChanged);

	_event_Set ();	
    _storeWindowData ();
}

//! Delete tab and apply to paragraph.
void 
AP_UnixDialog_Tab::onDeleteTab ()
{ 
	_event_Clear (); 
    _storeWindowData ();
}

//! Dispatch tab selection to xp land.
void 
AP_UnixDialog_Tab::onTabSelected ()
{
	gint ndx = _getSelectedIndex ();
	if (ndx > -1) {
		_event_TabSelected (ndx);
	}
}

/*!
* Apply changed position of a tab.
* \param value new position.
*/
void 
AP_UnixDialog_Tab::onPositionChanged (double value)
{
	UT_Dimension dim = _getDimension ();
	const gchar *text = UT_formatDimensionString (dim, value);
	g_signal_handler_block(G_OBJECT (m_sbPosition), m_hSigPositionChanged);
	g_signal_handler_block(G_OBJECT (m_tsSelection), m_hTabSelected);
	gtk_entry_set_text (GTK_ENTRY (m_sbPosition), text);

	_event_TabChange ();
	_event_Update ();
	g_signal_handler_unblock(G_OBJECT (m_tsSelection), m_hTabSelected);
	g_signal_handler_unblock(G_OBJECT (m_sbPosition), m_hSigPositionChanged);
}

//! Validate tab position, apply if ok.
void 
AP_UnixDialog_Tab::onPositionFocusOut ()
{
	const gchar *text = gtk_entry_get_text (GTK_ENTRY (m_sbPosition));
	UT_LocaleTransactor t(LC_NUMERIC, "C"); // FIXME: remove when we support localized dimensions
	if (UT_isValidDimensionString (text)) {
		// set
		float pos;
		sscanf (text, "%f", &pos);
		UT_Dimension dim = UT_determineDimension(text, _getDimension ());
		if (dim != _getDimension ()) {
			pos = UT_convertDimensions(pos, dim, _getDimension ());
		}

		const gchar *dimtext = UT_formatDimensionString (dim, pos);
		UT_DEBUGMSG (("onPositionFocusOut() '%s'\n", dimtext));

		g_signal_handler_block (G_OBJECT (m_sbPosition), m_hSigPositionChanged);
		gtk_spin_button_set_value (GTK_SPIN_BUTTON (m_sbPosition), pos);
		gtk_entry_set_text (GTK_ENTRY (m_sbPosition), dimtext);
		g_signal_handler_unblock (G_OBJECT (m_sbPosition), m_hSigPositionChanged);

		_event_Update ();
	}
	else {
		// reset
		UT_Dimension dim = _getDimension ();
		gdouble value = gtk_spin_button_get_value (GTK_SPIN_BUTTON (m_sbPosition));
		const gchar *dimtext = UT_formatDimensionString (dim, value);

		g_signal_handler_block (G_OBJECT (m_sbPosition), m_hSigPositionChanged);
		gtk_entry_set_text (GTK_ENTRY (m_sbPosition), dimtext);
		g_signal_handler_unblock (G_OBJECT (m_sbPosition), m_hSigPositionChanged);
	}
}

//! Dispatch event to xp-land and auto-apply.
void 
AP_UnixDialog_Tab::onAlignmentChanged ()
{
	_event_AlignmentChange ();
	_event_Update ();
}

//! Dispatch event to xp-land and auto-apply.
void 
AP_UnixDialog_Tab::onLeaderChanged ()
{
	_event_somethingChanged ();
	_event_Update ();
}



/*!
* Get index of selected tab in list.
* \return -1 if no selection
*/
UT_sint32
AP_UnixDialog_Tab::_getSelectedIndex ()
{
	GtkTreeIter iter;
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (m_lvTabs));
	GtkTreeModel *model = gtk_tree_view_get_model (GTK_TREE_VIEW (m_lvTabs));
	gboolean haveSelected = gtk_tree_selection_get_selected (selection, &model, &iter);

	if (haveSelected) {
		gchar *path_str = gtk_tree_model_get_string_from_iter (model, &iter);
		UT_DEBUGMSG (("%s\n", path_str));
		guint ndx = atoi (path_str);
		g_free (path_str);
		return ndx;
	}
	return -1;
}

//! Get alignment of selected tab.
eTabType 
AP_UnixDialog_Tab::_gatherAlignment ()
{
	const gchar *text =  gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (m_cobAlignment));
	for (guint i = 0; i < __FL_TAB_MAX; i++) {
		if (strcmp (text, m_AlignmentMapping[i]) == 0)
			return (eTabType)i;
	}
	return FL_TAB_NONE;
}

//! Set alignment of selected tab.
void 
AP_UnixDialog_Tab::_setAlignment (eTabType a)
{
	UT_DEBUGMSG (("ROB: _setAlignment() \n"));
	UT_return_if_fail (a < __FL_TAB_MAX);

	// FL_TAB_NONE == 0 ... not in the UI
	if (a > 0)
		a = (eTabType)((int)a - 1);

	g_signal_handler_block (G_OBJECT (m_cobAlignment), m_hSigAlignmentChanged);
	gtk_combo_box_set_active (GTK_COMBO_BOX (m_cobAlignment), a);
	g_signal_handler_unblock (G_OBJECT (m_cobAlignment), m_hSigAlignmentChanged);
}

//! Get leader of selected tab.
eTabLeader 
AP_UnixDialog_Tab::_gatherLeader ()
{
	const gchar *text =  gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (m_cobLeader));
	for (guint i = 0; i < __FL_LEADER_MAX; i++) {
		
		if (m_LeaderMapping[i] == NULL)
			break;

		UT_DEBUGMSG (("ROB: %d='%s' (%s)\n", i, m_LeaderMapping[i], text));
		if (strcmp (text, m_LeaderMapping[i]) == 0) 
			return (eTabLeader)i;
	}
	return FL_LEADER_NONE;
}

//! Set leader of selected tab.
void 
AP_UnixDialog_Tab::_setLeader (eTabLeader l)
{
	UT_DEBUGMSG (("ROB: _setLeader() \n"));
	UT_return_if_fail (l < __FL_LEADER_MAX);

	g_signal_handler_block (G_OBJECT (m_cobLeader), m_hSigLeaderChanged);
	gtk_combo_box_set_active (GTK_COMBO_BOX (m_cobLeader), l);
	g_signal_handler_unblock (G_OBJECT (m_cobLeader), m_hSigLeaderChanged);
}

//! Get default tab stop.
const gchar * 
AP_UnixDialog_Tab::_gatherDefaultTabStop ()
{
	double pos = gtk_spin_button_get_value (GTK_SPIN_BUTTON (m_sbDefaultTab));
	UT_Dimension dim = _getDimension ();
	const char *text = UT_formatDimensionString (dim, pos);
	UT_DEBUGMSG (("ROB: _gatherDefaultTabStop '%s'\n", text));
	return text;
}

//! Set default tab stop.
void 
AP_UnixDialog_Tab::_setDefaultTabStop (const gchar *defaultTabStop)
{
	UT_DEBUGMSG (("ROB: _setDefaultTabStop '%s'\n", defaultTabStop));

	UT_return_if_fail (defaultTabStop && *defaultTabStop && (defaultTabStop[0] != '0' || defaultTabStop[1] != '\0'));

	float pos;
	sscanf (defaultTabStop, "%f", &pos);

	UT_UTF8String text = defaultTabStop;
	if (!UT_hasDimensionComponent(defaultTabStop)) {
		UT_Dimension dim = _getDimension ();
		text = UT_formatDimensionString (dim, pos);
	}

	g_signal_handler_block (G_OBJECT (m_sbDefaultTab), m_hSigDefaultTabChanged);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (m_sbDefaultTab), pos);
	gtk_entry_set_text (GTK_ENTRY (m_sbDefaultTab), text.utf8_str());
	g_signal_handler_unblock (G_OBJECT (m_sbDefaultTab), m_hSigDefaultTabChanged);
}

//! Update list of tabs.
void 
AP_UnixDialog_Tab::_setTabList (UT_uint32 count)
{
	GtkListStore *store = GTK_LIST_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (m_lvTabs)));
	gtk_list_store_clear (store);

	GtkTreeIter iter;
	for (guint i = 0; i < count; i++) {

		gtk_list_store_append (store, &iter);
		gtk_list_store_set (store, &iter, 
							COLUMN_TAB, _getTabDimensionString(i),  
							-1);
	}

	// auto-expand if tabs
	if (count > 0) {
		gtk_expander_set_expanded (GTK_EXPANDER (m_exUserTabs), TRUE);
	}
}

//! Query currently selected tab index from xp class.
UT_sint32 
AP_UnixDialog_Tab::_gatherSelectTab ()
{
	return _getSelectedIndex ();
}

//! Set selected tab stop by index.
void 
AP_UnixDialog_Tab::_setSelectTab (UT_sint32 v)
{
	GtkTreeSelection *selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (m_lvTabs));
	if (v > -1) {
		GtkTreePath *path = gtk_tree_path_new_from_indices (v, -1);
		gtk_tree_selection_select_path (selection, path);
		gtk_tree_path_free (path);
	}
	else {
		 gtk_tree_selection_unselect_all (selection);
	}
}

//! Get selected tab stop position.
const char * 
AP_UnixDialog_Tab::_gatherTabEdit ()
{
	return gtk_entry_get_text (GTK_ENTRY (m_sbPosition));
}

//! Set selected tab stop position.
void 
AP_UnixDialog_Tab::_setTabEdit (const char *pszStr)
{
	UT_DEBUGMSG (("ROB: _setTabEdit '%s'\n", pszStr));

	float pos;
	UT_LocaleTransactor t(LC_NUMERIC, "C"); // FIXME: remove when we support localized dimensions
	sscanf (pszStr, "%f", &pos);

	g_signal_handler_block (G_OBJECT (m_sbPosition), m_hSigPositionChanged);
	gtk_spin_button_set_value (GTK_SPIN_BUTTON (m_sbPosition), pos);
	gtk_entry_set_text (GTK_ENTRY (m_sbPosition), pszStr);
	g_signal_handler_unblock (G_OBJECT (m_sbPosition), m_hSigPositionChanged);
}

//! Clear list of tab stops.
void 
AP_UnixDialog_Tab::_clearList ()
{
	GtkListStore *store = GTK_LIST_STORE (gtk_tree_view_get_model (GTK_TREE_VIEW (m_lvTabs)));
	gtk_list_store_clear (store);
}

/*!
* Query a widget by its XP-ID.
* Do not trust this implementation because the unix-dialog is quite different from the others
* (popup menus instead of radio buttons, no clear all ...).
*/
GtkWidget *
AP_UnixDialog_Tab::_lookupWidget (tControl id)
{
	UT_return_val_if_fail (id < id_last, NULL);

	switch (id) {

		case id_EDIT_TAB:
			return m_sbPosition;
		case id_LIST_TAB:
			return m_lvTabs;
		case id_SPIN_DEFAULT_TAB_STOP:
			return m_sbDefaultTab;

		case id_ALIGN_LEFT:
		case id_ALIGN_CENTER:
		case id_ALIGN_RIGHT:
		case id_ALIGN_DECIMAL:
		case id_ALIGN_BAR:
			// no reason to do anything particular with them
			// so we return the whole menu
			return m_cobAlignment;

		case id_LEADER_NONE:
		case id_LEADER_DOT:
		case id_LEADER_DASH:
		case id_LEADER_UNDERLINE:
			// no reason to do anything particular with them
			// so we return the whole menu
			return m_cobLeader;

		case id_BUTTON_SET:
			return GTK_WIDGET(gtk_builder_get_object(m_pBuilder, "btAdd"));
		case id_BUTTON_CLEAR:
		case id_BUTTON_CLEAR_ALL:
			return GTK_WIDGET(gtk_builder_get_object(m_pBuilder, "btDelete"));
		case id_BUTTON_OK:
		case id_BUTTON_CANCEL:
			return GTK_WIDGET(gtk_builder_get_object(m_pBuilder, "btClose"));
		
		default:
			return NULL;
	}
}

//! Set widget state by id.
void 
AP_UnixDialog_Tab::_controlEnable (tControl id, 
								   bool value)
{
	GtkWidget *w = _lookupWidget (id);
	UT_return_if_fail (w && GTK_IS_WIDGET (w));

	gtk_widget_set_sensitive (w, value);
	
	// HACK
	// en/dis able tab modification buttons with delete button
	// if we can delete we can also modify
	if (id == id_BUTTON_CLEAR) {
		gtk_widget_set_sensitive (GTK_WIDGET(gtk_builder_get_object(m_pBuilder, "tblNew")), value);
	}
}
