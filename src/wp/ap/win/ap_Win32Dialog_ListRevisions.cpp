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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <windows.h>
#include <commctrl.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_ListRevisions.h"
#include "ap_Win32Dialog_ListRevisions.h"
#include "xap_Win32LabelledSeparator.h"

#include "ap_Win32Resources.rc2"
#include "xap_Win32DialogHelper.h"

/*****************************************************************/

XAP_Dialog * AP_Win32Dialog_ListRevisions::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_Win32Dialog_ListRevisions * p = new AP_Win32Dialog_ListRevisions(pFactory,id);
	return p;
}

AP_Win32Dialog_ListRevisions::AP_Win32Dialog_ListRevisions(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_ListRevisions(pDlgFactory,id)
{
}

AP_Win32Dialog_ListRevisions::~AP_Win32Dialog_ListRevisions(void)
{
}

void AP_Win32Dialog_ListRevisions::runModal(XAP_Frame * pFrame)
{
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);

	XAP_Win32LabelledSeparator_RegisterClass(pWin32App);

	UT_return_if_fail (m_id == AP_DIALOG_ID_LIST_REVISIONS);

    createModal(pFrame, MAKEINTRESOURCEW(AP_RID_DIALOG_LIST_REVISIONS));
}

#define _DSX(c,s)	setDlgItemText(AP_RID_DIALOG_LIST_REVISIONS_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL AP_Win32Dialog_ListRevisions::_onInitDialog(HWND hWnd, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
    
	setDialogTitle (getTitle());

	// localize controls
	_DSX(BTN_OK,			DLG_OK);
	_DSX(BTN_CANCEL,		DLG_Cancel);

	setDlgItemText(AP_RID_DIALOG_LIST_REVISIONS_FRAME,getLabel1());

	// set the column headings
	HWND h = GetDlgItem(hWnd, AP_RID_DIALOG_LIST_REVISIONS_LIST);

    LVCOLUMNW col;
    UT_Win32LocaleString str;
	col.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;

	int col_hdr[3]={AP_STRING_ID_DLG_ListRevisions_Column1Label,
					AP_STRING_ID_DLG_ListRevisions_Column2Label,
					AP_STRING_ID_DLG_ListRevisions_Column3Label};
	int col_wid[3]={80,160,230};

	for (int i=0; i<3; i++) {
		col.iSubItem = i;
		col.cx = col_wid[i];
		str.fromUTF8(pSS->getValue(col_hdr[i]));
		col.pszText = (LPWSTR) str.c_str();
		SendMessageW(h, LVM_INSERTCOLUMNW, i, (LPARAM)&col);
	}

	ListView_SetItemCount(h, getItemCount());

	LVITEMW item;
	item.state = 0;
	item.stateMask = 0;
	item.iImage = 0;

	WCHAR buf[60];
	const char *tmp;
	item.pszText = buf;

	for(UT_uint32 i = 0; i < getItemCount(); i++)
	{
		wsprintfW(buf,L"%d",getNthItemId(i));
		item.pszText = buf;
		item.iItem = i;
		item.iSubItem = 0;
		item.lParam = getNthItemId(i);
		item.mask = LVIF_TEXT | LVIF_PARAM;
		SendMessageW(h, LVM_INSERTITEMW, 0, (LPARAM)&item);

		item.iSubItem = 1;
		tmp=getNthItemTime(i);
		if (tmp) {
			str.fromASCII(tmp,-1);
			item.pszText = (LPWSTR) str.c_str();
		} else {
			item.pszText = L"";
		}
		item.mask = LVIF_TEXT;
		SendMessageW(h, LVM_SETITEMW, 0, (LPARAM)&item);
		
		item.iSubItem = 2;
		str.fromUTF8(getNthItemText(i));
		item.pszText = (LPWSTR) str.c_str();
		item.mask = LVIF_TEXT;
		SendMessageW(h, LVM_SETITEMW, 0, (LPARAM)&item);
	}
   
	SendMessageW(h, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);  								
    centerDialog();	

	return 1;							// 1 == we did not call SetFocus()
}

BOOL AP_Win32Dialog_ListRevisions::_onCommand(HWND hWnd, WPARAM wParam, LPARAM /*lParam*/)
{
	WORD wId = LOWORD(wParam);

	switch (wId)
	{
		case IDCANCEL:
			m_answer = a_CANCEL;
			EndDialog(hWnd,0);
			return 1;

		case IDOK:
			{
				HWND h = GetDlgItem(hWnd, AP_RID_DIALOG_LIST_REVISIONS_LIST);
				int iSelCount = ListView_GetSelectedCount(h);
				m_answer = a_OK;
				if(iSelCount)
				{
					LVITEMW item;

					item.iSubItem = 0;
					item.iItem = ListView_GetSelectionMark(h);
					item.mask = LVIF_PARAM;
					item.pszText = 0;
					item.cchTextMax = 0;

					ListView_GetItem(h,&item);

					m_iId = item.lParam;
				}
				else
					m_iId = 0;

				EndDialog(hWnd,0);
				return 1;
			}

		default:							// we did not handle this notification
			UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
			return 0;						// return zero to let windows take care of it.
	}
}


