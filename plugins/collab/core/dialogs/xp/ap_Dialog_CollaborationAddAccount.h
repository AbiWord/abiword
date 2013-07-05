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

#ifndef AP_DIALOG_COLLABORATIONADDACCOUNT_H
#define AP_DIALOG_COLLABORATIONADDACCOUNT_H

#include "ut_types.h"
#include "xap_Frame.h"
#include "xap_Dialog.h"
#include "xav_View.h"
#include "ut_vector.h"

class AccountHandler;

extern pt2Constructor ap_Dialog_CollaborationAddAccount_Constructor;

class AP_Dialog_CollaborationAddAccount : public XAP_Dialog_NonPersistent
{
public:
	AP_Dialog_CollaborationAddAccount(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	virtual ~AP_Dialog_CollaborationAddAccount(void);

	virtual void							runModal(XAP_Frame * pFrame) = 0;

	AccountHandler*							getAccountHandler() const
		{ return m_pHandler; }

	typedef enum { a_OK, a_CANCEL } tAnswer;

	AP_Dialog_CollaborationAddAccount::tAnswer	getAnswer(void) const
		{ return m_answer; }

protected:
	void									_setAccountHandler(AccountHandler* pHandler);
	virtual void*							_getEmbeddingParent() = 0;

	AP_Dialog_CollaborationAddAccount::tAnswer m_answer;

private:
	AccountHandler*							m_pHandler;
};

#endif /* AP_DIALOG_COLLABORATIONADDACCOUNT_H */
