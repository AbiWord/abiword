/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"

#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_WindowMore.h"
#include "xap_Win32Dlg_WindowMore.h"

#include "xap_Win32Resources.rc2"

/*****************************************************************/
XAP_Dialog * XAP_Win32Dialog_WindowMore::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id id)
{
	XAP_Win32Dialog_WindowMore * p = new XAP_Win32Dialog_WindowMore(pFactory,id);
	return p;
}

XAP_Win32Dialog_WindowMore::XAP_Win32Dialog_WindowMore(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id id)
	: XAP_Dialog_WindowMore(pDlgFactory,id)
{
}

XAP_Win32Dialog_WindowMore::~XAP_Win32Dialog_WindowMore(void)
{
}

void XAP_Win32Dialog_WindowMore::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	UT_ASSERT(m_id == XAP_DIALOG_ID_WINDOWMORE);
	
	// NOTE: this work could be done in XP code
	m_ndxSelFrame = m_pApp->findFrame(pFrame);
	UT_ASSERT(m_ndxSelFrame >= 0);

	// raise the dialog
	setDialog(this);
	createModal(pFrame, MAKEINTRESOURCEW(XAP_RID_DIALOG_WINDOWMORE));

}

BOOL XAP_Win32Dialog_WindowMore::_onInitDialog(HWND /*hWnd*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	// localize controls
	localizeDialogTitle(XAP_STRING_ID_DLG_MW_MoreWindows);

	localizeControlText(XAP_RID_DIALOG_WINDOWMORE_TEXT_ACTIVATE,	XAP_STRING_ID_DLG_MW_Activate);
	localizeControlText(XAP_RID_DIALOG_WINDOWMORE_BTN_OK,			XAP_STRING_ID_DLG_OK);
	localizeControlText(XAP_RID_DIALOG_WINDOWMORE_BTN_CANCEL,		XAP_STRING_ID_DLG_Cancel);

	// load each frame name into the list
	for (UT_sint32 i=0; i<m_pApp->getFrameCount(); i++)
	{
		XAP_Frame * f = m_pApp->getFrame(i);
		UT_continue_if_fail(f);

		int nIndex = addItemToList(XAP_RID_DIALOG_WINDOWMORE_LIST, f->getTitle().c_str());
		setListDataItem(XAP_RID_DIALOG_WINDOWMORE_LIST, nIndex, (DWORD) i);
     } 

	// select the one we're in
	selectListItem(XAP_RID_DIALOG_WINDOWMORE_LIST, m_ndxSelFrame);

	return 1;							// 1 == we did not call SetFocus()
}

BOOL XAP_Win32Dialog_WindowMore::_onCommand(HWND hWnd, WPARAM wParam, LPARAM /*lParam*/)
{
	WORD wId = LOWORD(wParam);
	int nItem;

	switch (wId)
	{
	case XAP_RID_DIALOG_WINDOWMORE_LIST:
		switch (HIWORD(wParam))
		{
			case LBN_SELCHANGE:
				// NOTE: we could get away with only grabbing this in IDOK case
				nItem = getListSelectedIndex(wId);
				m_ndxSelFrame = getListDataItem(wId, nItem);
				return 1;

			case LBN_DBLCLK:
				nItem = getListSelectedIndex(wId);
				m_ndxSelFrame = getListDataItem(wId, nItem);
				EndDialog(hWnd,0);
				return 1;

			default:
				return 0;
		}
		break;

	case IDCANCEL:						// also XAP_RID_DIALOG_WINDOWMORE_BTN_CANCEL
		m_answer = a_CANCEL;
		// fall through

	case IDOK:							// also XAP_RID_DIALOG_WINDOWMORE_BTN_OK
		EndDialog(hWnd,0);
		return 1;

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}

BOOL XAP_Win32Dialog_WindowMore::_onDeltaPos(NM_UPDOWN * /*pnmud*/)
{
	return FALSE;
}
