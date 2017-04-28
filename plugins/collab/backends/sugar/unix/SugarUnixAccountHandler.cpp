/* Copyright (C) 2007 One Laptop Per Child
 * Author: Marc Maurer <uwog@uwog.net>
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

#include "SugarUnixAccountHandler.h"
#include "SugarBuddy.h"
#include <account/xp/AccountEvent.h>
#include <account/xp/Event.h>
#include <core/account/xp/SessionEvent.h>
#include <session/xp/AbiCollabSessionManager.h>
#include <session/xp/AbiCollab.h>
#include <ev_EditMethod.h>
#include <xap_App.h>
#include <fv_View.h>

// some fucntion prototype declarations
static bool s_offerTube(AV_View* v, EV_EditMethodCallData *d);
static bool s_joinTube(AV_View* v, EV_EditMethodCallData *d);
static bool s_disconnectTube(AV_View* v, EV_EditMethodCallData *d);
static bool s_buddyJoined(AV_View* v, EV_EditMethodCallData *d);
static bool s_buddyLeft(AV_View* v, EV_EditMethodCallData *d);
static DBusHandlerResult s_dbus_handle_message(DBusConnection *connection, DBusMessage *message, void *user_data);

#define INTERFACE "com.abisource.abiword.abicollab.olpc"
#define SEND_ALL_METHOD "SendAll"
#define SEND_ONE_METHOD "SendOne"

SugarAccountHandler* SugarAccountHandler::m_pHandler = NULL;
SugarAccountHandler* SugarAccountHandler::getHandler() { return m_pHandler; }

SugarAccountHandler::SugarAccountHandler()
	: AccountHandler(),
	m_pTube(NULL),
	m_bIsInSession(false)
{
	UT_DEBUGMSG(("SugarAccountHandler::SugarAccountHandler()\n"));
	m_pHandler = this;
	_registerEditMethods();
}

SugarAccountHandler::~SugarAccountHandler()
{
	m_pHandler = NULL;
	disconnect();
}

UT_UTF8String SugarAccountHandler::getDescription()
{
	return "Sugar Presence Service";
}

UT_UTF8String SugarAccountHandler::getDisplayType()
{
	return "Sugar Presence Service";
}

UT_UTF8String SugarAccountHandler::getStaticStorageType()
{
	return SUGAR_STATIC_STORAGE_TYPE;
}

void SugarAccountHandler::loadProperties()
{
	// no need to implement this as we will be getting
	// all our info always directly from sugar
}

void SugarAccountHandler::storeProperties()
{
	// no need to implement this as we will be getting
	// all our info always directly from sugar
}

ConnectResult SugarAccountHandler::connect()
{
	UT_ASSERT_HARMLESS(UT_NOT_REACHED);
	return CONNECT_SUCCESS;
}

bool SugarAccountHandler::disconnect()
{
	if (m_pTube)
	{
		dbus_connection_unref(m_pTube);
		m_pTube = NULL;
	}
	return true;
}

bool SugarAccountHandler::isOnline()
{
	return true;
}

BuddyPtr SugarAccountHandler::constructBuddy(const PropertyMap& props)
{
	UT_DEBUGMSG(("SugarAccountHandler::constructBuddy()\n"));

	PropertyMap::const_iterator cit = props.find("dbusAddress");
	UT_return_val_if_fail(cit != props.end(), SugarBuddyPtr());
	UT_return_val_if_fail(cit->second.size() > 0, SugarBuddyPtr());

	UT_DEBUGMSG(("Constructing SugarBuddy (dbusAddress: %s)\n", cit->second.c_str()));
	// NOTE: the buddy name must uniquely identify a buddy, and I can't
	// guarantee at the moment that the name we could get from the sugar
	// presence framework would always be unique to one buddy; hence the
	// dbus address will do for now
	return boost::shared_ptr<SugarBuddy>(new SugarBuddy(this, cit->second.c_str()));
}

BuddyPtr SugarAccountHandler::constructBuddy(const std::string& descriptor, BuddyPtr /*pBuddy*/)
{
	UT_DEBUGMSG(("SugarAccountHandler::constructBuddy() - descriptor: %s\n", descriptor.c_str()));

	std::string uri_id = "sugar://";
	UT_return_val_if_fail(descriptor.size() > uri_id.size(), SugarBuddyPtr());

	std::string dbusAddress = descriptor.substr(uri_id.size());
	SugarBuddyPtr pBuddy = getBuddy(dbusAddress.c_str());
	UT_return_val_if_fail(pBuddy, SugarBuddyPtr());

	return pBuddy;
}

bool SugarAccountHandler::recognizeBuddyIdentifier(const std::string& identifier)
{
	std::string uri_id = "sugar://";

	if (identifier.compare(0, uri_id.size(), uri_id) != 0)
		return false;

	// The rest of the buddy descriptor contains the dbus address, which we
	// can't really check.

	return true;
}

void  SugarAccountHandler::handleEvent(Session& /*pSession*/)
{
	// TODO: implement me
}

void SugarAccountHandler::signal(const Event& event, BuddyPtr pSource)
{
	UT_DEBUGMSG(("SugarAccountHandler::signal()\n"));

	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_if_fail(pManager);

	switch (event.getClassType())
	{
		case PCT_CloseSessionEvent:
			{
				UT_DEBUGMSG(("Got a PCT_CloseSessionEvent\n"));
				const CloseSessionEvent cse = static_cast<const CloseSessionEvent&>(event);
				UT_return_if_fail(!pSource); // we shouldn't receive these events over the wire on this backend

				// If the session that is closed was started by us, then disconnect from
				// the tube. Otherwise, just drop the event on the floor....

				if (cse.getSessionId() == m_sSessionId)
				{
					UT_DEBUGMSG(("We host session %s, disconnecting...\n", cse.getSessionId().c_str()));
					disconnect();
				}
			}
			break;

		case PCT_AccountBuddyAddDocumentEvent:
			{
				// Prevent joining other dochandles that come over the wire after having
				// already joined the one we received just now. This should ofcourse never 
				// happen, but it will in practice because of a Write/Activity bug.
				// See ... for details.
				if (m_bIsInSession)
				{
					UT_DEBUGMSG(("Received a bogus AccountBuddyAddDocumentEvent: we are already connected to a session.\n"));
					return;
				}

				// We've received a document handle from the other side. This obviously only 
				// makes sense for a joining party, not an offering one: the offering party 
				// should never even receive such an event

				UT_DEBUGMSG(("We received a document handle from an offering party; let's join it immediately!\n"));
				AccountBuddyAddDocumentEvent& abade = (AccountBuddyAddDocumentEvent&)event;

				// FIXME: should we check if we were waiting for a document to come our way?

				DocHandle* pDocHandle = abade.getDocHandle();
				UT_return_if_fail(pDocHandle);

				UT_DEBUGMSG(("Got dochandle, going to initiate a join on it!\n"));
				pManager->joinSessionInitiate(pSource, pDocHandle);
				m_bIsInSession = true;
			}
			break;

		default:
			AccountHandler::signal(event, pSource);
			break;
	}
}

bool SugarAccountHandler::send(const Packet* pPacket)
{
	UT_DEBUGMSG(("SugarAccountHandler::send(const Packet* pPacket)\n"));
	UT_return_val_if_fail(pPacket, false);
	UT_return_val_if_fail(m_pTube, false);

	return _send(pPacket, NULL);
}

bool SugarAccountHandler::send(const Packet* pPacket, BuddyPtr pBuddy)
{
	UT_DEBUGMSG(("SugarAccountHandler::send(const Packet* pPacket, const Buddy& buddy)\n"));
	UT_return_val_if_fail(pPacket, false);
	UT_return_val_if_fail(m_pTube, false);
	
	SugarBuddyPtr pSugarBuddy = boost::static_pointer_cast<SugarBuddy>(pBuddy);
	UT_DEBUGMSG(("Sending packet to sugar buddy on dbus addess: %s\n", pSugarBuddy->getDBusAddress().utf8_str()));

	return _send(pPacket, pSugarBuddy->getDBusAddress().utf8_str());
}

Packet* SugarAccountHandler::createPacket(const std::string& packet, BuddyPtr pBuddy)
{
	return _createPacket(packet, pBuddy);
}

bool SugarAccountHandler::_send(const Packet* pPacket, const char* dbusAddress)
{
	UT_DEBUGMSG(("SugarAccountHandler::_send() - dbusAddress: %s\n", dbusAddress ? dbusAddress : "(broadcast)"));
	UT_return_val_if_fail(pPacket, false);
	UT_return_val_if_fail(m_pTube, false);

	DBusMessage* pMessage = dbus_message_new_method_call(dbusAddress, "/org/laptop/Sugar/Presence/Buddies", INTERFACE, SEND_ONE_METHOD);
	if (dbusAddress)
	{
		// TODO: isn't this redudant? we already set a destination in dbus_message_new_method_call()
		if (!dbus_message_set_destination(pMessage, dbusAddress))
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			dbus_message_unref(pMessage);
			return false;
		}
		UT_DEBUGMSG(("Destination (%s) set on message\n", dbusAddress));
	}

	// we don't want replies, because then then easily run into dbus timeout problems 
	// when sending large packets
	// TODO: this means we should probably use signals though
	dbus_message_set_no_reply(pMessage, TRUE);
	
	// make to-be-send-stream once
	std::string data;
	_createPacketStream( data, pPacket );

	const char* packet_contents = &data[0];
	if (!dbus_message_append_args(pMessage, DBUS_TYPE_ARRAY, DBUS_TYPE_BYTE, &packet_contents, data.size(), DBUS_TYPE_INVALID))
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		dbus_message_unref(pMessage);
		return false;
	}

	UT_DEBUGMSG(("Appended packet contents\n"));

	bool sent = dbus_connection_send(m_pTube, pMessage, NULL);
	UT_ASSERT_HARMLESS(sent);
	if (sent)
		dbus_connection_flush(m_pTube);
	dbus_message_unref(pMessage);

	return sent;	
}

void SugarAccountHandler::_registerEditMethods()
{
	UT_DEBUGMSG(("SugarAccountHandler::_registerEditMethods()\n"));

    // First we need to get a pointer to the application itself.
    XAP_App *pApp = XAP_App::getApp();
    EV_EditMethodContainer* pEMC = pApp->getEditMethodContainer();

	EV_EditMethod *emOfferTube = new EV_EditMethod (
		"com.abisource.abiword.abicollab.olpc.offerTube",     // name of callback function
		s_offerTube,       // callback function itself.
		0,                      // no additional data required.
		""                      // description -- allegedly never used for anything
	);
	pEMC->addEditMethod(emOfferTube);

	EV_EditMethod *emJoinTube = new EV_EditMethod (
		"com.abisource.abiword.abicollab.olpc.joinTube",     // name of callback function
		s_joinTube,       // callback function itself.
		0,                      // no additional data required.
		""                      // description -- allegedly never used for anything
	);
	pEMC->addEditMethod(emJoinTube);

	EV_EditMethod *emDisconnectTube = new EV_EditMethod (
		"com.abisource.abiword.abicollab.olpc.disconnectTube",     // name of callback function
		s_disconnectTube,       // callback function itself.
		0,                      // no additional data required.
		""                      // description -- allegedly never used for anything
	);
	pEMC->addEditMethod(emDisconnectTube);

	EV_EditMethod *emBuddyJoined = new EV_EditMethod (
		"com.abisource.abiword.abicollab.olpc.buddyJoined",     // name of callback function
		s_buddyJoined,       // callback function itself.
		0,                      // no additional data required.
		""                      // description -- allegedly never used for anything
	);
	pEMC->addEditMethod(emBuddyJoined);

	EV_EditMethod *emBuddyLeft = new EV_EditMethod (
		"com.abisource.abiword.abicollab.olpc.buddyLeft",     // name of callback function
		s_buddyLeft,       // callback function itself.
		0,                      // no additional data required.
		""                      // description -- allegedly never used for anything
	);
	pEMC->addEditMethod(emBuddyLeft);

}

void SugarAccountHandler::_handlePacket(Packet* packet, BuddyPtr buddy)
{
	UT_DEBUGMSG(("SugarAccountHandler::_handlePacket()\n"));

	UT_return_if_fail(packet);
	UT_return_if_fail(buddy);

	switch (packet->getClassType())
	{
		case PCT_JoinSessionRequestResponseEvent:
		{
			JoinSessionRequestResponseEvent* jsre = static_cast<JoinSessionRequestResponseEvent*>( packet );
			m_sSessionId = jsre->getSessionId();
			// Let the AccountHandler::_handlePacket() call below handle the actual joining.
			// This could mean that when the actual joining fails (unlikely), we have
			// a bogus session ID stored. I doubt it will ever really be a problem though.
			break;
		}

		default:
			break;
	}

	AccountHandler::_handlePacket(packet, buddy);
}

bool SugarAccountHandler::offerTube(FV_View* pView, const UT_UTF8String& tubeDBusAddress)
{
	UT_DEBUGMSG(("SugarAccountHandler::offerTube()\n"));
	UT_return_val_if_fail(pView, false);

	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_val_if_fail(pManager, false);

	// TODO: check that we aren't already in a session; this backend can only host one session at a time (for now)

	PD_Document * pDoc = pView->getDocument();
	UT_return_val_if_fail(pDoc, false);

	UT_DEBUGMSG(("Got tube address: %s\n", tubeDBusAddress.utf8_str()));

	m_pTube = dbus_connection_open(tubeDBusAddress.utf8_str(), NULL);
	UT_return_val_if_fail(m_pTube, false);

	UT_DEBUGMSG(("Opened a dbus connection for tube: %s\n", tubeDBusAddress.utf8_str()));

	UT_DEBUGMSG(("Adding dbus handlers to the main loop\n"));
	dbus_connection_setup_with_g_main(m_pTube, NULL);

	UT_DEBUGMSG(("Adding message filter\n"));
	dbus_connection_add_filter(m_pTube, s_dbus_handle_message, this, NULL);

	// start hosting a session on the current document
	UT_return_val_if_fail(m_sSessionId == "", false);
	AbiCollab* pSession = pManager->startSession(pDoc, m_sSessionId, this, true, NULL, "");
	UT_return_val_if_fail(pSession, false);

	// we are "connected" now, time to start sending out, and listening to messages (such as events)
	pManager->registerEventListener(this);

	m_bIsInSession = true;

	return true;
}

bool SugarAccountHandler::joinTube(FV_View* pView, const UT_UTF8String& tubeDBusAddress)
{
	UT_DEBUGMSG(("SugarAccountHandler::joinTube()\n"));
	UT_return_val_if_fail(pView, false);

	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_val_if_fail(pManager, false);

	// TODO: check that we aren't already in a session; this backend can only join one session at a time (for now)

	m_pTube = dbus_connection_open(tubeDBusAddress.utf8_str(), NULL);
	UT_return_val_if_fail(m_pTube, false);

	UT_DEBUGMSG(("Opened a dbus connection for tube: %s\n", tubeDBusAddress.utf8_str()));

	UT_DEBUGMSG(("Adding dbus handlers to the main loop\n"));
	dbus_connection_setup_with_g_main(m_pTube, NULL);

	UT_DEBUGMSG(("Adding message filter\n"));
	dbus_connection_add_filter(m_pTube, s_dbus_handle_message, this, NULL);

	// we are "connected" now, time to start sending out, and listening to messages (such as events)
	pManager->registerEventListener(this);

	// broadcast a request for sessions; if everything is alright then we should
	// receive exactly 1 session in all the responses combined
	UT_DEBUGMSG(("Sending a broadcast GetSessionsEvent\n"));
	GetSessionsEvent event;
	send(&event);

	return true;
}

bool SugarAccountHandler::disconnectTube(FV_View* pView)
{
	UT_DEBUGMSG(("SugarAccountHandler::disconnectTube()\n"));
	UT_return_val_if_fail(pView, false);

	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_val_if_fail(pManager, false);

	PD_Document * pDoc = pView->getDocument();
	UT_return_val_if_fail(pDoc, false);

	AbiCollab* pSession = pManager->getSession(pDoc);
	UT_return_val_if_fail(pSession, false);
	pManager->disconnectSession(pSession);

	return true;
}

bool SugarAccountHandler::joinBuddy(FV_View* pView, const UT_UTF8String& buddyDBusAddress)
{
	UT_DEBUGMSG(("SugarAccountHandler::joinBuddy() - buddyDBusAddress: %s\n", buddyDBusAddress.utf8_str()));
	UT_return_val_if_fail(pView, false);

	SugarBuddyPtr pBuddy = boost::shared_ptr<SugarBuddy>(new SugarBuddy(this, buddyDBusAddress));
	addBuddy(pBuddy);

	return true;
}

bool SugarAccountHandler::disjoinBuddy(FV_View* pView, const UT_UTF8String& buddyDBusAddress)
{
	UT_DEBUGMSG(("SugarAccountHandler::disjoinBuddy()\n"));
	UT_return_val_if_fail(pView, false);

	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_val_if_fail(pManager, false);

	PD_Document * pDoc = pView->getDocument();
	UT_return_val_if_fail(pDoc, false);
	
	m_ignoredBuddies.erase( buddyDBusAddress ); // buddy name is buddyDBusAddress!

	BuddyPtr pBuddy = getBuddy(buddyDBusAddress);
	UT_return_val_if_fail(pBuddy, false);

	pManager->removeBuddy(pBuddy, false);
	
	// TODO: shouldn't we remove this buddy from our own buddy list?	

	return true;
}

void SugarAccountHandler::forceDisconnectBuddy(BuddyPtr pBuddy)
{
	UT_return_if_fail(pBuddy);
	m_ignoredBuddies.insert(pBuddy->getDescriptor(false));
}

bool SugarAccountHandler::hasAccess(const std::vector<std::string>& /*vAcl*/, BuddyPtr pBuddy)
{
	UT_DEBUGMSG(("SugarAccountHandler::hasAccess() - pBuddy: %s\n", pBuddy->getDescriptor(false).utf8_str()));
	UT_return_val_if_fail(pBuddy, false);

	// The sugar presence service is responsible for access control. Just do a quick
	// check here to see if we know the buddy on this account, and
	// then be done with it.
	SugarBuddyPtr pSugarBuddy = boost::dynamic_pointer_cast<SugarBuddy>(pBuddy);
	UT_return_val_if_fail(pSugarBuddy, false);

	SugarBuddyPtr pExistingBuddy = getBuddy(pSugarBuddy->getDBusAddress());
	if (!pExistingBuddy)
		return false;

	return true;
}

SugarBuddyPtr SugarAccountHandler::getBuddy(const UT_UTF8String& dbusAddress)
{
	for (std::vector<BuddyPtr>::iterator it = getBuddies().begin(); it != getBuddies().end(); it++)
	{
		SugarBuddyPtr pBuddy = boost::static_pointer_cast<SugarBuddy>(*it);
		UT_continue_if_fail(pBuddy);
		if (pBuddy->getDBusAddress() == dbusAddress)
			return pBuddy;
	}
	return SugarBuddyPtr();
}

static bool s_offerTube(AV_View* v, EV_EditMethodCallData *d)
{
	UT_DEBUGMSG(("s_offerTube()\n"));
	UT_return_val_if_fail(v, false);
	UT_return_val_if_fail(d && d->m_pData && d->m_dataLength > 0, false);

	FV_View* pView = static_cast<FV_View *>(v);
	UT_UTF8String tubeDBusAddress(d->m_pData, d->m_dataLength);

	SugarAccountHandler* pHandler = SugarAccountHandler::getHandler();
	UT_return_val_if_fail(pHandler, false);
	return pHandler->offerTube(pView, tubeDBusAddress);
}

static bool s_joinTube(AV_View* v, EV_EditMethodCallData *d)
{
	UT_DEBUGMSG(("s_joinTube()\n"));
	UT_return_val_if_fail(v, false);
	UT_return_val_if_fail(d && d->m_pData && d->m_dataLength > 0, false);

	FV_View* pView = static_cast<FV_View *>(v);
	UT_UTF8String tubeDBusAddress(d->m_pData, d->m_dataLength);
	UT_DEBUGMSG(("Got tube address: %s\n", tubeDBusAddress.utf8_str()));

	SugarAccountHandler* pHandler = SugarAccountHandler::getHandler();
	UT_return_val_if_fail(pHandler, false);
	return pHandler->joinTube(pView, tubeDBusAddress);
}

static bool s_disconnectTube(AV_View* v, EV_EditMethodCallData */*d*/)
{
	UT_DEBUGMSG(("s_disconnectTube()\n"));
	UT_return_val_if_fail(v, false);
	FV_View* pView = static_cast<FV_View *>(v);
	
	SugarAccountHandler* pHandler = SugarAccountHandler::getHandler();
	UT_return_val_if_fail(pHandler, false);
	return pHandler->disconnectTube(pView);
}

static bool s_buddyJoined(AV_View* v, EV_EditMethodCallData *d)
{
	UT_DEBUGMSG(("s_buddyJoined()\n"));
	UT_return_val_if_fail(SugarAccountHandler::getHandler(), false);
	UT_return_val_if_fail(d && d->m_pData && d->m_dataLength > 0, false);

	FV_View* pView = static_cast<FV_View *>(v);
	UT_UTF8String buddyPath(d->m_pData, d->m_dataLength);
	UT_DEBUGMSG(("Adding buddy with dbus path: %s\n", buddyPath.utf8_str()));

	SugarAccountHandler* pHandler = SugarAccountHandler::getHandler();
	UT_return_val_if_fail(pHandler, false);
	return pHandler->joinBuddy(pView, buddyPath);
}

static bool s_buddyLeft(AV_View* v, EV_EditMethodCallData *d)
{
	UT_DEBUGMSG(("s_buddyLeft()\n"));
	UT_return_val_if_fail(SugarAccountHandler::getHandler(), false);
	UT_return_val_if_fail(d && d->m_pData && d->m_dataLength > 0, false);

	FV_View* pView = static_cast<FV_View *>(v);
	UT_UTF8String buddyPath(d->m_pData, d->m_dataLength);
	UT_DEBUGMSG(("Removing buddy with dbus path %s\n", buddyPath.utf8_str()));

	SugarAccountHandler* pHandler = SugarAccountHandler::getHandler();
	UT_return_val_if_fail(pHandler, false);

	return pHandler->disjoinBuddy(pView, buddyPath);
}

DBusHandlerResult s_dbus_handle_message(DBusConnection *connection, DBusMessage *message, void *user_data)
{
	UT_DEBUGMSG(("s_dbus_handle_message()\n"));
	UT_return_val_if_fail(connection, DBUS_HANDLER_RESULT_NOT_YET_HANDLED);
	UT_return_val_if_fail(message, DBUS_HANDLER_RESULT_NOT_YET_HANDLED);
	UT_return_val_if_fail(user_data, DBUS_HANDLER_RESULT_NOT_YET_HANDLED);
	SugarAccountHandler* pHandler = reinterpret_cast<SugarAccountHandler*>(user_data);

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
			
			if (!pHandler->isIgnoredBuddy(senderDBusAddress))
			{
				// import the packet
				BuddyPtr pBuddy = pHandler->getBuddy(senderDBusAddress);
				if (!pBuddy)
				{
					// this can actually happen, for example when joining a tube
					// we send out a broadcast GetSessionsEvent. Responses from
					// that can return before joinBuddy() was called for that buddy.
					pBuddy = boost::shared_ptr<SugarBuddy>(new SugarBuddy( pHandler, senderDBusAddress));
					pHandler->addBuddy(pBuddy);
				}

				// FIXME: inefficient copying of data
				std::string packet_str(packet_size, ' ');
				memcpy(&packet_str[0], packet_data, packet_size);
				Packet* pPacket = pHandler->createPacket(packet_str, pBuddy);
				UT_return_val_if_fail(pPacket, DBUS_HANDLER_RESULT_NOT_YET_HANDLED); // TODO: shouldn't we just disconnect here?

				// handle!
				pHandler->handleMessage(pPacket, pBuddy);
			}

			//dbus_free(packet);
			return DBUS_HANDLER_RESULT_HANDLED;
		}
		else
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	}	

	UT_DEBUGMSG(("Unhandled message\n"));
	return DBUS_HANDLER_RESULT_NOT_YET_HANDLED;
}
