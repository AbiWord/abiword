/* AbiWord
* Copyright (C) 1998-2000 AbiSource, Inc.
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
* 02110-1301 USA.
*/

#include <windows.h>
#include <time.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Field.h"
#include "ap_Win32Dialog_Field.h"
#include "xap_Win32DialogHelper.h"
#include "ap_Win32Resources.rc2"

/*****************************************************************/

AP_Win32Dialog_Field::AP_Win32Dialog_Field(XAP_DialogFactory * pDlgFactory,
										   XAP_Dialog_Id id)
										   : AP_Dialog_Field(pDlgFactory,id)
{
}

/*****************************************************************/

XAP_Dialog * AP_Win32Dialog_Field::static_constructor(XAP_DialogFactory * pFactory,
													  XAP_Dialog_Id id)
{
	AP_Win32Dialog_Field * p = new AP_Win32Dialog_Field(pFactory,id);
	return p;
}

/*****************************************************************/

void AP_Win32Dialog_Field::runModal(XAP_Frame * pFrame)
{
	// raise the dialog
	UT_return_if_fail (m_id == AP_DIALOG_ID_FIELD);
	
	createModal(pFrame, MAKEINTRESOURCEW(AP_RID_DIALOG_FIELD));	
}

void AP_Win32Dialog_Field::SetTypesList(void)
{
	UT_Win32LocaleString str;
	for (int i = 0;fp_FieldTypes[i].m_Desc != NULL;i++) 
	{
		str.fromUTF8(fp_FieldTypes[i].m_Desc);
		SendMessageW(m_hwndTypes, LB_ADDSTRING, (WPARAM)0, (LPARAM)str.c_str());
	}
	SendMessageW(m_hwndTypes, LB_SETCURSEL, (WPARAM)0, (LPARAM)0);
	m_iTypeIndex = 0;
}

void AP_Win32Dialog_Field::SetFieldsList(void)
{
	fp_FieldTypesEnum  FType = fp_FieldTypes[m_iTypeIndex].m_Type;

	SendMessageW(m_hwndFormats, LB_RESETCONTENT, 0, 0);
	int i;
	for (i = 0;fp_FieldFmts[i].m_Tag != NULL;i++) 
	{
		if( fp_FieldFmts[i].m_Type == FType )
			break;
	}

	UT_Win32LocaleString str;

	for (;fp_FieldFmts[i].m_Tag != NULL && fp_FieldFmts[i].m_Type == FType;i++) 
	{
		if((fp_FieldFmts[i].m_Num != FPFIELD_endnote_anch) &&
		   (fp_FieldFmts[i].m_Num != FPFIELD_endnote_ref) &&
		   (fp_FieldFmts[i].m_Num != FPFIELD_footnote_anch) &&
		   (fp_FieldFmts[i].m_Num != FPFIELD_footnote_ref))
		{ 
			str.fromUTF8(fp_FieldFmts[i].m_Desc);
			UT_sint32 index = SendMessageW(m_hwndFormats, LB_ADDSTRING, 0, (LPARAM)str.c_str());
			if (index != LB_ERR && index != LB_ERRSPACE)
			{
				SendMessageW(m_hwndFormats, LB_SETITEMDATA, (WPARAM)index, (LPARAM)i);
			}
		}
	}
	SendMessageW(m_hwndFormats, LB_SETCURSEL, 0, 0);
	_FormatListBoxChange();
}

#define _DS(c,s)	setDlgItemText(AP_RID_DIALOG_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	setDlgItemText(AP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL AP_Win32Dialog_Field::_onInitDialog(HWND hWnd, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	setDialogTitle (pSS->getValue(AP_STRING_ID_DLG_Field_FieldTitle));
	
	// localize controls
	_DSX(FIELD_BTN_OK,			DLG_OK);
	_DSX(FIELD_BTN_CANCEL,		DLG_Cancel);
	
	_DS(FIELD_TEXT_TYPES,		DLG_Field_Types);
	_DS(FIELD_TEXT_FORMATS, 	DLG_Field_Fields);
	_DS(FIELD_TEXT_PARAM,		DLG_Field_Parameters);

	// set initial state
	m_hwndTypes = GetDlgItem(hWnd, AP_RID_DIALOG_FIELD_LIST_TYPES);
	m_hwndFormats = GetDlgItem(hWnd, AP_RID_DIALOG_FIELD_LIST_FORMATS);
	m_hwndParam = GetDlgItem(hWnd, AP_RID_DIALOG_FIELD_EDIT_PARAM);
	SetTypesList();
	SetFieldsList();
	centerDialog();		
    
	return 1;				// 1 == we did not call SetFocus()
}

BOOL AP_Win32Dialog_Field::_onCommand(HWND hWnd, WPARAM wParam, LPARAM /*lParam*/)
{
	WORD wId = LOWORD(wParam);
	
	switch (wId)
	{
	case IDCANCEL:			// also AP_RID_DIALOG_FIELD_BTN_CANCEL
		m_answer = a_CANCEL;
		// fall through
		
	case IDOK:				// also AP_RID_DIALOG_FIELD_BTN_OK
		// set the extra param
		gchar param[256];
		GetWindowTextW(m_hwndParam,(LPWSTR) &param[0], 256);
		setParameter(param);
		EndDialog(hWnd,0);
		return 1;
	case AP_RID_DIALOG_FIELD_LIST_TYPES:
		switch (HIWORD(wParam))
		{
		case LBN_SELCHANGE:
			_FormatListBoxChange();
			SetFieldsList();
			return 1;
			
		default:
			return 0;
		}
		break;
	case AP_RID_DIALOG_FIELD_LIST_FORMATS:
		switch (HIWORD(wParam))
		{
		case LBN_SELCHANGE:
			_FormatListBoxChange();
			return 1;
			
		case LBN_DBLCLK:
			EndDialog(hWnd,0);
			return 1;
			
		default:
			return 0;
		}
		break;
	default:			// we did not handle this notification
		//UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
		return 0;			// return zero to let windows take care of it.
	}
}


void AP_Win32Dialog_Field::_FormatListBoxChange(void)
{
	m_iTypeIndex = SendMessageW(m_hwndTypes, LB_GETCURSEL, 0, 0);

	UT_sint32 index = SendMessageW(m_hwndFormats, LB_GETCURSEL, 0, 0);
	UT_sint32 tag = SendMessageW(m_hwndFormats, LB_GETITEMDATA, index, 0);
	UT_ASSERT(tag != LB_ERR);

	m_iFormatIndex = tag;
}

