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

#ifdef _MSC_VER	 // MSVC++ warns about using 'this' in initializer list.
#pragma warning(disable: 4355)
#endif

#include <windows.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"

#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_Password.h"
#include "xap_Win32Dlg_Password.h"

#include "xap_Win32Resources.rc2"

/*****************************************************************/

XAP_Dialog * XAP_Win32Dialog_Password::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	XAP_Win32Dialog_Password * p = new XAP_Win32Dialog_Password(pFactory,id);
	return p;
}

XAP_Win32Dialog_Password::XAP_Win32Dialog_Password(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: XAP_Dialog_Password(pDlgFactory,id), 
      _win32Dialog(this),
      m_hThisDlg(NULL)
{
}

XAP_Win32Dialog_Password::~XAP_Win32Dialog_Password(void)
{
}

void XAP_Win32Dialog_Password::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	_win32Dialog.runModal( pFrame,
						   XAP_DIALOG_ID_PASSWORD,
                           XAP_RID_DIALOG_PASSWORD,
	         		       this );
}

#define _DSX(c,s)	SetDlgItemText(hWnd,XAP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL XAP_Win32Dialog_Password::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	XAP_Win32App * app = static_cast<XAP_Win32App *> (m_pApp);
	UT_ASSERT(app);

	m_hThisDlg = hWnd;
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	// localize dialog title
	_win32Dialog.setDialogTitle( pSS->getValue(XAP_STRING_ID_DLG_Password_Title) );

	// localize controls
	_DSX(PASSWORD_BTN_OK,				DLG_OK);
	_DSX(PASSWORD_BTN_CANCEL,			DLG_Cancel);
	_DSX(PASSWORD_LBL_PASSWORD,			DLG_Password_Password);

	return 1;
}

BOOL XAP_Win32Dialog_Password::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;

	switch (wId)
	{
	case XAP_RID_DIALOG_PASSWORD_BTN_CANCEL:
		setAnswer( a_Cancel );
		EndDialog(hWnd,0);
		return 1;

	case XAP_RID_DIALOG_PASSWORD_BTN_OK:
		{
			char buf[1024];
			_win32Dialog.getControlText(XAP_RID_DIALOG_PASSWORD_EBX_PASSWORD, buf, 1024);
			setPassword( buf );
		}
		setAnswer( a_OK );
		EndDialog(hWnd,0);
		return 1;

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}

BOOL XAP_Win32Dialog_Password::_onDeltaPos(NM_UPDOWN * pnmud)
{
	return 1;
}
