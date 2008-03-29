/* Copyright (C) 2007 by Marc Maurer <uwog@uwog.net>
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

#ifndef __IO_SERVER_HANDLER__
#define __IO_SERVER_HANDLER__

#include "ut_debugmsg.h"

#include <boost/bind.hpp>
#include <boost/utility.hpp>
#include <asio.hpp>

#include "IOServiceThread.h" 
#include "Synchronizer.h"
#include "Session.h"
#include "IOHandler.h"

class TCPAccountHandler;

class IOServerHandler : public IOHandler
{
public:
	IOServerHandler(int port, void (*af)(IOServerHandler*), TCPAccountHandler& handler)
	:	accept_synchronizer((void (*)(void*))af, (void*)this),
		io_service(),
		work(NULL),
		endpoint(asio::ip::tcp::v4(), port),
		m_pAcceptor(NULL),
		iot(io_service),
		m_handler(handler)
	{
		work = new asio::io_service::work(io_service);
		m_pAcceptor = new asio::ip::tcp::acceptor(io_service, endpoint);
		asio::thread thread(iot);
	}
	
	virtual ~IOServerHandler()
	{
		UT_DEBUGMSG(("IOServerHandler::~IOServerHandler()\n"));
	}

	virtual void stop()
	{
		if (m_pAcceptor)
		{
			m_pAcceptor->close();
			DELETEP(m_pAcceptor);
		}
		DELETEP(work);
		io_service.stop();
	}

	void run(Session& newSession)
	{
		asyncAccept(newSession);
	}

	void asyncAccept(Session& newSession)
	{
		m_pAcceptor->async_accept(newSession.getSocket(),
			boost::bind(&IOServerHandler::handleAsyncAccept,
				this, asio::placeholders::error,
				boost::ref(newSession)));
	}

	Session* constructSession(void (*ef)(Session*), TCPAccountHandler& handler)
	{
		return new Session(io_service, ef, handler);
	}

	/*
		Only called fron the abiword main loop
	*/
	void handleAccept(Session& newSession)
	{
		accept_synchronizer.consume();
		asyncAccept(newSession);
	}

	TCPAccountHandler& getAccountHandler()
	{
		return m_handler;
	}

private:
	void handleAsyncAccept(const asio::error_code& ec, Session& session)
	{
		if (!ec)
		{
			accept_synchronizer.signal();
			session.asyncReadHeader();
		}
	}

	Synchronizer				accept_synchronizer;
	asio::io_service			io_service;
	asio::io_service::work*		work;
	asio::ip::tcp::endpoint		endpoint;
	asio::ip::tcp::acceptor*	m_pAcceptor;
	
	IOServiceThread				iot;

	TCPAccountHandler&			m_handler;
};

#endif /* __IO_SERVER_HANDLER__ */
