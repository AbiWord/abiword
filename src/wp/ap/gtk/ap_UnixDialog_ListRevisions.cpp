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
	if(gtk_tree_selection_get_selected(select, &model, &iter)) {
		GtkTreePath * path = gtk_tree_model_get_path(model, &iter);
		gint* rows = gtk_tree_path_get_indices(path);
		if(rows) {
			me->select_Row (*rows);
		}
		gtk_tree_path_free(path);
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

void AP_UnixDialog_ListRevisions::select_Row (gint which)
{
  m_iId = getNthItemId ( which ) ;
  UT_DEBUGMSG(("DOM: select row: %d, %d\n", which, m_iId));
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
	
  gtk_window_set_modal (GTK_WINDOW (ap_UnixDialog_ListRevisions), TRUE);
  gtk_window_set_default_size ( GTK_WINDOW(ap_UnixDialog_ListRevisions), 250, 250 ) ;

  vbDialog = GTK_DIALOG (ap_UnixDialog_ListRevisions)->vbox;
  gtk_widget_show (vbDialog);
  gtk_container_set_border_width (GTK_CONTAINER (vbDialog), 5);

  aaDialog = GTK_DIALOG (ap_UnixDialog_ListRevisions)->action_area;
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

  vbContent = gtk_vbox_new (FALSE, 6);
  gtk_widget_show (vbContent);
  gtk_container_add (GTK_CONTAINER (vbDialog), vbContent);
  gtk_container_set_border_width (GTK_CONTAINER (vbContent), 5);

  lbExistingRevisions = gtk_label_new (getLabel1());
  gtk_widget_show (lbExistingRevisions);
  gtk_misc_set_alignment (GTK_MISC (lbExistingRevisions), 0.0, 0.5);
  gtk_box_pack_start (GTK_BOX (vbContent), lbExistingRevisions, FALSE, FALSE, 0);

  swExistingRevisions = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (swExistingRevisions);
  gtk_container_add (GTK_CONTAINER (vbContent), swExistingRevisions);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (swExistingRevisions), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  GtkListStore * list_store = gtk_list_store_new(3, G_TYPE_STRING, 
												 G_TYPE_STRING, G_TYPE_STRING);

  clExistingRevisions = gtk_tree_view_new_with_model (GTK_TREE_MODEL(list_store));
  gtk_widget_show (clExistingRevisions);
  gtk_container_add (GTK_CONTAINER (swExistingRevisions), clExistingRevisions);
  
  GtkCellRenderer *renderer = gtk_cell_renderer_text_new ();
  GtkTreeViewColumn *col;
  col = gtk_tree_view_column_new_with_attributes(getColumn1Label(),
												 renderer, "text", 0, NULL);
  gtk_tree_view_column_set_fixed_width(col, 80);
  gtk_tree_view_append_column(GTK_TREE_VIEW(clExistingRevisions), col);
  col = gtk_tree_view_column_new_with_attributes(getColumn2Label(),
												 renderer, "text", 1, NULL);
  gtk_tree_view_column_set_fixed_width(col, 80);
  gtk_tree_view_append_column(GTK_TREE_VIEW(clExistingRevisions), col);
  col = gtk_tree_view_column_new_with_attributes(getColumn3Label(),
												 renderer, "text", 2, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(clExistingRevisions), col);

  gtk_tree_view_set_headers_visible(GTK_TREE_VIEW(clExistingRevisions), TRUE);

//  g_object_freeze_notify(G_OBJECT(list_store));

  UT_uint32 itemCnt = getItemCount () ;

  UT_DEBUGMSG(("DOM: %d items\n", itemCnt));

  GtkTreeIter iter;
  for ( UT_uint32 i = 0; i < itemCnt; i++ )
  {
    gchar buf [ 35 ] ;
	
    sprintf (buf, "%d", getNthItemId(i));
	gtk_list_store_append(list_store, &iter);

	gchar * txt = getNthItemText(i);
	const gchar * itemtime = getNthItemTime(i);
	gtk_list_store_set(list_store, &iter, 
					   0, buf, 
					   1, itemtime?itemtime:"",
					   2, txt, 
					   -1);

    UT_DEBUGMSG(("appending revision %s : %s, %s\n", itemtime, buf, txt));

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
}
