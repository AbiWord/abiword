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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
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
   	UT_return_if_fail (m_id == AP_DIALOG_ID_MAILMERGE);
	createModeless(pFrame, MAKEINTRESOURCEW(AP_RID_DIALOG_MAILMERGE));

	// Save dialog the ID number and pointer to the widget
	m_pFrame = pFrame;
	UT_sint32 sid =(UT_sint32)  getDialogId();
	m_pApp->rememberModelessId( sid, (XAP_Dialog_Modeless *) m_pDialog);
}


#define _DS(c,s)	setDlgItemText(AP_RID_DIALOG_MAILMERGE_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	setDlgItemText(AP_RID_DIALOG_MAILMERGE_##c,pSS->getValue(XAP_STRING_ID_##s))


// This handles the WM_INITDIALOG message for the top-level dialog.
BOOL AP_Win32Dialog_MailMerge::_onInitDialog(HWND hWnd, WPARAM /*wParam*/, LPARAM /*lParam*/)
{		
	
	
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	// Localise Controls
	_DS(STATIC_AVAIL,	DLG_MailMerge_AvailableFields);		
	_DS(STATIC_FIELD,	DLG_MailMerge_Insert);		
	_DS(BTN_OPEN,		DLG_MailMerge_OpenFile);		
	_DS(BTN_INSERT,		DLG_InsertButton);		
	_DSX(BTN_CLOSE,		DLG_Close);				
	
	setDialogTitle (pSS->getValue(AP_STRING_ID_DLG_MailMerge_MailMergeTitle));	
	
	centerDialog();	
	
	SetFocus(GetDlgItem(hWnd,AP_RID_DIALOG_MAILMERGE_BTN_CLOSE));
	return 0; // 0 because we called SetFocus
}

BOOL AP_Win32Dialog_MailMerge::_onCommand(HWND /*hWnd*/, WPARAM wParam, LPARAM /*lParam*/)
{
//	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
//	HWND hWndCtrl = (HWND)lParam;

	switch (wId)
	{
		case AP_RID_DIALOG_MAILMERGE_LISTBOX:
		if (HIWORD(wParam)==LBN_DBLCLK)
		{
			UT_Win32LocaleString str;
			int nItem = SendMessageW(GetDlgItem(m_hDlg, AP_RID_DIALOG_MAILMERGE_LISTBOX), LB_GETCURSEL, 0, 0);
			
			if (nItem!=LB_ERR)
			{	
				// get the mail merge field from the listbox
				HWND lBox = GetDlgItem(m_hDlg, AP_RID_DIALOG_MAILMERGE_LISTBOX);
				UT_sint32 len = SendMessageW(lBox, LB_GETTEXTLEN, nItem, (LPARAM)0);
				wchar_t* szBuff = (wchar_t*)g_malloc(sizeof(wchar_t) * (len + 1));
				SendMessageW(lBox, LB_GETTEXT, nItem, (LPARAM)szBuff);
				str.fromLocale(szBuff);
				FREEP(szBuff);

				setMergeField(str.utf8_str());			
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
			UT_Win32LocaleString str;	
			int nChars = getDlgItemText(AP_RID_DIALOG_MAILMERGE_EDIT_FIELD, str);
			if (nChars > 0)
			{
				setMergeField(str.utf8_str ());
				addClicked();
			} 
			else
			{
				int nItem = SendMessageW(GetDlgItem(m_hDlg, AP_RID_DIALOG_MAILMERGE_LISTBOX), LB_GETCURSEL, 0, 0);
			
				if (nItem!=LB_ERR)
				{	
					// get the mail merge field from the listbox
					HWND lBox = GetDlgItem(m_hDlg, AP_RID_DIALOG_MAILMERGE_LISTBOX);
					UT_sint32 len = SendMessageW(lBox, LB_GETTEXTLEN, nItem, (LPARAM)0);
					wchar_t* szBuff = (wchar_t*)g_malloc(sizeof(wchar_t) * (len + 1));
					SendMessageW(lBox, LB_GETTEXT, nItem, (LPARAM)szBuff);
					str.fromLocale(szBuff);
					FREEP(szBuff);

					setMergeField(str.utf8_str());
					addClicked();
				}				 
			}

			return 1;
		}
		
		case AP_RID_DIALOG_MAILMERGE_BTN_OPEN:		
		{			
			eventOpen();
			SetFocus(m_hDlg);
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
	if (!m_vecFields.size())
		return;	

	resetContent(AP_RID_DIALOG_MAILMERGE_LISTBOX);
		
 	// build a list of all items
    for (UT_sint32 i = 0; i < m_vecFields.size(); i++)
	{
		UT_continue_if_fail(!m_vecFields[i].empty());
		
		UT_Win32LocaleString str;
		str.fromUTF8(m_vecFields[i].c_str());
		
		SendMessageW(GetDlgItem(m_hDlg, AP_RID_DIALOG_MAILMERGE_LISTBOX), LB_ADDSTRING,
			0, (LPARAM)str.ucs2_str());
	}
	
}

void AP_Win32Dialog_MailMerge::destroy(void)
{
	DestroyWindow(m_hDlg);
	modeless_cleanup();
}
