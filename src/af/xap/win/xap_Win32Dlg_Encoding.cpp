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
#include "xap_Win32Frame.h"

#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_Encoding.h"
#include "xap_Win32Dlg_Encoding.h"

#include "xap_Win32Resources.rc2"

/*****************************************************************/

XAP_Dialog * XAP_Win32Dialog_Encoding::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	XAP_Win32Dialog_Encoding * p = new XAP_Win32Dialog_Encoding(pFactory,id);
	return p;
}

XAP_Win32Dialog_Encoding::XAP_Win32Dialog_Encoding(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: XAP_Dialog_Encoding(pDlgFactory,id)
{
}

XAP_Win32Dialog_Encoding::~XAP_Win32Dialog_Encoding(void)
{
}

void XAP_Win32Dialog_Encoding::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);
	XAP_Win32Frame * pWin32Frame = static_cast<XAP_Win32Frame *>(pFrame);
	
	LPCTSTR lpTemplate = NULL;

	UT_ASSERT(m_id == XAP_DIALOG_ID_ENCODING);

	lpTemplate = MAKEINTRESOURCE(XAP_RID_DIALOG_ENCODING);

	int result = DialogBoxParam(pWin32App->getInstance(),
			                    lpTemplate,
								pWin32Frame->getTopLevelWindow(),
								(DLGPROC)s_dlgProc,
								(LPARAM)this );
	UT_ASSERT((result != -1));
}

BOOL CALLBACK XAP_Win32Dialog_Encoding::s_dlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// This is a static function.
	XAP_Win32Dialog_Encoding * pThis;
	
	switch (msg)
	{
	case WM_INITDIALOG:
		pThis = (XAP_Win32Dialog_Encoding *)lParam;
		SetWindowLong(hWnd,DWL_USER,lParam);
		return pThis->_onInitDialog(hWnd,wParam,lParam);
		
	case WM_COMMAND:
		pThis = (XAP_Win32Dialog_Encoding *)GetWindowLong(hWnd,DWL_USER);
		return pThis->_onCommand(hWnd,wParam,lParam);
		
	default:
		return 0;
	}
}

#define _DS(c,s)	SetDlgItemText(hWnd,XAP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL XAP_Win32Dialog_Encoding::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	SetWindowText(hWnd, pSS->getValue(XAP_STRING_ID_DLG_UENC_EncTitle));

	// localize controls
	_DS(ENCODING_BTN_OK,			DLG_OK);
	_DS(ENCODING_BTN_CANCEL,		DLG_Cancel);
	_DS(ENCODING_FRM_ENCODING,      DLG_UENC_EncLabel);
	
	// Load Initial Data into Listbox
	{
		HWND hwndList = GetDlgItem(hWnd, XAP_RID_DIALOG_ENCODING_LBX_ENCODING);  

		// load each string name into the list
		for ( UT_uint32 i=0; i < m_iEncCount;  i++ )
		{
			const XML_Char* s = m_ppEncodings[i];

            SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM) s); 
            SendMessage(hwndList, LB_SETITEMDATA, i, (LPARAM) i);  
        }
		// Set to default or guessed encoding
		SendMessage(hwndList, LB_SETCURSEL, m_iSelIndex, 0);
	}		

	return 1;							// 1 == we did not call SetFocus()
}

BOOL XAP_Win32Dialog_Encoding::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;
	HWND hWndList = NULL; 
	int nItem;

	switch (wId)
	{
	case XAP_RID_DIALOG_ENCODING_LBX_ENCODING:
		switch (HIWORD(wParam))
		{
			case LBN_SELCHANGE:
				// NOTE: we get away with only grabbing this in IDOK case
				return 0;

			case LBN_DBLCLK:
				nItem = SendMessage(hWndCtrl, LB_GETCURSEL, 0, 0);
				_setEncoding( m_ppEncodings[nItem] );
				m_answer = a_OK;
				EndDialog(hWnd,0);
				return 1;

			default:
				return 0;
		}
		break;

	case IDCANCEL:						// also XAP_RID_DIALOG_ENCODING_BTN_CANCEL
		m_answer = a_CANCEL;
		EndDialog(hWnd,0);
		return 1;

	case IDOK:							// also XAP_RID_DIALOG_ENCODING_BTN_OK
		hWndList = GetDlgItem(hWnd, XAP_RID_DIALOG_ENCODING_LBX_ENCODING);
		nItem = SendMessage(hWndList, LB_GETCURSEL, 0, 0);
		if( nItem != LB_ERR)
		{
			_setEncoding( m_ppEncodings[nItem] );
			m_answer = a_OK;
		}
		else
		{
			m_answer = a_CANCEL;
		}	
		EndDialog(hWnd,0);
		return 1;

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}
