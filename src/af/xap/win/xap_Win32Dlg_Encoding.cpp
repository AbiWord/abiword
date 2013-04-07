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
#include "xap_Win32Dlg_Encoding.h"

#include "xap_Win32Resources.rc2"

/*****************************************************************/

XAP_Dialog * XAP_Win32Dialog_Encoding::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	XAP_Win32Dialog_Encoding * p = new XAP_Win32Dialog_Encoding(pFactory,id);
	return p;
}

XAP_Win32Dialog_Encoding::XAP_Win32Dialog_Encoding(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: XAP_Dialog_Encoding(pDlgFactory,id)
{
}

XAP_Win32Dialog_Encoding::~XAP_Win32Dialog_Encoding(void)
{
}

void XAP_Win32Dialog_Encoding::runModal(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);
	UT_ASSERT(m_id == XAP_DIALOG_ID_ENCODING);

	setDialog(this);
	createModal(pFrame, MAKEINTRESOURCEW(XAP_RID_DIALOG_ENCODING));
}

BOOL XAP_Win32Dialog_Encoding::_onInitDialog(HWND hWnd, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	localizeDialogTitle(XAP_STRING_ID_DLG_UENC_EncTitle);

	// localize controls
	localizeControlText(XAP_RID_DIALOG_ENCODING_BTN_OK,			XAP_STRING_ID_DLG_OK);
	localizeControlText(XAP_RID_DIALOG_ENCODING_BTN_CANCEL,		XAP_STRING_ID_DLG_Cancel);
	localizeControlText(XAP_RID_DIALOG_ENCODING_FRM_ENCODING,   XAP_STRING_ID_DLG_UENC_EncLabel);
	
	// Load Initial Data into Listbox
	{
		resetContent(XAP_RID_DIALOG_ENCODING_LBX_ENCODING);

		// load each string name into the list
		for ( UT_uint32 i=0; i < _getEncodingsCount();  i++ )
		{
			const gchar* s = _getAllEncodings()[i];
			addItemToList(XAP_RID_DIALOG_ENCODING_LBX_ENCODING, (LPCSTR) s);
			setListDataItem(XAP_RID_DIALOG_ENCODING_LBX_ENCODING, i, (DWORD) i);
        }

		// Set to default or guessed encoding
		selectListItem(XAP_RID_DIALOG_ENCODING_LBX_ENCODING, _getSelectionIndex());
		
		SetFocus(GetDlgItem(hWnd, XAP_RID_DIALOG_ENCODING_LBX_ENCODING));
	}		
	
	return 0;							// 1 == we did not call SetFocus()
}

BOOL XAP_Win32Dialog_Encoding::_onCommand(HWND hWnd, WPARAM wParam, LPARAM /*lParam*/)
{
	WORD wId = LOWORD(wParam);
	int nItem;

	switch (wId)
	{
	case XAP_RID_DIALOG_ENCODING_LBX_ENCODING:
		switch (HIWORD(wParam))
		{
			case LBN_SELCHANGE:
				// NOTE: we get away with only grabbing this in IDOK case
				return 0;

			case LBN_DBLCLK:
				nItem = getListSelectedIndex(wId);
				_setEncoding( _getAllEncodings()[nItem] );
				_setAnswer(a_OK);
				EndDialog(hWnd,0);
				return 1;

			default:
				return 0;
		}
		break;

	case IDCANCEL:						// also XAP_RID_DIALOG_ENCODING_BTN_CANCEL
		_setAnswer(a_CANCEL);
		EndDialog(hWnd,0);
		return 1;

	case IDOK:							// also XAP_RID_DIALOG_ENCODING_BTN_OK
		nItem = getListSelectedIndex(XAP_RID_DIALOG_ENCODING_LBX_ENCODING);
		if( nItem != LB_ERR)
		{
			_setEncoding( _getAllEncodings()[nItem] );
			_setAnswer(a_OK);
		}
		else
		{
			_setAnswer(a_CANCEL);
		}	
		EndDialog(hWnd,0);
		return 1;

	default:							// we did not handle this notification
		UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;						// return zero to let windows take care of it.
	}
}

BOOL XAP_Win32Dialog_Encoding::_onDeltaPos(NM_UPDOWN * /*pnmud*/)
{
	// respond to WM_NOTIFY/UDN_DELTAPOS message
	// return TRUE to prevent the change from happening
	// return FALSE to allow it to occur
	return FALSE;
}
