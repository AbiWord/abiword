/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
 * Copyright (C) 2002 Tomas Frydrych <tomas@frydrych.uklinux.net>
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
#include <commctrl.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"

#include "xap_Dialog_Id.h"
#include "xap_Dlg_DocComparison.h"
#include "xap_Win32Dlg_DocComparison.h"
#include "xap_Win32LabelledSeparator.h"

#ifdef __MINGW32__
#define LVM_GETSELECTIONMARK    (LVM_FIRST+66)
#define ListView_GetSelectionMark(w) (INT)SNDMSG((w),LVM_GETSELECTIONMARK,0,0)
#endif


#include "xap_Win32Resources.rc2"
#include "xap_Win32DialogHelper.h"

/*****************************************************************/

XAP_Dialog * XAP_Win32Dialog_DocComparison::static_constructor(XAP_DialogFactory * pFactory,
														 XAP_Dialog_Id id)
{
	XAP_Win32Dialog_DocComparison * p = new XAP_Win32Dialog_DocComparison(pFactory,id);
	return p;
}

XAP_Win32Dialog_DocComparison::XAP_Win32Dialog_DocComparison(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: XAP_Dialog_DocComparison(pDlgFactory,id)
{
}

XAP_Win32Dialog_DocComparison::~XAP_Win32Dialog_DocComparison(void)
{
}

void XAP_Win32Dialog_DocComparison::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);
	// raise the dialog
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);

	XAP_Win32LabelledSeparator_RegisterClass(pWin32App);

	LPCTSTR lpTemplate = NULL;

	UT_ASSERT(m_id == XAP_DIALOG_ID_DOCCOMPARISON);

	lpTemplate = MAKEINTRESOURCE(XAP_RID_DIALOG_DOCCOMPARISON);

	int result = DialogBoxParam(pWin32App->getInstance(),lpTemplate,
						static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow(),
						(DLGPROC)s_dlgProc,(LPARAM)this);
	UT_ASSERT_HARMLESS((result != -1));
	if(result == -1)
	{
		UT_DEBUGMSG(( "XAP_Win32Dlg_DocComparison::runModal error %d\n", GetLastError() ));
	}
}

BOOL CALLBACK XAP_Win32Dialog_DocComparison::s_dlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// This is a static function.

	XAP_Win32Dialog_DocComparison * pThis;

	switch (msg)
	{
	case WM_INITDIALOG:
		pThis = (XAP_Win32Dialog_DocComparison *)lParam;
		SetWindowLong(hWnd,DWL_USER,lParam);
		return pThis->_onInitDialog(hWnd,wParam,lParam);

	case WM_COMMAND:
		pThis = (XAP_Win32Dialog_DocComparison *)GetWindowLong(hWnd,DWL_USER);
		return pThis->_onCommand(hWnd,wParam,lParam);

	default:
		return 0;
	}
}

BOOL XAP_Win32Dialog_DocComparison::_onInitDialog(HWND hWnd, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	// set the window title
	SetWindowText(hWnd, getWindowLabel());
	
	// localize buttons
	SetDlgItemText(hWnd,XAP_RID_DIALOG_DOCCOMPARISON_BTN_OK,getButtonLabel());

	// set frame titles
	SetDlgItemText(hWnd, XAP_RID_DIALOG_DOCCOMPARISON_FRAME1,getFrame1Label());
	SetDlgItemText(hWnd, XAP_RID_DIALOG_DOCCOMPARISON_FRAME2,getFrame2Label());

	// fill frame 1
	char * p = getPath1();
	SetDlgItemText(hWnd,XAP_RID_DIALOG_DOCCOMPARISON_PATH1,p);
	FREEP(p);

	p = getPath2();
	SetDlgItemText(hWnd,XAP_RID_DIALOG_DOCCOMPARISON_PATH2,p);
	FREEP(p);
	
	// fill frame 2
	UT_uint32 i;
	UT_uint32 k1 = XAP_RID_DIALOG_DOCCOMPARISON_LABEL_RELATIONSHIP;
	UT_uint32 k2 = XAP_RID_DIALOG_DOCCOMPARISON_RELATIONSHIP;
								  
	for(i = 0; i < getResultCount(); i++)
	{
		SetDlgItemText(hWnd,k1 + i,getResultLabel(i));
		
		char * t = getResultValue(i);
		SetDlgItemText(hWnd,k2 + i, t);
		FREEP(t);
	}

	XAP_Win32DialogHelper::s_centerDialog(hWnd);	

	return 1;							// 1 == we did not call SetFocus()
}

BOOL XAP_Win32Dialog_DocComparison::_onCommand(HWND hWnd, WPARAM wParam, LPARAM /*lParam*/)
{
	WORD wId = LOWORD(wParam);

	switch (wId)
	{
		case IDOK:
		case IDCANCEL:
			EndDialog(hWnd,0);
			return 1;

		default:							// we did not handle this notification
			UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
			return 0;						// return zero to let windows take care of it.
	}
}


