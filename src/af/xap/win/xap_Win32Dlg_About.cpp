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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Dialog_Id.h"
#include "xap_Win32App.h"
#include "xap_Win32Frame.h"

#include "xap_Dlg_About.h"
#include "xap_Win32Dlg_About.h"

#include "xap_Win32Resources.rc2"


/*****************************************************************/
XAP_Dialog * XAP_Win32Dialog_About::static_constructor(XAP_DialogFactory * pFactory,
															 XAP_Dialog_Id id)
{
	XAP_Win32Dialog_About * p = new XAP_Win32Dialog_About(pFactory,id);
	return p;
}

XAP_Win32Dialog_About::XAP_Win32Dialog_About(XAP_DialogFactory * pDlgFactory,
											 XAP_Dialog_Id id)
	: XAP_Dialog_About(pDlgFactory,id)
{
}

XAP_Win32Dialog_About::~XAP_Win32Dialog_About(void)
{
}

void XAP_Win32Dialog_About::runModal(XAP_Frame * pFrame)
{
	// raise the dialog
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);
	XAP_Win32Frame * pWin32Frame = static_cast<XAP_Win32Frame *>(pFrame);

	LPCTSTR lpTemplate = NULL;

	UT_ASSERT(m_id == XAP_DIALOG_ID_ABOUT);

	lpTemplate = MAKEINTRESOURCE(XAP_RID_DIALOG_ABOUT);

	int result = DialogBoxParam(pWin32App->getInstance(),lpTemplate,
								pWin32Frame->getTopLevelWindow(),
								(DLGPROC)s_dlgProc,(LPARAM)this);
	UT_ASSERT((result != -1));
}

BOOL CALLBACK XAP_Win32Dialog_About::s_dlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// This is a static function.

	XAP_Win32Dialog_About * pThis;
	
	switch (msg)
	{
	case WM_INITDIALOG:
		pThis = (XAP_Win32Dialog_About *)lParam;
		SetWindowLong(hWnd,DWL_USER,lParam);
		return pThis->_onInitDialog(hWnd,wParam,lParam);
		
	case WM_COMMAND:
		pThis = (XAP_Win32Dialog_About *)GetWindowLong(hWnd,DWL_USER);
		return pThis->_onCommand(hWnd,wParam,lParam);
		
	default:
		return 0;
	}
}

BOOL XAP_Win32Dialog_About::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	char buf[2048];

	const char * szAppName = m_pApp->getApplicationName();

	sprintf(buf, XAP_ABOUT_TITLE, szAppName);
	SetWindowText(hWnd, buf);

	{
		HWND hwndStatic;
		
		hwndStatic = GetDlgItem(hWnd, XAP_RID_DIALOG_ABOUT_DESCRIPTION);
		sprintf(buf, XAP_ABOUT_DESCRIPTION, szAppName);
		SetWindowText(hwndStatic, buf);

		hwndStatic = GetDlgItem(hWnd, XAP_RID_DIALOG_ABOUT_COPYRIGHT);
		SetWindowText(hwndStatic, XAP_ABOUT_COPYRIGHT);

		hwndStatic = GetDlgItem(hWnd, XAP_RID_DIALOG_ABOUT_GPL);
		sprintf(buf, XAP_ABOUT_GPL, szAppName);
		SetWindowText(hwndStatic, buf);

		hwndStatic = GetDlgItem(hWnd, XAP_RID_DIALOG_ABOUT_VERSION);
		sprintf(buf, XAP_ABOUT_VERSION, XAP_App::s_szBuild_Version); 
		SetWindowText(hwndStatic, buf);

		hwndStatic = GetDlgItem(hWnd, XAP_RID_DIALOG_ABOUT_URL);
		SetWindowText(hwndStatic, XAP_ABOUT_URL);

		hwndStatic = GetDlgItem(hWnd, XAP_RID_DIALOG_ABOUT_BUILD);
		sprintf(buf, XAP_ABOUT_BUILD, XAP_App::s_szBuild_Options);
		SetWindowText(hwndStatic, buf);
	}		

	return 1;							// 1 == we did not call SetFocus()
}

BOOL XAP_Win32Dialog_About::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;

	switch (wId)
	{
	case IDCANCEL:
	case IDOK:							// also XAP_RID_DIALOG_ABOUT_BTN_OK
		EndDialog(hWnd,0);
		return 1;

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}

