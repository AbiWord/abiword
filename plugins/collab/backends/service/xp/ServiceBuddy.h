/* Copyright (C) 2006,2007 Marc Maurer <uwog@uwog.net>
 * Copyright (C) 2008 AbiSource Corporation B.V.
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

#ifndef __SERVICEBUDDY__
#define __SERVICEBUDDY__

#include <string>
#ifdef _MSC_VER
#include "msc_stdint.h"
#else
#include <stdint.h>
#endif
#include <boost/lexical_cast.hpp>
#include <boost/shared_ptr.hpp>
#include "ut_string_class.h"
#include <core/account/xp/Buddy.h>
#include <core/account/xp/AccountHandler.h>

enum ServiceBuddyType
{
	SERVICE_USER = 0,
	SERVICE_FRIEND,
	SERVICE_GROUP
};

class ServiceBuddy : public Buddy
{
public:
	ServiceBuddy(AccountHandler* handler, ServiceBuddyType type_, uint64_t user_id, const std::string& name, const std::string& domain)
		: Buddy(handler),
		m_type(type_),
		m_user_id(user_id),
		m_name(name),
		m_domain(domain)
	{
		setVolatile(true);
	}

	virtual UT_UTF8String getDescriptor(bool include_session_info = false) const
	{
		if (include_session_info)
		{
			UT_ASSERT_HARMLESS(UT_NOT_REACHED);
		}
		std::string descr = std::string("acn://")+
								boost::lexical_cast<std::string>(m_user_id) + ":" +
								boost::lexical_cast<std::string>(m_type) + "@" +
								m_domain;
		return descr.c_str();
	}

	virtual UT_UTF8String getDescription() const
		{ return m_name.c_str(); }

	ServiceBuddyType getType() const
		{ return m_type; }

	uint64_t getUserId() const
		{ return m_user_id; }

	const std::string& getName() const
		{ return m_name; }

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
	ServiceBuddyType	m_type;
	uint64_t			m_user_id;
	std::string			m_name;
	std::string			m_domain;
};

typedef boost::shared_ptr<ServiceBuddy> ServiceBuddyPtr;

#endif /* __SERVICEBUDDY__ */
