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
}

void AP_Win32Dialog_Latex::runModeless(XAP_Frame * pFrame)
{
        // raise the dialog
        int iResult;
        XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);

        LPCTSTR lpTemplate = NULL;

        UT_return_if_fail (m_id == AP_RID_DIALOG_LATEX);

        lpTemplate = MAKEINTRESOURCE(AP_RID_DIALOG_LATEX);

        HWND hResult = CreateDialogParam(pWin32App->getInstance(),lpTemplate,
                                                        static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow(),
                                                        (DLGPROC)s_dlgProc,(LPARAM)this);

        UT_ASSERT_HARMLESS((hResult != NULL));

        m_hWnd = hResult;

        // Save dialog the ID number and pointer to the widget
        UT_sint32 sid =(UT_sint32)  getDialogId();
        m_pApp->rememberModelessId( sid, (XAP_Dialog_Modeless *) m_pDialog);

        iResult = ShowWindow( m_hWnd, SW_SHOW );

        iResult = BringWindowToTop( m_hWnd );

        UT_ASSERT_HARMLESS((iResult != 0));


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
        int iResult = DestroyWindow( m_hWnd );

        UT_ASSERT_HARMLESS((iResult != 0));

        modeless_cleanup();
}

void AP_Win32Dialog_Latex::setLatexInGUI(void)
{
	UT_UTF8String sLatex;
	getLatex(sLatex);

	setControlText(AP_RID_DIALOG_LATEX_EDIT_LATEX, (LPCSTR)(AP_Win32App::s_fromUTF8ToWinLocale(sLatex.utf8_str())).c_str());
}

bool AP_Win32Dialog_Latex::getLatexFromGUI(void)
{
	char buffer[2048]; // TODO: FIXME: BAD BAD FIXED LENGTH
	UT_UTF8String sLatex;

	getControlText(AP_RID_DIALOG_LATEX_EDIT_LATEX, (LPSTR) buffer, 2048);
	sLatex = AP_Win32App::s_fromWinLocaleToUTF8(buffer);
	
	UT_DEBUGMSG(("LaTeX from widget is %s \n",sLatex.utf8_str()));
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
