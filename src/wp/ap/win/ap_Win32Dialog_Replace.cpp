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

//////////////////////////////////////////////////////////////////
// THIS CODE RUNS BOTH THE "Find" AND THE "Find-Replace" DIALOGS.
//////////////////////////////////////////////////////////////////

#include <windows.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_Win32App.h"
#include "xap_Win32Frame.h"

#include "ap_Dialog_Id.h"
#include "ap_Dialog_Replace.h"
#include "ap_Win32Dialog_Replace.h"

#include "ap_Win32Resources.rc2"

#define FREEP(p)	do { if (p) free(p); (p)=NULL; } while (0)
#define DELETEP(p)	do { if (p) delete p; } while (0)

/*****************************************************************/
AP_Dialog * AP_Win32Dialog_Replace::static_constructor(AP_DialogFactory * pFactory,
													   AP_Dialog_Id id)
{
	AP_Win32Dialog_Replace * p = new AP_Win32Dialog_Replace(pFactory,id);
	return p;
}

AP_Win32Dialog_Replace::AP_Win32Dialog_Replace(AP_DialogFactory * pDlgFactory,
											   AP_Dialog_Id id)
	: AP_Dialog_Replace(pDlgFactory,id)
{
	m_findString = NULL;
	m_replaceString = NULL;
    m_matchCase = UT_TRUE;
}

AP_Win32Dialog_Replace::~AP_Win32Dialog_Replace(void)
{
}

void AP_Win32Dialog_Replace::runModal(XAP_Frame * pFrame)
{
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(pFrame->getApp());
	XAP_Win32Frame * pWin32Frame = static_cast<XAP_Win32Frame *>(pFrame);

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
	
	int result = DialogBoxParam(pWin32App->getInstance(),lpTemplate,
								pWin32Frame->getTopLevelWindow(),
								(DLGPROC)s_dlgProc,(LPARAM)this);
	UT_ASSERT((result != -1));
}

BOOL CALLBACK AP_Win32Dialog_Replace::s_dlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// This is a static function.

	AP_Win32Dialog_Replace * pThis;
	
	switch (msg)
	{
	case WM_INITDIALOG:
		pThis = (AP_Win32Dialog_Replace *)lParam;
		SetWindowLong(hWnd,DWL_USER,lParam);
		return pThis->_onInitDialog(hWnd,wParam,lParam);
		
	case WM_COMMAND:
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
		BOOL bEnableReplace = bEnableFind && (lenReplace > 0);
		EnableWindow(GetDlgItem(hWnd,AP_RID_DIALOG_REPLACE_BTN_REPLACE),bEnableReplace);
		EnableWindow(GetDlgItem(hWnd,AP_RID_DIALOG_REPLACE_BTN_REPLACEALL),bEnableReplace);
	}
	
	return;
}

BOOL AP_Win32Dialog_Replace::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	{
		UT_UCSChar * bufferUnicode = getFindString();
		UT_uint32 lenUnicode = UT_UCS_strlen(bufferUnicode);
		if (lenUnicode)
		{
			char * bufferNormal = new char [lenUnicode + 1];
			UT_UCS_strcpy_to_char(bufferNormal, bufferUnicode);
			SetDlgItemText(hWnd,AP_RID_DIALOG_REPLACE_EDIT_FIND,bufferNormal);
			DELETEP(bufferNormal);
		}
		FREEP(bufferUnicode);
	}
	if (m_id == AP_DIALOG_ID_REPLACE)
	{
		UT_UCSChar * bufferUnicode = getReplaceString();
		UT_uint32 lenUnicode = UT_UCS_strlen(bufferUnicode);
		if (lenUnicode)
		{
			char * bufferNormal = new char [lenUnicode + 1];
			UT_UCS_strcpy_to_char(bufferNormal, bufferUnicode);
			SetDlgItemText(hWnd,AP_RID_DIALOG_REPLACE_EDIT_REPLACE,bufferNormal);
			DELETEP(bufferNormal);
		}
		FREEP(bufferUnicode);
	}

	CheckDlgButton(hWnd,AP_RID_DIALOG_REPLACE_CHECK_MATCHCASE,
				   ((getMatchCase()) ? BST_CHECKED : BST_UNCHECKED));
	
	_initButtons(hWnd);
	return 1;							// 1 == we did not call SetFocus()
}

BOOL AP_Win32Dialog_Replace::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;

	switch (wId)
	{
	case AP_RID_DIALOG_REPLACE_EDIT_FIND:
	case AP_RID_DIALOG_REPLACE_EDIT_REPLACE:
		_initButtons(hWnd);
		return 1;

	case AP_RID_DIALOG_REPLACE_CHECK_MATCHCASE:
		setMatchCase((IsDlgButtonChecked(hWnd,AP_RID_DIALOG_REPLACE_CHECK_MATCHCASE)==BST_CHECKED));
		return 1;

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

	UT_UCS_cloneString_char(&pUCSFind,pBufFromDialogFind);
	if (!pUCSFind)
		goto FreeMemory;

	setFindString(pUCSFind);
	findNext();

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

	UT_UCS_cloneString_char(&pUCSFind,pBufFromDialogFind);
	if (!pUCSFind)
		goto FreeMemory;

	pBufFromDialogReplace = new char [lenReplace + 1];
	if (!pBufFromDialogReplace)
		goto FreeMemory;
	GetWindowText(hWndEditReplace,pBufFromDialogReplace,lenReplace+1);

	UT_DEBUGMSG(("Replace entry contents: [%s]\n",pBufFromDialogReplace));

	UT_UCS_cloneString_char(&pUCSReplace,pBufFromDialogReplace);
	if (!pUCSReplace)
		goto FreeMemory;

	setFindString(pUCSFind);
	setReplaceString(pUCSReplace);
	findReplace();

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

	UT_UCS_cloneString_char(&pUCSFind,pBufFromDialogFind);
	if (!pUCSFind)
		goto FreeMemory;

	pBufFromDialogReplace = new char [lenReplace + 1];
	if (!pBufFromDialogReplace)
		goto FreeMemory;
	GetWindowText(hWndEditReplace,pBufFromDialogReplace,lenReplace+1);

	UT_DEBUGMSG(("Replace entry contents: [%s]\n",pBufFromDialogReplace));

	UT_UCS_cloneString_char(&pUCSReplace,pBufFromDialogReplace);
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
