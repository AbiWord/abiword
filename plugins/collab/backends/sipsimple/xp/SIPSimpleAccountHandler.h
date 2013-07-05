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

#ifndef __SIPSIMPLEACCOUNTHANDLER__
#define __SIPSIMPLEACCOUNTHANDLER__

#include <string>

#include <account/xp/AccountHandler.h>
#include "ut_string_class.h"
#include "ut_types.h"
#include "SIPSimpleBuddy.h"

extern AccountHandlerConstructor SIPSimpleAccountHandlerConstructor;

class SIPSimpleAccountHandler : public AccountHandler
{
public:
	SIPSimpleAccountHandler();
	virtual ~SIPSimpleAccountHandler(void);

	// housekeeping
	static UT_UTF8String	getStaticStorageType();
	virtual UT_UTF8String	getStorageType()
		{ return getStaticStorageType(); }
	virtual UT_UTF8String	getDescription();
	virtual UT_UTF8String	getDisplayType();

	// connection management
	virtual ConnectResult	connect();
	virtual bool			disconnect(void);
	virtual bool			isOnline();

	// dialog management
	virtual void			embedDialogWidgets(void* pEmbeddingParent) = 0;
	virtual void			removeDialogWidgets(void* pEmbeddingParent) = 0;
	virtual void			storeProperties() = 0;

	// user management
	virtual BuddyPtr		constructBuddy(const PropertyMap& vProps);
	virtual BuddyPtr		constructBuddy(const std::string& descriptor, BuddyPtr pBuddy);
	virtual bool			recognizeBuddyIdentifier(const std::string& identifier);
	virtual bool			allowsManualBuddies()
		{ return true; }

	virtual void			forceDisconnectBuddy(BuddyPtr) { /* TODO: implement me? */ }
	virtual bool			hasPersistentAccessControl()
		{ return true; }

	// session management
	virtual bool			allowsSessionTakeover()
		{ return false; } // no technical reason not to allow this; we just didn't implement session takeover for this backend yet

		// packet management
	virtual bool			send(const Packet* pPacket);
	virtual bool 			send(const Packet* pPacket, BuddyPtr pBuddy);

private:
};

#endif /* __SIPSIMPLEACCOUNTHANDLER__ */
