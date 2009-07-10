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

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_ListRevisions.h"
#include "ap_Win32Dialog_ListRevisions.h"
#include "xap_Win32LabelledSeparator.h"

#ifdef __MINGW32__
#define LVM_GETSELECTIONMARK    (LVM_FIRST+66)
#define ListView_GetSelectionMark(w) (INT)SNDMSG((w),LVM_GETSELECTIONMARK,0,0)
#endif


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
	UT_return_if_fail (pFrame);
	// raise the dialog
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);

	XAP_Win32LabelledSeparator_RegisterClass(pWin32App);

	LPCTSTR lpTemplate = NULL;

	UT_return_if_fail (m_id == AP_DIALOG_ID_LIST_REVISIONS);

	lpTemplate = MAKEINTRESOURCE(AP_RID_DIALOG_LIST_REVISIONS);

	int result = DialogBoxParam(pWin32App->getInstance(),lpTemplate,
						static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow(),
						(DLGPROC)s_dlgProc,(LPARAM)this);
	UT_ASSERT_HARMLESS((result != -1));
	if (result == -1)
	{
		UT_DEBUGMSG(( "AP_Win32Dialog_ListRevisions::runModal error %d\n", GetLastError() ));
	}
}

BOOL CALLBACK AP_Win32Dialog_ListRevisions::s_dlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// This is a static function.

	AP_Win32Dialog_ListRevisions * pThis;

	switch (msg)
	{
	case WM_INITDIALOG:
		pThis = (AP_Win32Dialog_ListRevisions *)lParam;
		SetWindowLong(hWnd,DWL_USER,lParam);
		return pThis->_onInitDialog(hWnd,wParam,lParam);

	case WM_COMMAND:
		pThis = (AP_Win32Dialog_ListRevisions *)GetWindowLong(hWnd,DWL_USER);
		return pThis->_onCommand(hWnd,wParam,lParam);

	default:
		return 0;
	}
}

#define _DSX(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_LIST_REVISIONS_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL AP_Win32Dialog_ListRevisions::_onInitDialog(HWND hWnd, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();

	SetWindowText(hWnd, getTitle());

	// localize controls
	_DSX(BTN_OK,			DLG_OK);
	_DSX(BTN_CANCEL,		DLG_Cancel);

	SetDlgItemText(hWnd, AP_RID_DIALOG_LIST_REVISIONS_FRAME,getLabel1());

	// set the column headings
	HWND h = GetDlgItem(hWnd, AP_RID_DIALOG_LIST_REVISIONS_LIST);

	LVCOLUMN col;
	col.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;

	col.iSubItem = 0;
	col.cx = 80;

	col.pszText = const_cast<char*>(getColumn1Label());
	ListView_InsertColumn(h,0,&col);

	col.iSubItem = 1;
	col.cx = 160;
	col.pszText = const_cast<char*>(getColumn2Label());
	ListView_InsertColumn(h,1,&col);

	col.iSubItem = 2;
	col.cx = 230;
	col.pszText = const_cast<char*>(getColumn3Label());
	ListView_InsertColumn(h,2,&col);
	
	ListView_SetItemCount(h, getItemCount());

	LVITEM item;
	item.state = 0;
	item.stateMask = 0;
	item.iImage = 0;

	char buf[35];
	item.pszText = buf;
	char * t;

	for(UT_uint32 i = 0; i < getItemCount(); i++)
	{
		sprintf(buf,"%d",getNthItemId(i));
		item.pszText = buf;
		item.iItem = i;
		item.iSubItem = 0;
		item.lParam = getNthItemId(i);
		item.mask = LVIF_TEXT | LVIF_PARAM;
		ListView_InsertItem(h, &item);

		item.iSubItem = 1;
		item.pszText = const_cast<char *>(getNthItemTime(i));
		item.mask = LVIF_TEXT;
		ListView_SetItem(h, &item);
		
		item.iSubItem = 2;
		t = getNthItemText(i);
		item.pszText = t;
		item.mask = LVIF_TEXT;
		ListView_SetItem(h, &item);
		FREEP(t);
	}
	
	SendMessage(h, LVM_SETEXTENDEDLISTVIEWSTYLE, LVS_EX_FULLROWSELECT, LVS_EX_FULLROWSELECT);  								
	XAP_Win32DialogHelper::s_centerDialog(hWnd);	

	return 1;							// 1 == we did not call SetFocus()
}

BOOL AP_Win32Dialog_ListRevisions::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
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
					LVITEM item;

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


