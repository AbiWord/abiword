/* AbiSource Application Framework
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

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Break.h"
#include "ap_Win32Dialog_Break.h"

#include "ap_Win32Resources.rc2"

/*****************************************************************/

AP_Dialog * AP_Win32Dialog_Break::static_constructor(AP_DialogFactory * pFactory,
													   AP_Dialog_Id id)
{
	AP_Win32Dialog_Break * p = new AP_Win32Dialog_Break(pFactory,id);
	return p;
}

AP_Win32Dialog_Break::AP_Win32Dialog_Break(AP_DialogFactory * pDlgFactory,
											   AP_Dialog_Id id)
	: AP_Dialog_Break(pDlgFactory,id)
{
}

AP_Win32Dialog_Break::~AP_Win32Dialog_Break(void)
{
}

/*****************************************************************/

void AP_Win32Dialog_Break::runModal(XAP_Frame * pFrame)
{
	// raise the dialog
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);
	XAP_Win32Frame * pWin32Frame = static_cast<XAP_Win32Frame *>(pFrame);

	LPCTSTR lpTemplate = NULL;

	UT_ASSERT(m_id == AP_DIALOG_ID_BREAK);

	lpTemplate = MAKEINTRESOURCE(AP_RID_DIALOG_BREAK);

	int result = DialogBoxParam(pWin32App->getInstance(),lpTemplate,
								pWin32Frame->getTopLevelWindow(),
								(DLGPROC)s_dlgProc,(LPARAM)this);
	UT_ASSERT((result != -1));
}

BOOL CALLBACK AP_Win32Dialog_Break::s_dlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// This is a static function.

	AP_Win32Dialog_Break * pThis;
	
	switch (msg)
	{
	case WM_INITDIALOG:
		pThis = (AP_Win32Dialog_Break *)lParam;
		SetWindowLong(hWnd,DWL_USER,lParam);
		return pThis->_onInitDialog(hWnd,wParam,lParam);
		
	case WM_COMMAND:
		pThis = (AP_Win32Dialog_Break *)GetWindowLong(hWnd,DWL_USER);
		return pThis->_onCommand(hWnd,wParam,lParam);
		
	default:
		return 0;
	}
}

#define _DS(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL AP_Win32Dialog_Break::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	SetWindowText(hWnd, pSS->getValue(AP_STRING_ID_DLG_Break_BreakTitle));

	// localize controls
	_DSX(BREAK_BTN_OK,			DLG_OK);
	_DSX(BREAK_BTN_CANCEL,		DLG_Cancel);

	_DS(BREAK_TEXT_INSERT,		DLG_Break_Insert);
	_DS(BREAK_TEXT_SECTION,		DLG_Break_SectionBreaks);
	_DS(BREAK_RADIO_PAGE,		DLG_Break_PageBreak);
	_DS(BREAK_RADIO_COL	,		DLG_Break_ColumnBreak);
	_DS(BREAK_RADIO_NEXT,		DLG_Break_NextPage);
	_DS(BREAK_RADIO_CONT,		DLG_Break_Continuous);
	_DS(BREAK_RADIO_EVEN,		DLG_Break_EvenPage);
	_DS(BREAK_RADIO_ODD,		DLG_Break_OddPage);

	// set initial state
	CheckDlgButton(hWnd, AP_RID_DIALOG_BREAK_RADIO_PAGE + m_break, BST_CHECKED);

	return 1;							// 1 == we did not call SetFocus()
}

static int _getRBOffset(HWND hWnd, int nIDFirstButton, int nIDLastButton)
{
	UT_ASSERT(hWnd && IsWindow(hWnd));
	UT_ASSERT(nIDFirstButton < nIDLastButton);

	for (int i = nIDFirstButton; i <= nIDLastButton; i++)
		if (BST_CHECKED == IsDlgButtonChecked(hWnd, i))
			return (i - nIDFirstButton);

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return -1;
}

BOOL AP_Win32Dialog_Break::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;
	int n;

	switch (wId)
	{
	case IDCANCEL:						// also AP_RID_DIALOG_BREAK_BTN_CANCEL
		m_answer = a_CANCEL;
		// fall through

	case IDOK:							// also AP_RID_DIALOG_BREAK_BTN_OK
		n = _getRBOffset(hWnd, AP_RID_DIALOG_BREAK_RADIO_PAGE, AP_RID_DIALOG_BREAK_RADIO_ODD);
		UT_ASSERT(n >= 0);

		m_break = (AP_Dialog_Break::breakType) n;
		EndDialog(hWnd,0);
		return 1;

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}

