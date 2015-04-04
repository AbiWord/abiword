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

#ifndef __SIPSIMPLEBUDDY_H__
#define __SIPSIMPLEBUDDY_H__

#include <map>
#include <string>
#include <boost/shared_ptr.hpp>
#include "ut_string_class.h"
#include <core/account/xp/Buddy.h>
#include <core/account/xp/DocTreeItem.h>
#include <core/account/xp/AccountHandler.h>

class DocHandle;

class SIPSimpleBuddy : public Buddy
{
public:
	SIPSimpleBuddy(AccountHandler* handler, const std::string& address)
		: Buddy(handler),
		m_address(address)
	{
	}

	virtual UT_UTF8String getDescriptor(bool /*include_session_info = false*/) const
	{
		return UT_UTF8String("sipsimple://") + m_address.c_str();
	}

	virtual UT_UTF8String		getDescription() const
		{ return m_address.c_str(); }

	virtual const std::string& getAddress() const {
		return m_address;
	}

	virtual const DocTreeItem* getDocTreeItems() const
	{
		const std::vector<DocHandle*>& docHandles = getDocHandles();
		DocTreeItem* first = 0;
		DocTreeItem* prev = 0;
		for (std::vector<DocHandle*>::const_iterator pos = docHandles.begin(); pos != docHandles.end(); pos++)
		{
			DocTreeItem* item = new DocTreeItem();
			item->m_type = DOCTREEITEM_TYPE_DOCUMENT;
			item->m_docHandle = *pos;
			item->m_child = 0;
			item->m_next = 0;

			if (!first)
				first = item;
			if (prev)
				prev->m_next = item;
			prev = item;
		}
		return first;
	}

private:
	std::string m_address;
};

typedef boost::shared_ptr<SIPSimpleBuddy> SIPSimpleBuddyPtr;

#endif /* SIPSIMPLEBUDDY_H */
