/* AbiWord
 * Copyright (C) 2000-2002 AbiSource, Inc.
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

void XAP_UnixDialog_Password::event_Ok ()
{
  UT_String pass ( gtk_entry_get_text (GTK_ENTRY(mTextEntry)) );
  setPassword (pass);
  setAnswer(XAP_Dialog_Password::a_OK);
}

void XAP_UnixDialog_Password::event_Return()
{
  gtk_dialog_response ( GTK_DIALOG ( mMainWindow ), BUTTON_OK ) ;
}

void XAP_UnixDialog_Password::event_Cancel ()
{
  setAnswer(XAP_Dialog_Password::a_Cancel);
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
  // build the dialog
  GtkWidget * cf = _constructWindow();

  gdk_keyboard_grab(cf->window, FALSE, GDK_CURRENT_TIME);
  
  switch ( abiRunModalDialog ( GTK_DIALOG(cf), pFrame, this, BUTTON_CANCEL, false ) )
    {
    case BUTTON_OK:
      event_Ok() ; break ;
    default:
      event_Cancel() ; break;
    }
	
  gdk_keyboard_ungrab(GDK_CURRENT_TIME);

  abiDestroyWidget(cf);
}

void XAP_UnixDialog_Password::_constructWindowContents (GtkWidget * container)
{
  GtkWidget * label1;
  GtkWidget * password;

  const XAP_StringSet * pSS = m_pApp->getStringSet();

  label1 = gtk_label_new (pSS->getValueUTF8(XAP_STRING_ID_DLG_Password_Password).c_str());
  gtk_widget_show (label1);
  gtk_box_pack_start (GTK_BOX (container), label1, FALSE, FALSE, 0);

  password = gtk_entry_new ();
  gtk_widget_show (password);
  gtk_box_pack_start (GTK_BOX (container), password, TRUE, TRUE, 0);
  gtk_entry_set_visibility (GTK_ENTRY (password), FALSE);

  gtk_widget_grab_focus(password);

  g_signal_connect (G_OBJECT(password), "activate",
		    G_CALLBACK(s_return_hit),
		    (gpointer)this);

  mTextEntry = password;
}

GtkWidget * XAP_UnixDialog_Password::_constructWindow ()
{
  GtkWidget * dialog1;
  GtkWidget * dialog_vbox1;
  GtkWidget * hbox1;

  const XAP_StringSet * pSS = m_pApp->getStringSet();

  dialog1 = abiDialogNew ( "password dialog", TRUE, pSS->getValueUTF8(XAP_STRING_ID_DLG_Password_Title).c_str());

  dialog_vbox1 = GTK_DIALOG (dialog1)->vbox;
  gtk_widget_show (dialog_vbox1);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), hbox1, TRUE, TRUE, 0);

  _constructWindowContents (hbox1);

  abiAddStockButton(GTK_DIALOG(dialog1), GTK_STOCK_OK, BUTTON_OK);
  abiAddStockButton(GTK_DIALOG(dialog1), GTK_STOCK_CANCEL, BUTTON_CANCEL);

  mMainWindow = dialog1;

  return dialog1;
}
