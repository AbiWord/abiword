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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include <stdlib.h>
#include <glade/glade.h>

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

static void s_open(GObject * nil, AP_UnixDialog_MailMerge * me)
{
	me->eventOpen ();
}

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

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_MailMerge::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id id)
{
	AP_UnixDialog_MailMerge * p = new AP_UnixDialog_MailMerge(pFactory,id);
	return p;
}

AP_UnixDialog_MailMerge::AP_UnixDialog_MailMerge(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_MailMerge(pDlgFactory,id)
{
	m_windowMain = NULL;
}

AP_UnixDialog_MailMerge::~AP_UnixDialog_MailMerge(void)
{
}

/*****************************************************************/
/*****************************************************************/

void AP_UnixDialog_MailMerge::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);
	
	m_pFrame = pFrame;

    // Build the dialog's window
	_constructWindow();
	UT_return_if_fail(m_windowMain);

	switch ( abiRunModalDialog ( GTK_DIALOG(m_windowMain),
								 pFrame, this, GTK_RESPONSE_CANCEL, false ) )
	{
		case GTK_RESPONSE_OK:
			setAnswer(AP_Dialog_MailMerge::a_OK);
			break;
		default:
			setAnswer(AP_Dialog_MailMerge::a_CANCEL);
			break;
	}

	if (getAnswer() == AP_Dialog_MailMerge::a_OK)
		setMergeField (gtk_entry_get_text (GTK_ENTRY(m_entry)));
	
	abiDestroyWidget ( m_windowMain ) ;
}

/*****************************************************************/

void AP_UnixDialog_MailMerge::_constructWindow(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	// get the path where our glade file is located
	XAP_UnixApp * pApp = static_cast<XAP_UnixApp*>(m_pApp);
	UT_String glade_path( pApp->getAbiSuiteAppGladeDir() );
	glade_path += "/ap_UnixDialog_MailMerge.glade";
	
	// load the dialog from the glade file
	GladeXML *xml = abiDialogNewFromXML( glade_path.c_str() );
	
	// Update our member variables with the important widgets that 
	// might need to be queried or altered later
	m_windowMain = glade_xml_get_widget(xml, "ap_UnixDialog_MailMerge");
	m_entry = glade_xml_get_widget(xml, "mergeEntry");
	m_open = glade_xml_get_widget(xml, "loadBtn");
	m_treeview = glade_xml_get_widget(xml, "tvFields");

	// set the dialog title
	abiDialogSetTitle(m_windowMain, pSS->getValueUTF8(AP_STRING_ID_DLG_MailMerge_MailMergeTitle).c_str());
	
	// localize the strings in our dialog, and set tags for some widgets
	
	localizeLabelMarkup(glade_xml_get_widget(xml, "mergeLbl"), pSS, AP_STRING_ID_DLG_MailMerge_Insert);	

	g_signal_connect (G_OBJECT(m_open), "clicked", G_CALLBACK(s_open), this);
	g_signal_connect_after(G_OBJECT(m_treeview),
						   "cursor-changed",
						   G_CALLBACK(s_types_clicked),
						   static_cast<gpointer>(this));
}

void AP_UnixDialog_MailMerge::setFieldList()
{
	UT_uint32 i;
	
	GtkListStore *model;
	GtkTreeIter iter;
	
	UT_UTF8String * str;

	GtkCellRenderer * renderer = gtk_cell_renderer_text_new ();
	GtkTreeViewColumn * column = gtk_tree_view_column_new_with_attributes ("Format",
																		   renderer,
																		   "text", 
																		   0,
																		   NULL);

	model = (GtkListStore*)gtk_tree_view_get_model (GTK_TREE_VIEW(m_treeview));
	if (model) {
		column = gtk_tree_view_get_column (GTK_TREE_VIEW(m_treeview), 0);
		gtk_list_store_clear (model);
		gtk_tree_view_column_clear (column);
	}
	else {
		model = gtk_list_store_new (2,
									G_TYPE_STRING,
									G_TYPE_INT
			);
		gtk_tree_view_append_column( GTK_TREE_VIEW(m_treeview), column);
	}
	
 	// build a list of all items
    for (i = 0; i < m_vecFields.size(); i++)
	{
		str = (UT_UTF8String*)m_vecFields[i];

		// Add a new row to the model
		gtk_list_store_append (model, &iter);
		gtk_list_store_set (model, &iter,
					  		0, str->utf8_str(),
							1, i,
					  		-1);
	}
	
	gtk_tree_view_set_model(GTK_TREE_VIEW(m_treeview), 
							reinterpret_cast<GtkTreeModel *>(model));
	g_object_unref (model);	
	
	// now select first item in box
 	gtk_widget_grab_focus (m_treeview);
}

void AP_UnixDialog_MailMerge::fieldClicked(UT_uint32 index)
{
	UT_UTF8String * str = (UT_UTF8String*)m_vecFields[index];
	gtk_entry_set_text (GTK_ENTRY(m_entry), str->utf8_str());
}
