/* Copyright (C) 2007 One Laptop Per Child
 * Author: Marc Maurer <uwog@uwog.net>
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

#ifndef __TELEPATHY_ACCOUNT_HANDLER__
#define __TELEPATHY_ACCOUNT_HANDLERr__

#include <vector>

#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

#include <telepathy-glib/telepathy-glib.h>

#include <account/xp/AccountHandler.h>
#include "DTubeBuddy.h"
#include "TelepathyBuddy.h"

extern AccountHandlerConstructor TelepathyAccountHandlerConstructor;

class Session;
class FV_View;

class TelepathyAccountHandler : public AccountHandler
{
public:
	static TelepathyAccountHandler*				getHandler();
	TelepathyAccountHandler(); // TODO: this constructor shouldn't be public
	virtual ~TelepathyAccountHandler();

	// housekeeping
	virtual UT_UTF8String					getDescription();
	virtual UT_UTF8String					getDisplayType();
	virtual UT_UTF8String					getStorageType();
	
	// dialog management 
	virtual void							storeProperties();
	virtual void							embedDialogWidgets(void* /*pEmbeddingParent*/)
		{ UT_ASSERT_HARMLESS(UT_NOT_REACHED); }
	virtual void							removeDialogWidgets(void* /*pEmbeddingParent*/)
		{ UT_ASSERT_HARMLESS(UT_NOT_REACHED); }

	// connection management
	virtual ConnectResult					connect();
	virtual bool							disconnect();
	virtual bool							isOnline();
	void									acceptTube(TpChannel *tubes_chan, const char* address);
	bool									offerTube(TelepathyChatroomPtr pChatroom, TpChannel* chan, const UT_UTF8String& sSessionId);

	// user management
	virtual void							getBuddiesAsync();
	virtual BuddyPtr						constructBuddy(const PropertyMap& props);
	virtual BuddyPtr						constructBuddy(const std::string& descriptor, BuddyPtr pBuddy);
	virtual bool							recognizeBuddyIdentifier(const std::string& identifier);
	virtual bool							allowsManualBuddies()
		{ return false; }
	virtual void							forceDisconnectBuddy(BuddyPtr pBuddy);
	virtual bool 							hasAccess(const std::vector<std::string>& /*vAcl*/, BuddyPtr pBuddy);
	virtual bool							hasPersistentAccessControl()
		{ return true; }
	void									addContact(TpContact* contact);
	void									buddyDisconnected(TelepathyChatroomPtr pChatroom, TpHandle disconnected);

	// session management
	virtual bool							startSession(PD_Document* pDoc, const std::vector<std::string>& acl, AbiCollab** pSession);
	virtual bool							allowsSessionTakeover()
		{ return false; /* not right now */ }

	// signal management
	virtual void							signal(const Event& event, BuddyPtr pSource);
	
	// packet management
	virtual bool							send(const Packet* pPacket);
	virtual bool							send(const Packet* pPacket, BuddyPtr buddy);
	void									handleMessage(DTubeBuddyPtr pBuddy, const std::string& packet_str);
	
private:
	TelepathyBuddyPtr						_getBuddy(TelepathyBuddyPtr pBuddy);

	std::vector<TelepathyChatroomPtr>		m_chatrooms;
};

#endif /* __TELEPATHY_ACCOUNT_HANDLER__ */
