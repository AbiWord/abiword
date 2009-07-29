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

#include <session/xp/AbiCollab.h>

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
		case PCT_AccountAddBuddyEvent:
		case PCT_AccountDeleteBuddyEvent:
		case PCT_AccountBuddyOnlineEvent:
		case PCT_AccountBuddyOfflineEvent:
			// FIXME: ick ick ick! (I shouldn't need to explain this)
			_refreshWindow();
			break;
		default:
			// we will ignore the rest
			break;
	}
}

void AP_Dialog_CollaborationShare::_share(AccountHandler* pHandler, const std::vector<BuddyPtr>& vAcl)
{
	UT_DEBUGMSG(("AP_Dialog_CollaborationShare::_share()\n"));

	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_if_fail(pManager);

	// determine which document to share
	XAP_Frame* pFrame = XAP_App::getApp()->getLastFocussedFrame();
	UT_return_if_fail(pFrame);

	PD_Document* pDoc = static_cast<PD_Document *>(pFrame->getCurrentDoc());
	UT_return_if_fail(pDoc);

	AbiCollab* pSession = NULL;
	if (!pManager->isInSession(pDoc))
	{
		UT_DEBUGMSG(("Sharing document...\n"));

		// FIXME: this can cause a race condition: the other side can already be
		// offered the session before we actually started it!
		
		// tell the account handler that we start a new session, so
		// it set up things if needed
		bool b = pHandler->startSession(pDoc, vAcl);
		UT_return_if_fail(b); // TODO: notify the user?
		
		// ... and start the session!
		UT_UTF8String sSessionId("");
		// TODO: we could use/generate a proper descriptor when there is only
		// 1 account where we share this document over
		pSession = pManager->startSession(pDoc, sSessionId, NULL, "");
	}
	else
	{
		pSession = pManager->getSession(pDoc);
	}

	UT_return_if_fail(pSession);
	pManager->updateAcl(pSession, vAcl);
}
