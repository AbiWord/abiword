/* AbiWord
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

//////////////////////////////////////////////////////////////////
// THIS CODE RUNS BOTH THE "Find" AND THE "Find-Replace" DIALOGS.
//////////////////////////////////////////////////////////////////

#include <windows.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Replace.h"
#include "ap_Win32Dialog_Replace.h"
#include "xap_Win32DialogHelper.h"
#include "ap_Win32Resources.rc2"

/*****************************************************************/
XAP_Dialog * AP_Win32Dialog_Replace::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id id)
{
	AP_Win32Dialog_Replace * p = new AP_Win32Dialog_Replace(pFactory,id);
	return p;
}

AP_Win32Dialog_Replace::AP_Win32Dialog_Replace(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id id)
	: AP_Dialog_Replace(pDlgFactory,id)
{
	m_hWnd = 0;
}

AP_Win32Dialog_Replace::~AP_Win32Dialog_Replace(void)
{
}

void AP_Win32Dialog_Replace::activate(void)
{
	int iResult;

	// Update the caption
	ConstructWindowName();
	SetWindowText(m_hWnd, m_WindowName);

	SetFocus( GetDlgItem( m_hWnd,AP_RID_DIALOG_REPLACE_EDIT_FIND) );

	iResult = ShowWindow( m_hWnd, SW_SHOW );

	iResult = BringWindowToTop( m_hWnd );
	
	UT_ASSERT((iResult != 0));
	
	{
	UT_UCSChar * bufferUnicode = getFindString();
	UT_uint32 lenUnicode = UT_UCS4_strlen(bufferUnicode);
	if (lenUnicode)
	{
		char * bufferNormal = new char [lenUnicode + 1];
		UT_UCS4_strcpy_to_char(bufferNormal, bufferUnicode);
		SetDlgItemText(m_hWnd,AP_RID_DIALOG_REPLACE_EDIT_FIND,bufferNormal);
		DELETEP(bufferNormal);
	}
	FREEP(bufferUnicode);
	}
	
	if (m_id == AP_DIALOG_ID_REPLACE)
	{
	
		UT_UCSChar * bufferUnicode = getReplaceString();
		UT_uint32 lenUnicode = UT_UCS4_strlen(bufferUnicode);
		if (lenUnicode)
		{
			char * bufferNormal = new char [lenUnicode + 1];
			UT_UCS4_strcpy_to_char(bufferNormal, bufferUnicode);
			SetDlgItemText(m_hWnd,AP_RID_DIALOG_REPLACE_EDIT_REPLACE,bufferNormal);
			DELETEP(bufferNormal);
		}
		FREEP(bufferUnicode);
	}
}


void AP_Win32Dialog_Replace::destroy(void)
{
	int iResult = DestroyWindow( m_hWnd );

	UT_ASSERT((iResult != 0));

	modeless_cleanup();
}

void AP_Win32Dialog_Replace::notifyActiveFrame(XAP_Frame *pFrame)
{
	if((HWND)GetWindowLong(m_hWnd, GWL_HWNDPARENT) != static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow())
	{
		// Update the caption
		ConstructWindowName();
		SetWindowText(m_hWnd, m_WindowName);

		SetWindowLong(m_hWnd, GWL_HWNDPARENT, (long)static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow());
		SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0,
						SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}
}

void AP_Win32Dialog_Replace::notifyCloseFrame(XAP_Frame *pFrame)
{
	if((HWND)GetWindowLong(m_hWnd, GWL_HWNDPARENT) == static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow())
	{
		SetWindowLong(m_hWnd, GWL_HWNDPARENT, NULL);
		SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0,
						SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}
}

void AP_Win32Dialog_Replace::runModeless(XAP_Frame * pFrame)
{
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(XAP_App::getApp());

	LPCTSTR lpTemplate = NULL;
	if (m_id == AP_DIALOG_ID_REPLACE)
		lpTemplate = MAKEINTRESOURCE(AP_RID_DIALOG_REPLACE);
	else if (m_id == AP_DIALOG_ID_FIND)
		lpTemplate = MAKEINTRESOURCE(AP_RID_DIALOG_FIND);
	else
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return;
	}

	setView(static_cast<FV_View *> (pFrame->getCurrentView()) );

	int iResult;
	HWND hResult = CreateDialogParam(pWin32App->getInstance(),lpTemplate,
							static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow(),
							(DLGPROC)s_dlgProc,(LPARAM)this);

	UT_ASSERT((hResult != NULL));

	m_hWnd = hResult;

	// Save dialog the ID number and pointer to the widget
	UT_sint32 sid =(UT_sint32)	getDialogId();
	m_pApp->rememberModelessId( sid, (XAP_Dialog_Modeless *) m_pDialog);

	// Update the caption
	ConstructWindowName();
	SetWindowText(m_hWnd, m_WindowName);

	iResult = ShowWindow( m_hWnd, SW_SHOW );

	iResult = BringWindowToTop( m_hWnd );
	UT_ASSERT((iResult != 0));
	m_pView->focusChange(AV_FOCUS_MODELESS);
}

BOOL CALLBACK AP_Win32Dialog_Replace::s_dlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// This is a static function.

	AP_Win32Dialog_Replace * pThis;
	xxx_UT_DEBUGMSG(("s_dlgProc: msg 0x%x\n",msg));
	switch (msg)
	{
	
	     	
	case WM_INITDIALOG:
		pThis = (AP_Win32Dialog_Replace *)lParam;
		SetWindowLong(hWnd,DWL_USER,lParam);
		return pThis->_onInitDialog(hWnd,wParam,lParam);
	
	// The second time that the dialog box is displayed
	// if he hit return it does not process the order
	// because it sends an IDOK msg
	case WM_COMMAND:
		if (wParam==IDOK)
			wParam=AP_RID_DIALOG_REPLACE_BTN_FINDNEXT;
			
		pThis = (AP_Win32Dialog_Replace *)GetWindowLong(hWnd,DWL_USER);
		return pThis->_onCommand(hWnd,wParam,lParam);

	default:
		return 0;
	}
}

void AP_Win32Dialog_Replace::_initButtons(HWND hWnd)
{
	HWND hWndEditFind = GetDlgItem(hWnd,AP_RID_DIALOG_REPLACE_EDIT_FIND);
	HWND hWndEditReplace = GetDlgItem(hWnd,AP_RID_DIALOG_REPLACE_EDIT_REPLACE);
	DWORD lenFind = GetWindowTextLength(hWndEditFind);
	DWORD lenReplace = GetWindowTextLength(hWndEditReplace);

	BOOL bEnableFind = (lenFind > 0);
	EnableWindow(GetDlgItem(hWnd,AP_RID_DIALOG_REPLACE_BTN_FINDNEXT),bEnableFind);


	if (m_id == AP_DIALOG_ID_REPLACE)
	{
	
		BOOL bEnableReplace = bEnableFind;
		EnableWindow(GetDlgItem(hWnd,AP_RID_DIALOG_REPLACE_BTN_REPLACE),bEnableReplace);
		EnableWindow(GetDlgItem(hWnd,AP_RID_DIALOG_REPLACE_BTN_REPLACEALL),bEnableReplace);
	}

	CheckDlgButton(hWnd,AP_RID_DIALOG_REPLACE_CHECK_MATCHCASE,
				   ((getMatchCase()) ? BST_CHECKED : BST_UNCHECKED));

	CheckDlgButton(hWnd,AP_RID_DIALOG_REPLACE_CHECK_WHOLEWORD,
				   ((getWholeWord()) ? BST_CHECKED : BST_UNCHECKED));

	CheckDlgButton(hWnd,AP_RID_DIALOG_REPLACE_CHECK_REVERSEFIND,
				   ((getReverseFind()) ? BST_CHECKED : BST_UNCHECKED));

	return;
}

#define _DS(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL AP_Win32Dialog_Replace::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	if (m_id == AP_DIALOG_ID_FIND)
		SetWindowText(hWnd, pSS->getValue(AP_STRING_ID_DLG_FR_FindTitle));
	else
		SetWindowText(hWnd, pSS->getValue(AP_STRING_ID_DLG_FR_ReplaceTitle));

	// localize controls shared across dialogs
	_DS(REPLACE_BTN_FINDNEXT,		DLG_FR_FindNextButton);
	_DS(REPLACE_TEXT_FIND,			DLG_FR_FindLabel);
	_DS(REPLACE_CHECK_MATCHCASE,	DLG_FR_MatchCase);

	_DSX(REPLACE_BTN_CLOSE, 		DLG_Cancel);


	if (m_id == AP_DIALOG_ID_REPLACE)
	{

		// localize replace-specific controls
		_DS(REPLACE_BTN_REPLACE,	DLG_FR_ReplaceButton);
		_DS(REPLACE_BTN_REPLACEALL, DLG_FR_ReplaceAllButton);
		_DS(REPLACE_TEXT_REPLACE,	DLG_FR_ReplaceWithLabel);
	}

	_initButtons(hWnd);

	SetFocus( GetDlgItem(hWnd,AP_RID_DIALOG_REPLACE_EDIT_FIND) );

	XAP_Win32DialogHelper::s_centerDialog(hWnd);	
	
	return 0;							// 0 == we did call SetFocus()
}

BOOL AP_Win32Dialog_Replace::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;

	// before doing anything else, make sure that the main window is not entirely
	// without focus
	m_pView->focusChange(AV_FOCUS_MODELESS);
	xxx_UT_DEBUGMSG(("s_dlgProc: wId 0x%x\n",wId));

	switch (wId)
	{
	case AP_RID_DIALOG_REPLACE_EDIT_FIND:
	case AP_RID_DIALOG_REPLACE_EDIT_REPLACE:
		xxx_UT_DEBUGMSG(("_onCommand: edit control\n"));
		_initButtons(hWnd);
		return 0;

	case AP_RID_DIALOG_REPLACE_CHECK_MATCHCASE:
		setMatchCase((IsDlgButtonChecked(hWnd,AP_RID_DIALOG_REPLACE_CHECK_MATCHCASE)==BST_CHECKED));
		return 1;

	case AP_RID_DIALOG_REPLACE_CHECK_WHOLEWORD:
		setWholeWord((IsDlgButtonChecked(hWnd,AP_RID_DIALOG_REPLACE_CHECK_WHOLEWORD)==BST_CHECKED));
		return 1;

	case AP_RID_DIALOG_REPLACE_CHECK_REVERSEFIND:
		{
		bool currentVal = (IsDlgButtonChecked(hWnd,AP_RID_DIALOG_REPLACE_CHECK_REVERSEFIND)==BST_CHECKED);
		setReverseFind(currentVal);
		
		if (!m_pView->isSelectionEmpty()) 
		{
		// if there's a selection, clear it
			if (m_pView->getSelectionText() != NULL) 
			{
				PT_DocPosition pt = m_pView->getSelectionAnchor();
				PT_DocPosition ln = UT_UCS4_strlen (m_pView->getSelectionText());
				if (currentVal)
				{
					m_pView->moveInsPtTo(pt);
				}
				else
				{
					m_pView->moveInsPtTo(pt+ln);
				}
				m_pView->cmdUnselectSelection();
			}
		}

		m_pView->findSetStartAtInsPoint();

		return 1;	
		}
	case AP_RID_DIALOG_REPLACE_BTN_FINDNEXT:
		return _onBtn_FindNext(hWnd);
		
	case AP_RID_DIALOG_REPLACE_BTN_REPLACE:
		return _onBtn_Replace(hWnd);
		
	case AP_RID_DIALOG_REPLACE_BTN_REPLACEALL:
		return _onBtn_ReplaceAll(hWnd);
		
	case IDCANCEL:						// also AP_RID_DIALOG_REPLACE_BTN_CLOSE
		EndDialog(hWnd,0);
		return 1;

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}

BOOL AP_Win32Dialog_Replace::_onBtn_FindNext(HWND hWnd)
{
	char * pBufFromDialogFind = NULL;
	UT_UCSChar * pUCSFind = NULL;

	HWND hWndEditFind = GetDlgItem(hWnd,AP_RID_DIALOG_REPLACE_EDIT_FIND);
	DWORD lenFind = GetWindowTextLength(hWndEditFind);
	if (!lenFind)
		return 1;

	pBufFromDialogFind = new char [lenFind + 1];
	if (!pBufFromDialogFind)
		goto FreeMemory;
	GetWindowText(hWndEditFind,pBufFromDialogFind,lenFind+1);

	UT_DEBUGMSG(("Find entry contents: [%s]\n",pBufFromDialogFind));

	UT_UCS4_cloneString_char(&pUCSFind,pBufFromDialogFind);
	if (!pUCSFind)
		goto FreeMemory;

	setFindString(pUCSFind);
	if (!getReverseFind()) {
    	findNext();
		}
	else {
		findPrev();
		}

FreeMemory:
	DELETEP(pBufFromDialogFind);
	FREEP(pUCSFind);

	return 1;
}

BOOL AP_Win32Dialog_Replace::_onBtn_Replace(HWND hWnd)
{
	UT_ASSERT((m_id == AP_DIALOG_ID_REPLACE));

	char * pBufFromDialogFind = NULL;
	char * pBufFromDialogReplace = NULL;
	UT_UCSChar * pUCSFind = NULL;
	UT_UCSChar * pUCSReplace = NULL;

	HWND hWndEditFind = GetDlgItem(hWnd,AP_RID_DIALOG_REPLACE_EDIT_FIND);
	HWND hWndEditReplace = GetDlgItem(hWnd,AP_RID_DIALOG_REPLACE_EDIT_REPLACE);
	DWORD lenFind = GetWindowTextLength(hWndEditFind);
	DWORD lenReplace = GetWindowTextLength(hWndEditReplace);

	if (!lenFind)
		return 1;

	pBufFromDialogFind = new char [lenFind + 1];
	if (!pBufFromDialogFind)
		goto FreeMemory;
	GetWindowText(hWndEditFind,pBufFromDialogFind,lenFind+1);

	UT_DEBUGMSG(("Find entry contents: [%s]\n",pBufFromDialogFind));

	UT_UCS4_cloneString_char(&pUCSFind,pBufFromDialogFind);
	if (!pUCSFind)
		goto FreeMemory;

	pBufFromDialogReplace = new char [lenReplace + 1];
	if (!pBufFromDialogReplace)
		goto FreeMemory;
	GetWindowText(hWndEditReplace,pBufFromDialogReplace,lenReplace+1);

	UT_DEBUGMSG(("Replace entry contents: [%s]\n",pBufFromDialogReplace));

	UT_UCS4_cloneString_char(&pUCSReplace,pBufFromDialogReplace);
	if (!pUCSReplace)
		goto FreeMemory;

	setFindString(pUCSFind);
	setReplaceString(pUCSReplace);
	if (!getReverseFind()) {
    	findReplace();
		}
	else {
		findReplaceReverse();
		}

FreeMemory:
	DELETEP(pBufFromDialogFind);
	DELETEP(pBufFromDialogReplace);
	FREEP(pUCSFind);
	FREEP(pUCSReplace);
	
	return 1;
}

BOOL AP_Win32Dialog_Replace::_onBtn_ReplaceAll(HWND hWnd)
{
	UT_ASSERT((m_id == AP_DIALOG_ID_REPLACE));

	char * pBufFromDialogFind = NULL;
	char * pBufFromDialogReplace = NULL;
	UT_UCSChar * pUCSFind = NULL;
	UT_UCSChar * pUCSReplace = NULL;

	HWND hWndEditFind = GetDlgItem(hWnd,AP_RID_DIALOG_REPLACE_EDIT_FIND);
	HWND hWndEditReplace = GetDlgItem(hWnd,AP_RID_DIALOG_REPLACE_EDIT_REPLACE);
	DWORD lenFind = GetWindowTextLength(hWndEditFind);
	DWORD lenReplace = GetWindowTextLength(hWndEditReplace);

	if (!lenFind)
		return 1;

	pBufFromDialogFind = new char [lenFind + 1];
	if (!pBufFromDialogFind)
		goto FreeMemory;
	GetWindowText(hWndEditFind,pBufFromDialogFind,lenFind+1);

	UT_DEBUGMSG(("Find entry contents: [%s]\n",pBufFromDialogFind));

	UT_UCS4_cloneString_char(&pUCSFind,pBufFromDialogFind);
	if (!pUCSFind)
		goto FreeMemory;

	pBufFromDialogReplace = new char [lenReplace + 1];
	if (!pBufFromDialogReplace)
		goto FreeMemory;
	GetWindowText(hWndEditReplace,pBufFromDialogReplace,lenReplace+1);

	UT_DEBUGMSG(("Replace entry contents: [%s]\n",pBufFromDialogReplace));

	UT_UCS4_cloneString_char(&pUCSReplace,pBufFromDialogReplace);
	if (!pUCSReplace)
		goto FreeMemory;

	setFindString(pUCSFind);
	setReplaceString(pUCSReplace);
	findReplaceAll();

FreeMemory:
	DELETEP(pBufFromDialogFind);
	DELETEP(pBufFromDialogReplace);
	FREEP(pUCSFind);
	FREEP(pUCSReplace);
	
	return 1;
}

void AP_Win32Dialog_Replace::_updateLists()
{
	UT_ASSERT(UT_NOT_IMPLEMENTED);	
}
