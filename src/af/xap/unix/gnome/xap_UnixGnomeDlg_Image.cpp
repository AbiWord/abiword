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

#include "xap_UnixGnomeDlg_Image.h"

static void
cb_close (GtkWidget * w, XAP_UnixGnomeDialog_Image * dlg)
{
  UT_ASSERT(dlg);
  dlg->event_Cancel();
}

XAP_UnixGnomeDialog_Image::XAP_UnixGnomeDialog_Image(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id) : XAP_UnixDialog_Image(pDlgFactory, id)
{
}

XAP_UnixGnomeDialog_Image::~XAP_UnixGnomeDialog_Image(void)
{
}

XAP_Dialog * XAP_UnixGnomeDialog_Image::static_constructor(XAP_DialogFactory *pDlgFactory, XAP_Dialog_Id id)
{
  XAP_UnixGnomeDialog_Image *p = new XAP_UnixGnomeDialog_Image(pDlgFactory, id);
  return p;
}

GtkWidget * XAP_UnixGnomeDialog_Image::_constructWindow(void)
{
  	GtkWidget * windowImage;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	windowImage = gnome_dialog_new (pSS->getValue(XAP_STRING_ID_DLG_Image_Title), NULL);

	_constructWindowContents(GNOME_DIALOG(windowImage)->vbox);

	// ok button
	//
	gnome_dialog_append_button(GNOME_DIALOG(windowImage), GNOME_STOCK_BUTTON_OK);
	m_buttonOK = GTK_WIDGET (g_list_last (GNOME_DIALOG (windowImage)->buttons)->data);
	GTK_WIDGET_SET_FLAGS (m_buttonOK, GTK_CAN_DEFAULT);

	// cancel button
	//
	gnome_dialog_append_button(GNOME_DIALOG(windowImage), GNOME_STOCK_BUTTON_CANCEL);
	m_buttonCancel = GTK_WIDGET (g_list_last (GNOME_DIALOG (windowImage)->buttons)->data);
	GTK_WIDGET_SET_FLAGS (m_buttonCancel, GTK_CAN_DEFAULT);

	gtk_signal_connect (GTK_OBJECT(windowImage),
			    "close",
			    GTK_SIGNAL_FUNC(cb_close),
			    (gpointer) this);

	mMainWindow = windowImage;
	setDefaultButton (GNOME_DIALOG(windowImage), 1);
	_connectSignals();
	return windowImage;
}
