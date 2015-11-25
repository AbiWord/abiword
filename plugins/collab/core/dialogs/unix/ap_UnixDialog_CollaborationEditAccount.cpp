/* Copyright (C) 2010 AbiSource Corporation B.V.
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#include "xap_App.h"
#include "ap_UnixApp.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"
#include "xap_UnixDialogHelper.h"
#include "ut_string_class.h"
#include <xp/AbiCollabSessionManager.h>
#include <account/xp/AccountHandler.h>

#include "ap_UnixDialog_CollaborationEditAccount.h"

XAP_Dialog * AP_UnixDialog_CollaborationEditAccount::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	return static_cast<XAP_Dialog *>(new AP_UnixDialog_CollaborationEditAccount(pFactory, id));
}
pt2Constructor ap_Dialog_CollaborationEditAccount_Constructor = &AP_UnixDialog_CollaborationEditAccount::static_constructor;

AP_UnixDialog_CollaborationEditAccount::AP_UnixDialog_CollaborationEditAccount(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: AP_Dialog_CollaborationEditAccount(pDlgFactory, id),
	m_wWindowMain(NULL),
	m_wOk(NULL)
{
}

void AP_UnixDialog_CollaborationEditAccount::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);
	
    // Build the dialog's window
	m_wWindowMain = _constructWindow();
	UT_return_if_fail(m_wWindowMain);

	_populateWindowData();

	switch ( abiRunModalDialog ( GTK_DIALOG(m_wWindowMain),
								 pFrame, this, GTK_RESPONSE_OK, false ) )
	{
		case GTK_RESPONSE_CANCEL:
			m_answer = AP_UnixDialog_CollaborationEditAccount::a_CANCEL;
			break;
		case GTK_RESPONSE_OK:
		{
			AccountHandler* pHandler = getAccountHandler();
			UT_return_if_fail(pHandler);
			pHandler->storeProperties();
			m_answer = AP_UnixDialog_CollaborationEditAccount::a_OK;
			break;
		}
		default:
			m_answer = AP_UnixDialog_CollaborationEditAccount::a_CANCEL;
			break;
	}

	abiDestroyWidget(m_wWindowMain);
}

/*****************************************************************/
GtkWidget * AP_UnixDialog_CollaborationEditAccount::_constructWindow(void)
{
	GtkWidget* window;
	//const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
	
	// load the dialog from the UI file
	GtkBuilder* builder = newDialogBuilder("ap_UnixDialog_CollaborationEditAccount.ui");
	
	// Update our member variables with the important widgets that 
	// might need to be queried or altered later
	window = GTK_WIDGET(gtk_builder_get_object(builder, "ap_UnixDialog_CollaborationEditAccount"));
	GObject* object = gtk_builder_get_object(builder, "vbWidgetEmbedding");
	m_wEmbeddingParent = GTK_BOX(object);
	m_wOk = GTK_WIDGET(gtk_builder_get_object(builder, "btOK"));

	// set the dialog title
	// TODO
	
	// localize the strings in our dialog, and set tags for some widgets
	// TODO

	g_object_unref(G_OBJECT(builder));
	return window;
}

void AP_UnixDialog_CollaborationEditAccount::_populateWindowData()
{
	AccountHandler* pHandler = getAccountHandler();
	UT_return_if_fail(pHandler);

	void* embeddingParent = _getEmbeddingParent();
	UT_return_if_fail(embeddingParent);

	pHandler->embedDialogWidgets(embeddingParent);

	// load all the (default) values
	pHandler->loadProperties();
}
