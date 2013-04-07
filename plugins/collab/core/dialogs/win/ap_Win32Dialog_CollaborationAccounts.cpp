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

#include <windows.h>
#include "xap_App.h"
#include "ap_Win32App.h"
#include "xap_Win32App.h"
#include "xap_Frame.h"
#include "xap_Win32FrameImpl.h"
#include "xap_Win32DialogHelper.h"
#include "ut_string_class.h"

#include "ap_Win32Dialog_CollaborationAccounts.h"

BOOL CALLBACK AP_Win32Dialog_CollaborationAccounts::s_dlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			AP_Win32Dialog_CollaborationAccounts* pThis = (AP_Win32Dialog_CollaborationAccounts *)lParam;
			UT_return_val_if_fail(pThis, false);
			SetWindowLongPtrW(hWnd,DWLP_USER,lParam);
			return pThis->_onInitDialog(hWnd,wParam,lParam);
		}
		case WM_COMMAND:
		{
			AP_Win32Dialog_CollaborationAccounts* pThis = (AP_Win32Dialog_CollaborationAccounts *)GetWindowLongPtrW(hWnd,DWLP_USER);
			UT_return_val_if_fail(pThis, false);
			return pThis->_onCommand(hWnd,wParam,lParam);
		}
		case WM_DESTROY:
		{
			UT_DEBUGMSG(("Got WM_DESTROY\n"));
			AP_Win32Dialog_CollaborationAccounts* pThis = (AP_Win32Dialog_CollaborationAccounts *)GetWindowLongPtrW(hWnd,DWLP_USER);
			UT_return_val_if_fail(pThis, false);
			DELETEP(pThis->m_pWin32Dialog);
			return true;
		}
		case WM_NOTIFY:
		{
			AP_Win32Dialog_CollaborationAccounts* pThis = (AP_Win32Dialog_CollaborationAccounts *)GetWindowLongPtrW(hWnd,DWLP_USER);
			UT_return_val_if_fail(pThis, false);
			return pThis->_onNotify(hWnd,wParam,lParam);
		}
		default:
			// Message not processed - Windows should take care of it
			return false;
		}
}

XAP_Dialog * AP_Win32Dialog_CollaborationAccounts::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	return static_cast<XAP_Dialog *>(new AP_Win32Dialog_CollaborationAccounts(pFactory, id));
}
pt2Constructor ap_Dialog_CollaborationAccounts_Constructor = &AP_Win32Dialog_CollaborationAccounts::static_constructor;

AP_Win32Dialog_CollaborationAccounts::AP_Win32Dialog_CollaborationAccounts(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: AP_Dialog_CollaborationAccounts(pDlgFactory, id),
	m_pWin32Dialog(NULL),
	m_hInstance(NULL),
	m_hAccountList(NULL),
	m_bPopulating(false)
{
	AbiCollabSessionManager * pSessionManager= AbiCollabSessionManager::getManager();
	if (pSessionManager)
	{
		m_hInstance=pSessionManager->getInstance();
	}
}

void AP_Win32Dialog_CollaborationAccounts::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);
	UT_return_if_fail(m_hInstance);

	// create the dialog
	LPCTSTR lpTemplate = MAKEINTRESOURCE(AP_RID_DIALOG_COLLABORATIONACCOUNTS);
	int result = DialogBoxParam( m_hInstance, lpTemplate,
		static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow(),
		(DLGPROC)s_dlgProc, (LPARAM)this );
	switch (result)
	{
		case 0:
			// MSDN: If the function fails because the hWndParent parameter is invalid, the return value is zero.
			break;
		case -1:
			UT_DEBUGMSG(("Win32 error: %d.  lpTemplate: %d, RID:%d\n", GetLastError(), lpTemplate, AP_RID_DIALOG_COLLABORATIONACCOUNTS));
			break;
		default:
			break;
	};
}

void AP_Win32Dialog_CollaborationAccounts::signal(const Event& event, BuddyPtr pSource)
{
	UT_DEBUGMSG(("AP_Win32Dialog_CollaborationAccounts::signal()\n"));
	switch (event.getClassType())
	{
		case PCT_AccountNewEvent:
		case PCT_AccountOnlineEvent:
		case PCT_AccountOfflineEvent:
			// FIXME: VERY VERY BAD, CHECK WHICH ACCOUNT HAS CHANGED, AND UPDATE THAT
			_populateWindowData();
			break;
		default:
			// we will ignore the rest
			break;
	}
}

BOOL AP_Win32Dialog_CollaborationAccounts::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	// Set up common controls
	INITCOMMONCONTROLSEX icc;
	icc.dwSize=sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC=ICC_LISTVIEW_CLASSES;

	m_hAccountList = GetDlgItem(hWnd, AP_RID_DIALOG_COLLABORATIONACCOUNTS_ACCOUNT_LIST);

	// setup our listview columns

	// column 0
	LVCOLUMNW lvc;
	lvc.mask = LVCF_WIDTH | LVCF_TEXT; 
    lvc.pszText = L"Online";	
	lvc.cx = 50;
	ListView_InsertColumn(m_hAccountList, 0, &lvc);

	// column 1
	lvc.mask = LVCF_WIDTH | LVCF_TEXT; 
    lvc.pszText = L"Account";	
	lvc.cx = 180;
	ListView_InsertColumn(m_hAccountList, 1, &lvc);

	// column 2
	lvc.mask = LVCF_WIDTH | LVCF_TEXT; 
    lvc.pszText = L"Type";
	lvc.cx = 200;
	ListView_InsertColumn(m_hAccountList, 2, &lvc);

	// enable checkboxes and row select on items in the list view
	ListView_SetExtendedListViewStyle(m_hAccountList, LVS_EX_CHECKBOXES | LVS_EX_FULLROWSELECT);

	// Get ourselves a custom DialogHelper
	DELETEP(m_pWin32Dialog);
	m_pWin32Dialog = new XAP_Win32DialogHelper(hWnd);
	
	// Set up dialog initial state
	_populateWindowData();
	
	// we have no selection yet
	//_updateSelection();
	
	// Center Window
	m_pWin32Dialog->centerDialog();

	return true;
}

void AP_Win32Dialog_CollaborationAccounts::_populateWindowData()
{
	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_if_fail(pManager);

	m_bPopulating = true;

	// clear out the old contents, if any
	ListView_DeleteAllItems(m_hAccountList);

	for (UT_uint32 i = 0; i < pManager->getAccounts().size(); i++)
	{
		AccountHandler* pAccount = pManager->getAccounts()[i];
		UT_continue_if_fail(pAccount);

		UT_Win32LocaleString sAccountText = AP_Win32App::s_fromUTF8ToWinLocale(pAccount->getDescription().utf8_str());
		UT_Win32LocaleString sAccountTypeText = AP_Win32App::s_fromUTF8ToWinLocale(pAccount->getDisplayType().utf8_str());

		// insert a new account record
		LVITEMW lviAccount;
		lviAccount.mask = LVIF_STATE | LVIF_IMAGE | LVIF_PARAM;
		lviAccount.state = 1;
		lviAccount.iItem = i;
		lviAccount.iSubItem = 0;
		lviAccount.lParam = (LPARAM)pAccount;
		SendMessageW(m_hAccountList, LVM_INSERTITEMW, 0, (LPARAM) &lviAccount);
		ListView_SetCheckState(m_hAccountList, i, pAccount->isOnline());
		lviAccount.iSubItem=1;
		lviAccount.pszText= const_cast<LPWSTR>(sAccountText.c_str());
		SendMessageW(m_hAccountList, LVM_SETITEMTEXTW, i, (LPARAM) &lviAccount);
		lviAccount.iSubItem=2;
		lviAccount.pszText= const_cast<LPWSTR>(sAccountTypeText.c_str());
		SendMessageW(m_hAccountList, LVM_SETITEMTEXTW, i, (LPARAM) &lviAccount);
	}

	_updateSelection();

	m_bPopulating = false;
}

// return true if we process the command, false otherwise
BOOL AP_Win32Dialog_CollaborationAccounts::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);

	switch (wId)
	{
	case AP_RID_DIALOG_COLLABORATIONACCOUNTS_CLOSE_BUTTON:
		m_answer=AP_Dialog_CollaborationAccounts::a_CLOSE;
		EndDialog(hWnd,0);
		return true;
		
	case AP_RID_DIALOG_COLLABORATIONACCOUNTS_ADD_BUTTON:
		// open the Add dialog
		createNewAccount();
		// TODO: only refresh if it actually changed.
		_populateWindowData();
		return true;
	
	case AP_RID_DIALOG_COLLABORATIONACCOUNTS_DELETE_BUTTON:
		{
			AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
			UT_return_val_if_fail(pManager, false);

			bool hasSelection = ListView_GetSelectedCount(m_hAccountList);
			if (!hasSelection)
				return true;

			UT_sint32 iIndex = ListView_GetSelectionMark(m_hAccountList);
			UT_return_val_if_fail(iIndex >= 0, false);

			LVITEM lviAccount;
			lviAccount.mask = LVIF_PARAM;
			lviAccount.iItem = iIndex;
			lviAccount.iSubItem = 0;
			UT_return_val_if_fail(ListView_GetItem(m_hAccountList, &lviAccount), false);
			UT_return_val_if_fail(lviAccount.lParam, false);

			AccountHandler* pAccount = reinterpret_cast<AccountHandler*>(lviAccount.lParam);
			// TODO: we should ask for confirmation, as this account handler
			//		 could be in use by serveral AbiCollab Sessions
			UT_DEBUGMSG(("Delete account: %s of type %s\n", 
					pAccount->getDescription().utf8_str(), 
					pAccount->getDisplayType().utf8_str()
				));
			pManager->destroyAccount(pAccount);

			// for now, recreate the whole model; but we should really just delete
			// the iter we got above
			_populateWindowData();
		}
		return true;

	case AP_RID_DIALOG_COLLABORATIONACCOUNTS_PROPERTIES_BUTTON:
		// open the Properties dialog.  This does not exist yet, but when it does, we'll be ready.
		UT_DEBUGMSG(("AP_Win32Dialog_CollaborationAccounts::eventProperties()\n"));
		// TODO: implement me
		return true;

	default:
		// WM_COMMAND message NOT processed
		return false;
	}	
}

BOOL AP_Win32Dialog_CollaborationAccounts::_onNotify(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	LPNMHDR nmhdr = (LPNMHDR)lParam; 
	UT_return_val_if_fail(nmhdr, false);

	switch (nmhdr->code)
	{
		case LVN_ITEMCHANGED:
			// Not sure if LVN_ITEMCHANGED is the best place to check the selection state,
			// but I can't find a "selection changed" signal and this works...

			if (!m_bPopulating && wParam == AP_RID_DIALOG_COLLABORATIONACCOUNTS_ACCOUNT_LIST)
			{
				_updateSelection();

				LPNMLISTVIEW item = (LPNMLISTVIEW)lParam;
				UT_return_val_if_fail(item->iItem >= 0, false);
				UT_return_val_if_fail(item->lParam, false);

				AccountHandler* pAccount = reinterpret_cast<AccountHandler*>(item->lParam);
				bool isChecked = ListView_GetCheckState(m_hAccountList, item->iItem);
				_setOnline(pAccount, isChecked);
				return true;
			}
			return false;
		default:
			return false;
	}
}

void AP_Win32Dialog_CollaborationAccounts::_updateSelection()
{
	bool hasSelection = ListView_GetSelectedCount(m_hAccountList);
	if (hasSelection)
	{
		// TODO: Uncomment this line when Preferences does something.
		//m_pWin32Dialog->enableControl(AP_RID_DIALOG_COLLABORATIONACCOUNTS_PROPERTIES_BUTTON, true);
		m_pWin32Dialog->enableControl(AP_RID_DIALOG_COLLABORATIONACCOUNTS_DELETE_BUTTON, true);
	}
	else
	{
		m_pWin32Dialog->enableControl(AP_RID_DIALOG_COLLABORATIONACCOUNTS_PROPERTIES_BUTTON, false);
		m_pWin32Dialog->enableControl(AP_RID_DIALOG_COLLABORATIONACCOUNTS_DELETE_BUTTON, false);
	}
}

void AP_Win32Dialog_CollaborationAccounts::_setOnline(AccountHandler* pHandler, bool online)
{
	UT_DEBUGMSG(("AP_Win32Dialog_CollaborationAccounts::eventOnline()\n"));
	UT_return_if_fail(pHandler);
	
	if (!pHandler->isOnline() && online)
	{
		// if we're not online and we're asked to be
		pHandler->connect();
	}
	else if (pHandler->isOnline() && !online)
	{
		// if we are online and don't want to be
		pHandler->disconnect();
	}
}
