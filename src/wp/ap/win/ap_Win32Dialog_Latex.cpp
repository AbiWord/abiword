/* AbiWord
 * Copyright (C) 2005 Martin Sevior
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

#include <stdlib.h>
#include <stdio.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"
#include "xap_Frame.h"
#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Win32Resources.rc2"
#include "ap_Win32Dialog_Latex.h"
#include "ap_Win32App.h"

XAP_Dialog * AP_Win32Dialog_Latex::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	return new AP_Win32Dialog_Latex(pFactory,id);
}

AP_Win32Dialog_Latex::AP_Win32Dialog_Latex(XAP_DialogFactory * pDlgFactory,
												 XAP_Dialog_Id id)
	: AP_Dialog_Latex(pDlgFactory,id)
{
}

AP_Win32Dialog_Latex::~AP_Win32Dialog_Latex(void)
{
}

void  AP_Win32Dialog_Latex::activate(void)
{
	int iResult;	
	
	ConstructWindowName();
	setDialogTitle((LPCSTR)(AP_Win32App::s_fromUTF8ToWinLocale(m_sWindowName.utf8_str())).c_str());
	
	iResult = ShowWindow( m_hDlg, SW_SHOW );
	iResult = BringWindowToTop( m_hDlg );

	UT_ASSERT((iResult != 0));
}

void AP_Win32Dialog_Latex::runModeless(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	UT_ASSERT(m_id == AP_DIALOG_ID_LATEX);

	setDialog(this);
	HWND hWndDialog = createModeless( pFrame, MAKEINTRESOURCE(AP_RID_DIALOG_LATEX) );

	UT_ASSERT((hWndDialog != NULL));
	ShowWindow(hWndDialog, SW_SHOW);

	m_pApp->rememberModelessId(m_id, this);		
}


void AP_Win32Dialog_Latex::event_Insert(void)
{
  getLatexFromGUI();
  if(convertLatexToMathML())
  {
    insertIntoDoc();
  }
}

void AP_Win32Dialog_Latex::event_Close(void)
{
	m_answer = AP_Dialog_Latex::a_CANCEL;	
	destroyWindow();
}

void AP_Win32Dialog_Latex::notifyActiveFrame(XAP_Frame *pFrame)
{
	UT_return_if_fail(pFrame);

	HWND frameHWND = static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow();
	if((HWND)GetWindowLong(m_hDlg, GWL_HWNDPARENT) != frameHWND)
	{
		// Update the caption
		ConstructWindowName();
		setDialogTitle((LPCSTR)(AP_Win32App::s_fromUTF8ToWinLocale(m_sWindowName.utf8_str())).c_str());

		SetWindowLong(m_hDlg, GWL_HWNDPARENT, (long)frameHWND);
		SetWindowPos(m_hDlg, NULL, 0, 0, 0, 0,
						SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}
}

void AP_Win32Dialog_Latex::notifyCloseFrame(XAP_Frame *pFrame)
{
	UT_return_if_fail(pFrame);
	if((HWND)GetWindowLong(m_hDlg, GWL_HWNDPARENT) == static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow())
	{
		SetWindowLong(m_hDlg, GWL_HWNDPARENT, NULL);
		SetWindowPos(m_hDlg, NULL, 0, 0, 0, 0,
						SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}
}


void AP_Win32Dialog_Latex::destroy(void)
{
	m_answer = AP_Dialog_Latex::a_CANCEL;	
	modeless_cleanup();
	destroyWindow();
}

void AP_Win32Dialog_Latex::setLatexInGUI(void)
{
	UT_UTF8String sLatex;
	getLatex(sLatex);

	setControlText( AP_RID_DIALOG_LATEX_TEXT_EXAMPLE,(LPCSTR)(AP_Win32App::s_fromUTF8ToWinLocale(sLatex.utf8_str())).c_str() );
}

bool AP_Win32Dialog_Latex::getLatexFromGUI(void)
{
	char* buffer = NULL;
	UT_UTF8String sLatex;
	UT_sint32 length;

	getControlText(AP_RID_DIALOG_LATEX_TEXT_EXAMPLE, NULL, length);
	buffer = new char[length+1];
	getControlText(AP_RID_DIALOG_LATEX_TEXT_EXAMPLE, (LPSTR) buffer, length);
	sLatex = AP_Win32App::s_fromWinLocaleToUTF8(buffer);
	delete buffer;
	
	UT_DEBUGMSG(("LAtex from widget is %s \n",sLatex.utf8_str()));
	setLatex(sLatex);
		
	return true;
}


/*****************************************************************/

BOOL AP_Win32Dialog_Latex::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	m_hDlg = hWnd;
	
	// Update the caption
	ConstructWindowName();
	setDialogTitle((LPCSTR)(AP_Win32App::s_fromUTF8ToWinLocale(m_sWindowName.utf8_str())).c_str());

	// localize controls
	localizeControlText(AP_RID_DIALOG_LATEX_TEXT_EXAMPLE,AP_STRING_ID_DLG_Latex_Example);
	localizeControlText(AP_RID_DIALOG_LATEX_TEXT_LATEXEQUATION,AP_STRING_ID_DLG_Latex_LatexEquation);
	localizeControlText(AP_RID_DIALOG_LATEX_BTN_CLOSE,XAP_STRING_ID_DLG_Close);
	localizeControlText(AP_RID_DIALOG_LATEX_BTN_INSERT,XAP_STRING_ID_DLG_Insert);

	setLatexInGUI();
//	centerDialog();	

	return 1;							// 1 == we did not call SetFocus()
}


BOOL AP_Win32Dialog_Latex::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;
	XAP_Frame *	pFrame = getActiveFrame();

	switch (wId)
	{
	case AP_RID_DIALOG_LATEX_BTN_CLOSE:	// also AP_RID_DIALOG_WORDCOUNT_BTN_CLOSE
		event_Close();
		return 1;
		
	case AP_RID_DIALOG_LATEX_BTN_INSERT:
		event_Insert();
		return 1;		
	
	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}

BOOL AP_Win32Dialog_Latex::_onDeltaPos(NM_UPDOWN * pnmud)
{				
	return FALSE;
}
