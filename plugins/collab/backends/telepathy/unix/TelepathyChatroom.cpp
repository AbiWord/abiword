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

#include "TelepathyUnixAccountHandler.h"
#include "TelepathyChatroom.h"
#include "DTubeBuddy.h"

void TelepathyChatroom::stop()
{
	UT_DEBUGMSG(("TelepathyChatroom::stop()\n"));
	dbus_connection_unref(m_pTube);
}

void TelepathyChatroom::addBuddy(DTubeBuddyPtr pBuddy)
{
	// make sure we don't add this buddy twice
	UT_return_if_fail(pBuddy);
	for (std::vector<DTubeBuddyPtr>::iterator it = m_buddies.begin(); it != m_buddies.end(); it++)
	{
		DTubeBuddyPtr pB = (*it);
		UT_continue_if_fail(pB);
		if (pBuddy->getDBusName() == pB->getDBusName())
		{
			UT_DEBUGMSG(("Not adding buddy with dbus address %s twice\n", pBuddy->getDBusName().utf8_str()));
			return;
		}
	}

	m_buddies.push_back(pBuddy);

	// flush any queued up packets for this buddy
	std::map<std::string, std::vector<std::string> >::iterator pos = m_packet_queue.find(pBuddy->getDBusName().utf8_str());
	if (pos != m_packet_queue.end())
	{
		const std::vector<std::string>& packets = (*pos).second;
		UT_DEBUGMSG(("Flushing %d packets for buddy %s\n", (int)packets.size(), pBuddy->getDBusName().utf8_str()));
		for (UT_uint32 i = 0; i < packets.size(); i++)
			m_pHandler->handleMessage(pBuddy, packets[i]);

		m_packet_queue.erase(pos);
	}
}

DTubeBuddyPtr TelepathyChatroom::getBuddy(TpHandle handle)
{
	for (UT_uint32 i = 0; i < m_buddies.size(); i++)
	{
		DTubeBuddyPtr pBuddy = m_buddies[i];
		UT_continue_if_fail(pBuddy);

		if (pBuddy->getHandle() == handle)
			return pBuddy;
	}

	return DTubeBuddyPtr();
}

DTubeBuddyPtr TelepathyChatroom::getBuddy(UT_UTF8String dbusName)
{
	for (UT_uint32 i = 0; i < m_buddies.size(); i++)
	{
		DTubeBuddyPtr pBuddy = m_buddies[i];
		UT_continue_if_fail(pBuddy);

		if (pBuddy->getDBusName() == dbusName)
			return pBuddy;
	}

	return DTubeBuddyPtr();
}

void TelepathyChatroom::removeBuddy(TpHandle handle)
{
	for (std::vector<DTubeBuddyPtr>::iterator it = m_buddies.begin(); it != m_buddies.end(); it++)
	{
		DTubeBuddyPtr pB = *it;
		UT_continue_if_fail(pB);
		if (pB->getHandle() == handle)
		{
			m_buddies.erase(it);
			return;
		}
	}
	UT_ASSERT_HARMLESS(UT_NOT_REACHED);
}

UT_UTF8String TelepathyChatroom::getDocName()
{
	UT_return_val_if_fail(m_pDoc, "");
	UT_UTF8String docName = m_pDoc->getFilename();
	if (docName == "")
		return "Untitled"; // TODO: fetch the title from the frame somehow (which frame?) - MARCM
	return docName;
}

void TelepathyChatroom::queue(const std::string& dbusName, const std::string& packet)
{
	UT_DEBUGMSG(("Queueing packet for %s\n", dbusName.c_str()));
	m_packet_queue[dbusName].push_back(packet);
}
