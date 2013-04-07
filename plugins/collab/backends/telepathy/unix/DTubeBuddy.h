/* Copyright (C) 2007 One Laptop Per Child
 * Author: Marc Maurer <uwog@uwog.net>
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

#ifndef __DTUBEBUDDY_H__
#define __DTUBEBUDDY_H__

#include <map>
#include <telepathy-glib/telepathy-glib.h>
#include <account/xp/Buddy.h>
#include <account/xp/DocTreeItem.h>
#include <account/xp/AccountHandler.h>
#include "TelepathyChatroom.h"

class DocHandle;

class DTubeBuddy : public Buddy
{
public:
	DTubeBuddy(AccountHandler* handler, TelepathyChatroomPtr pChatRoom, TpHandle handle, const UT_UTF8String dbusName)
		: Buddy(handler),
		m_pChatRoom(pChatRoom),
		m_handle(handle),
		m_sDBusName(dbusName),
		m_pContact(NULL),
		m_pGlobalContact(NULL)
	{
		setVolatile(true);
	}

	virtual ~DTubeBuddy()
	{
		if (m_pContact)
			g_object_unref(m_pContact);
	}

	virtual UT_UTF8String getDescriptor(bool /*include_session_info = false*/) const
	{
		return UT_UTF8String("dtube://") + m_sDBusName;
	}

	virtual UT_UTF8String getDescription() const
	{
		static UT_UTF8String description = m_sDBusName;
		return description;
	}

	virtual const DocTreeItem* getDocTreeItems() const
	{
		UT_ASSERT_HARMLESS(UT_NOT_REACHED);
		return NULL;
	}

	TelepathyChatroomPtr getChatRoom()
	{
		return m_pChatRoom;
	}

	TpHandle getHandle()
	{
		return m_handle;
	}

	const UT_UTF8String& getDBusName()
	{
		return m_sDBusName;
	}

	void setContact(TpContact* pContact)
	{

		g_object_ref(pContact);
		m_pContact = pContact;
	}

	TpContact* getContact()
	{
		return m_pContact;
	}

	void setGlobalContact(TpContact* pGlobalContact)
	{

		g_object_ref(pGlobalContact);
		m_pGlobalContact = pGlobalContact;
	}

	TpContact* getGlobalContact()
	{
		return m_pGlobalContact;
	}

private:
	TelepathyChatroomPtr	m_pChatRoom;
	TpHandle				m_handle;
	UT_UTF8String			m_sDBusName;
	TpContact*				m_pContact;
	TpContact*				m_pGlobalContact;
};

typedef boost::shared_ptr<DTubeBuddy> DTubeBuddyPtr;

#endif /* __DTUBEBUDDY_H__ */
