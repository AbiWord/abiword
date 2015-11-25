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

#include "ap_UnixDialog_CollaborationShare.h"

enum
{
	SHARE_COLUMN = 0,
	DESC_COLUMN,
	BUDDY_COLUMN
};

static void s_ok_clicked(GtkWidget * /*wid*/, AP_UnixDialog_CollaborationShare * dlg)
{
	dlg->eventOk();
}

static void s_account_changed(GtkWidget * /*wid*/, AP_UnixDialog_CollaborationShare * dlg)
{
	dlg->eventAccountChanged();
}

static void s_share_toggled (GtkCellRendererToggle * /*cell*/, gchar * path_str, AP_UnixDialog_CollaborationShare * dlg)
{
	dlg->eventToggle(path_str);
}

XAP_Dialog * AP_UnixDialog_CollaborationShare::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	return static_cast<XAP_Dialog *>(new AP_UnixDialog_CollaborationShare(pFactory, id));
}
pt2Constructor ap_Dialog_CollaborationShare_Constructor = &AP_UnixDialog_CollaborationShare::static_constructor;

AP_UnixDialog_CollaborationShare::AP_UnixDialog_CollaborationShare(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: AP_Dialog_CollaborationShare(pDlgFactory, id),
	m_wWindowMain(NULL),
	m_wAccount(NULL),
	m_wAccountHint(NULL),
	m_wAccountHintSpacer(NULL),
	m_wAccountHintHbox(NULL),
	m_wBuddyTree(NULL),
	m_pAccountModel(NULL),
	m_pBuddyModel(NULL),
	m_crToggle(NULL),
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

	_freeBuddyList();
	
	abiDestroyWidget(m_wWindowMain);
}

/*****************************************************************/
GtkWidget * AP_UnixDialog_CollaborationShare::_constructWindow(void)
{
	GtkWidget* window;
	//const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
	
	// load the dialog from the UI file
	GtkBuilder* builder = newDialogBuilder("ap_UnixDialog_CollaborationShare.ui");
	
	// Update our member variables with the important widgets that 
	// might need to be queried or altered later
	window = GTK_WIDGET(gtk_builder_get_object(builder, "ap_UnixDialog_CollaborationShare"));
	m_wAccount = GTK_WIDGET(gtk_builder_get_object(builder, "cbAccount"));
	m_wAccountHint = GTK_WIDGET(gtk_builder_get_object(builder, "lbAccountHint"));
	m_wAccountHintSpacer = GTK_WIDGET(gtk_builder_get_object(builder, "spAccountHint"));
	m_wAccountHintHbox = GTK_WIDGET(gtk_builder_get_object(builder, "hbAccountHint"));
	m_crToggle = G_OBJECT(gtk_builder_get_object(builder, "crToggle"));
	m_wBuddyTree = GTK_WIDGET(gtk_builder_get_object(builder, "tvBuddies"));
	m_pBuddyModel = GTK_LIST_STORE(gtk_builder_get_object(builder, "lsBuddies"));
	m_wOk = GTK_WIDGET(gtk_builder_get_object(builder, "btOK"));

	// make sure the buddy list is sorted
	gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE (m_pBuddyModel), DESC_COLUMN, GTK_SORT_ASCENDING);

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

	g_signal_connect (m_crToggle, 
	                  		"toggled", 
	                  		G_CALLBACK (s_share_toggled), 
	                  		static_cast<gpointer>(this));
	
	g_object_unref(G_OBJECT(builder));
	return window;
}

void AP_UnixDialog_CollaborationShare::_populateWindowData()
{
	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_if_fail(pManager);

	// populate the account combobox
	GtkListStore* store = gtk_list_store_new (2, G_TYPE_STRING, G_TYPE_POINTER);
	GtkTreeIter iter;

	AccountHandler* pShareeableAcount = _getShareableAccountHandler();
	if (pShareeableAcount)
	{
			gtk_list_store_append (store, &iter);
			gtk_list_store_set (store, &iter,
						0, pShareeableAcount->getDescription().utf8_str(),
						1, pShareeableAcount,
						-1);
			gtk_widget_set_sensitive(m_wAccount, false);
	}
	else
	{
		for (std::vector<AccountHandler*>::const_iterator cit = pManager->getAccounts().begin(); cit != pManager->getAccounts().end(); cit++)
		{
			AccountHandler* pAccount = *cit;
			UT_continue_if_fail(pAccount);

			if (!pAccount->isOnline() || !pAccount->canManuallyStartSession())
				continue;

			gtk_list_store_append (store, &iter);
			gtk_list_store_set (store, &iter,
						0, pAccount->getDescription().utf8_str(),
						1, pAccount,
						-1);
		}
		gtk_widget_set_sensitive(m_wAccount, true);
	}
	m_pAccountModel = GTK_TREE_MODEL (store);
	gtk_combo_box_set_model(GTK_COMBO_BOX(m_wAccount), m_pAccountModel);

	// if we have at least one account handler, then make sure the first one is selected
	if (pManager->getRegisteredAccountHandlers().size() > 0)
	{
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_wAccount), 0);
	}
	else
	{
		// nope, we don't have any account handler :'-(
		gtk_combo_box_set_active(GTK_COMBO_BOX(m_wAccount), -1);
	}
}

void AP_UnixDialog_CollaborationShare::_populateBuddyModel(bool refresh)
{
	UT_DEBUGMSG(("AP_UnixDialog_CollaborationShare::_populateBuddyModel()\n"));
	
	UT_return_if_fail(m_pBuddyModel);
	
	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_if_fail(pManager);
	
	AccountHandler* pHandler = _getActiveAccountHandler();
	UT_return_if_fail(pHandler);
	
	if (refresh)
	{
		// signal the account to refresh its buddy list ...
		pHandler->getBuddiesAsync(); // this function is really sync() atm; we need to rework this dialog to make it proper async

		// fetch the current ACL
		m_vAcl = _getSessionACL();
	}

	// clear out the old contents, if any
	_freeBuddyList();

	GtkTreeIter iter;
	for (UT_uint32 i = 0; i < pHandler->getBuddies().size(); i++)
	{
		BuddyPtr pBuddy = pHandler->getBuddies()[i];
		UT_continue_if_fail(pBuddy);
		
		if (!pBuddy->getHandler()->canShare(pBuddy))
		{
			UT_DEBUGMSG(("Not allowed to share with buddy: %s\n", pBuddy->getDescription().utf8_str()));
			continue;
		}

		// crap, we can't store shared pointers in the list store; use a 
		// hack to do it (which kinda defies the whole shared pointer thingy, 
		// but alas...)
		BuddyPtrWrapper* pWrapper = new BuddyPtrWrapper(pBuddy);
		gtk_list_store_append (m_pBuddyModel, &iter);
		gtk_list_store_set (m_pBuddyModel, &iter, 
				SHARE_COLUMN, _populateShareState(pBuddy),
				DESC_COLUMN, pBuddy->getDescription().utf8_str(), 
				BUDDY_COLUMN, pWrapper, 
				-1);
	}
	
	gtk_widget_show_all(m_wBuddyTree);
}

AccountHandler* AP_UnixDialog_CollaborationShare::_getActiveAccountHandler()
{
	GtkTreeIter iter;
	if (gtk_combo_box_get_active_iter(GTK_COMBO_BOX(m_wAccount), &iter))
	{
		gchar * str_data;
		gpointer* ptr_data;
		AccountHandler* pHandler = 0;

		gtk_tree_model_get (m_pAccountModel, &iter, 
                          0, &str_data,
                          1, &ptr_data,
                          -1);		
		
		pHandler = reinterpret_cast<AccountHandler*>(ptr_data);
		return pHandler;
	}
	return 0;
}

void AP_UnixDialog_CollaborationShare::eventOk()
{
	UT_DEBUGMSG(("AP_UnixDialog_CollaborationShare::eventOk()\n"));
	m_pAccount = _getActiveAccountHandler();
	_getSelectedBuddies(m_vAcl);
}

void AP_UnixDialog_CollaborationShare::eventToggle(gchar* path_str)
{
	UT_DEBUGMSG(("AP_UnixDialog_CollaborationShare::eventToggle()\n"));
	
	GtkTreePath *path = gtk_tree_path_new_from_string (path_str);
	GtkTreeIter iter;
	gboolean bshare;

	// toggle the share state
	gtk_tree_model_get_iter (GTK_TREE_MODEL (m_pBuddyModel), &iter, path);
	gtk_tree_model_get (GTK_TREE_MODEL (m_pBuddyModel), &iter, SHARE_COLUMN, &bshare, -1);
	gtk_list_store_set (m_pBuddyModel, &iter, SHARE_COLUMN, !bshare, -1);

	// clean up
	gtk_tree_path_free (path);
}

void AP_UnixDialog_CollaborationShare::_setAccountHint(const UT_UTF8String& sHint)
{
	UT_DEBUGMSG(("AP_UnixDialog_CollaborationShare::_setAccountHint() - sHint: %s\n", sHint.utf8_str()));
	
	// show/hide the hint widgets
	GValue val;
	val.g_type = 0;
	g_value_init (&val, G_TYPE_BOOLEAN);
	g_value_set_boolean (&val, sHint != "");
	g_object_set_property(G_OBJECT(m_wAccountHintSpacer), "visible", &val);
	g_object_set_property(G_OBJECT(m_wAccountHintHbox), "visible", &val);

	// set the hint
	gtk_label_set_text(GTK_LABEL(m_wAccountHint), sHint.utf8_str());
}

void AP_UnixDialog_CollaborationShare::_refreshWindow()
{
	UT_DEBUGMSG(("AP_UnixDialog_CollaborationShare::_refreshWindow()\n"));
	_populateBuddyModel(false);
}

void AP_UnixDialog_CollaborationShare::_getSelectedBuddies(std::vector<std::string>& vACL)
{
	UT_DEBUGMSG(("AP_UnixDialog_CollaborationShare::_getSelectedBuddies()\n"));
	vACL.clear();

	GtkTreeIter iter;
	if (!gtk_tree_model_get_iter_first(GTK_TREE_MODEL (m_pBuddyModel), &iter))
		return;

	do
	{
		gboolean bshare;
		gpointer buddy_wrapper = NULL;
		gtk_tree_model_get (GTK_TREE_MODEL (m_pBuddyModel), &iter, SHARE_COLUMN, &bshare, -1);
		gtk_tree_model_get (GTK_TREE_MODEL (m_pBuddyModel), &iter, BUDDY_COLUMN, &buddy_wrapper, -1);
		if (bshare && buddy_wrapper)
		{
			BuddyPtr pBuddy = reinterpret_cast<BuddyPtrWrapper*>(buddy_wrapper)->getBuddy();
			vACL.push_back(pBuddy->getDescriptor(false).utf8_str());
		}
	} while (gtk_tree_model_iter_next(GTK_TREE_MODEL (m_pBuddyModel), &iter));
}

void AP_UnixDialog_CollaborationShare::_freeBuddyList()
{
	UT_DEBUGMSG(("AP_UnixDialog_CollaborationShare::_freeBuddyList()\n"));

	GtkTreeIter iter;
	if (!gtk_tree_model_get_iter_first(GTK_TREE_MODEL (m_pBuddyModel), &iter))
		return;

	do
	{
		gpointer buddy_wrapper = NULL;
		gtk_tree_model_get (GTK_TREE_MODEL (m_pBuddyModel), &iter, BUDDY_COLUMN, &buddy_wrapper, -1);
		if (buddy_wrapper)
		{
			BuddyPtrWrapper* pWrap = reinterpret_cast<BuddyPtrWrapper*>(buddy_wrapper);
			DELETEP(pWrap);
		}
	} while (gtk_tree_model_iter_next(GTK_TREE_MODEL (m_pBuddyModel), &iter));

	gtk_list_store_clear(m_pBuddyModel);
}
