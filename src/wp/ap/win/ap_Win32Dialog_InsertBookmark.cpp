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
#include "xap_Win32Frame.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_InsertBookmark.h"
#include "ap_Win32Dialog_InsertBookmark.h"
#include "xap_Win32DialogHelper.h"
#include "ap_Win32Resources.rc2"

/*****************************************************************/

XAP_Dialog * AP_Win32Dialog_InsertBookmark::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_Win32Dialog_InsertBookmark * p = new AP_Win32Dialog_InsertBookmark(pFactory,id);
	return p;
}

AP_Win32Dialog_InsertBookmark::AP_Win32Dialog_InsertBookmark(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_InsertBookmark(pDlgFactory,id),
	_win32Dialog(this),
	m_hThisDlg(NULL)
{
}

AP_Win32Dialog_InsertBookmark::~AP_Win32Dialog_InsertBookmark(void)
{
}

void AP_Win32Dialog_InsertBookmark::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	_win32Dialog.runModal( pFrame, 
						   AP_DIALOG_ID_INSERTBOOKMARK, 
						   AP_RID_DIALOG_INSERTBOOKMARK, 
						   this);


}

#define _DS(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))


BOOL AP_Win32Dialog_InsertBookmark::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	m_hThisDlg = hWnd;

	XAP_Win32App * app = static_cast<XAP_Win32App *> (m_pApp);
	UT_ASSERT(app);

	const XAP_StringSet * pSS = m_pApp->getStringSet();

	// localize dialog title
	_win32Dialog.setDialogTitle( pSS->getValue(AP_STRING_ID_DLG_InsertBookmark_Title) );

	// localize controls
	_DSX(INSERTBOOKMARK_BTN_OK, 			DLG_OK);
	_DSX(INSERTBOOKMARK_BTN_DELETE, 		DLG_Delete);
	_DSX(INSERTBOOKMARK_BTN_CANCEL, 		DLG_Cancel);

	_DS(INSERTBOOKMARK_LBL_MESSAGE, 		DLG_InsertBookmark_Msg);

	// initial data
	_win32Dialog.resetComboContent(AP_RID_DIALOG_INSERTBOOKMARK_CBX_BOOKMARK);

	UT_uint32 count = getExistingBookmarksCount();
	for( UT_uint32 i = 0; i < count; i++)
	{
		_win32Dialog.addItemToCombo( AP_RID_DIALOG_INSERTBOOKMARK_CBX_BOOKMARK,
									 getNthExistingBookmark( i ) );
	}
	XAP_Win32DialogHelper::s_centerDialog(hWnd);	
	return 1;
}

BOOL AP_Win32Dialog_InsertBookmark::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;

	switch (wId)
	{
	case IDCANCEL:
		setAnswer( a_CANCEL );
		EndDialog(hWnd,0);
		return 1;

	case IDOK:
		{
			XML_Char buf[BOOKMARK_SIZE_LIMIT+1];
			_win32Dialog.getControlText( AP_RID_DIALOG_INSERTBOOKMARK_CBX_BOOKMARK,
										 buf,
										 BOOKMARK_SIZE_LIMIT );
			setBookmark(buf);
		}
		setAnswer( a_OK );
		EndDialog(hWnd, 0);
		return 1;

	case AP_RID_DIALOG_INSERTBOOKMARK_BTN_DELETE:
		{
			XML_Char buf[BOOKMARK_SIZE_LIMIT+1];
			_win32Dialog.getControlText( AP_RID_DIALOG_INSERTBOOKMARK_CBX_BOOKMARK,
										 buf,
										 BOOKMARK_SIZE_LIMIT );
			setBookmark(buf);
		}
		setAnswer( a_DELETE );
		EndDialog(hWnd, 0);
		return 1;

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}

BOOL AP_Win32Dialog_InsertBookmark::_onDeltaPos(NM_UPDOWN * pnmud)
{
	return 0;
}
