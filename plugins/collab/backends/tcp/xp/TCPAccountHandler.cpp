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

#include "ut_timer.h"

#include "TCPAccountHandler.h"
#include "TCPBuddy.h"

#include <backends/xp/AccountEvent.h>

#include <xp/AbiCollabSessionManager.h>

static void IOAcceptFunc(IOServerHandler* pHandler)
{
	UT_DEBUGMSG(("IOAcceptFunc()\n"));
	UT_return_if_fail(pHandler);
	pHandler->getAccountHandler().handleAccept(pHandler);
}

static void IOEventFunc(Session* pSession)
{
	UT_DEBUGMSG(("IOEventFunc()\n"));
	UT_return_if_fail(pSession);
	pSession->consume();
	pSession->getAccountHandler().handleEvent(*pSession);
}

TCPAccountHandler::TCPAccountHandler()
	: AccountHandler(),
	m_bConnected(false),
	m_pDelegator(0),
	m_pPendingSession(0)
{
}

TCPAccountHandler::~TCPAccountHandler()
{
	if (m_bConnected)
		disconnect();
}

UT_UTF8String TCPAccountHandler::getDescription()
{
	const std::string host = getProperty("server");
	const std::string port = getProperty("port");	
	
	if (host == "")
		return UT_UTF8String_sprintf("Listening on port %s", port.c_str());
	return UT_UTF8String_sprintf("%s:%s", host.c_str(), port.c_str());
}

UT_UTF8String TCPAccountHandler::getDisplayType()
{
	return "Direct Connection (TCP)";
}

UT_UTF8String TCPAccountHandler::getStorageType()
{
	return "com.abisource.abiword.abicollab.backend.tcp";
}

void TCPAccountHandler::storeProperties()
{
	UT_DEBUGMSG(("TCPAccountHandler::storeProperties() - TODO: implement me\n"));
}

ConnectResult TCPAccountHandler::connect()
{
	UT_DEBUGMSG(("TCPAccountHandler::connect()\n"));

	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_val_if_fail(pManager, CONNECT_INTERNAL_ERROR);

	
	if (m_bConnected)
		return CONNECT_ALREADY_CONNECTED;

	// set up the connection
	if (getProperty("server") == "")
	{
		UT_sint32 port = _getPort(getProperties());
		UT_DEBUGMSG(("Start accepting connections on port %d...\n", port));

		// TODO: do we catch all possible acceptions?
		try
		{
			IOServerHandler* pDelegator = new IOServerHandler(port, IOAcceptFunc, *this);
			m_pDelegator = pDelegator;
			m_bConnected = true; // todo: ask it to the acceptor

			m_pPendingSession = pDelegator->constructSession(IOEventFunc, *this);
			pDelegator->run(*m_pPendingSession);
		} catch (asio::system_error se)
		{
			UT_DEBUGMSG(("Failed to start accepting connections: %s\n", se.what()));
			_teardownAndDestroyHandler();
			return CONNECT_FAILED;
		}
	}
	else
	{
		UT_DEBUGMSG(("Connecting to server %s on port %d...\n", getProperty("server").c_str(), _getPort(getProperties())));
	
		// TODO: catch exceptions
		try
		{
			IOClientHandler* pDelegator = new IOClientHandler(getProperty("server"), getProperty("port"));
			m_pDelegator = pDelegator;
		
			Session* pSession = pDelegator->constructSession(IOEventFunc, *this);
			if (pSession)
			{
				pDelegator->connect(*pSession);
				m_bConnected = true; // todo: ask it to the socket
			
				// Add a buddy
				UT_UTF8String name = getProperty("server").c_str();
				name += ":";
				name += getProperty("port").c_str();
				TCPBuddy* pBuddy = new TCPBuddy(this, name);
				addBuddy(pBuddy);
				m_clients.insert(std::pair<const TCPBuddy*, Session*>(pBuddy, pSession));
			}
		}
		catch (asio::system_error se)
		{
			UT_DEBUGMSG(("Failed to connect to server: %s\n", se.what()));
			_teardownAndDestroyHandler();
			return CONNECT_FAILED;
		}
	}
	
	if (m_bConnected)
	{
		// we are connected now, time to start sending out messages (such as events)
		pManager->registerEventListener(this);
		// signal all listeners we are logged in
		AccountOnlineEvent event;
		// TODO: fill the event
		AbiCollabSessionManager::getManager()->signal(event);
	}

	return CONNECT_SUCCESS;
}

bool TCPAccountHandler::disconnect()
{
	UT_DEBUGMSG(("TCPAccountHandler::disconnect()\n"));

	UT_return_val_if_fail(m_bConnected, false);

	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_val_if_fail(pManager, false);
	
	_teardownAndDestroyHandler();
	m_bConnected = false;

	// signal all listeners we are logged out
	AccountOfflineEvent event;
	// TODO: fill the event
	AbiCollabSessionManager::getManager()->signal(event);
	// we are disconnected now, no need to sent out messages (such as events) anymore
	pManager->unregisterEventListener(this);	
	return true;
}

void TCPAccountHandler::_teardownAndDestroyHandler()
{
	UT_return_if_fail(m_pDelegator);

	// first, stop the IO handler
	m_pDelegator->stop();

	// ... then destroy the pending client session (if any) (ugly)
	if (m_pPendingSession)
		DELETEP(m_pPendingSession);

	// ... then tear down all client connections
	for (std::map<const TCPBuddy*, Session*>::iterator it = m_clients.begin(); it != m_clients.end();)
	{
		std::map<const TCPBuddy*, Session*>::iterator next_it = it;
		next_it++;

		// const TCPBuddy* pBuddy = (*it).first;
		// TODO: signal the buddy leaving && free the buddy

		Session* pSession = (*it).second;
		DELETEP(pSession);

		m_clients.erase(it);
		it = next_it;
	}

	// ... and then kill off the IO handler alltogether
	DELETEP(m_pDelegator);
}

bool TCPAccountHandler::isOnline()
{
	return m_bConnected; // TODO: ask the IO handler
}

void TCPAccountHandler::handleAccept(IOServerHandler* pHandler)
{
	UT_DEBUGMSG(("TCPAccountHandler::handleAccept\n"));

	UT_return_if_fail(pHandler);
	
	if (m_pPendingSession)
	{
		// store this buddy/session
		UT_UTF8String name;
		UT_UTF8String_sprintf(name, "%s:%d", 
				m_pPendingSession->getSocket().remote_endpoint().address().to_string().c_str(), 
				m_pPendingSession->getSocket().remote_endpoint().port());
		TCPBuddy* pBuddy = new TCPBuddy(this, name);
		addBuddy(pBuddy);
		m_clients.insert(std::pair<const TCPBuddy*, Session*>(pBuddy, m_pPendingSession));
	}
	else
		UT_ASSERT_HARMLESS(UT_NOT_REACHED);
	
	// accept a new buddy/session
	m_pPendingSession = pHandler->constructSession(IOEventFunc, *this);
	pHandler->handleAccept(*m_pPendingSession);	
}

Buddy* TCPAccountHandler::constructBuddy(const PropertyMap& props)
{
	UT_DEBUGMSG(("TCPAccountHandler::constructBuddy()\n"));

	PropertyMap::const_iterator hi = props.find("server");
	UT_return_val_if_fail(hi != props.end(), 0);
	UT_return_val_if_fail(hi->second.size() > 0, 0);

	UT_sint32 port = _getPort(props);
	UT_return_val_if_fail(port != -1, 0);
	
	UT_DEBUGMSG(("Constructing TCP Buddy (host: %s, port: %d)\n", hi->second.c_str(), port));
	UT_UTF8String name;
	UT_UTF8String_sprintf(name, "%s:%d", hi->second.c_str(), port); 
	return new TCPBuddy(this, name);
}

void TCPAccountHandler::forceDisconnectBuddy(Buddy* buddy)
{
	UT_DEBUGMSG(("TCPAccountHandler::forceDisconnectBuddy()\n"));
	
	// locate this buddy!
	std::map<const TCPBuddy*, Session*>::iterator it=m_clients.find( static_cast<TCPBuddy*>(buddy)/*ugly*/ );
	if (it==m_clients.end()) 
	{
		for (it=m_clients.begin(); it!=m_clients.end(); ++it) 
		{
			if ((*it).first->getName() == buddy->getName()) 
			{
				break;
			}
		}
		UT_return_if_fail(it!=m_clients.end());
	}
	
	// disconnect it
	UT_ASSERT(it!=m_clients.end());
	try 
	{
		delete (*it).second;
	} 
	catch (...) 
	{ 
		UT_DEBUGMSG(("ERROR: exception during delete second\n"));
	}
	try 
	{
		delete (*it).first;
	} 
	catch (...) 
	{
		UT_DEBUGMSG(("ERROR: exception during delete first\n"));
	}
	m_clients.erase( it );
}

void TCPAccountHandler::handleEvent(Session& session)
{
	UT_DEBUGMSG(("TCPAccountHandler::handleEvent()\n"));
	
	// get an incoming packet, if any
	// TODO: we could read all packets here in one go
	int packet_size;
	char* packet_data;
	if (session.pop(packet_size, &packet_data))
	{
		/*
		printf("Got packet:\n");
		for (int i = 0; i < packet_size; i++)
		{
			printf("%02x ", (UT_uint8)packet_data[i] );
		}
		printf("\n");
		*/
		
		// setup raw packet struct
		RawPacket pRp;
		pRp.buddy = const_cast<TCPBuddy*>(_getBuddy(&session));
		pRp.packet.resize( packet_size );
		memcpy( &pRp.packet[0], packet_data, packet_size );
		
		// cleanup packet
		FREEP(packet_data);
		
		// handle!
		handleMessage( pRp );
	}

	// check the connection status
	if (!session.isConnected())
	{
		UT_DEBUGMSG(("Socket is not connected anymore!!!!!\n"));
		// FIXME: handle this (delete the session, and remove it from the client list)
	}

	// check other things here if needed...
}

UT_sint32 TCPAccountHandler::_getPort(const PropertyMap& props)
{
	PropertyMap::const_iterator pi = props.find("port");

	UT_sint32 port = -1;
	if (pi == props.end()) // no port specified, use the default port
	{
		port = DEFAULT_TCP_PORT;
	}
	else
	{
		long portl = strtol(pi->second.c_str(), (char **)NULL, 10);
		if (portl == LONG_MIN || portl == LONG_MAX) // TODO: we should check errno here for ERANGE
			port = DEFAULT_TCP_PORT;
		else
			port = (UT_sint32)portl;	
	}
	
	return port;
}

const TCPBuddy* TCPAccountHandler::_getBuddy(Session* pSession)
{
	for (std::map<const TCPBuddy*, Session*>::iterator it = m_clients.begin(); it != m_clients.end(); it++)
	{
		std::pair<const TCPBuddy*, Session*> pbs = *it;
		if (pbs.second == pSession)
			return pbs.first;
	}
	UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	return 0;
}

bool TCPAccountHandler::send(const Packet* packet)
{
	UT_DEBUGMSG(("TCPAccountHandler::_send(const UT_UTF8String& packet)\n"));

	// don't bother creating a nice buffer if there's noone to send it to
	if (!m_clients.empty()) 
	{
		// make to-be-send-stream once
		std::string data;
		_createPacketStream( data, packet );
		
		// send it to everyone we know!
		for (std::map<const TCPBuddy*, Session*>::iterator it = m_clients.begin(); it != m_clients.end(); it++)
		{
			std::pair<const TCPBuddy*, Session*> pbs = *it;
			if (pbs.second)
			{
				pbs.second->asyncWrite(data.size(), data.c_str());
			}
			else
				UT_ASSERT(UT_NOT_REACHED);
		}
	}
	return true;
}

bool TCPAccountHandler::send(const Packet* packet, const Buddy& buddy)
{
	UT_DEBUGMSG(("TCPAccountHandler::_send(const UT_UTF8String& packet, const Buddy& buddy)\n"));

	// buddy object might be cloned outside, so don't lookup by pointer
	TCPBuddy* ourBuddy = (TCPBuddy*) getBuddy( buddy.getName() );	// use baseclass getBuddy
	UT_return_val_if_fail( ourBuddy, false );
	
	// find the session
	std::map<const TCPBuddy*, Session*>::iterator pos = m_clients.find( ourBuddy );
	if (pos != m_clients.end())
	{
		Session* pSession = pos->second;
		UT_return_val_if_fail(pSession, false);
		
		std::string data;
		_createPacketStream( data, packet );

		pSession->asyncWrite( data.size(), data.c_str() );
		return true;
	}
	else
	{
		UT_DEBUGMSG(("TCPAccountHandler::send: buddy %s doesn't exist, hope you were dragging something ;)\n", buddy.getName().utf8_str()));
	}
	return false;
}
