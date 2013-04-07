/* AbiWord
 * Copyright (C) 2003 Tomas Frydrych <tomas@frydrych.uklinux.net>
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
#include <commctrl.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"
#include "xap_Win32DialogHelper.h"

#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_ListDocuments.h"
#include "xap_Win32Dlg_ListDocuments.h"
#include "xap_Win32LabelledSeparator.h"

#include "xap_Win32Resources.rc2"

/*****************************************************************/

XAP_Dialog * XAP_Win32Dialog_ListDocuments::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	XAP_Win32Dialog_ListDocuments * p = new XAP_Win32Dialog_ListDocuments(pFactory,id);
	return p;
}

XAP_Win32Dialog_ListDocuments::XAP_Win32Dialog_ListDocuments(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: XAP_Dialog_ListDocuments(pDlgFactory,id)
{
}

XAP_Win32Dialog_ListDocuments::~XAP_Win32Dialog_ListDocuments(void)
{
}

void XAP_Win32Dialog_ListDocuments::runModal(XAP_Frame * pFrame)
{
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);

	XAP_Win32LabelledSeparator_RegisterClass(pWin32App);
	createModal(pFrame, MAKEINTRESOURCEW(XAP_RID_DIALOG_LIST_DOCUMENTS));
}

#define _DSX(c,s) setDlgItemText(XAP_RID_DIALOG_LIST_DOCUMENTS_##c,pSS->getValue(XAP_STRING_ID_##s))
#define _DSXS(c,s) setDlgItemText(XAP_RID_DIALOG_LIST_DOCUMENTS_##c,s)

BOOL XAP_Win32Dialog_ListDocuments::_onInitDialog(HWND hWnd, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
    
	setDialogTitle(_getTitle());

	// localize controls
	_DSXS(BTN_OK,    _getOKButtonText());
	_DSX(BTN_CANCEL, DLG_Cancel);

	setDlgItemText(XAP_RID_DIALOG_LIST_DOCUMENTS_HEADING,_getHeading());
    
	// set the column headings
	HWND h = GetDlgItem(hWnd, XAP_RID_DIALOG_LIST_DOCUMENTS_LIST);

//	DWORD dwStyle = GetWindowLongPtrW(h, GWL_STYLE);

	RECT r;
	GetWindowRect(h, &r);
	
	LVCOLUMN col;
	col.mask = LVCF_SUBITEM | LVCF_WIDTH;

	col.iSubItem = 0;
	col.cx = r.right - r.left - 5;

	//col.pszText = const_cast<char*>(getColumn1Label());
	ListView_InsertColumn(h,0,&col); // TODO: to Unicode, names instead of URLs

	ListView_SetItemCount(h, _getDocumentCount());
	LVITEM item;
	for(UT_sint32 i = 0; i < _getDocumentCount(); i++)
	{
		item.pszText = (char *)_getNthDocumentName(i);
		item.iItem = i;
		item.iSubItem = 0;
		item.lParam = i;
		item.mask = LVIF_TEXT | LVIF_PARAM;
		ListView_InsertItem(h, &item);
	}

	centerDialog();
	return 1;							// 1 == we did not call SetFocus()
}

BOOL XAP_Win32Dialog_ListDocuments::_onCommand(HWND hWnd, WPARAM wParam, LPARAM /*lParam*/)
{
	WORD wId = LOWORD(wParam);

	switch (wId)
	{
		case IDCANCEL:
			_setAnswer(a_CANCEL);
			EndDialog(hWnd,0);
			return 1;

		case IDOK:
			{
				HWND h = GetDlgItem(hWnd, XAP_RID_DIALOG_LIST_DOCUMENTS_LIST);
				int iSelCount = ListView_GetSelectedCount(h);
				_setAnswer(a_OK);
				if(iSelCount)
				{
					LVITEM item;

					item.iSubItem = 0;
					item.iItem = ListView_GetSelectionMark(h);
					item.mask = LVIF_PARAM;
					item.pszText = 0;
					item.cchTextMax = 0;

					ListView_GetItem(h,&item);

					_setSelDocumentIndx(item.lParam);
				}
				else
					_setSelDocumentIndx(-1);

				EndDialog(hWnd,0);
				return 1;
			}

		default:							// we did not handle this notification
			UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
			return 0;						// return zero to let windows take care of it.
	}
}


