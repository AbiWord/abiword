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

#include "ap_Win32Res_DlgGenericInput.rc2"
#include "ap_Win32Dialog_GenericInput.h"

BOOL CALLBACK AP_Win32Dialog_GenericInput::s_dlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_INITDIALOG:
		{
			AP_Win32Dialog_GenericInput* pThis = (AP_Win32Dialog_GenericInput *)lParam;
			UT_return_val_if_fail(pThis, false);
			SetWindowLongPtrW(hWnd,DWLP_USER,lParam);
			return pThis->_onInitDialog(hWnd,wParam,lParam);
		}
		case WM_COMMAND:
		{
			AP_Win32Dialog_GenericInput* pThis = (AP_Win32Dialog_GenericInput *)GetWindowLongPtrW(hWnd,DWLP_USER);
			UT_return_val_if_fail(pThis, false);
			return pThis->_onCommand(hWnd,wParam,lParam);
		}
		case WM_DESTROY:
		{
			AP_Win32Dialog_GenericInput* pThis = (AP_Win32Dialog_GenericInput *)GetWindowLongPtrW(hWnd,DWLP_USER);
			UT_return_val_if_fail(pThis, false);
			DELETEP(pThis->m_pWin32Dialog);
			return true;
		}		
		default:
			// Message not processed - Windows should take care of it
			return false;
		}
}

XAP_Dialog * AP_Win32Dialog_GenericInput::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	return static_cast<XAP_Dialog *>(new AP_Win32Dialog_GenericInput(pFactory, id));
}
pt2Constructor ap_Dialog_GenericInput_Constructor = &AP_Win32Dialog_GenericInput::static_constructor;

AP_Win32Dialog_GenericInput::AP_Win32Dialog_GenericInput(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: AP_Dialog_GenericInput(pDlgFactory, id),
	m_pWin32Dialog(NULL),
	m_hInstance(NULL),
	m_hWnd(NULL),
	m_hOk(NULL)
{
	AbiCollabSessionManager * pSessionManager= AbiCollabSessionManager::getManager();
	if (pSessionManager)
	{
		m_hInstance=pSessionManager->getInstance();
	}
}

void AP_Win32Dialog_GenericInput::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(m_hInstance);

	// default value
	m_answer = AP_Dialog_GenericInput::a_CANCEL;		

	// create the dialog
	HWND hWndParent = pFrame ? static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow() : NULL;
	int result = DialogBoxParamA( m_hInstance, AP_RID_DIALOG_GENERICINPUT, hWndParent,
		(DLGPROC)s_dlgProc, (LPARAM)this );
	switch (result)
	{
		case 0:
			// MSDN: If the function fails because the hWndParent parameter is invalid, the return value is zero.
			break;
		case -1:
			UT_DEBUGMSG(("Win32 error: %d, RID: %s\n", GetLastError(), AP_RID_DIALOG_GENERICINPUT));
			break;
		default:
			break;
	};
}

BOOL AP_Win32Dialog_GenericInput::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	m_hOk = GetDlgItem(hWnd, AP_RID_DIALOG_GENERICINPUT_OK_BUTTON);
	UT_return_val_if_fail(m_hOk, false);

	m_hWnd = hWnd;

	// Get ourselves a custom DialogHelper
	DELETEP(m_pWin32Dialog);
	m_pWin32Dialog = new XAP_Win32DialogHelper(hWnd);
	
	// Center Window
	m_pWin32Dialog->centerDialog();

	// set the dialog title
	m_pWin32Dialog->setDialogTitle(getTitle().utf8_str());

	// set the question
	SetDlgItemTextA(hWnd, AP_RID_DIALOG_GENERICINPUT_QUESTION, getQuestion().utf8_str());
	SetDlgItemTextA(hWnd, AP_RID_DIALOG_GENERICINPUT_LABEL, getLabel().utf8_str());

	// set the password char for the password field
	if (isPassword())
	{
		HWND hPasswordEntry = GetDlgItem(hWnd, AP_RID_DIALOG_GENERICINPUT_PASSWORD_EDIT);
		UT_return_val_if_fail(hPasswordEntry != NULL, false)
		SendMessage(hPasswordEntry, EM_SETPASSWORDCHAR, '*', 0);
	}

	// set the initial input
	SetDlgItemTextA(hWnd, AP_RID_DIALOG_GENERICINPUT_PASSWORD_EDIT, getInput().utf8_str());

	// force the initial sensitivy state of the buttons
	_eventTextChanged(); 

	return true;
}

BOOL AP_Win32Dialog_GenericInput::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	
	switch (wId)
	{
	case AP_RID_DIALOG_GENERICINPUT_OK_BUTTON:
		setInput(_getText(hWnd, AP_RID_DIALOG_GENERICINPUT_PASSWORD_EDIT));
		m_answer = AP_Dialog_GenericInput::a_OK;
		EndDialog(hWnd, 0);
		return true;
	case AP_RID_DIALOG_GENERICINPUT_CANCEL_BUTTON:
		m_answer = AP_Dialog_GenericInput::a_CANCEL;
		EndDialog(hWnd, 0);
		return true;
	case AP_RID_DIALOG_GENERICINPUT_PASSWORD_EDIT:
		if (wNotifyCode == EN_UPDATE)
		{
			_eventTextChanged();
			return true;
		}
		return false;
	default:
		return false;
	}
}

UT_UTF8String AP_Win32Dialog_GenericInput::_getText(HWND hWnd, int nID)
{
	UT_return_val_if_fail(hWnd, UT_UTF8String());

	const int buflen = 4096;
	char szBuff[buflen];
	*szBuff=0;
	GetDlgItemTextA(hWnd, nID, szBuff, buflen);
	szBuff[buflen-1] = '\0';
	return AP_Win32App::s_fromWinLocaleToUTF8(szBuff);
}

void AP_Win32Dialog_GenericInput::_eventTextChanged()
{
	UT_return_if_fail(m_hOk);

	if (_getText(m_hWnd, AP_RID_DIALOG_GENERICINPUT_PASSWORD_EDIT).length() < getMinLenght())
		EnableWindow(m_hOk, false);
	else
		EnableWindow(m_hOk, true);
}
