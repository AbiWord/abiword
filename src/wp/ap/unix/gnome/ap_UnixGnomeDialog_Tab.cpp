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
#include "ap_UnixGnomeDialog_Tab.h"

#include <gnome.h>
#include "ut_dialogHelper.h"

XAP_Dialog * AP_UnixGnomeDialog_Tab::static_constructor(XAP_DialogFactory * pFactory,
							XAP_Dialog_Id id)
{
    AP_UnixGnomeDialog_Tab * p = new AP_UnixGnomeDialog_Tab(pFactory,id);
    return p;
}

AP_UnixGnomeDialog_Tab::AP_UnixGnomeDialog_Tab(XAP_DialogFactory * pDlgFactory,
					       XAP_Dialog_Id id)
  : AP_UnixDialog_Tab(pDlgFactory,id)
{
}

AP_UnixGnomeDialog_Tab::~AP_UnixGnomeDialog_Tab(void)
{
}

/*****************************************************************/

GtkWidget* AP_UnixGnomeDialog_Tab::_constructWindow (void )
{
	GtkWidget *windowTabs;

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	windowTabs = gnome_dialog_new(pSS->getValue(AP_STRING_ID_DLG_Tab_TabTitle), NULL);
       	m_windowMain = windowTabs;

	_constructWindowContents(GNOME_DIALOG(windowTabs)->vbox);

	gtk_widget_show_all(GNOME_DIALOG(windowTabs)->vbox);
	gtk_widget_show_all(windowTabs);

	gtk_signal_connect (GTK_OBJECT(windowTabs),
			    "close",
			    GTK_SIGNAL_FUNC(s_cancel_clicked),
			    (gpointer)this);

	return windowTabs;
}

void    AP_UnixGnomeDialog_Tab::_constructGnomeButtons(GtkWidget * /* ignored */)
{
	GtkWidget *buttonOK;
	GtkWidget *buttonCancel;
	GtkWidget *buttonApply;

	//
	// Gnome buttons
	//


	// apply button
	gnome_dialog_append_button(GNOME_DIALOG(m_windowMain), 
				   GNOME_STOCK_BUTTON_APPLY);
	buttonApply = GTK_WIDGET (g_list_last (GNOME_DIALOG (m_windowMain)->buttons)->data);
	gtk_widget_ref (buttonApply);
	gtk_object_set_data_full (GTK_OBJECT (m_windowMain), "buttonApply", buttonApply,
							(GtkDestroyNotify) gtk_widget_unref);
	GTK_WIDGET_SET_FLAGS (buttonApply, GTK_CAN_DEFAULT);


	// ok button
	gnome_dialog_append_button(GNOME_DIALOG(m_windowMain),
				   GNOME_STOCK_BUTTON_OK);
	buttonOK = GTK_WIDGET (g_list_last(GNOME_DIALOG(m_windowMain)->buttons)->data);
	gtk_widget_ref (buttonOK);
	gtk_object_set_data_full (GTK_OBJECT (m_windowMain), "buttonOK", buttonOK,
				  (GtkDestroyNotify) gtk_widget_unref);
	GTK_WIDGET_SET_FLAGS (buttonOK, GTK_CAN_DEFAULT);


	// cancel button
	gnome_dialog_append_button(GNOME_DIALOG(m_windowMain), 
				   GNOME_STOCK_BUTTON_CANCEL);
	buttonCancel = GTK_WIDGET (g_list_last (GNOME_DIALOG (m_windowMain)->buttons)->data);
	gtk_widget_ref(buttonCancel);
	gtk_object_set_data_full (GTK_OBJECT (m_windowMain), "buttonCancel", buttonCancel,
				  (GtkDestroyNotify) gtk_widget_unref);
	GTK_WIDGET_SET_FLAGS (buttonCancel, GTK_CAN_DEFAULT);

	m_buttonApply = buttonApply;
	m_buttonOK = buttonOK;
	m_buttonCancel = buttonCancel;
	setDefaultButton (GNOME_DIALOG(m_windowMain), 2);
}
