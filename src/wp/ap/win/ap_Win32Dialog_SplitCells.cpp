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
#include "ap_Dialog_SplitCells.h"
#include "ap_Win32Dialog_SplitCells.h"
#include "ap_Win32Resources.rc2"


#define GWL(hwnd)		(AP_Win32Dialog_SplitCells*)GetWindowLong((hwnd), DWL_USER)
#define SWL(hwnd, d)	(AP_Win32Dialog_SplitCells*)SetWindowLong((hwnd), DWL_USER,(LONG)(d))
#define DefineToolbarIcon(name)		{ #name, (const char **) name, sizeof(name)/sizeof(name[0]) },



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
	m_hBitmapLeft = NULL; 
	m_hBitmapRight = NULL; 
	m_hBitmapAbove = NULL; 
	m_hBitmapBelow = NULL; 
}   
    
AP_Win32Dialog_SplitCells::~AP_Win32Dialog_SplitCells(void)
{	
	if (m_hBitmapRight) DeleteObject(m_hBitmapRight);		
	if (m_hBitmapAbove) DeleteObject(m_hBitmapAbove);		
	if (m_hBitmapLeft) DeleteObject(m_hBitmapLeft);		
	if (m_hBitmapBelow) DeleteObject(m_hBitmapBelow);		
}

void AP_Win32Dialog_SplitCells::runModeless(XAP_Frame * pFrame)
{
	UT_ASSERT(pFrame);	
	
	// raise the dialog
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);

	LPCTSTR lpTemplate = NULL;

	UT_ASSERT(m_id == AP_DIALOG_ID_MERGE_CELLS);

	lpTemplate = MAKEINTRESOURCE(AP_RID_DIALOG_MERGECELLS);

	int result = DialogBoxParam(pWin32App->getInstance(),lpTemplate,
						static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow(),
						(DLGPROC)s_dlgProc,(LPARAM)this);
	UT_ASSERT((result != -1));
}

BOOL CALLBACK AP_Win32Dialog_SplitCells::s_dlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{		
	AP_Win32Dialog_SplitCells * pThis;
	
	switch (msg)
	{
	case WM_INITDIALOG:
		pThis = (AP_Win32Dialog_SplitCells *)lParam;
		SWL(hWnd,lParam);
		return pThis->_onInitDialog(hWnd,wParam,lParam);
		
	case WM_COMMAND:
		pThis = GWL(hWnd);
		return pThis->_onCommand(hWnd,wParam,lParam);
	default:
		return 0;
	}
}
#define _DS(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_MERGECELLS_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_MERGECELLS_##c,pSS->getValue(XAP_STRING_ID_##s))


HBITMAP AP_Win32Dialog_SplitCells::_loadBitmap(HWND hWnd, UINT nId, char* pName, int x, int y, UT_RGBColor color)
{
	HBITMAP hBitmap = NULL;
	
	AP_Win32Toolbar_Icons::getBitmapForIcon(hWnd, x,y, &color,	pName,	&hBitmap);	
				
	SendDlgItemMessage(hWnd,  nId, 
        	            BM_SETIMAGE,  IMAGE_BITMAP, (LPARAM) hBitmap);				
	
	return hBitmap; 
}

// This handles the WM_INITDIALOG message for the top-level dialog.
BOOL AP_Win32Dialog_SplitCells::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{	
	HDC hdc;
	int x, y;	
	RECT rect;
	DWORD dwColor = GetSysColor(COLOR_BTNFACE);	
	UT_RGBColor Color(GetRValue(dwColor),GetGValue(dwColor),GetBValue(dwColor));
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	m_hwndDlg = hWnd;	
				
	// localise controls 		

	_DSX(BTN_CANCEL,	DLG_Close);				
				
	// Localise caption
	SetWindowText(hWnd, pSS->getValue(AP_STRING_ID_DLG_SplitCellsTitle));	
	
	// The four items are the same size
	GetClientRect(GetDlgItem(hWnd, AP_RID_DIALOG_MERGECELLS_BMP_LEFT), &rect);			
		
	hdc = GetDC(hWnd);
	x = rect.right - rect.left,
	y = rect.bottom - rect.top,
	
	// Load the bitmaps into the dialog box								

	
	setAllSensitivities();
	XAP_Win32DialogHelper::s_centerDialog(hWnd);	
	
	SetFocus(GetDlgItem(hWnd,AP_RID_DIALOG_MERGECELLS_BTN_CANCEL));
	return 0; // 0 because we called SetFocus
}



void AP_Win32Dialog_SplitCells::setSensitivity(AP_CellSplitType splitThis, bool bSens)

{

}

void AP_Win32Dialog_SplitCells::event_Close(void)
{
	m_answer = AP_Dialog_SplitCells::a_CANCEL;
	destroy();
}


void AP_Win32Dialog_SplitCells::notifyActiveFrame(XAP_Frame *pFrame)
{
	ConstructWindowName();
	setAllSensitivities();
}

BOOL AP_Win32Dialog_SplitCells::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	return FALSE;
}



void AP_Win32Dialog_SplitCells::destroy(void)
{
	finalize();
	
}
void AP_Win32Dialog_SplitCells::activate(void)
{
	
        
	ConstructWindowName();	
	setAllSensitivities();
	
}
