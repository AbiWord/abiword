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

#include <gnome.h>
#include <stdlib.h>
#include <string.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_dialogHelper.h"

#include "xap_Dialog_Id.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Insert_DateTime.h"
#include "ap_UnixGnomeDialog_Insert_DateTime.h"

/*****************************************************************/
XAP_Dialog * AP_UnixGnomeDialog_Insert_DateTime::static_constructor(XAP_DialogFactory * pFactory,
								    XAP_Dialog_Id id)
{
	AP_UnixGnomeDialog_Insert_DateTime * p = new AP_UnixGnomeDialog_Insert_DateTime(pFactory,id);
	return p;
}

AP_UnixGnomeDialog_Insert_DateTime::AP_UnixGnomeDialog_Insert_DateTime(XAP_DialogFactory * pDlgFactory,
							    XAP_Dialog_Id id)
	: AP_UnixDialog_Insert_DateTime(pDlgFactory, id)
{
}

AP_UnixGnomeDialog_Insert_DateTime::~AP_UnixGnomeDialog_Insert_DateTime(void)
{
}

static void
cb_close (GtkWidget * w, AP_UnixGnomeDialog_Insert_DateTime * dlg)
{
  dlg->event_Cancel();
}

GtkWidget * AP_UnixGnomeDialog_Insert_DateTime::_constructWindow (void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	m_windowMain = gnome_dialog_new (pSS->getValue(AP_STRING_ID_DLG_DateTime_DateTimeTitle), NULL);

	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (m_windowMain)->vbox),
			    _constructWindowContents (), TRUE, TRUE, 0);

	// container for buttons
	gnome_dialog_append_button (GNOME_DIALOG (m_windowMain), GNOME_STOCK_BUTTON_OK);
	m_buttonOK = GTK_WIDGET (g_list_last (GNOME_DIALOG (m_windowMain)->buttons)->data);

	gnome_dialog_append_button (GNOME_DIALOG (m_windowMain), GNOME_STOCK_BUTTON_CANCEL);
	m_buttonCancel = GTK_WIDGET (g_list_last (GNOME_DIALOG (m_windowMain)->buttons)->data);

	//gtk_widget_show_all (m_windowMain);
	gtk_signal_connect (GTK_OBJECT(m_windowMain),
			    "close",
			    GTK_SIGNAL_FUNC(cb_close),
			    (gpointer)this);

	_connectSignals ();
	setDefaultButton (GNOME_DIALOG(m_windowMain), 1);

	return (m_windowMain);
}
