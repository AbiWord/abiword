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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef __TELEPATHY_CHATROOM_H__
#define __TELEPATHY_CHATROOM_H__

#include <map>
#include <vector>

#include <telepathy-glib/telepathy-glib.h>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

#include <ut_string_class.h>
#include "TelepathyBuddy.h"

class TelepathyAccountHandler;
class DTubeBuddy;
typedef boost::shared_ptr<DTubeBuddy> DTubeBuddyPtr;

class TelepathyChatroom : public boost::enable_shared_from_this<TelepathyChatroom>
{
public:
	TelepathyChatroom(TelepathyAccountHandler* pHandler, TpChannel* pChannel,
			PD_Document* pDoc, const UT_UTF8String& sSessionId);

	bool running()
		{ return m_pChannel != NULL; }

	bool tubeOffered()
		{ return m_pTube != NULL; }

	void stop();

	void finalize();

	void setChannel(TpChannel* pChannel);

	boost::shared_ptr<TelepathyChatroom> ptr()
		{ return shared_from_this(); }

	TelepathyAccountHandler* getHandler()
		{ return m_pHandler; }

	void addBuddy(DTubeBuddyPtr pBuddy);

	const std::vector<DTubeBuddyPtr>& getBuddies()
		{ return m_buddies; }

	DTubeBuddyPtr getBuddy(TpHandle handle);

	DTubeBuddyPtr getBuddy(UT_UTF8String dbusName);

	void removeBuddy(TpHandle handle);

	DBusConnection* getTube()
		{ return m_pTube; }

	const UT_UTF8String& getSessionId()
		{ return m_sSessionId; }

	void setSessionId(const UT_UTF8String& sSessionId)
		{ m_sSessionId = sSessionId; }

	std::string getDocName();

	void queue(const std::string& dbusName, const std::string& packet);

	void acceptTube(const char* address);

	void queueInvite(TelepathyBuddyPtr pBuddy);

	bool offerTube();

	void finalizeOfferTube(DBusConnection* pTube);

	bool isController(DTubeBuddyPtr pBuddy);

	bool isLocallyControlled();

private:
	TelepathyAccountHandler*	m_pHandler;
	TpChannel*					m_pChannel;
	PD_Document* 				m_pDoc;
	DBusConnection*				m_pTube;
	UT_UTF8String				m_sSessionId;
	std::vector<DTubeBuddyPtr>	m_buddies;
	std::vector<TelepathyBuddyPtr> m_pending_invitees;
	std::map<std::string, std::vector<std::string> > m_packet_queue;
	bool						m_bShuttingDown;

	std::vector<std::string> m_offered_tubes; // list of TelepathyBuddy descriptors
};

typedef boost::shared_ptr<TelepathyChatroom> TelepathyChatroomPtr;

#endif /* __TELEPATHY_CHATROOM_H__ */
