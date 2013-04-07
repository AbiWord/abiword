/* AbiCollab - Code to enable the modification of remote documents.
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

#ifndef AP_WIN32DIALOG_COLLABORATIONSHARE_H
#define AP_WIN32DIALOG_COLLABORATIONSHARE_H

#include <map>
#include <ut_string.h>
#include <windows.h>
#include <commctrl.h>

#include "ap_Win32Res_DlgCollaborationShare.rc2"

#include <dialogs/xp/ap_Dialog_CollaborationShare.h>

class XAP_Frame;

struct ShareListItem
{
	ShareListItem(BuddyPtr pBuddy_, DocHandle* pDocHandle_) : pBuddy(pBuddy_), pDocHandle(pDocHandle_) {};
	BuddyPtr pBuddy;
	DocHandle* pDocHandle;
};

class AP_Win32Dialog_CollaborationShare : public AP_Dialog_CollaborationShare
{
public:
	AP_Win32Dialog_CollaborationShare(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id);
	static XAP_Dialog * 		static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id);

	void						runModal(XAP_Frame * pFrame);
	static BOOL CALLBACK		s_dlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	BOOL 						_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam);
	BOOL 						_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam);

private:
	XAP_Win32DialogHelper *	m_pWin32Dialog;

	void					_populateWindowData();
	void					_populateBuddyModel(bool refresh);
	void					_refreshWindow();
	AccountHandler*			_getActiveAccountHandler();
	void					_setAccountHint(const UT_UTF8String& sHint);
	void					_getSelectedBuddies(std::vector<std::string>& vACL);
	void					_freeBuddyList();

	// Handles
	HINSTANCE 				m_hInstance;
	HWND					m_hAccountCombo;
	HWND					m_hBuddyList;

	// Internal states
	HTREEITEM				m_hSelected;

	// Model
	std::vector<AccountHandler*>		m_vAccountCombo;
	std::map< HTREEITEM, ShareListItem > m_mTreeItemHandles;
};

#endif /* AP_WIN32DIALOG_COLLABORATIONSHARE_H */
