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

#ifndef __IO_CLIENT_HANDLER__
#define __IO_CLIENT_HANDLER__

#include <boost/bind.hpp>
#include <boost/utility.hpp>
#include <asio.hpp>

#include "IOServiceThread.h" 
#include "Synchronizer.h"
#include "Session.h"
#include "IOHandler.h"

class IOClientHandler : public IOHandler
{
public:
	IOClientHandler(std::string hostname, std::string port)
		: io_service(),
		thread(NULL),
		work(NULL),
		resolver(io_service),
		query(hostname.c_str(), port.c_str())
	{
	}
	
	virtual ~IOClientHandler()
	{
		UT_DEBUGMSG(("IOClientHandler::~IOClientHandler\n"));
	}

	virtual void stop()
	{
		UT_DEBUGMSG(("IOClientHandler::stop()\n"));
		DELETEP(work);
		io_service.stop();
		if (thread)
		{
			thread->join();
			DELETEP(thread);
		}
		UT_DEBUGMSG(("IOClientHandler stopped\n"));
	}

	Session* constructSession(void (*ef)(Session*), TCPAccountHandler& handler)
	{
		return new Session(io_service, ef, handler);
	}

	void connect(Session& session)
	{
		UT_return_if_fail(work == NULL);
		
		work = new asio::io_service::work(io_service);
		thread = new asio::thread(IOServiceThread(io_service));
		
		// TODO: catch exceptions
		asio::ip::tcp::resolver::iterator iterator(resolver.resolve(query));
		session.getSocket().connect(*iterator);
		session.asyncReadHeader();
	}

private:
	asio::io_service					io_service;
	asio::thread*						thread;
	asio::io_service::work*				work;
	asio::ip::tcp::resolver				resolver;
	asio::ip::tcp::resolver::query		query;
};

#endif /* __IO_CLIENT_HANDLER__ */
