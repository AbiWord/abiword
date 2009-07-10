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
#include "ap_Dialog_MarkRevisions.h"
#include "ap_UnixDialog_MarkRevisions.h"

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_MarkRevisions::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	return new AP_UnixDialog_MarkRevisions(pFactory,id);
}

AP_UnixDialog_MarkRevisions::AP_UnixDialog_MarkRevisions(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_MarkRevisions(pDlgFactory,id)
{
	mRadio1  = mRadio2 = mComment = mEntryLbl = NULL ;
}

AP_UnixDialog_MarkRevisions::~AP_UnixDialog_MarkRevisions(void)
{
}

void AP_UnixDialog_MarkRevisions::runModal(XAP_Frame * pFrame)
{
	GtkWidget * mainWindow = constructWindow();
	UT_return_if_fail(mainWindow);
	
	// toggle what should be grayed and what shouldn't be
	event_FocusToggled () ;
   
	switch ( abiRunModalDialog ( GTK_DIALOG(mainWindow),
								 pFrame, this, BUTTON_CANCEL, false ) )
	{
		case BUTTON_OK:
			event_OK () ; break ;
		default:
			event_Cancel () ; break ;
	}
  
  /*if(mainWindow && GTK_IS_WIDGET(mainWindow))
    gtk_widget_destroy(mainWindow);*/
	abiDestroyWidget ( mainWindow ) ;
}

GtkWidget * AP_UnixDialog_MarkRevisions::constructWindow ()
{
  const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

  GtkWidget *dialog1;
  GtkWidget *dialog_vbox1;
  GtkWidget *dialog_action_area1;
  UT_UTF8String s;
  pSS->getValueUTF8(AP_STRING_ID_DLG_MarkRevisions_Title,s);
  dialog1 = abiDialogNew ( "mark revisions", TRUE, s.utf8_str());
  gtk_window_set_default_size ( GTK_WINDOW(dialog1), 250, 150 ) ;

  dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
  gtk_widget_show (dialog_vbox1);

  dialog_action_area1 = GTK_DIALOG (dialog1)->action_area;
  gtk_widget_show (dialog_action_area1);
  gtk_container_set_border_width (GTK_CONTAINER (dialog_action_area1), 10);

  constructWindowContents ( dialog_vbox1 ) ;

  abiAddStockButton ( GTK_DIALOG(dialog1), GTK_STOCK_CANCEL, BUTTON_CANCEL );
  abiAddStockButton ( GTK_DIALOG(dialog1), GTK_STOCK_OK, BUTTON_OK );
	
  return dialog1;
}

void AP_UnixDialog_MarkRevisions::constructWindowContents ( GtkWidget * container )
{
   GtkWidget *vbox1;
   GSList *vbox1_group = NULL;
   GtkWidget *radiobutton1 = NULL;
   GtkWidget *lbl1;
   GtkWidget *radiobutton2;
   GtkWidget *lbl2;
   GtkWidget *entry1;
   
   vbox1 = gtk_vbox_new (FALSE, 0);
   gtk_widget_show (vbox1);
   gtk_box_pack_start (GTK_BOX (container), vbox1, TRUE, TRUE, 0);
   gtk_container_set_border_width (GTK_CONTAINER (vbox1), 3);

   if(getRadio1Label() != NULL)
     {
		 if(isRev())
		 {
			 radiobutton1 = gtk_radio_button_new_with_label (vbox1_group, getRadio1Label());
			 vbox1_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radiobutton1));
			 gtk_widget_show (radiobutton1);
			 gtk_box_pack_start (GTK_BOX (vbox1), radiobutton1, FALSE, FALSE, 0);
			 lbl1 = gtk_label_new(getComment1());
			 gtk_widget_show (lbl1);
			 gtk_box_pack_start (GTK_BOX (vbox1), lbl1, FALSE, FALSE, 0);
		 }
		 radiobutton2 = gtk_radio_button_new_with_label (vbox1_group, getRadio2Label());
		 vbox1_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (radiobutton2));
		 
		 if (isRev ())
			 gtk_widget_show (radiobutton2);
		 
		 gtk_box_pack_start (GTK_BOX (vbox1), radiobutton2, FALSE, FALSE, 0);
		 
		 g_signal_connect ( G_OBJECT(radiobutton2), "toggled",
							G_CALLBACK(focus_toggled_callback), this ) ;
		 
		 mRadio1  = radiobutton1 ;
		 mRadio2  = radiobutton2 ;
     }
   
   lbl2 = gtk_label_new (getComment2Label());
   gtk_widget_show (lbl2);
   gtk_box_pack_start (GTK_BOX (vbox1), lbl2, FALSE, FALSE, 0);
   
   entry1 = gtk_entry_new();
   gtk_widget_show (entry1);
   gtk_box_pack_start (GTK_BOX (vbox1), entry1, FALSE, FALSE, 0);
   
   mEntryLbl = lbl2 ;
   mComment = entry1 ;
}

void AP_UnixDialog_MarkRevisions::event_OK ()
{
  m_answer = AP_Dialog_MarkRevisions::a_OK ;
  setComment2 ( gtk_entry_get_text ( GTK_ENTRY( mComment ) ) ) ;
}

void AP_UnixDialog_MarkRevisions::event_Cancel ()
{
  m_answer = AP_Dialog_MarkRevisions::a_CANCEL ;
}

void AP_UnixDialog_MarkRevisions::event_FocusToggled ()
{
	gboolean second_active = FALSE ;
	
	if ( ( mRadio2 && gtk_toggle_button_get_active ( GTK_TOGGLE_BUTTON(mRadio2) ) )
		 || getRadio1Label() == NULL )
		second_active = TRUE ;
	
	if ( mEntryLbl )
		gtk_widget_set_sensitive ( mEntryLbl, second_active ) ;
	
	if ( mComment )
		gtk_widget_set_sensitive ( mComment, second_active ) ;
}
