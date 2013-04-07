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

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_InsertBookmark.h"
#include "ap_Win32Dialog_InsertBookmark.h"
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
	: AP_Dialog_InsertBookmark(pDlgFactory,id)
{
}

AP_Win32Dialog_InsertBookmark::~AP_Win32Dialog_InsertBookmark(void)
{
}

void AP_Win32Dialog_InsertBookmark::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail (pFrame && m_id == AP_DIALOG_ID_INSERTBOOKMARK);
	
	setDialog(this);
	createModal(pFrame, MAKEINTRESOURCEW(AP_RID_DIALOG_INSERTBOOKMARK));	
}

#define _DS(c,s)	setDlgItemText(AP_RID_DIALOG_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	setDlgItemText(AP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL AP_Win32Dialog_InsertBookmark::_onInitDialog(HWND /*hWnd*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	const XAP_StringSet* pSS = m_pApp->getStringSet();

	// Update the caption
	setDialogTitle(pSS->getValue(AP_STRING_ID_DLG_InsertBookmark_Title));

	/* Localise controls*/
	_DSX(INSERTBOOKMARK_BTN_OK,			DLG_OK);
	_DSX(INSERTBOOKMARK_BTN_DELETE,			DLG_Delete);
	_DSX(INSERTBOOKMARK_BTN_CANCEL,			DLG_Cancel);
	_DS(INSERTBOOKMARK_LBL_MESSAGE,			DLG_InsertBookmark_Msg);

	// initial data
	resetComboContent(AP_RID_DIALOG_INSERTBOOKMARK_CBX_BOOKMARK);

	UT_uint32 count = getExistingBookmarksCount();
	for( UT_uint32 i = 0; i < count; i++)
	{
		addItemToCombo( AP_RID_DIALOG_INSERTBOOKMARK_CBX_BOOKMARK,
			getNthExistingBookmark( i ).c_str() );
	}

	UT_UCS4String suggestedBM = getSuggestedBM();

	if (!suggestedBM.empty())
	{
		setControlText(AP_RID_DIALOG_INSERTBOOKMARK_CBX_BOOKMARK, suggestedBM.utf8_str());
		enableControl(AP_RID_DIALOG_INSERTBOOKMARK_BTN_OK, true);
		enableControl(AP_RID_DIALOG_INSERTBOOKMARK_BTN_DELETE, (getComboItemIndex(AP_RID_DIALOG_INSERTBOOKMARK_CBX_BOOKMARK, suggestedBM.utf8_str())!=CB_ERR) ? true : false);
	}
	else
	{
		enableControl(AP_RID_DIALOG_INSERTBOOKMARK_BTN_OK, false);
		enableControl(AP_RID_DIALOG_INSERTBOOKMARK_BTN_DELETE, false);
	}

	centerDialog();	
	return 1;
}

BOOL AP_Win32Dialog_InsertBookmark::_onCommand(HWND hWnd, WPARAM wParam, LPARAM /*lParam*/)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
//	HWND hWndCtrl = (HWND)lParam;

	switch (wId)
	{
	case IDCANCEL:
		setAnswer( a_CANCEL );
		EndDialog(hWnd,0);
		return 1;

	case IDOK:
		{
			gchar buf[BOOKMARK_SIZE_LIMIT+1];
			getControlText( AP_RID_DIALOG_INSERTBOOKMARK_CBX_BOOKMARK,
							 buf,
							 BOOKMARK_SIZE_LIMIT );
			setBookmark(buf);
		}
		setAnswer( a_OK );
		EndDialog(hWnd, 0);
		return 1;

	case AP_RID_DIALOG_INSERTBOOKMARK_BTN_DELETE:
		{
			gchar buf[BOOKMARK_SIZE_LIMIT+1];
			getControlText( AP_RID_DIALOG_INSERTBOOKMARK_CBX_BOOKMARK,
							 buf,
							 BOOKMARK_SIZE_LIMIT );
			setBookmark(buf);
		}
		setAnswer( a_DELETE );
		EndDialog(hWnd, 0);
		return 1;

	case AP_RID_DIALOG_INSERTBOOKMARK_CBX_BOOKMARK:
		switch (wNotifyCode)
		{
			case CBN_SELCHANGE:
			{
				int sel = getComboSelectedIndex(AP_RID_DIALOG_INSERTBOOKMARK_CBX_BOOKMARK);

				enableControl(AP_RID_DIALOG_INSERTBOOKMARK_BTN_OK, (sel!=CB_ERR) ? true : false);
				enableControl(AP_RID_DIALOG_INSERTBOOKMARK_BTN_DELETE, (sel!=CB_ERR) ? true : false);
				return 1;
			}
			case CBN_EDITCHANGE:
			{
				gchar buf[BOOKMARK_SIZE_LIMIT+1];
				getControlText( AP_RID_DIALOG_INSERTBOOKMARK_CBX_BOOKMARK,
								buf,
								BOOKMARK_SIZE_LIMIT );

				enableControl(AP_RID_DIALOG_INSERTBOOKMARK_BTN_OK, *buf ? true : false);
				enableControl(AP_RID_DIALOG_INSERTBOOKMARK_BTN_DELETE, (getComboItemIndex(AP_RID_DIALOG_INSERTBOOKMARK_CBX_BOOKMARK, buf)!=CB_ERR) ? true : false);
				return 1;
			}
		}
		return 0;

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}

BOOL AP_Win32Dialog_InsertBookmark::_onDeltaPos(NM_UPDOWN * /*pnmud*/)
{
	return 0;
}
