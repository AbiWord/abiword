/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
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

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Stylist.h"
#include "ap_Win32Dialog_Stylist.h"

#include "ap_Win32Resources.rc2"

/*****************************************************************/

XAP_Dialog * AP_Win32Dialog_Stylist::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	AP_Win32Dialog_Stylist * p = new AP_Win32Dialog_Stylist(pFactory,id);
	return p;
}

AP_Win32Dialog_Stylist::AP_Win32Dialog_Stylist(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: AP_Dialog_Stylist(pDlgFactory,id)
{
}

AP_Win32Dialog_Stylist::~AP_Win32Dialog_Stylist(void)
{
}


void AP_Win32Dialog_Stylist::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);
	UT_ASSERT_HARMLESS(0);
}

void AP_Win32Dialog_Stylist::runModeless(XAP_Frame * pFrame)
{
	// raise the dialog
	int iResult;
	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);

	LPCTSTR lpTemplate = NULL;

	UT_return_if_fail (m_id == AP_DIALOG_ID_STYLIST);

	lpTemplate = MAKEINTRESOURCE(AP_RID_DIALOG_STYLIST);

	HWND hResult = CreateDialogParam(pWin32App->getInstance(),lpTemplate,
							static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow(),
							(DLGPROC)s_dlgProc,(LPARAM)this);

	//if (hResult == NULL)
	//{
	//	CHAR szBuf[80]; 
	//	DWORD dw = GetLastError(); 
	// 
	//	sprintf(szBuf, "failed: GetLastError returned %u\n", dw); 
	// 
	//	MessageBox(NULL, szBuf, "Error", MB_OK); 
	//}

	UT_ASSERT_HARMLESS((hResult != NULL));

	m_hWnd = hResult;

	// Save dialog the ID number and pointer to the widget
	UT_sint32 sid =(UT_sint32)  getDialogId();
	m_pApp->rememberModelessId( sid, (XAP_Dialog_Modeless *) m_pDialog);

	iResult = ShowWindow( m_hWnd, SW_SHOW );

	iResult = BringWindowToTop( m_hWnd );

	UT_ASSERT_HARMLESS((iResult != 0));
}

BOOL CALLBACK AP_Win32Dialog_Stylist::s_dlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// This is a static function.

	AP_Win32Dialog_Stylist * pThis;
	
	switch (msg)
	{
		case WM_INITDIALOG:
			pThis = (AP_Win32Dialog_Stylist *)lParam;
			SetWindowLong(hWnd,DWL_USER,lParam);
			return pThis->_onInitDialog(hWnd,wParam,lParam);
			
		case WM_COMMAND:
			pThis = (AP_Win32Dialog_Stylist *)GetWindowLong(hWnd,DWL_USER);
			if (pThis)
				return pThis->_onCommand(hWnd,wParam,lParam);
			else
				return 0;
			
		default:
			return 0;
	}
}

void  AP_Win32Dialog_Stylist::destroy(void)
{
	int iResult = DestroyWindow( m_hWnd );

	UT_ASSERT_HARMLESS((iResult != 0));

	modeless_cleanup();
}


void  AP_Win32Dialog_Stylist::setStyleInGUI(void)
{
	UT_ASSERT_HARMLESS(0);
}

void  AP_Win32Dialog_Stylist::activate(void)
{
	int iResult;

	//// Update the caption
	//ConstructWindowName();
	//SetWindowText(m_hWnd, m_WindowName);

	iResult = ShowWindow( m_hWnd, SW_SHOW );

	iResult = BringWindowToTop( m_hWnd );

	UT_ASSERT_HARMLESS((iResult != 0));
}


void AP_Win32Dialog_Stylist::notifyActiveFrame(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);
}

#define _DS(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_STYLIST_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	SetDlgItemText(hWnd,AP_RID_DIALOG_STYLIST_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL AP_Win32Dialog_Stylist::_onInitDialog(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	m_hWnd = hWnd;
	
	// Localise caption
	SetWindowText(hWnd, pSS->getValue(AP_STRING_ID_DLG_Stylist_Title));	
			
	// localize controls
	_DSX(BTN_OK,		DLG_OK);
	_DSX(BTN_CANCEL,	DLG_Close);

	_DS(TEXT_STYLES,	DLG_Stylist_Styles);

	_populateWindowData();

	return 1;							// 1 == we did not call SetFocus()
}

BOOL AP_Win32Dialog_Stylist::_onCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	WORD wNotifyCode = HIWORD(wParam);
	WORD wId = LOWORD(wParam);
	HWND hWndCtrl = (HWND)lParam;
	XAP_Frame *	pFrame = getActiveFrame();

	switch (wId)
	{
		case IDOK:
			
		case IDCANCEL:						// also AP_RID_DIALOG_STYLIST_BTN_CLOSE
//			m_answer = a_CANCEL;
			destroy();
			return 1;
		
		default:							// we did not handle this notification
			UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
			return 0;						// return zero to let windows take care of it.
	}
}

void AP_Win32Dialog_Stylist::_populateWindowData(void)
{
	_fillTree();
	setStyleInGUI();
}

/*!
 * Fill the GUI tree with the styles as defined in the XP tree.
 */
void AP_Win32Dialog_Stylist::_fillTree(void)
{
	Stylist_tree * pStyleTree = getStyleTree();
	if(pStyleTree == NULL)
	{
		updateDialog();
		pStyleTree = getStyleTree();
	}
	if(pStyleTree->getNumRows() == 0)
	{
		updateDialog();
		pStyleTree = getStyleTree();
	}
	UT_DEBUGMSG(("Number of rows of styles in document %d \n",pStyleTree->getNumRows()));

	HWND hTree = GetDlgItem(m_hWnd, AP_RID_DIALOG_STYLIST_TREE_STYLIST);

	// Purge any existing TreeView items
	TreeView_DeleteAllItems(hTree);

	TV_ITEM tvi;
	TV_INSERTSTRUCT tvins;
    HTREEITEM hParentItem; // Parent handle to link Styles to their Heading

	tvi.mask = TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;               
	tvi.stateMask =0;

	UT_sint32 row, col;
	UT_UTF8String sTmp("");
	int iter = 0; // Unique key for each item in the treeview
	for(row= 0; row < pStyleTree->getNumRows(); row++)
	{
		if(!pStyleTree->getNameOfRow(sTmp,row))
		{
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			break;
		}
		if(pStyleTree->getNumCols(row) > 0)
		{
			xxx_UT_DEBUGMSG(("Adding Heading %s at row %d \n",sTmp.utf8_str(),row));

			// Insert the item into the treeview
			tvi.pszText = const_cast<char *>(sTmp.utf8_str());
			tvi.cchTextMax = sTmp.length() + 1;
			tvi.cChildren = 1;
			tvi.lParam = iter++;

			tvins.item = tvi;
			tvins.hParent = TVI_ROOT;
			tvins.hInsertAfter = TVI_LAST;
			
			hParentItem = TreeView_InsertItem(hTree, &tvins);

			for(col = 0; col < pStyleTree->getNumCols(row); col++)
			{
				if(!pStyleTree->getStyleAtRowCol(sTmp,row,col))
				{
					UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
					break;
				}
				xxx_UT_DEBUGMSG(("Adding style %s at row %d col %d \n",sTmp.utf8_str(),row,col+1));
				// Insert the item into the treeview
				tvi.pszText = const_cast<char *>(sTmp.utf8_str());
				tvi.cchTextMax = sTmp.length() + 1;
				tvi.cChildren = 0;
				tvi.lParam = iter++;

				tvins.item = tvi;
				tvins.hParent = hParentItem;
				tvins.hInsertAfter = TVI_LAST;

				TreeView_InsertItem(hTree, &tvins);
			}
		}
		else
		{
			xxx_UT_DEBUGMSG(("Adding style %s at row %d \n",sTmp.utf8_str(),row));
			// TODO: Add any special case single-style rows here.
		}
	}

	setStyleTreeChanged(false);
}
