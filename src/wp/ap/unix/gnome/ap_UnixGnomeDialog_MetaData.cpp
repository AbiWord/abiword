/* AbiWord
 * Copyright (C) 2002 Dom Lachowicz
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

#include "ap_UnixGnomeDialog_MetaData.h"

AP_UnixGnomeDialog_MetaData::AP_UnixGnomeDialog_MetaData(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id) : AP_UnixDialog_MetaData(pDlgFactory, id)
{
}

AP_UnixGnomeDialog_MetaData::~AP_UnixGnomeDialog_MetaData(void)
{
}

XAP_Dialog * AP_UnixGnomeDialog_MetaData::static_constructor(XAP_DialogFactory *pDlgFactory, XAP_Dialog_Id id)
{
  AP_UnixGnomeDialog_MetaData *p = new AP_UnixGnomeDialog_MetaData(pDlgFactory, id);
  return p;
}

GtkWidget * AP_UnixGnomeDialog_MetaData::_constructWindow(void)
{
  	GtkWidget * windowMetaData;
	GtkWidget * ok_btn;
	GtkWidget * cancel_btn;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	windowMetaData = gnome_dialog_new (pSS->getValue(AP_STRING_ID_DLG_MetaData_Title), NULL);

	_constructWindowContents(GNOME_DIALOG(windowMetaData)->vbox);

	// apply button
	//
	gnome_dialog_append_button(GNOME_DIALOG(windowMetaData), GNOME_STOCK_BUTTON_OK);
	ok_btn = GTK_WIDGET (g_list_last (GNOME_DIALOG (windowMetaData)->buttons)->data);
	GTK_WIDGET_SET_FLAGS (ok_btn, GTK_CAN_DEFAULT);

	// close button
	//
	gnome_dialog_append_button(GNOME_DIALOG(windowMetaData), GNOME_STOCK_BUTTON_CLOSE);
	cancel_btn = GTK_WIDGET (g_list_last (GNOME_DIALOG (windowMetaData)->buttons)->data);
	GTK_WIDGET_SET_FLAGS (cancel_btn, GTK_CAN_DEFAULT);

	// connect signals

	g_signal_connect (G_OBJECT (windowMetaData),
			  "close",
			  G_CALLBACK(cancel_callback),
			  (gpointer) this);
	
	g_signal_connect(G_OBJECT(ok_btn), "clicked", G_CALLBACK(ok_callback), (gpointer)this);
	g_signal_connect(G_OBJECT(cancel_btn), "clicked", G_CALLBACK(cancel_callback), (gpointer)this);
	
	// the catch-alls
	
	g_signal_connect(G_OBJECT(windowMetaData), "delete_event", G_CALLBACK(delete_callback), (gpointer)this);
	g_signal_connect_after(G_OBJECT(windowMetaData), "destroy", NULL, NULL);

	setDefaultButton (GNOME_DIALOG(windowMetaData), 1);
	return windowMetaData;
}
