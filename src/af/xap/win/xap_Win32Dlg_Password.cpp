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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
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
	: XAP_Dialog_Password(pDlgFactory,id)
{
}

XAP_Win32Dialog_Password::~XAP_Win32Dialog_Password(void)
{
}

void XAP_Win32Dialog_Password::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	UT_ASSERT(m_id == XAP_DIALOG_ID_PASSWORD);
	
	setDialog(this);
	createModal(pFrame, MAKEINTRESOURCEW(XAP_RID_DIALOG_PASSWORD));
}

#define _DSX(c,s)	setDlgItemText(XAP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL XAP_Win32Dialog_Password::_onInitDialog(HWND /*hWnd*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	// localize dialog title
	localizeDialogTitle(XAP_STRING_ID_DLG_Password_Title);

	// localize controls
	localizeControlText(XAP_RID_DIALOG_PASSWORD_BTN_OK,		XAP_STRING_ID_DLG_OK);
	localizeControlText(XAP_RID_DIALOG_PASSWORD_BTN_CANCEL,	XAP_STRING_ID_DLG_Cancel);
	localizeControlText(XAP_RID_DIALOG_PASSWORD_LBL_PASSWORD,	XAP_STRING_ID_DLG_Password_Password);

	return 1;
}

BOOL XAP_Win32Dialog_Password::_onCommand(HWND hWnd, WPARAM wParam, LPARAM /*lParam*/)
{
	WORD wId = LOWORD(wParam);

	switch (wId)
	{
	case XAP_RID_DIALOG_PASSWORD_BTN_CANCEL:
		setAnswer( a_Cancel );
		EndDialog(hWnd,0);
		return 1;

	case XAP_RID_DIALOG_PASSWORD_BTN_OK:
		{
			char buf[1024];
			getControlText(XAP_RID_DIALOG_PASSWORD_EBX_PASSWORD, buf, 1024);
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

BOOL XAP_Win32Dialog_Password::_onDeltaPos(NM_UPDOWN * /*pnmud*/)
{
	return 1;
}
