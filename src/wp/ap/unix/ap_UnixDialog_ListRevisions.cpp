/* AbiWord
 * Copyright (C) 2002 Dom Lachowicz <cinamod@hotmail.com>
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

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_ListRevisions::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_UnixDialog_ListRevisions * p = new AP_UnixDialog_ListRevisions(pFactory,id);
	return p;
}

AP_UnixDialog_ListRevisions::AP_UnixDialog_ListRevisions(XAP_DialogFactory * pDlgFactory,
							 XAP_Dialog_Id id)
  : AP_Dialog_ListRevisions(pDlgFactory,id), mClist ( 0 )
{
}

AP_UnixDialog_ListRevisions::~AP_UnixDialog_ListRevisions(void)
{
}

void AP_UnixDialog_ListRevisions::runModal(XAP_Frame * pFrame)
{
	GtkWidget * mainWindow = constructWindow();
	UT_return_if_fail(mainWindow);

	switch ( abiRunModalDialog ( GTK_DIALOG(mainWindow),
								 pFrame, this, BUTTON_CANCEL, false ) )
	{
		case BUTTON_OK:
			event_OK () ; break ;
		default:
			event_Cancel () ; break ;
	}

	abiDestroyWidget ( mainWindow ) ;
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
  GtkWidget *dialog1;
  GtkWidget *dialog_vbox1;
  GtkWidget *dialog_action_area1;
	
  dialog1 = abiDialogNew ( "list revisions dialog", TRUE, getTitle());	
	
  gtk_window_set_modal (GTK_WINDOW (dialog1), TRUE);
  gtk_window_set_policy (GTK_WINDOW (dialog1), TRUE, TRUE, FALSE);
  gtk_widget_set_usize ( dialog1, 250, 250 ) ;

  dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
  gtk_widget_show (dialog_vbox1);
  gtk_container_set_border_width (GTK_CONTAINER (dialog_vbox1), 5);

  dialog_action_area1 = GTK_DIALOG (dialog1)->action_area;
  gtk_widget_show (dialog_action_area1);
  gtk_container_set_border_width (GTK_CONTAINER (dialog_action_area1), 10);

  constructWindowContents ( dialog_vbox1 ) ;

  abiAddStockButton ( GTK_DIALOG(dialog1), GTK_STOCK_CANCEL, BUTTON_CANCEL ) ;
  abiAddStockButton ( GTK_DIALOG(dialog1), GTK_STOCK_OK, BUTTON_OK ) ;

  return dialog1;
}

void AP_UnixDialog_ListRevisions::constructWindowContents ( GtkWidget * dialog_vbox1 )
{
  GtkWidget *frame1;
  GtkWidget *scrolledwindow1;
  GtkWidget *clist1;
  GtkWidget *rev_id;
  GtkWidget *comment;

  frame1 = gtk_frame_new (getLabel1());
  gtk_widget_show (frame1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), frame1, TRUE, TRUE, 0);
  gtk_container_set_border_width (GTK_CONTAINER (frame1), 5);

  scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
  gtk_widget_show (scrolledwindow1);
  gtk_container_add (GTK_CONTAINER (frame1), scrolledwindow1);
  gtk_container_set_border_width (GTK_CONTAINER (scrolledwindow1), 1);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);

  clist1 = gtk_clist_new (2);
  gtk_widget_show (clist1);
  gtk_container_add (GTK_CONTAINER (scrolledwindow1), clist1);
  gtk_clist_set_column_width (GTK_CLIST (clist1), 0, 80);
  gtk_clist_set_column_width (GTK_CLIST (clist1), 1, 80);
  gtk_clist_column_titles_show (GTK_CLIST (clist1));

  rev_id = gtk_label_new (getColumn1Label());
  gtk_widget_show (rev_id);
  gtk_clist_set_column_widget (GTK_CLIST (clist1), 0, rev_id);

  comment = gtk_label_new (getColumn2Label());
  gtk_widget_show (comment);
  gtk_clist_set_column_widget (GTK_CLIST (clist1), 1, comment);

  gtk_clist_freeze ( GTK_CLIST ( clist1 ) ) ;

  UT_uint32 itemCnt = getItemCount () ;

  UT_DEBUGMSG(("DOM: %d items\n", itemCnt));

  for ( UT_uint32 i = 0; i < itemCnt; i++ )
  {
    gchar * txt[3];
    gchar buf [ 35 ] ;

    sprintf ( buf, "%d", getNthItemId( i ) ) ;
    txt[0] = (gchar*)buf;
    txt[1] = (gchar*)getNthItemText ( i );
    txt[2] = NULL;

    gtk_clist_append ( GTK_CLIST(clist1), txt ) ;

    UT_DEBUGMSG(("DOM: appending revision %s : %s\n", txt[1], txt[0]));

    FREEP(txt[1]);
  }
  gtk_clist_thaw ( GTK_CLIST ( clist1 ) ) ;
  gtk_clist_select_row (GTK_CLIST (clist1), 0, 0);

  g_signal_connect (G_OBJECT(clist1), "select-row",
		    G_CALLBACK(select_row_callback), this);

  g_signal_connect (G_OBJECT(clist1), "unselect-row",
		    G_CALLBACK(select_row_callback), this);

  g_signal_connect(G_OBJECT(clist1),
		   "button_press_event",
		   G_CALLBACK(dblclick_callback),
		   (gpointer) this);

  mClist = clist1 ;
}
