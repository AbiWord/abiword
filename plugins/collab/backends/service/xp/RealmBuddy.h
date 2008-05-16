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
	RealmBuddy(AccountHandler* handler, UT_uint8 realm_connection_id, RealmConnection& connection)
		: Buddy(handler, boost::lexical_cast<std::string>(realm_connection_id).c_str()),
		m_realm_connection_id(realm_connection_id),
		m_connection(connection)
	{
		setVolatile(true);
	}
	
	virtual Buddy* clone() const { return new RealmBuddy( *this ); }
	
	virtual UT_UTF8String		getDescription() const
	{
		return getName();
	}
	
	virtual const DocTreeItem* getDocTreeItems() const
	{
		return NULL;
	}

	boost::shared_ptr<RealmBuddy> ptr() {
		return shared_from_this();
	}

	boost::shared_ptr<const RealmBuddy> ptr() const {
		return shared_from_this();
	}

	RealmConnection& connection() const {
		return m_connection;
	}

	UT_uint8 realm_connection_id() const {
		return m_realm_connection_id;
	}
	
private:
	UT_uint8 m_realm_connection_id;
	RealmConnection& m_connection;
};

#endif /* __REALM_BUDDY__ */
