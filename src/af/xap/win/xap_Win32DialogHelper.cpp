// xap_Win32DialogHelper.cpp

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

#include <windows.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32Frame.h"

#include "xap_Win32DialogHelper.h"

void XAP_Win32DialogHelper::runModal(XAP_Frame * pFrame, XAP_Dialog_Id dialog_id, UT_sint32 resource_id, XAP_Dialog *p_dialog)
{
	UT_ASSERT(m_pDialog != NULL);

	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(p_dialog->getApp());
	XAP_Win32Frame * pWin32Frame = static_cast<XAP_Win32Frame *>(pFrame);


	UT_ASSERT(p_dialog->getDialogId() == dialog_id);

	LPCTSTR lpTemplate = MAKEINTRESOURCE(resource_id);

	int result = DialogBoxParam(pWin32App->getInstance(), lpTemplate,
								pWin32Frame->getTopLevelWindow(),
								(DLGPROC)s_dlgProc, (LPARAM)this);
	UT_ASSERT((result != -1));
}

void XAP_Win32DialogHelper::runModeless(XAP_Frame * pFrame, XAP_Dialog_Id dialog_id, UT_sint32 resource_id, XAP_Dialog_Modeless *p_dialog)
{
	UT_ASSERT(pFrame);

	// raise the dialog
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(p_dialog->getApp());
	XAP_Win32Frame * pWin32Frame = static_cast<XAP_Win32Frame *>(pFrame);

	LPCTSTR lpTemplate = NULL;

	UT_ASSERT(p_dialog->getDialogId() == dialog_id);

	lpTemplate = MAKEINTRESOURCE(resource_id);

	HWND hWndDialog = CreateDialogParam(pWin32App->getInstance(),lpTemplate,
								pWin32Frame->getTopLevelWindow(),
								(DLGPROC)s_dlgProc,(LPARAM)this);
	ShowWindow(hWndDialog, SW_SHOW);
	UT_ASSERT((hWndDialog != NULL));

	p_dialog->getApp()->rememberModelessId(dialog_id, p_dialog);


}


BOOL CALLBACK XAP_Win32DialogHelper::s_dlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// This is a static function.

	XAP_Win32DialogHelper * pThis;
	
	switch (msg)
	{
	case WM_INITDIALOG:
		pThis = (XAP_Win32DialogHelper *)lParam;
		pThis->m_hDlg = hWnd;
		SetWindowLong(hWnd,DWL_USER,lParam);
		return pThis->m_pDialog->_onInitDialog(hWnd,wParam,lParam);
		
	case WM_COMMAND:
		pThis = (XAP_Win32DialogHelper *)GetWindowLong(hWnd,DWL_USER);
		if(pThis)
			return pThis->m_pDialog->_onCommand(hWnd,wParam,lParam);
		else
			return 0;


	default:
		return 0;
	}
}


