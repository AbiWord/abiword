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

#include "TelepathyChatroom.h"
#include "DTubeBuddy.h"

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
			UT_DEBUGMSG(("Notting adding buddy with dbus address %s twice\n", pBuddy->getDBusName().utf8_str()));
			return;
		}
	}

	m_buddies.push_back(pBuddy);
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
