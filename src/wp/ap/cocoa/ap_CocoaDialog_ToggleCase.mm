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

// This header defines some functions for Cocoa dialogs,
// like centering them, measuring them, etc.
#include "xap_CocoaDialogHelper.h"

#include "xap_App.h"
#include "xap_CocoaApp.h"
#include "xap_CocoaFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_ToggleCase.h"
#include "ap_CocoaDialog_ToggleCase.h"

/*****************************************************************/

static void s_toggled (GtkWidget * radio, AP_Dialog_ToggleCase * dlg)
{
  ToggleCase tc = (ToggleCase) GPOINTER_TO_INT (g_object_get_user_data (G_OBJECT(radio)));
  dlg->setCase (tc);
}

XAP_Dialog * AP_CocoaDialog_ToggleCase::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_CocoaDialog_ToggleCase * p = new AP_CocoaDialog_ToggleCase(pFactory,id);
	return p;
}

AP_CocoaDialog_ToggleCase::AP_CocoaDialog_ToggleCase(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_ToggleCase(pDlgFactory,id)
{
}

AP_CocoaDialog_ToggleCase::~AP_CocoaDialog_ToggleCase(void)
{
}

void AP_CocoaDialog_ToggleCase::runModal(XAP_Frame * pFrame)
{
    // Build the window's widgets and arrange them
    GtkWidget * mainWindow = _constructWindow();
    UT_ASSERT(mainWindow);

    connectFocus(GTK_WIDGET(mainWindow), pFrame);

    // To center the dialog, we need the frame of its parent.
    XAP_CocoaFrame * pCocoaFrame = static_cast<XAP_CocoaFrame *>(pFrame);
    UT_ASSERT(pCocoaFrame);
    
    // Get the GtkWindow of the parent frame
    GtkWidget * parentWindow = pCocoaFrame->getTopLevelWindow();
    UT_ASSERT(parentWindow);
    
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

static void s_ok_clicked (GtkWidget * w, AP_CocoaDialog_ToggleCase * tc)
{
  tc->setAnswer(AP_Dialog_ToggleCase::a_OK);
  gtk_main_quit ();
}

static void s_cancel_clicked (GtkWidget * w, AP_CocoaDialog_ToggleCase * tc)
{
  tc->setAnswer(AP_Dialog_ToggleCase::a_CANCEL);
  gtk_main_quit ();
}

static void s_delete_clicked (GtkWidget * w, gpointer data, 
			      AP_CocoaDialog_ToggleCase * tc)
{
  s_cancel_clicked (w, tc);
}

GtkWidget * AP_CocoaDialog_ToggleCase::_constructWindow (void)
{
  const XAP_StringSet * pSS = m_pApp->getStringSet();
  GtkWidget * buttonOK, * buttonCancel;

  GtkWidget * windowMain = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (windowMain), pSS->getValue(AP_STRING_ID_DLG_ToggleCase_Title));

  _constructWindowContents (GTK_DIALOG(windowMain)->vbox);

  buttonOK = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_OK));
  gtk_widget_show (buttonOK);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(windowMain)->action_area), 
		     buttonOK);

  buttonCancel = gtk_button_new_with_label (pSS->getValue(XAP_STRING_ID_DLG_Cancel));
  gtk_widget_show (buttonCancel);
  gtk_container_add (GTK_CONTAINER (GTK_DIALOG(windowMain)->action_area), buttonCancel);

	g_signal_connect_after(G_OBJECT(windowMain),
				 "destroy",
				 NULL,
				 NULL);
	g_signal_connect(G_OBJECT(windowMain),
			   "delete_event",
			   G_CALLBACK(s_delete_clicked),
			   (gpointer) this);

	g_signal_connect(G_OBJECT(buttonOK),
			   "clicked",
			   G_CALLBACK(s_ok_clicked),
			   (gpointer) this);
	g_signal_connect(G_OBJECT(buttonCancel),
			   "clicked",
			   G_CALLBACK(s_cancel_clicked),
			   (gpointer) this);

  return windowMain;
}

void AP_CocoaDialog_ToggleCase::_constructWindowContents (GtkWidget *vbox1)
{
  GSList *vbox1_group = NULL;
  GtkWidget *sentenceCase;
  GtkWidget *lowerCase;
  GtkWidget *upperCase;
  GtkWidget *firstUpperCase;
  GtkWidget *toggleCase;

  const XAP_StringSet * pSS = m_pApp->getStringSet();

  sentenceCase = gtk_radio_button_new_with_label (vbox1_group, 
						  pSS->getValue(AP_STRING_ID_DLG_ToggleCase_SentenceCase));
  vbox1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (sentenceCase));
  gtk_widget_show (sentenceCase);
  gtk_box_pack_start (GTK_BOX (vbox1), sentenceCase, FALSE, FALSE, 0);

  lowerCase = gtk_radio_button_new_with_label (vbox1_group, 
					       pSS->getValue(AP_STRING_ID_DLG_ToggleCase_LowerCase));
  vbox1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (lowerCase));
  gtk_widget_show (lowerCase);
  gtk_box_pack_start (GTK_BOX (vbox1), lowerCase, FALSE, FALSE, 0);

  upperCase = gtk_radio_button_new_with_label (vbox1_group, 
					       pSS->getValue(AP_STRING_ID_DLG_ToggleCase_UpperCase));
  vbox1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (upperCase));
  gtk_widget_show (upperCase);
  gtk_box_pack_start (GTK_BOX (vbox1), upperCase, FALSE, FALSE, 0);

  firstUpperCase = gtk_radio_button_new_with_label (vbox1_group,
					       pSS->getValue(AP_STRING_ID_DLG_ToggleCase_FirstUpperCase));
  vbox1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (firstUpperCase));
  gtk_widget_show (firstUpperCase);
  gtk_box_pack_start (GTK_BOX (vbox1), firstUpperCase, FALSE, FALSE, 0);

  toggleCase = gtk_radio_button_new_with_label (vbox1_group, 
						pSS->getValue(AP_STRING_ID_DLG_ToggleCase_ToggleCase));
  vbox1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (toggleCase));
  gtk_widget_show (toggleCase);
  gtk_box_pack_start (GTK_BOX (vbox1), toggleCase, FALSE, FALSE, 0);

  g_object_set_user_data (G_OBJECT(sentenceCase), GINT_TO_POINTER(CASE_SENTENCE));
  g_object_set_user_data (G_OBJECT(lowerCase), GINT_TO_POINTER(CASE_LOWER));
  g_object_set_user_data (G_OBJECT(upperCase), GINT_TO_POINTER(CASE_UPPER));
  g_object_set_user_data (G_OBJECT(firstUpperCase), GINT_TO_POINTER(CASE_FIRST_CAPITAL));
  g_object_set_user_data (G_OBJECT(toggleCase), GINT_TO_POINTER(CASE_TOGGLE));

  g_signal_connect (G_OBJECT(sentenceCase), "toggled",
		      G_CALLBACK(s_toggled), (gpointer)this);
  g_signal_connect (G_OBJECT(lowerCase), "toggled",
		      G_CALLBACK(s_toggled), (gpointer)this);
  g_signal_connect (G_OBJECT(upperCase), "toggled",
		      G_CALLBACK(s_toggled), (gpointer)this);
  g_signal_connect (G_OBJECT(firstUpperCase), "toggled",
		      G_CALLBACK(s_toggled), (gpointer)this);
  g_signal_connect (G_OBJECT(toggleCase), "toggled",
		      G_CALLBACK(s_toggled), (gpointer)this);
}
