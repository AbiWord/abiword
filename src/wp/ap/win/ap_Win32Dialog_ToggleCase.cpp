/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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
#include "xap_Win32FrameImpl.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_ToggleCase.h"
#include "ap_Win32Dialog_ToggleCase.h"

#include "ap_Win32Resources.rc2"

/*****************************************************************/

XAP_Dialog * AP_Win32Dialog_ToggleCase::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_Win32Dialog_ToggleCase * p = new AP_Win32Dialog_ToggleCase(pFactory,id);
	return p;
}

#ifdef _MSC_VER	// MSVC++ warns about using 'this' in initializer list.
#pragma warning(disable: 4355)
#endif

AP_Win32Dialog_ToggleCase::AP_Win32Dialog_ToggleCase(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_ToggleCase(pDlgFactory,id), m_iWhichCase(CASE_SENTENCE)
{
}

AP_Win32Dialog_ToggleCase::~AP_Win32Dialog_ToggleCase(void)
{
}

void AP_Win32Dialog_ToggleCase::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	UT_ASSERT(pFrame);
	UT_ASSERT(m_id == AP_DIALOG_ID_TOGGLECASE);
	setDialog(this);
	createModal(pFrame, MAKEINTRESOURCE(AP_RID_DIALOG_TOGGLECASE));


	
}

BOOL AP_Win32Dialog_ToggleCase::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	m_hThisDlg = hWnd;

	localizeDialogTitle(AP_STRING_ID_DLG_ToggleCase_Title);

	// localize controls
	localizeControlText(AP_RID_DIALOG_TOGGLECASE_BTN_OK,		XAP_STRING_ID_DLG_OK);
	localizeControlText(AP_RID_DIALOG_TOGGLECASE_BTN_CANCEL,	XAP_STRING_ID_DLG_Cancel);
	
	localizeControlText(AP_RID_DIALOG_TOGGLECASE_RDO_SentenceCase,	AP_STRING_ID_DLG_ToggleCase_SentenceCase);
	localizeControlText(AP_RID_DIALOG_TOGGLECASE_RDO_LowerCase,	AP_STRING_ID_DLG_ToggleCase_LowerCase);
	localizeControlText(AP_RID_DIALOG_TOGGLECASE_RDO_UpperCase,	AP_STRING_ID_DLG_ToggleCase_UpperCase);
	localizeControlText(AP_RID_DIALOG_TOGGLECASE_RDO_TitleCase,	AP_STRING_ID_DLG_ToggleCase_TitleCase);
	localizeControlText(AP_RID_DIALOG_TOGGLECASE_RDO_ToggleCase,	AP_STRING_ID_DLG_ToggleCase_ToggleCase);

	checkButton(AP_RID_DIALOG_TOGGLECASE_RDO_SentenceCase, true);

	return 1;							// 1 == we did not call SetFocus()
}

BOOL AP_Win32Dialog_ToggleCase::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;

	switch (wId)
	{
	case IDCANCEL:						// also AP_RID_DIALOG_TOGGLECASE_BTN_CANCEL
		setAnswer(a_CANCEL);
		EndDialog(hWnd,0);
		return 1;

	case IDOK:							// also AP_RID_DIALOG_TOGGLECASE_BTN_OK
		setAnswer(a_OK);
		setCase(m_iWhichCase);
		EndDialog(hWnd,0);
		return 1;

	case AP_RID_DIALOG_TOGGLECASE_RDO_SentenceCase:
		m_iWhichCase = CASE_SENTENCE;
		return 1;

	case AP_RID_DIALOG_TOGGLECASE_RDO_LowerCase:
		m_iWhichCase = CASE_LOWER;
		return 1;
		
	case AP_RID_DIALOG_TOGGLECASE_RDO_UpperCase:
		m_iWhichCase = CASE_UPPER;
		return 1;
		
	case AP_RID_DIALOG_TOGGLECASE_RDO_TitleCase:
		m_iWhichCase = CASE_FIRST_CAPITAL;
		return 1;
		
	case AP_RID_DIALOG_TOGGLECASE_RDO_ToggleCase:
		m_iWhichCase = CASE_TOGGLE;
		return 1;

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}
	
BOOL AP_Win32Dialog_ToggleCase::_onDeltaPos(NM_UPDOWN * pnmud)
{
	return 0;
}

