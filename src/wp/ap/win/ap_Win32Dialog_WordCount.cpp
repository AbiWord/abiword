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
	m_bAutoWC = 1;
	m_iUpdateRate = 1;
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

void AP_Win32Dialog_WordCount::runModeless(XAP_Frame * pFrame)
{
	// raise the dialog
	int iResult;
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);
	XAP_Win32Frame * pWin32Frame = static_cast<XAP_Win32Frame *>(pFrame);

	LPCTSTR lpTemplate = NULL;

	UT_ASSERT(m_id == AP_DIALOG_ID_WORDCOUNT);

	lpTemplate = MAKEINTRESOURCE(AP_RID_DIALOG_WORDCOUNT);

	HWND hResult = CreateDialogParam(pWin32App->getInstance(),lpTemplate,
								pWin32Frame->getTopLevelWindow(),
								(DLGPROC)s_dlgProc,(LPARAM)this);

	UT_ASSERT((hResult != NULL));

	m_hWnd = hResult;

	// Save dialog the ID number and pointer to the widget
	UT_sint32 sid =(UT_sint32)  getDialogId();
	m_pApp->rememberModelessId( sid, (XAP_Dialog_Modeless *) m_pDialog);

	iResult = ShowWindow( m_hWnd, SW_SHOW );

	iResult = BringWindowToTop( m_hWnd );

	UT_ASSERT((iResult != 0));
}

void    AP_Win32Dialog_WordCount::setUpdateCounter( UT_uint32 iRate )
{
	UT_uint32 iFactor = 1000;

	m_bDestroy_says_stopupdating = false;
	m_bAutoUpdate_happening_now = false;

	// Make a special case for 0 seconds in
	// an attempt to reduce screen flicker
	if( iRate == 0 )
		iFactor = 100;

	m_pAutoUpdateWC->stop();

	m_iUpdateRate = iRate;

	if(m_bAutoWC == true)
		m_pAutoUpdateWC->set(m_iUpdateRate * iFactor);
}         

void    AP_Win32Dialog_WordCount::autoupdateWC(UT_Timer * pTimer)
{
	UT_ASSERT(pTimer);

	// this is a static callback method and does not have a 'this' pointer.

	AP_Win32Dialog_WordCount * pDialog =  (AP_Win32Dialog_WordCount *) pTimer->getInstanceData();

	// Handshaking code

	if( pDialog->m_bDestroy_says_stopupdating != true)
	{
		pDialog->m_bAutoUpdate_happening_now = true;
		pDialog->event_Update();
		pDialog->m_bAutoUpdate_happening_now = false;
	}
}        

void AP_Win32Dialog_WordCount::event_Update(void)
{
	setCountFromActiveFrame();
	_updateWindowData();
}

void AP_Win32Dialog_WordCount::destroy(void)
{
	m_bDestroy_says_stopupdating = true;
	while (m_bAutoUpdate_happening_now == true) ;
	m_pAutoUpdateWC->stop();

	int iResult = DestroyWindow( m_hWnd );

	UT_ASSERT((iResult != 0));

	modeless_cleanup();
}

void AP_Win32Dialog_WordCount::activate(void)
{
	int iResult;

	// Update the caption
	ConstructWindowName();
	SetWindowText(m_hWnd, m_WindowName);

	iResult = ShowWindow( m_hWnd, SW_SHOW );

	iResult = BringWindowToTop( m_hWnd );

	UT_ASSERT((iResult != 0));
}

void AP_Win32Dialog_WordCount::notifyActiveFrame(XAP_Frame *pFrame)
{
	XAP_Win32Frame * pWin32Frame = static_cast<XAP_Win32Frame *>(pFrame);
	if((HWND)GetWindowLong(m_hWnd, GWL_HWNDPARENT) != pWin32Frame->getTopLevelWindow())
	{
		// Update the caption
		ConstructWindowName();
		SetWindowText(m_hWnd, m_WindowName);

		SetWindowLong(m_hWnd, GWL_HWNDPARENT, (long)pWin32Frame->getTopLevelWindow());
		SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0,
						SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
		
		event_Update();
	}
}

void AP_Win32Dialog_WordCount::notifyCloseFrame(XAP_Frame *pFrame)
{
	XAP_Win32Frame * pWin32Frame = static_cast<XAP_Win32Frame *>(pFrame);
	if((HWND)GetWindowLong(m_hWnd, GWL_HWNDPARENT) == pWin32Frame->getTopLevelWindow())
	{
		SetWindowLong(m_hWnd, GWL_HWNDPARENT, NULL);
		SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0,
						SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}
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
		if (pThis)
			return pThis->_onCommand(hWnd,wParam,lParam);
		else
			return 0;
		
	case WM_VSCROLL:
		pThis = (AP_Win32Dialog_WordCount *)GetWindowLong(hWnd,DWL_USER);
		pThis->setUpdateCounter( (UT_uint32)HIWORD(wParam) );
		pThis->event_Update();
		return 1;

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
	
	// Update the caption
	ConstructWindowName();
	SetWindowText(hWnd, m_WindowName);

	// Set the starting rate a 1 update/second
	SetDlgItemInt(hWnd, AP_RID_DIALOG_WORDCOUNT_EDIT_RATE, 1, FALSE );

	// Set the range for auto-updating to 0-10
	SendMessage(GetDlgItem(hWnd,AP_RID_DIALOG_WORDCOUNT_SPIN_RATE),UDM_SETRANGE,(WPARAM)0,(WPARAM)10);
	SendMessage(GetDlgItem(hWnd,AP_RID_DIALOG_WORDCOUNT_EDIT_RATE),EM_LIMITTEXT,(WPARAM)2,(WPARAM)0);

	EnableWindow( GetDlgItem(hWnd,AP_RID_DIALOG_WORDCOUNT_BTN_UPDATE), !m_bAutoWC );
	if( m_bAutoWC )
		CheckDlgButton(hWnd, AP_RID_DIALOG_WORDCOUNT_CHK_AUTOUPDATE, BST_CHECKED);

	GR_Graphics * pG = NULL;
	m_pAutoUpdateWC = UT_Timer::static_constructor(autoupdateWC,this,pG);
	setUpdateCounter( 1 );

	// localize controls
	_DSX(WORDCOUNT_BTN_CLOSE,		DLG_Close);
	_DSX(WORDCOUNT_BTN_UPDATE,		DLG_Update);

	_DS(WORDCOUNT_CHK_AUTOUPDATE,	DLG_WordCount_Auto_Update);
	_DS(WORDCOUNT_TEXT_RATE,		DLG_WordCount_Update_Rate);

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

void AP_Win32Dialog_WordCount::_updateWindowData(void)
{
	HWND hWnd = m_hWnd;

	_DSI(WORDCOUNT_VAL_PAGE,		page);
	_DSI(WORDCOUNT_VAL_WORD,		word);
	_DSI(WORDCOUNT_VAL_CH,			ch_no);
	_DSI(WORDCOUNT_VAL_CHSP,		ch_sp);
	_DSI(WORDCOUNT_VAL_PARA,		para);
	_DSI(WORDCOUNT_VAL_LINE,		line);

	// Update the caption in case the name of the document has changed
	ConstructWindowName();
	SetWindowText(hWnd, m_WindowName);
}

BOOL AP_Win32Dialog_WordCount::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;
	XAP_Frame *	pFrame = getActiveFrame();

	switch (wId)
	{
	case IDCANCEL:						// also AP_RID_DIALOG_WORDCOUNT_BTN_CLOSE
		m_answer = a_CANCEL;
		destroy();
		return 1;

	case AP_RID_DIALOG_WORDCOUNT_BTN_UPDATE:
		notifyActiveFrame(pFrame);
		return 1;

	case AP_RID_DIALOG_WORDCOUNT_CHK_AUTOUPDATE:
		m_bAutoWC = !m_bAutoWC;
		EnableWindow( GetDlgItem(m_hWnd,AP_RID_DIALOG_WORDCOUNT_BTN_UPDATE), !m_bAutoWC );
		EnableWindow( GetDlgItem(m_hWnd,AP_RID_DIALOG_WORDCOUNT_EDIT_RATE), m_bAutoWC );
		EnableWindow( GetDlgItem(m_hWnd,AP_RID_DIALOG_WORDCOUNT_SPIN_RATE), m_bAutoWC );
		setUpdateCounter( m_iUpdateRate );
		return 1;

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}

