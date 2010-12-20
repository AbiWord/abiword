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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
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

static DBusHandlerResult s_dbus_handle_message(DBusConnection *connection, DBusMessage *message, void *user_data);

#define INTERFACE "com.abisource.abiword.abicollab.telepathy"
#define SEND_ONE_METHOD "SendOne"

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

	// we could alternatively use tp_connection_dup_contact_if_possible(), but
	// tp_connection_get_contacts_by_handle seems more generic
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
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN)
		return;
	}

	TpDBusDaemon* dbus = tp_dbus_daemon_dup(NULL);
	UT_return_if_fail(dbus);

	UT_DEBUGMSG(("Got %d connections:\n", (int)n));

	for (UT_uint32 i = 0; i < n; i++)
	{
		UT_DEBUGMSG(("%d: Bus name %s, connection manager %s, protocol %s\n", i+1, bus_names[i], cms[i], protocols[i]));
		TpConnection* connection = tp_connection_new (dbus, bus_names[i], NULL, NULL);

		// TODO: make this async
		tp_connection_run_until_ready(connection, true, NULL, NULL);

		TpCapabilities* caps = tp_connection_get_capabilities(reinterpret_cast<TpConnection*>(connection));
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
tube_dbus_names_changed_cb(TpChannel* /*proxy*/,
							GHashTable* arg_Added,
							const GArray* arg_Removed,
							gpointer user_data,
							GObject* /*weak_object*/)
{
	UT_DEBUGMSG(("tube_dbus_names_changed_cb()\n"));
	UT_return_if_fail(arg_Added);
	UT_return_if_fail(arg_Removed);

	TelepathyChatroom* pChatroom = reinterpret_cast<TelepathyChatroom*>(user_data);
	UT_return_if_fail(pChatroom);

	TelepathyAccountHandler* pHandler = pChatroom->getHandler();
	UT_return_if_fail(pHandler);

	gpointer key;
	gpointer value;
	GHashTableIter iter;
	g_hash_table_iter_init(&iter, arg_Added);
	while (g_hash_table_iter_next(&iter, &key, &value))
	{
		TpHandle handle = GPOINTER_TO_UINT(key);
		const char* dbus_name = reinterpret_cast<const gchar*>(value);

		UT_DEBUGMSG(("Adding a new buddy: %d - %s\n", handle, dbus_name));
		DTubeBuddyPtr pBuddy = boost::shared_ptr<DTubeBuddy>(new DTubeBuddy(pHandler, pChatroom->ptr(), handle, dbus_name));
		pChatroom->addBuddy(pBuddy);
	}

	for (UT_uint32 i = 0; i < arg_Removed->len; i++)
	{
		TpHandle removed = g_array_index(arg_Removed, TpHandle, i);
		UT_DEBUGMSG(("Buddy with handle %d left\n", removed));

		pHandler->buddyDisconnected(pChatroom->ptr(), removed);
	}
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

	// offer the tube to the members we want to invite into the room
	pHandler->offerTube(pChatroom->ptr(), channel, pChatroom->getSessionId());
}

TelepathyAccountHandler::TelepathyAccountHandler()
	: AccountHandler()
{
	UT_DEBUGMSG(("TelepathyAccountHandler::TelepathyAccountHandler()\n"));
}

TelepathyAccountHandler::~TelepathyAccountHandler()
{
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

void TelepathyAccountHandler::storeProperties()
{
	// no need to implement this as we will be getting
	// all our info always directly from sugar
}

ConnectResult TelepathyAccountHandler::connect()
{
	UT_DEBUGMSG(("TelepathyAccountHandler::connect()\n"));

	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_val_if_fail(pManager, CONNECT_FAILED);

	// inform telepathy that we can handle incoming tubes on the
	// com.abisource.com.abiword.abicollab.telepathy service

	GError *error = NULL;
	TpDBusDaemon* dbus = tp_dbus_daemon_dup (&error);
	UT_return_val_if_fail(dbus, CONNECT_FAILED);

	TpBaseClient* handler = tp_simple_handler_new(dbus,
					TRUE, FALSE, "AbiCollabHandler", TRUE,
					handle_dbus_channel, this, NULL);

	tp_base_client_take_handler_filter(handler,
					tp_asv_new (
						TP_PROP_CHANNEL_CHANNEL_TYPE, G_TYPE_STRING, TP_IFACE_CHANNEL_TYPE_DBUS_TUBE,
						TP_PROP_CHANNEL_TARGET_HANDLE_TYPE, G_TYPE_UINT, TP_HANDLE_TYPE_ROOM,
						TP_PROP_CHANNEL_TYPE_DBUS_TUBE_SERVICE_NAME, G_TYPE_STRING, INTERFACE,
						NULL
					)
				);

	if (!tp_base_client_register(handler, &error))
	{
		UT_DEBUGMSG(("Error registering tube handler: %s", error->message));
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	}

	UT_DEBUGMSG(("Tube handler setup, listening for incoming tubes...\n"));

	// we are connected now, time to start sending out messages (such as events)
	pManager->registerEventListener(this);
	// signal all listeners we are logged in
	AccountOnlineEvent event;
	// TODO: fill the event
	pManager->signal(event);

	return CONNECT_SUCCESS;
}

bool TelepathyAccountHandler::disconnect()
{
	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_val_if_fail(pManager, false);

	// we are disconnected now, no need to receive events anymore
	pManager->unregisterEventListener(this);

	// signal all listeners we are logged out
	AccountOfflineEvent event;
	// TODO: fill the event
	AbiCollabSessionManager::getManager()->signal(event);

	return true;
}

bool TelepathyAccountHandler::isOnline()
{
	return true;
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

	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED)
}

bool TelepathyAccountHandler::hasAccess(const std::vector<std::string>& /*vAcl*/, BuddyPtr pBuddy)
{
	UT_DEBUGMSG(("TelepathyAccountHandler::hasAccess()\n"));
	UT_return_val_if_fail(pBuddy, false);

	// TODO: implement me

	return TRUE;
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

	pManager->removeBuddy(pBuddy, false);
	pChatroom->removeBuddy(disconnected);
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
	// TODO: fill in the buddy descriptor for proper text coloring?
	*pSession = pManager->startSession(pDoc, sSessionId, this, true, NULL, "");

	// create a chatroom to hold the session information
	TelepathyChatroomPtr pChatroom = boost::shared_ptr<TelepathyChatroom>(new TelepathyChatroom(this, NULL, sSessionId));
	m_chatrooms.push_back(pChatroom);

	// add the buddies in the acl list to the room invitee list
	// NOTE: this n^2 behavior shouldn't be too bad in practice: the ACL will never contain hundreds of elements
	for (std::vector<std::string>::const_iterator cit = vAcl.begin(); cit != vAcl.end(); cit++)
	{
		for (std::vector<BuddyPtr>::iterator it = getBuddies().begin(); it != getBuddies().end(); it++)
		{
			TelepathyBuddyPtr pBuddy = boost::static_pointer_cast<TelepathyBuddy>(*it);
			UT_continue_if_fail(pBuddy);
			if  (pBuddy->getDescriptor(false).utf8_str() == (*cit))
			{
				pChatroom->invite(pBuddy);
				break;
			}
		}
	}

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

	// create a MUC channel request
	GHashTable* props = tp_asv_new (
			TP_PROP_CHANNEL_CHANNEL_TYPE, G_TYPE_STRING, TP_IFACE_CHANNEL_TYPE_DBUS_TUBE,
			TP_PROP_CHANNEL_TARGET_HANDLE_TYPE, TP_TYPE_HANDLE, TP_HANDLE_TYPE_ROOM,
			TP_PROP_CHANNEL_TARGET_ID, G_TYPE_STRING, "abicollab4@conference.matthewwild.co.uk", /*target_id.utf8_str()*/
			TP_PROP_CHANNEL_TYPE_DBUS_TUBE_SERVICE_NAME, G_TYPE_STRING, INTERFACE,
			/*
			 * Enable TP_PROP_CHANNEL_INTERFACE_CONFERENCE_INITIAL_INVITEE_IDS if you want to use
			 * anonymous MUCs. We can't use it right now, because we run into bugs.
			 * Remove the HANDLE_TYPE and TARGET_ID when you enable this.
			 *
			 * TP_PROP_CHANNEL_INTERFACE_CONFERENCE_INITIAL_INVITEE_IDS, G_TYPE_STRV, invitee_ids,
			 */
			NULL);

	TpAccountChannelRequest * channel_request = tp_account_channel_request_new(selected_account, props, TP_USER_ACTION_TIME_NOT_USER_ACTION);
	UT_return_val_if_fail(channel_request, false);
	g_hash_table_destroy (props);

	tp_account_channel_request_create_and_handle_channel_async(channel_request, NULL, muc_channel_ready_cb, pChatroom.get());

	return true;
}

void TelepathyAccountHandler::signal(const Event& /*event*/, BuddyPtr /*pSource*/)
{
	UT_DEBUGMSG(("TelepathyAccountHandler::signal\n"));

	// NOTE: do NOT let AccountHandler::signal() send broadcast packets!
	// It will send them to all buddies, including the ones we created
	// to list the available documents: TelepathyBuddies. They are just fake
	// buddies however, and can't receive real packets. Only DTubeBuddy's
	// can be sent packets

	// TODO: handle the other events?
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

	// TODO: check dst
	dbus_message_set_destination(pMessage, pDTubeBuddy->getDBusName().utf8_str());
	UT_DEBUGMSG(("Destination (%s) set on message\n", pDTubeBuddy->getDBusName().utf8_str()));

	// we don't want replies, because they easily run into dbus timeout problems 
	// when sending large packets
	// TODO: this means we should probably use signals though
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

	DBusError dbus_error;
	dbus_error_init(&dbus_error);
	DBusConnection* pTube = dbus_connection_open(address, &dbus_error);
	if (!pTube)
	{
		UT_DEBUGMSG(("Error opening dbus connection to address %s: %s\n", address, dbus_error.message));
		dbus_error_free (&dbus_error);
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return;
	}

	// create a new room so we can store the buddies somewhere
	// the session id will be set as soon as we join the document
	TelepathyChatroomPtr pChatroom = boost::shared_ptr<TelepathyChatroom>(new TelepathyChatroom(this, pTube, ""));
	m_chatrooms.push_back(pChatroom);

	UT_DEBUGMSG(("Adding dbus handlers to the main loop for tube %s\n", address));
	dbus_connection_setup_with_g_main(pTube, NULL);

	UT_DEBUGMSG(("Adding message filter\n"));
	dbus_connection_add_filter(pTube, s_dbus_handle_message, pChatroom.get(), NULL);

	// start listening on the tube for people entering and leaving it
	GError* error = NULL;
	TpProxySignalConnection* signal = tp_cli_channel_type_dbus_tube_connect_to_dbus_names_changed(
															chan, tube_dbus_names_changed_cb,
															pChatroom.get(), NULL, NULL, &error);
	if (!signal)
	{
		UT_DEBUGMSG(("Error connecting to names_changes: %s\n", error ? error->message : "(null)"));
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return;
	}

	// retrieve who created this room
	TpHandle initiator_handle = tp_channel_get_initiator_handle(chan);
	const char* initiator_ident = tp_channel_get_initiator_identifier(chan);
	UT_DEBUGMSG(("Got initiator: %d - %s\n", initiator_handle, initiator_ident));
	UT_DEBUGMSG(("Channel identifier: %s\n", tp_channel_get_identifier(chan)));

	TpHandle self_handle = tp_channel_group_get_self_handle(chan);

	// retrieve the TpHandle <-> dbus address mapping for the people in the room
	GValue* prop = NULL;
	if (!tp_cli_dbus_properties_run_get(chan, -1, TP_IFACE_CHANNEL_TYPE_DBUS_TUBE, "DBusNames", &prop, &error, NULL))
	{
		UT_DEBUGMSG(("Failed to get dbus members: %s\n", error ? error->message : "null"));
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return;
	}

	UT_return_if_fail(G_VALUE_HOLDS(prop, TP_HASH_TYPE_DBUS_TUBE_PARTICIPANTS));
	GHashTable* name_mapping = reinterpret_cast<GHashTable*>(g_value_get_boxed(prop));
	gpointer key;
	gpointer value;
	GHashTableIter iter;
	g_hash_table_iter_init(&iter, name_mapping);
	while (g_hash_table_iter_next(&iter, &key, &value))
	{
		TpHandle contact_handle = GPOINTER_TO_UINT(key);
		const char* contact_address = reinterpret_cast<const gchar*>(value);
		UT_DEBUGMSG(("Got room member - handle: %d, address: %s\n", contact_handle, contact_address));

		// skip ourselves
		if (self_handle == contact_handle)
		{
			UT_DEBUGMSG(("Skipping self handle %d\n", contact_handle));
			continue;
		}

		UT_DEBUGMSG(("Added room member - handle: %d, address: %s\n", contact_handle, contact_address));
		DTubeBuddyPtr pBuddy = boost::shared_ptr<DTubeBuddy>(new DTubeBuddy(this, pChatroom, contact_handle, contact_address));
		pChatroom->addBuddy(pBuddy);
	}

	// send a request for sessions; if everything is alright then we should
	// receive exactly 1 session in all the responses combined
	UT_DEBUGMSG(("Sending a GetSessionsEvent to every participant in the room\n"));
	const std::vector<DTubeBuddyPtr> buddies = pChatroom->getBuddies();
	for (UT_uint32 i = 0; i < buddies.size(); i++)
		getSessionsAsync(buddies[i]);
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
	UT_return_if_fail(pPacket); // TODO: shouldn't we just disconnect here?

	switch (pPacket->getClassType())
	{
		case PCT_GetSessionsEvent:
		{
			if (pChatroom->getSessionId() != "")
			{
				AbiCollab* pSession = pManager->getSessionFromSessionId(pChatroom->getSessionId());
				UT_return_if_fail(pSession);

				if (pSession->isLocallyControlled())
				{
					// return only the session that belongs to the chatroom that the buddy is in
					GetSessionsResponseEvent gsre;
					gsre.m_Sessions[pChatroom->getSessionId()] = "bar"; // TODO: add document name
					send(&gsre, pBuddy);
				}
				else
					UT_DEBUGMSG(("Ignoring GetSessionsEvent, we are not controlling session %s\n", pChatroom->getSessionId().utf8_str()));
			} else
			{
				UT_DEBUGMSG(("Ignoring GetSessionsEvent, we are not controlling session %s (we didn't even join yet)\n", pChatroom->getSessionId().utf8_str()));
			}

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

DBusHandlerResult s_dbus_handle_message(DBusConnection *connection, DBusMessage *message, void *user_data)
{
	UT_DEBUGMSG(("s_dbus_handle_message()\n"));
	UT_return_val_if_fail(connection, DBUS_HANDLER_RESULT_NOT_YET_HANDLED);
	UT_return_val_if_fail(message, DBUS_HANDLER_RESULT_NOT_YET_HANDLED);
	UT_return_val_if_fail(user_data, DBUS_HANDLER_RESULT_NOT_YET_HANDLED);

	TelepathyChatroom* pChatroom = reinterpret_cast<TelepathyChatroom*>(user_data);
	UT_return_val_if_fail(pChatroom, DBUS_HANDLER_RESULT_NOT_YET_HANDLED);

	TelepathyAccountHandler* pHandler = pChatroom->getHandler();
	UT_return_val_if_fail(pHandler, DBUS_HANDLER_RESULT_NOT_YET_HANDLED);

	if (dbus_message_is_method_call(message, INTERFACE, SEND_ONE_METHOD))
	{
		const char* senderDBusAddress = dbus_message_get_sender(message);
		UT_DEBUGMSG(("%s message accepted from %s!\n", SEND_ONE_METHOD, senderDBusAddress));

		DBusError error;
		dbus_error_init (&error);
		const char* packet_data = 0;
		int packet_size = 0;
		if (dbus_message_get_args(message, &error,
					DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &packet_data, &packet_size,
					DBUS_TYPE_INVALID))
		{
			UT_DEBUGMSG(("Received packet from %s\n", senderDBusAddress));
			std::string packet(packet_data, packet_size);

			DTubeBuddyPtr pBuddy = pChatroom->getBuddy(senderDBusAddress);
			if (!pBuddy)
			{
				// Sometimes we already receive messages from people before we
				// leared about them from a "dbus names changes" signal...
				// Therefore we'll queue this message until we know who it
				// came from (ie. until we have a TpHandle)
				pChatroom->queue(senderDBusAddress, packet);
			}
			else
			{
				pHandler->handleMessage(pBuddy, packet);
			}

			return DBUS_HANDLER_RESULT_HANDLED;
		}
		else
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	}	

	UT_DEBUGMSG(("Unhandled message\n"));
	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

bool TelepathyAccountHandler::offerTube(TelepathyChatroomPtr pChatroom, TpChannel* chan,
		const UT_UTF8String& sSessionId)
{
	UT_DEBUGMSG(("TelepathyAccountHandler::offerTube()\n - session id: %s\n", sSessionId.utf8_str()));

	UT_return_val_if_fail(pChatroom, false);
	UT_return_val_if_fail(chan, false);
	UT_return_val_if_fail(sSessionId != "", false);

	const std::vector<TelepathyBuddyPtr>& vBuddies = pChatroom->getInvitees();
	UT_return_val_if_fail(vBuddies.size() > 0, false);

	// add members to the room
	GArray* members = g_array_new (FALSE, FALSE, sizeof(TpHandle));
	for (UT_uint32 i = 0; i < vBuddies.size(); i++)
	{
		TelepathyBuddyPtr pBuddy = vBuddies[i];
		UT_continue_if_fail(pBuddy && pBuddy->getContact());
		TpHandle handle = tp_contact_get_handle(pBuddy->getContact());
		UT_DEBUGMSG(("Adding %s to the invite list\n", tp_contact_get_identifier(pBuddy->getContact())));
		g_array_append_val(members, handle);
	}

	UT_DEBUGMSG(("Inviting members to the room...\n"));
	GError* error = NULL;
	if (!tp_cli_channel_interface_group_run_add_members(chan, -1,  members, "Hi there!", &error, NULL))
	{
		UT_DEBUGMSG(("Error inviting room members: %s\n", error ? error->message : "(null)"));
		return false;
	}
	UT_DEBUGMSG(("Members invited\n"));

	// TODO: hide this explicit clear, it's not nice API wise
	pChatroom->getInvitees().clear();

	GHashTable* params = tp_asv_new (
			"title", G_TYPE_STRING, /*pDoc->getFilename()*/ "TODO: get document title",
			NULL);

	// offer this tube to every participant in the room
	gchar* address = NULL;
	if (!tp_cli_channel_type_dbus_tube_run_offer(chan, -1, params, TP_SOCKET_ACCESS_CONTROL_LOCALHOST, &address, &error, NULL))
	{
		UT_DEBUGMSG(("Error offering tube to room participants: %s\n", error ? error->message : "(null)"));
		return false;
	}
	g_hash_table_destroy (params);

	UT_DEBUGMSG(("Tube offered, address: %s\n", address));

	// open and store the tube dbus connection
	DBusConnection* pTube = dbus_connection_open(address, NULL);
	UT_return_val_if_fail(pTube, FALSE);
	pChatroom->setTube(pTube);

	UT_DEBUGMSG(("Adding dbus handlers to the main loop for tube %s\n", address));
	dbus_connection_setup_with_g_main(pTube, NULL);

	UT_DEBUGMSG(("Adding message filter\n"));
	dbus_connection_add_filter(pTube, s_dbus_handle_message, pChatroom.get(), NULL);

	// start listening on the tube for people entering and leaving it
	TpProxySignalConnection* signal = tp_cli_channel_type_dbus_tube_connect_to_dbus_names_changed(
															chan, tube_dbus_names_changed_cb,
															pChatroom.get(), NULL, NULL, &error);
	if (!signal)
	{
		UT_DEBUGMSG(("Error connecting to names_changes: %s\n", error ? error->message : "(null)"));
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return false;
	}
	
	return true;
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


