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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include <stdio.h>
#include <windows.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_Win32LocaleString.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_WordCount.h"
#include "ap_Win32Dialog_WordCount.h"
#include "ap_Win32Resources.rc2"
#include "ap_Win32App.h"

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
	UT_return_if_fail (m_id == AP_DIALOG_ID_WORDCOUNT);
    createModal (pFrame, MAKEINTRESOURCEW(AP_RID_DIALOG_WORDCOUNT));	
}

void AP_Win32Dialog_WordCount::runModeless(XAP_Frame * pFrame)
{
	// raise the dialog	
	UT_return_if_fail (m_id == AP_DIALOG_ID_WORDCOUNT);	
	createModeless (pFrame, MAKEINTRESOURCEW(AP_RID_DIALOG_WORDCOUNT));

	// Save dialog the ID number and pointer to the widget
    UT_sint32 sid =(UT_sint32) getDialogId();
	m_pApp->rememberModelessId (sid, (XAP_Dialog_Modeless *) m_pDialog);
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
	UT_return_if_fail (pTimer);

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

	m_pAutoUpdateWC->stop();

    destroyWindow ();

	modeless_cleanup();
}

void AP_Win32Dialog_WordCount::activate(void)
{
	// Update the caption
	ConstructWindowName();
    setDialogTitle(m_WindowName);
	showWindow(SW_SHOW);
	bringWindowToTop();
}

void AP_Win32Dialog_WordCount::notifyActiveFrame(XAP_Frame *pFrame)
{
	if((HWND)GetWindowLongPtrW(m_hDlg, GWLP_HWNDPARENT) != static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow())
	{
		// Update the caption
		ConstructWindowName();
		setDialogTitle (m_WindowName);

		SetWindowLongPtrW(m_hDlg, GWLP_HWNDPARENT, (LONG_PTR)static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow());
		SetWindowPos(m_hDlg, NULL, 0, 0, 0, 0,
						SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
		
		event_Update();
	}
}

BOOL AP_Win32Dialog_WordCount::_onDlgMessage(HWND /*hWnd*/, UINT msg, WPARAM wParam, LPARAM /*lParam*/)
{
	if (msg != WM_VSCROLL)
		return FALSE;
	setUpdateCounter( (UT_uint32)HIWORD(wParam) );
	event_Update();		
	return TRUE;
}

static NUMBERFMTW numberfmt;
static wchar_t gszDecSep[16];
static wchar_t gszThSep[16];		

void AP_Win32Dialog_WordCount::_setDlgItemInt(UINT nCtrl, int nValue)
{	
	wchar_t szFormatted[64], szUnFormatted[64];
	swprintf(szUnFormatted, L"%d", nValue);

	if (numberfmt.lpThousandSep==NULL)	// We only do this the first time
	{
		wchar_t szBuffer[16];
		
		GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL, gszDecSep, sizeof(gszDecSep));
		GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, gszThSep, sizeof(gszThSep));
				
		numberfmt.NumDigits = 0;		
		GetLocaleInfoW(LOCALE_USER_DEFAULT, LOCALE_ILZERO, szBuffer, sizeof(szBuffer));
		numberfmt.LeadingZero = _wtoi(szBuffer);

		GetLocaleInfoW( LOCALE_USER_DEFAULT, LOCALE_SGROUPING, szBuffer, sizeof(szBuffer));
		numberfmt.Grouping = _wtoi(szBuffer);
		numberfmt.lpDecimalSep = gszDecSep;
		numberfmt.lpThousandSep = gszThSep;

		GetLocaleInfoW( LOCALE_USER_DEFAULT, LOCALE_INEGNUMBER, szBuffer, sizeof(szBuffer));
		numberfmt.NegativeOrder = _wtoi(szBuffer);
	}
	
	/* Convert the number string into the proper locale defn */
	GetNumberFormatW(LOCALE_USER_DEFAULT, 0, szUnFormatted,
		&numberfmt, szFormatted, sizeof(szFormatted));

	SetDlgItemTextW(m_hDlg, nCtrl, szFormatted);		
}						
					
					
#define _DS(c,s)	setDlgItemText(AP_RID_DIALOG_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	setDlgItemText(AP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))


BOOL AP_Win32Dialog_WordCount::_onInitDialog(HWND /*hWnd*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	// Update the caption
	ConstructWindowName();
	setDialogTitle (m_WindowName);
			
	m_pAutoUpdateWC = UT_Timer::static_constructor(autoupdateWC,this);
	setUpdateCounter( 1 );

	// localize controls
	_DSX(WORDCOUNT_BTN_CLOSE,		DLG_Close);
	
	_DS(WORDCOUNT_TEXT_STATS,		DLG_WordCount_Statistics);
	_DS(WORDCOUNT_TEXT_PAGE,		DLG_WordCount_Pages);
	_DS(WORDCOUNT_TEXT_WORD,		DLG_WordCount_Words);
	_DS(WORDCOUNT_TEXT_CH,			DLG_WordCount_Characters_No);
	_DS(WORDCOUNT_TEXT_CHSP,		DLG_WordCount_Characters_Sp);
	_DS(WORDCOUNT_TEXT_PARA,		DLG_WordCount_Paragraphs);
	_DS(WORDCOUNT_TEXT_LINE,		DLG_WordCount_Lines);
	_DS(WORDCOUNT_TEXT_WORDSONLY,	DLG_WordCount_Words_No_Notes);
	
	_updateWindowData ();
	centerDialog();

	return 1;							// 1 == we did not call SetFocus()
}

void AP_Win32Dialog_WordCount::_updateWindowData(void)
{
	_setDlgItemInt(AP_RID_DIALOG_WORDCOUNT_VAL_PAGE,		m_count.page);
	_setDlgItemInt(AP_RID_DIALOG_WORDCOUNT_VAL_WORD,		m_count.word);
	_setDlgItemInt(AP_RID_DIALOG_WORDCOUNT_VAL_CH,			m_count.ch_no);
	_setDlgItemInt(AP_RID_DIALOG_WORDCOUNT_VAL_CHSP,		m_count.ch_sp);
	_setDlgItemInt(AP_RID_DIALOG_WORDCOUNT_VAL_PARA,		m_count.para);
	_setDlgItemInt(AP_RID_DIALOG_WORDCOUNT_VAL_LINE,		m_count.line);
	_setDlgItemInt(AP_RID_DIALOG_WORDCOUNT_VAL_WORDSONLY,	m_count.words_no_notes);

	// Update the caption in case the name of the document has changed
	ConstructWindowName();
	setDialogTitle (m_WindowName);
}

BOOL AP_Win32Dialog_WordCount::_onCommand(HWND /*hWnd*/, WPARAM wParam, LPARAM /*lParam*/)
{
	WORD wId = LOWORD(wParam);

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

