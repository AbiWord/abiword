/* Copyright (C) 2010 AbiSource Corporation B.V.
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
#include <account/xp/AccountHandler.h>

#include "ap_Win32Dialog_CollaborationEditAccount.h"

// We can't seem to pass user data to our custom message proc,
// which is why we have the static dialog pointer below
static AP_Win32Dialog_CollaborationEditAccount* s_pThis = NULL;

BOOL CALLBACK AP_Win32Dialog_CollaborationEditAccount::s_dlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			AP_Win32Dialog_CollaborationEditAccount* pThis = (AP_Win32Dialog_CollaborationEditAccount *)lParam;
			UT_return_val_if_fail(pThis, false);
			SetWindowLongPtrW(hWnd,DWLP_USER,lParam);
			return pThis->_onInitDialog(hWnd,wParam,lParam);
		}
		case WM_COMMAND:
		{
			AP_Win32Dialog_CollaborationEditAccount* pThis = (AP_Win32Dialog_CollaborationEditAccount *)GetWindowLongPtrW(hWnd,DWLP_USER);
			UT_return_val_if_fail(pThis, false);
			return pThis->_onCommand(hWnd,wParam,lParam);
		}
		case WM_DESTROY:
		{
			UT_DEBUGMSG(("Got WM_DESTROY\n"));
			AP_Win32Dialog_CollaborationEditAccount* pThis = (AP_Win32Dialog_CollaborationEditAccount *)GetWindowLongPtrW(hWnd,DWLP_USER);
			UT_return_val_if_fail(pThis, false);
			DELETEP(pThis->m_pWin32Dialog);
			return true;
		}
		default:
			return false;
	}
}

BOOL CALLBACK AP_Win32Dialog_CollaborationEditAccount::s_detailsProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	AP_Win32Dialog_CollaborationEditAccount* pThis = (AP_Win32Dialog_CollaborationEditAccount *)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
	UT_return_val_if_fail(pThis, false);

	switch (msg)
	{
		case WM_COMMAND:
			if (pThis->_onDetailsCommand(hWnd, msg, wParam, lParam))
				return true;
			return pThis->detailsProc(hWnd, msg, wParam, lParam);
		default:
			return pThis->detailsProc(hWnd, msg, wParam, lParam);
	}
}

XAP_Dialog * AP_Win32Dialog_CollaborationEditAccount::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	return static_cast<XAP_Dialog *>(new AP_Win32Dialog_CollaborationEditAccount(pFactory, id));
}
pt2Constructor ap_Dialog_CollaborationEditAccount_Constructor = &AP_Win32Dialog_CollaborationEditAccount::static_constructor;

AP_Win32Dialog_CollaborationEditAccount::AP_Win32Dialog_CollaborationEditAccount(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: AP_Dialog_CollaborationEditAccount(pDlgFactory, id),
	m_pWin32Dialog(NULL),
	m_hInstance(NULL),
	m_hOk(NULL),
	m_pOldDetailsProc(NULL),
	m_hWnd(NULL),
	m_hDetails(NULL),
	m_hDetailsHook(NULL)
{
	AbiCollabSessionManager * pSessionManager = AbiCollabSessionManager::getManager();
	if (pSessionManager)
	{
		m_hInstance=pSessionManager->getInstance();
	}

	// hack
	s_pThis = this;
}

void AP_Win32Dialog_CollaborationEditAccount::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);
	UT_return_if_fail(m_hInstance);

	// create the dialog
	LPCTSTR lpTemplate = MAKEINTRESOURCE(AP_RID_DIALOG_COLLABORATIONEDITACCOUNT);
	int result = DialogBoxParam( m_hInstance, lpTemplate,
		static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow(),
		(DLGPROC)s_dlgProc, (LPARAM)this );
	switch (result)
	{
		case 0:
			// MSDN: If the function fails because the hWndParent parameter is invalid, the return value is zero.
			break;
		case -1:
			UT_DEBUGMSG(("Win32 error: %d.  lpTemplate: %d, RID:%d\n", GetLastError(), lpTemplate, AP_RID_DIALOG_COLLABORATIONEDITACCOUNT));
			break;
		default:
			break;
	};

	UnhookWindowsHookEx(m_hDetailsHook);
	m_hDetailsHook = NULL;

	s_pThis = NULL;
}

/*****************************************************************/
BOOL AP_Win32Dialog_CollaborationEditAccount::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	// Store handles for easy access
	m_hWnd = hWnd;
	m_hOk = GetDlgItem(hWnd, AP_RID_DIALOG_COLLABORATIONEDITACCOUNT_OK_BUTTON);
	UT_return_val_if_fail(m_hOk, false);
	m_hDetails = GetDlgItem(hWnd, AP_RID_DIALOG_COLLABORATIONEDITACCOUNT_DETAILS_BOX);
	UT_return_val_if_fail(m_hDetails, false);

	// trap the messages that will be sent to the m_hDetails window, so we can forward
	// them to the account handler
	m_pOldDetailsProc = GetWindowLongPtrW(m_hDetails, GWLP_WNDPROC);
	SetWindowLongPtrW(m_hDetails, GWLP_WNDPROC, (LPARAM)s_detailsProc);
	SetWindowLongPtrW(m_hDetails, GWLP_USERDATA, (LPARAM)this);

	// we need to trap the account details window's message hook,
	// so we can make TAB stops work
	m_hDetailsHook = SetWindowsHookEx(WH_GETMESSAGE, (HOOKPROC)s_detailsGetMsgProc, m_hInstance, GetCurrentThreadId());

	// Get ourselves a custom DialogHelper
	DELETEP(m_pWin32Dialog);
	m_pWin32Dialog = new XAP_Win32DialogHelper(hWnd);
	
	// Set up dialog initial state
	_populateWindowData();
	
	// Center Window
	m_pWin32Dialog->centerDialog();

	return true;
}

// return true if we process the command, false otherwise
BOOL AP_Win32Dialog_CollaborationEditAccount::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);

	AccountHandler* pHandler;
	switch (wId)
	{
	case AP_RID_DIALOG_COLLABORATIONEDITACCOUNT_OK_BUTTON:
		UT_DEBUGMSG(("Edit Account - OK button clicked\n"));
		pHandler = getAccountHandler();
		// if pHandler is no good, then we tell Windows we didn't handle the button
		UT_return_val_if_fail(pHandler, 1);
		pHandler->storeProperties();
		m_answer=AP_Dialog_CollaborationEditAccount::a_OK;
		EndDialog(hWnd,0);
		return true;

	case AP_RID_DIALOG_COLLABORATIONEDITACCOUNT_CANCEL_BUTTON:
		// Close without applying changes
		EndDialog(hWnd,0);
		m_answer=AP_Dialog_CollaborationEditAccount::a_CANCEL;
		return true;

	default:
		// WM_COMMAND message NOT processed
		return false;
	}
}

BOOL AP_Win32Dialog_CollaborationEditAccount::_onDetailsCommand(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	AccountHandler* pAccountHandler = getAccountHandler();
	UT_return_val_if_fail(pAccountHandler, false);
	return pAccountHandler->_onCommand(hWnd, wParam, lParam);
}

BOOL AP_Win32Dialog_CollaborationEditAccount::detailsProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	return CallWindowProc((WNDPROC)m_pOldDetailsProc, hWnd, msg, wParam, lParam);
}

LRESULT AP_Win32Dialog_CollaborationEditAccount::s_detailsGetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	if (s_pThis)
		return s_pThis->detailsGetMsgProc(nCode, wParam, lParam);
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

LRESULT AP_Win32Dialog_CollaborationEditAccount::detailsGetMsgProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	LPMSG lpMsg = (LPMSG)lParam;

	if (nCode >= 0 && PM_REMOVE == wParam)
	{
		// Don't translate non-input events.
		if ((lpMsg->message >= WM_KEYFIRST && lpMsg->message <= WM_KEYLAST))
		{
			AccountHandler* pAccount = getAccountHandler();
			HWND hwnd = pAccount && pAccount->shouldProcessFocus() ? m_hDetails : m_hWnd;
			if (IsDialogMessage(hwnd, lpMsg))
			{
				// The value returned from this hookproc is ignored, 
				// and it cannot be used to tell Windows the message
				// has been handled. To avoid further processing,
				// convert the message to WM_NULL before returning.
				lpMsg->message = WM_NULL;
				lpMsg->lParam = 0;
				lpMsg->wParam  = 0;
			}
		}
	}
	return CallNextHookEx(NULL, nCode, wParam, lParam);
}

void AP_Win32Dialog_CollaborationEditAccount::_populateWindowData()
{
	UT_DEBUGMSG(("AP_Win32Dialog_CollaborationEditAccount::_populateWindowData()\n"));
	
	AbiCollabSessionManager* pSessionManager = AbiCollabSessionManager::getManager();
	UT_return_if_fail(pSessionManager);
	
	UT_return_if_fail(m_pWin32Dialog);
	
	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
}

void* AP_Win32Dialog_CollaborationEditAccount::_getEmbeddingParent()
{
	return m_hDetails;
}

void AP_Win32Dialog_CollaborationEditAccount::setBackendValidity(bool valid)
{
	EnableWindow(m_hOk, valid);
}
