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
#include "xap_UnixGnomeApp.h"
#include "xap_UnixGnomeFrame.h"

#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_Password.h"
#include "xap_UnixGnomeDlg_Password.h"

/*****************************************************************/

XAP_Dialog * XAP_UnixGnomeDialog_Password::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	XAP_UnixGnomeDialog_Password * p = new XAP_UnixGnomeDialog_Password(pFactory,id);
	return p;
}

XAP_UnixGnomeDialog_Password::XAP_UnixGnomeDialog_Password(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: XAP_UnixDialog_Password(pDlgFactory,id)
{
}

XAP_UnixGnomeDialog_Password::~XAP_UnixGnomeDialog_Password(void)
{
}

static void s_delete_clicked(GtkWidget * widget,
			     gpointer data ,
			     XAP_UnixGnomeDialog_Password* dlg)
{
  dlg->event_Cancel ();
}

static void s_ok_clicked( GtkWidget * widget, XAP_UnixGnomeDialog_Password* dlg)
{
  dlg->event_Ok ();
}

static void s_cancel_clicked (GtkWidget * w, XAP_UnixGnomeDialog_Password* dlg)
{
  dlg->event_Cancel ();
}


GtkWidget * XAP_UnixGnomeDialog_Password::_constructWindow ()
{
  GtkWidget * dialog1;
  GtkWidget * ok_btn;
  GtkWidget * cancel_btn;
  GtkWidget * hbox1;

  const XAP_StringSet * pSS = m_pApp->getStringSet();

  dialog1 = gnome_dialog_new (pSS->getValue(XAP_STRING_ID_DLG_Password_Title),
			      GNOME_STOCK_BUTTON_OK, 
			      GNOME_STOCK_BUTTON_CANCEL, NULL);

  ok_btn = GTK_WIDGET (g_list_first (GNOME_DIALOG (dialog1)->buttons)->data);
  cancel_btn = GTK_WIDGET (g_list_last (GNOME_DIALOG (dialog1)->buttons)->data);

  hbox1 = gtk_hbox_new (FALSE, 0);
  gtk_widget_show (hbox1);
  gtk_box_pack_start (GTK_BOX (GNOME_DIALOG(dialog1)->vbox), hbox1, TRUE, TRUE, 0);

  _constructWindowContents (hbox1);

  gtk_signal_connect_after(GTK_OBJECT(dialog1),
			   "destroy",
			   NULL,
			   NULL);
  
  gtk_signal_connect(GTK_OBJECT(dialog1),
		     "delete_event",
		     GTK_SIGNAL_FUNC(s_delete_clicked),
		     (gpointer) this);

  gtk_signal_connect (GTK_OBJECT(ok_btn), "clicked",
		      GTK_SIGNAL_FUNC(s_ok_clicked), 
		      (gpointer)this);
  
  gtk_signal_connect (GTK_OBJECT(cancel_btn), "clicked",
		      GTK_SIGNAL_FUNC(s_cancel_clicked), 
		      (gpointer)this);

  GTK_WIDGET_SET_FLAGS (cancel_btn, GTK_CAN_DEFAULT);
  GTK_WIDGET_SET_FLAGS (ok_btn, GTK_CAN_DEFAULT);

  mMainWindow = dialog1;
  setDefaultButton (GNOME_DIALOG(dialog1), 1);

  return dialog1;
}
