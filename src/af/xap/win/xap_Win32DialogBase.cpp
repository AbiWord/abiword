// xap_Win32DialogBase.cpp

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

#include "xap_Dialog.h"
#include "xap_Strings.h"
#include "xap_Win32DialogBase.h"
#include "xap_Win32FrameImpl.h"
#include "ut_Win32LocaleString.h"

void XAP_Win32DialogBase::createModal(XAP_Frame* pFrame, LPCWSTR dlgTemplate)
{
	UT_ASSERT(m_tag == magic_tag);

	XAP_App* pApp = XAP_App::getApp();
	UT_ASSERT(pApp);

	XAP_Win32App* pWin32App = static_cast<XAP_Win32App*>(pApp);

	XAP_FrameImpl* pFrameImpl = pFrame->getFrameImpl();

	UT_ASSERT(pFrame);

	XAP_Win32FrameImpl* pWin32FrameImpl = static_cast<XAP_Win32FrameImpl*>(pFrameImpl);

	HWND hFrameWnd = pWin32FrameImpl->getTopLevelWindow();

	// raise the dialog
	UT_DebugOnly<int> result = DialogBoxParamW(pWin32App->getInstance(),
						dlgTemplate,
						hFrameWnd,
						(DLGPROC)&XAP_Win32DialogBase::s_dlgProc,
						(LPARAM)this);
	// since we can't really know what to do in case of failure, we really
	// *should* return the error-code from this function I think.
	UT_ASSERT(result != -1);
}


HWND XAP_Win32DialogBase::createModeless(XAP_Frame* pFrame, LPCWSTR dlgTemplate)
{
	UT_ASSERT(m_tag == magic_tag);
   	UT_return_val_if_fail(pFrame, NULL);

	XAP_App* pApp = XAP_App::getApp();
	UT_return_val_if_fail(pApp, NULL);

	XAP_Win32App* pWin32App = static_cast<XAP_Win32App*>(pApp);

	XAP_FrameImpl* pFrameImpl = pFrame->getFrameImpl();

	XAP_Win32FrameImpl* pWin32FrameImpl = static_cast<XAP_Win32FrameImpl*>(pFrameImpl);

	HWND hFrameWnd = pWin32FrameImpl->getTopLevelWindow();

	HWND hWnd = CreateDialogParamW(pWin32App->getInstance(),
							dlgTemplate,
							hFrameWnd,
							(DLGPROC)&XAP_Win32DialogBase::s_dlgProc,
							(LPARAM)this);
	UT_return_val_if_fail(hWnd, NULL);

    m_hDlg = hWnd;
	showWindow(SW_SHOW);
	bringWindowToTop();

	return hWnd;
}


BOOL CALLBACK XAP_Win32DialogBase::s_dlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	XAP_Win32DialogBase* pThis = NULL;

	if (msg == WM_INITDIALOG)
	{
		pThis = (XAP_Win32DialogBase*)lParam;
        SetWindowLongW(hWnd, DWLP_USER, lParam);
		pThis->m_hDlg = hWnd;
		return pThis->_onInitDialog(hWnd, wParam, lParam);
	}

    pThis = (XAP_Win32DialogBase *) GetWindowLongW(hWnd,DWLP_USER);
	
	if (pThis == NULL)
		return FALSE;
	UT_ASSERT(pThis->m_tag == magic_tag);


	switch (msg)
	{

	case WM_COMMAND:
		return pThis->_onCommand(hWnd, wParam, lParam);

	case WM_NOTIFY:
		switch (((LPNMHDR)lParam)->code)
		{
			case UDN_DELTAPOS:		
				return pThis->_onDeltaPos((NM_UPDOWN *)lParam);
				break;
				
			default:				
				break;
		}
		break;

	case WM_HELP:
		pThis->_callHelp();
		break;

	case WM_KEYDOWN:
		if (wParam == VK_F1)
		{
			// display help
			return TRUE;
		}
	}

	return pThis->_onDlgMessage(hWnd, msg, wParam, lParam);
}

BOOL XAP_Win32DialogBase::_onDlgMessage(HWND /*hWnd*/, UINT /*msg*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	return FALSE;
}

void XAP_Win32DialogBase::checkButton(UT_sint32 controlId, bool bChecked)
{
	UT_return_if_fail(IsWindow(m_hDlg));
	CheckDlgButton(m_hDlg, controlId, bChecked ? BST_CHECKED : BST_UNCHECKED);
}

void XAP_Win32DialogBase::enableControl(UT_sint32 controlId, bool bChecked)
{
	UT_return_if_fail(IsWindow(m_hDlg));
	EnableWindow(GetDlgItem(m_hDlg, controlId), bChecked ? TRUE : FALSE);
}

void XAP_Win32DialogBase::destroyWindow()
{
	UT_return_if_fail(IsWindow(m_hDlg));
	UT_DebugOnly<int> iResult = DestroyWindow(m_hDlg);
	UT_ASSERT_HARMLESS((iResult != 0));
}

void XAP_Win32DialogBase::setDialogTitle(const char* uft8_str)
{
	UT_return_if_fail(IsWindow(m_hDlg));
	UT_Win32LocaleString str;
	str.fromUTF8 (uft8_str);
	SetWindowTextW (m_hDlg, str.c_str());
}

int XAP_Win32DialogBase::showWindow(int Mode )
{
	UT_return_val_if_fail(IsWindow(m_hDlg), 0);
	return ShowWindow(m_hDlg, Mode);
}

int XAP_Win32DialogBase::showControl(UT_sint32 controlId, int Mode)
{
	UT_return_val_if_fail(IsWindow(m_hDlg), 0);
	return ShowWindow(GetDlgItem(m_hDlg, controlId), Mode);
}

int XAP_Win32DialogBase::bringWindowToTop()                     
{
	UT_return_val_if_fail(IsWindow(m_hDlg), 0);
    return BringWindowToTop(m_hDlg);
}

void XAP_Win32DialogBase::notifyCloseFrame(XAP_Frame *pFrame)
{
	if((HWND)GetWindowLongPtrW(m_hDlg, GWLP_HWNDPARENT) == static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow())
	{
		SetWindowLongPtrW(m_hDlg, GWLP_HWNDPARENT, 0);
		SetWindowPos(m_hDlg, NULL, 0, 0, 0, 0,
						SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}
}

// Combo boxes.

int XAP_Win32DialogBase::addItemToCombo(UT_sint32 controlId, LPCSTR p_str)
{
	UT_return_val_if_fail(IsWindow(m_hDlg), CB_ERR);
    UT_Win32LocaleString str;
	str.fromUTF8 (p_str); 	
	return SendDlgItemMessageW(m_hDlg, controlId, CB_ADDSTRING, 0, (LPARAM)str.c_str());	
}

int XAP_Win32DialogBase::setComboDataItem(UT_sint32 controlId, int nIndex, DWORD dwData)
{
	UT_return_val_if_fail(IsWindow(m_hDlg), CB_ERR);
	return SendDlgItemMessageW(m_hDlg, controlId, CB_SETITEMDATA, nIndex, dwData);
}


int XAP_Win32DialogBase::getComboDataItem(UT_sint32 controlId, int nIndex)
{
	UT_return_val_if_fail(IsWindow(m_hDlg), CB_ERR);
	return SendDlgItemMessageW(m_hDlg, controlId, CB_GETITEMDATA, nIndex,0);
}

int XAP_Win32DialogBase::getComboItemIndex(UT_sint32 controlId, LPCSTR p_str)
{
	UT_return_val_if_fail(IsWindow(m_hDlg), CB_ERR);
	return SendDlgItemMessageW(m_hDlg, controlId, CB_FINDSTRINGEXACT, -1, (LPARAM) p_str);
}

void XAP_Win32DialogBase::selectComboItem(UT_sint32 controlId, int index)
{
	UT_return_if_fail(IsWindow(m_hDlg));
	SendDlgItemMessageW(m_hDlg, controlId, CB_SETCURSEL, index, 0);
}

int XAP_Win32DialogBase::getComboSelectedIndex(UT_sint32 controlId) const
{
	UT_return_val_if_fail(IsWindow(m_hDlg), CB_ERR);
	return SendDlgItemMessageW(m_hDlg, controlId, CB_GETCURSEL, 0, 0);
}

void XAP_Win32DialogBase::resetComboContent(UT_sint32 controlId)
{
	UT_return_if_fail(IsWindow(m_hDlg));
	SendDlgItemMessageW(m_hDlg, controlId, CB_RESETCONTENT, 0, 0);
}

void XAP_Win32DialogBase::getComboTextItem(UT_sint32 controlId, int index, UT_Win32LocaleString& str)
{
	UT_return_if_fail(IsWindow(m_hDlg));
	wchar_t szBuff[1024];

	if (SendDlgItemMessageW(m_hDlg, controlId, CB_GETLBTEXT, index, (LPARAM)szBuff) != CB_ERR)
		str.fromLocale(szBuff);
	else
		str.clear();
}

// List boxes

void XAP_Win32DialogBase::resetContent(UT_sint32 controlId)
{
	UT_return_if_fail(IsWindow(m_hDlg));
	SendDlgItemMessageW(m_hDlg, controlId, LB_RESETCONTENT, 0, 0);
}


int XAP_Win32DialogBase::addItemToList(UT_sint32 controlId, LPCSTR p_str)
{
	UT_return_val_if_fail(IsWindow(m_hDlg), LB_ERR);
	UT_Win32LocaleString str;
	str.fromUTF8(p_str);
	return SendDlgItemMessageW(m_hDlg, controlId, LB_ADDSTRING, 0, (LPARAM)str.c_str());
}

int XAP_Win32DialogBase::setListDataItem(UT_sint32 controlId, int nIndex, DWORD dwData)
{
	UT_return_val_if_fail(IsWindow(m_hDlg), LB_ERR);
	return SendDlgItemMessageW(m_hDlg, controlId, LB_SETITEMDATA, nIndex, dwData);
}


int XAP_Win32DialogBase::getListDataItem(UT_sint32 controlId, int nIndex)
{
	UT_return_val_if_fail(IsWindow(m_hDlg), LB_ERR);
	return SendDlgItemMessageW(m_hDlg, controlId, LB_GETITEMDATA, nIndex,0);
}

int XAP_Win32DialogBase::getListSelectedIndex(UT_sint32 controlId) const
{
	UT_return_val_if_fail(IsWindow(m_hDlg), LB_ERR);
	return SendDlgItemMessageW(m_hDlg, controlId, LB_GETCURSEL, 0, 0);
}

void XAP_Win32DialogBase::selectListItem(UT_sint32 controlId, int index)
{
	UT_return_if_fail(IsWindow(m_hDlg));
	SendDlgItemMessageW(m_hDlg, controlId, LB_SETCURSEL, index, 0);
}

void XAP_Win32DialogBase::getListText(UT_sint32 controlId, int index, char *p_str) const
{
	UT_return_if_fail(IsWindow(m_hDlg));
	SendDlgItemMessageW(m_hDlg, controlId, LB_GETTEXT, index, (LPARAM)p_str);
}


// Controls
void XAP_Win32DialogBase::setControlText(UT_sint32 controlId, LPCSTR p_str)
{
	UT_return_if_fail(IsWindow(m_hDlg));
	//UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
	UT_Win32LocaleString str;
	str.fromUTF8(p_str);
	SetDlgItemTextW(m_hDlg, controlId, str.c_str());
}

bool XAP_Win32DialogBase::setDlgItemText(int nIDDlgItem,  const char* uft8_str)
{
	UT_return_val_if_fail(IsWindow(m_hDlg), false);
	
	UT_Win32LocaleString str;
	str.fromUTF8 (uft8_str);
	return (bool) SetDlgItemTextW (m_hDlg, nIDDlgItem, str.c_str());
}

bool XAP_Win32DialogBase::getDlgItemText(int nIDDlgItem, UT_Win32LocaleString& str)
{	
	wchar_t szBuff [1024];	
	bool rslt;
	
	rslt = (bool) GetDlgItemTextW(m_hDlg, nIDDlgItem, szBuff, 1024);
	
	if (rslt == true)
		str.fromLocale(szBuff);
	else
		str.clear();
	
	return rslt;
 }

void XAP_Win32DialogBase::setControlInt(UT_sint32 controlId, int value)
{
	UT_return_if_fail(IsWindow(m_hDlg));
	SetDlgItemInt(m_hDlg, controlId, value, TRUE);
}

int XAP_Win32DialogBase::getControlInt(UT_sint32 controlId) const
{
	UT_return_val_if_fail(IsWindow(m_hDlg), 0);
	return GetDlgItemInt(m_hDlg, controlId, NULL, FALSE);
}

void XAP_Win32DialogBase::selectControlText(UT_sint32 controlId, UT_sint32 start, UT_sint32 end)
{
	UT_return_if_fail(IsWindow(m_hDlg));
	SendDlgItemMessageW(m_hDlg, controlId, EM_SETSEL, start,end);
}

int XAP_Win32DialogBase::isChecked(UT_sint32 controlId) const
{
	UT_return_val_if_fail(IsWindow(m_hDlg), 0);
	return IsDlgButtonChecked(m_hDlg, controlId);
}

bool XAP_Win32DialogBase::isDialogValid() const
{
	return IsWindow(m_hDlg)!=0;
}

void XAP_Win32DialogBase::getControlText(UT_sint32 controlId,
											LPSTR p_buffer,
											UT_sint32 Buffer_length) const
{
	UT_return_if_fail(IsWindow(m_hDlg));
	UT_Win32LocaleString str;
	LPWSTR wbuf = (LPWSTR) UT_calloc(Buffer_length+1,2);
	GetDlgItemTextW(m_hDlg, controlId, wbuf, Buffer_length);
	str.fromLocale(wbuf);
	strncpy(p_buffer,str.utf8_str().utf8_str(),Buffer_length);
	FREEP(wbuf);
}

bool XAP_Win32DialogBase::isControlVisible(UT_sint32 controlId) const
{
	UT_return_val_if_fail(IsWindow(m_hDlg), false);

	HWND hControl = GetDlgItem(m_hDlg, controlId);
	if (hControl) {
		return (GetWindowLongPtrW(m_hDlg, GWL_STYLE) & WS_VISIBLE) ?
				true : false;
	}
	return false;
}

void XAP_Win32DialogBase::centerDialog()
{
	UT_return_if_fail(IsWindow(m_hDlg));
	
	RECT 	rc, rcParent;
	int 	nWidth, nHeight;
	POINT 	pt;
	
    GetWindowRect(m_hDlg, &rc);
    
   	if (!GetParent(m_hDlg))
	  GetWindowRect (GetDesktopWindow(), &rcParent);
	else
	  GetClientRect (GetParent(m_hDlg), &rcParent);	  
	  
	nWidth = rc.right - rc.left;
	nHeight = rc.bottom - rc.top;
	
	pt.x = (rcParent.right - rcParent.left) / 2;
	pt.y = (rcParent.bottom - rcParent.top) / 2;
	
	if (!GetParent(m_hDlg))
	  ClientToScreen (GetDesktopWindow(), &pt);
	else
	  ClientToScreen (GetParent(m_hDlg), &pt);

	pt.x -= nWidth / 2;
	pt.y -= nHeight / 2;

	MoveWindow (m_hDlg, pt.x, pt.y, nWidth, nHeight, TRUE);
}


void XAP_Win32DialogBase::localizeDialogTitle(UT_uint32 stringId)
{
	if (!m_pSS)
	{
		m_pSS = m_pDlg->getApp()->getStringSet();
	}
	setDialogTitle( m_pSS->getValue(stringId) );
}

void XAP_Win32DialogBase::localizeControlText(UT_sint32 controlId, UT_uint32 stringId)
{
	if (!m_pSS)
	{
		m_pSS = m_pDlg->getApp()->getStringSet();
	}
	setControlText( controlId, m_pSS->getValue(stringId) );
}

extern bool helpLocalizeAndOpenURL(const char* pathBeforeLang, const char* pathAfterLang, const char *remoteURLbase);

BOOL XAP_Win32DialogBase::_callHelp()
{
	if ( m_pDlg->getHelpUrl().size () > 0 )
    {
		helpLocalizeAndOpenURL ("help", m_pDlg->getHelpUrl().c_str(), "http://www.abisource.com/help/" );
    }
	else
    {
		// TODO: warn no help on this topic
//		UT_DEBUGMSG(("NO HELP FOR THIS TOPIC!!\n"));
    }

	return TRUE;
}

bool XAP_Win32DialogBase::setWindowText (HWND hWnd, const char* uft8_str)
{
	UT_return_val_if_fail(IsWindow(hWnd), false);
	
	UT_Win32LocaleString str;
	str.fromUTF8 (uft8_str);
	return (bool) SetWindowTextW (hWnd, str.c_str());
}

bool XAP_Win32DialogBase::getDlgItemText(HWND hWnd, int nIDDlgItem, UT_Win32LocaleString& str)
{	
	UT_return_val_if_fail(IsWindow(hWnd), false);
	
	wchar_t szBuff [1024];	
	bool rslt;
	
	rslt = (bool) GetDlgItemTextW(hWnd, nIDDlgItem, szBuff, 1024);
	
	if (rslt == true)
		str.fromLocale(szBuff);
	else
		str.clear();
	
	return rslt;
}

bool XAP_Win32DialogBase::setDlgItemText(HWND hWnd, int nIDDlgItem,  const char* uft8_str)
{
	UT_return_val_if_fail(IsWindow(hWnd), false);
	
	UT_Win32LocaleString str;
	str.fromUTF8 (uft8_str);
	return (bool) SetDlgItemTextW (hWnd, nIDDlgItem, str.c_str());
}

