/* Copyright (C) 2009 AbiSource Corporation B.V.
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

#include <stdlib.h>
#include <stdio.h>

#include "xap_App.h"
#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include <session/xp/AbiCollabSessionManager.h>

#include "ap_Dialog_CollaborationShare.h"
#include "ap_Dialog_CollaborationAddBuddy.h"

#include <account/xp/AccountHandler.h>
#include <account/xp/AccountEvent.h>

// TODO : remove this!!!
#include <backends/xmpp/xp/XMPPBuddy.h>

AP_Dialog_CollaborationShare::AP_Dialog_CollaborationShare(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent(pDlgFactory, id, "interface/dialogcollaborationshare")
{
	AbiCollabSessionManager::getManager()->registerEventListener(this);
}

AP_Dialog_CollaborationShare::~AP_Dialog_CollaborationShare(void)
{
	AbiCollabSessionManager::getManager()->unregisterEventListener(this);
}

void AP_Dialog_CollaborationShare::signal(const Event& event, BuddyPtr /*pSource*/)
{
	UT_DEBUGMSG(("AP_Dialog_CollaborationShare::signal()\n"));
	switch (event.getClassType())
	{
		case PCT_AccountNewEvent:
		// case Event::AccountDelete:
			//_refreshAccounts();
			break;
		case PCT_AccountAddBuddyEvent:
		case PCT_AccountDeleteBuddyEvent:
		case PCT_AccountBuddyOnlineEvent:
		case PCT_AccountBuddyOfflineEvent:
			// FIXME: ick ick ick! (I shouldn't need to explain this)
			//_refreshWindow();
			break;
		default:
			// we will ignore the rest
			break;
	}
}
