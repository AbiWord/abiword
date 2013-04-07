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

#include "xap_Dialog_Id.h"
#include "xap_Dlg_History.h"
#include "xap_Win32Dlg_History.h"
#include "xap_Win32LabelledSeparator.h"

#include "xap_Win32Resources.rc2"
#include "xap_Win32DialogHelper.h"

/*****************************************************************/

XAP_Dialog * XAP_Win32Dialog_History::static_constructor(XAP_DialogFactory * pFactory,
														 XAP_Dialog_Id id)
{
	XAP_Win32Dialog_History * p = new XAP_Win32Dialog_History(pFactory,id);
	return p;
}

XAP_Win32Dialog_History::XAP_Win32Dialog_History(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: XAP_Dialog_History(pDlgFactory,id)
{
}

XAP_Win32Dialog_History::~XAP_Win32Dialog_History(void)
{
}

void XAP_Win32Dialog_History::runModal(XAP_Frame * pFrame)
{
 	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);

	XAP_Win32LabelledSeparator_RegisterClass(pWin32App);
	UT_ASSERT(m_id == XAP_DIALOG_ID_HISTORY);	
	createModal(pFrame, MAKEINTRESOURCEW(XAP_RID_DIALOG_HISTORY));	
}

BOOL XAP_Win32Dialog_History::_onInitDialog(HWND hWnd, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
    UT_Win32LocaleString str;
	// set the window title
	setDialogTitle(getWindowLabel());
	
	// localize buttons
	setDlgItemText(XAP_RID_DIALOG_HISTORY_BTN_OK,getButtonLabel(0));
	//setDlgItemText(XAP_RID_DIALOG_HISTORY_BTN_SHOW,getButtonLabel(1));
	setDlgItemText(XAP_RID_DIALOG_HISTORY_BTN_CANCEL,getButtonLabel(getButtonCount()-1));

	// set the list title
	setDlgItemText(XAP_RID_DIALOG_HISTORY_FRAME,getListTitle());
	
	// fill in the section above the list
	UT_uint32 i;
	UT_uint32 k1 = XAP_RID_DIALOG_HISTORY_LABEL_PATH;
	UT_uint32 k2 = XAP_RID_DIALOG_HISTORY_PATH;
								  
	for(i = 0; i < getHeaderItemCount(); i++)
	{
		setDlgItemText(k1 + i,getHeaderLabel(i));
		
		char * t = getHeaderValue(i);
		setDlgItemText(hWnd,k2 + i, t); // !
		FREEP(t);
	}

	// set the column headings
	HWND h = GetDlgItem(hWnd, XAP_RID_DIALOG_HISTORY_LIST_VERSIONS);

	LVCOLUMNW col;
  

	for(i = 0; i < getListColumnCount(); i++)
	{
        str.fromUTF8(getListHeader(i));
		col.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;
		col.iSubItem = i;
		col.cx = 120;
		col.pszText = (wchar_t *)str.c_str();
		SendMessageW(h, LVM_INSERTCOLUMNW, i, (LPARAM)&col);
	}

	// fill the list
	ListView_SetItemCount(h, getListItemCount());

	LVITEMW item;
	item.state = 0;
	item.stateMask = 0;
	item.iImage = 0;

	//wchar_t buf[80];
	//item.pszText = buf;
	char * t;

	for(i = 0; i < getListItemCount(); i++)
	{
		item.iItem = i;

		for(UT_uint32 j = 0; j < getListColumnCount(); j++)
		{
			item.iSubItem = j;
			t = getListValue(i,j);
			str.fromUTF8(t);
			item.pszText = (wchar_t *) str.c_str();
            item.mask = LVIF_TEXT;
			
			if(j==0)
			{
				item.lParam = getListItemId(i);
				item.mask |= LVIF_PARAM;
				SendMessageW(h, LVM_INSERTITEMW, 0, (LPARAM)&item);
			}
			else
			{
				SendMessageW(h, LVM_SETITEMW, 0, (LPARAM)&item);
			}
			
			FREEP(t);
		}
	}
	
	SendMessageW(h, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);  								
	centerDialog();	

	return 1;							// 1 == we did not call SetFocus()
}

BOOL XAP_Win32Dialog_History::_onCommand(HWND hWnd, WPARAM wParam, LPARAM /*lParam*/)
{
	WORD wId = LOWORD(wParam);

	switch (wId)
	{
		case IDCANCEL:
			m_answer = a_CANCEL;
			EndDialog(hWnd,0);
			return 1;

		case XAP_RID_DIALOG_HISTORY_BTN_SHOW:
		case IDOK:
			{
				HWND h = GetDlgItem(hWnd, XAP_RID_DIALOG_HISTORY_LIST_VERSIONS);
				int iSelCount = ListView_GetSelectedCount(h);
				if(iSelCount)
				{
					LVITEMW item;

					item.iSubItem = 0;
					item.iItem = ListView_GetSelectionMark(h);
					item.mask = LVIF_PARAM;
					item.pszText = 0;
					item.cchTextMax = 0;

					SendMessageW(h, LVM_GETITEMW, 0, (LPARAM)&item);

					setSelectionId(item.lParam);
				}
				else
					setSelectionId(0);

				if(wId == IDOK)
					m_answer = a_OK;
				else
					m_answer = a_SHOW;

				EndDialog(hWnd,0);
				return 1;
			}

		default:							// we did not handle this notification
			UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
			return 0;						// return zero to let windows take care of it.
	}
}


