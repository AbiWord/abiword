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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */
 
#include <windows.h>
#include "xap_App.h"
#include "ap_Win32App.h"
#include "xap_Win32App.h"
#include "xap_Frame.h"
#include "xap_Win32DialogHelper.h"
#include "ut_string_class.h"
#include <session/xp/AbiCollabSessionManager.h>
#include <account/xp/Event.h>
#include <account/xp/AccountEvent.h>

#include "ap_Win32Dialog_CollaborationShare.h"

BOOL CALLBACK AP_Win32Dialog_CollaborationShare::s_dlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	AP_Win32Dialog_CollaborationShare * pThis;
	switch (msg)
	{
	case WM_INITDIALOG:
		pThis = (AP_Win32Dialog_CollaborationShare *)lParam;
		UT_return_val_if_fail(pThis, 0);
		SetWindowLong(hWnd,DWL_USER,lParam);
		return pThis->_onInitDialog(hWnd,wParam,lParam);
		
	case WM_COMMAND:
		pThis = (AP_Win32Dialog_CollaborationShare *)GetWindowLong(hWnd,DWL_USER);
		UT_return_val_if_fail(pThis, 0);
		return pThis->_onCommand(hWnd,wParam,lParam);
		
	case WM_DESTROY:
		pThis = (AP_Win32Dialog_CollaborationShare *)GetWindowLong(hWnd,DWL_USER);
		if (pThis->m_pWin32Dialog)
		{
			DELETEP(pThis->m_pWin32Dialog);
		}
		
		// WM_DESTROY processed
		return 0;
		
	case WM_NOTIFY:
		pThis = (AP_Win32Dialog_CollaborationShare *)GetWindowLong(hWnd,DWL_USER);
		UT_return_val_if_fail(pThis, 0);
		UT_return_val_if_fail(lParam, 0);
		return pThis->_onNotify(hWnd, wParam, lParam);
		
	default:
		// Windows system should process any other messages
		return false;
	}
}

XAP_Dialog * AP_Win32Dialog_CollaborationShare::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	return static_cast<XAP_Dialog *>(new AP_Win32Dialog_CollaborationShare(pFactory, id));
}
pt2Constructor ap_Dialog_CollaborationShare_Constructor = &AP_Win32Dialog_CollaborationShare::static_constructor;


AP_Win32Dialog_CollaborationShare::AP_Win32Dialog_CollaborationShare(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: AP_Dialog_CollaborationShare(pDlgFactory, id),
	m_pWin32Dialog(NULL),
	m_hInstance(NULL)
{
	UT_DEBUGMSG(("AP_Win32Dialog_CollaborationShare()\n"));
	AbiCollabSessionManager * pSessionManager= AbiCollabSessionManager::getManager();
	if (pSessionManager)
	{
		m_hInstance=pSessionManager->getInstance();
	}
}

void AP_Win32Dialog_CollaborationShare::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);
	UT_return_if_fail(m_hInstance);
	
	LPCTSTR lpTemplate = MAKEINTRESOURCE(AP_RID_DIALOG_COLLABORATIONSHARE);
	
	int result = DialogBoxParam( m_hInstance, lpTemplate,
		static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow(),
		(DLGPROC)s_dlgProc, (LPARAM)this );
	switch (result) {
	case 0:
		// MSDN: If the function fails because the hWndParent parameter is invalid, the return value is zero.
		break;
	case -1:
		UT_DEBUGMSG(("Win32 error: %d.  lpTemplate: %d, RID:%d\n", GetLastError(), lpTemplate, AP_RID_DIALOG_COLLABORATIONSHARE));
		
		break;
	default:
		break;
		// ok!
	};

}

/*****************************************************************/

BOOL AP_Win32Dialog_CollaborationShare::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	// Welcome, let's initialize a dialog!
	//////
	// Store handles for easy access
	// Reminder: hDlg is in our DialogHelper
	m_hDocumentTreeview = GetDlgItem(hWnd, AP_RID_DIALOG_COLLABORATIONSHARE_DOCUMENT_TREE);

	// Set up common controls
	INITCOMMONCONTROLSEX icc;
	icc.dwSize=sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC=ICC_TREEVIEW_CLASSES;
	
	// If we can't init common controls, bail out
	UT_return_val_if_fail(InitCommonControlsEx(&icc), false);
	
	// Get ourselves a custom DialogHelper
	DELETEP(m_pWin32Dialog);
	m_pWin32Dialog = new XAP_Win32DialogHelper(hWnd);
	
	_populateWindowData();
	_setModel();
	//_refreshAccounts();
	
	// we have no selection yet
	_updateSelection();
	
	// Center Window
	m_pWin32Dialog->centerDialog();
	
	// WM_INITDIALOG wants True returned in order to continue processing
	return true;
}

BOOL AP_Win32Dialog_CollaborationShare::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wId = LOWORD(wParam);
	
	switch (wId)
	{
	
	case AP_RID_DIALOG_COLLABORATIONSHARE_CANCEL_BUTTON:
		EndDialog(hWnd,0);
		return true;

	case AP_RID_DIALOG_COLLABORATIONSHARE_SHARE_BUTTON:
		EndDialog(hWnd, 0);
		return true;

	default:
		return false;
	}
}

BOOL AP_Win32Dialog_CollaborationShare::_onNotify(HWND hWnd, WPARAM wParam, LPARAM lParam){
	switch (((LPNMHDR)lParam)->code)
	{
		//case UDN_DELTAPOS:		return pThis->_onDeltaPos((NM_UPDOWN *)lParam);
		//UT_DEBUGMSG(("Notify: Code=0x%x\n", ((LPNMHDR)lParam)->code));
		
		case NM_DBLCLK:
			return false; // need to think this through, just toggling the state sounds wrong to me, UI-wise - MARCM
			
/*			// A double-click will toggle join status
			// TODO: This is probably awful GUI-wise
			_updateSelection();
			// join stuff!
			if (m_bShareSelected)
			{
				// if they double clicked on a shared document
				if (!_setJoin(m_hSelected, true))
				{
					// if setting join to true didn't change join status
					// then set it to false.
					// this minimizes compares required to do the toggle,
					// while leaving the safety logic in setJoin to prevent double-join or disjoins
					_setJoin(m_hSelected, false);
				}
				
				// refresh list after toggle
				_refreshAllDocHandlesAsync();
				_setModel();
			}
			return 1;*/
			
		case TVN_SELCHANGED:
			_updateSelection();
			return 1;
			
		default:
			return 0;
	}
}

void AP_Win32Dialog_CollaborationShare::_populateWindowData()
{
	UT_DEBUGMSG(("AP_Win32Dialog_CollaborationShare::_populateWindowData()\n"));
	UT_return_if_fail(m_pWin32Dialog);

	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_if_fail(pManager);
	
	AccountHandler* pShareeableAcount = _getShareableAccountHandler();

	for (std::vector<AccountHandler*>::const_iterator cit = pManager->getAccounts().begin(); cit != pManager->getAccounts().end(); cit++)
	{
		AccountHandler* pAccount = *cit;
		UT_continue_if_fail(pAccount);

		if (pShareeableAcount && pShareeableAcount != pAccount)
			continue;

		UT_sint32 index = m_pWin32Dialog->addItemToCombo(AP_RID_DIALOG_COLLABORATIONSHARE_ACCOUNTCOMBO, (LPCSTR) AP_Win32App::s_fromUTF8ToWinLocale(pAccount->getDescription().utf8_str()).c_str());
		if (index >= 0)
		{
			UT_DEBUGMSG(("Added handler to index %d\n", index));
			//m_vAccountTypeCombo.insert(m_vAccountTypeCombo.begin()+index, pHandler);
		}
		else
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	}

	// if we have at least one account, then make sure the first one is selected
	if (pManager->getAccounts().size() > 0)
	{
		m_pWin32Dialog->selectComboItem(AP_RID_DIALOG_COLLABORATIONSHARE_ACCOUNTCOMBO, 0);
	}
	else
	{
		// nope, we don't have any account handler :'-( 
		// Add a sample item to show that we can.  Then, disable the box and the ok button
		m_pWin32Dialog->addItemToCombo(AP_RID_DIALOG_COLLABORATIONSHARE_ACCOUNTCOMBO, (LPCSTR) "No Accounts Available!");
		m_pWin32Dialog->selectComboItem(AP_RID_DIALOG_COLLABORATIONSHARE_ACCOUNTCOMBO, 0);
	}
}

void AP_Win32Dialog_CollaborationShare::_setModel()
{
	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_if_fail(pManager);

/*
	const std::vector<AccountHandler *>& accounts = pManager->getAccounts();
	
	// clear the treeview; items will not be displayed until the window styles are reset
	m_mTreeItemHandles.clear();
	DWORD styles = GetWindowLong(m_hDocumentTreeview, GWL_STYLE);
	TreeView_DeleteAllItems(m_hDocumentTreeview);

	// Loop through accounts
	for (UT_uint32 i = 0; i < accounts.size(); i++)
	{
		// Loop through buddies in accounts
		for (UT_uint32 j = 0; j < accounts[i]->getBuddies().size(); j++)
		{
			BuddyPtr pBuddy = accounts[i]->getBuddies()[j];
			UT_UTF8String buddyDesc = pBuddy->getDescription();
			UT_DEBUGMSG(("Adding buddy (%s) to the treeview\n", buddyDesc.utf8_str()));

			UT_String sBuddyText = AP_Win32App::s_fromUTF8ToWinLocale(buddyDesc.utf8_str());
			TV_INSERTSTRUCT tviBuddy;
			tviBuddy.item.mask = TVIF_TEXT| TVIF_STATE; // text only right now
			tviBuddy.item.stateMask = TVIS_BOLD|TVIS_EXPANDED;
			tviBuddy.hInsertAfter = TVI_LAST;  // only insert at the end			
			tviBuddy.hParent = NULL; // top most level Item
			tviBuddy.item.state = 0;
			tviBuddy.item.pszText = const_cast<char*>(sBuddyText.c_str());
			HTREEITEM htiBuddy = (HTREEITEM)SendMessage(m_hDocumentTreeview, TVM_INSERTITEM,0,(LPARAM)&tviBuddy);
			m_mTreeItemHandles.insert(std::pair<HTREEITEM, ShareListItem>(htiBuddy, ShareListItem(pBuddy, NULL)));
			
			// Loop through documents for each buddy
			for (const DocTreeItem* item = pBuddy->getDocTreeItems(); item; item = item->m_next)
			{
				UT_continue_if_fail(item->m_docHandle);
				UT_UTF8String docDesc = item->m_docHandle->getName();
				UT_DEBUGMSG(("Adding document (%s) to the treeview\n", docDesc.utf8_str()));
				
				UT_String sDocText = AP_Win32App::s_fromUTF8ToWinLocale(docDesc.utf8_str());
				TV_INSERTSTRUCT tviDocument;
				tviDocument.item.mask = TVIF_TEXT| TVIF_STATE; // text only right now
				tviDocument.item.stateMask = TVIS_BOLD|TVIS_EXPANDED;
				tviDocument.hInsertAfter = TVI_LAST;  // only insert at the end			
				tviDocument.hParent = htiBuddy;
				tviDocument.hInsertAfter = TVI_LAST;
				tviDocument.item.pszText = const_cast<char*>(sDocText.c_str());
				// if we are connected to this document, bold it.  Eventually checkboxes would be cooler, that's a TODO
				tviDocument.item.state = pManager->isActive(item->m_docHandle->getSessionId()) ? TVIS_BOLD : TVIS_EXPANDED;
				HTREEITEM htiDoc = (HTREEITEM)SendMessage(m_hDocumentTreeview, TVM_INSERTITEM, 0, (LPARAM)&tviDocument);
				m_mTreeItemHandles.insert(std::pair<HTREEITEM, ShareListItem>(htiDoc, ShareListItem(pBuddy, item->m_docHandle)));
			}
		}
	}
	
	// Now expand all of the buddies
	for (std::map< HTREEITEM, ShareListItem >::const_iterator cit = m_mTreeItemHandles.begin(); cit != m_mTreeItemHandles.end(); cit++)
	{
		UT_continue_if_fail(cit->first);
		if (!cit->second.pDocHandle)
			TreeView_Expand(m_hDocumentTreeview, cit->first, TVE_EXPAND);
	}
	
	// reset the window styles; shows all items
	SetWindowLong(m_hDocumentTreeview, GWL_STYLE, styles);

	_updateSelection();
*/
}

void AP_Win32Dialog_CollaborationShare::_refreshWindow()
{
	_setModel();
}

void AP_Win32Dialog_CollaborationShare::_updateSelection()
{
	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_if_fail(pManager);	
	
	HTREEITEM hSelItem = TreeView_GetSelection(m_hDocumentTreeview);
	if (hSelItem)
	{
		m_hSelected = hSelItem;

		std::map< HTREEITEM, ShareListItem >::const_iterator cit = m_mTreeItemHandles.find(hSelItem);
		UT_return_if_fail(cit != m_mTreeItemHandles.end());
		if (cit->second.pDocHandle)
		{
			UT_DEBUGMSG(("Document selected\n"));
			bool bIsConnected = pManager->isActive(cit->second.pDocHandle->getSessionId());
			//m_pWin32Dialog->enableControl(AP_RID_DIALOG_COLLABORATIONSHARE_CONNECT_BUTTON, !bIsConnected );
		}
		else
		{
			UT_DEBUGMSG(("Buddy selected\n"));
			//m_pWin32Dialog->enableControl(AP_RID_DIALOG_COLLABORATIONSHARE_CONNECT_BUTTON, false);
		}
	}
	else
	{
		//m_pWin32Dialog->enableControl(AP_RID_DIALOG_COLLABORATIONSHARE_CONNECT_BUTTON, false);
	}
}
