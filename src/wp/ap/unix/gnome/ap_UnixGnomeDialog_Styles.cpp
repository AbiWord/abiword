/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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

#include "ap_UnixGnomeDialog_Styles.h"


AP_UnixGnomeDialog_Styles::AP_UnixGnomeDialog_Styles(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id) : AP_UnixDialog_Styles(pDlgFactory, id)
{
}

AP_UnixGnomeDialog_Styles::~AP_UnixGnomeDialog_Styles(void)
{
}

XAP_Dialog * AP_UnixGnomeDialog_Styles::static_constructor(XAP_DialogFactory *pDlgFactory, XAP_Dialog_Id id)
{
  AP_UnixGnomeDialog_Styles *p = new AP_UnixGnomeDialog_Styles(pDlgFactory, id);
  return p;
}

GtkWidget * AP_UnixGnomeDialog_Styles::_constructWindow(void)
{
  	GtkWidget * windowStyles;
	GtkWidget * vboxContents;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	windowStyles = gnome_dialog_new (pSS->getValue(AP_STRING_ID_DLG_Styles_StylesTitle), NULL);

	vboxContents = _constructWindowContents(GNOME_DIALOG(windowStyles)->vbox);
	gtk_widget_show (vboxContents);
	gtk_container_add(GTK_CONTAINER(GNOME_DIALOG(windowStyles)->vbox), 
			  vboxContents);

	// ok button
	//
	gnome_dialog_append_button(GNOME_DIALOG(windowStyles), GNOME_STOCK_BUTTON_OK);
	m_wbuttonOk = GTK_WIDGET (g_list_last (GNOME_DIALOG (windowStyles)->buttons)->data);
	GTK_WIDGET_SET_FLAGS (m_wbuttonOk, GTK_CAN_DEFAULT);

	// cancel button
	//
	gnome_dialog_append_button(GNOME_DIALOG(windowStyles), GNOME_STOCK_BUTTON_CANCEL);
	m_wbuttonCancel = GTK_WIDGET (g_list_last (GNOME_DIALOG (windowStyles)->buttons)->data);
	GTK_WIDGET_SET_FLAGS (m_wbuttonCancel, GTK_CAN_DEFAULT);

	_connectsignals();
	return windowStyles;
}
