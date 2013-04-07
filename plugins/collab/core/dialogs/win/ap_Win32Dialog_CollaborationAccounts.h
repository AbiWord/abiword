/* AbiCollab - Code to enable the modification of remote documents.
 * Copyright (C) 2007 by Ryan Pavlik <abiryan@ryand.net>
 * Copyright (C) 2006 by Marc Maurer <uwog@uwog.net>
 * Copyright (C) 2009 by AbiSource Corporation B.V.
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

#ifndef AP_WIN32DIALOG_COLLABORATIONACCOUNTS_H
#define AP_WIN32DIALOG_COLLABORATIONACCOUNTS_H

#include <map>
#include <ut_string.h>
#include "ap_Win32Dialog_CollaborationAddAccount.h"

#include <dialogs/xp/ap_Dialog_CollaborationAccounts.h>
#include "ap_Win32Res_DlgCollaborationAccounts.rc2"

class XAP_Frame;

class AP_Win32Dialog_CollaborationAccounts : public AP_Dialog_CollaborationAccounts
{
public:
	AP_Win32Dialog_CollaborationAccounts(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);

	static XAP_Dialog * 	static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id);
	void					runModal(XAP_Frame * pFrame);
	static BOOL CALLBACK	s_dlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL 					_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL 					_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL					_onNotify(HWND hWnd, WPARAM wParam, LPARAM lParam);

	virtual void			signal(const Event& event, BuddyPtr pSource);

private:
	void					_populateWindowData();
	void					_updateSelection();
	void					_setOnline(AccountHandler* pHandler, bool online);

	XAP_Win32DialogHelper *	m_pWin32Dialog;
	HINSTANCE 				m_hInstance;
	HWND					m_hAccountList;
	bool					m_bPopulating; // hack
};

#endif /* AP_WIN32DIALOG_COLLABORATIONACCOUNTS_H */
