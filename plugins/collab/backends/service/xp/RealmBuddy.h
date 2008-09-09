/* Copyright (C) 2008 AbiSource Corporation B.V.
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

#ifndef __REALM_BUDDY__
#define __REALM_BUDDY__

#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/lexical_cast.hpp>
#include "ut_string_class.h"
#include <backends/xp/Buddy.h>
#include <backends/xp/AccountHandler.h>

class RealmConnection;

class RealmBuddy : public Buddy , public boost::enable_shared_from_this<RealmBuddy>
{
public:
	RealmBuddy(AccountHandler* handler, const std::string& email, const std::string& domain,
					UT_uint8 realm_connection_id, bool master, boost::shared_ptr<RealmConnection> connection)
		: Buddy(handler),
		m_email(email),
		m_domain(domain),
		m_realm_connection_id(realm_connection_id),
		m_master(master),
		m_connection(connection)
	{
		setVolatile(true);
	}
	
	virtual const UT_UTF8String& getDescriptor() const
	{
		static UT_UTF8String descriptor = UT_UTF8String("acn://") + m_email.c_str() + UT_UTF8String("@") + m_domain.c_str();
		return descriptor;
	}
	
	virtual UT_UTF8String getDescription() const
	{
		// shouldn't be called from anywhere; instead, ServiceBuddy's are shown in the interface
		UT_ASSERT_HARMLESS(UT_NOT_REACHED);
		return "";
	}
	
	virtual const DocTreeItem* getDocTreeItems() const
	{
		return NULL;
	}

	boost::shared_ptr<RealmBuddy> ptr() {
		return shared_from_this();
	}

	boost::shared_ptr<RealmConnection> connection() {
		return m_connection;
	}

	UT_uint8 realm_connection_id() const {
		return m_realm_connection_id;
	}
	
	bool master() const {
		return m_master;
	}
	
	void demote() {
		UT_ASSERT_HARMLESS(m_master);
		m_master = false;
	}
	
private:
	std::string			m_email;
	std::string			m_domain;
	UT_uint8			m_realm_connection_id;
	bool				m_master;
	boost::shared_ptr<RealmConnection>		m_connection;
};

typedef boost::shared_ptr<RealmBuddy> RealmBuddyPtr;

#endif /* __REALM_BUDDY__ */
