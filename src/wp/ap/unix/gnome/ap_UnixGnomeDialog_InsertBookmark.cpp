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
#include "xap_UnixDialogHelper.h"

#include "ap_UnixGnomeDialog_InsertBookmark.h"

static void
cb_close (GtkWidget * w, AP_UnixGnomeDialog_InsertBookmark * dlg)
{
  UT_ASSERT(dlg);
  dlg->event_Cancel();
}

AP_UnixGnomeDialog_InsertBookmark::AP_UnixGnomeDialog_InsertBookmark(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id) : AP_UnixDialog_InsertBookmark(pDlgFactory, id)
{
}

AP_UnixGnomeDialog_InsertBookmark::~AP_UnixGnomeDialog_InsertBookmark(void)
{
}

XAP_Dialog * AP_UnixGnomeDialog_InsertBookmark::static_constructor(XAP_DialogFactory *pDlgFactory, XAP_Dialog_Id id)
{
  AP_UnixGnomeDialog_InsertBookmark *p = new AP_UnixGnomeDialog_InsertBookmark(pDlgFactory, id);
  return p;
}

GtkWidget * AP_UnixGnomeDialog_InsertBookmark::_constructWindow(void)
{
  	GtkWidget * windowInsertBookmark;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	windowInsertBookmark = gnome_dialog_new (pSS->getValue(AP_STRING_ID_DLG_InsertBookmark_Title), NULL);

	_constructWindowContents(GNOME_DIALOG(windowInsertBookmark)->vbox);

	// ok button
	//
	gnome_dialog_append_button(GNOME_DIALOG(windowInsertBookmark), GNOME_STOCK_BUTTON_OK);
	m_buttonOK = GTK_WIDGET (g_list_last (GNOME_DIALOG (windowInsertBookmark)->buttons)->data);
	GTK_WIDGET_SET_FLAGS (m_buttonOK, GTK_CAN_DEFAULT);

	// cancel button
	//
	gnome_dialog_append_button(GNOME_DIALOG(windowInsertBookmark), GNOME_STOCK_BUTTON_CANCEL);
	m_buttonCancel = GTK_WIDGET (g_list_last (GNOME_DIALOG (windowInsertBookmark)->buttons)->data);
	GTK_WIDGET_SET_FLAGS (m_buttonCancel, GTK_CAN_DEFAULT);

	gtk_signal_connect (GTK_OBJECT(windowInsertBookmark),
			    "close",
			    GTK_SIGNAL_FUNC(cb_close),
			    (gpointer) this);

	m_windowMain = windowInsertBookmark;
	setDefaultButton (GNOME_DIALOG(windowInsertBookmark), 1);
	_connectSignals();
	return windowInsertBookmark;
}
