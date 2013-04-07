/* AbiCollab - Code to enable the modification of remote documents.
 * Copyright (C) 2007 by Ryan Pavlik <abiryan@ryand.net>
 * Copyright (C) 2006 by Marc Maurer <uwog@uwog.net>
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

#ifndef AP_WIN32DIALOG_COLLABORATIONADDACCOUNT_H
#define AP_WIN32DIALOG_COLLABORATIONADDACCOUNT_H

#include <vector>

#include "xap_Win32DialogHelper.h"
#include "ut_assert.h"

#include <dialogs/xp/ap_Dialog_CollaborationAddAccount.h>
#include "ap_Win32Res_DlgCollaborationAddAccount.rc2"

class XAP_Frame;

class AP_Win32Dialog_CollaborationAddAccount : public AP_Dialog_CollaborationAddAccount
{
public:
	AP_Win32Dialog_CollaborationAddAccount(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	static XAP_Dialog * 		static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id);

	void						runModal(XAP_Frame * pFrame);
	static BOOL CALLBACK		s_dlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static BOOL CALLBACK		s_detailsProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL 						_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL 						_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL 						_onDetailsCommand(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL						detailsProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	static LRESULT				s_detailsGetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);
	LRESULT						detailsGetMsgProc(int nCode, WPARAM wParam, LPARAM lParam);
	void						setBackendValidity(bool valid);

protected:
	XAP_Win32DialogHelper *		m_pWin32Dialog;

	virtual void*				_getEmbeddingParent();
	virtual AccountHandler*		_getActiveAccountHandler();

	// Handles
	HINSTANCE 					m_hInstance;
	HWND						m_hOk;
	LONG_PTR					m_pOldDetailsProc;
	HWND						m_hWnd;
	HWND						m_hDetails;
	HHOOK						m_hDetailsHook;

	// Data
	std::vector<AccountHandler*>	m_vAccountTypeCombo;

private:

	void						_populateWindowData(void);
};

#endif /* AP_WIN32DIALOG_COLLABORATIONADDACCOUNT_H */
