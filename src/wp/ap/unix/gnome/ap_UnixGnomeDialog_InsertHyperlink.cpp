/* AbiWord
 * Copyright (C) 2001 AbiSource, Inc.
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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ap_Strings.h"
#include "ut_dialogHelper.h"

#include "ap_UnixGnomeDialog_InsertHyperlink.h"

static void
cb_close (GtkWidget * w, AP_UnixGnomeDialog_InsertHyperlink * dlg)
{
  UT_ASSERT(dlg);
  dlg->event_Cancel();
}

AP_UnixGnomeDialog_InsertHyperlink::AP_UnixGnomeDialog_InsertHyperlink(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id) : AP_UnixDialog_InsertHyperlink(pDlgFactory, id)
{
}

AP_UnixGnomeDialog_InsertHyperlink::~AP_UnixGnomeDialog_InsertHyperlink(void)
{
}

XAP_Dialog * AP_UnixGnomeDialog_InsertHyperlink::static_constructor(XAP_DialogFactory *pDlgFactory, XAP_Dialog_Id id)
{
  AP_UnixGnomeDialog_InsertHyperlink *p = new AP_UnixGnomeDialog_InsertHyperlink(pDlgFactory, id);
  return p;
}

GtkWidget * AP_UnixGnomeDialog_InsertHyperlink::_constructWindow(void)
{
  	GtkWidget * windowInsertHyperlink;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	windowInsertHyperlink = gnome_dialog_new (pSS->getValue(AP_STRING_ID_DLG_InsertHyperlink_Title), NULL);

	_constructWindowContents(GNOME_DIALOG(windowInsertHyperlink)->vbox);

	// ok button
	//
	gnome_dialog_append_button(GNOME_DIALOG(windowInsertHyperlink), GNOME_STOCK_BUTTON_OK);
	m_buttonOK = GTK_WIDGET (g_list_last (GNOME_DIALOG (windowInsertHyperlink)->buttons)->data);
	GTK_WIDGET_SET_FLAGS (m_buttonOK, GTK_CAN_DEFAULT);

	// cancel button
	//
	gnome_dialog_append_button(GNOME_DIALOG(windowInsertHyperlink), GNOME_STOCK_BUTTON_CANCEL);
	m_buttonCancel = GTK_WIDGET (g_list_last (GNOME_DIALOG (windowInsertHyperlink)->buttons)->data);
	GTK_WIDGET_SET_FLAGS (m_buttonCancel, GTK_CAN_DEFAULT);

	gtk_signal_connect (GTK_OBJECT(windowInsertHyperlink),
			    "close",
			    GTK_SIGNAL_FUNC(cb_close),
			    (gpointer) this);

	m_windowMain = windowInsertHyperlink;
	setDefaultButton (GNOME_DIALOG(windowInsertHyperlink), 1);
	_connectSignals();
	return windowInsertHyperlink;
}
