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
#include "xap_Frame.h"

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

  GtkWidget * windowMain = abiDialogNew("toggle case dialog", TRUE, pSS->getValueUTF8(AP_STRING_ID_DLG_ToggleCase_Title).c_str());

  _constructWindowContents (GTK_DIALOG(windowMain)->vbox);

  abiAddStockButton(GTK_DIALOG(windowMain), GTK_STOCK_CANCEL, BUTTON_CANCEL);
  abiAddStockButton(GTK_DIALOG(windowMain), GTK_STOCK_OK, BUTTON_OK);

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

  sentenceCase = gtk_radio_button_new_with_label (vbox1_group, 
						  pSS->getValueUTF8(AP_STRING_ID_DLG_ToggleCase_SentenceCase).c_str());
  vbox1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (sentenceCase));
  gtk_widget_show (sentenceCase);
  gtk_box_pack_start (GTK_BOX (vbox1), sentenceCase, FALSE, FALSE, 0);

  lowerCase = gtk_radio_button_new_with_label (vbox1_group, 
					       pSS->getValueUTF8(AP_STRING_ID_DLG_ToggleCase_LowerCase).c_str());
  vbox1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (lowerCase));
  gtk_widget_show (lowerCase);
  gtk_box_pack_start (GTK_BOX (vbox1), lowerCase, FALSE, FALSE, 0);

  upperCase = gtk_radio_button_new_with_label (vbox1_group, 
					       pSS->getValueUTF8(AP_STRING_ID_DLG_ToggleCase_UpperCase).c_str());
  vbox1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (upperCase));
  gtk_widget_show (upperCase);
  gtk_box_pack_start (GTK_BOX (vbox1), upperCase, FALSE, FALSE, 0);

  firstUpperCase = gtk_radio_button_new_with_label (vbox1_group,
					       pSS->getValueUTF8(AP_STRING_ID_DLG_ToggleCase_FirstUpperCase).c_str());
  vbox1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (firstUpperCase));
  gtk_widget_show (firstUpperCase);
  gtk_box_pack_start (GTK_BOX (vbox1), firstUpperCase, FALSE, FALSE, 0);

  toggleCase = gtk_radio_button_new_with_label (vbox1_group, 
						pSS->getValueUTF8(AP_STRING_ID_DLG_ToggleCase_ToggleCase).c_str());
  vbox1_group = gtk_radio_button_group (GTK_RADIO_BUTTON (toggleCase));
  gtk_widget_show (toggleCase);
  gtk_box_pack_start (GTK_BOX (vbox1), toggleCase, FALSE, FALSE, 0);

  gtk_object_set_user_data (GTK_OBJECT(sentenceCase), GINT_TO_POINTER(CASE_SENTENCE));
  gtk_object_set_user_data (GTK_OBJECT(lowerCase), GINT_TO_POINTER(CASE_LOWER));
  gtk_object_set_user_data (GTK_OBJECT(upperCase), GINT_TO_POINTER(CASE_UPPER));
  gtk_object_set_user_data (GTK_OBJECT(firstUpperCase), GINT_TO_POINTER(CASE_FIRST_CAPITAL));
  gtk_object_set_user_data (GTK_OBJECT(toggleCase), GINT_TO_POINTER(CASE_TOGGLE));

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
