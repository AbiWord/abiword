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

#undef GTK_DISABLE_DEPRECATED

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
								 pFrame, this, BUTTON_OK, false ) )
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
  GtkWidget *lbColumnRevisionID;
  GtkWidget *lbColumnDate;
  GtkWidget *lbColumnComment;

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

  clExistingRevisions = gtk_clist_new (3);
  gtk_widget_show (clExistingRevisions);
  gtk_container_add (GTK_CONTAINER (swExistingRevisions), clExistingRevisions);
  gtk_clist_set_column_width (GTK_CLIST (clExistingRevisions), 0, 80);
  gtk_clist_set_column_width (GTK_CLIST (clExistingRevisions), 1, 80);
  gtk_clist_column_titles_show (GTK_CLIST (clExistingRevisions));

  lbColumnRevisionID = gtk_label_new (getColumn1Label());
  gtk_widget_show (lbColumnRevisionID);
  gtk_clist_set_column_widget (GTK_CLIST (clExistingRevisions), 0, lbColumnRevisionID);

  lbColumnDate = gtk_label_new (getColumn2Label());
  gtk_widget_show (lbColumnDate);
  gtk_clist_set_column_widget (GTK_CLIST (clExistingRevisions), 1, lbColumnDate);

  lbColumnComment = gtk_label_new (getColumn3Label());
  gtk_widget_show (lbColumnComment);
  gtk_clist_set_column_widget (GTK_CLIST (clExistingRevisions), 2, lbColumnComment);

  gtk_clist_freeze ( GTK_CLIST ( clExistingRevisions ) ) ;

  UT_uint32 itemCnt = getItemCount () ;

  UT_DEBUGMSG(("DOM: %d items\n", itemCnt));

  for ( UT_uint32 i = 0; i < itemCnt; i++ )
  {
    gchar * txt[4];
    gchar buf [ 35 ] ;

    sprintf ( buf, "%d", getNthItemId( i ) ) ;
    txt[0] = static_cast<gchar*>(buf);
    txt[1] = const_cast<gchar*>(getNthItemTime ( i ));
    txt[2] = static_cast<gchar*>(getNthItemText ( i ));
    txt[3] = NULL;

    gtk_clist_append ( GTK_CLIST(clExistingRevisions), txt ) ;

    UT_DEBUGMSG(("DOM: appending revision %s : %s\n", txt[1], txt[0]));

    FREEP(txt[2]);
  }
  gtk_clist_thaw ( GTK_CLIST ( clExistingRevisions ) ) ;
  gtk_clist_select_row (GTK_CLIST (clExistingRevisions), 0, 0);

  g_signal_connect (G_OBJECT(clExistingRevisions), "select-row",
		    G_CALLBACK(select_row_callback), this);

  g_signal_connect (G_OBJECT(clExistingRevisions), "unselect-row",
		    G_CALLBACK(select_row_callback), this);

  g_signal_connect(G_OBJECT(clExistingRevisions),
		   "button_press_event",
		   G_CALLBACK(dblclick_callback),
		   static_cast<gpointer>(this));

  mClist = clExistingRevisions ;
}
