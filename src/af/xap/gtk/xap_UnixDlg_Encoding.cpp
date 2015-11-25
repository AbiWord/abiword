/* AbiSource Application Framework
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ut_string.h"
#include "ut_debugmsg.h"
#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "xap_Dialog_Id.h"
#include "xap_UnixDlg_Encoding.h"

XAP_Dialog * XAP_UnixDialog_Encoding::static_constructor(XAP_DialogFactory * pFactory,
							 XAP_Dialog_Id id)
{
  XAP_UnixDialog_Encoding * p = new XAP_UnixDialog_Encoding(pFactory,id);
  return p;
}

XAP_UnixDialog_Encoding::XAP_UnixDialog_Encoding(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
  : XAP_Dialog_Encoding(pDlgFactory,id)
{ 
}

XAP_UnixDialog_Encoding::~XAP_UnixDialog_Encoding(void)
{
}

void XAP_UnixDialog_Encoding::s_encoding_dblclicked(GtkTreeView * /*treeview*/,
													GtkTreePath * /*arg1*/,
													GtkTreeViewColumn * /*arg2*/,
													XAP_UnixDialog_Encoding * me)
{
	gtk_dialog_response (GTK_DIALOG(me->m_windowMain), GTK_RESPONSE_OK);
}

/*****************************************************************/

void XAP_UnixDialog_Encoding::runModal(XAP_Frame * pFrame)
{
  // Build the window's widgets and arrange them
  GtkWidget * mainWindow = _constructWindow();
  UT_return_if_fail(mainWindow);	
	
  // Populate the window's data items
  _populateWindowData();
  
  switch ( abiRunModalDialog ( GTK_DIALOG(mainWindow), pFrame, this, GTK_RESPONSE_CANCEL, false ) )
    {
    case GTK_RESPONSE_OK:
      event_Ok (); break;
    default:
      event_Cancel (); break;
    }
  abiDestroyWidget ( mainWindow ) ;
}

void XAP_UnixDialog_Encoding::event_Ok(void)
{
	GtkTreeSelection * selection;
	GtkTreeIter iter;
	GtkTreeModel * model;

	gint row = 0;

	selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(m_listEncodings) );

	// if there is no selection, or the selection's data (GtkListItem widget)
	// is empty, return cancel.  GTK can make this happen.
	if ( !selection || 
		 !gtk_tree_selection_get_selected (selection, &model, &iter)
	   )
	{
		_setAnswer (XAP_Dialog_Encoding::a_CANCEL);
		return;
	}

	// get the ID of the selected Type
	gtk_tree_model_get (model, &iter, 1, &row, -1);
  
	if (row >= 0) {
		_setSelectionIndex(static_cast<UT_uint32>(row));
		_setEncoding (_getAllEncodings()[row]);
		_setAnswer (XAP_Dialog_Encoding::a_OK);
	} else {
		UT_ASSERT_NOT_REACHED();
		_setAnswer (XAP_Dialog_Encoding::a_CANCEL);
	}
}

void XAP_UnixDialog_Encoding::event_Cancel(void)
{
	_setAnswer (XAP_Dialog_Encoding::a_CANCEL);
}

/*****************************************************************/

GtkWidget * XAP_UnixDialog_Encoding::_constructWindow(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;
	
	// load the dialog from the UI file
	GtkBuilder* builder = newDialogBuilder("xap_UnixDlg_Encoding.ui");
	
	// Update our member variables with the important widgets that 
	// might need to be queried or altered later
	m_windowMain = GTK_WIDGET(gtk_builder_get_object(builder, "xap_UnixDlg_Encoding"));
	m_listEncodings = GTK_WIDGET(gtk_builder_get_object(builder, "encodingList"));

	std::string s;
	pSS->getValueUTF8(XAP_STRING_ID_DLG_UENC_EncTitle,s);
	gtk_window_set_title (GTK_WINDOW(m_windowMain), s.c_str());
	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lblEncoding")), pSS, XAP_STRING_ID_DLG_UENC_EncLabel);

	// add a column to our TreeViews

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Format",
													   renderer,
													   "text", 
													   0,
													   NULL);
	gtk_tree_view_append_column( GTK_TREE_VIEW(m_listEncodings), column);
	
	// connect a dbl-clicked signal to the column
	
	g_signal_connect_after(G_OBJECT(m_listEncodings),
						   "row-activated",
						   G_CALLBACK(s_encoding_dblclicked),
						   static_cast<gpointer>(this));
  
	g_object_unref(G_OBJECT(builder));

	return m_windowMain;
}

void XAP_UnixDialog_Encoding::_populateWindowData(void)
{
	GtkListStore *model;
	GtkTreeIter iter;
	
	model = gtk_list_store_new (2, 
							    G_TYPE_STRING,
								G_TYPE_INT);
	
	for (UT_uint32 i = 0; i < _getEncodingsCount(); i++)
    {
		const gchar* s = _getAllEncodings()[i];
		
		// Add a new row to the model
		gtk_list_store_append (model, &iter);
		
		gtk_list_store_set (model, &iter,
							0, s,
							1, i,
							-1);
    } 
	
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_listEncodings), reinterpret_cast<GtkTreeModel *>(model));
	
	g_object_unref (model);	
	
	// now select first item in box
 	gtk_widget_grab_focus (m_listEncodings);
}
