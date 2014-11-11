/* Copyright (C) 2007 One Laptop Per Child
 * Copyright (C) 2009 Marc Maurer <uwog@uwog.net>
 * Copyright (C) 2010 AbiSource Corporation B.V.
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

#include <account/xp/Event.h>
#include <account/xp/AccountEvent.h>
#include <session/xp/AbiCollabSessionManager.h>
#include <session/xp/AbiCollab.h>
#include <core/account/xp/SessionEvent.h>
#include <ev_EditMethod.h>
#include <xap_App.h>
#include <fv_View.h>
#include <xap_Frame.h>
#include <xap_UnixApp.h>

#include <telepathy-glib/telepathy-glib.h>

#include "TelepathyUnixAccountHandler.h"
#include "DTubeBuddy.h"
#include "TelepathyChatroom.h"

static void
list_contacts_for_connection_cb(TpConnection* /*connection*/,
						guint n_contacts,
						TpContact * const *contacts,
						guint /*n_invalid*/,
						const TpHandle* /*invalid*/,
						const GError* error,
						gpointer user_data,
						GObject* /*weak_object*/)
{
	UT_DEBUGMSG(("list_contacts_for_connection_cb()\n"));
	UT_return_if_fail(!error);

	TelepathyAccountHandler* pHandler = reinterpret_cast<TelepathyAccountHandler*>(user_data);
	UT_return_if_fail(pHandler);

	UT_DEBUGMSG(("Got %d contacts!\n", n_contacts));
	for (UT_uint32 i = 0; i < n_contacts; i++)
	{
		TpContact* contact = contacts[i];
		UT_continue_if_fail(contact);
		UT_DEBUGMSG(("Alias: '%s'\n", tp_contact_get_alias(contact)));

		pHandler->addContact(contact);
	}
}

static void
tp_connection_get_contact_list_attributes_cb(TpConnection* connection,
						GHashTable *out_Attributes,
						const GError* error,
						gpointer user_data,
						GObject* /*weak_object*/)
{
	UT_DEBUGMSG(("tp_connection_get_contact_list_attributes\n"));
	if (error)
		UT_DEBUGMSG(("%s\n", error->message));
	UT_return_if_fail(!error);

	std::vector<TpHandle> handles;

	// get the list of contact handles
	gpointer key;
	GHashTableIter iter;
	g_hash_table_iter_init(&iter, out_Attributes);
	while (g_hash_table_iter_next(&iter, &key, NULL))
	{
		TpHandle contact_handle = GPOINTER_TO_UINT(key);
		handles.push_back(contact_handle);
	}

	// fetch the contacts belonging to the handles
	static TpContactFeature features[] = {
		TP_CONTACT_FEATURE_ALIAS,
		TP_CONTACT_FEATURE_PRESENCE
	};

	tp_connection_get_contacts_by_handle (connection,
			handles.size(), &handles[0],
			G_N_ELEMENTS (features), features,
			list_contacts_for_connection_cb,
			user_data, NULL, NULL);
}

static void
validate_connection(TpConnection* connection, gpointer user_data)
{
	UT_DEBUGMSG(("validate_connection()\n"));
	UT_return_if_fail(connection);

	// check if this connection supports MUC tubes
	TpCapabilities* caps = tp_connection_get_capabilities(connection);
	UT_return_if_fail(caps);

	if (!tp_capabilities_supports_dbus_tubes(caps, TP_HANDLE_TYPE_ROOM, NULL))
	{
		UT_DEBUGMSG(("Connection does not support MUC text channels\n"));
		return;
	}
	UT_DEBUGMSG(("Connection supports tube MUC rooms!\n"));

	// update the list of contacts for this connection
	tp_connection_get_contact_list_attributes(connection,
						-1,
						NULL,
						TRUE,
						tp_connection_get_contact_list_attributes_cb,
						user_data,
						NULL,
						NULL);
}

static void
prepare_connection_cb(GObject* connection, GAsyncResult *res, gpointer user_data)
{
	UT_DEBUGMSG(("prepare_connection_cb()\n"));

	if (!tp_proxy_prepare_finish(connection, res, NULL))
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return;
	}

	// proceed checking if this connection is usable
	validate_connection(reinterpret_cast<TpConnection*>(connection), user_data);
}

static void
list_connection_names_cb (const gchar * const *bus_names,
							gsize n,
							const gchar * const * cms,
							const gchar * const * protocols,
							const GError *error,
							gpointer user_data,
							GObject * /*unused*/)
{
	UT_DEBUGMSG(("list_connection_names_cb()\n"));
	TelepathyAccountHandler* pHandler = reinterpret_cast<TelepathyAccountHandler*>(user_data);
	UT_return_if_fail(pHandler);

	if (error != NULL)
	{
		UT_DEBUGMSG(("List connectiones failed: %s", error->message));
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return;
	}

	TpDBusDaemon* dbus = tp_dbus_daemon_dup(NULL);
	UT_return_if_fail(dbus);

	UT_DEBUGMSG(("Got %d connections:\n", (int)n));

	for (UT_uint32 i = 0; i < n; i++)
	{
		UT_DEBUGMSG(("%d: Bus name %s, connection manager %s, protocol %s\n", i+1, bus_names[i], cms[i], protocols[i]));
		TpConnection* connection = tp_connection_new (dbus, bus_names[i], NULL, NULL);
		UT_continue_if_fail(connection);

		TpCapabilities* caps = tp_connection_get_capabilities(connection);
		if (!caps)
		{
			GQuark features[] = { TP_CONNECTION_FEATURE_CAPABILITIES, 0 };
			tp_proxy_prepare_async(connection, features, prepare_connection_cb, pHandler);
		}
		else
		{
			validate_connection(connection, pHandler);
		}
	}

	g_object_unref(dbus);
}

static void
tube_accept_cb(TpChannel* channel,
				const char* address,
				const GError* error,
				gpointer user_data,
				GObject* /*weak_obj*/)
{
	UT_DEBUGMSG(("tube_accept_cb() - address: %s\n", address));
	UT_return_if_fail(!error);

	TelepathyAccountHandler* pHandler = reinterpret_cast<TelepathyAccountHandler*>(user_data);
	UT_return_if_fail(pHandler);

	pHandler->acceptTube(channel, address);
}

static void
handle_dbus_channel(TpSimpleHandler* /*handler*/,
	TpAccount* /*account*/,
	TpConnection* /*connection*/,
	GList* channels,
	GList* /*requests*/,
	gint64 /*user_action_time*/,
	TpHandleChannelsContext* context,
	gpointer user_data)
{
	UT_DEBUGMSG(("handle_dbus_channel()\n"));

	TelepathyAccountHandler* pHandler = reinterpret_cast<TelepathyAccountHandler*>(user_data);
	UT_return_if_fail(pHandler);

	for (GList* chan = channels; chan; chan = chan->next)
	{
		TpChannel* channel = TP_CHANNEL(chan->data);
		UT_continue_if_fail(channel);
		UT_DEBUGMSG((">>>>> incoming dbus channel: %s\n", tp_channel_get_identifier(channel)));

		if (tp_channel_get_channel_type_id(channel) != TP_IFACE_QUARK_CHANNEL_TYPE_DBUS_TUBE)
			continue;

		/* accept the channel */
		tp_cli_channel_type_dbus_tube_call_accept(channel, -1,
					TP_SOCKET_ACCESS_CONTROL_LOCALHOST,
					tube_accept_cb, user_data, NULL, NULL);
	}

	tp_handle_channels_context_accept(context);
}

void muc_channel_ready_cb(GObject* source_object, GAsyncResult* result, gpointer user_data)
{
	UT_DEBUGMSG(("muc_channel_ready_cb()\n"));

	TelepathyChatroom* pChatroom = reinterpret_cast<TelepathyChatroom*>(user_data);
	UT_return_if_fail(pChatroom);

	TelepathyAccountHandler* pHandler = pChatroom->getHandler();
	UT_return_if_fail(pHandler);

	GError* error = NULL;
	TpChannel * channel = tp_account_channel_request_create_and_handle_channel_finish(
			TP_ACCOUNT_CHANNEL_REQUEST(source_object), result, NULL, &error);
	if (!channel)
	{
		UT_DEBUGMSG(("Error creating MUC channel: %s\n", error->message));
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return;
	}

	// store the channel for safe-keeping
	pChatroom->setChannel(channel);

	// offer the tube to the members we want to invite into the room
	// TODO: drop this call when TP_PROP_CHANNEL_INTERFACE_CONFERENCE_INITIAL_INVITEE_IDS is usable
	pChatroom->offerTube();
}

TelepathyAccountHandler::TelepathyAccountHandler()
	: AccountHandler(),
	table(NULL),
	conference_entry(NULL),
	autoconnect_button(NULL),
	m_pTpClient(NULL)
{
	UT_DEBUGMSG(("TelepathyAccountHandler::TelepathyAccountHandler()\n"));

	if (!hasProperty("conference_server"))
		addProperty("conference_server", DEFAULT_CONFERENCE_SERVER);
}

TelepathyAccountHandler::~TelepathyAccountHandler()
{
	if (isOnline())
		disconnect();
}

UT_UTF8String TelepathyAccountHandler::getDescription()
{
	return "Telepathy";
}

UT_UTF8String TelepathyAccountHandler::getDisplayType()
{
	return "Telepathy";
}

UT_UTF8String TelepathyAccountHandler::getStorageType()
{
	return "com.abisource.abiword.abicollab.backend.telepathy";
}

void TelepathyAccountHandler::embedDialogWidgets(void* pEmbeddingParent)
{
	UT_DEBUGMSG(("TelepathyAccountHandler::embedDialogWidgets()\n"));
	UT_return_if_fail(pEmbeddingParent);

	table = gtk_table_new(2, 2, FALSE);
	GtkVBox* parent = (GtkVBox*)pEmbeddingParent;

	// Jabber conference server
	GtkWidget* conference_label
          = gtk_widget_new(GTK_TYPE_LABEL,
                           "label", "Jabber conference server:",
                           "xalign", 0.0, "yalign", 0.5,
                           NULL);
	gtk_table_attach_defaults(GTK_TABLE(table), conference_label, 0, 1, 0, 1);
	conference_entry = gtk_entry_new();
	gtk_table_attach_defaults(GTK_TABLE(table), conference_entry, 1, 2, 0, 1);
	gtk_entry_set_activates_default(GTK_ENTRY(conference_entry), true);

	// autoconnect
	autoconnect_button = gtk_check_button_new_with_label ("Connect on application startup");
	gtk_table_attach_defaults(GTK_TABLE(table), autoconnect_button, 0, 2, 1, 2);

	gtk_box_pack_start(GTK_BOX(parent), table, FALSE, TRUE, 0);
	gtk_widget_show_all(GTK_WIDGET(parent));
}

void TelepathyAccountHandler::removeDialogWidgets(void* pEmbeddingParent)
{
	UT_DEBUGMSG(("TelepathyAccountHandler::removeDialogWidgets()\n"));
	UT_return_if_fail(pEmbeddingParent);

	// this will conveniently destroy all contained widgets as well
	if (table && GTK_IS_WIDGET(table))
		gtk_widget_destroy(table);
}

void TelepathyAccountHandler::loadProperties()
{
	UT_DEBUGMSG(("TelepathyAccountHandler::loadProperties()\n"));

	std::string conference_server = getProperty("conference_server");
	if (conference_entry && GTK_IS_ENTRY(conference_entry))
		gtk_entry_set_text(GTK_ENTRY(conference_entry), conference_server.c_str());

	bool autoconnect = hasProperty("autoconnect") ? getProperty("autoconnect") == "true" : true;
	if (autoconnect_button && GTK_IS_TOGGLE_BUTTON(autoconnect_button))
		gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(autoconnect_button), autoconnect);
}

void TelepathyAccountHandler::storeProperties()
{
	UT_DEBUGMSG(("TelepathyAccountHandler::storeProperties()\n"));

	if (conference_entry && GTK_IS_ENTRY(conference_entry))
		addProperty("conference_server", gtk_entry_get_text(GTK_ENTRY(conference_entry)));

	if (autoconnect_button && GTK_IS_TOGGLE_BUTTON(autoconnect_button))
		addProperty("autoconnect", gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(autoconnect_button)) ? "true" : "false" );
}

ConnectResult TelepathyAccountHandler::connect()
{
	UT_DEBUGMSG(("TelepathyAccountHandler::connect()\n"));

	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_val_if_fail(pManager, CONNECT_FAILED);

	UT_return_val_if_fail(m_pTpClient == NULL, CONNECT_INTERNAL_ERROR);

	// inform telepathy that we can handle incoming AbiCollab tubes

	GError *error = NULL;
	TpDBusDaemon* dbus = tp_dbus_daemon_dup (&error);
	UT_return_val_if_fail(dbus, CONNECT_FAILED);

	m_pTpClient = tp_simple_handler_new(dbus,
					TRUE, FALSE, "AbiCollab", FALSE,
					handle_dbus_channel, this, NULL);

	tp_base_client_take_handler_filter(m_pTpClient,
					tp_asv_new (
						TP_PROP_CHANNEL_CHANNEL_TYPE, G_TYPE_STRING, TP_IFACE_CHANNEL_TYPE_DBUS_TUBE,
						TP_PROP_CHANNEL_TARGET_HANDLE_TYPE, G_TYPE_UINT, TP_HANDLE_TYPE_ROOM,
						TP_PROP_CHANNEL_TYPE_DBUS_TUBE_SERVICE_NAME, G_TYPE_STRING, INTERFACE,
						NULL
					)
				);

	if (!tp_base_client_register(m_pTpClient, &error))
	{
		UT_DEBUGMSG(("Error registering tube handler: %s", error->message));
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	}

	UT_DEBUGMSG(("Tube handler setup, listening for incoming tubes...\n"));

	// we are connected now, time to start sending out messages (such as events)
	pManager->registerEventListener(this);
	// signal all listeners we are logged in
	AccountOnlineEvent event;
	pManager->signal(event);

	return CONNECT_SUCCESS;
}

bool TelepathyAccountHandler::disconnect()
{
	UT_DEBUGMSG(("TelepathyAccountHandler::disconnect()\n"));
	UT_return_val_if_fail(m_pTpClient, false);

	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_val_if_fail(pManager, false);

	// unregister as a telepathy client
	tp_base_client_unregister(m_pTpClient);
	m_pTpClient = NULL;

	// tear down all active rooms
	for (std::vector<TelepathyChatroomPtr>::iterator it = m_chatrooms.begin(); it != m_chatrooms.end(); it++)
		(*it)->stop();

	// we are disconnected now, no need to receive events anymore
	pManager->unregisterEventListener(this);

	// signal all listeners we are logged out
	AccountOfflineEvent event;
	AbiCollabSessionManager::getManager()->signal(event);

	return true;
}

bool TelepathyAccountHandler::isOnline()
{
	return m_pTpClient != NULL;
}

void TelepathyAccountHandler::getBuddiesAsync()
{
	UT_DEBUGMSG(("TelepathyAccountHandler::getBuddiesAsync()\n"));

	// ask telepathy for the connection names
	TpDBusDaemon* dbus = tp_dbus_daemon_dup(NULL);
	UT_return_if_fail(dbus);
	tp_list_connection_names(dbus, list_connection_names_cb, this, NULL, NULL);
	g_object_unref(dbus);
}

BuddyPtr TelepathyAccountHandler::constructBuddy(const PropertyMap& /*props*/)
{
	UT_DEBUGMSG(("TelepathyAccountHandler::constructBuddy()\n"));
	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
	return BuddyPtr();
}

BuddyPtr TelepathyAccountHandler::constructBuddy(const std::string& /*descriptor*/, BuddyPtr /*pBuddy*/)
{
	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
	return BuddyPtr();
}

bool TelepathyAccountHandler::recognizeBuddyIdentifier(const std::string& /*identifier*/)
{
	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
	return false;
}

void TelepathyAccountHandler::forceDisconnectBuddy(BuddyPtr pBuddy)
{
	UT_return_if_fail(pBuddy);

	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
}

bool TelepathyAccountHandler::hasAccess(const std::vector<std::string>& /*vAcl*/, BuddyPtr pBuddy)
{
	UT_DEBUGMSG(("TelepathyAccountHandler::hasAccess()\n"));
	UT_return_val_if_fail(pBuddy, false);

	// Unfortunately, MUC rooms on conference.telepathy.im don't expose the real JIDs
	// at the moment. This means that without it, we can't do proper security in ::hasAccess()
	// See https://bugs.freedesktop.org/show_bug.cgi?id=37631 for details.
	// Enable the code below when the global contact is accessible; see add_buddy_to_room()
	// for details.

	return true;

	/*
	DTubeBuddyPtr pDTubeBuddy = boost::static_pointer_cast<DTubeBuddy>(pBuddy);
	TpContact* pGlobalContact = pDTubeBuddy->getGlobalContact();
	UT_return_val_if_fail(pGlobalContact, false);

	const gchar* global_ident = tp_contact_get_identifier(pGlobalContact);
	UT_return_val_if_fail(global_ident, false);
	for (std::vector<std::string>::const_iterator cit = vAcl.begin(); cit != vAcl.end(); cit++)
	{
		UT_DEBUGMSG(("%s vs %s\n", global_ident, (*cit).c_str()));
		if (global_ident == (*cit))
			return true;
	}
	*/

	return false;
}

void TelepathyAccountHandler::addContact(TpContact* contact)
{
	UT_DEBUGMSG(("TelepathyAccountHandler::addContact()\n"));
	UT_return_if_fail(contact);

	TelepathyBuddyPtr pBuddy = boost::shared_ptr<TelepathyBuddy>(new TelepathyBuddy(this, contact));
	TelepathyBuddyPtr pExistingBuddy = _getBuddy(pBuddy);
	if (!pExistingBuddy)
		addBuddy(pBuddy);
}

void TelepathyAccountHandler::buddyDisconnected(TelepathyChatroomPtr pChatroom, TpHandle disconnected)
{
	UT_DEBUGMSG(("TelepathyAccountHandler::buddyDisconnected() - handle: %d\n", disconnected));
	UT_return_if_fail(pChatroom);

	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_if_fail(pManager);

	DTubeBuddyPtr pBuddy = pChatroom->getBuddy(disconnected);
	bool isController = pChatroom->isController(pBuddy);

	pManager->removeBuddy(pBuddy, false);
	pChatroom->removeBuddy(disconnected);
	if (isController)
	{
		UT_DEBUGMSG(("The master buddy left; stopping the chatroom!\n"));
		pChatroom->stop();
	}
}

bool TelepathyAccountHandler::startSession(PD_Document* pDoc, const std::vector<std::string>& vAcl, AbiCollab** pSession)
{
	UT_DEBUGMSG(("TelepathyAccountHandler::startSession()\n"));
	UT_return_val_if_fail(pDoc, false);

	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_val_if_fail(pManager, false);

	// generate a unique session id to use
	UT_UTF8String sSessionId;
	UT_UUID* pUUID = XAP_App::getApp()->getUUIDGenerator()->createUUID();
	pUUID->toString(sSessionId);
	DELETEP(pUUID);

	// start the session already, while we'll continue to setup a
	// MUC asynchronously below
	// TODO: we should fill in the in the master buddy descriptor so we can do
	// proper author coloring and session takeover; we can't do that however, since
	// the following bugs needs to be fixed first:
	//
	//   https://bugs.freedesktop.org/show_bug.cgi?id=37631
	*pSession = pManager->startSession(pDoc, sSessionId, this, true, NULL, "");

	// create a chatroom to hold the session information
	TelepathyChatroomPtr pChatroom = boost::shared_ptr<TelepathyChatroom>(new TelepathyChatroom(this, NULL, pDoc, sSessionId));
	m_chatrooms.push_back(pChatroom);

	// add the buddies in the acl list to the room invitee list
	/*
	std::vector<TelepathyBuddyPtr> buddies = _getBuddies(vAcl);
	gchar** invitee_ids = reinterpret_cast<gchar**>(malloc(sizeof(gchar*) * vAcl.size()+1));
	int i = 0;
	for (std::vector<TelepathyBuddyPtr>::iterator it = buddies.begin(); it != buddies.end(); it++)
	{
		UT_continue_if_fail(*it);
		invitee_ids[i] = strdup(tp_contact_get_identifier((*it)->getContact()));
		UT_DEBUGMSG(("Added %s to the invite list\n", invitee_ids[i]));
	}
	invitee_ids[vAcl.size()] = NULL;
	*/
	_inviteBuddies(pChatroom, vAcl); // use the above code when TP_PROP_CHANNEL_INTERFACE_CONFERENCE_INITIAL_INVITEE_IDS is usable

	// a quick hack to determine the account to offer the request on
	TpAccountManager* manager = tp_account_manager_dup();
	UT_return_val_if_fail(manager, false);

	GList* accounts = tp_account_manager_get_valid_accounts(manager);
	UT_return_val_if_fail(accounts, false);

	// TODO: make sure the accounts are ready
	TpAccount* selected_account = NULL;
	for (GList* account = accounts; account; account = account->next)
	{
		selected_account = TP_ACCOUNT(account->data);
		break;
	}
	UT_return_val_if_fail(selected_account, false);
	g_list_free(accounts);

	// determine the room target id
	std::string target_id = sSessionId.utf8_str();
	std::string conference_server = getProperty("conference_server");
	if (conference_server != "")
		target_id += "@" + conference_server;
	UT_DEBUGMSG(("Using room target ID: %s\n", target_id.c_str()));

	// create a anonymous MUC channel request
	GHashTable* props = tp_asv_new (
			TP_PROP_CHANNEL_CHANNEL_TYPE, G_TYPE_STRING, TP_IFACE_CHANNEL_TYPE_DBUS_TUBE,
			TP_PROP_CHANNEL_TARGET_HANDLE_TYPE, TP_TYPE_HANDLE, TP_HANDLE_TYPE_ROOM,
			TP_PROP_CHANNEL_TARGET_ID, G_TYPE_STRING, target_id.c_str(),
			TP_PROP_CHANNEL_TYPE_DBUS_TUBE_SERVICE_NAME, G_TYPE_STRING, INTERFACE,
			/*
			 * Enable TP_PROP_CHANNEL_INTERFACE_CONFERENCE_INITIAL_INVITEE_IDS if you want to use
			 * anonymous MUCs. We can't use it right now, anonymous DBUS_TUBE MUCs are not implemented yet.
			 * Remove the HANDLE_TYPE and TARGET_ID when you enable this.
			 *
			 * See https://bugs.freedesktop.org/show_bug.cgi?id=37630 for details.
			 *
			 * TP_PROP_CHANNEL_INTERFACE_CONFERENCE_INITIAL_INVITEE_IDS, G_TYPE_STRV, invitee_ids,
			 */
			NULL);

	TpAccountChannelRequest * channel_request = tp_account_channel_request_new(selected_account, props, TP_USER_ACTION_TIME_NOT_USER_ACTION);
	UT_return_val_if_fail(channel_request, false);
	g_hash_table_destroy (props);
	// TODO: free invitee_ids

	tp_account_channel_request_create_and_handle_channel_async(channel_request, NULL, muc_channel_ready_cb, pChatroom.get());

	return true;
}

bool TelepathyAccountHandler::setAcl(AbiCollab* pSession, const std::vector<std::string>& vAcl)
{
	UT_DEBUGMSG(("TelepathyAccountHandler::setAcl()\n"));

	// NOTE: having more than 2 persons in a room won't work yet
	// due to this bug: https://bugs.freedesktop.org/show_bug.cgi?id=37729

	TelepathyChatroomPtr pChatroom = _getChatroom(pSession->getSessionId());
	UT_return_val_if_fail(pChatroom, false);

	// we can invite all buddies on the acl; the chatroom
	// will make sure people are not invited twice
	_inviteBuddies(pChatroom, vAcl);

	// offer a tube to the newly added buddies
	if (pChatroom->running())
		pChatroom->offerTube();

	return true;
}

void TelepathyAccountHandler::unregisterChatroom(TelepathyChatroomPtr pChatroom)
{
	UT_DEBUGMSG(("TelepathyAccountHandler::unregisterChatroom()\n"));
	std::vector<TelepathyChatroomPtr>::iterator pos = std::find(m_chatrooms.begin(), m_chatrooms.end(), pChatroom);
	UT_return_if_fail(pos != m_chatrooms.end());
	m_chatrooms.erase(pos);
}

void TelepathyAccountHandler::signal(const Event& event, BuddyPtr pSource)
{
	UT_DEBUGMSG(("TelepathyAccountHandler::signal\n"));

	// NOTE: do NOT let AccountHandler::signal() send broadcast packets!
	// It will send them to all buddies, including the ones we created
	// to list the available documents: TelepathyBuddies. They are just fake
	// buddies however, and can't receive real packets. Only DTubeBuddy's
	// can be sent packets

	// Note: there is no real need to pass the PCT_CloseSessionEvent and
	// PCT_DisjoinSessionEvent signals to the AccountHandler::signal()
	// function: that one will send all buddies the 'session is closed'
	// signal. However, on this backend, Telepathy will handle that for us

	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_if_fail(pManager);

	switch (event.getClassType())
	{
		case PCT_CloseSessionEvent:
			{
				UT_DEBUGMSG(("Got a PCT_CloseSessionEvent\n"));
				const CloseSessionEvent cse = static_cast<const CloseSessionEvent&>(event);
				// check if this event came from this account in the first place
				if (pSource && pSource->getHandler() != this)
				{
					// nope, a session was closed on some other account; ignore this...
					return;
				}
				UT_return_if_fail(!pSource); // we shouldn't receive these events over the wire on this backend

				UT_DEBUGMSG(("Disconnecting the tube for room with session id %s\n", cse.getSessionId().utf8_str()));
				TelepathyChatroomPtr pChatroom = _getChatroom(cse.getSessionId());
				UT_return_if_fail(pChatroom);

				pChatroom->stop();
			}
			break;
		case PCT_DisjoinSessionEvent:
			{
				UT_DEBUGMSG(("Got a PCT_DisjoinSessionEvent\n"));
				const DisjoinSessionEvent dse = static_cast<const DisjoinSessionEvent&>(event);
				// check if this event came from this account in the first place
				if (pSource && pSource->getHandler() != this)
				{
					// nope, a session was closed on some other account; ignore this...
					return;
				}
				UT_return_if_fail(!pSource); // we shouldn't receive these events over the wire on this backend

				UT_DEBUGMSG(("Disconnecting the tube for room with session id %s\n", dse.getSessionId().utf8_str()));
				TelepathyChatroomPtr pChatroom = _getChatroom(dse.getSessionId());
				UT_return_if_fail(pChatroom);

				pChatroom->stop();
			}
			break;
		default:
			// I think we can ignore all other signals on this backend, at
			// least for now
			break;
	}
}

bool TelepathyAccountHandler::send(const Packet* pPacket)
{
	UT_DEBUGMSG(("TelepathyAccountHandler::send(const Packet* pPacket)\n"));
	UT_return_val_if_fail(pPacket, false);
	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
	return true;
}

bool TelepathyAccountHandler::send(const Packet* pPacket, BuddyPtr pBuddy)
{
	UT_DEBUGMSG(("TelepathyAccountHandler::send(const Packet* pPacket, BuddyPtr pBuddy\n"));
	UT_return_val_if_fail(pPacket, false);
	UT_return_val_if_fail(pBuddy, false);

	DTubeBuddyPtr pDTubeBuddy = boost::static_pointer_cast<DTubeBuddy>(pBuddy);
	UT_DEBUGMSG(("Sending packet to d-tube buddy on dbus addess: %s\n", pDTubeBuddy->getDBusName().utf8_str()));

	DBusMessage* pMessage = dbus_message_new_method_call (pDTubeBuddy->getDBusName().utf8_str(), "/org/laptop/DTube/Presence/Buddies", INTERFACE, SEND_ONE_METHOD);
	UT_return_val_if_fail(pMessage, false);

	bool dst = dbus_message_set_destination(pMessage, pDTubeBuddy->getDBusName().utf8_str());
	UT_return_val_if_fail(dst, false);
	UT_DEBUGMSG(("Destination (%s) set on message\n", pDTubeBuddy->getDBusName().utf8_str()));

	// we don't want replies, because they easily run into dbus timeout problems
	// when sending large packets; this means we should probably use signals though.
	dbus_message_set_no_reply(pMessage, TRUE);

	// make to-be-send-stream once
	std::string data;
	_createPacketStream( data, pPacket );

	const char* packet_contents = &data[0];
	dbus_message_append_args(pMessage,
					DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &packet_contents, data.size(),
					DBUS_TYPE_INVALID);
	UT_DEBUGMSG(("Appended packet contents\n"));

	bool sent = dbus_connection_send(pDTubeBuddy->getChatRoom()->getTube(), pMessage, NULL);
	UT_ASSERT_HARMLESS(sent);
	if (sent)
		dbus_connection_flush(pDTubeBuddy->getChatRoom()->getTube());
	dbus_message_unref(pMessage);
	return sent;
}

void TelepathyAccountHandler::acceptTube(TpChannel *chan, const char* address)
{
	UT_DEBUGMSG(("TelepathyAccountHandler::acceptTube() - address: %s\n", address));
	UT_return_if_fail(chan);
	UT_return_if_fail(address);

	// create a new room and make it setup the tube
	TelepathyChatroomPtr pChatroom = boost::shared_ptr<TelepathyChatroom>(new TelepathyChatroom(this, chan, NULL, ""));
	m_chatrooms.push_back(pChatroom);

	pChatroom->acceptTube(address);
}

void TelepathyAccountHandler::handleMessage(DTubeBuddyPtr pBuddy, const std::string& packet_str)
{
	UT_DEBUGMSG(("TelepathyAccountHandler::handleMessage()\n"));
	UT_return_if_fail(pBuddy);

	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_if_fail(pManager);

	TelepathyChatroomPtr pChatroom = pBuddy->getChatRoom();
	UT_return_if_fail(pChatroom);

	// construct the packet
	Packet* pPacket = _createPacket(packet_str, pBuddy);
	UT_return_if_fail(pPacket);

	switch (pPacket->getClassType())
	{
		case PCT_GetSessionsEvent:
		{
			if (pChatroom->isLocallyControlled())
			{
				// return only the session that belongs to the chatroom that the buddy is in
				GetSessionsResponseEvent gsre;
				gsre.m_Sessions[pChatroom->getSessionId()] = pChatroom->getDocName();
				send(&gsre, pBuddy);
			}
			else
				UT_DEBUGMSG(("Ignoring GetSessionsEvent, we are not controlling session '%s'\n", pChatroom->getSessionId().utf8_str()));

			break;
		}
		case PCT_GetSessionsResponseEvent:
		{
			// check if we received 1 (and only 1) session, and join it
			// immediately if that is the case

			GetSessionsResponseEvent* gsre = static_cast<GetSessionsResponseEvent*>( pPacket );
			UT_return_if_fail(gsre->m_Sessions.size() == 1);
			std::map<UT_UTF8String,UT_UTF8String>::iterator it=gsre->m_Sessions.begin();
			DocHandle* pDocHandle = new DocHandle((*it).first, (*it).second);

			// store the session id
			pChatroom->setSessionId(pDocHandle->getSessionId());

			// join the session
			UT_DEBUGMSG(("Got a running session (%s - %s), let's join it immediately\n", pDocHandle->getSessionId().utf8_str(), pDocHandle->getName().utf8_str()));
			pManager->joinSessionInitiate(pBuddy, pDocHandle);
			DELETEP(pDocHandle);
			break;
		}
		default:
			// let the default handler handle it
			AccountHandler::handleMessage(pPacket, pBuddy);
			break;
	}
}

std::vector<TelepathyBuddyPtr> TelepathyAccountHandler::_getBuddies(const std::vector<std::string>& vAcl)
{
	std::vector<TelepathyBuddyPtr> result;
	for (std::vector<std::string>::const_iterator cit = vAcl.begin(); cit != vAcl.end(); cit++ /*, i++*/)
	{
		for (std::vector<BuddyPtr>::iterator it = getBuddies().begin(); it != getBuddies().end(); it++)
		{
			TelepathyBuddyPtr pBuddy = boost::static_pointer_cast<TelepathyBuddy>(*it);
			UT_continue_if_fail(pBuddy);
			if  (pBuddy->getDescriptor(false).utf8_str() == (*cit))
			{
				result.push_back(pBuddy);
				break;
			}
		}
	}

	return result;
}

void TelepathyAccountHandler::_inviteBuddies(TelepathyChatroomPtr pChatroom, const std::vector<std::string>& vAcl)
{
	UT_return_if_fail(pChatroom);

	std::vector<TelepathyBuddyPtr> buddies = _getBuddies(vAcl);

	// add the buddies in the acl list to the room invitee list
	for (std::vector<TelepathyBuddyPtr>::iterator it = buddies.begin(); it != buddies.end(); it++)
	{
		UT_continue_if_fail(*it);
		pChatroom->queueInvite(*it);
	}
}

TelepathyBuddyPtr TelepathyAccountHandler::_getBuddy(TelepathyBuddyPtr pBuddy)
{
	UT_return_val_if_fail(pBuddy, TelepathyBuddyPtr());
	for (std::vector<BuddyPtr>::iterator it = getBuddies().begin(); it != getBuddies().end(); it++)
	{
		TelepathyBuddyPtr pB = boost::static_pointer_cast<TelepathyBuddy>(*it);
		UT_continue_if_fail(pB);
		if (pBuddy->equals(pB))
			return pB;
	}
	return TelepathyBuddyPtr();
}

TelepathyChatroomPtr TelepathyAccountHandler::_getChatroom(const UT_UTF8String& sSessionId)
{
	for (std::vector<TelepathyChatroomPtr>::iterator it = m_chatrooms.begin(); it != m_chatrooms.end(); it++)
	{
		TelepathyChatroomPtr pChatroom = *it;
		UT_continue_if_fail(pChatroom);

		if (pChatroom->getSessionId() == sSessionId)
			return pChatroom;
	}

	return TelepathyChatroomPtr();
}
