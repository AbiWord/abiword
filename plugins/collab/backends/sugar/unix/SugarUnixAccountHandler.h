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

#ifndef __SUGARACCOUNTHANDLER__
#define __SUGARACCOUNTHANDLER__

#include <set>

#include <account/xp/AccountHandler.h>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include "SugarBuddy.h"

#define SUGAR_STATIC_STORAGE_TYPE "com.abisource.abiword.abicollab.backend.sugar"

extern AccountHandlerConstructor SugarAccountHandlerConstructor;

class Session;
class FV_View;

class SugarAccountHandler : public AccountHandler
{
public:
	static SugarAccountHandler*				getHandler();
	SugarAccountHandler(); // TODO: this constructor shouldn't be public
	virtual ~SugarAccountHandler();

	// housekeeping
	static UT_UTF8String					getStaticStorageType();
	virtual UT_UTF8String					getStorageType() override
		{ return getStaticStorageType(); }
	virtual UT_UTF8String					getDescription() override;
	virtual UT_UTF8String					getDisplayType() override;

	// dialog management
	virtual void embedDialogWidgets(void* /*pEmbeddingParent*/) override
		{ UT_ASSERT_HARMLESS(UT_NOT_REACHED); }
	virtual void removeDialogWidgets(void* /*pEmbeddingParent*/) override
		{ UT_ASSERT_HARMLESS(UT_NOT_REACHED); }
	virtual bool canDelete() override
		{ return false; }
	virtual bool canEditProperties() override
		{ return false; }
	virtual void loadProperties() override;
	virtual void storeProperties() override;

	// connection management
	virtual ConnectResult connect() override;
	virtual bool disconnect() override;
	virtual bool isOnline() override;

	// user management
	virtual BuddyPtr constructBuddy(const PropertyMap& props) override;
	virtual BuddyPtr constructBuddy(const std::string& descriptor, BuddyPtr pBuddy) override;
	virtual bool allowsManualBuddies() override
		{ return false; }
	virtual void forceDisconnectBuddy(BuddyPtr pBuddy) override;
	virtual bool hasAccess(const std::vector<std::string>& vAcl, BuddyPtr pBuddy) override;
	virtual bool hasPersistentAccessControl()  override
		{ return false; }
	virtual bool recognizeBuddyIdentifier(const std::string& identifier) override;

	// session management
	virtual bool allowsSessionTakeover() override
		{ return true; }
	virtual bool canManuallyStartSession() override
		{ return false; }

	// packet management
	virtual bool send(const Packet* pPacket) override;
	virtual bool send(const Packet* pPacket, BuddyPtr buddy) override;
	Packet*									createPacket(const std::string& packet, BuddyPtr pBuddy);

	// event management
	void									handleEvent(Session& pSession);

	// signal management
	virtual void signal(const Event& event, BuddyPtr pSource) override;

	// tube & buddy management
	SugarBuddyPtr							getBuddy(const UT_UTF8String& dbusAddress);
	bool									offerTube(FV_View* pView, const UT_UTF8String& tubeDBusAddress);
	bool									joinTube(FV_View* pView, const UT_UTF8String& tubeDBusAddress);
	bool									disconnectTube(FV_View* pView);
	bool									joinBuddy(FV_View* pView, const UT_UTF8String& buddyDBusAddress);
	bool									disjoinBuddy(FV_View* pView, const UT_UTF8String& buddyDBusAddress);

	bool									isIgnoredBuddy(const UT_UTF8String& buddyName)
		{ return m_ignoredBuddies.find(buddyName) != m_ignoredBuddies.end(); }

protected:
	bool									_send(const Packet* pPacket, const char* dbusAddress);
	void									_registerEditMethods();
	virtual	void _handlePacket(Packet* packet, BuddyPtr buddy) override;

private:
	static SugarAccountHandler* 			m_pHandler;
	DBusConnection*							m_pTube;
	bool									m_bIsInSession;
	std::set<UT_UTF8String>					m_ignoredBuddies;
	std::string							m_sSessionId;
};

#endif /* __SUGARACCOUNTHANDLER__ */
