/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2004 Martin Sevior <msevior@physics.unimelb.edu.au>
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
#include "xap_Dlg_History.h"
#include "xap_UnixDlg_History.h"

/*****************************************************************/

#if 1
static void s_history_selected(GtkTreeView *treeview,
                            XAP_UnixDialog_History * dlg)
{
	UT_ASSERT(treeview && dlg);

	GtkTreeSelection * selection;
	GtkTreeIter iter;
	GtkTreeModel * model;
	UT_sint32 item;
	selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(treeview) );
	if (!selection || !gtk_tree_selection_get_selected (selection, &model, &iter)) {
		return;
	}
	UT_DEBUGMSG(("In s_history_selected \n"));
	// Get the row and col number
	GValue value;
	g_value_init(&value, G_TYPE_INVALID);
	gtk_tree_model_get_value (model, &iter,3,&value);
	item = g_value_get_int(&value);
	UT_DEBUGMSG(("Vlaue of id selected %d \n",item));
	dlg->setSelectionId(item);
}

#endif
XAP_Dialog * XAP_UnixDialog_History::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	XAP_UnixDialog_History * p = new XAP_UnixDialog_History(pFactory,id);
	return p;
}

XAP_UnixDialog_History::XAP_UnixDialog_History(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: XAP_Dialog_History(pDlgFactory,id),
	  m_pXML(NULL),
	  m_windowMain(NULL),
	  m_wListWindow(NULL),
	  m_wTreeView(NULL)
{
}

XAP_UnixDialog_History::~XAP_UnixDialog_History(void)
{
}

void XAP_UnixDialog_History::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	// build the dialog
	GtkWidget * cf = _constructWindow();    
	UT_return_if_fail(cf);	
	
	_populateWindowData();  

	switch (abiRunModalDialog ( GTK_DIALOG(cf), pFrame, this, GTK_RESPONSE_CLOSE,false ))
	{
	case GTK_RESPONSE_CLOSE:
		m_answer = a_CANCEL;
		break;
	case GTK_RESPONSE_OK:
		m_answer = a_OK;
		break;
	default:
		m_answer = a_CANCEL;
		break;
	}
	abiDestroyWidget(cf);
}

GtkWidget * XAP_UnixDialog_History::_constructWindow(void)
{
    const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	// get the path where our glade file is located
	XAP_UnixApp * pApp = static_cast<XAP_UnixApp*>(m_pApp);
	UT_String glade_path( pApp->getAbiSuiteAppGladeDir() );
	glade_path += "/xap_UnixDlg_History.glade";
	
	// load the dialog from the glade file
	m_pXML = abiDialogNewFromXML( glade_path.c_str() );
	if (!m_pXML)
		return NULL;

	// Update our member variables with the important widgets that 
	// might need to be queried or altered later
	m_windowMain = glade_xml_get_widget(m_pXML, "xap_UnixDlg_History");
	UT_ASSERT(m_windowMain);
	UT_UTF8String s;
	pSS->getValueUTF8(XAP_STRING_ID_DLG_History_WindowLabel,s);
	gtk_window_set_title (GTK_WINDOW(m_windowMain), s.utf8_str());
	m_wListWindow = glade_xml_get_widget(m_pXML, "wListWindow");

	_fillHistoryTree();

	// set the single selection mode for the TreeView
    gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (m_wTreeView)), GTK_SELECTION_SINGLE);	
	gtk_container_add (GTK_CONTAINER (m_wListWindow), m_wTreeView);
#if 1
	g_signal_connect_after(G_OBJECT(m_wTreeView),
						   "cursor-changed",
						   G_CALLBACK(s_history_selected),
						   static_cast<gpointer>(this));
#endif
	gtk_widget_show_all(m_wTreeView);	
	return m_windowMain;
}

void XAP_UnixDialog_History::_fillHistoryTree(void)
{

	UT_uint32 i;
	
	GtkTreeIter iter;

	GtkTreeStore * model = gtk_tree_store_new (4, // Total number of columns
                                          G_TYPE_STRING,   //Version number
										  G_TYPE_STRING, //           
										  G_TYPE_STRING,
										  G_TYPE_INT); //

	// build a list of all items
    for (i = 0; i < getListItemCount(); i++)
	{
		// Add a new row to the model
		gtk_tree_store_append (model, &iter,NULL);
		gtk_tree_store_set (model, &iter, 0, getListValue(i,0), 
							1,getListValue(i,1) ,
							2,getListValue(i,2) ,
							3,getListItemId(i) ,
							-1);
	}
    m_wTreeView = gtk_tree_view_new_with_model (GTK_TREE_MODEL (model));

	g_object_unref (model);	
//
// Renderer for the view
//
	GtkCellRenderer * renderer = gtk_cell_renderer_text_new ();
//
// Now create columns and add them to the tree
//
	GtkTreeViewColumn * column0 = gtk_tree_view_column_new_with_attributes (getListHeader(0), renderer,
                                                      "text", 0,
                                                      NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (m_wTreeView), column0);
	GtkTreeViewColumn * column1 = gtk_tree_view_column_new_with_attributes (getListHeader(1), renderer,
                                                      "text", 1,
                                                      NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (m_wTreeView), column1);
	GtkTreeViewColumn * column2 = gtk_tree_view_column_new_with_attributes (getListHeader(2), renderer,
                                                      "text", 2,
                                                      NULL);
	gtk_tree_view_append_column (GTK_TREE_VIEW (m_wTreeView), column2);
	
 	
	
	// now select first item in box
 	gtk_widget_grab_focus (m_wTreeView);

}

void XAP_UnixDialog_History::_populateWindowData(void)
{
    const XAP_StringSet * pSS = m_pApp->getStringSet();
	localizeLabelMarkup (glade_xml_get_widget(m_pXML, "lbDocumentDetails"), pSS, XAP_STRING_ID_DLG_History_DocumentDetails);
	setLabelMarkup (glade_xml_get_widget(m_pXML, "lbDocumentName"), getHeaderLabel(0));
	setLabelMarkup (glade_xml_get_widget(m_pXML, "lbDocNameVal"), getHeaderValue(0));
	setLabelMarkup (glade_xml_get_widget(m_pXML, "lbVersion"), getHeaderLabel(1));
	setLabelMarkup (glade_xml_get_widget(m_pXML, "lbVersionVal"), getHeaderValue(1));
	setLabelMarkup (glade_xml_get_widget(m_pXML, "lbCreated"), getHeaderLabel(2));
	setLabelMarkup (glade_xml_get_widget(m_pXML, "lbCreatedVal"), getHeaderValue(2));
	setLabelMarkup (glade_xml_get_widget(m_pXML, "lbSaved"), getHeaderLabel(3));
	setLabelMarkup (glade_xml_get_widget(m_pXML, "lbSavedVal"), getHeaderValue(3));
	setLabelMarkup (glade_xml_get_widget(m_pXML, "lbEditTime"), getHeaderLabel(4));
	setLabelMarkup (glade_xml_get_widget(m_pXML, "lbEditTimeVal"), getHeaderValue(4));
	setLabelMarkup (glade_xml_get_widget(m_pXML, "lbIdentifier"), getHeaderLabel(5));
	setLabelMarkup (glade_xml_get_widget(m_pXML, "lbIdentifierVal"), getHeaderValue(5));
	setLabelMarkup (glade_xml_get_widget(m_pXML, "lbVersionHistory"), getListTitle());

}


