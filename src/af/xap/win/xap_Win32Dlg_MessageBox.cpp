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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include <windows.h>
#include "ut_assert.h"
#include "xap_Win32Dlg_MessageBox.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"
#include "xap_EncodingManager.h"
#include "ut_Win32LocaleString.h"

/*****************************************************************/
XAP_Dialog * XAP_Win32Dialog_MessageBox::static_constructor(XAP_DialogFactory * pFactory,
														  XAP_Dialog_Id id)
{
	XAP_Win32Dialog_MessageBox * p = new XAP_Win32Dialog_MessageBox(pFactory,id);
	return p;
}

XAP_Win32Dialog_MessageBox::XAP_Win32Dialog_MessageBox(XAP_DialogFactory * pDlgFactory,
													 XAP_Dialog_Id id)
	: XAP_Dialog_MessageBox(pDlgFactory,id)
{
}

XAP_Win32Dialog_MessageBox::~XAP_Win32Dialog_MessageBox(void)
{
}

/*****************************************************************/

void XAP_Win32Dialog_MessageBox::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);
	XAP_Win32App * pApp = static_cast<XAP_Win32App *>(XAP_App::getApp());
	UT_return_if_fail(pApp);

	UT_Win32LocaleString caption, message;
	caption.fromASCII (pApp->getApplicationTitleForTitleBar());

	HWND hwnd = static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow();
	UINT flags;

	switch (m_buttons)
	{
	case b_O:
		flags = MB_ICONASTERISK | MB_OK;
		flags |= ((m_defaultAnswer == a_OK) ? MB_DEFBUTTON1 : 0);
		break;

	case b_OC:
		flags = MB_ICONQUESTION | MB_OKCANCEL;
		flags |= ((m_defaultAnswer == a_OK) ? MB_DEFBUTTON1 : 0);
		flags |= ((m_defaultAnswer == a_CANCEL) ? MB_DEFBUTTON2 : 0);
		break;

	case b_YN:
		flags = MB_ICONQUESTION | MB_YESNO;
		flags |= ((m_defaultAnswer == a_YES) ? MB_DEFBUTTON1 : 0);
		flags |= ((m_defaultAnswer == a_NO) ? MB_DEFBUTTON2 : 0);
		break;

	case b_YNC:
		flags = MB_ICONQUESTION | MB_YESNOCANCEL;
		flags |= ((m_defaultAnswer == a_YES) ? MB_DEFBUTTON1 : 0);
		flags |= ((m_defaultAnswer == a_NO) ? MB_DEFBUTTON2 : 0);
		flags |= ((m_defaultAnswer == a_CANCEL) ? MB_DEFBUTTON3 : 0);
		break;

	default:
		flags = 0;
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	}

	message.fromUTF8(m_szMessage);
	int res = MessageBoxW(hwnd, message.c_str(), caption.c_str(), flags);

	switch (res)
	{
	case IDCANCEL:
		m_answer = a_CANCEL;
		break;

	case IDNO:
		m_answer = a_NO;
		break;

	case IDOK:
		m_answer = a_OK;
		break;

	case IDYES:
		m_answer = a_YES;
		break;

	case IDABORT:
	case IDIGNORE:
	case IDRETRY:
	default:
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	}

	// the caller can get the answer from getAnswer().
}

