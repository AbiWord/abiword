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
#include "ap_Dialog_Columns.h"
#include "ap_Win32Dialog_Columns.h"

#include "ap_Win32Resources.rc2"
#include "xap_Win32Toolbar_Icons.h"

/*****************************************************************/

XAP_Dialog * AP_Win32Dialog_Columns::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id id)
{
	AP_Win32Dialog_Columns * p = new AP_Win32Dialog_Columns(pFactory,id);
	return p;
}

#ifdef _MSC_VER	// MSVC++ warns about using 'this' in initializer list.
#pragma warning(disable: 4355)
#endif

AP_Win32Dialog_Columns::AP_Win32Dialog_Columns(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id id)
	: AP_Dialog_Columns(pDlgFactory,id), _win32Dialog(this)
{
}

AP_Win32Dialog_Columns::~AP_Win32Dialog_Columns(void)
{
}

/*****************************************************************/

void AP_Win32Dialog_Columns::runModal(XAP_Frame * pFrame)
{
	// raise the dialog

	_win32Dialog.runModal(pFrame, AP_DIALOG_ID_COLUMNS, AP_RID_DIALOG_COLUMNS, this);

}

void AP_Win32Dialog_Columns::enableLineBetweenControl(bool bState)
	{
	_win32Dialog.enableControl(AP_RID_DIALOG_COLUMN_CHECK_LINE_BETWEEN, bState);
	}

#define _DS(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL AP_Win32Dialog_Columns::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	XAP_Win32App * app = static_cast<XAP_Win32App *> (m_pApp);
	UT_ASSERT(app);

	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	SetWindowText(hWnd, pSS->getValue(AP_STRING_ID_DLG_Column_ColumnTitle));

	// localize controls
	_DSX(COLUMN_BTN_OK,					DLG_OK);
	_DSX(COLUMN_BTN_CANCEL,				DLG_Cancel);
										
	_DS(COLUMN_GROUP1,					DLG_Column_Number);
	_DS(COLUMN_GROUP2,					DLG_Column_Preview);
	_DS(COLUMN_TEXT_ONE,				DLG_Column_One);
	_DS(COLUMN_TEXT_TWO,				DLG_Column_Two);
	_DS(COLUMN_TEXT_THREE,				DLG_Column_Three);
	_DS(COLUMN_CHECK_LINE_BETWEEN,		DLG_Column_Line_Between);

	RECT rect;
	GetClientRect(GetDlgItem(hWnd, AP_RID_DIALOG_COLUMN_RADIO_ONE), &rect);
	int iWidth = rect.right - rect.left;
	int iHeight = rect.bottom - rect.top;

	HBITMAP hBitmap;
	AP_Win32Toolbar_Icons Icons;
	COLORREF ColorRef = GetSysColor(COLOR_BTNFACE);
	UT_RGBColor Color(GetRValue(ColorRef), GetGValue(ColorRef), GetBValue(ColorRef));

	bool bFoundIcon = Icons.getBitmapForIcon(hWnd, iWidth, iHeight, &Color, "tb_1column_xpm",
																&hBitmap);
	UT_ASSERT(bFoundIcon);
	SendDlgItemMessage(hWnd, AP_RID_DIALOG_COLUMN_RADIO_ONE, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);

	bFoundIcon = Icons.getBitmapForIcon(hWnd, iWidth, iHeight, &Color, "tb_2column_xpm",
																&hBitmap);
	UT_ASSERT(bFoundIcon);
	SendDlgItemMessage(hWnd, AP_RID_DIALOG_COLUMN_RADIO_TWO, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);

	bFoundIcon = Icons.getBitmapForIcon(hWnd, iWidth, iHeight, &Color, "tb_3column_xpm",
																&hBitmap);
	UT_ASSERT(bFoundIcon);
	SendDlgItemMessage(hWnd, AP_RID_DIALOG_COLUMN_RADIO_THREE, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);

	// set initial state
	CheckDlgButton(hWnd, AP_RID_DIALOG_COLUMN_RADIO_ONE + getColumns() - 1, BST_CHECKED);
	enableLineBetweenControl(getColumns() != 1);
	CheckDlgButton(hWnd, AP_RID_DIALOG_COLUMN_CHECK_LINE_BETWEEN, getLineBetween() ? BST_CHECKED : BST_UNCHECKED);

	// Create a preview window.

	HWND hwndChild = GetDlgItem(hWnd, AP_RID_DIALOG_COLUMN_PREVIEW);

	m_pPreviewWidget = new XAP_Win32PreviewWidget(static_cast<XAP_Win32App *>(m_pApp),
													  hwndChild,
													  0);
	UT_uint32 w,h;
	m_pPreviewWidget->getWindowSize(&w,&h);
	_createPreviewFromGC(m_pPreviewWidget->getGraphics(), w, h);
	m_pPreviewWidget->setPreview(m_pColumnsPreview);


	return 1;							// 1 == we did not call SetFocus()
}

BOOL AP_Win32Dialog_Columns::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;

	switch (wId)
	{
	case IDCANCEL:						// also AP_RID_DIALOG_COLUMN_BTN_CANCEL
		m_answer = a_CANCEL;
		// fall through

	case IDOK:							// also AP_RID_DIALOG_COLUMN_BTN_OK
		EndDialog(hWnd,0);
		return 1;

	case AP_RID_DIALOG_COLUMN_RADIO_ONE:
		setColumns(1);
		CheckDlgButton(hWnd, AP_RID_DIALOG_COLUMN_RADIO_TWO, BST_UNCHECKED);
		CheckDlgButton(hWnd, AP_RID_DIALOG_COLUMN_RADIO_THREE, BST_UNCHECKED);
		return 1;

	case AP_RID_DIALOG_COLUMN_RADIO_TWO:
		setColumns(2);
		CheckDlgButton(hWnd, AP_RID_DIALOG_COLUMN_RADIO_ONE, BST_UNCHECKED);
		CheckDlgButton(hWnd, AP_RID_DIALOG_COLUMN_RADIO_THREE, BST_UNCHECKED);
		return 1;

	case AP_RID_DIALOG_COLUMN_RADIO_THREE:
		setColumns(3);
		CheckDlgButton(hWnd, AP_RID_DIALOG_COLUMN_RADIO_ONE, BST_UNCHECKED);
		CheckDlgButton(hWnd, AP_RID_DIALOG_COLUMN_RADIO_TWO, BST_UNCHECKED);
		return 1;

	case AP_RID_DIALOG_COLUMN_CHECK_LINE_BETWEEN:
		setLineBetween( (IsDlgButtonChecked(hWnd, AP_RID_DIALOG_COLUMN_CHECK_LINE_BETWEEN) == BST_CHECKED) );
		return 1;

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}

BOOL AP_Win32Dialog_Columns::_onDeltaPos(NM_UPDOWN * pnmud)
{
	return 0;
}
