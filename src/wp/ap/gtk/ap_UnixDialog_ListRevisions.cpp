/* AbiWord
 * Copyright (C) 2002 Dom Lachowicz <cinamod@hotmail.com>
 * Copyright (c) 2009 Hubert Figuiere
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
#include "xap_Gtk2Compat.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_ListRevisions.h"
#include "ap_UnixDialog_ListRevisions.h"


void 
AP_UnixDialog_ListRevisions::select_row_cb(GtkTreeSelection * select, 
										   AP_UnixDialog_ListRevisions * me )
{
	GtkTreeIter iter;
	GtkTreeModel *model;
	if(gtk_tree_selection_get_selected(select, &model, &iter))
    {
        me->select_Row (iter);
    }
}


void 
AP_UnixDialog_ListRevisions::row_activated_cb(GtkTreeView *, 
											  GtkTreePath *, 
											  GtkTreeViewColumn*, 
											  AP_UnixDialog_ListRevisions * me) 
{
	UT_DEBUGMSG(("row_activated\n"));
	gtk_dialog_response(GTK_DIALOG(me->m_mainWindow), BUTTON_OK);
}


/*****************************************************************/

XAP_Dialog * AP_UnixDialog_ListRevisions::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_UnixDialog_ListRevisions * p = new AP_UnixDialog_ListRevisions(pFactory,id);
	return p;
}

AP_UnixDialog_ListRevisions::AP_UnixDialog_ListRevisions(XAP_DialogFactory * pDlgFactory,
							 XAP_Dialog_Id id)
  : AP_Dialog_ListRevisions(pDlgFactory,id)
  , m_mainWindow(NULL)
  , m_treeModel(NULL)
{
}

AP_UnixDialog_ListRevisions::~AP_UnixDialog_ListRevisions(void)
{
}

void AP_UnixDialog_ListRevisions::runModal(XAP_Frame * pFrame)
{
	m_mainWindow = constructWindow();
	UT_return_if_fail(m_mainWindow);

	switch ( abiRunModalDialog ( GTK_DIALOG(m_mainWindow),
								 pFrame, this, BUTTON_OK, false ) )
	{
		case BUTTON_OK:
			event_OK () ; break ;
		default:
			event_Cancel () ; break ;
	}

	abiDestroyWidget ( m_mainWindow ) ;
}

void AP_UnixDialog_ListRevisions::event_Cancel ()
{
  m_iId = 0 ;
  m_answer = AP_Dialog_ListRevisions::a_CANCEL ;
}

void AP_UnixDialog_ListRevisions::event_OK ()
{
  m_answer = AP_Dialog_ListRevisions::a_OK ;
}

void AP_UnixDialog_ListRevisions::select_Row (GtkTreeIter iter)
{
    guint t = 0;
    gtk_tree_model_get (GTK_TREE_MODEL(m_treeModel), &iter, COL_REVID, &t, -1);
    m_iId = t;
    UT_DEBUGMSG(("DOM: select row: %d\n", m_iId));  
}

void AP_UnixDialog_ListRevisions::unselect_Row()
{
  UT_DEBUGMSG(("DOM: unselect row: %d 0\n", m_iId));
  m_iId = 0 ;
}

GtkWidget * AP_UnixDialog_ListRevisions::constructWindow ()
{
  GtkWidget *ap_UnixDialog_ListRevisions;
  GtkWidget *vbDialog;
  GtkWidget *aaDialog;
	
  ap_UnixDialog_ListRevisions = abiDialogNew ( "list revisions dialog", TRUE, getTitle());	
#if !GTK_CHECK_VERSION(3,0,0)
  gtk_dialog_set_has_separator(GTK_DIALOG(ap_UnixDialog_ListRevisions), FALSE);
#endif
	
  gtk_window_set_modal (GTK_WINDOW (ap_UnixDialog_ListRevisions), TRUE);
  gtk_window_set_default_size ( GTK_WINDOW(ap_UnixDialog_ListRevisions), 800, 450 ) ;

  vbDialog = gtk_dialog_get_content_area(GTK_DIALOG(ap_UnixDialog_ListRevisions));
  gtk_widget_show (vbDialog);
  gtk_container_set_border_width (GTK_CONTAINER (vbDialog), 5);

  aaDialog = gtk_dialog_get_action_area(GTK_DIALOG(ap_UnixDialog_ListRevisions));
  gtk_widget_show (aaDialog);

  constructWindowContents ( vbDialog ) ;

  abiAddStockButton ( GTK_DIALOG(ap_UnixDialog_ListRevisions), GTK_STOCK_CANCEL, BUTTON_CANCEL ) ;
  abiAddStockButton ( GTK_DIALOG(ap_UnixDialog_ListRevisions), GTK_STOCK_OK, BUTTON_OK ) ;

  return ap_UnixDialog_ListRevisions;
}

void AP_UnixDialog_ListRevisions::constructWindowContents ( GtkWidget * vbDialog )
{
  GtkWidget *vbContent;
  GtkWidget *lbExistingRevisions;
  GtkWidget *swExistingRevisions;
  GtkWidget *clExistingRevisions;

  vbContent = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  gtk_widget_show (vbContent);
  gtk_container_add (GTK_CONTAINER (vbDialog), vbContent);
  gtk_container_set_border_width (GTK_CONTAINER (vbContent), 5);

  std::string s("<b>");
  s += getLabel1();
  s += "</b>";
  lbExistingRevisions = gtk_widget_new (GTK_TYPE_LABEL,
                                        "label", s.c_str(),
                                        "use-markup", TRUE,
                                        "xalign", 0.0, "yalign", 0.5,
                                        NULL);
  gtk_widget_show (lbExistingRevisions);
  gtk_box_pack_start (GTK_BOX (vbContent), lbExistingRevisions, FALSE, FALSE, 0);

  swExistingRevisions = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (swExistingRevisions);
  gtk_container_add (GTK_CONTAINER (vbContent), swExistingRevisions);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swExistingRevisions), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  m_treeModel = gtk_list_store_new(4, G_TYPE_UINT, G_TYPE_STRING,
                                   G_TYPE_STRING, G_TYPE_LONG );

  clExistingRevisions = gtk_tree_view_new_with_model (GTK_TREE_MODEL(m_treeModel));
  gtk_widget_show (clExistingRevisions);
  gtk_container_add (GTK_CONTAINER (swExistingRevisions), clExistingRevisions);

  // Note that columns are displayed in a different order to the model,
  // data from col2 is shown in the first column in the view.
  GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
  GtkTreeViewColumn *col;

  // comment column
  col = gtk_tree_view_column_new_with_attributes(getColumn3Label(),
												 renderer, "text", COL_COMMENT, NULL);
  gtk_tree_view_column_set_sort_column_id(col, COL_COMMENT);
  gtk_tree_view_append_column(GTK_TREE_VIEW(clExistingRevisions), col);

  // revision date column
  col = gtk_tree_view_column_new_with_attributes(getColumn2Label(),
												 renderer, "text", COL_DATE_STRING, NULL);
  // we sort on the numerical tt column instead of the human readable text
  gtk_tree_view_column_set_sort_column_id(col, COL_DATE_AS_TIMET);
  // later we sort on date desc.
  gtk_tree_view_column_set_sort_order( col, GTK_SORT_DESCENDING);
  gtk_tree_view_column_set_fixed_width(col, 80);
  gtk_tree_view_append_column(GTK_TREE_VIEW(clExistingRevisions), col);

  
  // revision # column
  col = gtk_tree_view_column_new_with_attributes(getColumn1Label(),
												 renderer, "text", COL_REVID, NULL);
  gtk_tree_view_column_set_fixed_width(col, 80);
  gtk_tree_view_column_set_sort_column_id(col, COL_REVID);
  gtk_tree_view_append_column(GTK_TREE_VIEW(clExistingRevisions), col);


  
  gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(clExistingRevisions), TRUE);


  
//  g_object_freeze_notify(G_OBJECT(m_treeModel));

  UT_uint32 itemCnt = getItemCount () ;

  UT_DEBUGMSG(("DOM: %d items\n", itemCnt));

  GtkTreeIter iter;
  for ( UT_uint32 i = 0; i < itemCnt; i++ )
  {
    char buf [ 35 ] ;

    g_snprintf(buf, 35, "%d", getNthItemId(i));
    gtk_list_store_append(m_treeModel, &iter);

    gchar * txt = getNthItemText(i, true);
    gchar * itemtime = g_locale_to_utf8(getNthItemTime(i), -1, NULL, NULL, NULL);
    gtk_list_store_set(m_treeModel, &iter,
                       COL_REVID,         getNthItemId(i),
                       COL_DATE_STRING,   itemtime?itemtime:"",
                       COL_COMMENT,       txt,
                       COL_DATE_AS_TIMET, getNthItemTimeT(i),
                       -1);
    UT_DEBUGMSG(("appending revision %s : %s, %s\n", itemtime, buf, txt));

    g_free(itemtime);

    FREEP(txt);
  }
//  g_object_thaw_notify(G_OBJECT(list_store));

//  gtk_clist_select_row (GTK_CLIST (clExistingRevisions), 0, 0);

  GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(clExistingRevisions));
  gtk_tree_selection_set_mode (select, GTK_SELECTION_SINGLE);
  g_signal_connect (G_OBJECT(select), "changed",
					G_CALLBACK(select_row_cb), this);

  g_signal_connect(G_OBJECT(clExistingRevisions),
		   "row-activated",
		   G_CALLBACK(row_activated_cb),
		   static_cast<gpointer>(this));

  gtk_tree_sortable_set_sort_column_id( GTK_TREE_SORTABLE(m_treeModel),
                                        3, GTK_SORT_DESCENDING );
}
