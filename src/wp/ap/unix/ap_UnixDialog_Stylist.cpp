/* AbiWord
 * Copyright (C) 2003 Dom Lachowicz
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

#include "xap_UnixDialogHelper.h"

#include "xap_App.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_UnixDialog_Stylist.h"

static void s_response(GtkWidget * wid, gint id, AP_UnixDialog_Stylist * me )
{
    abiDestroyWidget( wid ) ;// will emit signals for us
}

XAP_Dialog * AP_UnixDialog_Stylist::static_constructor(XAP_DialogFactory * pFactory,
														  XAP_Dialog_Id id)
{
	return new AP_UnixDialog_Stylist(pFactory,id);
}

AP_UnixDialog_Stylist::AP_UnixDialog_Stylist(XAP_DialogFactory * pDlgFactory,
												   XAP_Dialog_Id id)
	: AP_Dialog_Stylist(pDlgFactory,id), m_windowMain(0)
{
}

AP_UnixDialog_Stylist::~AP_UnixDialog_Stylist(void)
{
}

void AP_UnixDialog_Stylist::event_Close(void)
{
	destroy();
}

void AP_UnixDialog_Stylist::destroy(void)
{
	finalize();
	gtk_widget_destroy(m_windowMain);
	m_windowMain = NULL;
}

void AP_UnixDialog_Stylist::activate(void)
{
	UT_ASSERT (m_windowMain);
	gdk_window_raise (m_windowMain->window);
}

void AP_UnixDialog_Stylist::notifyActiveFrame(XAP_Frame *pFrame)
{
    UT_ASSERT(m_windowMain);
}

void AP_UnixDialog_Stylist::runModeless(XAP_Frame * pFrame)
{
	// Build the window's widgets and arrange them
	GtkWidget * mainWindow = _constructWindow();
	UT_return_if_fail(mainWindow);

	// Populate the window's data items
	//_populateWindowData();
	//_connectSignals();
	//abiSetupModelessDialog(GTK_DIALOG(mainWindow),pFrame,this,GTK_RESPONSE_CLOSE);
	startUpdater();
}

GtkWidget * AP_UnixDialog_Stylist::_constructWindow(void)
{
	return NULL;
}
