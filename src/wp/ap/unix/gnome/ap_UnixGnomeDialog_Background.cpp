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

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "ut_dialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_UnixGnomeDialog_Background.h"

static void s_ok_clicked (GtkWidget * btn, AP_UnixDialog_Background * dlg)
{
  UT_ASSERT(dlg);
  dlg->eventOk();
}

static void s_cancel_clicked (GtkWidget * btn, AP_UnixDialog_Background * dlg)
{
  UT_ASSERT(dlg);
  dlg->eventCancel();
}

static void s_delete_clicked(GtkWidget * /* widget */,
			     gpointer /* data */,
			     AP_UnixDialog_Background * dlg)
{
  UT_ASSERT(dlg);
  dlg->eventCancel();
}

/*****************************************************************/

XAP_Dialog * AP_UnixGnomeDialog_Background::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_UnixGnomeDialog_Background * p = new AP_UnixGnomeDialog_Background(pFactory,id);
	return p;
}

AP_UnixGnomeDialog_Background::AP_UnixGnomeDialog_Background(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_UnixDialog_Background(pDlgFactory,id)
{
}

AP_UnixGnomeDialog_Background::~AP_UnixGnomeDialog_Background(void)
{
}

GtkWidget * AP_UnixGnomeDialog_Background::_constructWindow (void)
{
  GtkWidget * dlg;
  GtkWidget * k;
  GtkWidget * cancel;

  const XAP_StringSet * pSS = m_pApp->getStringSet();

  dlg = gnome_dialog_new (pSS->getValue(AP_STRING_ID_DLG_Background_Title),
			  GNOME_STOCK_BUTTON_OK, GNOME_STOCK_BUTTON_CANCEL, 
			  NULL);

  k = GTK_WIDGET (g_list_first (GNOME_DIALOG (dlg)->buttons)->data);
  gtk_signal_connect (GTK_OBJECT(k), "clicked", 
		      GTK_SIGNAL_FUNC(s_ok_clicked), (gpointer)this);

  cancel = GTK_WIDGET (g_list_last (GNOME_DIALOG (dlg)->buttons)->data);
  gtk_signal_connect (GTK_OBJECT(cancel), "clicked", 
		      GTK_SIGNAL_FUNC(s_cancel_clicked), (gpointer)this);

  gtk_signal_connect (GTK_OBJECT(dlg), "close", 
		      GTK_SIGNAL_FUNC(s_cancel_clicked), (gpointer)this);

  gtk_signal_connect_after(GTK_OBJECT(dlg),
			   "destroy",
			   NULL,
			   NULL);

  gtk_signal_connect(GTK_OBJECT(dlg),
		     "delete_event",
		     GTK_SIGNAL_FUNC(s_delete_clicked),
		     (gpointer) this);
  
  _constructWindowContents (GNOME_DIALOG(dlg)->vbox);

  setDefaultButton (GNOME_DIALOG(dlg), 1);

  return dlg;
}
