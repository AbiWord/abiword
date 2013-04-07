/* Copyright (C) 2008 AbiSource Corporation B.V.
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

#include "xap_App.h"
#include "ap_Win32App.h"
#include "xap_Win32App.h"
#include "xap_Frame.h"
#include "xap_Win32DialogHelper.h"
#include "ut_string_class.h"
#include <session/xp/AbiCollabSessionManager.h>

#include "ap_Win32Res_DlgGenericProgress.rc2"
#include "ap_Win32Dialog_GenericProgress.h"

BOOL CALLBACK AP_Win32Dialog_GenericProgress::s_dlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			AP_Win32Dialog_GenericProgress* pThis = (AP_Win32Dialog_GenericProgress *)lParam;
			UT_return_val_if_fail(pThis, false);
			SetWindowLongPtrW(hWnd,DWLP_USER,lParam);
			return pThis->_onInitDialog(hWnd,wParam,lParam);
		}
		case WM_COMMAND:
		{
			AP_Win32Dialog_GenericProgress* pThis = (AP_Win32Dialog_GenericProgress *)GetWindowLongPtrW(hWnd,DWLP_USER);
			UT_return_val_if_fail(pThis, false);
			return pThis->_onCommand(hWnd,wParam,lParam);
		}
		case WM_DESTROY:
		{
			AP_Win32Dialog_GenericProgress* pThis = (AP_Win32Dialog_GenericProgress *)GetWindowLongPtrW(hWnd,DWLP_USER);
			UT_return_val_if_fail(pThis, false);
			DELETEP(pThis->m_pWin32Dialog);
			return true;
		}		
		default:
			// Message not processed - Windows should take care of it
			return false;
		}
}

XAP_Dialog * AP_Win32Dialog_GenericProgress::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	return static_cast<XAP_Dialog *>(new AP_Win32Dialog_GenericProgress(pFactory, id));
}
pt2Constructor ap_Dialog_GenericProgress_Constructor = &AP_Win32Dialog_GenericProgress::static_constructor;

AP_Win32Dialog_GenericProgress::AP_Win32Dialog_GenericProgress(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: AP_Dialog_GenericProgress(pDlgFactory, id),
	m_pWin32Dialog(NULL),
	m_hInstance(NULL),
	m_hWnd(NULL),
	m_hProgress(NULL)
{
	AbiCollabSessionManager * pSessionManager= AbiCollabSessionManager::getManager();
	if (pSessionManager)
	{
		m_hInstance=pSessionManager->getInstance();
	}
}

void AP_Win32Dialog_GenericProgress::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);
	UT_return_if_fail(m_hInstance);

	// default value
	m_answer = AP_Dialog_GenericProgress::a_CANCEL;		

	// create the dialog
	int result = DialogBoxParamA( m_hInstance, AP_RID_DIALOG_GENERICPROGRESS,
		static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow(),
		(DLGPROC)s_dlgProc, (LPARAM)this );
	switch (result)
	{
		case 0:
			// MSDN: If the function fails because the hWndParent parameter is invalid, the return value is zero.
			break;
		case -1:
			UT_DEBUGMSG(("Win32 error: %d, RID: %s\n", GetLastError(), AP_RID_DIALOG_GENERICPROGRESS));
			break;
		default:
			break;
	};
}

void AP_Win32Dialog_GenericProgress::close(bool cancel)
{
	UT_DEBUGMSG(("AP_Win32Dialog_GenericProgress::close()\n"));
	UT_return_if_fail(m_hWnd);
	m_answer = cancel ? AP_Dialog_GenericProgress::a_CANCEL : AP_Dialog_GenericProgress::a_OK;
	EndDialog(m_hWnd, 0);
}

void AP_Win32Dialog_GenericProgress::setProgress(UT_uint32 progress)
{
	UT_DEBUGMSG(("AP_Win32Dialog_GenericProgress::setProgress() - progress: %d\n", progress));
	UT_return_if_fail(progress >= 0 && progress <= 100);
	SendMessage(m_hProgress, PBM_SETPOS, progress, 0);
}

BOOL AP_Win32Dialog_GenericProgress::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	// Get ourselves a custom DialogHelper
	DELETEP(m_pWin32Dialog);
	m_pWin32Dialog = new XAP_Win32DialogHelper(hWnd);
	m_hWnd = hWnd;
	
	// Center Window
	m_pWin32Dialog->centerDialog();

	// set the dialog title
	m_pWin32Dialog->setDialogTitle(getTitle().utf8_str());

	// set the informative label
	SetDlgItemTextA(hWnd, AP_RID_DIALOG_GENERICINPUT_INFORMATION, getInformation().utf8_str());

	// add the progress bar
	RECT r;
	r.left = 8;
	r.top = 16;
	r.right = 8 + 184 - 1;
	r.bottom = 16 + 14 - 1;
	MapDialogRect(m_hWnd, &r);
	m_hProgress = CreateWindowExW(WS_EX_NOPARENTNOTIFY, PROGRESS_CLASSW, L"Progress", WS_CHILD | WS_GROUP | WS_VISIBLE,
	r.left, r.top, r.right - r.left + 1, r.bottom - r.top + 1, m_hWnd, (HMENU) AP_RID_DIALOG_GENERICPROGRESS_PROGRESS, m_hInstance, 0);
	UT_return_val_if_fail(m_hProgress, false);

	return true;
}

BOOL AP_Win32Dialog_GenericProgress::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wId = LOWORD(wParam);
	
	switch (wId)
	{
	case AP_RID_DIALOG_GENERICPROGRESS_CANCEL_BUTTON:
		m_answer = AP_Dialog_GenericProgress::a_CANCEL;
		EndDialog(hWnd, 0);
		return true;		
	default:
		return false;
	}
}
