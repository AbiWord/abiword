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
#include <gnome.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

// This header defines some functions for Unix dialogs,
// like centering them, measuring them, etc.
#include "ut_dialogHelper.h"

#include "gr_UnixGraphics.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

#include "ap_Dialog_Id.h"

#include "ap_Strings.h"

#include "ap_Preview_Paragraph.h"
#include "ap_UnixGnomeDialog_Paragraph.h"

/*****************************************************************/

static void
cb_close (GtkWidget * w, AP_UnixGnomeDialog_Paragraph * dlg)
{
  UT_ASSERT(dlg);
  dlg->event_Cancel();
}

XAP_Dialog * AP_UnixGnomeDialog_Paragraph::static_constructor(XAP_DialogFactory * pFactory,
														 XAP_Dialog_Id id)
{
	AP_UnixGnomeDialog_Paragraph * p = new AP_UnixGnomeDialog_Paragraph(pFactory,id);
	return p;
}

AP_UnixGnomeDialog_Paragraph::AP_UnixGnomeDialog_Paragraph(XAP_DialogFactory * pDlgFactory,
												 XAP_Dialog_Id id)
	: AP_UnixDialog_Paragraph(pDlgFactory,id)
{
}

AP_UnixGnomeDialog_Paragraph::~AP_UnixGnomeDialog_Paragraph(void)
{
}

/*****************************************************************/

GtkWidget * AP_UnixGnomeDialog_Paragraph::_constructWindow(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	GtkWidget * windowContents;
	XML_Char * unixstr = NULL;

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_ParaTitle) );
	m_windowMain = gnome_dialog_new (unixstr, NULL);
	gtk_widget_set_usize (m_windowMain, 441, -2);
	FREEP(unixstr);
	gtk_window_set_policy (GTK_WINDOW (m_windowMain), FALSE, FALSE, FALSE);

	UT_XML_cloneNoAmpersands(unixstr, pSS->getValue(AP_STRING_ID_DLG_Para_ButtonTabs));
	gnome_dialog_append_button(GNOME_DIALOG(m_windowMain), unixstr);
	FREEP(unixstr);

	gnome_dialog_append_button(GNOME_DIALOG(m_windowMain), GNOME_STOCK_BUTTON_OK);
	gnome_dialog_append_button(GNOME_DIALOG(m_windowMain), GNOME_STOCK_BUTTON_CANCEL);

	windowContents = _constructWindowContents(m_windowMain);
	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (m_windowMain)->vbox), windowContents, TRUE, TRUE, 0);

	m_buttonTabs = GTK_WIDGET (g_list_nth_data (GNOME_DIALOG (m_windowMain)->buttons, 0));
	m_buttonOK = GTK_WIDGET (g_list_nth_data (GNOME_DIALOG (m_windowMain)->buttons, 1));
	m_buttonCancel = GTK_WIDGET (g_list_nth_data (GNOME_DIALOG (m_windowMain)->buttons, 2));

	gtk_signal_connect (GTK_OBJECT(m_windowMain),
			    "close",
			    GTK_SIGNAL_FUNC(cb_close),
			    (gpointer)this);

	setDefaultButton (GNOME_DIALOG(m_windowMain), 2);
	return m_windowMain;
}
