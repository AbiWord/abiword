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

#ifndef __SESSION__
#define __SESSION__

#include <deque>

class TCPAccountHandler;

class Session : public Synchronizer, public boost::noncopyable
{
public:
	Session(asio::io_service& io_service, void (*ef)(Session*), TCPAccountHandler& handler)
		: Synchronizer((void (*)(void*))ef, (void*)this),
		socket(io_service),
		queue_protector(),
		owner(handler)
	{
	}

	virtual ~Session()
	{
		UT_DEBUGMSG(("~Session()\n"));
		socket.close();
	}

	asio::ip::tcp::socket& getSocket()
	{	
		return socket;
	}

	void push(int size, char* data)
	{
		{
			boost::mutex::scoped_lock lock(queue_protector); 
			incoming.push_back( std::pair<int, char*>(size, data) );
		}
		signal();
	}

	/*
		Only called fron the abiword main loop
	*/
	bool pop(int& size, char** data)
	{
		if (incoming.size() == 0)
			return false;
		{
			boost::mutex::scoped_lock lock(queue_protector); 
			std::pair<int, char*> p = incoming.front();
			size = p.first;
			*data = p.second;
			incoming.pop_front();
		}
		return true;
	}

	void asyncReadHeader()
	{
		packet_data = 0; // just to be sure we'll never touch a datablock we might have read before
		asio::async_read(socket, 
			asio::buffer(&packet_size, 4),
			boost::bind(&Session::asyncReadHeaderHandler, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
	}

	void asyncWrite(int size, const char* data)
	{
		// TODO: this is a race condition, mutex this
		bool writeInProgress = outgoing.size() > 0;

		// FIXME: inefficient memory copy
		char* store_data = reinterpret_cast<char*>(malloc(size));
		memcpy(store_data, data, size);
		outgoing.push_back(std::pair<int, char*>(size, store_data));

		if (!writeInProgress)
		{	
			packet_size_write = size;
			packet_data_write = store_data;
		
			UT_DEBUGMSG(("sending datablock of length: %d\n", packet_size_write));
			asio::async_write(socket, 
				asio::buffer(&packet_size_write, 4),
				boost::bind(&Session::asyncWriteHeaderHandler, this, asio::placeholders::error));
		}
	}

	/*
		Only called fron the abiword main loop
	*/
	bool isConnected()
	{
		return socket.is_open();
	}

	TCPAccountHandler& getAccountHandler()
	{
		return owner;
	}

private:
	void asyncReadHeaderHandler(const asio::error_code& error,
		std::size_t bytes_transferred)
	{
		if (!error)
		{
			if (bytes_transferred == 4)
			{
				UT_DEBUGMSG(("going to read datablock of length: %d\n", packet_size));

				// now continue reading the packet data
				packet_data = reinterpret_cast<char*>(malloc(packet_size));
				asio::async_read(socket,
					asio::buffer(packet_data, packet_size),
					boost::bind(&Session::asyncReadHandler, this, asio::placeholders::error, asio::placeholders::bytes_transferred));
			}
			else
				close(); // TODO: should not happen, handle this			
		}
		else
		{
			UT_DEBUGMSG(("asyncReadHeaderHandler generic error\n"));
			close();
		}
	}

	void asyncReadHandler(const asio::error_code& error,
		std::size_t bytes_transferred)
	{
		if (!error)
		{
			if (bytes_transferred == std::size_t(packet_size))
			{
				push(packet_size, packet_data);
				// start over for a new packet
				asyncReadHeader();
			}
			else
			{
				UT_DEBUGMSG(("asyncReadHandler error: wrong number of byes received: %d, expected: %d\n", bytes_transferred, packet_size));
				close();
			}
		}
		else
		{
			UT_DEBUGMSG(("asyncReadHandler generic error\n"));
			close();
		}
		
	}
	
	void asyncWriteHeaderHandler(const asio::error_code& ec)
	{
		UT_DEBUGMSG(("Session::asyncWriteHeaderHandler()\n"));
		if (!ec)
		{
			// write the packet body
			asio::async_write(socket, 
				asio::buffer(packet_data_write, packet_size_write),
				boost::bind(&Session::asyncWriteHandler, this, asio::placeholders::error));
		}
		else
		{
			UT_DEBUGMSG(("asyncWriteHeaderHandler generic error\n"));
			close();
		}
	}

	void asyncWriteHandler(const asio::error_code& ec)
	{
		UT_DEBUGMSG(("Session::asyncWriteHandler()\n"));
		FREEP(packet_data_write);
		if (!ec)
		{
			// TODO: this is a race condition, mutext this
			outgoing.pop_front();
			if (outgoing.size() > 0)
			{
				std::pair<int, char*> p = outgoing.front();
				packet_size_write = p.first;
				packet_data_write = p.second;
				
				UT_DEBUGMSG(("sending datablock of length: %d\n", packet_size_write));

				asio::async_write(socket, 
					asio::buffer(&packet_size_write, 4),
					boost::bind(&Session::asyncWriteHeaderHandler, this, asio::placeholders::error));
			}
		}
		else
		{
			UT_DEBUGMSG(("asyncWriteHandler generic error\n"));
			close();
		}
	}

	void close()
	{
		socket.close();
		UT_DEBUGMSG(("socket closed\n"));
		signal();
	}

	asio::ip::tcp::socket					socket;
	boost::mutex 							queue_protector;
	std::deque< std::pair<int, char*> >		incoming;
	std::deque< std::pair<int, char*> >		outgoing;

	int										packet_size; // state needed for async reads
	char*									packet_data; // state needed for async reads

	int										packet_size_write; // state needed for async writes
	char*									packet_data_write; // state needed for async writes
	
	TCPAccountHandler&						owner;
};

#endif /* __SESSION__ */
