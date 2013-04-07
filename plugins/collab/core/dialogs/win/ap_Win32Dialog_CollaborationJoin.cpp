/* AbiCollab - Code to enable the modification of remote documents.
 * Copyright (C) 2007 by Ryan Pavlik <abiryan@ryand.net>
 * Copyright (C) 2006, 2007 by Marc Maurer <uwog@uwog.net>
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
#include "xap_Win32DialogHelper.h"
#include "ut_string_class.h"
#include <session/xp/AbiCollabSessionManager.h>
#include <account/xp/Event.h>
#include <account/xp/AccountEvent.h>
#include <backends/xmpp/xp/XMPPBuddy.h>

#include "ap_Win32Dialog_CollaborationJoin.h"

BOOL CALLBACK AP_Win32Dialog_CollaborationJoin::s_dlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	AP_Win32Dialog_CollaborationJoin * pThis;
	switch (msg)
	{
	case WM_INITDIALOG:
		pThis = (AP_Win32Dialog_CollaborationJoin *)lParam;
		UT_return_val_if_fail(pThis, 0);
		SetWindowLongPtrW(hWnd,DWLP_USER,lParam);
		return pThis->_onInitDialog(hWnd,wParam,lParam);
		
	case WM_COMMAND:
		pThis = (AP_Win32Dialog_CollaborationJoin *)GetWindowLongPtrW(hWnd,DWLP_USER);
		UT_return_val_if_fail(pThis, 0);
		return pThis->_onCommand(hWnd,wParam,lParam);
		
	case WM_DESTROY:
		pThis = (AP_Win32Dialog_CollaborationJoin *)GetWindowLongPtrW(hWnd,DWLP_USER);
		pThis->_freeBuddyList();
		if (pThis->p_win32Dialog)
		{
			DELETEP(pThis->p_win32Dialog);
		}
		
		// WM_DESTROY processed
		return 0;
		
	case WM_NOTIFY:
		pThis = (AP_Win32Dialog_CollaborationJoin *)GetWindowLongPtrW(hWnd,DWLP_USER);
		UT_return_val_if_fail(pThis, 0);
		UT_return_val_if_fail(lParam, 0);
		return pThis->_onNotify(hWnd, wParam, lParam);
		
	default:
		// Windows system should process any other messages
		return false;
	}
}

XAP_Dialog * AP_Win32Dialog_CollaborationJoin::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	return static_cast<XAP_Dialog *>(new AP_Win32Dialog_CollaborationJoin(pFactory, id));
}
pt2Constructor ap_Dialog_CollaborationJoin_Constructor = &AP_Win32Dialog_CollaborationJoin::static_constructor;


AP_Win32Dialog_CollaborationJoin::AP_Win32Dialog_CollaborationJoin(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: AP_Dialog_CollaborationJoin(pDlgFactory, id),
	p_win32Dialog(NULL),
	m_hInstance(NULL)
{
	UT_DEBUGMSG(("AP_Win32Dialog_CollaborationJoin()\n"));
	AbiCollabSessionManager * pSessionManager= AbiCollabSessionManager::getManager();
	if (pSessionManager)
	{
		m_hInstance=pSessionManager->getInstance();
	}
}

void AP_Win32Dialog_CollaborationJoin::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);
	UT_return_if_fail(m_hInstance);
	
	LPCTSTR lpTemplate = MAKEINTRESOURCE(AP_RID_DIALOG_COLLABORATIONJOIN);
	
	int result = DialogBoxParam( m_hInstance, lpTemplate,
		static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow(),
		(DLGPROC)s_dlgProc, (LPARAM)this );
	switch (result) {
	case 0:
		// MSDN: If the function fails because the hWndParent parameter is invalid, the return value is zero.
		break;
	case -1:
		UT_DEBUGMSG(("Win32 error: %d.  lpTemplate: %d, RID:%d\n", GetLastError(), lpTemplate, AP_RID_DIALOG_COLLABORATIONJOIN));
		
		break;
	default:
		break;
	};
}

/*****************************************************************/

BOOL AP_Win32Dialog_CollaborationJoin::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	// Store handles for easy access
	// Reminder: hDlg is in our DialogHelper
	m_hDocumentTreeview = GetDlgItem(hWnd, AP_RID_DIALOG_COLLABORATIONJOIN_DOCUMENT_TREE);

	// Set up common controls
	INITCOMMONCONTROLSEX icc;
	icc.dwSize=sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC=ICC_TREEVIEW_CLASSES;
	
	// If we can't init common controls, bail out
	UT_return_val_if_fail(InitCommonControlsEx(&icc), false);
	
	// Get ourselves a custom DialogHelper
	DELETEP(p_win32Dialog);
	p_win32Dialog = new XAP_Win32DialogHelper(hWnd);
	
	// Set up dialog initial state
	_refreshAllDocHandlesAsync();
	_populateWindowData();
	_refreshAccounts();
	
	// Center Window
	p_win32Dialog->centerDialog();
	
	// WM_INITDIALOG wants True returned in order to continue processing
	return true;
}

BOOL AP_Win32Dialog_CollaborationJoin::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wId = LOWORD(wParam);
	
	switch (wId)
	{
	case AP_RID_DIALOG_COLLABORATIONJOIN_OPEN_BUTTON:
		_eventOpen();
		EndDialog(hWnd, 0);
		return true;

	case AP_RID_DIALOG_COLLABORATIONJOIN_CANCEL_BUTTON:
		EndDialog(hWnd,0);
		return true;

	case AP_RID_DIALOG_COLLABORATIONJOIN_ADDBUDDY_BUTTON:
		_eventAddBuddy();
		return true;

	case AP_RID_DIALOG_COLLABORATIONJOIN_REFRESH_BUTTON:
		// TODO: we really should refresh the buddies here as well, 
		// as they could pop up automatically as well (for example with a 
		// avahi backend)
		_refreshAllDocHandlesAsync();
		return true;

	default:
		return false;
	}
}

BOOL AP_Win32Dialog_CollaborationJoin::_onNotify(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch (((LPNMHDR)lParam)->code)
	{
		case TVN_SELCHANGED:
			_updateSelection();
			return 1;
			
		default:
			return 0;
	}
}

HTREEITEM AP_Win32Dialog_CollaborationJoin::_addBuddyToTree(BuddyPtr pBuddy)
{
	UT_UTF8String buddyDesc = pBuddy->getDescription();

	UT_Win32LocaleString sBuddyText = AP_Win32App::s_fromUTF8ToWinLocale(buddyDesc.utf8_str());
	TV_INSERTSTRUCTW tviBuddy;
	tviBuddy.item.mask = TVIF_TEXT | TVIF_STATE | TVIF_PARAM;
	tviBuddy.hInsertAfter = TVI_SORT;
	tviBuddy.hParent = NULL; // top most level Item
	tviBuddy.item.state = 0;
	tviBuddy.item.pszText = (LPWSTR) sBuddyText.c_str();
	tviBuddy.item.lParam = (LPARAM)new ShareListItem(pBuddy, NULL);
	return (HTREEITEM)SendMessageW(m_hDocumentTreeview, TVM_INSERTITEMW,0,(LPARAM)&tviBuddy);
}

HTREEITEM AP_Win32Dialog_CollaborationJoin::_addDocumentToBuddy(HTREEITEM buddyItem, BuddyPtr pBuddy, DocHandle* pDocHandle)
{
	UT_return_val_if_fail(buddyItem, NULL);
	UT_return_val_if_fail(pBuddy, NULL);
	UT_return_val_if_fail(pDocHandle, NULL);

	UT_UTF8String docDesc = pDocHandle->getName();
	UT_Win32LocaleString sDocText = AP_Win32App::s_fromUTF8ToWinLocale(docDesc.utf8_str());
	TV_INSERTSTRUCTW tviDocument;
	tviDocument.item.mask = TVIF_TEXT | TVIF_STATE | TVIF_PARAM;
	tviDocument.hInsertAfter = TVI_SORT;
	tviDocument.hParent = buddyItem;
	tviDocument.item.pszText = const_cast<LPWSTR>(sDocText.c_str());
	tviDocument.item.state = 0;
	tviDocument.item.lParam = (LPARAM)new ShareListItem(pBuddy, pDocHandle);
	return (HTREEITEM)SendMessageW(m_hDocumentTreeview, TVM_INSERTITEMW, 0, (LPARAM)&tviDocument);
}

void AP_Win32Dialog_CollaborationJoin::_populateWindowData()
{
	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	UT_return_if_fail(pManager);
	
	const std::vector<AccountHandler *>& accounts = pManager->getAccounts();
	
	// clear out the old contents, if any; items will not be displayed until the window styles are reset
	LONG_PTR styles = GetWindowLongPtrW(m_hDocumentTreeview, GWL_STYLE);
	_freeBuddyList();

	// Loop through accounts
	for (UT_uint32 i = 0; i < accounts.size(); i++)
	{
		UT_continue_if_fail(accounts[i]);
		if (!accounts[i]->isOnline())
			continue;

		// Loop through buddies in accounts
		for (UT_uint32 j = 0; j < accounts[i]->getBuddies().size(); j++)
		{
			BuddyPtr pBuddy = accounts[i]->getBuddies()[j];
			UT_continue_if_fail(pBuddy);

			const DocTreeItem* docTreeItems = pBuddy->getDocTreeItems();
			// let's skip buddies that have no document shared
			if (!docTreeItems)
				continue;

			HTREEITEM htiBuddy = _addBuddyToTree(pBuddy);
			
			// add all documents for this buddy
			for (const DocTreeItem* item = docTreeItems; item; item = item->m_next)
			{
				UT_continue_if_fail(item);
				UT_continue_if_fail(_addDocumentToBuddy(htiBuddy, pBuddy, item->m_docHandle));
			}
		}
	}
	
	// reset the window styles; shows all items
	SetWindowLongPtrW(m_hDocumentTreeview, GWL_STYLE, styles);

	_updateSelection();
}

void AP_Win32Dialog_CollaborationJoin::_refreshWindow()
{
	_populateWindowData();
}

void AP_Win32Dialog_CollaborationJoin::_enableBuddyAddition(bool bEnabled)
{
	p_win32Dialog->enableControl(AP_RID_DIALOG_COLLABORATIONJOIN_ADDBUDDY_BUTTON, bEnabled);
}

ShareListItem* AP_Win32Dialog_CollaborationJoin::_getSelectedItem()
{
	UT_return_val_if_fail(m_hDocumentTreeview, NULL);

	HTREEITEM hSelItem = TreeView_GetSelection(m_hDocumentTreeview);
	if (!hSelItem)
	{
		p_win32Dialog->enableControl(AP_RID_DIALOG_COLLABORATIONJOIN_OPEN_BUTTON, false);
		return NULL;
	}

	TVITEM item;
	item.mask = TVIF_PARAM;
	item.hItem = hSelItem;
	UT_return_val_if_fail(SendMessageW(m_hDocumentTreeview, LVM_GETITEM, 0, 
		(LPARAM) &item), NULL);

	return reinterpret_cast<ShareListItem*>(item.lParam); 
}

void AP_Win32Dialog_CollaborationJoin::_updateSelection()
{	
	ShareListItem* sli = _getSelectedItem();
	if (!sli)
		return;

	if (sli->pDocHandle)
	{
		UT_DEBUGMSG(("Document selected\n"));
		p_win32Dialog->enableControl(AP_RID_DIALOG_COLLABORATIONJOIN_OPEN_BUTTON, true );
	}
	else
	{
		UT_DEBUGMSG(("Buddy selected\n"));
		p_win32Dialog->enableControl(AP_RID_DIALOG_COLLABORATIONJOIN_OPEN_BUTTON, false);
	}
}

void AP_Win32Dialog_CollaborationJoin::_eventOpen()
{
	UT_DEBUGMSG(("AP_Win32Dialog_CollaborationJoin::_eventOpen()\n"));
	ShareListItem* sli = _getSelectedItem();
	UT_return_if_fail(sli && sli->pBuddy && sli->pDocHandle);

	m_answer = AP_Dialog_CollaborationJoin::a_OPEN;
	m_pBuddy = sli->pBuddy;
	m_pDocHandle = sli->pDocHandle;
}

void AP_Win32Dialog_CollaborationJoin::_addDocument(BuddyPtr pBuddy, DocHandle* pDocHandle)
{
	UT_DEBUGMSG(("AP_Win32Dialog_CollaborationJoin::_addDocument()"));
	UT_return_if_fail(pBuddy);
	UT_return_if_fail(pDocHandle);

	// check if this buddy is already in our tree
	HTREEITEM item = TreeView_GetRoot(m_hDocumentTreeview);
    while (item)
	{
		TVITEMW buddyItem;
		buddyItem.mask = TVIF_PARAM;
		buddyItem.hItem = item;
		UT_return_if_fail(SendMessageW(m_hDocumentTreeview, TVM_GETITEM, 0, (LPARAM) &buddyItem));		
		UT_return_if_fail(buddyItem.lParam);

		ShareListItem* sli = reinterpret_cast<ShareListItem*>(buddyItem.lParam);
		if (pBuddy->getDescriptor(false) == sli->pBuddy->getDescriptor(false))
			break;

		item = TreeView_GetNextItem(m_hDocumentTreeview, item, TVGN_NEXT);
	}
	
	if (!item)
	{
		// the buddy is not in our tree yet, let's add it
		item = _addBuddyToTree(pBuddy);
		UT_return_if_fail(item);
	}

	// add the document to the buddy
	HTREEITEM htiDoc = _addDocumentToBuddy(item, pBuddy, pDocHandle);
	UT_ASSERT_HARMLESS(htiDoc);
}

static void s_treeview_delete_lparam(HWND hTreeView, HTREEITEM hItem)
{
	UT_return_if_fail(hTreeView);
	UT_return_if_fail(hItem);

	// fetch the lparam
	TVITEM item;
	item.mask = TVIF_PARAM;
	item.hItem = hItem;
	UT_return_if_fail(TreeView_GetItem(hTreeView, &item));

	// delete the lparam
	ShareListItem* sli = reinterpret_cast<ShareListItem*>(item.lParam);
	DELETEP(sli);

	// reset the lparam to NULL
	item.lParam = (LPARAM)NULL;
	UT_return_if_fail(TreeView_SetItem(hTreeView, &item));
}

void AP_Win32Dialog_CollaborationJoin::_freeBuddyList()
{
	UT_DEBUGMSG(("AP_Win32Dialog_CollaborationJoin::_freeBuddyList()\n"));

	HTREEITEM hItem = TreeView_GetRoot(m_hDocumentTreeview);
	while (hItem)
	{
		// delete all the child document items...
		HTREEITEM hChildItem = TreeView_GetNextItem(m_hDocumentTreeview, hItem, TVGN_CHILD);
		while (hChildItem)
		{
			s_treeview_delete_lparam(m_hDocumentTreeview, hChildItem);
			hChildItem = TreeView_GetNextItem(m_hDocumentTreeview, hChildItem, TVGN_NEXT);
		}

		// ... and delete the parent buddy item as well
		s_treeview_delete_lparam(m_hDocumentTreeview, hItem);
		hItem = TreeView_GetNextItem(m_hDocumentTreeview, hItem, TVGN_NEXT);
	}

	TreeView_DeleteAllItems(m_hDocumentTreeview);
}
