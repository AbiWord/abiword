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

#include <stdlib.h>
#include <stdio.h>

#include "xap_App.h"
#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include <session/xp/AbiCollabSessionManager.h>

#include "ap_Dialog_CollaborationEditAccount.h"


AP_Dialog_CollaborationEditAccount::AP_Dialog_CollaborationEditAccount(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent(pDlgFactory, id, "interface/dialogcollaborationeditaccount"),
	m_pHandler(NULL)
{
}

AP_Dialog_CollaborationEditAccount::~AP_Dialog_CollaborationEditAccount(void)
{
}

void AP_Dialog_CollaborationEditAccount::setAccountHandler(AccountHandler* pHandler)
{
	UT_DEBUGMSG(("AP_Dialog_CollaborationEditAccount::setAccountHandler()\n"));
	m_pHandler = pHandler;
}
