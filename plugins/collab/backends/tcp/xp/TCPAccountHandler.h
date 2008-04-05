/* Copyright (C) 2006,2007 by Marc Maurer <uwog@uwog.net>
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

#ifndef __TCPACCOUNTHANDLER__
#define __TCPACCOUNTHANDLER__

#include "IOServerHandler.h"
#include "IOClientHandler.h"

#include <backends/xp/AccountHandler.h>

class TCPBuddy;

#define DEFAULT_TCP_PORT 25509  /* log2(e + pi) * 10^4 */

extern AccountHandlerConstructor TCPAccountHandlerConstructor;

class ABI_EXPORT TCPAccountHandler : public AccountHandler
{
public:
	TCPAccountHandler();
	virtual ~TCPAccountHandler();

	// housekeeping
	virtual UT_UTF8String					getDescription();
	virtual UT_UTF8String					getDisplayType();
	virtual UT_UTF8String					getStorageType();
	
	// dialog management 
	virtual void							storeProperties();

	// connection management
	virtual ConnectResult					connect();
	virtual bool							disconnect();
	virtual bool							isOnline();
	void									handleAccept(IOServerHandler* pHandler);

	// user management
	virtual Buddy*							constructBuddy(const PropertyMap& props);
	virtual bool							allowsManualBuddies()
		{ return false; }
	virtual void							forceDisconnectBuddy(Buddy* buddy);
		
	// packet management
	virtual bool							send(const Packet* packet);
	virtual bool							send(const Packet*, const Buddy& buddy);

	// event management
	void									handleEvent(Session& pSession);

protected:

	// connection management
	virtual UT_sint32						_getPort(const PropertyMap& props);

	// user management
	const TCPBuddy*							_getBuddy(Session* pSession);
	
private:
	void									_teardownAndDestroyHandler();
		
	bool									m_bConnected; // TODO: drop this, ask the IO handler
	IOHandler*								m_pDelegator;

	Session*								m_pPendingSession; // only used when accepting connections
	std::map<const TCPBuddy*, Session*>		m_clients; // mapping buddies and their accompanying session
};

#endif /* __TCPACCOUNTHANDLER__ */
