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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef __TELEPATHY_ACCOUNT_HANDLER__
#define __TELEPATHY_ACCOUNT_HANDLER__

#include <vector>
#include <string>

#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

#include <telepathy-glib/telepathy-glib.h>

#include <account/xp/AccountHandler.h>
#include "DTubeBuddy.h"
#include "TelepathyBuddy.h"

#define DEFAULT_CONFERENCE_SERVER "conference.telepathy.im"
#define INTERFACE "org.freedesktop.Telepathy.Client.AbiCollab"
#define SEND_ONE_METHOD "SendOne"

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
	virtual UT_UTF8String getDescription() override;
	virtual UT_UTF8String getDisplayType() override;
	virtual UT_UTF8String getStorageType() override;

	// dialog management
	virtual void embedDialogWidgets(void* pEmbeddingParent) override;
	virtual void removeDialogWidgets(void* pEmbeddingParent) override;
	virtual bool canDelete() override
		{ return false; }
	virtual void loadProperties() override;
	virtual void storeProperties() override;

	// connection management
	virtual ConnectResult connect() override;
	virtual bool disconnect() override;
	virtual bool isOnline() override;
	void									acceptTube(TpChannel *tubes_chan, const char* address);
	void									finalizeOfferTube(TelepathyChatroomPtr pChatroom);

	// user management
	virtual void getBuddiesAsync() override;
	virtual BuddyPtr constructBuddy(const PropertyMap& props) override;
	virtual BuddyPtr constructBuddy(const std::string& descriptor, BuddyPtr pBuddy) override;
	virtual bool recognizeBuddyIdentifier(const std::string& identifier) override;
	virtual bool allowsManualBuddies() override
		{ return false; }
	virtual void forceDisconnectBuddy(BuddyPtr pBuddy) override;
	virtual bool hasAccess(const std::vector<std::string>& /*vAcl*/, BuddyPtr pBuddy) override;
	virtual bool hasPersistentAccessControl() override
		{ return true; }
	void									addContact(TpContact* contact);
	void									buddyDisconnected(TelepathyChatroomPtr pChatroom, TpHandle disconnected);

	// session management
	virtual bool startSession(PD_Document* pDoc, const std::vector<std::string>& acl, AbiCollab** pSession) override;
	virtual bool setAcl(AbiCollab* /*pSession*/, const std::vector<std::string>& /*vAcl*/) override;
	virtual bool allowsSessionTakeover() override
		{ return false; /* not right now */ }
	void									unregisterChatroom(TelepathyChatroomPtr pChatroom);

	// signal management
	virtual void signal(const Event& event, BuddyPtr pSource) override;

	// packet management
	virtual bool send(const Packet* pPacket) override;
	virtual bool send(const Packet* pPacket, BuddyPtr buddy) override;
	void									handleMessage(DTubeBuddyPtr pBuddy, const std::string& packet_str);

private:
	void									_inviteBuddies(TelepathyChatroomPtr pChatroom, const std::vector<std::string>& /*vAcl*/);
	std::vector<TelepathyBuddyPtr>			_getBuddies(const std::vector<std::string>& vAcl);
	TelepathyBuddyPtr						_getBuddy(TelepathyBuddyPtr pBuddy);
	TelepathyChatroomPtr					_getChatroom(const std::string& sSessionId);

	GtkWidget*								table;
	GtkWidget*								conference_entry;
	GtkWidget*								autoconnect_button;

	TpBaseClient*							m_pTpClient;
	std::vector<TelepathyChatroomPtr>		m_chatrooms;
};

#endif /* __TELEPATHY_ACCOUNT_HANDLER__ */
