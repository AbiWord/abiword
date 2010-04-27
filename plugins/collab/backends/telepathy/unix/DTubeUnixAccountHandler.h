/* Copyright (C) 2007 One Laptop Per Child
 * Author: Marc Maurer <uwog@uwog.net>
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

#ifndef __DTUBEACCOUNTHANDLER__
#define __DTUBEACCOUNTHANDLER__

#include <map>

#include <dbus/dbus.h>
#include <dbus/dbus-glib.h>
#include <dbus/dbus-glib-lowlevel.h>

#include <libempathy/empathy-tube-handler.h>
#include <telepathy-glib/handle.h>
#include <telepathy-glib/channel.h>

#include <account/xp/AccountHandler.h>
#include "DTubeBuddy.h"
#include "TelepathyBuddy.h"

extern AccountHandlerConstructor DTubeAccountHandlerConstructor;

class Session;
class FV_View;

class DTubeAccountHandler : public AccountHandler
{
public:
	static DTubeAccountHandler*				getHandler();
	DTubeAccountHandler(); // TODO: this constructor shouldn't be public
	virtual ~DTubeAccountHandler();

	// housekeeping
	virtual UT_UTF8String					getDescription();
	virtual UT_UTF8String					getDisplayType();
	virtual UT_UTF8String					getStorageType();
	
	// dialog management 
	virtual void							storeProperties();
	virtual void							embedDialogWidgets(void* /*pEmbeddingParent*/)
		{ UT_ASSERT_HARMLESS(UT_NOT_REACHED); }
	virtual void							removeDialogWidgets(void* /*pEmbeddingParent*/)
		{ UT_ASSERT_HARMLESS(UT_NOT_REACHED); }

	// connection management
	virtual ConnectResult					connect();
	virtual bool							disconnect();
	virtual bool							isOnline();
	bool									isLocallyControlled()
		{ return m_bLocallyControlled; }
	
	// user management
	virtual void							getBuddiesAsync();
	void									getBuddiesAsync_cb(guint n_contacts, TpContact * const *contacts); // private, but should be callable from C code
	virtual BuddyPtr						constructBuddy(const PropertyMap& props);
	virtual BuddyPtr						constructBuddy(const std::string& descriptor, BuddyPtr pBuddy);
	virtual bool							recognizeBuddyIdentifier(const std::string& identifier);
	virtual bool							allowsManualBuddies()
		{ return false; }
	virtual void							forceDisconnectBuddy(BuddyPtr pBuddy);
	virtual bool							hasPersistentAccessControl()
		{ return true; }


	// session management
	virtual bool							startSession(PD_Document* pDoc, const std::vector<std::string>& acl, AbiCollab** pSession);
	virtual bool							allowsSessionTakeover()
		{ return false; /* not right now */ }
	
	// packet management
	virtual bool							send(const Packet* pPacket);
	virtual bool							send(const Packet* pPacket, BuddyPtr buddy);
	void									handleMessage(const char* senderDBusAddress, const char* packet_data, int packet_size);
	
	// event management
	void									handleEvent(Session& pSession);

	// signal management
	virtual void							signal(const Event& event, BuddyPtr pSource);

	// tube & buddy management
	bool									joinTube(const UT_UTF8String& tubeDBusAddress);
	bool									joinBuddy(PD_Document* pDoc, TpHandle handle, const UT_UTF8String& buddyDBusAddress);
	bool									disjoinBuddy(FV_View* pView, const UT_UTF8String& buddyDBusAddress);

	// FIXME: should be private but have to be callable from C
	void									acceptTube(TpChannel *tubes_chan, guint id, TpHandle initiator);

private:
	bool									_startSession(PD_Document* pDoc, const UT_UTF8String& tubeDBusAddress);
	bool									_createAndOfferTube(PD_Document* pDoc, const std::vector<TelepathyBuddyPtr>& vBuddies, UT_UTF8String& sTubeAddress);
	TelepathyBuddyPtr						_getBuddy(TpContact* pContact);

	EmpathyTubeHandler*						tube_handler;
	/* TpHandle -> buddyPath (UT_UTF8String) */
	GHashTable*								handle_to_bus_name;
	static DTubeAccountHandler* 			m_pHandler;
	bool									m_bLocallyControlled;
};

#endif /* __DTUBEACCOUNTHANDLER__ */
