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

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_WordCount.h"
#include "ap_Win32Dialog_WordCount.h"

#include "ap_Win32Resources.rc2"

/*****************************************************************/

XAP_Dialog * AP_Win32Dialog_WordCount::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_Win32Dialog_WordCount * p = new AP_Win32Dialog_WordCount(pFactory,id);
	return p;
}

AP_Win32Dialog_WordCount::AP_Win32Dialog_WordCount(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_WordCount(pDlgFactory,id)
{
}

AP_Win32Dialog_WordCount::~AP_Win32Dialog_WordCount(void)
{
}

/*****************************************************************/

void AP_Win32Dialog_WordCount::runModal(XAP_Frame * pFrame)
{
	// raise the dialog
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);
	XAP_Win32Frame * pWin32Frame = static_cast<XAP_Win32Frame *>(pFrame);

	LPCTSTR lpTemplate = NULL;

	UT_ASSERT(m_id == AP_DIALOG_ID_WORDCOUNT);

	lpTemplate = MAKEINTRESOURCE(AP_RID_DIALOG_WORDCOUNT);

	int result = DialogBoxParam(pWin32App->getInstance(),lpTemplate,
								pWin32Frame->getTopLevelWindow(),
								(DLGPROC)s_dlgProc,(LPARAM)this);
	UT_ASSERT((result != -1));
}

BOOL CALLBACK AP_Win32Dialog_WordCount::s_dlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// This is a static function.

	AP_Win32Dialog_WordCount * pThis;
	
	switch (msg)
	{
	case WM_INITDIALOG:
		pThis = (AP_Win32Dialog_WordCount *)lParam;
		SetWindowLong(hWnd,DWL_USER,lParam);
		return pThis->_onInitDialog(hWnd,wParam,lParam);
		
	case WM_COMMAND:
		pThis = (AP_Win32Dialog_WordCount *)GetWindowLong(hWnd,DWL_USER);
		return pThis->_onCommand(hWnd,wParam,lParam);
		
	default:
		return 0;
	}
}

#define _DSI(c,i)	SetDlgItemInt(hWnd,AP_RID_DIALOG_##c,m_count.##i,FALSE)
#define _DS(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL AP_Win32Dialog_WordCount::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	SetWindowText(hWnd, pSS->getValue(AP_STRING_ID_DLG_WordCount_WordCountTitle));

	// localize controls
	_DSX(WORDCOUNT_BTN_CLOSE,		DLG_Close);

	_DS(WORDCOUNT_TEXT_STATS,		DLG_WordCount_Statistics);
	_DS(WORDCOUNT_TEXT_PAGE,		DLG_WordCount_Pages);
	_DS(WORDCOUNT_TEXT_WORD,		DLG_WordCount_Words);
	_DS(WORDCOUNT_TEXT_CH,			DLG_WordCount_Characters_No);
	_DS(WORDCOUNT_TEXT_CHSP,		DLG_WordCount_Characters_Sp);
	_DS(WORDCOUNT_TEXT_PARA,		DLG_WordCount_Paragraphs);
	_DS(WORDCOUNT_TEXT_LINE,		DLG_WordCount_Lines);

	// set initial state
	_DSI(WORDCOUNT_VAL_PAGE,		page);
	_DSI(WORDCOUNT_VAL_WORD,		word);
	_DSI(WORDCOUNT_VAL_CH,			ch_no);
	_DSI(WORDCOUNT_VAL_CHSP,		ch_sp);
	_DSI(WORDCOUNT_VAL_PARA,		para);
	_DSI(WORDCOUNT_VAL_LINE,		line);

	return 1;							// 1 == we did not call SetFocus()
}

BOOL AP_Win32Dialog_WordCount::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;
	int n;

	switch (wId)
	{
	case IDCANCEL:						// also AP_RID_DIALOG_WORDCOUNT_BTN_CLOSE
		m_answer = a_CANCEL;
		EndDialog(hWnd,0);
		return 1;

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}

