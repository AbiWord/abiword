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

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"
#include "xap_Win32LabelledSeparator.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Break.h"
#include "ap_Win32Dialog_Break.h"
#include "ap_Win32Resources.rc2"

/*****************************************************************/

XAP_Dialog * AP_Win32Dialog_Break::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id id)
{
	AP_Win32Dialog_Break * p = new AP_Win32Dialog_Break(pFactory,id);
	return p;
}

AP_Win32Dialog_Break::AP_Win32Dialog_Break(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id id)
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
	UT_return_if_fail (m_id == AP_DIALOG_ID_BREAK);
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);
	XAP_Win32LabelledSeparator_RegisterClass(pWin32App);
	setDialog(this);
	createModal(pFrame, MAKEINTRESOURCE(AP_RID_DIALOG_BREAK));
}

BOOL AP_Win32Dialog_Break::_onInitDialog(HWND /*hWnd*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	localizeDialogTitle(AP_STRING_ID_DLG_Break_BreakTitle);

	// localize controls
	localizeControlText(AP_RID_DIALOG_BREAK_BTN_OK,	 XAP_STRING_ID_DLG_OK);
	localizeControlText(AP_RID_DIALOG_BREAK_BTN_CANCEL, XAP_STRING_ID_DLG_Cancel);

	localizeControlText(AP_RID_DIALOG_BREAK_TEXT_INSERT, AP_STRING_ID_DLG_Break_Insert);
	localizeControlText(AP_RID_DIALOG_BREAK_TEXT_SECTION,AP_STRING_ID_DLG_Break_SectionBreaks);
	localizeControlText(AP_RID_DIALOG_BREAK_RADIO_PAGE,	AP_STRING_ID_DLG_Break_PageBreak);
	localizeControlText(AP_RID_DIALOG_BREAK_RADIO_COL,	AP_STRING_ID_DLG_Break_ColumnBreak);
	localizeControlText(AP_RID_DIALOG_BREAK_RADIO_NEXT,	AP_STRING_ID_DLG_Break_NextPage);
	localizeControlText(AP_RID_DIALOG_BREAK_RADIO_CONT,	AP_STRING_ID_DLG_Break_Continuous);
	localizeControlText(AP_RID_DIALOG_BREAK_RADIO_EVEN,	AP_STRING_ID_DLG_Break_EvenPage);
	localizeControlText(AP_RID_DIALOG_BREAK_RADIO_ODD,	AP_STRING_ID_DLG_Break_OddPage);

	// set initial state
	checkButton(AP_RID_DIALOG_BREAK_RADIO_PAGE + m_break);
	centerDialog();
	return 1;							// 1 == we did not call SetFocus()
}

BOOL AP_Win32Dialog_Break::_onCommand(HWND hWnd, WPARAM wParam, LPARAM /*lParam*/)
{
	WORD wId = LOWORD(wParam);


	switch (wId)
	{
	case IDCANCEL:						// also AP_RID_DIALOG_BREAK_BTN_CANCEL
		m_answer = a_CANCEL;
		// fall through

	case IDOK:							// also AP_RID_DIALOG_BREAK_BTN_OK
		{
			for (int i = AP_RID_DIALOG_BREAK_RADIO_PAGE; i <= AP_RID_DIALOG_BREAK_RADIO_ODD; i++)
			{
				if (isChecked(i))
				{
					m_break = (AP_Dialog_Break::breakType)(i - AP_RID_DIALOG_BREAK_RADIO_PAGE);
					break;
				}
			}
		}
		EndDialog(hWnd,0);
		return 1;

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}

BOOL AP_Win32Dialog_Break::_onDeltaPos(NM_UPDOWN * /*pnmud*/)
{
	return FALSE;
}
