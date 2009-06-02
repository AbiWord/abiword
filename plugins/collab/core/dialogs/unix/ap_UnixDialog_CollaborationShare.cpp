/* Copyright (C) 2009 AbiSource Corporation B.V.
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

#include "xap_App.h"
#include "ap_UnixApp.h"
#include "xap_UnixApp.h"
#include "xap_Frame.h"
#include "xap_UnixDialogHelper.h"
#include "ut_string_class.h"
#include <xp/AbiCollabSessionManager.h>
#include <account/xp/AccountHandler.h>

#include "ap_UnixDialog_CollaborationShare.h"

static void s_ok_clicked(GtkWidget * /*wid*/, AP_UnixDialog_CollaborationShare * dlg)
{
	dlg->eventOk();
}

static void s_account_changed(GtkWidget * /*wid*/, AP_UnixDialog_CollaborationShare * dlg)
{
	dlg->eventAccountChanged();
}

XAP_Dialog * AP_UnixDialog_CollaborationShare::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	return static_cast<XAP_Dialog *>(new AP_UnixDialog_CollaborationShare(pFactory, id));
}
pt2Constructor ap_Dialog_CollaborationShare_Constructor = &AP_UnixDialog_CollaborationShare::static_constructor;

AP_UnixDialog_CollaborationShare::AP_UnixDialog_CollaborationShare(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: AP_Dialog_CollaborationShare(pDlgFactory, id),
	m_wWindowMain(NULL),
	m_wOk(NULL)
{
}

void AP_UnixDialog_CollaborationShare::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);
	
    // Build the dialog's window
	m_wWindowMain = _constructWindow();
	UT_return_if_fail(m_wWindowMain);

	_populateWindowData();

	switch ( abiRunModalDialog ( GTK_DIALOG(m_wWindowMain),
								 pFrame, this, GTK_RESPONSE_CANCEL, false ) )
	{
		case GTK_RESPONSE_CANCEL:
			m_answer = AP_UnixDialog_CollaborationShare::a_CANCEL;
			break;
		case GTK_RESPONSE_OK:
			m_answer = AP_UnixDialog_CollaborationShare::a_OK;
			break;			
		default:
			m_answer = AP_UnixDialog_CollaborationShare::a_CANCEL;
			break;
	}

	abiDestroyWidget(m_wWindowMain);
}

/*****************************************************************/
GtkWidget * AP_UnixDialog_CollaborationShare::_constructWindow(void)
{
	GtkWidget* window;
	//const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
	
	// get the path where our UI file is located
	std::string ui_path = static_cast<XAP_UnixApp*>(XAP_App::getApp())->getAbiSuiteAppUIDir() + "/ap_UnixDialog_CollaborationShare.xml";
	// load the dialog from the UI file
	GtkBuilder* builder = gtk_builder_new();
	gtk_builder_add_from_file(builder, ui_path.c_str(), NULL);
	
	// Update our member variables with the important widgets that 
	// might need to be queried or altered later
	window = GTK_WIDGET(gtk_builder_get_object(builder, "ap_UnixDialog_CollaborationShare"));
	m_wAccount = GTK_WIDGET(gtk_builder_get_object(builder, "cbAccount"));
	m_wOk = GTK_WIDGET(gtk_builder_get_object(builder, "btOK"));

	// set the dialog title
	// TODO
	
	// localize the strings in our dialog, and set tags for some widgets
	// TODO

	// connect our signals
	g_signal_connect(G_OBJECT(m_wOk),
							"clicked",
							G_CALLBACK(s_ok_clicked),
							static_cast<gpointer>(this));

	g_signal_connect(G_OBJECT(m_wAccount),
							"changed",
							G_CALLBACK(s_account_changed),
							static_cast<gpointer>(this));
	g_object_unref(G_OBJECT(builder));
	return window;
}

void AP_UnixDialog_CollaborationShare::_populateWindowData()
{
/*
	// populate the account type combobox
	GtkListStore* store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_POINTER);
	GtkTreeIter iter;
	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();

	for (std::map<UT_UTF8String, AccountHandlerConstructor>::const_iterator cit = pManager->getRegisteredAccountHandlers().begin(); cit != pManager->getRegisteredAccountHandlers().end(); cit++)
	{
		AccountHandlerConstructor pConstructor = cit->second;
		UT_continue_if_fail(pConstructor);

		// TODO: we need to free these somewhere
		AccountHandler* pHandler = pConstructor();
		if (pHandler)
		{
			gtk_list_store_append (store, &iter);
			gtk_list_store_set (store, &iter,
						0, pHandler->getDisplayType().utf8_str(),
						1, pHandler,
						-1);
		}
	}
	m_model = GTK_TREE_MODEL (store);
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_wAccountType), m_model);

	// if we have at least one account handler, then make sure the first one is selected
	if (pManager->getRegisteredAccountHandlers().size() > 0)
	{
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_wAccountType), 0);
	}
	else
	{
		// nope, we don't have any account handler :'-(
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_wAccountType), -1);
	}
*/
}

void AP_UnixDialog_CollaborationShare::eventOk()
{
	UT_DEBUGMSG(("AP_UnixDialog_CollaborationShare::eventOk()\n"));
	// TODO: implement me
}

void AP_UnixDialog_CollaborationShare::eventAccountChanged()
{
	UT_DEBUGMSG(("AP_UnixDialog_CollaborationShare::eventAccountChanged()\n"));
	// TODO: implement me
}
