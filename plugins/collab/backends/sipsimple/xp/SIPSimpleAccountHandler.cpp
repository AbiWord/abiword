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

#include <gsf/gsf-utils.h>

#include "pd_Document.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_Frame.h"
#include "xap_Strings.h"
#include "xap_App.h"
#include "xap_Dlg_MessageBox.h"

#include "SIPSimpleAccountHandler.h"

#include <account/xp/AccountEvent.h>
#include <account/xp/SessionEvent.h>

#include <session/xp/AbiCollabSessionManager.h>
#include <session/xp/AbiCollab.h>

#include <packet/xp/AbiCollab_Packet.h>
#include <packet/xp/EventPacket.h>

#include <plugin/xp/AbiCollab_Plugin.h>

SIPSimpleAccountHandler::SIPSimpleAccountHandler()
{
}

SIPSimpleAccountHandler::~SIPSimpleAccountHandler()
{
}

UT_UTF8String SIPSimpleAccountHandler::getDescription()
{
	return getProperty("address").c_str();
}

UT_UTF8String SIPSimpleAccountHandler::getDisplayType()
{
	return "Session Initiation Protocol (SIP)";
}

UT_UTF8String SIPSimpleAccountHandler::getStaticStorageType()
{
	return "com.abisource.abiword.abicollab.backend.sipsimple";
}

ConnectResult SIPSimpleAccountHandler::connect()
{
	UT_DEBUGMSG(("SIPSimpleAccountHandler::connect()\n"));
	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);

	return CONNECT_FAILED;
}

bool SIPSimpleAccountHandler::disconnect()
{
	UT_DEBUGMSG(("SIPSimpleAccountHandler::disconnect()\n"));
	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);

	return FALSE;
}

bool SIPSimpleAccountHandler::isOnline()
{
	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);

	return FALSE;
}	

bool SIPSimpleAccountHandler::send(const Packet* pPacket)
{
	UT_return_val_if_fail(pPacket, false);
	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
	
	return true;
}

bool SIPSimpleAccountHandler::send(const Packet* pPacket, BuddyPtr pBuddy)
{
	UT_return_val_if_fail(pPacket, false);
	UT_return_val_if_fail(pBuddy, false);
	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
	
	return true;
}

BuddyPtr SIPSimpleAccountHandler::constructBuddy(const PropertyMap& /*vProps*/)
{
	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
	
	return SIPSimpleBuddyPtr();
}

BuddyPtr SIPSimpleAccountHandler::constructBuddy(const std::string& /*descriptor*/, BuddyPtr /*pBuddy*/)
{
	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
	
	return SIPSimpleBuddyPtr();
}

bool SIPSimpleAccountHandler::recognizeBuddyIdentifier(const std::string& /*identifier*/)
{
	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
	
	return false;
}

