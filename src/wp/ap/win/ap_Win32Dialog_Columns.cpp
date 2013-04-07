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

#include <windows.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Columns.h"
#include "ap_Win32Dialog_Columns.h"

#include "gr_Win32Graphics.h"
#include "xap_Win32DialogHelper.h"
#include "ap_Win32Resources.rc2"
#include "xap_Win32Toolbar_Icons.h"

#define BUFSIZE 1024

/*****************************************************************/

XAP_Dialog * AP_Win32Dialog_Columns::static_constructor(XAP_DialogFactory * pFactory,
													   XAP_Dialog_Id id)
{
	AP_Win32Dialog_Columns * p = new AP_Win32Dialog_Columns(pFactory,id);
	return p;
}

AP_Win32Dialog_Columns::AP_Win32Dialog_Columns(XAP_DialogFactory * pDlgFactory,
											   XAP_Dialog_Id id)
	: AP_Dialog_Columns(pDlgFactory,id)
{
	m_pPreviewWidget = NULL;
}

AP_Win32Dialog_Columns::~AP_Win32Dialog_Columns(void)
{
	delete m_pPreviewWidget;
}

/*****************************************************************/

void AP_Win32Dialog_Columns::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail (m_id == AP_DIALOG_ID_COLUMNS);
	
	// raise the dialog
	setViewAndDoc(pFrame);
	setDialog(this);
	createModal(pFrame, MAKEINTRESOURCEW(AP_RID_DIALOG_COLUMNS));
}

void AP_Win32Dialog_Columns::enableLineBetweenControl(bool bState)
{
	// As this function gets called prior to getting an hWnd for the dialog
	// we check to see if the dialog has been initialized prior to
	// running the conrol
	if ( isDialogValid() )
	{
		enableControl(AP_RID_DIALOG_COLUMN_CHECK_LINE_BETWEEN, bState);
	}
}

#define _DS(c,s)	setDlgItemText(AP_RID_DIALOG_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	setDlgItemText(AP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL AP_Win32Dialog_Columns::_onInitDialog(HWND hWnd, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	const XAP_StringSet* pSS = m_pApp->getStringSet();

	// Update the caption
	setDialogTitle(pSS->getValue(AP_STRING_ID_DLG_Column_ColumnTitle));

	/* Localise controls*/
	_DSX(COLUMN_BTN_OK,			DLG_OK);
	_DSX(COLUMN_BTN_CANCEL,			DLG_Cancel);
	_DS(COLUMN_GROUP1,			DLG_Column_Number);
	_DS(COLUMN_GROUP2,			DLG_Column_Preview);
	_DS(COLUMN_TEXT_ONE,			DLG_Column_One);
	_DS(COLUMN_TEXT_TWO,			DLG_Column_Two);
	_DS(COLUMN_TEXT_THREE,			DLG_Column_Three);
	_DS(COLUMN_CHECK_LINE_BETWEEN,		DLG_Column_Line_Between);
	_DS(COLUMN_TEXT_NUMCOLUMNS,		DLG_Column_Number_Cols);
	_DS(COLUMN_TEXT_SPACEAFTER,		DLG_Column_Space_After);
	_DS(COLUMN_TEXT_MAXSIZE,		DLG_Column_Size);
	_DS(COLUMN_CHECK_RTL_ORDER,		DLG_Column_RtlOrder);

	// Do Bitmaps
	RECT rect;
	GetClientRect(GetDlgItem(hWnd, AP_RID_DIALOG_COLUMN_RADIO_ONE), &rect);
	int iWidth = rect.right - rect.left;
	int iHeight = rect.bottom - rect.top;

	HBITMAP hBitmap;
	XAP_Win32Toolbar_Icons Icons;
	COLORREF ColorRef = GetSysColor(COLOR_BTNFACE);
	UT_RGBColor Color(GetRValue(ColorRef), GetGValue(ColorRef), GetBValue(ColorRef));

	bool bFoundIcon = Icons.getBitmapForIcon(hWnd, iWidth, iHeight, &Color, "1COLUMN",
																&hBitmap);
	UT_return_val_if_fail (bFoundIcon, false);
	SendDlgItemMessageW(hWnd, AP_RID_DIALOG_COLUMN_RADIO_ONE, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);

	bFoundIcon = Icons.getBitmapForIcon(hWnd, iWidth, iHeight, &Color, "2COLUMN",
																&hBitmap);
	UT_return_val_if_fail (bFoundIcon, false);
	SendDlgItemMessageW(hWnd, AP_RID_DIALOG_COLUMN_RADIO_TWO, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);

	bFoundIcon = Icons.getBitmapForIcon(hWnd, iWidth, iHeight, &Color, "3COLUMN",
																&hBitmap);
	UT_return_val_if_fail (bFoundIcon, false);
	SendDlgItemMessageW(hWnd, AP_RID_DIALOG_COLUMN_RADIO_THREE, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hBitmap);

	// set initial state
	wchar_t buf[BUFSIZE];
	checkButton(AP_RID_DIALOG_COLUMN_RADIO_ONE + getColumns() - 1, true);
	enableLineBetweenControl(getColumns() != 1);
	checkButton(AP_RID_DIALOG_COLUMN_CHECK_LINE_BETWEEN, getLineBetween());
	SetDlgItemTextW(m_hDlg, AP_RID_DIALOG_COLUMN_EDIT_NUMCOLUMNS, _itow( getColumns(),buf,10) );    
	setControlText(AP_RID_DIALOG_COLUMN_EDIT_SPACEAFTER, getSpaceAfterString());
	setControlText(AP_RID_DIALOG_COLUMN_EDIT_MAXSIZE, getHeightString());

	showControl( AP_RID_DIALOG_COLUMN_CHECK_RTL_ORDER, SW_NORMAL );
	checkButton(AP_RID_DIALOG_COLUMN_CHECK_RTL_ORDER, getColumnOrder()!=false);

	// Create a preview window.

	HWND hwndChild = GetDlgItem(hWnd, AP_RID_DIALOG_COLUMN_PREVIEW);

	m_pPreviewWidget = new XAP_Win32PreviewWidget(static_cast<XAP_Win32App *>(m_pApp),
													  hwndChild,
													  0);
	UT_uint32 w,h;
	m_pPreviewWidget->getWindowSize(&w,&h);
	//m_pPreviewWidget->init3dColors(m_wpreviewArea->style);
	m_pPreviewWidget->getGraphics()->init3dColors();
	_createPreviewFromGC(m_pPreviewWidget->getGraphics(), w, h);
	m_pPreviewWidget->setPreview(m_pColumnsPreview);
	
	centerDialog();	
	
	return 1;	// 1 == we did not call SetFocus()
}


BOOL AP_Win32Dialog_Columns::_onCommand(HWND hWnd, WPARAM wParam, LPARAM /*lParam*/)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	wchar_t buf[BUFSIZE];
    UT_Win32LocaleString str;
    
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
		checkButton(AP_RID_DIALOG_COLUMN_RADIO_TWO, false);
		checkButton(AP_RID_DIALOG_COLUMN_RADIO_THREE, false);
		SetDlgItemTextW(m_hDlg, AP_RID_DIALOG_COLUMN_EDIT_NUMCOLUMNS, _itow(getColumns(),buf,10));
		return 1;

	case AP_RID_DIALOG_COLUMN_RADIO_TWO:
		setColumns(2);
		checkButton(AP_RID_DIALOG_COLUMN_RADIO_ONE, false);
		checkButton(AP_RID_DIALOG_COLUMN_RADIO_THREE, false);
		SetDlgItemTextW(m_hDlg, AP_RID_DIALOG_COLUMN_EDIT_NUMCOLUMNS, _itow(getColumns(),buf,10));
		return 1;

	case AP_RID_DIALOG_COLUMN_RADIO_THREE:
		setColumns(3);
		checkButton(AP_RID_DIALOG_COLUMN_RADIO_ONE, false);
		checkButton(AP_RID_DIALOG_COLUMN_RADIO_TWO, false);
		SetDlgItemTextW(m_hDlg, AP_RID_DIALOG_COLUMN_EDIT_NUMCOLUMNS, _itow(getColumns(),buf,10));
		return 1;

	case AP_RID_DIALOG_COLUMN_CHECK_LINE_BETWEEN:
		setLineBetween( isChecked(AP_RID_DIALOG_COLUMN_CHECK_LINE_BETWEEN)==BST_CHECKED );
		return 1;

	case AP_RID_DIALOG_COLUMN_EDIT_NUMCOLUMNS:
		if( wNotifyCode == EN_KILLFOCUS )
		{
			GetDlgItemTextW( hWnd, wId, buf, BUFSIZE );
			if( _wtoi( buf ) > 0 && _wtoi(buf) != (signed) getColumns() )
			{
				setColumns( _wtoi(buf) );
			}
			SetDlgItemTextW(m_hDlg, wId, _itow(getColumns(),buf,10));
			checkButton(AP_RID_DIALOG_COLUMN_RADIO_ONE, (getColumns()==1));
			checkButton(AP_RID_DIALOG_COLUMN_RADIO_TWO, (getColumns()==2));
			checkButton(AP_RID_DIALOG_COLUMN_RADIO_THREE, (getColumns()==3));
		}
		return 1;

	case AP_RID_DIALOG_COLUMN_EDIT_SPACEAFTER:
		if( wNotifyCode == EN_KILLFOCUS )
		{
			GetDlgItemTextW( hWnd, wId, buf, BUFSIZE );
			str.fromLocale (buf);
			setSpaceAfter( str.utf8_str().utf8_str() );
			str.fromUTF8 (getSpaceAfterString ());
			SetDlgItemTextW(m_hDlg, wId, str.c_str ());
		}
		return 1;

	case AP_RID_DIALOG_COLUMN_EDIT_MAXSIZE:
		if( wNotifyCode == EN_KILLFOCUS )
		{
			GetDlgItemTextW( hWnd, wId, buf, BUFSIZE );
            str.fromLocale (buf);
			setMaxHeight( str.utf8_str().utf8_str() );
            str.fromUTF8 (getHeightString());
            SetDlgItemTextW(m_hDlg, wId, str.c_str ());
		}
		return 1;

	case AP_RID_DIALOG_COLUMN_CHECK_RTL_ORDER:
		setColumnOrder( (UT_uint32) (isChecked(AP_RID_DIALOG_COLUMN_CHECK_RTL_ORDER) == BST_CHECKED) );
		return 1;

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}

BOOL AP_Win32Dialog_Columns::_onDeltaPos(NM_UPDOWN * pnmud)
{
	wchar_t buf[BUFSIZE];
	UT_Win32LocaleString str;
    
	switch( pnmud->hdr.idFrom )
	{
	case AP_RID_DIALOG_COLUMN_SPIN_NUMCOLUMNS:
		if( pnmud->iDelta < 0 )
		{
			setColumns( getColumns() + 1 );
		}
		else
		{
			if( getColumns() > 1 )
			{
				setColumns( getColumns() - 1 );
			}
		}
		SetDlgItemTextW(m_hDlg, AP_RID_DIALOG_COLUMN_EDIT_NUMCOLUMNS, _itow(getColumns(),buf,10));
		checkButton(AP_RID_DIALOG_COLUMN_RADIO_ONE, (getColumns()==1));
		checkButton(AP_RID_DIALOG_COLUMN_RADIO_TWO, (getColumns()==2));
		checkButton(AP_RID_DIALOG_COLUMN_RADIO_THREE, (getColumns()==3));
		return 1;

	case AP_RID_DIALOG_COLUMN_SPIN_SPACEAFTER:
		if( pnmud->iDelta < 0 )
		{
			incrementSpaceAfter( true );
		}
		else
		{
			incrementSpaceAfter( false );
		}
        str.fromUTF8 (getSpaceAfterString());
        SetDlgItemTextW(m_hDlg, AP_RID_DIALOG_COLUMN_EDIT_SPACEAFTER, str.c_str ());
		return 1;

	case AP_RID_DIALOG_COLUMN_SPIN_MAXSIZE:
		if( pnmud->iDelta < 0 )
		{
			incrementMaxHeight( true );
		}
		else
		{
			incrementMaxHeight( false );
		}
        str.fromUTF8 (getHeightString());
        SetDlgItemTextW(m_hDlg, AP_RID_DIALOG_COLUMN_EDIT_MAXSIZE, str.c_str ());
		return 1;

	default:
		return 0;
	}
}
