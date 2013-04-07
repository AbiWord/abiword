/* Copyright (C) 2006 Marc Maurer <uwog@uwog.net>
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

#include "ap_Dialog_CollaborationAddAccount.h"


AP_Dialog_CollaborationAddAccount::AP_Dialog_CollaborationAddAccount(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent(pDlgFactory, id, "interface/dialogcollaborationaddaccount"),
	m_pHandler(0)
{
}

AP_Dialog_CollaborationAddAccount::~AP_Dialog_CollaborationAddAccount(void)
{
}

void AP_Dialog_CollaborationAddAccount::_setAccountHandler(AccountHandler* pHandler)
{
	UT_DEBUGMSG(("AP_Dialog_CollaborationAddAccount::_setAccountHandler()\n"));
	
	void* embeddingParent = _getEmbeddingParent();
	UT_return_if_fail(embeddingParent);
		
	if (m_pHandler)
		m_pHandler->removeDialogWidgets(embeddingParent);
	pHandler->embedDialogWidgets(embeddingParent);
	pHandler->loadProperties();
	m_pHandler = pHandler;
}
