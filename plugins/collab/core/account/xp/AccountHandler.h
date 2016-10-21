/* Copyright (C) 2006 by Marc Maurer <uwog@uwog.net>
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

#ifndef __ACCOUNTHANDLER_H__
#define __ACCOUNTHANDLER_H__

#include <map>
#include <string>
#include <list>
#include <vector>
#include "ut_types.h"
#include "ut_string_class.h"
#include "ut_vector.h"
#include "DocHandle.h"
#include <packet/xp/AbiCollab_Packet.h>
#include <account/xp/Buddy.h>
#include <account/xp/EventListener.h>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

class AbiCollab;

typedef enum _ConnectResult
{
	CONNECT_SUCCESS = 0,
	CONNECT_FAILED,
	CONNECT_IN_PROGRESS,
	CONNECT_AUTHENTICATION_FAILED,
	CONNECT_ALREADY_CONNECTED,
	CONNECT_INTERNAL_ERROR
} ConnectResult;

typedef AccountHandler* (*AccountHandlerConstructor)();

typedef std::map<std::string, std::string> PropertyMap;

class ProtocolErrorPacket : public Packet
{
public:
	ProtocolErrorPacket();
	ProtocolErrorPacket( UT_sint32 errorEnum );
	DECLARE_PACKET(ProtocolErrorPacket);

	virtual UT_sint32						getProtocolVersion() const
		{ return 0; } // not ABICOLLAB_PROTOCOL_VERSION!!
	UT_sint32 								getErrorEnum() const
		{ return m_errorEnum; }
	UT_sint32								getRemoteVersion() const
		{ return m_remoteVersion; }
protected:
	UT_sint32		m_errorEnum;
	UT_sint32		m_remoteVersion;
};

class AccountHandler : public EventListener
{
public:
	AccountHandler() {}
	virtual ~AccountHandler() {}

	// housekeeping
	static UT_UTF8String					getStaticStorageType()
		{ /* every backend should re-implement this static function */ return "null"; };
	virtual UT_UTF8String					getStorageType() = 0;
	virtual UT_UTF8String					getDescription() = 0;
	virtual UT_UTF8String					getDisplayType() = 0;

	void									addProperty(const std::string& key, const std::string& value)
		{ m_properties[key] = value; }
	bool									hasProperty(const std::string& key);
	const std::string							getProperty(const std::string& key);
	PropertyMap&							getProperties()
		{ return m_properties; }

	// dialog management
	virtual UT_UTF8String					getShareHint(PD_Document* /*pDoc*/) { return ""; }
	virtual void							embedDialogWidgets(void* pEmbeddingParent) = 0;
	virtual void							removeDialogWidgets(void* pEmbeddingParent) = 0;
	virtual bool							canDelete()
		{ return true; }
	virtual bool							canEditProperties()
		{ return true; }
	virtual void							loadProperties() = 0;
	virtual void							storeProperties() = 0;

	#ifdef _WIN32
	virtual BOOL							_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam) = 0;
	virtual bool							shouldProcessFocus() = 0;
	#endif

	// connection management
	virtual ConnectResult					connect() = 0;
	virtual bool							disconnect() = 0;
	virtual bool							isOnline() = 0;
	virtual bool							autoConnect();

	// for duplicate prevention
	virtual bool							operator==(AccountHandler & rhHandler);

	// packet management
	virtual bool							send(const Packet* packet) = 0;
	virtual bool							send(const Packet* packet, BuddyPtr buddy) = 0;

	// user management
	void									addBuddy(BuddyPtr pBuddy);
	std::vector<BuddyPtr>&					getBuddies()
		{ return m_vBuddies; }
	virtual void							getBuddiesAsync() {}
	void									deleteBuddy(BuddyPtr pBuddy);
	void									deleteBuddies();
	virtual BuddyPtr						constructBuddy(const PropertyMap& vProps) = 0;
	// Constructs a buddy given a buddy descriptor
	// NOTE: some backends require additional *backend specific* session information
	// to construct a particular buddy. Information that is not available in the buddy
	// descriptor (the service and sugar backends need this for example)
	// This additional information can then be retrieved via the 'pBuddy'
	// argument. That means that the pBuddy should already be in a session and
	// thus have access to all backend specific session information
	virtual BuddyPtr						constructBuddy(const std::string& descriptor, BuddyPtr pBuddy) = 0;
	virtual bool							recognizeBuddyIdentifier(const std::string& identifier) = 0;
	virtual bool							allowsManualBuddies() = 0;
	virtual void							forceDisconnectBuddy(BuddyPtr /*buddy*/) = 0;
	virtual bool							hasAccess(const std::vector<std::string>& vAcl, BuddyPtr pBuddy);
	virtual bool							hasPersistentAccessControl() = 0;
	// Return true if you can to share a file with this buddy, false otherwise
	virtual bool							canShare(BuddyPtr /*pBuddy*/)
		{ return true; }
	virtual bool							defaultShareState(BuddyPtr /*pBuddy*/)
		{ return false; }

	// session management
	virtual void							getSessionsAsync();
	virtual void							getSessionsAsync(BuddyPtr pBuddy);
	virtual bool							startSession(PD_Document* /*pDoc*/, const std::vector<std::string>& /*vAcl*/, AbiCollab** /*pSession*/)
		{ return true; }
	virtual bool							getAcl(AbiCollab* /*pSession*/, std::vector<std::string>& /*vAcl*/)
		{ return true; }
	virtual bool							setAcl(AbiCollab* /*pSession*/, const std::vector<std::string>& /*vAcl*/)
		{ return true; }
	virtual void							joinSessionAsync(BuddyPtr pBuddy, DocHandle& docHandle);
	virtual bool							hasSession(const UT_UTF8String& sSessionId);
	virtual bool							allowsSessionTakeover() = 0;
	bool									getCanOffer()
		{ return m_bCanOffer; }
	void									setOffering(bool bCanOffer)
		{ m_bCanOffer = bCanOffer; }
	virtual bool							canManuallyStartSession()
		{ return true; }

	// generic session management packet implementation
	virtual void							handleMessage(Packet* pPacket, BuddyPtr pBuddy);

	// signal management
	virtual void							signal(const Event& event, BuddyPtr pSource);

	// protocol error management
	enum
	{
		PE_Invalid_Version 		= 1,	// only possible error atm ^_^
	};

protected:
	// packet management
	Packet*									_createPacket(const std::string& packet, BuddyPtr pBuddy);
	void 									_createPacketStream(std::string& sString, const Packet* pPacket);	// creates binary string!
	virtual bool							_handleProtocolError(Packet* packet, BuddyPtr buddy);
	virtual	void							_handlePacket(Packet* packet, BuddyPtr buddy);

	// protocol error management
	void									_sendProtocolError(BuddyPtr, UT_sint32 errorEnum);

	// bad bad, protected variables are bad
	PropertyMap								m_properties;

private:
	static void								_reportProtocolError(UT_sint32 remoteVersion, UT_sint32 errorEnum, BuddyPtr buddy);

	bool									m_bCanOffer;
	std::vector<BuddyPtr>					m_vBuddies;
};


#endif /* ACCOUNTHANDLER_H */
