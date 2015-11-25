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
#include "ap_Dialog_ToggleCase.h"
#include "ap_UnixDialog_ToggleCase.h"

/*****************************************************************/

static void s_toggled (GtkWidget * radio, AP_Dialog_ToggleCase * dlg)
{
  ToggleCase tc = (ToggleCase) GPOINTER_TO_INT (g_object_get_data (G_OBJECT(radio), "user_data"));
  dlg->setCase (tc);
}

XAP_Dialog * AP_UnixDialog_ToggleCase::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	return new AP_UnixDialog_ToggleCase(pFactory,id);
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
	UT_return_if_fail(pFrame);
	
    // Build the window's widgets and arrange them
    GtkWidget * mainWindow = _constructWindow();
    UT_return_if_fail(mainWindow);

	switch(abiRunModalDialog(GTK_DIALOG(mainWindow), pFrame, this,
							 BUTTON_CANCEL, true))
	{
		case BUTTON_OK:
			setAnswer(AP_Dialog_ToggleCase::a_OK); break ;
		default:
			setAnswer(AP_Dialog_ToggleCase::a_CANCEL); break ;
	}
}

GtkWidget * AP_UnixDialog_ToggleCase::_constructWindow (void)
{
  const XAP_StringSet * pSS = m_pApp->getStringSet();

  std::string s;
  pSS->getValueUTF8(AP_STRING_ID_DLG_ToggleCase_Title,s);
  GtkWidget * windowMain = abiDialogNew("toggle case dialog", TRUE, s.c_str());

  GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 2);
  gtk_widget_show(vbox);
  gtk_container_set_border_width(GTK_CONTAINER(vbox), 12);
  gtk_container_add(GTK_CONTAINER(gtk_dialog_get_content_area(GTK_DIALOG(windowMain))), vbox);
  _constructWindowContents(vbox);

  pSS->getValueUTF8(XAP_STRING_ID_DLG_Cancel, s);
  abiAddButton(GTK_DIALOG(windowMain), s, BUTTON_CANCEL);
  pSS->getValueUTF8(XAP_STRING_ID_DLG_OK, s);
  abiAddButton(GTK_DIALOG(windowMain), s, BUTTON_OK);

  return windowMain;
}

void AP_UnixDialog_ToggleCase::_constructWindowContents (GtkWidget *vbox1)
{
  GSList *vbox1_group = NULL;
  GtkWidget *sentenceCase;
  GtkWidget *lowerCase;
  GtkWidget *upperCase;
  GtkWidget *firstUpperCase;
  GtkWidget *toggleCase;

  const XAP_StringSet * pSS = m_pApp->getStringSet();
  std::string s;
  pSS->getValueUTF8(AP_STRING_ID_DLG_ToggleCase_SentenceCase,s);
  sentenceCase = gtk_radio_button_new_with_label (vbox1_group,s.c_str());
  vbox1_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (sentenceCase));
  gtk_widget_show (sentenceCase);
  gtk_box_pack_start (GTK_BOX (vbox1), sentenceCase, FALSE, FALSE, 0);

  pSS->getValueUTF8(AP_STRING_ID_DLG_ToggleCase_LowerCase,s);
  lowerCase = gtk_radio_button_new_with_label (vbox1_group,s.c_str());
  vbox1_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (lowerCase));
  gtk_widget_show (lowerCase);
  gtk_box_pack_start (GTK_BOX (vbox1), lowerCase, FALSE, FALSE, 0);

  pSS->getValueUTF8(AP_STRING_ID_DLG_ToggleCase_UpperCase,s);
  upperCase = gtk_radio_button_new_with_label (vbox1_group,s.c_str());
  vbox1_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (upperCase));
  gtk_widget_show (upperCase);
  gtk_box_pack_start (GTK_BOX (vbox1), upperCase, FALSE, FALSE, 0);

  pSS->getValueUTF8(AP_STRING_ID_DLG_ToggleCase_FirstUpperCase,s);
  firstUpperCase = gtk_radio_button_new_with_label (vbox1_group,s.c_str());
  vbox1_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (firstUpperCase));
  gtk_widget_show (firstUpperCase);
  gtk_box_pack_start (GTK_BOX (vbox1), firstUpperCase, FALSE, FALSE, 0);

  pSS->getValueUTF8(AP_STRING_ID_DLG_ToggleCase_ToggleCase,s);
  toggleCase = gtk_radio_button_new_with_label (vbox1_group,s.c_str());
  vbox1_group = gtk_radio_button_get_group (GTK_RADIO_BUTTON (toggleCase));
  gtk_widget_show (toggleCase);
  gtk_box_pack_start (GTK_BOX (vbox1), toggleCase, FALSE, FALSE, 0);

  g_object_set_data (G_OBJECT(sentenceCase), "user_data", GINT_TO_POINTER(CASE_SENTENCE));
  g_object_set_data (G_OBJECT(lowerCase), "user_data", GINT_TO_POINTER(CASE_LOWER));
  g_object_set_data (G_OBJECT(upperCase), "user_data", GINT_TO_POINTER(CASE_UPPER));
  g_object_set_data (G_OBJECT(firstUpperCase), "user_data", GINT_TO_POINTER(CASE_FIRST_CAPITAL));
  g_object_set_data (G_OBJECT(toggleCase), "user_data", GINT_TO_POINTER(CASE_TOGGLE));

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
