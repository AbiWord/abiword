/* AbiWord
 * Copyright (C) 2002 Gabriel Gerhardsson
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
#include <string.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Frame.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Win32Dialog_Download_File.h"

#include "ut_worker.h"
#include "ut_string_class.h"


#include "ap_Win32Resources.rc2"

XAP_Dialog * AP_Win32Dialog_Download_File::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_Win32Dialog_Download_File * p = new AP_Win32Dialog_Download_File(pFactory,id);
	return (XAP_Dialog*)p;
}

AP_Win32Dialog_Download_File::AP_Win32Dialog_Download_File(XAP_DialogFactory * pDlgFactory,
											 XAP_Dialog_Id id)
	: AP_Dialog_Download_File(pDlgFactory,id),
	  dlDlgHandle(NULL)
{
}

AP_Win32Dialog_Download_File::~AP_Win32Dialog_Download_File(void)
{
	_abortDialog();	// does nothing if no dialog
}


// called to display the dialog, displayed until user presses cancel
// or download complete as indicated by call to _abortDialog() below.
void AP_Win32Dialog_Download_File::_runModal(XAP_Frame * pFrame)
{
	// raise the dialog
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);

	LPCTSTR lpTemplate = NULL;

	UT_ASSERT(m_id == AP_DIALOG_ID_DOWNLOAD_FILE);

	lpTemplate = MAKEINTRESOURCE(AP_RID_DIALOG_DOWNLOADFILE);

	int result = DialogBoxParam(pWin32App->getInstance(),lpTemplate,
					static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow(),
					(DLGPROC)s_dlgProc,(LPARAM)this);
	UT_ASSERT((result != -1));
}

void
AP_Win32Dialog_Download_File::_abortDialog(void)
{
	if (dlDlgHandle != NULL)
	{
		_setUserAnswer(a_NONE);
		EndDialog(dlDlgHandle, 0);
	}
}


BOOL CALLBACK AP_Win32Dialog_Download_File::s_dlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// This is a static function.

	AP_Win32Dialog_Download_File * pThis;
	
	switch (msg)
	{
	case WM_INITDIALOG:
		pThis = (AP_Win32Dialog_Download_File *)lParam;
		SetWindowLong(hWnd,DWL_USER,lParam);
		return pThis->_onInitDialog(hWnd,wParam,lParam);
		
	case WM_COMMAND:
		pThis = (AP_Win32Dialog_Download_File *)GetWindowLong(hWnd,DWL_USER);
		return pThis->_onCommand(hWnd,wParam,lParam);
		
	default:
		return 0;
	}
}

#define _DS(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL AP_Win32Dialog_Download_File::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	dlDlgHandle = hWnd;	// store dialog handle so we can programmatically close it

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	SetWindowText(hWnd, getTitle());

	// localize controls
	_DSX(DOWNLOADFILE_BTN_CANCEL,		DLG_Cancel);

	char buf[4096];
	sprintf(buf, pSS->getValue(AP_STRING_ID_DLG_DlFile_Status), getDescription(), getURL());
	SetDlgItemText(hWnd,AP_RID_DIALOG_DOWNLOADFILE_TEXT, buf);

	return 1;							// 1 == we did not call SetFocus()
}

BOOL AP_Win32Dialog_Download_File::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;

	switch (wId)
	{
	case IDCANCEL:						// also AP_RID_DIALOG_DOWNLOADFILE_BTN_CANCEL
		_setUserAnswer(a_CANCEL);
		dlDlgHandle = NULL;				// important, indicate no dialog open

		EndDialog(hWnd,0);
		return 1;

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}
