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

#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_Win32Dlg_HTMLOptions.h"
#include "xap_Win32Resources.rc2"

/*****************************************************************/

XAP_Dialog * XAP_Win32Dialog_HTMLOptions::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	XAP_Win32Dialog_HTMLOptions * p = new XAP_Win32Dialog_HTMLOptions(pFactory,id);
	return p;
}

XAP_Win32Dialog_HTMLOptions::XAP_Win32Dialog_HTMLOptions(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: XAP_Dialog_HTMLOptions(pDlgFactory,id)
{
}

XAP_Win32Dialog_HTMLOptions::~XAP_Win32Dialog_HTMLOptions(void)
{
}

void XAP_Win32Dialog_HTMLOptions::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);
	
	XAP_App *pApp = XAP_App::getApp();
	UT_return_if_fail(pApp);


	LPCTSTR lpTemplate = NULL;

	UT_ASSERT(m_id == XAP_DIALOG_ID_HTMLOPTIONS);
	lpTemplate = MAKEINTRESOURCE(XAP_RID_DIALOG_HTMLOPTIONS);

	int result = DialogBoxParam(	static_cast<XAP_Win32App *>(XAP_App::getApp())->getInstance(),
					  				lpTemplate,
									static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow(),
									(DLGPROC)s_dlgProc,
									(LPARAM)this );
	UT_ASSERT((result != -1));	
}


BOOL CALLBACK XAP_Win32Dialog_HTMLOptions::s_dlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// This is a static function.

	XAP_Win32Dialog_HTMLOptions * pThis;
	
	switch (msg)
	{
	case WM_INITDIALOG:
		pThis = (XAP_Win32Dialog_HTMLOptions *)lParam;
		SetWindowLong(hWnd,DWL_USER,lParam);
		return pThis->_onInitDialog(hWnd,wParam,lParam);
		
	case WM_COMMAND:
		pThis = (XAP_Win32Dialog_HTMLOptions *)GetWindowLong(hWnd,DWL_USER);
		if (pThis)
			return pThis->_onCommand(hWnd,wParam,lParam);
		else
			return 0;

	default:
		return 0;
	}
}

#define _DS(c,s)	SetDlgItemText(hWnd,XAP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL XAP_Win32Dialog_HTMLOptions::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	SetWindowText(hWnd, pSS->getValue(XAP_STRING_ID_DLG_HTMLOPT_ExpTitle));

	// localize controls
	_DS(HTMLOPTIONS_LBL,			DLG_HTMLOPT_ExpLabel);
	_DS(HTMLOPTIONS_CHK_HTML4,		DLG_HTMLOPT_ExpIs4);
	_DS(HTMLOPTIONS_CHK_PHP,		DLG_HTMLOPT_ExpAbiWebDoc);
	_DS(HTMLOPTIONS_CHK_XML,		DLG_HTMLOPT_ExpDeclareXML);
	_DS(HTMLOPTIONS_CHK_AWML,		DLG_HTMLOPT_ExpAllowAWML);
	_DS(HTMLOPTIONS_CHK_EMBEDCSS,	DLG_HTMLOPT_ExpEmbedCSS);
	_DS(HTMLOPTIONS_CHK_URLIMAGE,	DLG_HTMLOPT_ExpEmbedImages);
	_DS(HTMLOPTIONS_BTN_SAVE,		DLG_HTMLOPT_ExpSave);
	_DS(HTMLOPTIONS_BTN_RESTORE,	DLG_HTMLOPT_ExpRestore);
	_DS(HTMLOPTIONS_BTN_OK,			DLG_OK);
	_DS(HTMLOPTIONS_BTN_CANCEL,		DLG_Cancel);

	// Set Initial conditions
	refreshStates(hWnd);

	return 1;							// 1 == we did not call SetFocus()
}

BOOL XAP_Win32Dialog_HTMLOptions::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;

	switch (wId)
	{
	case XAP_RID_DIALOG_HTMLOPTIONS_CHK_HTML4:
		set_HTML4( (IsDlgButtonChecked(hWnd, wId)!=0) ? true : false );
		return 1;
		
	case XAP_RID_DIALOG_HTMLOPTIONS_CHK_PHP:
		set_PHTML( (IsDlgButtonChecked(hWnd, wId)!=0) ? true : false );
		return 1;

	case XAP_RID_DIALOG_HTMLOPTIONS_CHK_XML:
		set_Declare_XML( (IsDlgButtonChecked(hWnd, wId)!=0) ? true : false );
		return 1;

	case XAP_RID_DIALOG_HTMLOPTIONS_CHK_AWML:
		set_Allow_AWML( (IsDlgButtonChecked(hWnd, wId)!=0) ? true : false );
		return 1;

	case XAP_RID_DIALOG_HTMLOPTIONS_CHK_EMBEDCSS:
		set_Embed_CSS( (IsDlgButtonChecked(hWnd, wId)!=0) ? true : false );
		return 1;

	case XAP_RID_DIALOG_HTMLOPTIONS_CHK_URLIMAGE:
		set_Embed_Images( (IsDlgButtonChecked(hWnd, wId)!=0) ? true : false );
		return 1;

	case XAP_RID_DIALOG_HTMLOPTIONS_BTN_SAVE:
		saveDefaults();
		return 1;

	case XAP_RID_DIALOG_HTMLOPTIONS_BTN_RESTORE:
		restoreDefaults();
		refreshStates(hWnd);
		return 1;
					
	case IDCANCEL:						// also XAP_RID_DIALOG_ZOOM_BTN_CANCEL
		m_bShouldSave = false;
		EndDialog(hWnd,0);
		return 1;

	case IDOK:							// also XAP_RID_DIALOG_ZOOM_BTN_OK
		m_bShouldSave = true;
		EndDialog(hWnd,0);
		return 1;

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}

void XAP_Win32Dialog_HTMLOptions::refreshStates(HWND hWnd)
{
	// determine which controls are available.
	EnableWindow(GetDlgItem(hWnd, XAP_RID_DIALOG_HTMLOPTIONS_CHK_XML),can_set_Declare_XML());
	EnableWindow(GetDlgItem(hWnd, XAP_RID_DIALOG_HTMLOPTIONS_CHK_AWML),can_set_Allow_AWML());
	EnableWindow(GetDlgItem(hWnd, XAP_RID_DIALOG_HTMLOPTIONS_CHK_EMBEDCSS),can_set_Embed_CSS());
	EnableWindow(GetDlgItem(hWnd, XAP_RID_DIALOG_HTMLOPTIONS_CHK_URLIMAGE),can_set_Embed_Images());

	// set initial state
	CheckDlgButton(hWnd, XAP_RID_DIALOG_HTMLOPTIONS_CHK_HTML4,get_HTML4());
	CheckDlgButton(hWnd, XAP_RID_DIALOG_HTMLOPTIONS_CHK_PHP,get_PHTML()); 
	CheckDlgButton(hWnd, XAP_RID_DIALOG_HTMLOPTIONS_CHK_XML,get_Declare_XML());
	CheckDlgButton(hWnd, XAP_RID_DIALOG_HTMLOPTIONS_CHK_AWML,get_Allow_AWML());
	CheckDlgButton(hWnd, XAP_RID_DIALOG_HTMLOPTIONS_CHK_EMBEDCSS,get_Embed_CSS());
	CheckDlgButton(hWnd, XAP_RID_DIALOG_HTMLOPTIONS_CHK_URLIMAGE,get_Embed_Images());
}
