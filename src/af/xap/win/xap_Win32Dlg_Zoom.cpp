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

#include <windows.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32Frame.h"

#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_Zoom.h"
#include "xap_Win32Dlg_Zoom.h"

#include "xap_Win32Resources.rc2"

/*****************************************************************/

XAP_Dialog * XAP_Win32Dialog_Zoom::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id id)
{
	XAP_Win32Dialog_Zoom * p = new XAP_Win32Dialog_Zoom(pFactory,id);
	return p;
}

XAP_Win32Dialog_Zoom::XAP_Win32Dialog_Zoom(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id id)
	: XAP_Dialog_Zoom(pDlgFactory,id)
{

}

XAP_Win32Dialog_Zoom::~XAP_Win32Dialog_Zoom(void)
{
}

/*****************************************************************/

void XAP_Win32Dialog_Zoom::runModal(XAP_Frame * pFrame)
{
	/*
	  This dialog is non-persistent.
	  
	  This dialog should do the following:

	  - Construct itself to represent the base-class zoomTypes
	    z_200, z_100, z_75, z_PageWidth, z_WholePage, and z_Percent.
		The Unix one looks just like Microsoft Word 97, with the preview
		and all (even though it's not hooked up yet).

	  - Set zoom type to match "m_zoomType" and value of radio button
	    to match "m_zoomPercent".

	  On "OK" (or during user-interaction) the dialog should:

	  - Save the zoom type to "m_zoomType".
	  
	  - Save the value in the Percent spin button box to "m_zoomPercent".

	  On "Cancel" the dialog should:

	  - Just quit, the data items will be ignored by the caller.

	*/

	// raise the dialog
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);
	XAP_Win32Frame * pWin32Frame = static_cast<XAP_Win32Frame *>(pFrame);

	LPCTSTR lpTemplate = NULL;

	UT_ASSERT(m_id == XAP_DIALOG_ID_ZOOM);

	lpTemplate = MAKEINTRESOURCE(XAP_RID_DIALOG_ZOOM);

	int result = DialogBoxParam(pWin32App->getInstance(),lpTemplate,
								pWin32Frame->getTopLevelWindow(),
								(DLGPROC)s_dlgProc,(LPARAM)this);
	UT_ASSERT((result != -1));
}

BOOL CALLBACK XAP_Win32Dialog_Zoom::s_dlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// This is a static function.

	XAP_Win32Dialog_Zoom * pThis;
	
	switch (msg)
	{
	case WM_INITDIALOG:
		pThis = (XAP_Win32Dialog_Zoom *)lParam;
		SetWindowLong(hWnd,DWL_USER,lParam);
		return pThis->_onInitDialog(hWnd,wParam,lParam);
		
	case WM_COMMAND:
		pThis = (XAP_Win32Dialog_Zoom *)GetWindowLong(hWnd,DWL_USER);
		return pThis->_onCommand(hWnd,wParam,lParam);
		
	default:
		return 0;
	}
}

#define _DS(c,s)	SetDlgItemText(hWnd,XAP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL XAP_Win32Dialog_Zoom::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	SetWindowText(hWnd, pSS->getValue(XAP_STRING_ID_DLG_Zoom_ZoomTitle));

	// localize controls
	_DS(ZOOM_TEXT_ZOOMTO,		DLG_Zoom_RadioFrameCaption);
	_DS(ZOOM_RADIO_200,			DLG_Zoom_200);
	_DS(ZOOM_RADIO_100,			DLG_Zoom_100);
	_DS(ZOOM_RADIO_75,			DLG_Zoom_75);
	_DS(ZOOM_RADIO_WIDTH,		DLG_Zoom_PageWidth);
	_DS(ZOOM_RADIO_WHOLE,		DLG_Zoom_WholePage);
	_DS(ZOOM_RADIO_PCT,			DLG_Zoom_Percent);
	_DS(ZOOM_TEXT_PREVIEW,		DLG_Zoom_PreviewFrame);
	_DS(ZOOM_BTN_OK,			DLG_OK);
	_DS(ZOOM_BTN_CANCEL,		DLG_Cancel);

	// set initial state
	CheckDlgButton(hWnd, XAP_RID_DIALOG_ZOOM_RADIO_200 + m_zoomType, BST_CHECKED);
	SetDlgItemInt(hWnd, XAP_RID_DIALOG_ZOOM_EDIT_PCT, (UINT) m_zoomPercent, FALSE);
 
#if 0
XAP_RID_DIALOG_ZOOM_SPIN_PCT				1009
XAP_RID_DIALOG_ZOOM_TEXT_FONT				1011

	{
		HWND hwndPreview = GetDlgItem(hWnd, XAP_RID_DIALOG_ZOOM_PREVIEW);  
		RECT r;
		GetClientRect(hwndPreview, &r);

		GR_Win32Graphics * pG = new GR_Win32Graphics(GetDC(hwndPreview), hwndPreview);
		m_pGPreview = pG;

		// instantiate the XP preview widget
		_createPreviewFromGC(m_pGPreview,(UT_uint32) r.right,(UT_uint32) r.bottom);
		_updatePreviewZoomPercent(getZoomPercent());

		// TODO: how do we set CS_OWNDC on this, so that we can reuse pG?
	}		
#endif

	return 1;							// 1 == we did not call SetFocus()
}

// TODO: move this to UTIL/WIN
static int _getRBOffset(HWND hWnd, int nIDFirstButton, int nIDLastButton)
{
	UT_ASSERT(hWnd && IsWindow(hWnd));
	UT_ASSERT(nIDFirstButton < nIDLastButton);

	for (int i = nIDFirstButton; i <= nIDLastButton; i++)
		if (BST_CHECKED == IsDlgButtonChecked(hWnd, i))
			return (i - nIDFirstButton);

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return -1;
}

BOOL XAP_Win32Dialog_Zoom::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;
	int n;
	BOOL bOK;

	switch (wId)
	{
#if 0
	case XAP_RID_DIALOG_ZOOM_LIST:
		switch (HIWORD(wParam))
		{
			case LBN_SELCHANGE:
				// NOTE: we could get away with only grabbing this in IDOK case
				nItem = SendMessage(hWndCtrl, LB_GETCURSEL, 0, 0);
				m_ndxSelFrame = SendMessage(hWndCtrl, LB_GETITEMDATA, nItem, 0);
				return 1;

			case LBN_DBLCLK:
				nItem = SendMessage(hWndCtrl, LB_GETCURSEL, 0, 0);
				m_ndxSelFrame = SendMessage(hWndCtrl, LB_GETITEMDATA, nItem, 0);
				EndDialog(hWnd,0);
				return 1;

			default:
				return 0;
		}
		break;
#endif

	case IDCANCEL:						// also XAP_RID_DIALOG_ZOOM_BTN_CANCEL
		m_answer = a_CANCEL;
		// fall through

	case IDOK:							// also XAP_RID_DIALOG_ZOOM_BTN_OK
		n = _getRBOffset(hWnd, XAP_RID_DIALOG_ZOOM_RADIO_200, XAP_RID_DIALOG_ZOOM_RADIO_PCT);
		UT_ASSERT(n >= 0);

		m_zoomType = (XAP_Dialog_Zoom::zoomType) n;
		m_zoomPercent = (UT_uint32) GetDlgItemInt(hWnd, XAP_RID_DIALOG_ZOOM_EDIT_PCT, &bOK, FALSE); 
		UT_ASSERT(bOK);
 
		EndDialog(hWnd,0);
		return 1;

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}

