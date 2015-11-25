/* AbiWord
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

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Field.h"
#include "ap_UnixDialog_Field.h"


/*****************************************************************/

#define	LIST_ITEM_INDEX_KEY "index"
#define CUSTOM_RESPONSE_INSERT 1

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_Field::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	return new AP_UnixDialog_Field(pFactory,id);
}

AP_UnixDialog_Field::AP_UnixDialog_Field(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_Field(pDlgFactory,id)
{
	m_windowMain = NULL;
	m_listTypes = NULL;
	m_listFields = NULL;
	m_entryParam = NULL;
}

AP_UnixDialog_Field::~AP_UnixDialog_Field(void)
{
}

/*****************************************************************/

static void s_types_clicked(GtkTreeView *treeview,
                            AP_UnixDialog_Field * dlg)
{
	UT_ASSERT(treeview && dlg);
	dlg->types_changed(treeview);
}

void AP_UnixDialog_Field::s_field_dblclicked(GtkTreeView * /*treeview*/,
											 GtkTreePath * /*arg1*/,
											 GtkTreeViewColumn * /*arg2*/,
											 AP_UnixDialog_Field * me)
{
	gtk_dialog_response (GTK_DIALOG(me->m_windowMain), CUSTOM_RESPONSE_INSERT);
}

/*****************************************************************/

void AP_UnixDialog_Field::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);
	
	// Build the window's widgets and arrange them
	m_windowMain = _constructWindow();
	UT_return_if_fail(m_windowMain);

	// Populate the window's data items
	_populateCatogries();

	switch ( abiRunModalDialog ( GTK_DIALOG(m_windowMain), pFrame, this,
								 CUSTOM_RESPONSE_INSERT, false ) )
	{
		case CUSTOM_RESPONSE_INSERT:
			event_Insert();
			break;
		default:
			m_answer = AP_Dialog_Field::a_CANCEL;
			break;
	}

        // We need to disconnect handler or answer will reset to "Cancel" during
        // destruction
        g_signal_handler_disconnect(G_OBJECT(m_listTypes), m_cursorChangedHandlerId);
        g_signal_handler_disconnect(G_OBJECT(m_listFields), m_rowActivatedHandlerId);
	abiDestroyWidget ( m_windowMain ) ;
}

void AP_UnixDialog_Field::event_Insert(void)
{
	UT_ASSERT(m_windowMain && m_listTypes && m_listFields);
	
	// find item selected in the Types list box, save it to m_iTypeIndex

	GtkTreeSelection * selection;
	GtkTreeIter iter;
	GtkTreeModel * model;

	selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(m_listTypes) );

	// if there is no selection, or the selection's data (GtkListItem widget)
	// is empty, return cancel.  GTK can make this happen.
	if ( !selection || 
		 !gtk_tree_selection_get_selected (selection, &model, &iter)
	   )
	{
		m_answer = AP_Dialog_Field::a_CANCEL;
		return;
	}

	// get the ID of the selected Type
	gtk_tree_model_get (model, &iter, 1, &m_iTypeIndex, -1);

	
	// find item selected in the Field list box, save it to m_iFormatIndex

	selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(m_listFields) );

	// if there is no selection, or the selection's data (GtkListItem widget)
	// is empty, return cancel.  GTK can make this happen.
	if ( !selection || 
		 !gtk_tree_selection_get_selected (selection, &model, &iter)
	   )
	{
		m_answer = AP_Dialog_Field::a_CANCEL;
		return;
	}

	// get the ID of the selected Type
	gtk_tree_model_get (model, &iter, 1, &m_iFormatIndex, -1);
	
	setParameter(gtk_entry_get_text(GTK_ENTRY(m_entryParam)));	
	m_answer = AP_Dialog_Field::a_OK;
}


void AP_UnixDialog_Field::types_changed(GtkTreeView *treeview)
{
	GtkTreeSelection * selection;
	GtkTreeIter iter;
	GtkTreeModel * model;

	selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(treeview) );

	// if there is no selection, or the selection's data (GtkListItem widget)
	// is empty, return cancel.  GTK can make this happen.
	if ( !selection || 
		 !gtk_tree_selection_get_selected (selection, &model, &iter)
	   )
	{
		m_answer = AP_Dialog_Field::a_CANCEL;
		return;
	}	

	// Update m_iTypeIndex with the row number
	gtk_tree_model_get (model, &iter, 1, &m_iTypeIndex, -1);	

	// Update the fields list with this new Type
	setFieldsList();
}

void AP_UnixDialog_Field::setTypesList(void)
{
	UT_ASSERT(m_listTypes);
	
	UT_sint32 i;
	
	GtkListStore *model;
	GtkTreeIter iter;
	
	model = gtk_list_store_new (2, 
							    G_TYPE_STRING,
								G_TYPE_INT
	                            );
	
 	// build a list of all items
    for (i = 0; fp_FieldTypes[i].m_Desc != NULL; i++)
	{
		// Add a new row to the model
		gtk_list_store_append (model, &iter);
		gtk_list_store_set (model, &iter,
					  		0, fp_FieldTypes[i].m_Desc,
							1, i,
					  		-1);
	}
	
	gtk_tree_view_set_model( GTK_TREE_VIEW(m_listTypes), reinterpret_cast<GtkTreeModel *>(model));

	g_object_unref (model);	
	
	// now select first item in box
 	gtk_widget_grab_focus (m_listTypes);

	GtkTreeSelection* selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(m_listTypes) );
	if (selection)
	  {
	    GtkTreePath * path = gtk_tree_path_new_first ();
	    gtk_tree_selection_select_path (selection, path);
	    gtk_tree_path_free (path);
	  }

	m_iTypeIndex = 0;
}

void AP_UnixDialog_Field::setFieldsList(void)
{
	UT_ASSERT(m_listFields);
	
	fp_FieldTypesEnum FType = fp_FieldTypes[m_iTypeIndex].m_Type;
	
	UT_sint32 i;
	
	GtkListStore *model;
	GtkTreeIter iter;
	
	model = gtk_list_store_new (2, 
							    G_TYPE_STRING,
								G_TYPE_INT
	                            );
	
 	// build a list of all items
    for (i = 0; fp_FieldFmts[i].m_Tag != NULL; i++)
	{
		if((fp_FieldFmts[i].m_Num != FPFIELD_endnote_anch) &&
		   (fp_FieldFmts[i].m_Num != FPFIELD_endnote_ref) &&
		   (fp_FieldFmts[i].m_Num != FPFIELD_footnote_anch) &&
		   (fp_FieldFmts[i].m_Num != FPFIELD_footnote_ref))
		{ 

			if (fp_FieldFmts[i].m_Type == FType)
			{
				// Add a new row to the model
				gtk_list_store_append (model, &iter);
				gtk_list_store_set (model, &iter,
									0, fp_FieldFmts[i].m_Desc,
									1, i,
									-1);
			}
		}
	}
	
	gtk_tree_view_set_model( GTK_TREE_VIEW(m_listFields), reinterpret_cast<GtkTreeModel *>(model));

	g_object_unref (model);
		
	// now select first item in box
 	gtk_widget_grab_focus (m_listFields);
}


/*****************************************************************/


GtkWidget * AP_UnixDialog_Field::_constructWindow(void)
{
	GtkWidget * window;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;

	GtkBuilder * builder = newDialogBuilder("ap_UnixDialog_Field.ui");
	
	// Update our member variables with the important widgets that 
	// might need to be queried or altered later
	window = GTK_WIDGET(gtk_builder_get_object(builder, "ap_UnixDialog_Field"));
	m_listTypes = GTK_WIDGET(gtk_builder_get_object(builder, "tvTypes"));
	m_listFields = GTK_WIDGET(gtk_builder_get_object(builder, "tvFields"));
	m_entryParam = GTK_WIDGET(gtk_builder_get_object(builder, "edExtraParameters"));
	
	// set the single selection mode for the TreeViews
    gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (m_listTypes)), GTK_SELECTION_SINGLE);	
    gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (m_listFields)), GTK_SELECTION_SINGLE);	

	// set the dialog title
	std::string s;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Field_FieldTitle,s);
	abiDialogSetTitle(window, "%s", s.c_str());
	
	// localize the strings in our dialog, and set some userdata for some widg

	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbTypes")), pSS, AP_STRING_ID_DLG_Field_Types_No_Colon);
	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbFields")), pSS, AP_STRING_ID_DLG_Field_Fields_No_Colon);
	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbExtraParameters")), pSS, AP_STRING_ID_DLG_Field_Parameters);
	localizeButtonUnderline(GTK_WIDGET(gtk_builder_get_object(builder, "btInsert")), pSS, AP_STRING_ID_DLG_InsertButton);

	// add a column to our TreeViews

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Format",
							 renderer,
							 "text", 
							 0,
							 NULL);
	gtk_tree_view_append_column( GTK_TREE_VIEW(m_listTypes), column);

	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Format",
							 renderer,
							 "text", 
							 0,
							 NULL);
	gtk_tree_view_append_column( GTK_TREE_VIEW(m_listFields), column);	

	// connect a clicked signal to the column

	m_cursorChangedHandlerId = g_signal_connect_after(G_OBJECT(m_listTypes),
						   "cursor-changed",
						   G_CALLBACK(s_types_clicked),
						   static_cast<gpointer>(this));

	m_rowActivatedHandlerId = g_signal_connect_after(G_OBJECT(m_listFields),
						   "row-activated",
						   G_CALLBACK(s_field_dblclicked),
						   static_cast<gpointer>(this));

	g_object_unref(G_OBJECT(builder));

	return window;
}

void AP_UnixDialog_Field::_populateCatogries(void)
{
	// Fill in the two lists
	setTypesList();
	setFieldsList();
}
