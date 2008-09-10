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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */

#ifndef __TCPBUDDY_H__
#define __TCPBUDDY_H__

#include <map>
#include <string>
#include <boost/shared_ptr.hpp>
#include "ut_string_class.h"
#include <backends/xp/Buddy.h>
#include <backends/xp/DocTreeItem.h>
#include <backends/xp/AccountHandler.h>

class DocHandle;

 class TCPBuddy : public Buddy
{
public:
	TCPBuddy(AccountHandler* handler, const std::string& server, const std::string& port)
		: Buddy(handler),
		m_server(server),
		m_port(port)
	{
		setVolatile(true);
	}
	
	virtual const UT_UTF8String& getDescriptor(bool include_session_info = false) const
	{
		static UT_UTF8String descriptor = UT_UTF8String("tcp://") + m_server.c_str() + UT_UTF8String(":") + m_port.c_str();
		return descriptor;
	}
	
	virtual UT_UTF8String getDescription() const
	{
		static UT_UTF8String description = m_server.c_str() + UT_UTF8String(":") + m_port.c_str();
		return description;
	}

	const std::string& getServer() const
		{ return m_server; }

	const std::string& getPort() const
		{ return m_port; }
		
	virtual const DocTreeItem* getDocTreeItems() const
	{
		const vector<DocHandle*>& docHandles = getDocHandles();
		DocTreeItem* first = 0;
		DocTreeItem* prev = 0;		
		for (vector<DocHandle*>::const_iterator pos = docHandles.begin(); pos != docHandles.end(); pos++)
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
	std::string m_server;
	std::string m_port;
};

typedef boost::shared_ptr<TCPBuddy> TCPBuddyPtr;

#endif /* TCPBUDDY_H */
