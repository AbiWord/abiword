/* AbiWord
 * Copyright (C) 2003 Jordi Mas i Hern�dez, jmas@softcatala.org
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
#include "ap_Win32App.h"
#include "xap_Win32FrameImpl.h"
#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_MailMerge.h"
#include "ap_Win32Dialog_MailMerge.h"
#include "ap_Win32Resources.rc2"

#define GWL(hwnd)		(AP_Win32Dialog_MailMerge*)GetWindowLong((hwnd), DWL_USER)
#define SWL(hwnd, d)	(AP_Win32Dialog_MailMerge*)SetWindowLong((hwnd), DWL_USER,(LONG)(d))


/*****************************************************************/
XAP_Dialog * AP_Win32Dialog_MailMerge::static_constructor(XAP_DialogFactory * pFactory,
													       XAP_Dialog_Id id)
{
	AP_Win32Dialog_MailMerge * p = new AP_Win32Dialog_MailMerge(pFactory,id);
	return p;
}

AP_Win32Dialog_MailMerge::AP_Win32Dialog_MailMerge(XAP_DialogFactory * pDlgFactory,
										             XAP_Dialog_Id id)
	: AP_Dialog_MailMerge(pDlgFactory,id)
{		
	
}   
    
AP_Win32Dialog_MailMerge::~AP_Win32Dialog_MailMerge(void)
{	
	
}

void AP_Win32Dialog_MailMerge::runModeless(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	UT_ASSERT(m_id == AP_DIALOG_ID_MAILMERGE);
	int iResult;

	setDialog(this);
	HWND hWndDialog = createModeless( pFrame, MAKEINTRESOURCE(AP_RID_DIALOG_MAILMERGE) );

	UT_ASSERT((hWndDialog != NULL));
	iResult = ShowWindow(hWndDialog, SW_SHOW);
	iResult = BringWindowToTop(hWndDialog);

	m_pApp->rememberModelessId(m_id, this);		
	UT_ASSERT((iResult != 0));
}

BOOL CALLBACK AP_Win32Dialog_MailMerge::s_dlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{		
	AP_Win32Dialog_MailMerge * pThis;
	
	switch (msg)
	{
	case WM_INITDIALOG:
		pThis = (AP_Win32Dialog_MailMerge *)lParam;
		SWL(hWnd,lParam);
		return pThis->_onInitDialog(hWnd,wParam,lParam);
		
	case WM_COMMAND:
		pThis = GWL(hWnd);
		return pThis->_onCommand(hWnd,wParam,lParam);
	default:
		return 0;
	}
}

// This handles the WM_INITDIALOG message for the top-level dialog.
BOOL AP_Win32Dialog_MailMerge::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{		
	init();
	
	// Localise Controls
	localizeControlText(AP_RID_DIALOG_MAILMERGE_BTN_CLOSE,		XAP_STRING_ID_DLG_Close);
	localizeControlText(AP_RID_DIALOG_MAILMERGE_STATIC_AVAIL,	AP_STRING_ID_DLG_MailMerge_AvailableFields);
	localizeControlText(AP_RID_DIALOG_MAILMERGE_STATIC_FIELD,	AP_STRING_ID_DLG_MailMerge_Insert_No_Colon);
	localizeControlText(AP_RID_DIALOG_MAILMERGE_BTN_OPEN,		AP_STRING_ID_DLG_MailMerge_OpenFile);
	localizeControlText(AP_RID_DIALOG_MAILMERGE_BTN_INSERT,		AP_STRING_ID_DLG_InsertButton);
	
	localizeDialogTitle(AP_STRING_ID_DLG_MailMerge_MailMergeTitle);	
	
	centerDialog();	
	
	SetFocus(GetDlgItem(hWnd,AP_RID_DIALOG_MAILMERGE_BTN_CLOSE));
	return 0; // 0 because we called SetFocus
}



BOOL AP_Win32Dialog_MailMerge::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;

	switch (wId)
	{
		case AP_RID_DIALOG_MAILMERGE_LISTBOX:
		if (HIWORD(wParam)==LBN_DBLCLK)
		{
			char szBuff[255];
			int nItem = SendMessage(GetDlgItem(m_hwndDlg, AP_RID_DIALOG_MAILMERGE_LISTBOX), LB_GETCURSEL, 0, 0);
			
			if (nItem!=LB_ERR)
			{	
				SendMessage(GetDlgItem(m_hwndDlg, AP_RID_DIALOG_MAILMERGE_LISTBOX), LB_GETTEXT, nItem,  (LPARAM)szBuff);			
				setMergeField(szBuff);			
				addClicked();
			}
			return 1;
		}		
		
		case AP_RID_DIALOG_MAILMERGE_BTN_INSERT:		
		{	
			char szBuff[255];
			
			getControlText(AP_RID_DIALOG_MAILMERGE_EDIT_FIELD, szBuff, 255);
			
			setMergeField(szBuff);
			addClicked();
			return 1;
		}
		
		case AP_RID_DIALOG_MAILMERGE_BTN_OPEN:		
		{			
			eventOpen();
			SetFocus(m_hwndDlg);
			return 1;
		}	
		
		
		case AP_RID_DIALOG_MAILMERGE_BTN_CLOSE:		
		case IDCANCEL:		// We want to close button work
		{			
			DestroyWindow(m_hwndDlg);		
			modeless_cleanup();
			return 1;
		}	
		
		default:							// we did not handle this notification
			UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
			return 0;						// return zero to let windows take care of it.
	}
}


void AP_Win32Dialog_MailMerge::setFieldList()
{
	if(!m_vecFields.size())	return;	

	UT_uint32 i;
	UT_UTF8String * str;
	
	SendMessage(GetDlgItem(m_hwndDlg, AP_RID_DIALOG_MAILMERGE_LISTBOX), LB_RESETCONTENT,	0, 0);
		
 	// build a list of all items
    for (i = 0; i < m_vecFields.size(); i++)
	{
		str = (UT_UTF8String*)m_vecFields[i];
		
		SendMessage(GetDlgItem(m_hwndDlg, AP_RID_DIALOG_MAILMERGE_LISTBOX), LB_ADDSTRING,
			0, (LPARAM)XAP_Win32App::getWideString(str->utf8_str()));
	}
	
}