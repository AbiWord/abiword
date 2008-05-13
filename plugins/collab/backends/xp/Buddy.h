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

#ifndef __BUDDY_H__
#define __BUDDY_H__

#include <vector>
#include "ut_string_class.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "DocHandle.h"
#include "DocTreeItem.h"

class AccountHandler;

using std::vector;

 class Buddy 
{
public:
	Buddy(AccountHandler* handler, const UT_UTF8String& name)
		: m_handler(handler),
		m_name(name),
		m_volatile(false)
	{
	}
	virtual ~Buddy() {}
	
	/*
	 * Buddy management
	 */
	virtual Buddy* clone() const = 0;

	/*
	 * User management
	 */
	virtual const UT_UTF8String&	getName() const
		{ return m_name; }
	virtual UT_UTF8String			getDescription() const = 0;
	AccountHandler*					getHandler() const
		{ return m_handler; }
	void							setVolatile(bool _volatile)
		{ m_volatile = _volatile; }
	bool							isVolatile()
		{ return m_volatile; }
	
	/*
	 * Document management
	 */
	virtual const DocTreeItem*		getDocTreeItems() const = 0;
	void							addDocHandle(DocHandle* pDocHandle);
	const vector<DocHandle*>&		getDocHandles() const
		{ return m_docHandles; }
	DocHandle*						getDocHandle(const UT_UTF8String& sSessionId) const
	{
		for (std::vector<DocHandle*>::const_iterator cit = m_docHandles.begin(); cit != m_docHandles.end(); cit++)
		{
			DocHandle* pHandle = *cit;
			if (pHandle->getSessionId() == sSessionId)
				return pHandle;
		}
		return NULL;
	}
	void							destroyDocHandle(const UT_UTF8String& sSessionId)
	{
		UT_DEBUGMSG(("Request to destroy dochandle %s\n", sSessionId.utf8_str()));
		for (std::vector<DocHandle*>::iterator it = m_docHandles.begin(); it != m_docHandles.end(); it++)
		{
			DocHandle* pCurHandle = *it;
			UT_DEBUGMSG(("Comparing with dochandle: %s\n", pCurHandle->getSessionId().utf8_str()));
			if (pCurHandle && pCurHandle->getSessionId() == sSessionId)
			{
				UT_DEBUGMSG(("Destroying document handle: %s\n", pCurHandle->getSessionId().utf8_str()));
				m_docHandles.erase(it);
				DELETEP(pCurHandle);
				return;
			}
		}
		UT_ASSERT(UT_NOT_REACHED);
	}	
	
private:
	AccountHandler*				m_handler;
	UT_UTF8String				m_name;
	vector<DocHandle*>			m_docHandles;
	bool						m_volatile;
};

#endif /* BUDDY_H */
