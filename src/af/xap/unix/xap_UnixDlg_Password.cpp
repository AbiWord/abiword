/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_Stub.h"
#include "xap_UnixDlg_Password.h"

/*****************************************************************/

static void s_delete_clicked(GtkWidget * widget,
			     gpointer data ,
			     XAP_UnixDialog_Password* dlg)
{
  dlg->event_Cancel ();
}

static void s_ok_clicked( GtkWidget * widget, XAP_UnixDialog_Password* dlg)
{
  dlg->event_Ok ();
}

static void s_cancel_clicked (GtkWidget * w, XAP_UnixDialog_Password* dlg)
{
  dlg->event_Cancel ();
}

void XAP_UnixDialog_Password::event_Ok ()
{
  UT_String pass ( gtk_entry_get_text (GTK_ENTRY(mTextEntry)) );
  
  UT_DEBUGMSG(("ok: %s\n", pass.c_str()));

  setPassword (pass);
  setAnswer(XAP_Dialog_Password::a_OK);
  gtk_main_quit ();
}

void XAP_UnixDialog_Password::event_Cancel ()
{
  UT_DEBUGMSG(("cancel\n"));
  setAnswer(XAP_Dialog_Password::a_Cancel);
  gtk_main_quit ();
}

XAP_Dialog * XAP_UnixDialog_Password::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
  XAP_UnixDialog_Password * p = new XAP_UnixDialog_Password(pFactory,id);
  return p;
}

XAP_UnixDialog_Password::XAP_UnixDialog_Password(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: XAP_Dialog_Password(pDlgFactory,id)
{
}

XAP_UnixDialog_Password::~XAP_UnixDialog_Password(void)
{
}

void XAP_UnixDialog_Password::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);

	// build the dialog
	GtkWidget * cf = _constructWindow();
	UT_ASSERT(cf);
	connectFocus(GTK_WIDGET(cf),pFrame);

	// get top level window and its GtkWidget *
	XAP_UnixFrame * frame = static_cast<XAP_UnixFrame *>(pFrame);
	UT_ASSERT(frame);
	GtkWidget * parent = frame->getTopLevelWindow();
	UT_ASSERT(parent);

	// center it
	centerDialog(parent, cf);
	
	// Run the dialog
	gtk_widget_show (cf);
	gtk_grab_add (cf);

	// grab focus from the keyboard to the current window
	// reduces chances of password snooping
	gdk_keyboard_grab(cf->window, FALSE, GDK_CURRENT_TIME);

	gtk_main();
	
	gdk_keyboard_ungrab(GDK_CURRENT_TIME);

	if (cf && GTK_IS_WIDGET(cf))
	  gtk_widget_destroy (cf);
}

void XAP_UnixDialog_Password::_constructWindowContents (GtkWidget * container)
{
  GtkWidget * label1;
  GtkWidget * password;

  const XAP_StringSet * pSS = m_pApp->getStringSet();

  label1 = gtk_label_new (pSS->getValue(XAP_STRING_ID_DLG_Password_Password));
  gtk_widget_show (label1);
  gtk_box_pack_start (GTK_BOX (container), label1, FALSE, FALSE, 0);

  password = gtk_entry_new ();
  gtk_widget_show (password);
  gtk_box_pack_start (GTK_BOX (container), password, TRUE, TRUE, 0);
  gtk_entry_set_visibility (GTK_ENTRY (password), FALSE);

  gtk_widget_grab_focus(password);

  g_signal_connect (G_OBJECT(password), "activate",
                     G_CALLBACK(s_ok_clicked),
                     (gpointer)this);

  mTextEntry = password;
}

GtkWidget * XAP_UnixDialog_Password::_constructWindow ()
{
  GtkWidget * dialog1;
  GtkWidget * dialog_vbox1;
  GtkWidget * hbox1;
  GtkWidget * ok_btn;
  GtkWidget * cancel_btn;
  GtkWidget * hbuttonbox1;
  GtkWidget * dialog_action_area1;

  const XAP_StringSet * pSS = m_pApp->getStringSet();

  dialog1 = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (dialog1), 
			pSS->getValue(XAP_STRING_ID_DLG_Password_Title));
  gtk_window_set_position (GTK_WINDOW (dialog1), GTK_WIN_POS_CENTER);
  gtk_window_set_modal (GTK_WINDOW (dialog1), TRUE);
  gtk_window_set_policy (GTK_WINDOW (dialog1), TRUE, TRUE, FALSE);

  dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
  gtk_widget_show (dialog_vbox1);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), hbox1, TRUE, TRUE, 0);

  _constructWindowContents (hbox1);

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

  g_signal_connect_after(G_OBJECT(dialog1),
			   "destroy",
			   NULL,
			   NULL);
  
  g_signal_connect(G_OBJECT(dialog1),
		     "delete_event",
		     G_CALLBACK(s_delete_clicked),
		     (gpointer) this);

  g_signal_connect (G_OBJECT(ok_btn), "clicked",
		      G_CALLBACK(s_ok_clicked), 
		      (gpointer)this);
  
  g_signal_connect (G_OBJECT(cancel_btn), "clicked",
		      G_CALLBACK(s_cancel_clicked), 
		      (gpointer)this);

  GTK_WIDGET_SET_FLAGS (cancel_btn, GTK_CAN_DEFAULT);
  GTK_WIDGET_SET_FLAGS (ok_btn, GTK_CAN_DEFAULT);

  mMainWindow = dialog1;

  return dialog1;
}
