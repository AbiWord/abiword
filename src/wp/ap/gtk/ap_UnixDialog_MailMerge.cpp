/* AbiWord
 * Copyright (C) 2003 Dom Lachowicz
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

#include <stdlib.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "ap_UnixApp.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_MailMerge.h"
#include "ap_UnixDialog_MailMerge.h"

/*****************************************************************/

#define CUSTOM_RESPONSE_OPEN_FILE 2
#define CUSTOM_RESPONSE_INSERT 1

/*****************************************************************/

static void s_types_clicked(GtkTreeView *treeview,
                            AP_UnixDialog_MailMerge * dlg)
{
	UT_ASSERT(treeview && dlg);

	GtkTreeSelection * selection;
	GtkTreeIter iter;
	GtkTreeModel * model;

	UT_uint32 typeIndex;

	selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(treeview) );
	if (!selection || !gtk_tree_selection_get_selected (selection, &model, &iter)) {
		return;
	}

	// Update m_iTypeIndex with the row number
	gtk_tree_model_get (model, &iter, 1, &typeIndex, -1);

	dlg->fieldClicked(typeIndex);
}

static void s_types_dblclicked(GtkTreeView *treeview,
							   GtkTreePath * /*arg1*/,
							   GtkTreeViewColumn * /*arg2*/,
							   AP_UnixDialog_MailMerge * me)
{
	// simulate the effects of a single click
	s_types_clicked (treeview, me);
	me->event_AddClicked ();
}

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_MailMerge::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id id)
{
	return new AP_UnixDialog_MailMerge(pFactory,id);
}

AP_UnixDialog_MailMerge::AP_UnixDialog_MailMerge(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_MailMerge(pDlgFactory,id), m_windowMain(NULL)
{
}

AP_UnixDialog_MailMerge::~AP_UnixDialog_MailMerge(void)
{
}

/*****************************************************************/
/*****************************************************************/

void AP_UnixDialog_MailMerge::runModeless(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);
	
	m_pFrame = pFrame;

    // Build the dialog's window
	m_windowMain = _constructWindow();
	UT_return_if_fail(m_windowMain);

	abiSetupModelessDialog(GTK_DIALOG(m_windowMain),
						   pFrame, this, GTK_RESPONSE_CANCEL);
	init ();
}

/*****************************************************************/

void AP_UnixDialog_MailMerge::event_Close()
{
	destroy();
}

static void s_destroy_clicked(GtkWidget * /*widget*/,
							  AP_UnixDialog_MailMerge * dlg)
{
	dlg->event_Close();
}

static void s_delete_clicked(GtkWidget * widget,
							 gpointer,
							 gpointer * /*dlg*/)
{
	abiDestroyWidget(widget);
}

static void s_response_triggered(GtkWidget * widget, gint resp, AP_UnixDialog_MailMerge * dlg)
{
	UT_return_if_fail(widget && dlg);
	
	if ( resp == CUSTOM_RESPONSE_INSERT )
	  dlg->event_AddClicked();
	else if ( resp == CUSTOM_RESPONSE_OPEN_FILE )
	  dlg->eventOpen ();
	else
	  abiDestroyWidget ( widget ) ; // will trigger other events
}

void AP_UnixDialog_MailMerge::event_AddClicked ()
{
	setMergeField (gtk_entry_get_text (GTK_ENTRY(m_entry)));
	addClicked();
}

GtkWidget * AP_UnixDialog_MailMerge::_constructWindow(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	// load the dialog from the UI file
#if GTK_CHECK_VERSION(3,0,0)
	GtkBuilder* builder = newDialogBuilder("ap_UnixDialog_MailMerge.ui");
#else
	GtkBuilder* builder = newDialogBuilder("ap_UnixDialog_MailMerge-2.ui");
#endif
	
	// Update our member variables with the important widgets that 
	// might need to be queried or altered later
	m_windowMain = GTK_WIDGET(gtk_builder_get_object(builder, "ap_UnixDialog_MailMerge"));
	m_entry = GTK_WIDGET(gtk_builder_get_object(builder, "edFieldName"));
	m_treeview = GTK_WIDGET(gtk_builder_get_object(builder, "tvAvailableFields"));

	// set the single selection mode for the TreeView
    gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (m_treeview)), GTK_SELECTION_SINGLE);	

	// set the dialog title
	std::string s;
	pSS->getValueUTF8(AP_STRING_ID_DLG_MailMerge_MailMergeTitle,s);
	abiDialogSetTitle(m_windowMain, "%s", s.c_str());
	
	// localize the strings in our dialog, and set tags for some widgets
	
	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbAvailableFields")), pSS, AP_STRING_ID_DLG_MailMerge_AvailableFields);

	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbFieldName")), pSS, AP_STRING_ID_DLG_MailMerge_Insert);	

	localizeLabel(GTK_WIDGET(gtk_builder_get_object(builder, "lbOpenFile")), pSS, AP_STRING_ID_DLG_MailMerge_OpenFile);	

	localizeButtonUnderline(GTK_WIDGET(gtk_builder_get_object(builder, "btInsert")), pSS, AP_STRING_ID_DLG_InsertButton);

	g_signal_connect_after(G_OBJECT(m_treeview),
						   "cursor-changed",
						   G_CALLBACK(s_types_clicked),
						   static_cast<gpointer>(this));
	g_signal_connect_after(G_OBJECT(m_treeview),
						   "row-activated",
						   G_CALLBACK(s_types_dblclicked),
						   static_cast<gpointer>(this));

	g_signal_connect(G_OBJECT(m_windowMain), "response", 
					 G_CALLBACK(s_response_triggered), this);
	// the catch-alls
	// Dont use gtk_signal_connect_after for modeless dialogs
	g_signal_connect(G_OBJECT(m_windowMain),
			   "destroy",
			   G_CALLBACK(s_destroy_clicked),
			   (gpointer) this);
	g_signal_connect(G_OBJECT(m_windowMain),
			   "delete_event",
			   G_CALLBACK(s_delete_clicked),
			   (gpointer) this);

	g_object_unref(G_OBJECT(builder));
			   
	return m_windowMain;			   
}

void AP_UnixDialog_MailMerge::setFieldList()
{
	if(!m_vecFields.size())
		return;

	UT_sint32 i;
	
	GtkTreeIter iter;
	
	GtkTreeViewColumn * column;
	GtkListStore * model;

	model = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_INT);
	column = gtk_tree_view_get_column (GTK_TREE_VIEW(m_treeview), 0);
	if (!column) {
		column = gtk_tree_view_column_new_with_attributes ("Format", gtk_cell_renderer_text_new (), "text", 0, NULL);
		gtk_tree_view_append_column(GTK_TREE_VIEW(m_treeview), column);
	}
	
 	// build a list of all items
	for (i = 0; i < m_vecFields.size(); i++)
	{
		const std::string & str = m_vecFields[i];

		// Add a new row to the model
		gtk_list_store_append (model, &iter);
		gtk_list_store_set (model, &iter, 0, str.c_str(), 1, i, -1);
	}
	
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_treeview), 
							reinterpret_cast<GtkTreeModel *>(model));
	g_object_unref (model);	
	
	// now select first item in box
 	gtk_widget_grab_focus (m_treeview);
}

void AP_UnixDialog_MailMerge::fieldClicked(UT_uint32 index)
{
	const std::string & str = m_vecFields[index];
	gtk_entry_set_text (GTK_ENTRY(m_entry), str.c_str());
}
