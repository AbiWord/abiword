/* AbiWord
 * Copyright (C) 2002 Jordi Mas i Hernàndez <jmas@softcatala.org>
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

#include <stdio.h>
#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>

#include "ut_types.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_Win32OS.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"

#include "ap_Win32App.h"
#include "ap_Win32Frame.h"
#include "ap_Strings.h"
#include "ap_Dialog_Id.h"

#include "ap_Win32Dialog_InsertTable.h"
#include "ap_Win32Resources.rc2"
#include "xap_Win32DialogHelper.h"

#define BUFSIZE		64

/*****************************************************************/

XAP_Dialog* AP_Win32Dialog_InsertTable::static_constructor(XAP_DialogFactory* pDlgFactory, XAP_Dialog_Id id)
{
	AP_Win32Dialog_InsertTable* dlg = new AP_Win32Dialog_InsertTable (pDlgFactory, id);
	return dlg;
}

//
// Init
//
AP_Win32Dialog_InsertTable::AP_Win32Dialog_InsertTable (XAP_DialogFactory *pDlgFactory, XAP_Dialog_Id id)
: AP_Dialog_InsertTable (pDlgFactory, id)
{
	
}
AP_Win32Dialog_InsertTable::~AP_Win32Dialog_InsertTable()
{
}




 void AP_Win32Dialog_InsertTable::runModal(XAP_Frame *pFrame)
 {
    createModal(pFrame, MAKEINTRESOURCEW(AP_RID_DIALOG_INSERT_TABLE));
 }


/*****************************************************************/

#define _DS(c,s)	setDlgItemText(AP_RID_DIALOG_INSERTTABLE_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	setDlgItemText(AP_RID_DIALOG_INSERTTABLE_##c,pSS->getValue(XAP_STRING_ID_##s))

// This handles the WM_INITDIALOG message for the top-level dialog.
BOOL AP_Win32Dialog_InsertTable::_onInitDialog(HWND hWnd, WPARAM /*wParam*/, LPARAM /*lParam*/)
{	
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	wchar_t 	szValue[BUFSIZE];	
		
		
	// localize controls 
	_DSX(BTN_OK,		DLG_OK);
	_DSX(BTN_CANCEL,	DLG_Cancel);			
	_DS(TEXT_SIZE,		DLG_InsertTable_TableSize);		
	_DS(TEXT_COLUMN,	DLG_InsertTable_NumCols);		
	_DS(TEXT_ROW,		DLG_InsertTable_NumRows);		
	_DS(TEXT_AUTOFIT,	DLG_InsertTable_AutoFit);		
	_DS(RADIO_AUTO,		DLG_InsertTable_AutoColSize);		
	_DS(RADIO_FIXED,	DLG_InsertTable_FixedColSize);			
		
	// Localise caption
	setDialogTitle (pSS->getValue(AP_STRING_ID_DLG_InsertTable_TableTitle));

	// Set Spin range (TODO: check if the max value is correct, copied from the unix version)
	SendMessageW(GetDlgItem(hWnd,AP_RID_DIALOG_INSERTTABLE_SPIN_COLUMN),UDM_SETRANGE,(WPARAM)0,(LPARAM)MAKELONG((short)64,(short)1));
	SendMessageW(GetDlgItem(hWnd,AP_RID_DIALOG_INSERTTABLE_SPIN_ROW),UDM_SETRANGE,(WPARAM)0,(LPARAM)MAKELONG((short)500,(short)1));
	SendMessageW(GetDlgItem(hWnd,AP_RID_DIALOG_INSERTTABLE_SPIN_SIZE),UDM_SETRANGE,(WPARAM)0,(LPARAM)MAKELONG((short)9999,(short)1));
	
	// Limit to four chars
	SendMessageW(GetDlgItem(hWnd,AP_RID_DIALOG_INSERTTABLE_TEXT_COLUMN),EM_LIMITTEXT,(WPARAM)5,(WPARAM)0);
	SendMessageW(GetDlgItem(hWnd,AP_RID_DIALOG_INSERTTABLE_TEXT_ROW),EM_LIMITTEXT,(WPARAM)5,(WPARAM)0);
	SendMessageW(GetDlgItem(hWnd,AP_RID_DIALOG_INSERTTABLE_TEXT_SIZE),EM_LIMITTEXT,(WPARAM)5,(WPARAM)0);

	CheckRadioButton(hWnd, AP_RID_DIALOG_INSERTTABLE_RADIO_AUTO,
		AP_RID_DIALOG_INSERTTABLE_RADIO_FIXED, AP_RID_DIALOG_INSERTTABLE_RADIO_AUTO);
	
	
	SetFocus(GetDlgItem(hWnd,AP_RID_DIALOG_INSERTTABLE_VAL_COLUMN));
	SendDlgItemMessageW(hWnd, AP_RID_DIALOG_INSERTTABLE_VAL_COLUMN, EM_SETSEL, 0, -1);
	
	// Set default values		
	swprintf(szValue, L"%u", getNumCols());
	SetDlgItemTextW(m_hDlg, AP_RID_DIALOG_INSERTTABLE_VAL_COLUMN, szValue);
		
	swprintf(szValue, L"%u", getNumRows());
	SetDlgItemTextW(m_hDlg, AP_RID_DIALOG_INSERTTABLE_VAL_ROW, szValue);
				
	swprintf(szValue, L"%02.2f", getColumnWidth());
	SetDlgItemTextW(m_hDlg, AP_RID_DIALOG_INSERTTABLE_VAL_SIZE, szValue);

	/* Units name*/
	setDlgItemText(AP_RID_DIALOG_INSERTTABLE_TEXT_UNITS, UT_dimensionName(m_dim));	

	/* Autosize by default*/
	EnableWindow(GetDlgItem(m_hDlg,AP_RID_DIALOG_INSERTTABLE_VAL_SIZE), FALSE);
	EnableWindow(GetDlgItem(m_hDlg,AP_RID_DIALOG_INSERTTABLE_SPIN_SIZE), FALSE);

	centerDialog();	
	
	return 0; // 0 because we called SetFocus
}

float atofw (const WCHAR *st)
{
	UT_Win32LocaleString str;
	str.fromLocale (st);
	return atof (str.utf8_str().utf8_str());
}



//
// Gets the values from the spin controls
//
void AP_Win32Dialog_InsertTable::getCtrlValues(void)
{	
	UT_Win32LocaleString str;
		
	if (getDlgItemText(AP_RID_DIALOG_INSERTTABLE_VAL_COLUMN, str))
		m_numCols = _wtoi(str.c_str ());
	
	if (getDlgItemText(AP_RID_DIALOG_INSERTTABLE_VAL_ROW, str))
		m_numRows = _wtoi(str.c_str ());
		
	if (getDlgItemText(AP_RID_DIALOG_INSERTTABLE_VAL_SIZE, str))
		setColumnWidth((float) atofw(str.c_str ()));
	
	if (IsDlgButtonChecked(m_hDlg, AP_RID_DIALOG_INSERTTABLE_RADIO_AUTO))
		m_columnType = AP_Dialog_InsertTable::b_AUTOSIZE;
		
	if (IsDlgButtonChecked(m_hDlg, AP_RID_DIALOG_INSERTTABLE_RADIO_FIXED))
		m_columnType = AP_Dialog_InsertTable::b_FIXEDSIZE;
}

BOOL AP_Win32Dialog_InsertTable::_onDeltaPos(NM_UPDOWN * pnmud)
{
	if(pnmud->hdr.idFrom!=AP_RID_DIALOG_INSERTTABLE_SPIN_SIZE)
		return FALSE;

	UT_DEBUGMSG(("onDeltaPos: [idFrom %d][iPos %d][iDelta %d]\n",
				 pnmud->hdr.idFrom,pnmud->iPos,pnmud->iDelta));
				 
	UT_Win32LocaleString str;
	wchar_t szBuff[255]; 					 
	double dValue = 0;
	
	if (GetDlgItemTextW(m_hDlg, AP_RID_DIALOG_INSERTTABLE_VAL_SIZE, szBuff, 255 ))	
		dValue = (float) atofw(szBuff);				 
	
	_doSpin((0 - (UT_sint32) (-1) *pnmud->iDelta), dValue);
	
	swprintf (szBuff, L"%02.2f", dValue);
	SetWindowTextW (GetDlgItem(m_hDlg,AP_RID_DIALOG_INSERTTABLE_VAL_SIZE), szBuff);
	
	return TRUE;	
}

BOOL AP_Win32Dialog_InsertTable::_onCommand(HWND hWnd, WPARAM wParam, LPARAM /*lParam*/)
{
	WORD wId = LOWORD(wParam);

	switch (wId)
	{				
		case IDCANCEL:						
			m_answer = a_CANCEL;
			EndDialog(hWnd,0);
			return 1;
				
		case IDOK:		
			m_answer = a_OK;
			getCtrlValues();		
			EndDialog(hWnd,0);
			return 1;

		case AP_RID_DIALOG_INSERTTABLE_RADIO_FIXED:
		case AP_RID_DIALOG_INSERTTABLE_RADIO_AUTO:
		{
			BOOL bEnable = IsDlgButtonChecked(m_hDlg, AP_RID_DIALOG_INSERTTABLE_RADIO_FIXED);

			EnableWindow(GetDlgItem(m_hDlg, AP_RID_DIALOG_INSERTTABLE_VAL_SIZE), bEnable);
			EnableWindow(GetDlgItem(m_hDlg, AP_RID_DIALOG_INSERTTABLE_SPIN_SIZE), bEnable);
			return 1;
		}
			

		default:							// we did not handle this notification
			UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
			return 0;						// return zero to let windows take care of it.
	}	
	
}

