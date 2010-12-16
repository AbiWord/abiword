/* Copyright (C) 2010 AbiSource Corporation B.V.
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

#ifndef __TELEPATHY_CHATROOM_H__
#define __TELEPATHY_CHATROOM_H__

#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>
#include <ut_string_class.h>

class TelepathyAccountHandler;
class DTubeBuddy;
typedef boost::shared_ptr<DTubeBuddy> DTubeBuddyPtr;

class TelepathyChatroom : public boost::enable_shared_from_this<TelepathyChatroom>
{
public:
	TelepathyChatroom(TelepathyAccountHandler* pHandler, DBusConnection* pTube, const UT_UTF8String& sSessionId)
		: m_pHandler(pHandler),
		m_pTube(pTube),
		m_sSessionId(sSessionId)
	{
		// TODO: we should prolly ref the tube
		// dbus_connection_unref(m_pTube);
	}

	boost::shared_ptr<TelepathyChatroom> ptr()
	{
		return shared_from_this();
	}

	TelepathyAccountHandler* getHandler()
	{
		return m_pHandler;
	}

	void addBuddy(DTubeBuddyPtr pBuddy)
	{
		m_buddies.push_back(pBuddy);
	}

	const std::vector<DTubeBuddyPtr>& getBuddies()
	{
		return m_buddies;
	}

	DTubeBuddyPtr getBuddy(UT_UTF8String dbusName);

	DBusConnection* getTube()
	{
		return m_pTube;
	}
	
	const UT_UTF8String& getSessionId()
	{
		return m_sSessionId;
	}

private:
	TelepathyAccountHandler*	m_pHandler;
	DBusConnection*				m_pTube;
	UT_UTF8String				m_sSessionId;
	std::vector<DTubeBuddyPtr>	m_buddies;
};

typedef boost::shared_ptr<TelepathyChatroom> TelepathyChatroomPtr;

#endif /* __TELEPATHY_CHATROOM_H__ */
