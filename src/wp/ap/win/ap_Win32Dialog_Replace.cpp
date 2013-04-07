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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
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
#include "ap_Win32App.h"

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

}

AP_Win32Dialog_Replace::~AP_Win32Dialog_Replace(void)
{
}

void AP_Win32Dialog_Replace::activate(void)
{
	int iResult;
    UT_Win32LocaleString findstr;

	// Update the caption
	ConstructWindowName();
	setDialogTitle(m_WindowName);

	SetFocus( GetDlgItem( m_hDlg,AP_RID_DIALOG_REPLACE_COMBO_FIND) );

	iResult = ShowWindow( m_hDlg, SW_SHOW );

	iResult = BringWindowToTop( m_hDlg );
	
	UT_return_if_fail ((iResult != 0));
	
	{
	UT_UCSChar * bufferUnicode = getFindString();
	findstr.fromUCS4 (bufferUnicode);
	if (!findstr.empty())
	{
	    SetDlgItemTextW(m_hDlg, AP_RID_DIALOG_REPLACE_COMBO_FIND,findstr.c_str());
		SendMessageW(GetDlgItem(m_hDlg,AP_RID_DIALOG_REPLACE_COMBO_FIND), CB_SETEDITSEL, 0, MAKELONG (0, -1));	
	}
	FREEP(bufferUnicode);
	}
	
	if (m_id == AP_DIALOG_ID_REPLACE)
	{
	
		UT_UCSChar * bufferUnicode = getReplaceString();
		findstr.fromUCS4 (bufferUnicode);
		if (!findstr.empty())
		{			
			SetDlgItemTextW(m_hDlg,AP_RID_DIALOG_REPLACE_COMBO_REPLACE,findstr.c_str());
			SendMessageW(GetDlgItem(m_hDlg,AP_RID_DIALOG_REPLACE_COMBO_FIND), CB_SETEDITSEL, 0, MAKELONG (0, -1));		
		}
		FREEP(bufferUnicode);
	}
}


void AP_Win32Dialog_Replace::destroy(void)
{
	UT_DebugOnly<int> iResult = DestroyWindow( m_hDlg );

	UT_ASSERT_HARMLESS((iResult != 0));

	modeless_cleanup();
}

void AP_Win32Dialog_Replace::notifyActiveFrame(XAP_Frame *pFrame)
{
	if((HWND)GetWindowLongPtrW(m_hDlg, GWLP_HWNDPARENT) != static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow())
	{
		// Update the caption
		ConstructWindowName();
		setDialogTitle(m_WindowName);

		SetWindowLongPtrW(m_hDlg, GWLP_HWNDPARENT, (LONG_PTR)static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow());
		SetWindowPos(m_hDlg, NULL, 0, 0, 0, 0,
						SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}
}

void AP_Win32Dialog_Replace::notifyCloseFrame(XAP_Frame *pFrame)
{
	if((HWND)GetWindowLongPtrW(m_hDlg, GWLP_HWNDPARENT) == static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow())
	{
		SetWindowLongPtrW(m_hDlg, GWLP_HWNDPARENT, 0);
		SetWindowPos(m_hDlg, NULL, 0, 0, 0, 0,
						SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}
}

void AP_Win32Dialog_Replace::runModeless(XAP_Frame * pFrame)
{
	LPCWSTR lpTemplate = NULL;
	if (m_id == AP_DIALOG_ID_REPLACE)
		lpTemplate = MAKEINTRESOURCEW(AP_RID_DIALOG_REPLACE);
	else if (m_id == AP_DIALOG_ID_FIND)
		lpTemplate = MAKEINTRESOURCEW(AP_RID_DIALOG_FIND);
	else
	{
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return;
	}

	setView(static_cast<FV_View *> (pFrame->getCurrentView()) );
    createModeless(pFrame, lpTemplate);

    //  Save dialog the ID number and pointer to the widget	
	m_pApp->rememberModelessId(/* (UT_sint32)*/	getDialogId(), (XAP_Dialog_Modeless *) m_pDialog);

	// Update the caption
	ConstructWindowName();
	setDialogTitle(m_WindowName);

    ShowWindow( m_hDlg, SW_SHOW );	
	m_pView->focusChange(AV_FOCUS_MODELESS);
}

void AP_Win32Dialog_Replace::_initButtons(HWND hWnd)
{
	HWND hWndEditFind = GetDlgItem(hWnd,AP_RID_DIALOG_REPLACE_COMBO_FIND);
//	HWND hWndEditReplace = GetDlgItem(hWnd,AP_RID_DIALOG_REPLACE_COMBO_REPLACE);
	DWORD lenFind = GetWindowTextLengthW(hWndEditFind);
//	DWORD lenReplace = GetWindowTextLengthW(hWndEditReplace);

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

#define _DS(c,s)	setDlgItemText(AP_RID_DIALOG_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	setDlgItemText(AP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL AP_Win32Dialog_Replace::_onInitDialog(HWND hWnd, WPARAM /*wParam*/, LPARAM /*lParam*/)
{	
	
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	if (m_id == AP_DIALOG_ID_FIND)
		setDialogTitle(pSS->getValue(AP_STRING_ID_DLG_FR_FindTitle));
	else
		setDialogTitle(pSS->getValue(AP_STRING_ID_DLG_FR_ReplaceTitle));

	// localize controls shared across dialogs
	_DS(REPLACE_BTN_FINDNEXT,		DLG_FR_FindNextButton);
	_DS(REPLACE_TEXT_FIND,			DLG_FR_FindLabel);
	_DS(REPLACE_CHECK_MATCHCASE,	DLG_FR_MatchCase);
	_DS(REPLACE_CHECK_REVERSEFIND, DLG_FR_ReverseFind);
	_DS(REPLACE_CHECK_WHOLEWORD, DLG_FR_WholeWord);
	
	_DSX(REPLACE_BTN_CLOSE, 		DLG_Cancel);


	if (m_id == AP_DIALOG_ID_REPLACE)
	{

		// localize replace-specific controls
		_DS(REPLACE_BTN_REPLACE,	DLG_FR_ReplaceButton);
		_DS(REPLACE_BTN_REPLACEALL, DLG_FR_ReplaceAllButton);
		_DS(REPLACE_TEXT_REPLACE,	DLG_FR_ReplaceWithLabel);
	}

	_initButtons(hWnd);

	SetFocus( GetDlgItem(hWnd,AP_RID_DIALOG_REPLACE_COMBO_FIND) );

	centerDialog();
	
	return 0;							// 0 == we did call SetFocus()
}

BOOL AP_Win32Dialog_Replace::_onCommand(HWND hWnd, WPARAM wParam, LPARAM /*lParam*/)
{
	WORD wId = LOWORD(wParam);

	// before doing anything else, make sure that the main window is not entirely
	// without focus
	m_pView->focusChange(AV_FOCUS_MODELESS);
	xxx_UT_DEBUGMSG(("s_dlgProc: wId 0x%x\n",wId));

	switch (wId)
	{
	case AP_RID_DIALOG_REPLACE_COMBO_FIND:
	case AP_RID_DIALOG_REPLACE_COMBO_REPLACE:
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
			UT_UCS4Char * pSelection;
			m_pView->getSelectionText(pSelection);
			
			if ( pSelection != NULL) 
			{
				PT_DocPosition pt = m_pView->getSelectionAnchor();
				PT_DocPosition ln = UT_UCS4_strlen (pSelection);
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

			FREEP(pSelection);
		}

		m_pView->findSetStartAtInsPoint();

		return 1;	
		}
	case AP_RID_DIALOG_REPLACE_BTN_FINDNEXT:
		return _onBtn_Find(hWnd, find_FIND_NEXT);
		
	case AP_RID_DIALOG_REPLACE_BTN_REPLACE:
		return _onBtn_Find(hWnd, find_REPLACE);
		
	case AP_RID_DIALOG_REPLACE_BTN_REPLACEALL:
		return _onBtn_Find(hWnd, find_REPLACE_ALL);
		
	case IDCANCEL:						// also AP_RID_DIALOG_REPLACE_BTN_CLOSE
		EndDialog(hWnd,0);
		return 1;

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}

BOOL AP_Win32Dialog_Replace::_onBtn_Find(HWND hWnd, tFindType tFindType)
{
	wchar_t * pBufFromDialogFind = NULL;
	wchar_t * pBufFromDialogReplace = NULL;
	UT_UCSChar * pUCSReplace = NULL;
    UT_Win32LocaleString findstr;    
    UT_Win32LocaleString replacestr;

	// Check find string
	HWND hWndEditFind = GetDlgItem(hWnd,AP_RID_DIALOG_REPLACE_COMBO_FIND);
	DWORD lenFind = GetWindowTextLengthW(hWndEditFind);
	if (!lenFind)
		return 1;

	pBufFromDialogFind = new wchar_t [lenFind + 1];
	if (!pBufFromDialogFind)
		goto FreeMemory;
	GetWindowTextW(hWndEditFind,pBufFromDialogFind,lenFind+1);

	UT_DEBUGMSG(("Find entry contents: [%s]\n",pBufFromDialogFind));

    findstr.fromLocale (pBufFromDialogFind);

	setFindString(findstr.ucs4_str().ucs4_str());

    if (m_id == AP_DIALOG_ID_REPLACE)
	{
		// Check Replace string
		HWND hWndEditReplace = GetDlgItem(hWnd,AP_RID_DIALOG_REPLACE_COMBO_REPLACE);
		DWORD lenReplace = GetWindowTextLengthW(hWndEditReplace);

		pBufFromDialogReplace = new wchar_t [lenReplace + 1];
		if (!pBufFromDialogReplace)
			goto FreeMemory;
		GetWindowTextW(hWndEditReplace,pBufFromDialogReplace,lenReplace+1);

		UT_DEBUGMSG(("Replace entry contents: [%s]\n",pBufFromDialogReplace));

	    replacestr.fromLocale (pBufFromDialogReplace);
		setReplaceString(replacestr.ucs4_str().ucs4_str());
	}

	if (tFindType == find_REPLACE_ALL)
	{
		UT_return_val_if_fail ((m_id == AP_DIALOG_ID_REPLACE),0); //should never happen in Find dialog
		findReplaceAll();
	}
	else if (tFindType == find_REPLACE)
	{
		UT_return_val_if_fail ((m_id == AP_DIALOG_ID_REPLACE),0); //should never happen in Find dialog
		if (!getReverseFind())
			findReplace();
		else
			findReplaceReverse();
	}
	else if (tFindType == find_FIND_NEXT)
	{
		if (!getReverseFind())
			findNext();
		else
			findPrev();
	}

    FreeMemory:
	DELETEP(pBufFromDialogFind);
	DELETEP(pBufFromDialogReplace);
	FREEP(pUCSReplace);
	return 1;
}

void AP_Win32Dialog_Replace::_updateLists()
{		
	_updateList(GetDlgItem(m_hDlg, AP_RID_DIALOG_REPLACE_COMBO_FIND), 		&m_findList);
	_updateList(GetDlgItem(m_hDlg, AP_RID_DIALOG_REPLACE_COMBO_REPLACE), 	&m_replaceList);
}

void AP_Win32Dialog_Replace::_updateList(HWND hWnd, UT_GenericVector<UT_UCS4Char*> *list)
{
	UT_DEBUGMSG(("AP_Win32Dialog_Replace::_updateList\n"));
	
    UT_Win32LocaleString str;
	
	SendMessageW(hWnd, CB_RESETCONTENT, 0,0);		
	
	for (UT_sint32 i = 0; i < list->getItemCount(); i++)
	{
		// leaving the size 0 causes the string class to determine the length itself
        str.fromUCS4 ((UT_UCS4Char*)list->getNthItem(i));
    	SendMessageW(hWnd, CB_ADDSTRING, 0, (LPARAM) str.c_str());		
	}		
	
	SendMessageW(hWnd, CB_SETCURSEL, 0,0);		
}

