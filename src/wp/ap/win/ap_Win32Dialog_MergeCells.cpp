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
#include "ap_Dialog_MergeCells.h"
#include "ap_Win32Dialog_MergeCells.h"
#include "ap_Win32Resources.rc2"

#define BITMAP_WITDH	32
#define BITMAP_HEIGHT	32

#define GWL(hwnd)		(AP_Win32Dialog_MergeCells*)GetWindowLong((hwnd), DWL_USER)
#define SWL(hwnd, d)	(AP_Win32Dialog_MergeCells*)SetWindowLong((hwnd), DWL_USER,(LONG)(d))


XAP_Dialog * AP_Win32Dialog_MergeCells::static_constructor(XAP_DialogFactory * pFactory,
													       XAP_Dialog_Id id)
{
	AP_Win32Dialog_MergeCells * p = new AP_Win32Dialog_MergeCells(pFactory,id);
	return p;
}

AP_Win32Dialog_MergeCells::AP_Win32Dialog_MergeCells(XAP_DialogFactory * pDlgFactory,
										             XAP_Dialog_Id id)
	: AP_Dialog_MergeCells(pDlgFactory,id)
{		
	m_hBitmapLeft = NULL; 
	m_hBitmapRight = NULL; 
	m_hBitmapAbove = NULL; 
	m_hBitmapBelow = NULL; 
}   
    
AP_Win32Dialog_MergeCells::~AP_Win32Dialog_MergeCells(void)
{	
	if (m_hBitmapRight) DeleteObject(m_hBitmapRight);		
	if (m_hBitmapAbove) DeleteObject(m_hBitmapAbove);		
	if (m_hBitmapLeft) DeleteObject(m_hBitmapLeft);		
	if (m_hBitmapBelow) DeleteObject(m_hBitmapBelow);		
}

void AP_Win32Dialog_MergeCells::runModeless(XAP_Frame * pFrame)
{
	// raise the dialog
	int iResult;
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);

	LPCTSTR lpTemplate = NULL;

	UT_return_if_fail (m_id == AP_DIALOG_ID_MERGE_CELLS);

	lpTemplate = MAKEINTRESOURCE(AP_RID_DIALOG_MERGECELLS);

	HWND hResult = CreateDialogParam(pWin32App->getInstance(),lpTemplate,
							static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow(),
							(DLGPROC)s_dlgProc,(LPARAM)this);
	UT_ASSERT_HARMLESS((hResult != NULL));

	m_hwndDlg = hResult;

	// Save dialog the ID number and pointer to the widget
	UT_sint32 sid =(UT_sint32)  getDialogId();
	m_pApp->rememberModelessId( sid, (XAP_Dialog_Modeless *) m_pDialog);

	iResult = ShowWindow( m_hwndDlg, SW_SHOW );

	iResult = BringWindowToTop( m_hwndDlg );

	startUpdater();

	UT_ASSERT_HARMLESS((iResult != 0));

}

BOOL CALLBACK AP_Win32Dialog_MergeCells::s_dlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{		
	AP_Win32Dialog_MergeCells * pThis;
	
	switch (msg)
	{
	case WM_INITDIALOG:
		pThis = (AP_Win32Dialog_MergeCells *)lParam;
		SWL(hWnd,lParam);
		return pThis->_onInitDialog(hWnd,wParam,lParam);

	case WM_COMMAND:
		pThis = GWL(hWnd);
		return pThis->_onCommand(hWnd,wParam,lParam);

	case WM_DESTROY:
		pThis = GWL(hWnd);
		if(pThis)
			pThis->destroy();
		return 0;

	default:
		return 0;
	}
}
#define _DS(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_MERGECELLS_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_MERGECELLS_##c,pSS->getValue(XAP_STRING_ID_##s))


HBITMAP AP_Win32Dialog_MergeCells::_loadBitmap(HWND hWnd, UINT nId, char* pName, int width, int height, UT_RGBColor color)
{
	HBITMAP hBitmap = NULL;
	
	AP_Win32Toolbar_Icons::getBitmapForIcon(hWnd, width,height, &color,	pName,	&hBitmap);	
				
	SendDlgItemMessage(hWnd,  nId, 
        	            BM_SETIMAGE,  IMAGE_BITMAP, (LPARAM) hBitmap);				
	
	return hBitmap; 
}

// This handles the WM_INITDIALOG message for the top-level dialog.
BOOL AP_Win32Dialog_MergeCells::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{	
	HDC hdc;
	int x, y;	
	RECT rect;
	DWORD dwColor = GetSysColor(COLOR_BTNFACE);	
	UT_RGBColor Color(GetRValue(dwColor),GetGValue(dwColor),GetBValue(dwColor));
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	m_hwndDlg = hWnd;

	// localise controls 		
	_DS(TEXT_LEFT,		DLG_MergeCells_Left);		
	_DS(TEXT_RIGHT,		DLG_MergeCells_Right);		
	_DS(TEXT_ABOVE,		DLG_MergeCells_Above);		
	_DS(TEXT_BELOW,		DLG_MergeCells_Below);		
	_DS(TEXT_FRAME,		DLG_MergeCells_Frame);		
	_DSX(BTN_CANCEL,	DLG_Close);				
				
	// Localise caption
	ConstructWindowName();
	SetWindowText(m_hwndDlg, m_WindowName);

	// The four items are the same size
	GetClientRect(GetDlgItem(hWnd, AP_RID_DIALOG_MERGECELLS_BMP_LEFT), &rect);			
		
	hdc = GetDC(hWnd);
	x = rect.right - rect.left,
	y = rect.bottom - rect.top,
	
	// Load the bitmaps into the dialog box								
    m_hBitmapLeft = _loadBitmap(hWnd,AP_RID_DIALOG_MERGECELLS_BMP_LEFT, "MERGELEFT",  BITMAP_WITDH, BITMAP_HEIGHT, Color);
    m_hBitmapRight = _loadBitmap(hWnd,AP_RID_DIALOG_MERGECELLS_BMP_RIGHT, "MERGERIGHT", BITMAP_WITDH, BITMAP_HEIGHT, Color);
    m_hBitmapAbove = _loadBitmap(hWnd,AP_RID_DIALOG_MERGECELLS_BMP_ABOVE, "MERGEABOVE", BITMAP_WITDH, BITMAP_HEIGHT, Color);
    m_hBitmapBelow = _loadBitmap(hWnd,AP_RID_DIALOG_MERGECELLS_BMP_BELOW, "MERGEBELOW", BITMAP_WITDH, BITMAP_HEIGHT, Color);
	
	setAllSensitivities();
	XAP_Win32DialogHelper::s_centerDialog(hWnd);	
	
	SetFocus(GetDlgItem(hWnd,AP_RID_DIALOG_MERGECELLS_BTN_CANCEL));
	return 0; // 0 because we called SetFocus
}



void AP_Win32Dialog_MergeCells::setSensitivity(AP_Dialog_MergeCells::mergeWithCell mergeThis, bool bSens)
{
	switch(mergeThis)
	{
	case AP_Dialog_MergeCells::radio_left:
		EnableWindow(GetDlgItem(m_hwndDlg, AP_RID_DIALOG_MERGECELLS_BMP_LEFT), bSens);
		EnableWindow(GetDlgItem(m_hwndDlg, AP_RID_DIALOG_MERGECELLS_TEXT_LEFT), bSens);
		break;
		
	case AP_Dialog_MergeCells::radio_right:
		EnableWindow(GetDlgItem(m_hwndDlg, AP_RID_DIALOG_MERGECELLS_BMP_RIGHT), bSens);
		EnableWindow(GetDlgItem(m_hwndDlg, AP_RID_DIALOG_MERGECELLS_TEXT_RIGHT), bSens);
		break;	
	case AP_Dialog_MergeCells::radio_above:
		EnableWindow(GetDlgItem(m_hwndDlg, AP_RID_DIALOG_MERGECELLS_BMP_ABOVE), bSens);
		EnableWindow(GetDlgItem(m_hwndDlg, AP_RID_DIALOG_MERGECELLS_TEXT_ABOVE), bSens);
		break;
	case AP_Dialog_MergeCells::radio_below:
		EnableWindow(GetDlgItem(m_hwndDlg, AP_RID_DIALOG_MERGECELLS_BMP_BELOW), bSens);
		EnableWindow(GetDlgItem(m_hwndDlg, AP_RID_DIALOG_MERGECELLS_TEXT_BELOW), bSens);
		break;		
	default:
		break;
	}

}

void AP_Win32Dialog_MergeCells::event_Close(void)
{
	m_answer = AP_Dialog_MergeCells::a_CANCEL;
	destroy();
}


void AP_Win32Dialog_MergeCells::notifyActiveFrame(XAP_Frame *pFrame)
{
	if((HWND)GetWindowLong(m_hwndDlg, GWL_HWNDPARENT) != static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow())
	{
		ConstructWindowName();
		SetWindowText(m_hwndDlg, m_WindowName);

		SetWindowLong(m_hwndDlg, GWL_HWNDPARENT, (long)static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow());
		SetWindowPos(m_hwndDlg, NULL, 0, 0, 0, 0,
						SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
	}
	setAllSensitivities();
}

BOOL AP_Win32Dialog_MergeCells::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wId = LOWORD(wParam);

	switch (wId)
	{
		case AP_RID_DIALOG_MERGECELLS_BMP_LEFT:		
		{	
			setMergeType(AP_Dialog_MergeCells::radio_left);
			onMerge();
			return 1;
		}
		
		case AP_RID_DIALOG_MERGECELLS_BMP_RIGHT:		
		{	
			setMergeType(AP_Dialog_MergeCells::radio_right);
			onMerge();
			return 1;
		}
		
		case AP_RID_DIALOG_MERGECELLS_BMP_ABOVE:		
		{	
			setMergeType(AP_Dialog_MergeCells::radio_above);
			onMerge();
			return 1;
		}
		case AP_RID_DIALOG_MERGECELLS_BMP_BELOW:		
		{	
			setMergeType(AP_Dialog_MergeCells::radio_below);
			onMerge();
			return 1;
		}		
		
		case AP_RID_DIALOG_MERGECELLS_BTN_CANCEL:						
		case IDCANCEL:		// We want to close button work
			m_answer = a_CANCEL; 						
			destroy();
			return 1;
			
		default:							// we did not handle this notification
			UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
			return 0;						// return zero to let windows take care of it.
	}
}



void AP_Win32Dialog_MergeCells::destroy(void)
{
	finalize();

	int iResult = DestroyWindow( m_hwndDlg );
	UT_ASSERT_HARMLESS((iResult != 0));
}
void AP_Win32Dialog_MergeCells::activate(void)
{
	ConstructWindowName();
	SetWindowText(m_hwndDlg, m_WindowName);

	setAllSensitivities();
}
