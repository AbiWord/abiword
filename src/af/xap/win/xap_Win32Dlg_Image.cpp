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

#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_Image.h"
#include "xap_Win32Dlg_Image.h"

#include "xap_Win32Resources.rc2"

#define BUFSIZE 1024
/*****************************************************************/

XAP_Dialog * XAP_Win32Dialog_Image::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	XAP_Win32Dialog_Image * p = new XAP_Win32Dialog_Image(pFactory,id);
	return p;
}

#ifdef _MSC_VER	// MSVC++ warns about using 'this' in initializer list.
#pragma warning(disable: 4355)
#endif

XAP_Win32Dialog_Image::XAP_Win32Dialog_Image(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: XAP_Dialog_Image(pDlgFactory,id)
{
}

XAP_Win32Dialog_Image::~XAP_Win32Dialog_Image(void)
{
}

void XAP_Win32Dialog_Image::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	UT_ASSERT(m_id == XAP_DIALOG_ID_IMAGE);

	setDialog(this);
	createModal(pFrame, MAKEINTRESOURCE(XAP_RID_DIALOG_IMAGE));
}

BOOL XAP_Win32Dialog_Image::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	// localize dialog title
	localizeDialogTitle(XAP_STRING_ID_DLG_Image_Title);

	// localize controls
	localizeControlText(XAP_RID_DIALOG_IMAGE_BTN_OK,		XAP_STRING_ID_DLG_OK);
	localizeControlText(XAP_RID_DIALOG_IMAGE_BTN_CANCEL,	XAP_STRING_ID_DLG_Cancel);
	localizeControlText(XAP_RID_DIALOG_IMAGE_LBL_HEIGHT,	XAP_STRING_ID_DLG_Image_Height);
	localizeControlText(XAP_RID_DIALOG_IMAGE_LBL_WIDTH,		XAP_STRING_ID_DLG_Image_Width);
	localizeControlText(XAP_RID_DIALOG_IMAGE_CHK_ASPECT,	XAP_STRING_ID_DLG_Image_Aspect);

	// Initialize controls
	setControlText( XAP_RID_DIALOG_IMAGE_EBX_HEIGHT, getHeightString() );
	setControlText( XAP_RID_DIALOG_IMAGE_EBX_WIDTH, getWidthString() );
	checkButton( XAP_RID_DIALOG_IMAGE_CHK_ASPECT, getPreserveAspect() );

	return TRUE;
}

BOOL XAP_Win32Dialog_Image::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;

	switch (wId)
	{
	case IDCANCEL:						// also AP_RID_DIALOG_COLUMN_BTN_CANCEL
		setAnswer( a_Cancel );
		EndDialog(hWnd,0);
		return 1;

	case IDOK:							// also AP_RID_DIALOG_COLUMN_BTN_OK
		setAnswer( a_OK );
		EndDialog(hWnd,0);
		return 1;

	case XAP_RID_DIALOG_IMAGE_EBX_HEIGHT:
		if( wNotifyCode == EN_KILLFOCUS )
		{
			char buf[BUFSIZE];
			GetDlgItemTextA( hWnd, wId, buf, BUFSIZE ); //!TODO Using ANSI function
			setHeight( buf );
			setControlText( XAP_RID_DIALOG_IMAGE_EBX_HEIGHT, getHeightString() );
			setControlText( XAP_RID_DIALOG_IMAGE_EBX_WIDTH, getWidthString() );
		}
		return 1;

	case XAP_RID_DIALOG_IMAGE_EBX_WIDTH:
		if( wNotifyCode == EN_KILLFOCUS )
		{
			char buf[BUFSIZE];
			GetDlgItemTextA( hWnd, wId, buf, BUFSIZE ); //!TODO Using ANSI function
			setWidth( buf );
			setControlText( XAP_RID_DIALOG_IMAGE_EBX_HEIGHT, getHeightString() );
			setControlText( XAP_RID_DIALOG_IMAGE_EBX_WIDTH, getWidthString() );
		}
		return 1;

	case XAP_RID_DIALOG_IMAGE_CHK_ASPECT:
		setPreserveAspect( isChecked(wId)!=0 );
		return 1;

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}

BOOL XAP_Win32Dialog_Image::_onDeltaPos(NM_UPDOWN * pnmud)
{
	switch( pnmud->hdr.idFrom )
	{
	case XAP_RID_DIALOG_IMAGE_SPN_WIDTH:
		if( pnmud->iDelta < 0 )
		{
			incrementWidth( true );
		}
		else
		{
			incrementWidth( false );
		}
		setControlText( XAP_RID_DIALOG_IMAGE_EBX_HEIGHT, getHeightString() );
		setControlText( XAP_RID_DIALOG_IMAGE_EBX_WIDTH, getWidthString() );
		return 1;

	case XAP_RID_DIALOG_IMAGE_SPN_HEIGHT:
		if( pnmud->iDelta < 0 )
		{
			incrementHeight( true );
		}
		else
		{
			incrementHeight( false );
		}
		setControlText( XAP_RID_DIALOG_IMAGE_EBX_HEIGHT, getHeightString() );
		setControlText( XAP_RID_DIALOG_IMAGE_EBX_WIDTH, getWidthString() );
		return 1;

	default:
		return 0;
	}
}
