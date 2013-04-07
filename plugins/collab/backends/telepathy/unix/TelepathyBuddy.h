/* Copyright (C) 2009 Marc Maurer <uwog@uwog.net>
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

#ifndef __TELEPATHY_BUDDY_H__
#define __TELEPATHY_BUDDY_H__

#include <map>
#include <account/xp/Buddy.h>
#include <account/xp/DocTreeItem.h>
#include <account/xp/AccountHandler.h>

#include <telepathy-glib/contact.h>

class DocHandle;
class TelepathyBuddy;

typedef boost::shared_ptr<TelepathyBuddy> TelepathyBuddyPtr;

class TelepathyBuddy : public Buddy
{
public:
	TelepathyBuddy(AccountHandler* handler, TpContact* pContact)
		: Buddy(handler),
		m_pContact(pContact)
	{
		setVolatile(true);
		g_object_ref(m_pContact);
	}

	virtual ~TelepathyBuddy()
	{
		g_object_unref(m_pContact);
	}

	virtual UT_UTF8String getDescriptor(bool /*include_session_info = false*/) const
	{
		return UT_UTF8String("telepathy://") + tp_contact_get_identifier (m_pContact);
	}

	virtual UT_UTF8String getDescription() const
	{
		UT_UTF8String description = tp_contact_get_identifier (m_pContact);
		return description;
	}

	virtual const DocTreeItem* getDocTreeItems() const
	{
		UT_ASSERT_HARMLESS(UT_NOT_REACHED);
		return NULL;
	}

	TpContact* getContact()
	{
		return m_pContact;
	}

	bool equals(TelepathyBuddyPtr pBuddy)
	{
		UT_return_val_if_fail(pBuddy, false);
		TpContact* pContact = pBuddy->getContact();
		return strcmp(tp_contact_get_identifier(pContact), tp_contact_get_identifier (m_pContact)) == 0;
	}

private:

	TpContact*			m_pContact;
};

#endif /* __TELEPATHY_BUDDY_H__ */
