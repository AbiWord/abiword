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

#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_WindowMore.h"
#include "xap_Win32Dlg_WindowMore.h"

#include "xap_Win32Resources.rc2"

/*****************************************************************/
XAP_Dialog * XAP_Win32Dialog_WindowMore::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id id)
{
	XAP_Win32Dialog_WindowMore * p = new XAP_Win32Dialog_WindowMore(pFactory,id);
	return p;
}

XAP_Win32Dialog_WindowMore::XAP_Win32Dialog_WindowMore(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id id)
	: XAP_Dialog_WindowMore(pDlgFactory,id)
{
}

XAP_Win32Dialog_WindowMore::~XAP_Win32Dialog_WindowMore(void)
{
}

void XAP_Win32Dialog_WindowMore::runModal(XAP_Frame * pFrame)
{
	// NOTE: this work could be done in XP code
	m_ndxSelFrame = m_pApp->findFrame(pFrame);
	UT_ASSERT(m_ndxSelFrame >= 0);

	// raise the dialog
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);
	XAP_Win32Frame * pWin32Frame = static_cast<XAP_Win32Frame *>(pFrame);

	LPCTSTR lpTemplate = NULL;

	UT_ASSERT(m_id == XAP_DIALOG_ID_WINDOWMORE);

	lpTemplate = MAKEINTRESOURCE(XAP_RID_DIALOG_WINDOWMORE);

	int result = DialogBoxParam(pWin32App->getInstance(),lpTemplate,
								pWin32Frame->getTopLevelWindow(),
								(DLGPROC)s_dlgProc,(LPARAM)this);
	UT_ASSERT((result != -1));
}

BOOL CALLBACK XAP_Win32Dialog_WindowMore::s_dlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// This is a static function.

	XAP_Win32Dialog_WindowMore * pThis;
	
	switch (msg)
	{
	case WM_INITDIALOG:
		pThis = (XAP_Win32Dialog_WindowMore *)lParam;
		SetWindowLong(hWnd,DWL_USER,lParam);
		return pThis->_onInitDialog(hWnd,wParam,lParam);
		
	case WM_COMMAND:
		pThis = (XAP_Win32Dialog_WindowMore *)GetWindowLong(hWnd,DWL_USER);
		return pThis->_onCommand(hWnd,wParam,lParam);
		
	default:
		return 0;
	}
}

#define _DS(c,s)	SetDlgItemText(hWnd,XAP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL XAP_Win32Dialog_WindowMore::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	SetWindowText(hWnd, pSS->getValue(XAP_STRING_ID_DLG_MW_MoreWindows));

	// localize controls
	_DS(WINDOWMORE_TEXT_ACTIVATE,	DLG_MW_Activate);
	_DS(WINDOWMORE_BTN_OK,			DLG_OK);
	_DS(WINDOWMORE_BTN_CANCEL,		DLG_Cancel);

	{
		HWND hwndList = GetDlgItem(hWnd, XAP_RID_DIALOG_WINDOWMORE_LIST);  

		// load each frame name into the list
		for (UT_uint32 i=0; i<m_pApp->getFrameCount(); i++)
		{
			XAP_Frame * f = m_pApp->getFrame(i);
			UT_ASSERT(f);
			const char * s = f->getTitle(128);	// TODO: chop this down more? 

            SendMessage(hwndList, LB_ADDSTRING, 0, (LPARAM) s); 
            SendMessage(hwndList, LB_SETITEMDATA, i, (LPARAM) i);  
        } 

		// select the one we're in
		SendMessage(hwndList, LB_SETCURSEL, (WPARAM) m_ndxSelFrame, 0);
	}		

	return 1;							// 1 == we did not call SetFocus()
}

BOOL XAP_Win32Dialog_WindowMore::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;
	int nItem;

	switch (wId)
	{
	case XAP_RID_DIALOG_WINDOWMORE_LIST:
		switch (HIWORD(wParam))
		{
			case LBN_SELCHANGE:
				// NOTE: we could get away with only grabbing this in IDOK case
				nItem = SendMessage(hWndCtrl, LB_GETCURSEL, 0, 0);
				m_ndxSelFrame = SendMessage(hWndCtrl, LB_GETITEMDATA, nItem, 0);
				return 1;

			case LBN_DBLCLK:
				nItem = SendMessage(hWndCtrl, LB_GETCURSEL, 0, 0);
				m_ndxSelFrame = SendMessage(hWndCtrl, LB_GETITEMDATA, nItem, 0);
				EndDialog(hWnd,0);
				return 1;

			default:
				return 0;
		}
		break;

	case IDCANCEL:						// also XAP_RID_DIALOG_WINDOWMORE_BTN_CANCEL
		m_answer = a_CANCEL;
		// fall through

	case IDOK:							// also XAP_RID_DIALOG_WINDOWMORE_BTN_OK
		EndDialog(hWnd,0);
		return 1;

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}

