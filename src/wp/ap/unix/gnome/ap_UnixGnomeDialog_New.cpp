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

#include "ut_types.h"
#include "ut_string.h"
#include "ut_units.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "ut_dialogHelper.h"

#include "gr_UnixGraphics.h"
#include "fl_BlockLayout.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"
#include "xap_Prefs.h"

#include "ap_Dialog_Id.h"
#include "ap_Prefs_SchemeIds.h"

#include "ap_Strings.h"
#include "ap_UnixGnomeDialog_New.h"

#include <gnome.h>
#include "ut_dialogHelper.h"

XAP_Dialog * AP_UnixGnomeDialog_New::static_constructor(XAP_DialogFactory * pFactory,
														XAP_Dialog_Id id)
{
    AP_UnixGnomeDialog_New * p = new AP_UnixGnomeDialog_New(pFactory,id);
    return p;
}

AP_UnixGnomeDialog_New::AP_UnixGnomeDialog_New(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id id)
	: AP_UnixDialog_New(pDlgFactory,id)
{
}

AP_UnixGnomeDialog_New::~AP_UnixGnomeDialog_New(void)
{
}

/*****************************************************************/
/*****************************************************************/

static void
cb_close (GtkWidget * w, AP_UnixGnomeDialog_New * dlg)
{
  UT_ASSERT(dlg);
  dlg->event_Cancel();
}

GtkWidget* AP_UnixGnomeDialog_New::_constructWindow (void )
{
  	GtkWidget * windowNew;
	GtkWidget * buttonOk;
	GtkWidget * buttonCancel;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	windowNew = gnome_dialog_new (pSS->getValue(AP_STRING_ID_DLG_NEW_Title), NULL);
	
	// ok button
	gnome_dialog_append_button(GNOME_DIALOG(windowNew), GNOME_STOCK_BUTTON_OK);
	buttonOk = GTK_WIDGET (g_list_last (GNOME_DIALOG (windowNew)->buttons)->data);
	GTK_WIDGET_SET_FLAGS (buttonOk, GTK_CAN_DEFAULT);

	// cancel button
	gnome_dialog_append_button(GNOME_DIALOG(windowNew), GNOME_STOCK_BUTTON_CANCEL);
	buttonCancel = GTK_WIDGET (g_list_last (GNOME_DIALOG (windowNew)->buttons)->data);
	GTK_WIDGET_SET_FLAGS (buttonCancel, GTK_CAN_DEFAULT);

	// catch the close signal
	gtk_signal_connect (GTK_OBJECT (windowNew),
						"close",
						GTK_SIGNAL_FUNC(cb_close),
						(gpointer) this);

	// assign these pointers to (private-ish) data members
	m_mainWindow   = windowNew;
	m_buttonOk     = buttonOk;
	m_buttonCancel = buttonCancel;

	_constructWindowContents(GNOME_DIALOG(windowNew)->vbox);
	_connectSignals();
	setDefaultButton (GNOME_DIALOG(windowNew), 1);

	return windowNew;
}
