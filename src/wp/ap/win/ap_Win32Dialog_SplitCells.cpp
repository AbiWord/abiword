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

#include <stdlib.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"
#include "xap_Win32Toolbar_Icons.h"
#include "xap_Win32DialogHelper.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_SplitCells.h"
#include "ap_Win32Dialog_SplitCells.h"
#include "ap_Win32Resources.rc2"


#define BITMAP_WITDH	50
#define BITMAP_HEIGHT	50


XAP_Dialog * AP_Win32Dialog_SplitCells::static_constructor(XAP_DialogFactory * pFactory,
													       XAP_Dialog_Id id)
{
	AP_Win32Dialog_SplitCells * p = new AP_Win32Dialog_SplitCells(pFactory,id);
	return p;
}

AP_Win32Dialog_SplitCells::AP_Win32Dialog_SplitCells(XAP_DialogFactory * pDlgFactory,
										             XAP_Dialog_Id id)
	: AP_Dialog_SplitCells(pDlgFactory,id)
{		
	m_hBitmapAbove = NULL;
	m_hBitmapHoriMid = NULL;
	m_hBitmapBelow = NULL;
	m_hBitmapLeft = NULL;
	m_hBitmapVertMid = NULL;
	m_hBitmapRight = NULL;
}   
    
AP_Win32Dialog_SplitCells::~AP_Win32Dialog_SplitCells(void)
{	
	if (m_hBitmapAbove) DeleteObject(m_hBitmapAbove);
	if (m_hBitmapHoriMid) DeleteObject(m_hBitmapHoriMid);
	if (m_hBitmapBelow) DeleteObject(m_hBitmapBelow);
	if (m_hBitmapLeft) DeleteObject(m_hBitmapLeft);
	if (m_hBitmapVertMid) DeleteObject(m_hBitmapVertMid);
	if (m_hBitmapRight) DeleteObject(m_hBitmapRight);
}

void AP_Win32Dialog_SplitCells::runModeless(XAP_Frame * pFrame)
{
	UT_return_if_fail (pFrame);
	UT_return_if_fail (m_id == AP_DIALOG_ID_SPLIT_CELLS);

	createModeless(pFrame, MAKEINTRESOURCEW(AP_RID_DIALOG_SPLITCELLS));

	// Save dialog the ID number and pointer to the widget
	UT_sint32 sid =(UT_sint32)  getDialogId();
	m_pApp->rememberModelessId( sid, (XAP_Dialog_Modeless *) m_pDialog);

	startUpdater();
}


#define _DS(c,s)	setDlgItemText(AP_RID_DIALOG_SPLITCELLS_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	setDlgItemText(AP_RID_DIALOG_SPLITCELLS_##c,pSS->getValue(XAP_STRING_ID_##s))


// This handles the WM_INITDIALOG message for the top-level dialog.
BOOL AP_Win32Dialog_SplitCells::_onInitDialog(HWND hWnd, WPARAM /*wParam*/, LPARAM /*lParam*/)
{	
	RECT rect;
	DWORD dwColor = GetSysColor(COLOR_BTNFACE);	
	UT_RGBColor Color(GetRValue(dwColor),GetGValue(dwColor),GetBValue(dwColor));
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	// Localise caption
	setDialogTitle (pSS->getValue(AP_STRING_ID_DLG_SplitCellsTitle));


	// localise controls 		
	_DS(TEXT_LEFT,		DLG_SplitCells_Left);
	_DS(TEXT_VERTMID,	DLG_SplitCells_VertMid);
	_DS(TEXT_RIGHT,		DLG_SplitCells_Right);		
	_DS(TEXT_ABOVE,		DLG_SplitCells_Above);
	_DS(TEXT_HORIMID,	DLG_SplitCells_HoriMid);
	_DS(TEXT_BELOW,		DLG_SplitCells_Below);		
	_DS(TEXT_FRAME,		DLG_SplitCells_Frame);		
	_DSX(BTN_CANCEL,	DLG_Close);				
				

	// The six items are the same size
	GetClientRect(GetDlgItem(hWnd, AP_RID_DIALOG_SPLITCELLS_BMP_LEFT), &rect);			

	// Load the bitmaps into the dialog box								
	m_hBitmapLeft = XAP_Win32DialogHelper::s_loadBitmap(hWnd,AP_RID_DIALOG_SPLITCELLS_BMP_LEFT, "SPLITLEFT",  BITMAP_WITDH, BITMAP_HEIGHT, Color);
	m_hBitmapRight = XAP_Win32DialogHelper::s_loadBitmap(hWnd,AP_RID_DIALOG_SPLITCELLS_BMP_VERTMID, "SPLITVERTMID", BITMAP_WITDH, BITMAP_HEIGHT, Color);
	m_hBitmapRight = XAP_Win32DialogHelper::s_loadBitmap(hWnd,AP_RID_DIALOG_SPLITCELLS_BMP_RIGHT, "SPLITRIGHT", BITMAP_WITDH, BITMAP_HEIGHT, Color);
	m_hBitmapAbove = XAP_Win32DialogHelper::s_loadBitmap(hWnd,AP_RID_DIALOG_SPLITCELLS_BMP_ABOVE, "SPLITABOVE", BITMAP_WITDH, BITMAP_HEIGHT, Color);
	m_hBitmapAbove = XAP_Win32DialogHelper::s_loadBitmap(hWnd,AP_RID_DIALOG_SPLITCELLS_BMP_HORIMID, "SPLITHORIMID", BITMAP_WITDH, BITMAP_HEIGHT, Color);
	m_hBitmapBelow = XAP_Win32DialogHelper::s_loadBitmap(hWnd,AP_RID_DIALOG_SPLITCELLS_BMP_BELOW, "SPLITBELOW", BITMAP_WITDH, BITMAP_HEIGHT, Color);

	setAllSensitivities();
	centerDialog();	
	
	SetFocus(GetDlgItem(hWnd,AP_RID_DIALOG_SPLITCELLS_BTN_CANCEL));
	return 0; // 0 because we called SetFocus
}



void AP_Win32Dialog_SplitCells::setSensitivity(AP_CellSplitType splitThis, bool bSens)
{
	switch(splitThis)
	{
	case hori_left:
		EnableWindow(GetDlgItem(m_hDlg, AP_RID_DIALOG_SPLITCELLS_BMP_LEFT), bSens);
		EnableWindow(GetDlgItem(m_hDlg, AP_RID_DIALOG_SPLITCELLS_TEXT_LEFT), bSens);
		break;
	case hori_mid:
		EnableWindow(GetDlgItem(m_hDlg, AP_RID_DIALOG_SPLITCELLS_BMP_HORIMID), bSens);
		EnableWindow(GetDlgItem(m_hDlg, AP_RID_DIALOG_SPLITCELLS_TEXT_HORIMID), bSens);
		break;
	case hori_right:
		EnableWindow(GetDlgItem(m_hDlg, AP_RID_DIALOG_SPLITCELLS_BMP_RIGHT), bSens);
		EnableWindow(GetDlgItem(m_hDlg, AP_RID_DIALOG_SPLITCELLS_TEXT_RIGHT), bSens);
		break;	
	case vert_above:
		EnableWindow(GetDlgItem(m_hDlg, AP_RID_DIALOG_SPLITCELLS_BMP_ABOVE), bSens);
		EnableWindow(GetDlgItem(m_hDlg, AP_RID_DIALOG_SPLITCELLS_TEXT_ABOVE), bSens);
		break;
	case vert_mid:
		EnableWindow(GetDlgItem(m_hDlg, AP_RID_DIALOG_SPLITCELLS_BMP_VERTMID), bSens);
		EnableWindow(GetDlgItem(m_hDlg, AP_RID_DIALOG_SPLITCELLS_TEXT_VERTMID), bSens);
		break;
	case vert_below:
		EnableWindow(GetDlgItem(m_hDlg, AP_RID_DIALOG_SPLITCELLS_BMP_BELOW), bSens);
		EnableWindow(GetDlgItem(m_hDlg, AP_RID_DIALOG_SPLITCELLS_TEXT_BELOW), bSens);
		break;		
	default:
		break;
	}

}

void AP_Win32Dialog_SplitCells::event_Close(void)
{
	m_answer = a_CANCEL;
	destroy();
}


void AP_Win32Dialog_SplitCells::notifyActiveFrame(XAP_Frame *pFrame)
{
	if((HWND)GetWindowLongPtrW(m_hDlg, GWLP_HWNDPARENT) != static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow())
	{
		ConstructWindowName();
		setDialogTitle (m_WindowName);

		SetWindowLongPtrW(m_hDlg, GWLP_HWNDPARENT, (LONG_PTR)static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow());
		SetWindowPos(m_hDlg, NULL, 0, 0, 0, 0,
						SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}
	setAllSensitivities();
}

BOOL AP_Win32Dialog_SplitCells::_onCommand(HWND /*hWnd*/, WPARAM wParam, LPARAM /*lParam*/)
{
	WORD wId = LOWORD(wParam);

	switch (wId)
	{
		case AP_RID_DIALOG_SPLITCELLS_BMP_LEFT:		
		{	
			setSplitType(hori_left);
			onSplit();
			return 1;
		}
		case AP_RID_DIALOG_SPLITCELLS_BMP_HORIMID:		
		{	
			setSplitType(hori_mid);
			onSplit();
			return 1;
		}		
		case AP_RID_DIALOG_SPLITCELLS_BMP_RIGHT:		
		{	
			setSplitType(hori_right);
			onSplit();
			return 1;
		}
		
		case AP_RID_DIALOG_SPLITCELLS_BMP_ABOVE:
		{
			setSplitType(vert_above);
			onSplit();
			return 1;
		}
		case AP_RID_DIALOG_SPLITCELLS_BMP_VERTMID:
		{
			setSplitType(vert_mid);
			onSplit();
			return 1;
		}
		case AP_RID_DIALOG_SPLITCELLS_BMP_BELOW:		
		{	
			setSplitType(vert_below);
			onSplit();
			return 1;
		}
		
		case AP_RID_DIALOG_SPLITCELLS_BTN_CANCEL:						
			m_answer = a_CANCEL; 						
			destroy();
			return 1;
			
		default:							// we did not handle this notification
			UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
			return 0;						// return zero to let windows take care of it.
	}
}



void AP_Win32Dialog_SplitCells::destroy(void)
{
	finalize();

	UT_DebugOnly<int> iResult = DestroyWindow( m_hDlg );
	UT_ASSERT_HARMLESS((iResult != 0));
}
void AP_Win32Dialog_SplitCells::activate(void)
{
	ConstructWindowName();
	setDialogTitle (m_WindowName);

	setAllSensitivities();
}
