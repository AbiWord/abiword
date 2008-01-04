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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef AP_DIALOG_COLLABORATIONJOIN_H
#define AP_DIALOG_COLLABORATIONJOIN_H

#include "ut_types.h"
#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "xav_View.h"
#include "ut_vector.h"
#include <backends/xp/AccountHandler.h>
#include <backends/xp/Buddy.h>
#include <backends/xp/EventListener.h>

extern pt2Constructor ap_Dialog_CollaborationJoin_Constructor;

class AP_Dialog_CollaborationJoin : public XAP_Dialog_NonPersistent, EventListener
{
public:
	AP_Dialog_CollaborationJoin(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_CollaborationJoin(void);

	virtual void		runModal(XAP_Frame * pFrame) = 0;
	
	void				signal(const Event& event, const Buddy* pSource);

protected:
	void				_eventAddBuddy();
	void				_join(Buddy* pBuddy, DocHandle* pDocHandle);
	void				_disjoin(Buddy* pBuddy, DocHandle* pDocHandle);	

	void				_addBuddy(AccountHandler* pHandler, Buddy* pBuddy);
	void				_refreshAllDocHandlesAsync();
	
	void				_refreshAccounts();
	
	virtual void		_refreshWindow() = 0;
	virtual void		_enableBuddyAddition(bool bEnabled) = 0;
	
};

#endif /* AP_DIALOG_COLLABORATIONJOIN_H */
