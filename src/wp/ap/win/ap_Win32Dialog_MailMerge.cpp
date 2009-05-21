/* AbiWord
 * Copyright (C) 2003 Jordi Mas i Hernàdez, jmas@softcatala.org
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
#include "xap_Win32DialogHelper.h"
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
	return new AP_Win32Dialog_MailMerge(pFactory,id);
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
	UT_return_if_fail (pFrame);	
	
	m_pFrame = pFrame;

	int iResult;
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);

	LPCTSTR lpTemplate = NULL;

	UT_return_if_fail (m_id == AP_DIALOG_ID_MAILMERGE);

	lpTemplate = MAKEINTRESOURCE(AP_RID_DIALOG_MAILMERGE);

	HWND hResult = CreateDialogParam(pWin32App->getInstance(),lpTemplate,
							static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow(),
							(DLGPROC)s_dlgProc,(LPARAM)this);

	UT_return_if_fail ((hResult != NULL));

	m_hwndDlg = hResult;
	init();

	// Save dialog the ID number and pointer to the widget
	UT_sint32 sid =(UT_sint32)  getDialogId();
	m_pApp->rememberModelessId( sid, (XAP_Dialog_Modeless *) m_pDialog);

	iResult = ShowWindow(m_hwndDlg, SW_SHOW );
	iResult = BringWindowToTop( m_hwndDlg );

	UT_ASSERT_HARMLESS((iResult != 0));	
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

	case WM_DESTROY:
		pThis = GWL(hWnd);
		if(pThis)
			pThis->destroy();
		return 0;

	default:
		return 0;
	}
}
#define _DS(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_MAILMERGE_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_MAILMERGE_##c,pSS->getValue(XAP_STRING_ID_##s))


// This handles the WM_INITDIALOG message for the top-level dialog.
BOOL AP_Win32Dialog_MailMerge::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{		
	
	
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	// Localise Controls
	_DS(STATIC_AVAIL,	DLG_MailMerge_AvailableFields);		
	_DS(STATIC_FIELD,	DLG_MailMerge_Insert_No_Colon);		
	_DS(BTN_OPEN,		DLG_MailMerge_OpenFile);		
	_DS(BTN_INSERT,		DLG_InsertButton);		
	_DSX(BTN_CLOSE,		DLG_Close);				
	
	SetWindowText(hWnd, pSS->getValue(AP_STRING_ID_DLG_MailMerge_MailMergeTitle));	
	
	XAP_Win32DialogHelper::s_centerDialog(hWnd);	
	
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
		else
		{
			return 0;
		}
		
		case AP_RID_DIALOG_MAILMERGE_BTN_INSERT:		
		{	
			char szBuff[255];
			
			int nChars = GetDlgItemText(m_hwndDlg,  AP_RID_DIALOG_MAILMERGE_EDIT_FIELD, szBuff, 255);
			if (nChars > 0)
			{
				setMergeField(szBuff);
				addClicked();
			} 
			else
			{
				char szBuff[255];
				int nItem = SendMessage(GetDlgItem(m_hwndDlg, AP_RID_DIALOG_MAILMERGE_LISTBOX), LB_GETCURSEL, 0, 0);
			
				if (nItem!=LB_ERR)
				{	
					SendMessage(GetDlgItem(m_hwndDlg, AP_RID_DIALOG_MAILMERGE_LISTBOX), LB_GETTEXT, nItem,  (LPARAM)szBuff);			
					setMergeField(szBuff);			
					addClicked();
				}				 
			}

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
			destroy();
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

	UT_UTF8String * str;
	UT_String	sAnsi;
	
	SendMessage(GetDlgItem(m_hwndDlg, AP_RID_DIALOG_MAILMERGE_LISTBOX), LB_RESETCONTENT,	0, 0);
		
 	// build a list of all items
    for (UT_sint32 i = 0; i < m_vecFields.size(); i++)
	{
		str = (UT_UTF8String*)m_vecFields[i];
		sAnsi = 	AP_Win32App::s_fromUTF8ToWinLocale(str->utf8_str());
		
		SendMessage(GetDlgItem(m_hwndDlg, AP_RID_DIALOG_MAILMERGE_LISTBOX), LB_ADDSTRING,
			0, (LPARAM)sAnsi.c_str());
	}
	
}

void AP_Win32Dialog_MailMerge::destroy(void)
{
	DestroyWindow(m_hwndDlg);
	modeless_cleanup();
}
