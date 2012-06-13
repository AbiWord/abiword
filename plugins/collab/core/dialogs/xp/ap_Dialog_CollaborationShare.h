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

#ifndef AP_DIALOG_COLLABORATIONSHARE_H
#define AP_DIALOG_COLLABORATIONSHARE_H

#include "ut_types.h"
#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "xav_View.h"
#include "ut_vector.h"
#include <account/xp/AccountHandler.h>
#include <account/xp/Buddy.h>
#include <account/xp/EventListener.h>

// We can't store shared pointers in the GtkListStore or the Win32 ListView control, 
// so we use a hack to store it anyway: store the shared pointer in a C struct called 
// BuddyPtrWrapper. This obviously defies the whole reason of shared pointers, but we 
// have no choice with these C APIs ;/
struct BuddyPtrWrapper
{
public:
	BuddyPtrWrapper(BuddyPtr pBuddy) : m_pBuddy(pBuddy) {}
	BuddyPtr getBuddy() { return m_pBuddy; }
private:
	BuddyPtr m_pBuddy;
};

extern pt2Constructor ap_Dialog_CollaborationShare_Constructor;

class AP_Dialog_CollaborationShare: public XAP_Dialog_NonPersistent, EventListener
{
public:
	AP_Dialog_CollaborationShare(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_CollaborationShare(void);

	virtual void		runModal(XAP_Frame * pFrame) = 0;
	
	void				signal(const Event& event, BuddyPtr pSource);

	typedef enum { a_OK, a_CANCEL } tAnswer;

	AP_Dialog_CollaborationShare::tAnswer	getAnswer(void) const
		{ return m_answer; }

	AccountHandler* getAccount()
		{ return m_pAccount; }

	const std::vector<std::string>& getAcl()
		{ return m_vAcl; }

	void						eventAccountChanged();
	void						share(AccountHandler* pAccount, const std::vector<std::string>& vAcl);

protected:
	AbiCollab*					_getActiveSession();
	AccountHandler*				_getShareableAccountHandler();
	std::vector<std::string>	_getSessionACL();
	
	bool						_populateShareState(BuddyPtr pBuddy);
	virtual void				_refreshWindow() = 0;
	virtual void				_populateBuddyModel(bool refresh) = 0;
	virtual AccountHandler*		_getActiveAccountHandler() = 0;
	virtual void				_setAccountHint(const UT_UTF8String& sHint) = 0;
	
	AP_Dialog_CollaborationShare::tAnswer m_answer;
	AccountHandler*				m_pAccount;
	std::vector<std::string>	m_vAcl;
	
private:
	bool						_inAcl(const std::vector<std::string>& vAcl, BuddyPtr pBuddy);
};

#endif /* AP_DIALOG_COLLABORATIONSHARE_H */
