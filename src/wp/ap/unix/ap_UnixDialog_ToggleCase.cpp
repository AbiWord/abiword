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
#include "ut_dialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_ToggleCase.h"
#include "ap_UnixDialog_ToggleCase.h"

/*****************************************************************/

static void s_toggled (GtkWidget * radio, AP_Dialog_ToggleCase * dlg)
{
  ToggleCase tc = (ToggleCase) GPOINTER_TO_INT (gtk_object_get_user_data (GTK_OBJECT(radio)));
  dlg->setCase (tc);
}

XAP_Dialog * AP_UnixDialog_ToggleCase::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_UnixDialog_ToggleCase * p = new AP_UnixDialog_ToggleCase(pFactory,id);
	return p;
}

AP_UnixDialog_ToggleCase::AP_UnixDialog_ToggleCase(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_ToggleCase(pDlgFactory,id)
{
}

AP_UnixDialog_ToggleCase::~AP_UnixDialog_ToggleCase(void)
{
}

void AP_UnixDialog_ToggleCase::runModal(XAP_Frame * pFrame)
{
    // Build the window's widgets and arrange them
    GtkWidget * mainWindow = _constructWindow();
    UT_ASSERT(mainWindow);

    connectFocus(GTK_WIDGET(mainWindow), pFrame);

    // To center the dialog, we need the frame of its parent.
    XAP_UnixFrame * pUnixFrame = static_cast<XAP_UnixFrame *>(pFrame);
    UT_ASSERT(pUnixFrame);
    
    // Get the GtkWindow of the parent frame
    GtkWidget * parentWindow = pUnixFrame->getTopLevelWindow();
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

static void s_ok_clicked (GtkWidget * w, AP_UnixDialog_ToggleCase * tc)
{
  tc->setAnswer(AP_Dialog_ToggleCase::a_OK);
  gtk_main_quit ();
}

static void s_cancel_clicked (GtkWidget * w, AP_UnixDialog_ToggleCase * tc)
{
  tc->setAnswer(AP_Dialog_ToggleCase::a_CANCEL);
  gtk_main_quit ();
}

static void s_delete_clicked (GtkWidget * w, gpointer data, 
			      AP_UnixDialog_ToggleCase * tc)
{
  s_cancel_clicked (w, tc);
}

GtkWidget * AP_UnixDialog_ToggleCase::_constructWindow (void)
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

	gtk_signal_connect_after(GTK_OBJECT(windowMain),
				 "destroy",
				 NULL,
				 NULL);
	gtk_signal_connect(GTK_OBJECT(windowMain),
			   "delete_event",
			   GTK_SIGNAL_FUNC(s_delete_clicked),
			   (gpointer) &m_answer);

	gtk_signal_connect(GTK_OBJECT(buttonOK),
			   "clicked",
			   GTK_SIGNAL_FUNC(s_ok_clicked),
			   (gpointer) &m_answer);
	gtk_signal_connect(GTK_OBJECT(buttonCancel),
			   "clicked",
			   GTK_SIGNAL_FUNC(s_cancel_clicked),
			   (gpointer) &m_answer);

  return windowMain;
}

void AP_UnixDialog_ToggleCase::_constructWindowContents (GtkWidget *vbox1)
{
  GSList *vbox1_group = NULL;
  GtkWidget *sentenceCase;
  GtkWidget *lowerCase;
  GtkWidget *upperCase;
  GtkWidget *titleCase;
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

  titleCase = gtk_radio_button_new_with_label (vbox1_group, 
					       pSS->getValue(AP_STRING_ID_DLG_ToggleCase_TitleCase));
  vbox1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (titleCase));
  gtk_widget_show (titleCase);
  gtk_box_pack_start (GTK_BOX (vbox1), titleCase, FALSE, FALSE, 0);

  toggleCase = gtk_radio_button_new_with_label (vbox1_group, 
						pSS->getValue(AP_STRING_ID_DLG_ToggleCase_ToggleCase));
  vbox1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (toggleCase));
  gtk_widget_show (toggleCase);
  gtk_box_pack_start (GTK_BOX (vbox1), toggleCase, FALSE, FALSE, 0);

  gtk_object_set_user_data (GTK_OBJECT(sentenceCase), GINT_TO_POINTER(CASE_SENTENCE));
  gtk_object_set_user_data (GTK_OBJECT(lowerCase), GINT_TO_POINTER(CASE_LOWER));
  gtk_object_set_user_data (GTK_OBJECT(upperCase), GINT_TO_POINTER(CASE_UPPER));
  gtk_object_set_user_data (GTK_OBJECT(titleCase), GINT_TO_POINTER(CASE_TITLE));
  gtk_object_set_user_data (GTK_OBJECT(toggleCase), GINT_TO_POINTER(CASE_TOGGLE));

  gtk_signal_connect (GTK_OBJECT(sentenceCase), "toggled",
		      GTK_SIGNAL_FUNC(s_toggled), (gpointer)this);
  gtk_signal_connect (GTK_OBJECT(lowerCase), "toggled",
		      GTK_SIGNAL_FUNC(s_toggled), (gpointer)this);
  gtk_signal_connect (GTK_OBJECT(upperCase), "toggled",
		      GTK_SIGNAL_FUNC(s_toggled), (gpointer)this);
  gtk_signal_connect (GTK_OBJECT(titleCase), "toggled",
		      GTK_SIGNAL_FUNC(s_toggled), (gpointer)this);
  gtk_signal_connect (GTK_OBJECT(toggleCase), "toggled",
		      GTK_SIGNAL_FUNC(s_toggled), (gpointer)this);
}
