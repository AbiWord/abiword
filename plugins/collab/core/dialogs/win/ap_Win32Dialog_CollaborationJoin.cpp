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
#include <xp/AbiCollabSessionManager.h>
#include <backends/xp/Event.h>
#include <backends/xp/AccountEvent.h>
#include <backends/xmpp/xp/XMPPBuddy.h>

#include "ap_Win32Dialog_CollaborationJoin.h"

static BOOL CALLBACK AP_Win32Dialog_CollaborationJoin::s_dlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	AP_Win32Dialog_CollaborationJoin * pThis;
	switch (msg)
	{
	case WM_INITDIALOG:
		pThis = (AP_Win32Dialog_CollaborationJoin *)lParam;
		UT_return_val_if_fail(pThis, 0);
		SetWindowLong(hWnd,DWL_USER,lParam);
		return pThis->_onInitDialog(hWnd,wParam,lParam);
		
	case WM_COMMAND:
		pThis = (AP_Win32Dialog_CollaborationJoin *)GetWindowLong(hWnd,DWL_USER);
		UT_return_val_if_fail(pThis, 0);
		return pThis->_onCommand(hWnd,wParam,lParam);
		
	case WM_DESTROY:
		pThis = (AP_Win32Dialog_CollaborationJoin *)GetWindowLong(hWnd,DWL_USER);
		if (pThis->p_win32Dialog)
		{
			DELETEP(pThis->p_win32Dialog);
		}
		
		// WM_DESTROY processed
		return 0;
		
	case WM_NOTIFY:
		pThis = (AP_Win32Dialog_CollaborationJoin *)GetWindowLong(hWnd,DWL_USER);
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
	
	XAP_Win32App* pWin32App = static_cast<XAP_Win32App*>(XAP_App::getApp());
	
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
		// ok!
	};

}

/*****************************************************************/

BOOL AP_Win32Dialog_CollaborationJoin::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	// Welcome, let's initialize a dialog!
	//////
	// Store handles for easy access
	// Reminder: hDlg is in our DialogHelper
	m_hDocumentTreeview = GetDlgItem(hWnd, AP_RID_DIALOG_COLLABORATIONJOIN_DOCUMENT_TREE);
	
	// Set up common controls
	INITCOMMONCONTROLSEX icc;
	icc.dwSize=sizeof(INITCOMMONCONTROLSEX);
	icc.dwICC=ICC_TREEVIEW_CLASSES;
	
	// If we can't init common controls, bail out
	UT_return_val_if_fail(InitCommonControlsEx(&icc), false);
	
	//////
	// Get ourselves a custom DialogHelper
	if (p_win32Dialog)
	{
		DELETEP(p_win32Dialog);
	}
	p_win32Dialog = new XAP_Win32DialogHelper(hWnd);
	
	//////
	// Set up dialog initial state
	_refreshAllDocHandlesAsync();
	_setModel(_constructModel());
	_refreshAccounts();
	
	// we have no selection yet
	_updateSelection();
	
	// Center Window
	p_win32Dialog->centerDialog();
	

	// WM_INITDIALOG wants True returned in order to continue processing
	return true;
}

BOOL AP_Win32Dialog_CollaborationJoin::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;

	
	switch (wId)
	{
	case AP_RID_DIALOG_COLLABORATIONJOIN_JOIN_BUTTON:
		// join and close
		// Formerly/secretly the OK button
		_setJoin(m_hSelected, true);
		EndDialog(hWnd,0);
		
		// WM_COMMAND message processed - return 0
		return 0;

	case AP_RID_DIALOG_COLLABORATIONJOIN_CLOSE_BUTTON:
		// Close without necessarily joining
		// formerly/secretly the Cancel button
		EndDialog(hWnd,0);
		
		// WM_COMMAND message processed - return 0
		return 0;

	case AP_RID_DIALOG_COLLABORATIONJOIN_ADDBUDDY_BUTTON:
		// open the Add Buddy dialog
		_eventAddBuddy();
		
		// Would be nice to know if we actually succeeded in adding a buddy
		// to avoid gratuitous refreshes
		
		// have to refresh buddies
		
		// Refresh documents
		_refreshAllDocHandlesAsync();
		_setModel(_constructModel());
		
		// WM_COMMAND message processed - return 0
		return 0;
	
	case AP_RID_DIALOG_COLLABORATIONJOIN_DELETE_BUTTON:
		// right now, do nothing
		// TODO: Implement!
		
		// didn't actually handle this
		return 1;

	case AP_RID_DIALOG_COLLABORATIONJOIN_REFRESH_BUTTON:
		// TODO: we really should refresh the buddies here as well, 
		// as they could pop up automatically as well (for example with a 
		// avahi backend)
		_refreshAllDocHandlesAsync();
		_setModel(_constructModel());
		
		// WM_COMMAND message processed - return 0
		return 0;

	default:
		// WM_COMMAND message not handled - return 1 to require Windows to process it.
		return 1;
	}
}

BOOL AP_Win32Dialog_CollaborationJoin::_onNotify(HWND hWnd, WPARAM wParam, LPARAM lParam){
	switch (((LPNMHDR)lParam)->code)
	{
		//case UDN_DELTAPOS:		return pThis->_onDeltaPos((NM_UPDOWN *)lParam);
		//UT_DEBUGMSG(("Notify: Code=0x%x\n", ((LPNMHDR)lParam)->code));
		
		case NM_DBLCLK:
			// A double-click will toggle join status
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
				_setModel(_constructModel());
			}
			return 1;
			
		case TVN_SELCHANGED:
			_updateSelection();
			return 1;
			
		default:
			return 0;
	}

}

std::map<UT_UTF8String, ShareListItem> AP_Win32Dialog_CollaborationJoin::_constructModel()
{
	std::map<UT_UTF8String, ShareListItem> mModel;
	UT_UTF8String currentEntry;
	ShareListItem currentItem;

	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	const UT_GenericVector<AccountHandler*>& accounts = pManager->getAccounts();

	// Loop through accounts
	for (UT_sint32 i = 0; i < accounts.getItemCount(); i++)
	{
		// Loop through buddies in accounts
		for (UT_sint32 j = 0; j < accounts.getNthItem(i)->getBuddies().size(); j++)
		{
			const Buddy* pBuddy = accounts.getNthItem(i)->getBuddies()[j];
			currentEntry = pBuddy->getDescription();
			currentItem.pBuddy=pBuddy;
			currentItem.pDocHandle=NULL;
			mModel[currentEntry]=currentItem;
			
			// Loop through documents for each buddy
			for (const DocTreeItem* item = pBuddy->getDocTreeItems(); item; item = item->m_next)
			{
				if (item->m_docHandle)
				{
					if (item->m_docHandle)
					{
						currentEntry = item->m_docHandle->getName();
					}
					else
					{
						currentEntry ="null";
					}
					currentItem.pDocHandle=item->m_docHandle;
					
					mModel[currentEntry]=currentItem;
				}
			}
		}
	}
	
	return mModel;
}

void AP_Win32Dialog_CollaborationJoin::_setModel(std::map<UT_UTF8String, ShareListItem> model)
{
	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	TV_INSERTSTRUCT tvinsert;
	HTREEITEM htiPrevBuddy(NULL);
	HTREEITEM htiCurrent;
	UT_UTF8String sTextItem;
	ShareListItem item;
	char szBuf[256];
	std::map<UT_UTF8String, ShareListItem>::iterator iter=model.begin();
	std::map<UT_UTF8String, ShareListItem>::iterator end=model.end();
	
	// clear map
	m_mTreeItemHandles.clear();
	
	// clear the treeview
	DWORD styles = GetWindowLong(m_hDocumentTreeview, GWL_STYLE);
	TreeView_DeleteAllItems(m_hDocumentTreeview);
	SetWindowLong(m_hDocumentTreeview, GWL_STYLE, styles);
	

	
	tvinsert.item.mask=TVIF_TEXT| TVIF_STATE; // text only right now
	tvinsert.item.stateMask=TVIS_BOLD|TVIS_EXPANDED;
	tvinsert.hInsertAfter=TVI_LAST;  // only insert at the end
	
	while (iter!=end)
	{
		sTextItem=(iter->first);
		item=(iter->second);
		memset(szBuf, 0, 256);
		if (item.pDocHandle)
		{
			// This must be a Document - it has a doc handle
			
			// If the prev buddy item is null, we have serious issues: constructModel returned a document before its buddy
			UT_return_if_fail(htiPrevBuddy);
			strcpy(szBuf, AP_Win32App::s_fromUTF8ToWinLocale(sTextItem.utf8_str()).c_str());
			tvinsert.hParent=htiPrevBuddy;
			tvinsert.hInsertAfter=TVI_LAST;
			tvinsert.item.pszText=szBuf;
			// if we are connected to this document, bold it.  Eventually checkboxes would be cooler, that's a TODO
			if (pManager->isActive(item.pDocHandle->getSessionId()))
			{
				tvinsert.item.state=TVIS_BOLD;
			}
			else
			{
				tvinsert.item.state=TVIS_EXPANDED;
			}
			htiCurrent=(HTREEITEM)SendMessage(m_hDocumentTreeview, TVM_INSERTITEM,0,(LPARAM)&tvinsert);
			
		}
		else
		{
			// If doc handle is null for this entry in the model, this is a Buddy
			strcpy(szBuf, AP_Win32App::s_fromUTF8ToWinLocale(sTextItem.utf8_str()).c_str());
			tvinsert.hParent=NULL; // top most level Item
			tvinsert.item.state=NULL;
			tvinsert.item.pszText=szBuf;
			htiCurrent=(HTREEITEM)SendMessage(m_hDocumentTreeview, TVM_INSERTITEM,0,(LPARAM)&tvinsert);
			htiPrevBuddy=htiCurrent;
		}
		// Either way, add it to our map.
		m_mTreeItemHandles[htiCurrent]=(iter->second);
		iter++;
	}
	
	m_mModel = model;
	
	// Now expand all of the buddies
	std::map<HTREEITEM, ShareListItem>::iterator dispiter=m_mTreeItemHandles.begin();
	std::map<HTREEITEM, ShareListItem>::iterator dispend=m_mTreeItemHandles.end();
	while (dispiter!=dispend)
	{
		if (!(dispiter->second).pDocHandle)
		{
			// if the current doc handle is null, this is a buddy, expand it.
			 TreeView_Expand(m_hDocumentTreeview, (dispiter->first), TVE_EXPAND);
		}
		dispiter++;
	}
	
	_updateSelection();
	

}

void AP_Win32Dialog_CollaborationJoin::_refreshWindow()
{
	_setModel(_constructModel());
}

void AP_Win32Dialog_CollaborationJoin::_enableBuddyAddition(bool bEnabled)
{
	p_win32Dialog->enableControl(AP_RID_DIALOG_COLLABORATIONJOIN_ADDBUDDY_BUTTON, bEnabled);
}

void AP_Win32Dialog_CollaborationJoin::_updateSelection()
{
	
	HTREEITEM hSelItem = TreeView_GetSelection(m_hDocumentTreeview);
	if (hSelItem)
	{
		m_bHasSelection = true;
		m_hSelected = hSelItem;
		if (m_mTreeItemHandles[hSelItem].pDocHandle)
		{
			// If the doc handle isn't null then this is a document
			m_bShareSelected=true;
			p_win32Dialog->enableControl(AP_RID_DIALOG_COLLABORATIONJOIN_JOIN_BUTTON, true);
			p_win32Dialog->enableControl(AP_RID_DIALOG_COLLABORATIONJOIN_DELETE_BUTTON, false);
		}
		else
		{
			// This is just a buddy.
			m_bShareSelected=false;
			p_win32Dialog->enableControl(AP_RID_DIALOG_COLLABORATIONJOIN_JOIN_BUTTON, false);
			p_win32Dialog->enableControl(AP_RID_DIALOG_COLLABORATIONJOIN_DELETE_BUTTON, true);
		}
	}
	else
	{
		m_bHasSelection = false;
		m_bShareSelected = false;
		p_win32Dialog->enableControl(AP_RID_DIALOG_COLLABORATIONJOIN_JOIN_BUTTON, false);
		p_win32Dialog->enableControl(AP_RID_DIALOG_COLLABORATIONJOIN_DELETE_BUTTON, false);
	}

}
bool AP_Win32Dialog_CollaborationJoin::_setJoin(HTREEITEM hItem, bool joinStatus)
{
	// Return value: 	true if the join status was updated
	//				false if the desired status was already the current status
	Buddy* pBuddy=m_mTreeItemHandles[hItem].pBuddy;
	DocHandle* pDocHandle=m_mTreeItemHandles[hItem].pDocHandle;
	AbiCollabSessionManager* pManager = AbiCollabSessionManager::getManager();
	
	UT_return_if_fail(pBuddy);
	UT_return_if_fail(pDocHandle);
	UT_return_if_fail(pManager);
	
	bool currentlyJoined=pManager->isActive(pDocHandle->getSessionId());
	
	if (joinStatus && !currentlyJoined)
	{
		_join(pBuddy, pDocHandle);
		return true;
	}
	
	if (!joinStatus && currentlyJoined)
	{
		_disjoin(pBuddy, pDocHandle);
		return true;
	}
	
	// We did not have to change the status
	return false;
}
