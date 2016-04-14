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

#include <session/xp/AbiCollab.h>
#include <session/xp/AbiCollabSessionManager.h>

#include "TelepathyUnixAccountHandler.h"
#include "TelepathyChatroom.h"
#include "DTubeBuddy.h"

static void
tube_call_offer_cb(TpChannel* /*proxy*/,
		const gchar* out_address,
		const GError *error,
		gpointer user_data,
		GObject* /*weak_object*/)
{
	UT_DEBUGMSG(("tube_call_offer_cb()\n"));
	if (error)
	{
		UT_DEBUGMSG(("Error offering tube to room members: %s\n", error->message));
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return;
	}

	TelepathyChatroom* pChatroom = reinterpret_cast<TelepathyChatroom*>(user_data);
	UT_return_if_fail(pChatroom);

	TelepathyAccountHandler* pHandler = pChatroom->getHandler();
	UT_return_if_fail(pHandler);

	// open and store the tube dbus connection
	UT_DEBUGMSG(("Tube offered, address: %s\n", out_address));
	DBusConnection* pTube = dbus_connection_open_private(out_address, NULL);
	UT_return_if_fail(pTube);

	pChatroom->finalizeOfferTube(pTube);
}

static void
group_call_add_members_cb(TpChannel* chan,
		const GError *error,
		gpointer user_data,
		GObject* /*weak_object*/)
{
	UT_DEBUGMSG(("group_call_add_members_cb()\n"));
	if (error)
	{
		UT_DEBUGMSG(("Error inviting room members: %s\n", error->message));
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return;
	}

	TelepathyChatroom* pChatroom = reinterpret_cast<TelepathyChatroom*>(user_data);
	UT_return_if_fail(pChatroom);

	if (!pChatroom->tubeOffered())
	{
		UT_DEBUGMSG(("Offering tube to room members...\n"));

		TelepathyAccountHandler* pHandler = pChatroom->getHandler();
		UT_return_if_fail(pHandler);

		GHashTable* params = tp_asv_new (
				"title", G_TYPE_STRING, pChatroom->getDocName().c_str(),
				NULL);

		// offer this tube to every participant in the room
		tp_cli_channel_type_dbus_tube_call_offer(
				chan, -1, params, TP_SOCKET_ACCESS_CONTROL_LOCALHOST,
				tube_call_offer_cb,
				pChatroom, NULL, NULL);

		g_hash_table_destroy (params);
	}
	else
		UT_DEBUGMSG(("Tube was already offered to this room, skipping\n"));
}

static void
get_contact_for_new_buddie_cb(TpConnection* /*connection*/,
		guint n_contacts,
		TpContact * const *contacts,
		guint /*n_invalid*/,
		const TpHandle* /*invalid*/,
		const GError* error,
		gpointer user_data,
		GObject* /*weak_object*/)
{
	UT_DEBUGMSG(("get_contact_for_new_buddie_cb()\n"));
	UT_return_if_fail(!error);
	//UT_return_if_fail(n_contacts == 2);
	UT_return_if_fail(n_contacts == 1);

	DTubeBuddy* pBuddy = reinterpret_cast<DTubeBuddy*>(user_data);
	UT_return_if_fail(pBuddy);

	TelepathyChatroomPtr pChatroom = pBuddy->getChatRoom();
	UT_return_if_fail(pChatroom);

	DTubeBuddyPtr pDTubeBuddy = boost::shared_ptr<DTubeBuddy>(pBuddy);
	pDTubeBuddy->setContact(contacts[0]);
	//pDTubeBuddy->setGlobalContact(contacts[1]);
	pChatroom->addBuddy(pDTubeBuddy);

	if (!pChatroom->isLocallyControlled())
	{
		// send a request for sessions; if everything is alright then we should
		// receive exactly 1 session from 1 of the buddies that will be added to the room;
		// maybe it would be a better idea to send it only to the session master, but
		// we don't know who it is (is the current master always the channel initiator?)
		// TODO: we should not send this event when we already have a master in this room
		UT_DEBUGMSG(("Sending a GetSessionsEvent to the new buddy %s\n", pDTubeBuddy->getDescriptor(false).utf8_str()));
		pChatroom->getHandler()->getSessionsAsync(pDTubeBuddy);
	}
}

static void
add_buddy_to_room(TpConnection* connection, TpChannel* chan, TpHandle handle, DTubeBuddy* pBuddy)
{
	UT_DEBUGMSG(("add_buddy_to_room()\n"));
	UT_return_if_fail(connection);
	UT_return_if_fail(chan);
	UT_return_if_fail(pBuddy);

	static TpContactFeature features[] = {
		TP_CONTACT_FEATURE_ALIAS,
		TP_CONTACT_FEATURE_PRESENCE
	};

	// Unfortunately, MUC rooms on conference.telepathy.im don't expose the real JIDs
	// at the moment. This means that without it, we can't do proper security in ::hasAccess()
	// See https://bugs.freedesktop.org/show_bug.cgi?id=37631 for details.
	//TpHandle global_handle = tp_channel_group_get_handle_owner(chan, handle);
	//UT_return_if_fail(global_handle != 0);

	std::vector<TpHandle> handles;
	handles.push_back(handle);
	//handles.push_back(global_handle);
	tp_connection_get_contacts_by_handle (connection,
			handles.size(), &handles[0],
			G_N_ELEMENTS (features), features,
			get_contact_for_new_buddie_cb,
			pBuddy, NULL, NULL);
}

static void
tube_dbus_names_changed_cb(TpChannel* chan,
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

	TpConnection* pConnection = tp_channel_borrow_connection(chan);
	UT_return_if_fail(pConnection);

	gpointer key;
	gpointer value;
	GHashTableIter iter;
	g_hash_table_iter_init(&iter, arg_Added);
	while (g_hash_table_iter_next(&iter, &key, &value))
	{
		TpHandle handle = GPOINTER_TO_UINT(key);
		const char* dbus_name = reinterpret_cast<const gchar*>(value);
		UT_DEBUGMSG(("Adding a new buddy: %d - %s\n", handle, dbus_name));
		add_buddy_to_room(pConnection, chan, handle, new DTubeBuddy(pHandler, pChatroom->ptr(), handle, dbus_name));
	}

	for (UT_uint32 i = 0; i < arg_Removed->len; i++)
	{
		TpHandle removed = g_array_index(arg_Removed, TpHandle, i);
		UT_DEBUGMSG(("Buddy with handle %d left\n", removed));

		pHandler->buddyDisconnected(pChatroom->ptr(), removed);
	}
}

static void
tp_channel_close_cb(TpChannel* /*proxy*/,
		const GError *error,
		gpointer user_data,
		GObject* /*weak_object*/)
{
	TelepathyChatroom* pChatroom = reinterpret_cast<TelepathyChatroom*>(user_data);
	UT_return_if_fail(pChatroom);

	if (error)
	{
		UT_DEBUGMSG(("Failed to close channel %s\n", error->message));
		pChatroom->finalize();
		return;
	}

	pChatroom->finalize();
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

static void
retrieve_buddy_dbus_mappings_cb(TpProxy* proxy,
		const GValue *out_Value,
		const GError *error,
		gpointer user_data,
		GObject* /*weak_object*/)
{
	UT_DEBUGMSG(("retrieve_buddy_dbus_mappings_cb()\n"));
	if (error)
	{
		UT_DEBUGMSG(("Error retrieving buddy dbus addresses: %s\n", error->message));
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return;
	}

	UT_return_if_fail(G_VALUE_HOLDS(out_Value, TP_HASH_TYPE_DBUS_TUBE_PARTICIPANTS));

	TelepathyChatroom* pChatroom = reinterpret_cast<TelepathyChatroom*>(user_data);
	UT_return_if_fail(pChatroom);

	TpChannel* chan = TP_CHANNEL(proxy);
	UT_return_if_fail(chan);

	TpConnection* connection = tp_channel_borrow_connection(chan);
	UT_return_if_fail(connection);

	TpHandle self_handle = tp_channel_group_get_self_handle(chan);

	GHashTable* name_mapping = reinterpret_cast<GHashTable*>(g_value_get_boxed(out_Value));
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
		add_buddy_to_room(connection, chan, contact_handle, new DTubeBuddy(pChatroom->getHandler(), pChatroom->ptr(), contact_handle, contact_address));
	}
}

TelepathyChatroom::TelepathyChatroom(TelepathyAccountHandler* pHandler, TpChannel* pChannel,
		PD_Document* pDoc, const UT_UTF8String& sSessionId)
	: m_pHandler(pHandler),
	m_pChannel(pChannel),
	m_pDoc(pDoc),
	m_pTube(NULL),
	m_sSessionId(sSessionId),
	m_bShuttingDown(false)
{
	if (m_pChannel)
		g_object_ref(m_pChannel);

	// prevent the TelepathyAccountHandler from being deleted while
	// there are (possible) outstanding asynchronous operations
	// for this room
	AbiCollabSessionManager::getManager()->beginAsyncOperation(m_pHandler);
}

void TelepathyChatroom::stop()
{
	UT_DEBUGMSG(("TelepathyChatroom::stop()\n"));

	if (m_pChannel)
		tp_cli_channel_call_close(m_pChannel, 5000, tp_channel_close_cb, this, NULL, NULL);
	else
		finalize();
}

void TelepathyChatroom::finalize()
{
	UT_DEBUGMSG(("TelepathyChatroom::finalize()\n"));

	if (m_pChannel)
	{
		g_object_ref(m_pChannel);
		m_pChannel = NULL;
	}

	if (m_pTube)
	{
		dbus_connection_close(m_pTube);
		m_pTube = NULL;
	}

	// keep a shared pointer to ourselves: the unregisterChatroom() call
	// on the next line will make use self-destruct without it, because it
	// removes the last reference to this room. Every call after that call
	// would then result in a stack corruption.
	TelepathyChatroomPtr self = ptr();

	// unregister ourselves from the TelepathyAccountHandler
	m_pHandler->unregisterChatroom(self);

	AbiCollabSessionManager::getManager()->endAsyncOperation(m_pHandler);
}

void TelepathyChatroom::setChannel(TpChannel* pChannel)
{
	m_pChannel = pChannel;
	g_object_ref(m_pChannel);
}

void TelepathyChatroom::addBuddy(DTubeBuddyPtr pBuddy)
{
	// make sure we don't add this buddy twice
	UT_return_if_fail(pBuddy);
	for (std::vector<DTubeBuddyPtr>::iterator it = m_buddies.begin(); it != m_buddies.end(); it++)
	{
		DTubeBuddyPtr pB = (*it);
		UT_continue_if_fail(pB);
		if (pBuddy->getDBusName() == pB->getDBusName())
		{
			UT_DEBUGMSG(("Not adding buddy with dbus address %s twice\n", pBuddy->getDBusName().utf8_str()));
			return;
		}
	}

	m_buddies.push_back(pBuddy);

	// flush any queued up packets for this buddy
	std::map<std::string, std::vector<std::string> >::iterator pos = m_packet_queue.find(pBuddy->getDBusName().utf8_str());
	if (pos != m_packet_queue.end())
	{
		const std::vector<std::string>& packets = (*pos).second;
		UT_DEBUGMSG(("Flushing %d packets for buddy %s\n", (int)packets.size(), pBuddy->getDBusName().utf8_str()));
		for (UT_uint32 i = 0; i < packets.size(); i++)
			m_pHandler->handleMessage(pBuddy, packets[i]);

		m_packet_queue.erase(pos);
	}
}

DTubeBuddyPtr TelepathyChatroom::getBuddy(TpHandle handle)
{
	for (UT_uint32 i = 0; i < m_buddies.size(); i++)
	{
		DTubeBuddyPtr pBuddy = m_buddies[i];
		UT_continue_if_fail(pBuddy);

		if (pBuddy->getHandle() == handle)
			return pBuddy;
	}

	return DTubeBuddyPtr();
}

DTubeBuddyPtr TelepathyChatroom::getBuddy(UT_UTF8String dbusName)
{
	for (UT_uint32 i = 0; i < m_buddies.size(); i++)
	{
		DTubeBuddyPtr pBuddy = m_buddies[i];
		UT_continue_if_fail(pBuddy);

		if (pBuddy->getDBusName() == dbusName)
			return pBuddy;
	}

	return DTubeBuddyPtr();
}

void TelepathyChatroom::removeBuddy(TpHandle handle)
{
	for (std::vector<DTubeBuddyPtr>::iterator it = m_buddies.begin(); it != m_buddies.end(); it++)
	{
		DTubeBuddyPtr pB = *it;
		UT_continue_if_fail(pB);
		if (pB->getHandle() == handle)
		{
			// TODO: when we know the global JID of a DTubeBuddy, then
			// remove it from the m_pending_invitees list as well
			m_buddies.erase(it);
			return;
		}
	}
	UT_ASSERT_HARMLESS(UT_NOT_REACHED);
}

std::string TelepathyChatroom::getDocName()
{
	UT_return_val_if_fail(m_pDoc, "");
        std::string docName = m_pDoc->getFilename();
	if (docName == "")
		return "Untitled"; // TODO: fetch the title from the frame somehow (which frame?) - MARCM
	return docName;
}

void TelepathyChatroom::queue(const std::string& dbusName, const std::string& packet)
{
	UT_DEBUGMSG(("Queueing packet for %s\n", dbusName.c_str()));
	m_packet_queue[dbusName].push_back(packet);
}

void TelepathyChatroom::acceptTube(const char* address)
{
	UT_DEBUGMSG(("TelepathyChatroom::acceptTube() - address: %s\n", address));
	UT_return_if_fail(address);
	UT_return_if_fail(m_pChannel);
	UT_return_if_fail(!m_pTube)

	TpConnection* connection = tp_channel_borrow_connection(m_pChannel);
	UT_return_if_fail(connection);

	DBusError dbus_error;
	dbus_error_init(&dbus_error);
	m_pTube = dbus_connection_open_private(address, &dbus_error);
	if (!m_pTube)
	{
		UT_DEBUGMSG(("Error opening dbus connection to address %s: %s\n", address, dbus_error.message));
		dbus_error_free (&dbus_error);
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return;
	}

	UT_DEBUGMSG(("Adding dbus handlers to the main loop for tube %s\n", address));
	dbus_connection_setup_with_g_main(m_pTube, NULL);

	UT_DEBUGMSG(("Adding message filter\n"));
	dbus_connection_add_filter(m_pTube, s_dbus_handle_message, this, NULL);

	// start listening on the tube for people entering and leaving it
	GError* error = NULL;
	TpProxySignalConnection* proxy_signal = tp_cli_channel_type_dbus_tube_connect_to_dbus_names_changed(
															m_pChannel, tube_dbus_names_changed_cb,
															this, NULL, NULL, &error);
	if (!proxy_signal)
	{
		UT_DEBUGMSG(("Error connecting to names_changes: %s\n", error ? error->message : "(null)"));
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return;
	}

	// retrieve the TpHandle <-> dbus address mapping for the people in the room
	// so we can add them as buddies to our chatroom
	tp_cli_dbus_properties_call_get(
			m_pChannel, -1, TP_IFACE_CHANNEL_TYPE_DBUS_TUBE, "DBusNames",
			retrieve_buddy_dbus_mappings_cb,
			this, NULL, NULL);
}

void TelepathyChatroom::queueInvite(TelepathyBuddyPtr pBuddy)
{
	UT_DEBUGMSG(("TelepathyChatroom::queueInvite() - %s\n", pBuddy->getDescriptor(false).utf8_str()));
	UT_return_if_fail(pBuddy);

	// check if we have already offered a tube to this buddy
	for (std::vector<std::string>::iterator it = m_offered_tubes.begin(); it != m_offered_tubes.end(); it++)
	{
		if ((*it) == pBuddy->getDescriptor(false).utf8_str())
		{
			UT_DEBUGMSG(("Tube already offered to %s, skipping\n", pBuddy->getDescriptor(false).utf8_str()));
			return;
		}
	}

	// check if this buddy is already on the invite list
	for (std::vector<TelepathyBuddyPtr>::iterator it = m_pending_invitees.begin(); it != m_pending_invitees.end(); it++)
	{
		UT_continue_if_fail(*it);
		if ((*it)->getDescriptor(false) == pBuddy->getDescriptor(false))
		{
			UT_DEBUGMSG(("%s already queued for an invitation, skipping\n", pBuddy->getDescriptor(false).utf8_str()));
			return;
		}
	}

	m_pending_invitees.push_back(pBuddy);
}


bool TelepathyChatroom::offerTube()
{
	UT_DEBUGMSG(("TelepathyChatroom::offerTube()\n - session id: %s\n", m_sSessionId.utf8_str()));
	UT_return_val_if_fail(m_sSessionId != "", false);
	UT_return_val_if_fail(m_pChannel, false);

	if (m_pending_invitees.size() == 0)
	{
		UT_DEBUGMSG(("0 pending invitees, skipping tube offering\n"));
		return TRUE;
	}

	// add members to the room
	GArray* members = g_array_new (FALSE, FALSE, sizeof(TpHandle));
	for (UT_uint32 i = 0; i < m_pending_invitees.size(); i++)
	{
		TelepathyBuddyPtr pBuddy = m_pending_invitees[i];
		UT_continue_if_fail(pBuddy && pBuddy->getContact());
		TpHandle handle = tp_contact_get_handle(pBuddy->getContact());
		UT_DEBUGMSG(("Adding %s to the invite list\n", tp_contact_get_identifier(pBuddy->getContact())));
		g_array_append_val(members, handle);
		m_offered_tubes.push_back(pBuddy->getDescriptor(false).utf8_str());
	}
	m_pending_invitees.clear();

	// create the welcome string
	UT_UTF8String sWelcomeMsg = UT_UTF8String_sprintf("A document called '%s' has been shared with you", getDocName().c_str());

	UT_DEBUGMSG(("Inviting members to the room...\n"));
	tp_cli_channel_interface_group_call_add_members(
			m_pChannel, -1,  members, sWelcomeMsg.utf8_str(),
			group_call_add_members_cb,
			this, NULL, NULL);

	return true;
}

void TelepathyChatroom::finalizeOfferTube(DBusConnection* pTube)
{
	UT_DEBUGMSG(("TelepathyChatroom::finalizeOfferTube()\n"));
	UT_return_if_fail(pTube);
	m_pTube = pTube;

	UT_DEBUGMSG(("Adding tube dbus handlers to the main loop\n"));
	dbus_connection_setup_with_g_main(m_pTube, NULL);

	UT_DEBUGMSG(("Adding message filter\n"));
	dbus_connection_add_filter(m_pTube, s_dbus_handle_message, this, NULL);

	// start listening on the tube for people entering and leaving it
	GError* error;
	TpProxySignalConnection* proxy_signal = tp_cli_channel_type_dbus_tube_connect_to_dbus_names_changed(
															m_pChannel, tube_dbus_names_changed_cb,
															this, NULL, NULL, &error);
	if (!proxy_signal)
	{
		UT_DEBUGMSG(("Error connecting to names_changes: %s\n", error ? error->message : "(null)"));
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return;
	}
}

bool TelepathyChatroom::isController(DTubeBuddyPtr pBuddy)
{
	UT_return_val_if_fail(m_sSessionId != "", FALSE);

	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_val_if_fail(pManager, false);

	AbiCollab* pSession = pManager->getSessionFromSessionId(m_sSessionId);
	UT_return_val_if_fail(pSession, false);

	return pSession->isController(pBuddy);
}

bool TelepathyChatroom::isLocallyControlled()
{
	if (m_sSessionId == "")
		return FALSE;

	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_val_if_fail(pManager, false);

	AbiCollab* pSession = pManager->getSessionFromSessionId(m_sSessionId);
	UT_return_val_if_fail(pSession, false);

	return pSession->isLocallyControlled();
}
