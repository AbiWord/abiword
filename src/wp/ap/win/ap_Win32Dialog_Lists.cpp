/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_timer.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Lists.h"
#include "ap_Win32Dialog_Lists.h"

#include "ap_Win32Resources.rc2"
#include "xap_Win32Toolbar_Icons.h"

/*****************************************************************/

XAP_Dialog * AP_Win32Dialog_Lists::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id id)
{
	AP_Win32Dialog_Lists * p = new AP_Win32Dialog_Lists(pFactory,id);
	return p;
}

AP_Win32Dialog_Lists::AP_Win32Dialog_Lists(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id id)
	: AP_Dialog_Lists(pDlgFactory,id), _win32Dialog(this)
{
}

AP_Win32Dialog_Lists::~AP_Win32Dialog_Lists(void)
{
	m_pAutoUpdateLists->stop();
	DELETEP(m_pAutoUpdateLists);
}

/*****************************************************************/

void AP_Win32Dialog_Lists::runModeless(XAP_Frame * pFrame)
{
	// raise the dialog

	_win32Dialog.runModeless(pFrame, AP_DIALOG_ID_LISTS, AP_RID_DIALOG_LIST, this);

}


#define _DS(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL AP_Win32Dialog_Lists::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	_fillListTypeMenu( AP_RID_DIALOG_LIST_COMBO_NEW_LIST_TYPE);
	_fillListTypeMenu( AP_RID_DIALOG_LIST_COMBO_CURRENT_LIST_TYPE);

	_win32Dialog.selectComboItem(AP_RID_DIALOG_LIST_COMBO_NEW_LIST_TYPE, 0);

	_win32Dialog.setControlInt( AP_RID_DIALOG_LIST_EDIT_NEW_STARTING_VALUE, 1);
	
	enableControls();

	m_pAutoUpdateLists = UT_Timer::static_constructor(autoupdateLists, this);
	m_pAutoUpdateLists->set(500);

	return 1;							// 1 == we did not call SetFocus()
}

BOOL AP_Win32Dialog_Lists::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;

	switch (wId)
	{
	case AP_RID_DIALOG_LIST_CHECK_START_NEW_LIST:
		_win32Dialog.checkButton(AP_RID_DIALOG_LIST_CHECK_STOP_CURRENT_LIST, FALSE);
		enableControls();
		return 1;

	case AP_RID_DIALOG_LIST_CHECK_STOP_CURRENT_LIST:
		_win32Dialog.checkButton(AP_RID_DIALOG_LIST_CHECK_START_NEW_LIST, FALSE);
		enableControls();
		return 1;

	case AP_RID_DIALOG_LIST_BTN_CLOSE:
	case IDCANCEL:
		_win32Dialog.showWindow( SW_HIDE );
		return 1;

	case AP_RID_DIALOG_LIST_BTN_APPLY:
		_onApply();
		return 1;

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}

BOOL AP_Win32Dialog_Lists::_onDeltaPos(NM_UPDOWN * pnmud)
{
	return 0;
}

void AP_Win32Dialog_Lists::destroy(void)
{
	_win32Dialog.destroyWindow();
}

void AP_Win32Dialog_Lists::activate(void)
{
	int iResult;

	// Update the caption
	ConstructWindowName();
	_win32Dialog.setDialogTitle(m_WindowName);

	iResult = _win32Dialog.showWindow( SW_SHOW );

	iResult = _win32Dialog.bringWindowToTop();

	UT_ASSERT((iResult != 0));
}

/*
void AP_Win32Dialog_Lists::notifyActiveFrame(XAP_Frame *pFrame)
{
	XAP_Win32Frame * pWin32Frame = static_cast<XAP_Win32Frame *>(pFrame);
	if((HWND)GetWindowLong(m_hDlg, GWL_HWNDPARENT) != pWin32Frame->getTopLevelWindow())
	{
		// Update the caption
		ConstructWindowName();
		SetWindowText(m_hDlg, m_WindowName);

		SetWindowLong(m_hDlg, GWL_HWNDPARENT, (long)pWin32Frame->getTopLevelWindow());
		SetWindowPos(m_hDlg, NULL, 0, 0, 0, 0,
						SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}
}

void AP_Win32Dialog_Lists::notifyCloseFrame(XAP_Frame *pFrame)
{
	XAP_Win32Frame * pWin32Frame = static_cast<XAP_Win32Frame *>(pFrame);
	if((HWND)GetWindowLong(m_hDlg, GWL_HWNDPARENT) == pWin32Frame->getTopLevelWindow())
	{
		SetWindowLong(m_hDlg, GWL_HWNDPARENT, NULL);
		SetWindowPos(m_hDlg, NULL, 0, 0, 0, 0,
						SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}
}
*/

void AP_Win32Dialog_Lists::_fillListTypeMenu( int List_id)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	_win32Dialog.addItemToCombo(List_id, pSS->getValue(AP_STRING_ID_DLG_Lists_Numbered_List));
	_win32Dialog.addItemToCombo(List_id, pSS->getValue(AP_STRING_ID_DLG_Lists_Lower_Case_List));
	_win32Dialog.addItemToCombo(List_id, pSS->getValue(AP_STRING_ID_DLG_Lists_Upper_Case_List));
	_win32Dialog.addItemToCombo(List_id, pSS->getValue(AP_STRING_ID_DLG_Lists_Bullet_List));

}

void AP_Win32Dialog_Lists::enableControls(void)
{ 
	PopulateDialogData();

	UT_Bool bStartChecked = _win32Dialog.isChecked(AP_RID_DIALOG_LIST_CHECK_START_NEW_LIST);
	UT_Bool bStopChecked = _win32Dialog.isChecked(AP_RID_DIALOG_LIST_CHECK_STOP_CURRENT_LIST);

	_win32Dialog.enableControl(AP_RID_DIALOG_LIST_CHECK_START_NEW_LIST, !bStopChecked);

	_win32Dialog.enableControl( AP_RID_DIALOG_LIST_COMBO_NEW_LIST_TYPE, bStartChecked);
	_win32Dialog.enableControl( AP_RID_DIALOG_LIST_EDIT_NEW_STARTING_VALUE, bStartChecked);

	_win32Dialog.enableControl( AP_RID_DIALOG_LIST_COMBO_CURRENT_LIST_TYPE, m_isListAtPoint && !bStartChecked && !bStopChecked);
	_win32Dialog.selectComboItem(AP_RID_DIALOG_LIST_COMBO_CURRENT_LIST_TYPE, (UT_uint32) m_iListType);

	_win32Dialog.enableControl( AP_RID_DIALOG_LIST_EDIT_CURRENT_STARTING_VALUE, m_isListAtPoint && !bStartChecked && !bStopChecked);
	_win32Dialog.setControlInt( AP_RID_DIALOG_LIST_EDIT_CURRENT_STARTING_VALUE, m_curStartValue);

	_win32Dialog.enableControl( AP_RID_DIALOG_LIST_CHECK_CHANGE_CURRENT_LIST, m_isListAtPoint && !bStartChecked);
	_win32Dialog.showControl( AP_RID_DIALOG_LIST_CHECK_CHANGE_CURRENT_LIST, SW_HIDE);
	_win32Dialog.enableControl( AP_RID_DIALOG_LIST_CHECK_STOP_CURRENT_LIST, m_isListAtPoint && !bStartChecked);

}

void AP_Win32Dialog_Lists::_onApply()
{
	m_bStartList = _win32Dialog.isChecked(AP_RID_DIALOG_LIST_CHECK_START_NEW_LIST);
	m_bStopList = _win32Dialog.isChecked(AP_RID_DIALOG_LIST_CHECK_STOP_CURRENT_LIST);
	m_bChangeStartValue = UT_FALSE;

	if(m_bStartList)
	{
		m_newStartValue = _win32Dialog.getControlInt(AP_RID_DIALOG_LIST_EDIT_NEW_STARTING_VALUE);
		m_iListType = (List_Type) _win32Dialog.getComboSelectedIndex(AP_RID_DIALOG_LIST_COMBO_NEW_LIST_TYPE);
		if(m_iListType == NUMBERED_LIST)
		{
			strcpy(m_newListType, "%*%d.");
		}
		else if (m_iListType == LOWERCASE_LIST)
		{
			strcpy(m_newListType,"%*%a.");
		}
		else if (m_iListType == UPPERCASE_LIST)
		{
			strcpy(m_newListType,"%*%A.");
		}
		else if (m_iListType == BULLETED_LIST)
		{
			m_curStartValue = 1;
			strcpy(m_newListType, "%b");
		}

	}
	else
	{

		if(m_bStopList)
		{
		}
		else
		{
			int newStartValue = _win32Dialog.getControlInt(AP_RID_DIALOG_LIST_EDIT_CURRENT_STARTING_VALUE);
			int newListType = _win32Dialog.getComboSelectedIndex(AP_RID_DIALOG_LIST_COMBO_NEW_LIST_TYPE);

			if(newStartValue != m_curStartValue && newListType != (UT_uint32) m_iListType)
			{
				m_bChangeStartValue = UT_TRUE;
				m_curStartValue = newStartValue;
				if(newListType == (int) NUMBERED_LIST)
				{
					strcpy(m_newListType, "%*%d.");
				}
				else if (newListType == (int) LOWERCASE_LIST)
				{
					strcpy(m_newListType,"%*%a.");
				}
				else if (newListType == (int) UPPERCASE_LIST)
				{
					strcpy(m_newListType,"%*%A.");
				}
				else if (newListType == (int) BULLETED_LIST)
				{
					m_curStartValue = 1;
					strcpy(m_newListType, "%b");
				}
			}

		}
	}

	Apply();

	_win32Dialog.checkButton(AP_RID_DIALOG_LIST_CHECK_START_NEW_LIST, FALSE);
	_win32Dialog.checkButton(AP_RID_DIALOG_LIST_CHECK_STOP_CURRENT_LIST, FALSE);
	enableControls();
		
}

void AP_Win32Dialog_Lists::autoupdateLists(UT_Timer * pTimer)
{
	UT_ASSERT(pTimer);
	// this is a static callback method and does not have a 'this' pointer.
	AP_Win32Dialog_Lists * pDialog =  (AP_Win32Dialog_Lists *) pTimer->getInstanceData();

	pDialog->enableControls();
}
