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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32DialogHelper.h"
#include "xap_Win32Toolbar_Icons.h"
#include "ut_Win32LocaleString.h"


static void _assertValidDlgHandle(HWND hDlg)
{
	UT_DEBUG_ONLY_ARG(hDlg);
	UT_ASSERT(IsWindow(hDlg));
}



void XAP_Win32DialogHelper::runModal(XAP_Frame * pFrame, XAP_Dialog_Id dialog_id, UT_sint32 resource_id, XAP_Dialog *p_dialog)
{
	UT_DEBUG_ONLY_ARG(dialog_id);
	UT_ASSERT(m_pDialog != NULL);

	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(p_dialog->getApp());

	UT_ASSERT(p_dialog->getDialogId() == dialog_id);

	LPCWSTR lpTemplate = MAKEINTRESOURCEW(resource_id);

	UT_DebugOnly<int> result = DialogBoxParamW(pWin32App->getInstance(), lpTemplate,
								static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow(),
								(DLGPROC)s_dlgProc, (LPARAM)this);
	UT_ASSERT((result != -1));
}



void XAP_Win32DialogHelper::runModeless(XAP_Frame * pFrame, XAP_Dialog_Id dialog_id, UT_sint32 resource_id, XAP_Dialog_Modeless *p_dialog)
{
	UT_ASSERT(pFrame);

	// raise the dialog
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(p_dialog->getApp());

	LPCWSTR lpTemplate = NULL;

	UT_ASSERT(p_dialog->getDialogId() == dialog_id);

	lpTemplate = MAKEINTRESOURCEW(resource_id);

	HWND hWndDialog = CreateDialogParamW(pWin32App->getInstance(),lpTemplate,
								static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow(),
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
		_assertValidDlgHandle(hWnd);
		SetWindowLongPtrW(hWnd,DWLP_USER,lParam);
		return pThis->m_pDialog->_onInitDialog(hWnd,wParam,lParam);

	case WM_COMMAND:
		pThis = (XAP_Win32DialogHelper *)GetWindowLongPtrW(hWnd,DWLP_USER);
		if(pThis)
			return pThis->m_pDialog->_onCommand(hWnd,wParam,lParam);
		else
			return 0;

	case WM_NOTIFY:
		pThis = (XAP_Win32DialogHelper *)GetWindowLongPtrW(hWnd,DWLP_USER);
		switch (((LPNMHDR)lParam)->code)
		{
			case UDN_DELTAPOS:		return pThis->m_pDialog->_onDeltaPos((NM_UPDOWN *)lParam);
			default:				return 0;
		}


	default:
		return 0;
	}
}


void XAP_Win32DialogHelper::checkButton(UT_sint32 controlId, bool bChecked)
{
	_assertValidDlgHandle(m_hDlg);
	CheckDlgButton(m_hDlg, controlId, bChecked ? BST_CHECKED : BST_UNCHECKED);
}

void XAP_Win32DialogHelper::enableControl(UT_sint32 controlId, bool bChecked)
{
	_assertValidDlgHandle(m_hDlg);
	EnableWindow(GetDlgItem(m_hDlg, controlId), bChecked ? TRUE : FALSE);
}

void XAP_Win32DialogHelper::destroyWindow()
{
	_assertValidDlgHandle(m_hDlg);
	DestroyWindow(m_hDlg);
}

void XAP_Win32DialogHelper::setDialogTitle(LPCSTR p_str)
{
	_assertValidDlgHandle(m_hDlg);
	UT_Win32LocaleString str;
	str.fromUTF8(p_str);
	SetWindowTextW(m_hDlg, str.c_str());
}

int XAP_Win32DialogHelper::showWindow(int Mode )
{
	_assertValidDlgHandle(m_hDlg);
	return ShowWindow(m_hDlg, Mode);
}

int XAP_Win32DialogHelper::showControl(UT_sint32 controlId, int Mode)
{
	_assertValidDlgHandle(m_hDlg);
	return ShowWindow(GetDlgItem(m_hDlg, controlId), Mode);
}

int XAP_Win32DialogHelper::bringWindowToTop()
{
	_assertValidDlgHandle(m_hDlg);
	const UINT uFlags =	SWP_NOMOVE |
						SWP_NOOWNERZORDER |
						SWP_NOSIZE |
						SWP_NOACTIVATE;
	return SetWindowPos(m_hDlg, HWND_TOP, 0, 0, 0, 0, uFlags);
}

// Combo boxes.

int XAP_Win32DialogHelper::addItemToCombo(UT_sint32 controlId, LPCSTR p_str)
{
	_assertValidDlgHandle(m_hDlg);
	UT_Win32LocaleString str;
	str.fromUTF8(p_str);
	return SendDlgItemMessageW(m_hDlg, controlId, CB_ADDSTRING, 0, (LPARAM)str.c_str());
}

int XAP_Win32DialogHelper::setComboDataItem(UT_sint32 controlId, int nIndex, DWORD dwData)
{
	_assertValidDlgHandle(m_hDlg);
	return SendDlgItemMessageW(m_hDlg, controlId, CB_SETITEMDATA, nIndex, dwData);
}


int XAP_Win32DialogHelper::getComboDataItem(UT_sint32 controlId, int nIndex)
{
	_assertValidDlgHandle(m_hDlg);
	return SendDlgItemMessageW(m_hDlg, controlId, CB_GETITEMDATA, nIndex,0);
}

void XAP_Win32DialogHelper::selectComboItem(UT_sint32 controlId, int index)
{
	_assertValidDlgHandle(m_hDlg);
	SendDlgItemMessageW(m_hDlg, controlId, CB_SETCURSEL, index, 0);
}

int XAP_Win32DialogHelper::getComboSelectedIndex(UT_sint32 controlId) const
{
	_assertValidDlgHandle(m_hDlg);
	return SendDlgItemMessageW(m_hDlg, controlId, CB_GETCURSEL, 0, 0);
}

void XAP_Win32DialogHelper::resetComboContent(UT_sint32 controlId)
{
	_assertValidDlgHandle(m_hDlg);
	SendDlgItemMessageW(m_hDlg, controlId, CB_RESETCONTENT, 0, 0);
}

// List boxes

void XAP_Win32DialogHelper::resetContent(UT_sint32 controlId)
{
	_assertValidDlgHandle(m_hDlg);
	SendDlgItemMessageW(m_hDlg, controlId, LB_RESETCONTENT, 0, 0);
}


int XAP_Win32DialogHelper::addItemToList(UT_sint32 controlId, LPCSTR p_str)
{
	_assertValidDlgHandle(m_hDlg);
	UT_Win32LocaleString str;
	str.fromUTF8(p_str);
	return SendDlgItemMessageW(m_hDlg, controlId, LB_ADDSTRING, 0, (LPARAM)str.c_str());
}

int XAP_Win32DialogHelper::setListDataItem(UT_sint32 controlId, int nIndex, DWORD dwData)
{
	_assertValidDlgHandle(m_hDlg);
	return SendDlgItemMessageW(m_hDlg, controlId, LB_SETITEMDATA, nIndex, dwData);
}


int XAP_Win32DialogHelper::getListDataItem(UT_sint32 controlId, int nIndex)
{
	_assertValidDlgHandle(m_hDlg);
	return SendDlgItemMessageW(m_hDlg, controlId, LB_GETITEMDATA, nIndex,0);
}

int XAP_Win32DialogHelper::getListSelectedIndex(UT_sint32 controlId) const
{
	_assertValidDlgHandle(m_hDlg);
	return SendDlgItemMessageW(m_hDlg, controlId, LB_GETCURSEL, 0, 0);
}

void XAP_Win32DialogHelper::selectListItem(UT_sint32 controlId, int index)
{
	_assertValidDlgHandle(m_hDlg);
	SendDlgItemMessageW(m_hDlg, controlId, LB_SETCURSEL, index, 0);
}

void XAP_Win32DialogHelper::getListText(UT_sint32 controlId, int index, char *p_str) const
{
	_assertValidDlgHandle(m_hDlg);
	SendDlgItemMessageW(m_hDlg, controlId, LB_GETTEXT, index, (LPARAM)p_str);
}


// Controls
void XAP_Win32DialogHelper::setControlText(UT_sint32 controlId, LPCSTR p_str)
{
	_assertValidDlgHandle(m_hDlg);
	UT_Win32LocaleString str;
	str.fromUTF8(p_str);
	SetDlgItemTextW(m_hDlg, controlId, str.c_str());
}

void XAP_Win32DialogHelper::setControlInt(UT_sint32 controlId, int value)
{
	_assertValidDlgHandle(m_hDlg);
	SetDlgItemInt(m_hDlg, controlId, value, TRUE);
}

int XAP_Win32DialogHelper::getControlInt(UT_sint32 controlId) const
{
	_assertValidDlgHandle(m_hDlg);
	return GetDlgItemInt(m_hDlg, controlId, NULL, FALSE);
}

void XAP_Win32DialogHelper::selectControlText(UT_sint32 controlId, UT_sint32 start, UT_sint32 end)
{
	_assertValidDlgHandle(m_hDlg);
	SendDlgItemMessageW(m_hDlg, controlId, EM_SETSEL, start,end);
}

int XAP_Win32DialogHelper::isChecked(UT_sint32 controlId) const
{
	_assertValidDlgHandle(m_hDlg);
	return IsDlgButtonChecked(m_hDlg, controlId);
}

void XAP_Win32DialogHelper::getControlText(	UT_sint32 controlId,
											LPSTR p_buffer,
											UT_sint32 Buffer_length) const
{
	LPWSTR buf;
	UINT   len;
	UT_Win32LocaleString str;

	_assertValidDlgHandle(m_hDlg);
	len=GetWindowTextLengthW(GetDlgItem(m_hDlg, controlId));
	buf=g_new(WCHAR,len+1);
	GetDlgItemTextW(m_hDlg, controlId, buf, len+1);
	str.fromLocale(buf);
	FREEP(buf);
	// FIXME: validation
	strncpy(p_buffer,str.utf8_str().utf8_str(),Buffer_length);
}

bool XAP_Win32DialogHelper::isControlVisible(UT_sint32 controlId) const
{
	_assertValidDlgHandle(m_hDlg);
	HWND hControl = GetDlgItem(m_hDlg, controlId);
	if (hControl) {
		return (GetWindowLongPtrW(m_hDlg, GWL_STYLE) & WS_VISIBLE) ?
				true : false;
	}
	return false;
}

bool XAP_Win32DialogHelper::isParentFrame(/*const*/ XAP_Frame& frame) const
{
	_assertValidDlgHandle(m_hDlg);
	XAP_FrameImpl *pFrameImpl = frame.getFrameImpl();
	return ((HWND)GetWindowLongPtrW(m_hDlg, GWLP_HWNDPARENT) ==
		static_cast<XAP_Win32FrameImpl *>(pFrameImpl)->getTopLevelWindow()) ? true : false;
}

void XAP_Win32DialogHelper::setParentFrame(const XAP_Frame* pFrame)
{
	_assertValidDlgHandle(m_hDlg);
	SetWindowLongPtrW(m_hDlg, GWLP_HWNDPARENT, (LONG_PTR)(pFrame ? static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow() : NULL));
	SetWindowPos(m_hDlg, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
}


XAP_Frame* XAP_Win32DialogHelper::getParentFrame()
{
	_assertValidDlgHandle(m_hDlg);
	return reinterpret_cast<XAP_Frame*>(
			GetWindowLongPtrW(m_hDlg, GWLP_HWNDPARENT));
}

void XAP_Win32DialogHelper::centerDialog()
{
	_assertValidDlgHandle(m_hDlg);
	
	s_centerDialog(m_hDlg);    
} 	

void XAP_Win32DialogHelper::s_centerDialog(HWND hWnd)
{
	RECT 	rc, rcParent;
	int 	nWidth, nHeight;
	POINT 	pt;
	
    GetWindowRect(hWnd, &rc);
    
   	if (!GetParent(hWnd))
	  GetWindowRect (GetDesktopWindow(), &rcParent);
	else
	  GetClientRect (GetParent(hWnd), &rcParent);	  
	  
	nWidth = rc.right - rc.left;
	nHeight = rc.bottom - rc.top;
	
	pt.x = (rcParent.right - rcParent.left) / 2;
	pt.y = (rcParent.bottom - rcParent.top) / 2;
	
	if (!GetParent(hWnd))
	  ClientToScreen (GetDesktopWindow(), &pt);
	else
	  ClientToScreen (GetParent(hWnd), &pt);

	pt.x -= nWidth / 2;
	pt.y -= nHeight / 2;

	// Move your arse...
	MoveWindow (hWnd, pt.x, pt.y, nWidth, nHeight, TRUE);
}

HBITMAP XAP_Win32DialogHelper::s_loadBitmap(HWND hWnd, UINT nId,
					     const char* pName,
					     int width, int height,
					     const UT_RGBColor & color)
{
	HBITMAP hBitmap = NULL;

	XAP_Win32Toolbar_Icons::getBitmapForIcon(hWnd, width,height, &color,	pName,	&hBitmap);
	SendDlgItemMessageW(hWnd,  nId,  BM_SETIMAGE,  IMAGE_BITMAP, (LPARAM) hBitmap);
	return hBitmap;
}
