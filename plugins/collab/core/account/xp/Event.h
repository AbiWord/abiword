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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef __EVENT_H__
#define __EVENT_H__

#include <vector>
#include <account/xp/Buddy.h>
#include <packet/xp/EventPacket.h>

class Event	: public EventPacket
{
public:
	DECLARE_ABSTRACT_PACKET(Event);

	Event()
		: m_bBroadcast(false)
	{
	}

	virtual ~Event()
	{
	}

	const std::vector<BuddyPtr>&		getRecipients() const
		{ return m_vRecipients; }

	void 								setRecipients(std::vector<BuddyPtr>& vRecipients)
	{
			m_vRecipients = vRecipients;
	}

	void								addRecipient(BuddyPtr pBuddy)
	{
			UT_return_if_fail(pBuddy);
			m_vRecipients.push_back(pBuddy);
	}

	void								setBroadcast(bool bBroadcast)
		{ m_bBroadcast = bBroadcast; }
	bool								isBroadcast() const
		{ return m_bBroadcast; }

private:

	std::vector<BuddyPtr>		   m_vRecipients;
	bool							m_bBroadcast;
};

#endif /* __EVENT_H__ */
