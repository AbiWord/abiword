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
#include "ap_Dialog_Lists.h"
#include "ap_UnixGnomeDialog_Lists.h"

/*****************************************************************/
XAP_Dialog * AP_UnixGnomeDialog_Lists::static_constructor(XAP_DialogFactory * pFactory, 
							  XAP_Dialog_Id id)
{
	AP_UnixGnomeDialog_Lists * p = new AP_UnixGnomeDialog_Lists(pFactory,id);
	return p;
}

AP_UnixGnomeDialog_Lists::AP_UnixGnomeDialog_Lists(XAP_DialogFactory * pDlgFactory,
						   XAP_Dialog_Id id)
  : AP_UnixDialog_Lists(pDlgFactory, id)
{
}

AP_UnixGnomeDialog_Lists::~AP_UnixGnomeDialog_Lists(void)
{
}

GtkWidget * AP_UnixGnomeDialog_Lists::_constructWindow(void)
{
	ConstructWindowName();
	m_wMainWindow = gnome_dialog_new (m_WindowName, NULL);

	gtk_box_pack_start (GTK_BOX (GNOME_DIALOG (m_wMainWindow)->vbox),
			    _constructWindowContents (), TRUE, TRUE, 0);

	// close button
	gnome_dialog_append_button(GNOME_DIALOG(m_wMainWindow), 
				   GNOME_STOCK_BUTTON_CLOSE);
	m_wClose = GTK_WIDGET (g_list_last (GNOME_DIALOG (m_wMainWindow)->buttons)->data);
	
	// apply button
	gnome_dialog_append_button(GNOME_DIALOG(m_wMainWindow), 
				   GNOME_STOCK_BUTTON_APPLY);
	m_wApply = GTK_WIDGET (g_list_last (GNOME_DIALOG (m_wMainWindow)->buttons)->data);

	gtk_widget_show_all(m_wMainWindow);
	_connectSignals();

	return (m_wMainWindow);
}
