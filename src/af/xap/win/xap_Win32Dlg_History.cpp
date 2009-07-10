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

#ifdef __MINGW32__
#define LVM_GETSELECTIONMARK    (LVM_FIRST+66)
#define ListView_GetSelectionMark(w) (INT)SNDMSG((w),LVM_GETSELECTIONMARK,0,0)
#endif


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
	UT_return_if_fail(pFrame);
	// raise the dialog
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);

	XAP_Win32LabelledSeparator_RegisterClass(pWin32App);

	LPCTSTR lpTemplate = NULL;

	UT_ASSERT(m_id == XAP_DIALOG_ID_HISTORY);

	lpTemplate = MAKEINTRESOURCE(XAP_RID_DIALOG_HISTORY);

	int result = DialogBoxParam(pWin32App->getInstance(),lpTemplate,
						static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow(),
						(DLGPROC)s_dlgProc,(LPARAM)this);
	UT_ASSERT_HARMLESS((result != -1));
	if(result == -1)
	{
		UT_DEBUGMSG(( "XAP_Win32Dialog_History::runModal error %d\n", GetLastError() ));
	}
}

BOOL CALLBACK XAP_Win32Dialog_History::s_dlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// This is a static function.

	XAP_Win32Dialog_History * pThis;

	switch (msg)
	{
	case WM_INITDIALOG:
		pThis = (XAP_Win32Dialog_History *)lParam;
		SetWindowLong(hWnd,DWL_USER,lParam);
		return pThis->_onInitDialog(hWnd,wParam,lParam);

	case WM_COMMAND:
		pThis = (XAP_Win32Dialog_History *)GetWindowLong(hWnd,DWL_USER);
		return pThis->_onCommand(hWnd,wParam,lParam);

	default:
		return 0;
	}
}

BOOL XAP_Win32Dialog_History::_onInitDialog(HWND hWnd, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	// set the window title
	SetWindowText(hWnd, getWindowLabel());
	
	// localize buttons
	SetDlgItemText(hWnd,XAP_RID_DIALOG_HISTORY_BTN_OK,getButtonLabel(0));
	//SetDlgItemText(hWnd,XAP_RID_DIALOG_HISTORY_BTN_SHOW,getButtonLabel(1));
	SetDlgItemText(hWnd,XAP_RID_DIALOG_HISTORY_BTN_CANCEL,getButtonLabel(getButtonCount()-1));

	// set the list title
	SetDlgItemText(hWnd, XAP_RID_DIALOG_HISTORY_FRAME,getListTitle());
	
	// fill in the section above the list
	UT_uint32 i;
	UT_uint32 k1 = XAP_RID_DIALOG_HISTORY_LABEL_PATH;
	UT_uint32 k2 = XAP_RID_DIALOG_HISTORY_PATH;
								  
	for(i = 0; i < getHeaderItemCount(); i++)
	{
		SetDlgItemText(hWnd,k1 + i,getHeaderLabel(i));
		
		char * t = getHeaderValue(i);
		SetDlgItemText(hWnd,k2 + i, t);
		FREEP(t);
	}

	// set the column headings
	HWND h = GetDlgItem(hWnd, XAP_RID_DIALOG_HISTORY_LIST_VERSIONS);

	LVCOLUMN col;

	for(i = 0; i < getListColumnCount(); i++)
	{
		col.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;
		col.iSubItem = i;
		col.cx = 120;
		col.pszText = const_cast<char*>(getListHeader(i));
		ListView_InsertColumn(h,i,&col);
	}

	// fill the list
	ListView_SetItemCount(h, getListItemCount());

	LVITEM item;
	item.state = 0;
	item.stateMask = 0;
	item.iImage = 0;

	char buf[35];
	item.pszText = buf;
	char * t;

	for(i = 0; i < getListItemCount(); i++)
	{
		item.iItem = i;

		for(UT_uint32 j = 0; j < getListColumnCount(); j++)
		{
			item.iSubItem = j;
			t = getListValue(i,j);
			item.pszText = t;
			item.mask = LVIF_TEXT;
			
			if(j==0)
			{
				item.lParam = getListItemId(i);
				item.mask |= LVIF_PARAM;
				ListView_InsertItem(h, &item);
			}
			else
			{
				ListView_SetItem(h, &item);
			}
			
			FREEP(t);
		}
	}
	
	SendMessage(h, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);  								
	XAP_Win32DialogHelper::s_centerDialog(hWnd);	

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
					LVITEM item;

					item.iSubItem = 0;
					item.iItem = ListView_GetSelectionMark(h);
					item.mask = LVIF_PARAM;
					item.pszText = 0;
					item.cchTextMax = 0;

					ListView_GetItem(h,&item);

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


