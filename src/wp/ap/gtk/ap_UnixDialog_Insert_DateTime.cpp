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
#include "ap_Dialog_Insert_DateTime.h"
#include "ap_UnixDialog_Insert_DateTime.h"

/*****************************************************************/

#define	LIST_ITEM_INDEX_KEY "index"
#define CUSTOM_RESPONSE_INSERT 1

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_Insert_DateTime::static_constructor(XAP_DialogFactory * pFactory,
															   XAP_Dialog_Id id)
{
	AP_UnixDialog_Insert_DateTime * p = new AP_UnixDialog_Insert_DateTime(pFactory,id);
	return p;
}

AP_UnixDialog_Insert_DateTime::AP_UnixDialog_Insert_DateTime(XAP_DialogFactory * pDlgFactory,
															 XAP_Dialog_Id id)
	: AP_Dialog_Insert_DateTime(pDlgFactory,id)
{
	m_windowMain = NULL;
	m_tvFormats = NULL;
}

AP_UnixDialog_Insert_DateTime::~AP_UnixDialog_Insert_DateTime(void)
{
}

/*****************************************************************/
/*****************************************************************/

void AP_UnixDialog_Insert_DateTime::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);
	
	// Build the window's widgets and arrange them
	m_windowMain = _constructWindow();
	UT_return_if_fail(m_windowMain);

	// Populate the window's data items
	_populateWindowData();

	switch(abiRunModalDialog(GTK_DIALOG(m_windowMain), pFrame, this,
							 CUSTOM_RESPONSE_INSERT, false ))
	{
		case CUSTOM_RESPONSE_INSERT:
			event_Insert();
			break;
		default:
			m_answer = AP_Dialog_Insert_DateTime::a_CANCEL;
			break;
	}

	abiDestroyWidget ( m_windowMain ) ;
}

void AP_UnixDialog_Insert_DateTime::s_date_dblclicked(GtkTreeView * /*treeview*/,
													  GtkTreePath * /*arg1*/,
													  GtkTreeViewColumn * /*arg2*/,
													  AP_UnixDialog_Insert_DateTime * me)
{
	gtk_dialog_response (GTK_DIALOG(me->m_windowMain), CUSTOM_RESPONSE_INSERT);
}

void AP_UnixDialog_Insert_DateTime::event_Insert(void)
{
	UT_ASSERT(m_windowMain && m_tvFormats);

	GtkTreeSelection * selection;
	GtkTreeIter iter;
	GtkTreeModel * model;

	selection = gtk_tree_view_get_selection( GTK_TREE_VIEW(m_tvFormats) );

	// if there is no selection, or the selection's data (GtkListItem widget)
	// is empty, return cancel.  GTK can make this happen.
	if ( !selection || 
		 !gtk_tree_selection_get_selected (selection, &model, &iter)
	   )
	{
		m_answer = AP_Dialog_Insert_DateTime::a_CANCEL;
		return;
	}

	// get the ID of the selected DataTime format	
	gtk_tree_model_get (model, &iter, 1, &m_iFormatIndex, -1);
	m_answer = AP_Dialog_Insert_DateTime::a_OK;
}

/*****************************************************************/
GtkWidget * AP_UnixDialog_Insert_DateTime::_constructWindow(void)
{
	GtkWidget * window;	
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *column;	
	
	GtkBuilder * builder = newDialogBuilder("ap_UnixDialog_Insert_DateTime.ui");
	
	// Update our member variables with the important widgets that 
	// might need to be queried or altered later
	window = GTK_WIDGET(gtk_builder_get_object(builder, "ap_UnixDialog_Insert_DateTime"));
	m_tvFormats = GTK_WIDGET(gtk_builder_get_object(builder, "tvFormats"));

	// set the single selection mode for the TreeView
    gtk_tree_selection_set_mode (gtk_tree_view_get_selection (GTK_TREE_VIEW (m_tvFormats)), GTK_SELECTION_SINGLE);		
	
	// set the dialog title
	std::string s;
	pSS->getValueUTF8(AP_STRING_ID_DLG_DateTime_DateTimeTitle,s);
	abiDialogSetTitle(window, "%s", s.c_str());
	
	// localize the strings in our dialog
	
	localizeLabelMarkup(GTK_WIDGET(gtk_builder_get_object(builder, "lbAvailableFormats")), pSS, AP_STRING_ID_DLG_DateTime_AvailableFormats);
	localizeButtonUnderline(GTK_WIDGET(gtk_builder_get_object(builder, "btInsert")), pSS, AP_STRING_ID_DLG_InsertButton);
	
	// add a column to our TreeView
	renderer = gtk_cell_renderer_text_new ();
	column = gtk_tree_view_column_new_with_attributes ("Format",
							 renderer,
							 "text", 
							 0,
							 NULL);
	gtk_tree_view_append_column( GTK_TREE_VIEW(m_tvFormats), column);	

	g_signal_connect_after(G_OBJECT(m_tvFormats),
						   "row-activated",
						   G_CALLBACK(s_date_dblclicked),
						   static_cast<gpointer>(this));
	
	g_object_unref(G_OBJECT(builder));
	return window;
}

void AP_UnixDialog_Insert_DateTime::_populateWindowData(void)
{
	UT_ASSERT(m_windowMain && m_tvFormats);

	// NOTE : this code is similar to the Windows dialog code to do
	// NOTE : the same thing.  if you are implementing this dialog
	// NOTE : for a new front end, this is the formatting logic 
	// NOTE : you'll want to use to populate your list

	UT_sint32 i;
	
	// this constant comes from ap_Dialog_Insert_DateTime.h
    char szCurrentDateTime[CURRENT_DATE_TIME_SIZE];
	
    time_t tim = time(NULL);
	
    struct tm *pTime = localtime(&tim);
	
	GtkListStore *model;
	GtkTreeIter iter;
	
	model = gtk_list_store_new (2, 
							    G_TYPE_STRING,
								G_TYPE_INT
	                            );
	
 	// build a list of all items
    for (i = 0; InsertDateTimeFmts[i] != NULL; i++)
	{
		gsize bytes_read = 0, bytes_written = 0;
		char * utf;

        strftime(szCurrentDateTime, CURRENT_DATE_TIME_SIZE, InsertDateTimeFmts[i], pTime);

		utf = g_locale_to_utf8(szCurrentDateTime, -1, &bytes_read, &bytes_written, NULL);
		if (utf) {
			// Add a new row to the model
			gtk_list_store_append (model, &iter);
			gtk_list_store_set (model, &iter,
								0, utf,
								1, i,
								-1);
		}
		g_free(utf);
	}
	
	gtk_tree_view_set_model( GTK_TREE_VIEW(m_tvFormats), reinterpret_cast<GtkTreeModel *>(model));
	
	g_object_unref (model);
	
	// now select first item in box
 	gtk_widget_grab_focus (m_tvFormats);
}
