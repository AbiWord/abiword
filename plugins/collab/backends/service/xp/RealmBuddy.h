/* Copyright (C) 2008-2009 AbiSource Corporation B.V.
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

#ifndef __REALM_BUDDY__
#define __REALM_BUDDY__

#ifdef _MSC_VER
#include "msc_stdint.h"
#else
#include <stdint.h>
#endif
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/lexical_cast.hpp>
#include "ut_string_class.h"
#include <core/account/xp/Buddy.h>
#include <core/account/xp/AccountHandler.h>

class RealmConnection;

class RealmBuddy : public Buddy , public boost::enable_shared_from_this<RealmBuddy>
{
public:
	RealmBuddy(AccountHandler* handler, uint64_t _user_id, const std::string& domain_,
					UT_uint8 realm_conn_id, bool _master, boost::shared_ptr<RealmConnection> conn)
		: Buddy(handler),
		m_user_id(_user_id),
		m_domain(domain_),
		m_realm_connection_id(realm_conn_id),
		m_master(_master),
		m_connection(conn)
	{
		setVolatile(true);
	}

	virtual UT_UTF8String getDescriptor(bool include_session_info = false) const
	{
		return UT_UTF8String("acn://") +
					boost::lexical_cast<std::string>(m_user_id).c_str() +
					(include_session_info ? UT_UTF8String(":") + boost::lexical_cast<std::string>((uint32_t)m_realm_connection_id).c_str() : UT_UTF8String("")) +
					UT_UTF8String("@") +
					m_domain.c_str();
	}

	virtual UT_UTF8String getDescription() const
	{
		return getDescriptor();
	}

	virtual const DocTreeItem* getDocTreeItems() const
	{
		return NULL;
	}

	boost::shared_ptr<RealmBuddy> ptr() {
		return shared_from_this();
	}

	std::string domain() {
		return m_domain;
	}

	boost::shared_ptr<RealmConnection> connection() {
		return m_connection;
	}

	uint64_t user_id() const {
		return m_user_id;
	}

	UT_uint8 realm_connection_id() const {
		return m_realm_connection_id;
	}

	bool master() const {
		return m_master;
	}

	void demote() {
		m_master = false;
	}

	void promote() {
		m_master = true;
	}

private:
	uint64_t			m_user_id;
	std::string			m_domain;
	UT_uint8			m_realm_connection_id;
	bool				m_master;
	boost::shared_ptr<RealmConnection>		m_connection;
};

typedef boost::shared_ptr<RealmBuddy> RealmBuddyPtr;

#endif /* __REALM_BUDDY__ */
