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
#include "xap_UnixFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_MarkRevisions.h"
#include "ap_UnixDialog_MarkRevisions.h"

/*****************************************************************/

XAP_Dialog * AP_UnixDialog_MarkRevisions::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_UnixDialog_MarkRevisions * p = new AP_UnixDialog_MarkRevisions(pFactory,id);
	return p;
}

AP_UnixDialog_MarkRevisions::AP_UnixDialog_MarkRevisions(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_MarkRevisions(pDlgFactory,id)
{
}

AP_UnixDialog_MarkRevisions::~AP_UnixDialog_MarkRevisions(void)
{
}

void AP_UnixDialog_MarkRevisions::runModal(XAP_Frame * pFrame)
{
  /*
    This is the only function you need to implement, and the MarkRevisions
    dialogue should look like this:
    
    ----------------------------------------------------
    | Title                                            |
    ----------------------------------------------------
    |                                                  |
    | O Radio1                                         |
    |    Comment1 (a label)                            |
    |                                                  |
    | O Radio2                                         |
    |    Comment2Label                                 |
    |    Comment2 (an edit control)                    |
    |                                                  |
    |                                                  |
    |     OK_BUTTON              CANCEL_BUTTON         |
    ----------------------------------------------------
    
    Where: Title, Comment1 and Comment2Label are labels, Radio1-2
    is are radio buttons, Comment2 is an Edit control.
    
    Use getTitle(), getComment1(), getComment2Label(), getRadio1Label()
    and getRadio2Label() to get the labels (the last two for the radio
    buttons), note that you are responsible for freeing the
    pointers returned by getLable1() and getComment1() using FREEP
    (but not the rest!)
    
    if getLabel1() returns NULL, hide the radio buttons and enable
    the Edit box; otherwise the Edit box should be only enabled when
    Radio2 is selected.
    
    Use setComment2(const char * pszString) to store the contents of the Edit control
    when the dialogue closes; make sure that you freee pszString afterwards.
    
    
  */
  
  UT_return_if_fail(pFrame);
  
  GtkWidget * mainWindow = constructWindow();
  
  connectFocus(GTK_WIDGET(mainWindow),pFrame);
  
  // To center the dialog, we need the frame of its parent.
  XAP_UnixFrame * pUnixFrame = static_cast<XAP_UnixFrame *>(pFrame);
  
  // Get the GtkWindow of the parent frame
  GtkWidget * parentWindow = pUnixFrame->getTopLevelWindow();
  UT_return_if_fail(parentWindow);
  
  // Center our new dialog in its parent and make it a transient
  // so it won't get lost underneath
  centerDialog(parentWindow, mainWindow);

  // Show the top level dialog,
  gtk_widget_show(mainWindow);
  
  // Make it modal, and stick it up top
  gtk_grab_add(mainWindow);
  
  // Run into the GTK event loop for this window.	
  gtk_main();
  
  if(mainWindow && GTK_IS_WIDGET(mainWindow))
    gtk_widget_destroy(mainWindow);
}

GtkWidget * AP_UnixDialog_MarkRevisions::constructWindow ()
{
  const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

  GtkWidget *dialog1;
  GtkWidget *dialog_vbox1;
  GtkWidget *dialog_action_area1;
  GtkWidget *hbuttonbox1;
  GtkWidget *ok_btn;
  GtkWidget *cancel_btn;

  dialog1 = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog1), pSS->getValue(AP_STRING_ID_DLG_MarkRevisions_Title));
  GTK_WINDOW (dialog1)->type = GTK_WINDOW_DIALOG;
  gtk_window_set_position (GTK_WINDOW (dialog1), GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (dialog1), TRUE);
  gtk_window_set_policy (GTK_WINDOW (dialog1), TRUE, TRUE, FALSE);
  gtk_widget_set_usize ( dialog1, 250, 250 ) ;

  dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
  gtk_widget_show (dialog_vbox1);

  dialog_action_area1 = GTK_DIALOG (dialog1)->action_area;
  gtk_widget_show (dialog_action_area1);
  gtk_container_set_border_width (GTK_CONTAINER (dialog_action_area1), 10);

  hbuttonbox1 = gtk_hbutton_box_new ();
  gtk_widget_show (hbuttonbox1);
  gtk_box_pack_start (GTK_BOX (dialog_action_area1), hbuttonbox1, TRUE, TRUE, 0);

  ok_btn = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_OK));
  gtk_widget_show (ok_btn);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), ok_btn);
  GTK_WIDGET_SET_FLAGS (ok_btn, GTK_CAN_DEFAULT);

  cancel_btn = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_Cancel));
  gtk_widget_show (cancel_btn);
  gtk_container_add (GTK_CONTAINER (hbuttonbox1), cancel_btn);
  GTK_WIDGET_SET_FLAGS (cancel_btn, GTK_CAN_DEFAULT);

  constructWindowContents ( dialog_vbox1 ) ;

  // connect signals
  g_signal_connect (G_OBJECT(ok_btn), 
		    "clicked",
		    G_CALLBACK(ok_callback), 
		    (gpointer)this);

  g_signal_connect (G_OBJECT(cancel_btn), 
		    "clicked",
		    G_CALLBACK(cancel_callback), 
		    (gpointer)this);

  g_signal_connect(G_OBJECT(dialog1),
		   "delete_event",
		   G_CALLBACK(destroy_callback),
		   (gpointer) this);
  
  g_signal_connect_after(G_OBJECT(dialog1),
			 "destroy",
			 NULL,
			 NULL);

  return dialog1;
}

void AP_UnixDialog_MarkRevisions::constructWindowContents ( GtkWidget * container )
{
   const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

   GtkWidget *vbox1;
   GSList *vbox1_group = NULL;
   GtkWidget *radiobutton1;
   GtkWidget *entry1;
   GtkWidget *radiobutton2;
   GtkWidget *entry2;
   GtkWidget *scrolledwindow1;
   GtkWidget *text1;
   
#if 0
dcl(DLG_MarkRevisions_Check1Label, "Contiue previous revision (number %d)")
dcl(DLG_MarkRevisions_Check2Label, "Start a new revision")
dcl(DLG_MarkRevisions_Comment2Label, "Comment to be associated with the revision:")
#endif

   vbox1 = gtk_vbox_new (FALSE, 0);
   gtk_widget_show (vbox1);
   gtk_box_pack_start (GTK_BOX (container), vbox1, TRUE, TRUE, 0);
   gtk_container_set_border_width (GTK_CONTAINER (vbox1), 3);
   
   radiobutton1 = gtk_radio_button_new_with_label (vbox1_group, _("radiobutton1"));
   vbox1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobutton1));
   gtk_widget_show (radiobutton1);
   gtk_box_pack_start (GTK_BOX (vbox1), radiobutton1, FALSE, FALSE, 0);
   
   entry1 = gtk_entry_new ();
   gtk_widget_show (entry1);
   gtk_box_pack_start (GTK_BOX (vbox1), entry1, FALSE, FALSE, 0);
   
   radiobutton2 = gtk_radio_button_new_with_label (vbox1_group, _("radiobutton2"));
   vbox1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (radiobutton2));
   gtk_widget_show (radiobutton2);
   gtk_box_pack_start (GTK_BOX (vbox1), radiobutton2, FALSE, FALSE, 0);
   
   entry2 = gtk_entry_new ();
   gtk_widget_show (entry2);
   gtk_box_pack_start (GTK_BOX (vbox1), entry2, FALSE, FALSE, 0);
   
   scrolledwindow1 = gtk_scrolled_window_new (NULL, NULL);
   gtk_widget_show (scrolledwindow1);
   gtk_box_pack_start (GTK_BOX (vbox1), scrolledwindow1, TRUE, TRUE, 0);
   gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolledwindow1), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
   
   text1 = gtk_text_new (NULL, NULL);
   gtk_widget_show (text1);
   gtk_container_add (GTK_CONTAINER (scrolledwindow1), text1);

   mRadio1 = radiobutton1 ;
   mComment1 = entry1 ;

   mRadio2 = radiobutton2 ;
   mComment2Lbl = entry2 ;
   mComment2 = text1 ;
}

void AP_UnixDialog_MarkRevisions::event_Ok ()
{
  m_answer = AP_Dialog_MarkRevisions::a_OK ;
  gtk_main_quit () ;
}

void AP_UnixDialog_MarkRevisions::event_Cancel ()
{
  m_answer = AP_Dialog_MarkRevisions::a_CANCEL ;
  gtk_main_quit () ;
}
