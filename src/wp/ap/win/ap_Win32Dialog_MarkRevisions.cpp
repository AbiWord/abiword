/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2002 Tomas Frydrych <tomas@frydrych.uklinux.net>
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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_MarkRevisions.h"
#include "ap_Win32Dialog_MarkRevisions.h"
#include "xap_Win32LabelledSeparator.h"
#include "xap_Win32DialogHelper.h"
#include "ap_Win32Resources.rc2"

/*****************************************************************/

XAP_Dialog * AP_Win32Dialog_MarkRevisions::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_Win32Dialog_MarkRevisions * p = new AP_Win32Dialog_MarkRevisions(pFactory,id);
	return p;
}

AP_Win32Dialog_MarkRevisions::AP_Win32Dialog_MarkRevisions(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_MarkRevisions(pDlgFactory,id)
{
}

AP_Win32Dialog_MarkRevisions::~AP_Win32Dialog_MarkRevisions(void)
{
}

void AP_Win32Dialog_MarkRevisions::runModal(XAP_Frame * pFrame)
{
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);
	XAP_Win32LabelledSeparator_RegisterClass(pWin32App);
	UT_return_if_fail (m_id == AP_DIALOG_ID_MARK_REVISIONS);

    createModal(pFrame, MAKEINTRESOURCEW(AP_RID_DIALOG_MARK_REVISIONS));
}

#define _DSX(c,s)	setDlgItemText(AP_RID_DIALOG_MARK_REVISIONS_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL AP_Win32Dialog_MarkRevisions::_onInitDialog(HWND hWnd, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	setDialogTitle (getTitle());

	// localize controls
	_DSX(BTN_OK,			DLG_OK);
	_DSX(BTN_CANCEL,		DLG_Cancel);

	char * pStr = getRadio1Label();
	if(pStr)
	{
		setDlgItemText(AP_RID_DIALOG_MARK_REVISIONS_RADIO1,pStr);
		FREEP(pStr);

		CheckDlgButton(hWnd, AP_RID_DIALOG_MARK_REVISIONS_RADIO1,BST_CHECKED);
		HWND h = GetDlgItem(hWnd,AP_RID_DIALOG_MARK_REVISIONS_RADIO1);
		SetFocus(h);

		pStr = getComment1();
		setDlgItemText(AP_RID_DIALOG_MARK_REVISIONS_LABEL1,pStr);
		FREEP(pStr);

		setDlgItemText(AP_RID_DIALOG_MARK_REVISIONS_RADIO2,getRadio2Label());

		//disable the edit box
		h = GetDlgItem(hWnd,AP_RID_DIALOG_MARK_REVISIONS_EDIT2);
		EnableWindow(h, FALSE);
	}
	else
	{
		// there are no exising revisions in this doc, so we hide
		// everything but the edit control
		HWND h = GetDlgItem(hWnd,AP_RID_DIALOG_MARK_REVISIONS_RADIO1);
		ShowWindow(h,SW_HIDE);

		h = GetDlgItem(hWnd,AP_RID_DIALOG_MARK_REVISIONS_RADIO2);
		ShowWindow(h,SW_HIDE);

		h = GetDlgItem(hWnd,AP_RID_DIALOG_MARK_REVISIONS_LABEL1);
		ShowWindow(h,SW_HIDE);

		h = GetDlgItem(hWnd,AP_RID_DIALOG_MARK_REVISIONS_EDIT2);
		EnableWindow(h, TRUE);

		//move the edit control and its label higher up
		SetWindowPos(h,0,30,80,0,0,SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER);
		SetFocus(h);

		h = GetDlgItem(hWnd,AP_RID_DIALOG_MARK_REVISIONS_LABEL2);
		SetWindowPos(h,0,30,60,0,0,SWP_NOOWNERZORDER | SWP_NOSIZE | SWP_NOZORDER);


	}

	setDlgItemText(AP_RID_DIALOG_MARK_REVISIONS_LABEL2,getComment2Label());


	centerDialog();	

	return 0;							// 1 == we did not call SetFocus()
}

static int _getRBOffset(HWND hWnd, int nIDFirstButton, int nIDLastButton)
{
	UT_return_val_if_fail (hWnd && IsWindow(hWnd), -1);
	UT_return_val_if_fail (nIDFirstButton < nIDLastButton, -1);

	for (int i = nIDFirstButton; i <= nIDLastButton; i++)
		if (BST_CHECKED == IsDlgButtonChecked(hWnd, i))
			return (i - nIDFirstButton);

	//UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return -1;
}

BOOL AP_Win32Dialog_MarkRevisions::_onCommand(HWND hWnd, WPARAM wParam, LPARAM /*lParam*/)
{
	WORD wId = LOWORD(wParam);
	HWND h;

	int n;

	switch (wId)
	{
	case IDCANCEL:
		m_answer = a_CANCEL;
		EndDialog(hWnd,0);
		return 1;

	case IDOK:
		n = _getRBOffset(hWnd,
						 AP_RID_DIALOG_MARK_REVISIONS_RADIO1,
						 AP_RID_DIALOG_MARK_REVISIONS_RADIO2);

		//UT_ASSERT(n >= 0);

		if(n == 1 || n == -1)
		{
            UT_Win32LocaleString str;
			getDlgItemText(hWnd, AP_RID_DIALOG_MARK_REVISIONS_EDIT2, str);
			setComment2(str.utf8_str().utf8_str());
		}

		m_answer = a_OK;

		EndDialog(hWnd,0);
		return 1;

	case AP_RID_DIALOG_MARK_REVISIONS_RADIO1:
		// disable the edit box
		h = GetDlgItem(hWnd,AP_RID_DIALOG_MARK_REVISIONS_EDIT2);
		EnableWindow(h, FALSE);
		return 0;

	case AP_RID_DIALOG_MARK_REVISIONS_RADIO2:
		// enable the edit box
		h = GetDlgItem(hWnd,AP_RID_DIALOG_MARK_REVISIONS_EDIT2);
		EnableWindow(h, TRUE);
		SetFocus(h);
		return 0;

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}


