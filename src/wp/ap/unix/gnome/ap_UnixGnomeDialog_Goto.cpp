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
#include "ap_Dialog_Goto.h"
#include "ap_UnixGnomeDialog_Goto.h"

/*****************************************************************/
XAP_Dialog * AP_UnixGnomeDialog_Goto::static_constructor(XAP_DialogFactory * pFactory,
														 XAP_Dialog_Id id)
{
	AP_UnixGnomeDialog_Goto * p = new AP_UnixGnomeDialog_Goto(pFactory,id);
	return p;
}

AP_UnixGnomeDialog_Goto::AP_UnixGnomeDialog_Goto(XAP_DialogFactory * pDlgFactory,
												 XAP_Dialog_Id id)
	: AP_UnixDialog_Goto(pDlgFactory, id)
{
}

AP_UnixGnomeDialog_Goto::~AP_UnixGnomeDialog_Goto(void)
{
}

GtkWidget * AP_UnixGnomeDialog_Goto::_constructWindow (void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	ConstructWindowName();
	m_wMainWindow = gnome_dialog_new (m_WindowName, NULL);

	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (m_wMainWindow)->vbox),
						_constructWindowContents (), TRUE, TRUE, 0);

	// TODO: This call must be in _constructWindowContents
	gtk_window_add_accel_group (GTK_WINDOW (m_wMainWindow), m_accelGroup);
	
	// container for buttons
	gnome_dialog_append_button (GNOME_DIALOG (m_wMainWindow), GNOME_STOCK_BUTTON_PREV);
	m_wPrev = GTK_WIDGET (g_list_last (GNOME_DIALOG (m_wMainWindow)->buttons)->data);
	gnome_dialog_append_button (GNOME_DIALOG (m_wMainWindow), GNOME_STOCK_BUTTON_NEXT);
	m_wNext = GTK_WIDGET (g_list_last (GNOME_DIALOG (m_wMainWindow)->buttons)->data);
	gnome_dialog_append_button_with_pixmap (GNOME_DIALOG (m_wMainWindow),
											pSS->getValue (AP_STRING_ID_DLG_Goto_Btn_Goto),
											GNOME_STOCK_PIXMAP_JUMP_TO);
	m_wGoto = GTK_WIDGET (g_list_last (GNOME_DIALOG (m_wMainWindow)->buttons)->data);
	gnome_dialog_append_button (GNOME_DIALOG (m_wMainWindow), GNOME_STOCK_BUTTON_CLOSE);
	m_wClose = GTK_WIDGET (g_list_last (GNOME_DIALOG (m_wMainWindow)->buttons)->data);

	gtk_widget_show_all (m_wMainWindow);
	_connectSignals ();

	return (m_wMainWindow);
}
