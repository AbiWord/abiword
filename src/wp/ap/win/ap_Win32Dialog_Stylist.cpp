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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include <windows.h>

#include "ut_std_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_Win32LocaleString.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"
#include "xap_Win32DialogBase.h"

#include "ap_Strings.h"
#include "ap_Dialog_Id.h"
#include "ap_Dialog_Stylist.h"
#include "ap_Win32Dialog_Stylist.h"
#include "xap_Win32DialogHelper.h"
#include "ap_Win32App.h"
#include "ap_Win32Resources.rc2"
#include "pt_PieceTable.h"

#if defined(STRICT)
#define WHICHPROC	WNDPROC
#else
#define WHICHPROC	FARPROC
#endif

WHICHPROC hTreeProc;

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
	
	UT_return_if_fail(m_id == AP_DIALOG_ID_STYLIST);
	m_bIsModal = true;

    createModal (pFrame, MAKEINTRESOURCEW(AP_RID_DIALOG_STYLIST));
}

void AP_Win32Dialog_Stylist::runModeless(XAP_Frame * pFrame)
{
	// raise the dialog
	UT_DebugOnly<int> iResult;
	m_bIsModal = false;
	UT_return_if_fail (m_id == AP_DIALOG_ID_STYLIST);
	createModeless (pFrame, MAKEINTRESOURCEW(AP_RID_DIALOG_STYLIST));

	// Save dialog the ID number and pointer to the widget
	UT_sint32 sid =(UT_sint32)  getDialogId();
	m_pApp->rememberModelessId( sid, (XAP_Dialog_Modeless *) m_pDialog);
	iResult = ShowWindow( m_hDlg, SW_SHOW );

	iResult = BringWindowToTop( m_hDlg );

	UT_ASSERT_HARMLESS((iResult != 0));
}


void  AP_Win32Dialog_Stylist::destroy(void)
{
	if (!m_bIsModal)
	{
		UT_DebugOnly<int> iResult = DestroyWindow( m_hDlg );

		UT_ASSERT_HARMLESS((iResult != 0));

		modeless_cleanup();
	}
	else
		EndDialog(m_hDlg,0);
	
}


void  AP_Win32Dialog_Stylist::setStyleInGUI(void)
{
	UT_sint32 row,col;
	std::string sCurStyle = getCurStyle();

	if((getStyleTree() == NULL) || (sCurStyle.size() == 0))
		updateDialog();

	if(isStyleTreeChanged())
		_fillTree();

	getStyleTree()->findStyle(sCurStyle,row,col);
	UT_DEBUGMSG(("After findStyle row %d col %d col \n",row,col));
	std::string sPathFull = UT_std_string_sprintf("%d:%d",row,col);
	std::string sPathRow = UT_std_string_sprintf("%d",row);
	UT_DEBUGMSG(("Full Path string is %s \n",sPathFull.c_str()));

	HWND hTree = GetDlgItem(m_hDlg, AP_RID_DIALOG_STYLIST_TREE_STYLIST);
	HTREEITEM hitem = NULL;

	hitem = TreeView_GetRoot(hTree);
	UT_sint32 i;
	// Go through each row until we've found ours
	for (i = 0; i < row; i++)
		hitem = TreeView_GetNextItem(hTree, hitem, TVGN_NEXT);

	hitem = TreeView_GetNextItem(hTree, hitem, TVGN_CHILD);
	for (i = 0; i < col; i++)
		hitem = TreeView_GetNextItem(hTree, hitem, TVGN_NEXT);

	TreeView_SelectItem(hTree, hitem);

	setStyleChanged(false);
}

void  AP_Win32Dialog_Stylist::activate(void)
{
	UT_DebugOnly<int> iResult;

	//// Update the caption
	//ConstructWindowName();
	//SetWindowText(m_hDlg, m_WindowName);

	iResult = ShowWindow( m_hDlg, SW_SHOW );

	iResult = BringWindowToTop( m_hDlg );

	UT_ASSERT_HARMLESS((iResult != 0));
}


void AP_Win32Dialog_Stylist::notifyActiveFrame(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);
	setStyleInGUI();
}

#define _DS(c,s)	setDlgItemText(AP_RID_DIALOG_STYLIST_##c,pSS->getValue(AP_STRING_ID_##s))
#define _DSX(c,s)	setDlgItemText(AP_RID_DIALOG_STYLIST_##c,pSS->getValue(XAP_STRING_ID_##s))

BOOL AP_Win32Dialog_Stylist::_onInitDialog(HWND /*hWnd*/, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
    //localize Caption
  	
	setDialogTitle(pSS->getValue(AP_STRING_ID_DLG_Stylist_Title));	
			
	// localize controls
	_DSX(BTN_OK,		DLG_OK);
	_DSX(BTN_CANCEL,	DLG_Close);

	_DS(TEXT_STYLES,	DLG_Stylist_Styles);

	_populateWindowData();

	HWND hTree = GetDlgItem(m_hDlg, AP_RID_DIALOG_STYLIST_TREE_STYLIST);
	hTreeProc = (WHICHPROC) GetWindowLongPtrW(hTree, GWLP_WNDPROC); // save off our prior callback
	SetWindowLongPtrW(hTree, GWLP_WNDPROC, (LONG_PTR)s_treeProc); // tie the treeview to the new callback
	SetWindowLongPtrW(hTree, GWLP_USERDATA, (LONG_PTR)this);
	centerDialog();

	return 1;							// 1 == we did not call SetFocus()
}

BOOL AP_Win32Dialog_Stylist::_onCommand(HWND /*hWnd*/, WPARAM wParam, LPARAM /*lParam*/)
{
	WORD wId = LOWORD(wParam);

	switch (wId)
	{
		case IDOK: {
			setStyleValid(true);
			if (_styleClicked())
				Apply();
			}
			destroy();
			return 0;

		case IDCANCEL:						// also AP_RID_DIALOG_STYLIST_BTN_CLOSE
//			m_answer = a_CANCEL;
			setStyleValid(false);
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

	HWND hTree = GetDlgItem(m_hDlg, AP_RID_DIALOG_STYLIST_TREE_STYLIST);

	// Purge any existing TreeView items
	TreeView_DeleteAllItems(hTree);

	TV_ITEMW tvi;
	TV_INSERTSTRUCTW tvins;
    HTREEITEM hParentItem; // Parent handle to link Styles to their Heading

	tvi.mask = TVIF_TEXT | TVIF_PARAM | TVIF_CHILDREN;               
	tvi.stateMask =0;

	UT_sint32 row, col;
	std::string sTmp, str_loc;
    
    UT_Win32LocaleString str;
	//int iter = 0; // Unique key for each item in the treeview
	for(row= 0; row < pStyleTree->getNumRows(); row++)
	{
		if(!pStyleTree->getNameOfRow(sTmp,row))
		{
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
			break;
		}
		
		pt_PieceTable::s_getLocalisedStyleName (sTmp.c_str(), str_loc);
		str.fromUTF8(str_loc.c_str());

		xxx_UT_DEBUGMSG(("Adding Heading %s at row %d \n",sTmp.c_str(),row));

		// Insert the item into the treeview
		tvi.pszText = (LPWSTR)str.c_str();
		tvi.cchTextMax = str.length() + 1;
		tvi.lParam = row;
		if (pStyleTree->getNumCols(row) > 0)
			tvi.cChildren = 1;
		else
			tvi.cChildren = 0;

		tvins.item = tvi;
		tvins.hParent = TVI_ROOT;
		tvins.hInsertAfter = TVI_LAST;
		
		hParentItem = (HTREEITEM)SendMessageW(hTree, TVM_INSERTITEMW, 0, (LPARAM)&tvins);

		// Add any children (columns) this row contains to be added
		if (pStyleTree->getNumCols(row) > 0)
		{
			for(col = 0; col < pStyleTree->getNumCols(row); col++)
			{
				if(!pStyleTree->getStyleAtRowCol(sTmp,row,col))
				{
					UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
					break;
				}
				xxx_UT_DEBUGMSG(("Adding style %s at row %d col %d \n",sTmp.c_str(),row,col+1));

				pt_PieceTable::s_getLocalisedStyleName (sTmp.c_str(), str_loc);
				str.fromUTF8(str_loc.c_str());

				// Insert the item into the treeview
				tvi.pszText = (LPWSTR)str.c_str();
				tvi.cchTextMax = str.length() + 1;
				tvi.cChildren = 0;
				tvi.lParam = col;

				tvins.item = tvi;
				tvins.hParent = hParentItem;
				tvins.hInsertAfter = TVI_LAST;

				//TreeView_InsertItem(hTree, &tvins);
				SendMessageW(hTree, TVM_INSERTITEMW, 0, (LPARAM)&tvins);
			}
		}
	}

	setStyleTreeChanged(false);
}

BOOL CALLBACK AP_Win32Dialog_Stylist::s_treeProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{		
	if (msg == WM_LBUTTONDBLCLK)
	{
		// The user has double clicked on a tree item
		AP_Win32Dialog_Stylist * pThis = (AP_Win32Dialog_Stylist *)GetWindowLongPtrW(hWnd,GWLP_USERDATA);
		if (pThis->_styleClicked())
			pThis->Apply();
		return 1;
	}

	return CallWindowProcW(hTreeProc, hWnd, msg, wParam, lParam);
}

#ifndef UNICODE
#define UNICODE
#endif

BOOL AP_Win32Dialog_Stylist::_styleClicked(void)
{

	UT_sint32 row, col;
	TVITEMW tvi;

	HWND hTree = GetDlgItem(m_hDlg, AP_RID_DIALOG_STYLIST_TREE_STYLIST);

	// Selected item
	tvi.hItem =  TreeView_GetSelection(hTree);
			
	if (!tvi.hItem)
		return 0;

	// Associated data
	tvi.mask = TVIF_HANDLE | TVIF_CHILDREN;
	TreeView_GetItem(hTree, &tvi);

	// Retrieve the row/column information from the treeview
	// This maps back to the pStyleList's row&column identifiers
	if (TreeView_GetParent(hTree, tvi.hItem) == NULL)
	{
		if (tvi.cChildren >= 1)
			return 0; // we've clicked on a style category, not a style

		row = tvi.lParam;
		col = 0;
	}
	else
	{
		col = tvi.lParam;
		// Get parent node for row information
		tvi.mask = TVIF_HANDLE;
		tvi.hItem = TreeView_GetParent(hTree, tvi.hItem);
		TreeView_GetItem(hTree, &tvi);
		row = tvi.lParam;
	}

	std::string sStyle;

	getStyleTree()->getStyleAtRowCol(sStyle,row,col);
	
	UT_DEBUGMSG(("StyleClicked row %d col %d style %s \n",row,col,sStyle.utf8_str()));
	setCurStyle(sStyle);
	return 1;
}

#undef UNICODE
