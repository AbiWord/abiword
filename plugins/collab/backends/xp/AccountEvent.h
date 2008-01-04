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

#ifndef __ACCOUNTEVENT_H__
#define __ACCOUNTEVENT_H__

#include "Event.h"

ABI_EXPORT class AccountNewEvent : public Event
{
public:
	DECLARE_PACKET(AccountNewEvent);
};

ABI_EXPORT class AccountOnlineEvent : public Event
{
public:
	DECLARE_PACKET(AccountOnlineEvent);
};

ABI_EXPORT class AccountOfflineEvent : public Event
{
public:
	DECLARE_PACKET(AccountOfflineEvent);
};

ABI_EXPORT class AccountAddBuddyEvent : public Event
{
public:
	DECLARE_PACKET(AccountAddBuddyEvent);
};

ABI_EXPORT class AccountDeleteBuddyEvent : public Event
{
public:
	DECLARE_PACKET(AccountDeleteBuddyEvent);
};

ABI_EXPORT class AccountBuddyOnlineEvent : public Event
{
public:
	DECLARE_PACKET(AccountBuddyOnlineEvent);
};

ABI_EXPORT class AccountBuddyOfflineEvent : public Event
{
public:
	DECLARE_PACKET(AccountBuddyOfflineEvent);
};

ABI_EXPORT class AccountAddBuddyRequestEvent : public Event
{
public:
	DECLARE_PACKET(AccountAddBuddyRequestEvent);
};

ABI_EXPORT class AccountBuddyAddDocumentEvent : public Event
{
public:
	DECLARE_PACKET(AccountBuddyAddDocumentEvent);
	AccountBuddyAddDocumentEvent()
		: m_pDocHandle(NULL)
		{}
	AccountBuddyAddDocumentEvent(DocHandle* pDocHandle)
		: m_pDocHandle(pDocHandle)
		{}
	DocHandle*					getDocHandle() const
		{ return m_pDocHandle; }
	
private:
	DocHandle* 					m_pDocHandle;	
};

#endif /* __ACCOUNTEVENT_H__ */
