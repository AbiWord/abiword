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
#include "ap_Dialog_WordCount.h"
#include "ap_Win32Dialog_WordCount.h"
#include "xap_Win32DialogHelper.h"
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
	UT_ASSERT(pFrame);
	UT_ASSERT(m_id == AP_DIALOG_ID_WORDCOUNT);

	// raise the dialog
	setDialog(this);
	createModal(pFrame, MAKEINTRESOURCE(AP_RID_DIALOG_WORDCOUNT));
}

void AP_Win32Dialog_WordCount::runModeless(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	UT_ASSERT(m_id == AP_DIALOG_ID_WORDCOUNT);
	int iResult;

	setDialog(this);
	HWND hWndDialog = createModeless( pFrame, MAKEINTRESOURCE(AP_RID_DIALOG_WORDCOUNT) );

	UT_ASSERT((hWndDialog != NULL));
	iResult = ShowWindow(hWndDialog, SW_SHOW);
	m_hWnd = hWndDialog;
	iResult = BringWindowToTop(m_hWnd);

	m_pApp->rememberModelessId(m_id, this);		
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

void    AP_Win32Dialog_WordCount::autoupdateWC(UT_Worker * pTimer)
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
	setDialogTitle(m_WindowName); 

	iResult = ShowWindow( m_hWnd, SW_SHOW );

	iResult = BringWindowToTop( m_hWnd );

	UT_ASSERT((iResult != 0));
}

void AP_Win32Dialog_WordCount::notifyActiveFrame(XAP_Frame *pFrame)
{
	if((HWND)GetWindowLong(m_hWnd, GWL_HWNDPARENT) != static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow())
	{
		// Update the caption
		ConstructWindowName();
		setDialogTitle(m_WindowName); 

		SetWindowLong(m_hWnd, GWL_HWNDPARENT, (long)static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow());
		SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0,
						SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
		
		event_Update();
	}
}

void AP_Win32Dialog_WordCount::notifyCloseFrame(XAP_Frame *pFrame)
{
	if((HWND)GetWindowLong(m_hWnd, GWL_HWNDPARENT) == static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow())
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

void AP_Win32Dialog_WordCount::_setDlgItemInt(UINT nCtrl, int nValue)
{
	char szUnFormatted[128], szFormatted[128];

	sprintf(szUnFormatted, "%u", nValue);

	GetNumberFormat(LOCALE_USER_DEFAULT, 0, szUnFormatted,
		NULL, szFormatted, sizeof (szFormatted)/sizeof(char));		

	SetDlgItemText(m_hWnd, nCtrl, szFormatted);		
}						
					
					
#define _DS(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))


BOOL AP_Win32Dialog_WordCount::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	m_hWnd = hWnd;
	
	// Update the caption
	ConstructWindowName();
	setDialogTitle(m_WindowName); 
			
	GR_Graphics * pG = NULL;
	m_pAutoUpdateWC = UT_Timer::static_constructor(autoupdateWC,this,pG);
	setUpdateCounter( 1 );

	// localize controls
	localizeControlText(AP_RID_DIALOG_WORDCOUNT_BTN_CLOSE,		XAP_STRING_ID_DLG_Close);
	localizeControlText(AP_RID_DIALOG_WORDCOUNT_TEXT_STATS,	AP_STRING_ID_DLG_WordCount_Statistics);
	localizeControlText(AP_RID_DIALOG_WORDCOUNT_TEXT_PAGE,	AP_STRING_ID_DLG_WordCount_Pages);
	localizeControlText(AP_RID_DIALOG_WORDCOUNT_TEXT_WORD,	AP_STRING_ID_DLG_WordCount_Words);
	localizeControlText(AP_RID_DIALOG_WORDCOUNT_TEXT_CH,		AP_STRING_ID_DLG_WordCount_Characters_No);
	localizeControlText(AP_RID_DIALOG_WORDCOUNT_TEXT_CHSP,	AP_STRING_ID_DLG_WordCount_Characters_Sp);
	localizeControlText(AP_RID_DIALOG_WORDCOUNT_TEXT_PARA,	AP_STRING_ID_DLG_WordCount_Paragraphs);
	localizeControlText(AP_RID_DIALOG_WORDCOUNT_TEXT_LINE,	AP_STRING_ID_DLG_WordCount_Lines);

	// set initial state
	_setDlgItemInt(AP_RID_DIALOG_WORDCOUNT_VAL_PAGE,		m_count.page);
	_setDlgItemInt(AP_RID_DIALOG_WORDCOUNT_VAL_WORD,		m_count.word);
	_setDlgItemInt(AP_RID_DIALOG_WORDCOUNT_VAL_CH,			m_count.ch_no);
	_setDlgItemInt(AP_RID_DIALOG_WORDCOUNT_VAL_CHSP,		m_count.ch_sp);
	_setDlgItemInt(AP_RID_DIALOG_WORDCOUNT_VAL_PARA,		m_count.para);
	_setDlgItemInt(AP_RID_DIALOG_WORDCOUNT_VAL_LINE,		m_count.line);
	
	centerDialog();	

	return 1;							// 1 == we did not call SetFocus()
}

void AP_Win32Dialog_WordCount::_updateWindowData(void)
{
	HWND hWnd = m_hWnd;

	_setDlgItemInt(AP_RID_DIALOG_WORDCOUNT_VAL_PAGE,		m_count.page);
	_setDlgItemInt(AP_RID_DIALOG_WORDCOUNT_VAL_WORD,		m_count.word);
	_setDlgItemInt(AP_RID_DIALOG_WORDCOUNT_VAL_CH,			m_count.ch_no);
	_setDlgItemInt(AP_RID_DIALOG_WORDCOUNT_VAL_CHSP,		m_count.ch_sp);
	_setDlgItemInt(AP_RID_DIALOG_WORDCOUNT_VAL_PARA,		m_count.para);
	_setDlgItemInt(AP_RID_DIALOG_WORDCOUNT_VAL_LINE,		m_count.line);

	// Update the caption in case the name of the document has changed
	ConstructWindowName();
	setDialogTitle(m_WindowName); 
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
	
	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}

