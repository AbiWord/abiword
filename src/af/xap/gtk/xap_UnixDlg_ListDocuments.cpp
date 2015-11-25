/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2004 Hubert Figuiere
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
#include <time.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_ListDocuments.h"
#include "xap_UnixDlg_ListDocuments.h"

#define CUSTOM_RESPONSE_VIEW 1

/*****************************************************************/

XAP_Dialog * XAP_UnixDialog_ListDocuments::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	XAP_UnixDialog_ListDocuments * p = new XAP_UnixDialog_ListDocuments(pFactory,id);
	return p;
}

XAP_UnixDialog_ListDocuments::XAP_UnixDialog_ListDocuments(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: XAP_Dialog_ListDocuments(pDlgFactory,id),
		m_listWindows(NULL),
		m_windowMain(NULL)
{
}

XAP_UnixDialog_ListDocuments::~XAP_UnixDialog_ListDocuments(void)
{
}

void XAP_UnixDialog_ListDocuments::s_list_dblclicked(GtkTreeView * /*treeview*/,
													 GtkTreePath * /*arg1*/,
													 GtkTreeViewColumn * /*arg2*/,
													 XAP_UnixDialog_ListDocuments * me)
{
	gtk_dialog_response (GTK_DIALOG(me->m_windowMain), CUSTOM_RESPONSE_VIEW);
}

void XAP_UnixDialog_ListDocuments::runModal(XAP_Frame * pFrame)
{
  // Build the window's widgets and arrange them
  GtkWidget * mainWindow = _constructWindow();
  UT_return_if_fail(mainWindow);
	
  // Populate the window's data items
  _populateWindowData();

  switch ( abiRunModalDialog ( GTK_DIALOG(mainWindow), pFrame, this, CUSTOM_RESPONSE_VIEW, false ) )
    {
    case CUSTOM_RESPONSE_VIEW:
      event_View () ; break ;
    default:
      event_Cancel (); break ;
    }

  abiDestroyWidget ( mainWindow ) ;
}

void XAP_UnixDialog_ListDocuments::event_View(void)
{
	GtkTreeSelection * selection;
	GtkTreeIter iter;
	GtkTreeModel * model;

	gint row = 0;

	selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(m_listWindows) );

	// if there is no selection, or the selection's data (GtkListItem widget)
	// is empty, return cancel.  GTK can make this happen.
	if ( !selection || 
		 !gtk_tree_selection_get_selected (selection, &model, &iter)
	   )
		return;
	
	// get the ID of the selected Type
	gtk_tree_model_get (model, &iter, 1, &row, -1);
	  
	if (row >= 0) {
		_setSelDocumentIndx(static_cast<UT_uint32>(row));
	}
}

void XAP_UnixDialog_ListDocuments::event_Cancel(void)
{
}

/*****************************************************************/

GtkWidget * XAP_UnixDialog_ListDocuments::_constructWindow(void)
{
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	GtkWidget *w;
	
	// load the dialog from the UI file
	GtkBuilder* builder = newDialogBuilder("xap_UnixDlg_ListDocuments.ui");
	
	// Update our member variables with the important widgets that 
	// might need to be queried or altered later
	m_windowMain = GTK_WIDGET(gtk_builder_get_object(builder, "xap_UnixDlg_ListDocuments"));
	m_listWindows = GTK_WIDGET(gtk_builder_get_object(builder, "tvAvailableDocuments"));

	gtk_window_set_title (GTK_WINDOW(m_windowMain), _getTitle());
	w = GTK_WIDGET(gtk_builder_get_object(builder, "lbAvailableDocuments"));
	setLabelMarkup(w, _getHeading());
	w = GTK_WIDGET(gtk_builder_get_object(builder, "btView"));
	gtk_button_set_label(GTK_BUTTON(w), _getOKButtonText());

	// add a column to our TreeViews

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Format",
													   renderer,
													   "text", 
													   0,
													   NULL);
	gtk_tree_view_append_column( GTK_TREE_VIEW(m_listWindows), column);
	
	// connect a dbl-clicked signal to the column
	
	g_signal_connect_after(G_OBJECT(m_listWindows),
						   "row-activated",
						   G_CALLBACK(s_list_dblclicked),
						   static_cast<gpointer>(this));
  
	g_object_unref(G_OBJECT(builder));

	return m_windowMain;
}

void XAP_UnixDialog_ListDocuments::_populateWindowData(void)
{
	GtkListStore *model;
	GtkTreeIter iter;
	
	model = gtk_list_store_new (2, 
							    G_TYPE_STRING,
								G_TYPE_INT);
	
	for (UT_sint32 i = 0; i < _getDocumentCount(); i++)
    {		
		const char *s = _getNthDocumentName(i);
		UT_return_if_fail(s);
		// Add a new row to the model
		gtk_list_store_append (model, &iter);
		
		gtk_list_store_set (model, &iter,
							0, s,
							1, i,
							-1);
    } 
	
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_listWindows), reinterpret_cast<GtkTreeModel *>(model));
	
	g_object_unref (model);	
	
	// now select first item in box
 	gtk_widget_grab_focus (m_listWindows);
}

