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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
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
	: XAP_Dialog_NonPersistent(pDlgFactory, id, "interface/dialogcollaborationshare"),
	m_pAccount(NULL)
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

AbiCollab* AP_Dialog_CollaborationShare::_getActiveSession()
{
	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_val_if_fail(pManager, NULL);

	XAP_Frame* pFrame = XAP_App::getApp()->getLastFocussedFrame();
	UT_return_val_if_fail(pFrame, NULL);

	PD_Document* pDoc = static_cast<PD_Document *>(pFrame->getCurrentDoc());
	UT_return_val_if_fail(pDoc, NULL);

	if (!pManager->isInSession(pDoc))
		return NULL;

	return pManager->getSession(pDoc);
}

// If the current document is already in session, and has buddies in its
// access control list, then the document can't be shared with buddies from
// other accounts. This function returns NULL if no buddies are in the
// current session's ACL, or the document is not shared yet. Otherwise, it
// returns the (only) handler that is allowed to share the document.
AccountHandler* AP_Dialog_CollaborationShare::_getShareableAccountHandler()
{
	AbiCollab* pSession = _getActiveSession();
	if (!pSession)
		return NULL;

	return pSession->getAclAccount();
}

std::vector<std::string> AP_Dialog_CollaborationShare::_getSessionACL()
{
	AbiCollab* pSession = _getActiveSession();
	if (!pSession)
		return std::vector<std::string>();

	AccountHandler* pAclAccount = pSession->getAclAccount();
	UT_return_val_if_fail(pAclAccount, std::vector<std::string>());

	std::vector<std::string> vAcl = pSession->getAcl();
	if (!pAclAccount->getAcl(pSession, vAcl))
	{
		UT_return_val_if_fail(false, vAcl); // TODO; this return is probably not correct
	}
	return vAcl;
}

bool AP_Dialog_CollaborationShare::_inAcl(const std::vector<std::string>& vAcl, BuddyPtr pBuddy)
{
	UT_return_val_if_fail(pBuddy, false);
	
	for (UT_uint32 i = 0; i < vAcl.size(); i++)
	{
		if (vAcl[i] == pBuddy->getDescriptor(false).utf8_str())
			return true;
	}

	return false;
}

bool AP_Dialog_CollaborationShare::_populateShareState(BuddyPtr pBuddy)
{
	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_val_if_fail(pManager, false);

	PD_Document *pDoc = static_cast<PD_Document*>(XAP_App::getApp()->getLastFocussedFrame()->getCurrentDoc());
	UT_return_val_if_fail(pDoc, false);

	if (!pManager->isInSession(pDoc))
	{
		AccountHandler* pHandler = pBuddy->getHandler();
		UT_return_val_if_fail(pHandler, false);
	
		return pHandler->defaultShareState(pBuddy);
	}

	return _inAcl(m_vAcl, pBuddy);
}

void AP_Dialog_CollaborationShare::eventAccountChanged()
{
	UT_DEBUGMSG(("AP_Dialog_CollaborationShare::eventAccountChanged()\n"));
	AccountHandler* pHandler = _getActiveAccountHandler();
	UT_return_if_fail(pHandler);
	
	UT_DEBUGMSG(("Changed account handler to type: %s\n", pHandler->getDisplayType().utf8_str()));
	PD_Document *pDoc = static_cast<PD_Document*>(XAP_App::getApp()->getLastFocussedFrame()->getCurrentDoc());
	UT_return_if_fail(pDoc);
	_setAccountHint(pHandler->getShareHint(pDoc));	
	_populateBuddyModel(true);
}

void AP_Dialog_CollaborationShare::share(AccountHandler* pAccount, const std::vector<std::string>& vAcl)
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
		
		// Tell the account handler that we start a new session, so
		// it set up things if needed. This call may just setup some stuff 
		// for a new session, or it might actually start a new session.
		bool b = pAccount->startSession(pDoc, m_vAcl, &pSession);
		if (!b)
		{
			XAP_App::getApp()->getLastFocussedFrame()->showMessageBox(
						"There was an error sharing this document!", 
						XAP_Dialog_MessageBox::b_O, XAP_Dialog_MessageBox::a_OK);
			return;
		}
		
		// start the session ourselves when the account handler did not...
		if (!pSession)
		{
			// ... and start the session!
			std::string sSessionId("");
			// TODO: we could use/generate a proper descriptor when there is only
			// 1 account where we share this document over
			pSession = pManager->startSession(pDoc, sSessionId, pAccount, true, NULL, "");
		}
	}
	else
	{
		pSession = pManager->getSession(pDoc);
	}

	UT_return_if_fail(pSession);
	pManager->updateAcl(pSession, pAccount, vAcl);
}
