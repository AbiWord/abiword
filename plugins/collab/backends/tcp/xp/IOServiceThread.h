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

#ifndef __IO_SERVICE_THREAD__
#define __IO_SERVICE_THREAD__

// TODO: drop this class
class IOServiceThread
{
public:
	IOServiceThread(asio::io_service& io_service)
		: io_service_(io_service)
	{
	}

	void operator()()
	{
		UT_DEBUGMSG(("io_service run start\n"));
		io_service_.run();
		UT_DEBUGMSG(("io_service run exit\n"));
	}

private:
	asio::io_service& io_service_;
};

#endif /* __IO_SERVICE_THREAD__ */
