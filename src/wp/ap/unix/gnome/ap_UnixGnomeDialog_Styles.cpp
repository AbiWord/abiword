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

static void
cb_close (GtkWidget * w, AP_UnixGnomeDialog_Styles * dlg)
{
  UT_ASSERT(dlg);
  dlg->event_Close();
}

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

	// apply button
	//
	gnome_dialog_append_button(GNOME_DIALOG(windowStyles), GNOME_STOCK_BUTTON_APPLY);
	m_wbuttonApply = GTK_WIDGET (g_list_last (GNOME_DIALOG (windowStyles)->buttons)->data);
	GTK_WIDGET_SET_FLAGS (m_wbuttonApply, GTK_CAN_DEFAULT);

	// close button
	//
	gnome_dialog_append_button(GNOME_DIALOG(windowStyles), GNOME_STOCK_BUTTON_CLOSE);
	m_wbuttonClose = GTK_WIDGET (g_list_last (GNOME_DIALOG (windowStyles)->buttons)->data);
	GTK_WIDGET_SET_FLAGS (m_wbuttonClose, GTK_CAN_DEFAULT);

	gtk_signal_connect (GTK_OBJECT (windowStyles),
						"close",
						GTK_SIGNAL_FUNC(cb_close),
						(gpointer) this);

	_connectsignals();
	setDefaultButton (GNOME_DIALOG(windowStyles), 1);
	return windowStyles;
}

void   AP_UnixGnomeDialog_Styles::_constructGnomeModifyButtons( GtkWidget * dialog_action_area)
{
	GtkWidget *bottomButtons;
	GtkWidget *buttonOK;
	GtkWidget *cancelButton;
	GtkWidget *FormatMenu;
	GtkWidget *shortCutButton;
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	bottomButtons = gtk_hbox_new (TRUE, 5);
	gtk_widget_show (bottomButtons);
	gtk_box_pack_start (GTK_BOX (dialog_action_area), bottomButtons, TRUE, TRUE, 0);
	gtk_container_set_border_width (GTK_CONTAINER (bottomButtons), 3);
	
	buttonOK = gnome_stock_button ( GNOME_STOCK_BUTTON_OK );
	gtk_widget_show (buttonOK);
	gtk_box_pack_start (GTK_BOX (bottomButtons), buttonOK, TRUE, TRUE, 0);

	cancelButton = gnome_stock_button ( GNOME_STOCK_BUTTON_CANCEL );
	gtk_widget_show (cancelButton);
	gtk_box_pack_start (GTK_BOX (bottomButtons), cancelButton, TRUE, TRUE, 0);

	FormatMenu = gtk_option_menu_new ();
	gtk_widget_show (FormatMenu);
	gtk_box_pack_start (GTK_BOX (bottomButtons), FormatMenu, FALSE, FALSE, 0);

	_constructFormatList(FormatMenu);

	shortCutButton = gtk_button_new_with_label (pSS->getValue(AP_STRING_ID_DLG_Styles_ModifyShortCut));
	gtk_widget_show (shortCutButton);
	gtk_box_pack_start (GTK_BOX (bottomButtons), shortCutButton, TRUE, TRUE, 0);

	m_wModifyOk = buttonOK;
	m_wModifyCancel = cancelButton;
	m_wFormatMenu = FormatMenu;
	m_wModifyShortCutKey = shortCutButton;
	
}
