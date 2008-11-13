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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef __ACCOUNTHANDLER_H__
#define __ACCOUNTHANDLER_H__

#include <map>
#include <string>
#include <list>
#include <vector>
#include "ut_types.h"
#include "ut_string_class.h"
#include <backends/xp/Buddy.h>
#include "ut_vector.h"
#include "DocHandle.h"
#include <xp/AbiCollab_Packet.h>
#include <backends/xp/EventListener.h>
#ifdef WIN32
#include <windows.h>
#endif

class AbiCollab;

using std::string;
using std::map;
using std::vector;

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

typedef map<string, string> PropertyMap; 

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
	virtual UT_UTF8String					getDescription() = 0;	
	virtual UT_UTF8String					getDisplayType() = 0;
	virtual UT_UTF8String					getStorageType() = 0;	
	
	void									addProperty(const string& key, const string& value)
		{ m_properties[key] = value; }
	const string							getProperty(const string& key);
	PropertyMap&							getProperties()
		{ return m_properties; }

	// dialog management 
	virtual void							embedDialogWidgets(void* pEmbeddingParent) = 0;
	virtual void							removeDialogWidgets(void* pEmbeddingParent) = 0;
	virtual void							storeProperties() = 0;
	
	#ifdef WIN32
	virtual BOOL							_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
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
	virtual bool							send(const Packet* packet, const Buddy& buddy) = 0;
	
	// user management
	void									addBuddy(Buddy* buddy);
	const UT_GenericVector<Buddy*>&	 		getBuddies() const
		{ return m_vecBuddies; }
	Buddy*									getBuddy(const UT_UTF8String& name);
	void									deleteBuddy(const UT_UTF8String& name);
	void									deleteBuddies();
	virtual Buddy*							constructBuddy(const PropertyMap& vProps) = 0;
	virtual bool							allowsManualBuddies() = 0;
	virtual void							forceDisconnectBuddy(Buddy* buddy);
	
	bool getCanOffer()
		{ return m_bCanOffer; }

	void setOffering(bool bCanOffer)
		{ m_bCanOffer = bCanOffer; }
		
	// session management
	virtual void							getSessionsAsync();
	virtual void							getSessionsAsync(const Buddy& buddy);
	virtual void							joinSessionAsync(const Buddy& buddy, DocHandle& docHandle);
	virtual bool							hasSession(const UT_UTF8String& sSessionId);

	// generic session management packet implementation
	virtual void 							handleMessage(const RawPacket& pRp);
	virtual void							handleMessage(Packet* pPacket, Buddy* pBuddy);

	// signal management
	virtual void							signal(const Event& event, const Buddy* pSource);
	
	// protocol error management
	static void enableProtocolErrorReports(bool enable); // NOTE: enabled by default!
	enum
	{
		PE_Invalid_Version 		= 1,	// only possible error atm ^_^
	};

protected:
	Packet*									_createPacket(const std::string& packet, Buddy* pBuddy);
	void 									_createPacketStream(std::string& sString, const Packet* pPacket);	// creates binary string!
	void									_sendProtocolError(const Buddy& buddy, UT_sint32 errorEnum);
	virtual bool							_handleProtocolError(Packet* packet, Buddy* buddy);
	virtual	void							_handlePacket(Packet* packet, Buddy* buddy, bool autoAddBuddyOnJoin = false);

	// bad bad, protected variables are bad
	PropertyMap								m_properties;

private:
	static void								_reportProtocolError(UT_sint32 remoteVersion, UT_sint32 errorEnum, const Buddy& buddy);
	static bool								showProtocolErrorReports;

	bool									m_bCanOffer;
	UT_GenericVector<Buddy*>				m_vecBuddies;
};


#endif /* ACCOUNTHANDLER_H */
