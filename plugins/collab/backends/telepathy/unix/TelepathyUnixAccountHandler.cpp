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
#include <ev_EditMethod.h>
#include <xap_App.h>
#include <fv_View.h>
#include <xap_Frame.h>
#include <xap_UnixApp.h>

#include <telepathy-glib/telepathy-glib.h>

#include "TelepathyUnixAccountHandler.h"
#include "DTubeBuddy.h"

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
		tp_connection_run_until_ready(connection, NULL, NULL, NULL);

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
tube_dbus_names_changed_cb (TpChannel * /*proxy*/,
							guint /*id*/,
							const GPtrArray *added,
							const GArray *removed,
							gpointer user_data,
							GObject * /*weak_object*/)
{
	TelepathyAccountHandler *obj = (TelepathyAccountHandler *) user_data;
	guint i;

	UT_DEBUGMSG(("Tube D-Bus names changed\n"));

	for (i = 0; i < added->len; i++)
	{
		GValueArray *v;
		TpHandle handle;
		const gchar *name;

		v = (GValueArray *) g_ptr_array_index (added, i);

		handle = g_value_get_uint (g_value_array_get_nth (v, 0));
		name = g_value_get_string (g_value_array_get_nth (v, 1));

		UT_DEBUGMSG(("... added %s (%d)\n", name, handle));

		UT_UTF8String buddyPath(name, strlen(name));
		PD_Document *pDoc = static_cast<PD_Document*>(XAP_App::getApp()->getLastFocussedFrame()->getCurrentDoc());

		obj->joinBuddy(pDoc, handle, buddyPath);
	}

	for (i = 0; i < removed->len; i++)
	{
		TpHandle handle;

		handle = g_array_index (removed, TpHandle, i);

		UT_DEBUGMSG(("... removed %d\n", handle));

		/* TODO: call buddyLeft */
	}
}

static void
tube_accept_cb(TpChannel* channel,
				const char* address,
				const GError* error,
				gpointer user_data,
				GObject* weak_obj)
{
	UT_DEBUGMSG(("tube_accept_cb() - address: %s\n", address));

	UT_return_if_fail(!error);
	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
}

static void
_handle_dbus_channel(TpSimpleHandler *handler,
	TpAccount* account,
	TpConnection* connection,
	GList* channels,
	GList* requests,
	gint64 user_action_time,
	TpHandleChannelsContext* context,
	gpointer user_data)
{
	UT_DEBUGMSG(("_handle_dbus_channel\n"));

	TelepathyAccountHandler* pHandler = reinterpret_cast<TelepathyAccountHandler*>(user_data);
	UT_return_if_fail(pHandler);

	for (GList* chan = channels; chan; chan = chan->next)
	{
		GError *error = NULL;

		TpChannel* channel = TP_CHANNEL(chan->data);
		UT_continue_if_fail(channel);
		UT_DEBUGMSG((">>>>> incoming dbus channel: %s\n", tp_channel_get_identifier(channel)));

		if (tp_channel_get_channel_type_id(channel) != TP_IFACE_QUARK_CHANNEL_TYPE_DBUS_TUBE)
			continue;

		/*
		// TODO: hook up some callbacks to important signals
		tp_cli_channel_interface_tube_connect_to_tube_channel_state_changed (
					channel, tube_state_changed_cb,
					user_data, NULL, NULL, &error);
		UT_continue_if_fail(!error);

		tp_cli_channel_type_dbus_tube_connect_to_dbus_names_changed (
					channel, dbus_names_changed_cb,
					user_data, NULL, NULL, &error);
		UT_continue_if_fail(!error);
		*/

		/* accept the channel */
		tp_cli_channel_type_dbus_tube_call_accept(channel, -1,
					TP_SOCKET_ACCESS_CONTROL_LOCALHOST,
					tube_accept_cb, user_data, NULL, NULL);
	}

	tp_handle_channels_context_accept(context);
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
					_handle_dbus_channel, this, NULL);

	tp_base_client_take_handler_filter(handler,
					tp_asv_new (
						TP_PROP_CHANNEL_CHANNEL_TYPE, G_TYPE_STRING, TP_IFACE_CHANNEL_TYPE_DBUS_TUBE,
						TP_PROP_CHANNEL_TARGET_HANDLE_TYPE, G_TYPE_UINT, TP_HANDLE_TYPE_ROOM,
						//TP_PROP_CHANNEL_TARGET_ID, G_TYPE_STRING, targetid,
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

void TelepathyAccountHandler::addContact(TpContact* contact)
{
	UT_DEBUGMSG(("TelepathyAccountHandler::addContact()\n"));
	UT_return_if_fail(contact);

	TelepathyBuddyPtr pBuddy = boost::shared_ptr<TelepathyBuddy>(new TelepathyBuddy(this, contact));
	TelepathyBuddyPtr pExistingBuddy = _getBuddy(pBuddy);
	if (!pExistingBuddy)
		addBuddy(pBuddy);
}

bool TelepathyAccountHandler::startSession(PD_Document* pDoc, const std::vector<std::string>& vAcl, AbiCollab** /*pSession*/)
{
	UT_DEBUGMSG(("TelepathyAccountHandler::startSession()\n"));
	UT_return_val_if_fail(pDoc, false);

	std::vector<TelepathyBuddyPtr> acl_;
	// this n^2 behavior shouldn't be too bad in practice: the ACL will never contain hundreds of elements
	for (std::vector<std::string>::const_iterator cit = vAcl.begin(); cit != vAcl.end(); cit++)
	{
		for (std::vector<BuddyPtr>::iterator it = getBuddies().begin(); it != getBuddies().end(); it++)
		{
			TelepathyBuddyPtr pBuddy = boost::static_pointer_cast<TelepathyBuddy>(*it);
			UT_continue_if_fail(pBuddy);
			if  (pBuddy->getDescriptor(false).utf8_str() == (*cit))
			{
				acl_.push_back(pBuddy);
				break;
			}
		}
	}

	UT_UTF8String sTubeAddress; 
	if (!_createAndOfferTube(pDoc, acl_, sTubeAddress))
		return false;

	DBusConnection* pTube = dbus_connection_open(sTubeAddress.utf8_str(), NULL);
	UT_return_val_if_fail(pTube, FALSE);

	UT_DEBUGMSG(("Adding dbus handlers to the main loop for tube %s\n", sTubeAddress.utf8_str()));
	dbus_connection_setup_with_g_main(pTube, NULL);

	UT_DEBUGMSG(("Adding message filter\n"));
	dbus_connection_add_filter(pTube, s_dbus_handle_message, this, NULL);
	
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

	bool sent = dbus_connection_send(pDTubeBuddy->getTube(), pMessage, NULL);
	UT_ASSERT_HARMLESS(sent);
	if (sent)
		dbus_connection_flush(pDTubeBuddy->getTube());
	dbus_message_unref(pMessage);
	return sent;
}

bool TelepathyAccountHandler::joinTube(const UT_UTF8String& tubeDBusAddress)
{
	UT_DEBUGMSG(("TelepathyAccountHandler::joinTube()\n"));

	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_val_if_fail(pManager, false);

	// TODO: check that we aren't already in a session; this backend can only join one session at a time (for now)

	//DBusError error;
	DBusConnection* pTube = dbus_connection_open(tubeDBusAddress.utf8_str(), NULL);
	UT_return_val_if_fail(pTube, false);

	UT_DEBUGMSG(("Adding dbus handlers to the main loop for tube %s\n", tubeDBusAddress.utf8_str()));
	dbus_connection_setup_with_g_main(pTube, NULL);

	UT_DEBUGMSG(("Adding message filter\n"));
	dbus_connection_add_filter(pTube, s_dbus_handle_message, this, NULL);

	return true;
}

bool TelepathyAccountHandler::joinBuddy(PD_Document* /*pDoc*/, TpHandle handle, const UT_UTF8String& buddyDBusAddress)
{
	UT_DEBUGMSG(("TelepathyAccountHandler::joinBuddy()\n"));

	if (g_hash_table_lookup (handle_to_bus_name, GUINT_TO_POINTER (handle)) != NULL)
		return false;
	g_hash_table_insert (handle_to_bus_name, GUINT_TO_POINTER (handle), (void *) &buddyDBusAddress);

	// TODO: implement me properly
    UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
	return false;
}

void TelepathyAccountHandler::acceptTube(TpChannel *chan,
                                     guint id,
                                     TpHandle initiator)
{
	gboolean result;
	gchar *address;
	GPtrArray *names;
	guint i;

	result = tp_cli_channel_type_tubes_run_accept_d_bus_tube (chan, -1, id,
				&address, NULL, NULL);
	g_assert (result);

	UT_UTF8String tubeDBusAddress(address, strlen (address));
	joinTube(tubeDBusAddress);

	tp_cli_channel_type_tubes_connect_to_d_bus_names_changed (chan,
				tube_dbus_names_changed_cb, this, NULL, NULL, NULL);

	/* add initiator */
	result = tp_cli_channel_type_tubes_run_get_d_bus_names (chan, -1,
	id, &names, NULL, NULL);

	for (i = 0; i < names->len; i++)
	{
		GValueArray *v;
		TpHandle handle;
		const gchar *name;

		v = (GValueArray *) g_ptr_array_index (names, i);

		handle = g_value_get_uint (g_value_array_get_nth (v, 0));
		name = g_value_get_string (g_value_array_get_nth (v, 1));

		if (handle != initiator)
			continue;

		UT_DEBUGMSG(("Found initiator %s (%d)\n", name, handle));
		UT_UTF8String buddyPath(name, strlen(name));
		PD_Document *pDoc = static_cast<PD_Document*>(XAP_App::getApp()->getLastFocussedFrame()->getCurrentDoc());

		joinBuddy(pDoc, handle, buddyPath);
		break;
	}
}

void TelepathyAccountHandler::handleMessage(const char* senderDBusAddress, const char* packet_data, int packet_size)
{
	UT_DEBUGMSG(("TelepathyAccountHandler::handleMessage()\n"));

	// get the buddy for this session
	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
	DTubeBuddyPtr pBuddy;// = _getBuddy(senderDBusAddress);
	UT_return_if_fail(pBuddy); // TODO: shouldn't we just disconnect here?

	// construct the packet
	// FIXME: inefficient copying of data
	std::string packet_str(packet_size, ' ');
	memcpy(&packet_str[0], packet_data, packet_size);
	FREEP(packet_data);
	Packet* pPacket = _createPacket(packet_str, pBuddy);
	UT_return_if_fail(pPacket); // TODO: shouldn't we just disconnect here?

	// handle!
	AccountHandler::handleMessage(pPacket, pBuddy);
}

DBusHandlerResult s_dbus_handle_message(DBusConnection *connection, DBusMessage *message, void *user_data)
{
	UT_DEBUGMSG(("s_dbus_handle_message()\n"));
	UT_return_val_if_fail(connection, DBUS_HANDLER_RESULT_NOT_YET_HANDLED);
	UT_return_val_if_fail(message, DBUS_HANDLER_RESULT_NOT_YET_HANDLED);
	UT_return_val_if_fail(user_data, DBUS_HANDLER_RESULT_NOT_YET_HANDLED);
	TelepathyAccountHandler* pHandler = reinterpret_cast<TelepathyAccountHandler*>(user_data);

	if (dbus_message_is_method_call(message, INTERFACE, SEND_ONE_METHOD))
	{
		UT_DEBUGMSG(("%s message accepted!\n", SEND_ONE_METHOD));

		const char* senderDBusAddress = dbus_message_get_sender(message);

		DBusError error;
		dbus_error_init (&error);
		const char* packet_data = 0;
		int packet_size = 0;
	    if (dbus_message_get_args(message, &error, 
					DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &packet_data, &packet_size,
					DBUS_TYPE_INVALID))
		{
			UT_DEBUGMSG(("Received packet from %s\n", senderDBusAddress));
			pHandler->handleMessage(senderDBusAddress, packet_data, packet_size);
			//dbus_free(packet);
			return DBUS_HANDLER_RESULT_HANDLED;
		}
		else
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	}	

	UT_DEBUGMSG(("Unhandled message\n"));
	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}

// TODO: cleanup after errors
bool TelepathyAccountHandler::_createAndOfferTube(PD_Document* pDoc, const std::vector<TelepathyBuddyPtr>& vBuddies, UT_UTF8String& sTubeAddress)
{
	GError* error = NULL;
	gchar* object_path;

	UT_return_val_if_fail(pDoc, false);
	UT_return_val_if_fail(vBuddies.size() > 0, false);
	
	// get some connection belonging to this contact
	// TODO: we probably want to change this to some user selectable thingy
	TpContact* pContact = vBuddies[0]->getContact();
	TpConnection * conn = tp_contact_get_connection (pContact);

	//
	// create a room, so we can invite some members in it
	//

	// generate a unique room ID (no room ID would not be needed for MSN MUCs, we don't support those for now)
	// we leave out the server address and let telepathy auto-discover it, so we don't have
	// to bother our users with it
	UT_UUID* uuid = XAP_App::getApp()->getUUIDGenerator()->createUUID();
	UT_return_val_if_fail(uuid, FALSE);
	UT_UTF8String target_id;
	uuid->toString(target_id);
	DELETEP(uuid);

	/*
	gchar** invitee_ids = reinterpret_cast<gchar**>(malloc(sizeof(gchar*) * vBuddies.size()+1));
	for (UT_uint32 i = 0; i < vBuddies.size(); i++)
		invitee_ids[i] = strdup(tp_contact_get_identifier(vBuddies[i]->getContact()));
	invitee_ids[vBuddies.size()] = NULL;
	*/

	// setup the room properties
	GHashTable* props = tp_asv_new (
			TP_PROP_CHANNEL_CHANNEL_TYPE, G_TYPE_STRING, TP_IFACE_CHANNEL_TYPE_DBUS_TUBE,
			TP_PROP_CHANNEL_TARGET_HANDLE_TYPE, TP_TYPE_HANDLE, TP_HANDLE_TYPE_ROOM,
			TP_PROP_CHANNEL_TARGET_ID, G_TYPE_STRING, target_id.utf8_str(),
			TP_PROP_CHANNEL_TYPE_DBUS_TUBE_SERVICE_NAME, G_TYPE_STRING, INTERFACE,
			/*
			 * Enable TP_PROP_CHANNEL_INTERFACE_CONFERENCE_INITIAL_INVITEE_IDS if you want to use
			 * anonymous MUCs. We can't use it right now, because we run into bugs.
			 * Remove the HANDLE_TYPE and TARGET_ID when you enable this.
			 *
			 * TP_PROP_CHANNEL_INTERFACE_CONFERENCE_INITIAL_INVITEE_IDS, G_TYPE_STRV, invitee_ids,
			 */
			NULL);

	// ... then actually create the room
	GHashTable* channel_properties = NULL;
	if (!tp_cli_connection_interface_requests_run_create_channel (conn, -1, props, &object_path, &channel_properties, &error, NULL))
	{
		UT_DEBUGMSG(("Error creating room: %s\n", error ? error->message : "(null)"));
		g_hash_table_destroy (props);
		return false;
	}
	UT_DEBUGMSG(("Got a room, path: %s\n", object_path));
	g_hash_table_destroy (props);
	
	// get a channel to the new room
	TpChannel* chan = tp_channel_new_from_properties (conn, object_path, channel_properties, NULL);
	UT_return_val_if_fail(chan, FALSE);
	tp_channel_run_until_ready (chan, NULL, NULL);
	// TODO: check for errors
	UT_DEBUGMSG(("Channel created to the room\n"));
	
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
	if (!tp_cli_channel_interface_group_run_add_members(chan, -1,  members, "Hi there!", &error, NULL))
	{
		UT_DEBUGMSG(("Error inviting room members: %s\n", error ? error->message : "(null)"));
		return false;
	}
	UT_DEBUGMSG(("Members invited\n"));

	GHashTable* params = tp_asv_new (
			"title", G_TYPE_STRING, pDoc->getFilename(),
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
	sTubeAddress = address;
	
	// start listening on the tube for people entering and leaving it
	tp_cli_channel_type_tubes_connect_to_d_bus_names_changed (chan, tube_dbus_names_changed_cb, this, NULL, NULL, NULL);

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


