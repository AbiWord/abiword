/* AbiWord
 * Copyright (C) 2002 Jordi Mas i Hern�ndez <jmas@softcatala.org>
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

#define BUFSIZE		128

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
: AP_Dialog_InsertTable (pDlgFactory, id), m_hwndDlg(NULL)
{
	
}
AP_Win32Dialog_InsertTable::~AP_Win32Dialog_InsertTable()
{
}


void AP_Win32Dialog_InsertTable::runModal(XAP_Frame *pFrame)
{
	UT_ASSERT(pFrame);
	UT_ASSERT(m_id == AP_DIALOG_ID_INSERT_TABLE);
	
	// raise the dialog
	setDialog(this);
	createModal(pFrame, MAKEINTRESOURCE(AP_RID_DIALOG_INSERT_TABLE));

}

/*****************************************************************/

#define GWL(hwnd)		(AP_Win32Dialog_InsertTable*)GetWindowLong((hwnd), DWL_USER)
#define SWL(hwnd, d)	(AP_Win32Dialog_InsertTable*)SetWindowLong((hwnd), DWL_USER,(LONG)(d))


BOOL CALLBACK AP_Win32Dialog_InsertTable::s_dlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
		
	AP_Win32Dialog_InsertTable * pThis;
	
	switch (msg)
	{
	case WM_INITDIALOG:
		pThis = (AP_Win32Dialog_InsertTable *)lParam;
		SWL(hWnd,lParam);
		return pThis->_onInitDialog(hWnd,wParam,lParam);

	case WM_NOTIFY:
		pThis = GWL(hWnd);
		switch (((LPNMHDR)lParam)->code)
		{
			case UDN_DELTAPOS:		return pThis->_onDeltaPos((NM_UPDOWN *)lParam);
			default:				return 0;
		}
		
	case WM_COMMAND:
		pThis = GWL(hWnd);
		return pThis->_onCommand(hWnd,wParam,lParam);
	default:
		return 0;
	}
}


// This handles the WM_INITDIALOG message for the top-level dialog.
BOOL AP_Win32Dialog_InsertTable::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{	
	char 	szValue[BUFSIZE];	
	m_hwndDlg = hWnd;
		
		
	// localize controls
	localizeControlText(AP_RID_DIALOG_INSERTTABLE_BTN_OK,		XAP_STRING_ID_DLG_OK);
	localizeControlText(AP_RID_DIALOG_INSERTTABLE_BTN_CANCEL,	XAP_STRING_ID_DLG_Cancel);
	localizeControlText(AP_RID_DIALOG_INSERTTABLE_TEXT_SIZE,	AP_STRING_ID_DLG_InsertTable_TableSize);
	localizeControlText(AP_RID_DIALOG_INSERTTABLE_TEXT_COLUMN,	AP_STRING_ID_DLG_InsertTable_NumCols);
	localizeControlText(AP_RID_DIALOG_INSERTTABLE_TEXT_ROW,		AP_STRING_ID_DLG_InsertTable_NumRows);
	localizeControlText(AP_RID_DIALOG_INSERTTABLE_RADIO_AUTO,	AP_STRING_ID_DLG_InsertTable_AutoColSize);
	localizeControlText(AP_RID_DIALOG_INSERTTABLE_RADIO_FIXED,	AP_STRING_ID_DLG_InsertTable_FixedColSize);
		
	// Localise caption
	localizeDialogTitle(AP_STRING_ID_DLG_InsertTable_TableTitle);

	// Set Spin range (TODO: check if the max value is correct, copied from the unix version)
	SendMessage(GetDlgItem(hWnd,AP_RID_DIALOG_INSERTTABLE_SPIN_COLUMN),UDM_SETRANGE,(WPARAM)1,(WPARAM)9999);
	SendMessage(GetDlgItem(hWnd,AP_RID_DIALOG_INSERTTABLE_SPIN_ROW),UDM_SETRANGE,(WPARAM)1,(WPARAM)9999);
	SendMessage(GetDlgItem(hWnd,AP_RID_DIALOG_INSERTTABLE_SPIN_SIZE),UDM_SETRANGE,(WPARAM)1,(WPARAM)9999);
	
	// Limit to four chars
	SendMessage(GetDlgItem(hWnd,AP_RID_DIALOG_INSERTTABLE_TEXT_COLUMN),EM_LIMITTEXT,(WPARAM)5,(WPARAM)0);
	SendMessage(GetDlgItem(hWnd,AP_RID_DIALOG_INSERTTABLE_TEXT_ROW),EM_LIMITTEXT,(WPARAM)5,(WPARAM)0);
	SendMessage(GetDlgItem(hWnd,AP_RID_DIALOG_INSERTTABLE_TEXT_SIZE),EM_LIMITTEXT,(WPARAM)5,(WPARAM)0);

	CheckRadioButton(hWnd, AP_RID_DIALOG_INSERTTABLE_RADIO_AUTO,
		AP_RID_DIALOG_INSERTTABLE_RADIO_FIXED, AP_RID_DIALOG_INSERTTABLE_RADIO_AUTO);
	
	
	SetFocus(GetDlgItem(hWnd,AP_RID_DIALOG_INSERTTABLE_VAL_COLUMN));
	SendDlgItemMessage(hWnd, AP_RID_DIALOG_INSERTTABLE_VAL_COLUMN, EM_SETSEL, 0, -1);
	
	// Set default values		
	sprintf(szValue, "%u", getNumCols());
	setControlText(AP_RID_DIALOG_INSERTTABLE_VAL_COLUMN, szValue);
		
	sprintf(szValue, "%u", getNumRows());
	setControlText(AP_RID_DIALOG_INSERTTABLE_VAL_ROW, szValue);
				
	sprintf(szValue, "%02.2f", getColumnWidth());
	setControlText(AP_RID_DIALOG_INSERTTABLE_VAL_SIZE, szValue);

	/* Units name*/
	setControlText(AP_RID_DIALOG_INSERTTABLE_TEXT_UNITS, UT_dimensionName(m_dim));	

	/* Autosize by default*/
	EnableWindow(GetDlgItem(m_hwndDlg,AP_RID_DIALOG_INSERTTABLE_VAL_SIZE), FALSE);
	EnableWindow(GetDlgItem(m_hwndDlg,AP_RID_DIALOG_INSERTTABLE_SPIN_SIZE), FALSE);

	//XAP_Win32DialogHelper::s_centerDialog(hWnd);
	centerDialog();	
	
	return 0; // 0 because we called SetFocus
}

//
// Gets the values from the spin controls
//
void AP_Win32Dialog_InsertTable::getCtrlValues(void)
{	
	char 	szValue[BUFSIZE];	
		
	if (getControlText(AP_RID_DIALOG_INSERTTABLE_VAL_COLUMN, szValue, BUFSIZE ))	
		m_numCols = atoi(szValue);
	
	if (getControlText(AP_RID_DIALOG_INSERTTABLE_VAL_ROW, szValue, BUFSIZE ))	
		m_numRows = atoi(szValue);
		
	if (getControlText(AP_RID_DIALOG_INSERTTABLE_VAL_SIZE, szValue, BUFSIZE ))	
		setColumnWidth((float) atof(szValue));
	
	if (IsDlgButtonChecked(m_hwndDlg, AP_RID_DIALOG_INSERTTABLE_RADIO_AUTO))
		m_columnType = AP_Dialog_InsertTable::b_AUTOSIZE;
		
	if (IsDlgButtonChecked(m_hwndDlg, AP_RID_DIALOG_INSERTTABLE_RADIO_FIXED))
		m_columnType = AP_Dialog_InsertTable::b_FIXEDSIZE;
}

BOOL AP_Win32Dialog_InsertTable::_onDeltaPos(NM_UPDOWN * pnmud)
{
	UT_DEBUGMSG(("onDeltaPos: [idFrom %d][iPos %d][iDelta %d]\n",
				 pnmud->hdr.idFrom,pnmud->iPos,pnmud->iDelta));
				 
	char szBuff[256]="";			 					 
	double dValue = 0;
	
	if (getControlText(AP_RID_DIALOG_INSERTTABLE_VAL_SIZE, szBuff, 255 ))	
		dValue = (float) atof(szBuff);
												 
	dValue = atof(szBuff);						 
	
	_doSpin((0 - (UT_sint32) (-1) *pnmud->iDelta), dValue);
	
	sprintf (szBuff, "%02.2f", dValue);
	SetWindowTextA (GetDlgItem(m_hwndDlg,AP_RID_DIALOG_INSERTTABLE_VAL_SIZE), szBuff); //!TODO Using ANSI function
	
	return TRUE;	
}

BOOL AP_Win32Dialog_InsertTable::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;

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
			BOOL bEnable = IsDlgButtonChecked(m_hwndDlg, AP_RID_DIALOG_INSERTTABLE_RADIO_FIXED);

			EnableWindow(GetDlgItem(m_hwndDlg,AP_RID_DIALOG_INSERTTABLE_VAL_SIZE), bEnable);
			EnableWindow(GetDlgItem(m_hwndDlg,AP_RID_DIALOG_INSERTTABLE_SPIN_SIZE), bEnable);
			return 1;
		}
			

		default:							// we did not handle this notification
			UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
			return 0;						// return zero to let windows take care of it.
	}	
	
}

