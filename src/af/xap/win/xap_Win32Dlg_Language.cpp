/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * 			 (c) 2002 Jordi Mas i Hernàndez	
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


#include "xap_Strings.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_Language.h"
#include "xap_Win32Dlg_Language.h"
#include "xap_Win32DialogHelper.h"
#include "xap_Win32Resources.rc2"
#include "xap_Win32Toolbar_Icons.h"
#include "ap_Win32App.h"

#ifdef STRICT   
#define WHICHPROC	WNDPROC
#else   
#define WHICHPROC	FARPROC
#endif
 
WHICHPROC gTreeProc;
#define _DS(c,s)	SetDlgItemText(hWnd,XAP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))


/*****************************************************************/

XAP_Dialog * XAP_Win32Dialog_Language::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	XAP_Win32Dialog_Language * p = new XAP_Win32Dialog_Language(pFactory,id);
	return p;
}

XAP_Win32Dialog_Language::XAP_Win32Dialog_Language(XAP_DialogFactory * pDlgFactory,
										 XAP_Dialog_Id id)
	: XAP_Dialog_Language(pDlgFactory,id)
{
	m_hNormIml =NULL;
}

XAP_Win32Dialog_Language::~XAP_Win32Dialog_Language(void)
{
	if (m_hNormIml)
		 ImageList_Destroy(m_hNormIml);
}

void XAP_Win32Dialog_Language::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);

	XAP_Win32App * pWin32App = static_cast<XAP_Win32App *>(m_pApp);
	UT_return_if_fail(pWin32App);
	
	LPCTSTR lpTemplate = NULL;

	UT_ASSERT(m_id == XAP_DIALOG_ID_LANGUAGE);

	lpTemplate = MAKEINTRESOURCE(XAP_RID_DIALOG_LANGUAGE);

	int result = DialogBoxParam(pWin32App->getInstance(),
						lpTemplate,
						static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow(),
						(DLGPROC)s_dlgProc,
						(LPARAM)this );
	UT_ASSERT((result != -1));
}

/*
	
*/
BOOL CALLBACK XAP_Win32Dialog_Language::s_dlgProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	// This is a static function.
	XAP_Win32Dialog_Language * pThis;
	
	switch (msg)
	{			
	case WM_INITDIALOG:
		pThis = (XAP_Win32Dialog_Language *)lParam;
		SetWindowLong(hWnd,DWL_USER,lParam);
		return pThis->_onInitDialog(hWnd,wParam,lParam);
		
	case WM_COMMAND:
		pThis = (XAP_Win32Dialog_Language *)GetWindowLong(hWnd,DWL_USER);
		return pThis->_onCommand(hWnd,wParam,lParam);
		
	default:
		return 0;
	}
}


/*
	Fills up the tree with the languages names
*/
void  XAP_Win32Dialog_Language::_fillTreeview(HWND hTV) 
{
	TV_INSERTSTRUCT tvins;
	TV_ITEM tvi;
	HTREEITEM hItem;
	HTREEITEM hSel = NULL;	
	
	UT_Vector* pVec = getAvailableDictionaries();		
	if (!pVec)
		return; // occurs when you compile without ENABLE_SPELL

	tvins.hParent = NULL;
	tvins.hInsertAfter = TVI_LAST;  	
	tvi.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE| TVIF_PARAM;               
	tvi.stateMask =0;
			  
	for (UT_uint32 i=0; i < m_iLangCount;  i++ )
	{
		const gchar* sLang = m_ppLanguages[i];
		const UT_uint32 nItems = pVec->getItemCount();	
		
		tvi.iImage = 0;    
		tvi.iSelectedImage = tvi.iImage;
			
		//Loop over all the languages for which we have a spell checker
		for (UT_uint32 iItem = nItems; iItem; --iItem)
		{
			const char* pText  = (const char*) pVec->getNthItem(iItem - 1);							
			if (strcmp(pText,  m_ppLanguagesCode[i])==0)
			{				
				tvi.iImage = 1;    
				tvi.iSelectedImage = tvi.iImage;
				break;
			}
		}		
		tvi.pszText = (char *)sLang;
		tvi.cchTextMax = lstrlen(sLang);				
		tvi.lParam=i;
		tvins.item = tvi;
		hItem = TreeView_InsertItem(hTV, &tvins);
		
		if (strcmp(m_pLanguage, sLang)==0)
			hSel = hItem;	
	}
	
	::SendMessage(hTV, TVM_SELECTITEM, TVGN_CARET,  (LONG)hSel);	
	delete pVec;
}

/*

*/
BOOL CALLBACK XAP_Win32Dialog_Language::s_treeProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{		
	XAP_Win32Dialog_Language *pThis;
	
	// The user has double clicked on a tree item (a language name)
	if (msg==WM_LBUTTONDBLCLK)
	{
		pThis = (XAP_Win32Dialog_Language *)GetWindowLong(hWnd,GWL_USERDATA);
		TVITEM tvi;		
		
		// Selected item
		tvi.hItem =  (HTREEITEM)::SendMessage(hWnd, TVM_GETNEXTITEM, TVGN_CARET, 0);
				
		if (tvi.hItem)
		{
			// Associated data		 
			tvi.mask = TVIF_PARAM;
			SendMessage(hWnd, TVM_GETITEM, 0, (LPARAM)&tvi);
			pThis->_setLanguageAndExit(tvi.lParam);			
		}
		return 1;						
	}	
		
	return CallWindowProc(gTreeProc,  hWnd, msg, wParam, lParam);
}

/*

*/
void XAP_Win32Dialog_Language::_setLanguageAndExit(UINT nLang)
{
	_setLanguage(m_ppLanguages[nLang]);
	m_bChangedLanguage = true;
	m_answer = a_OK;
	EndDialog(m_hWnd,0);
}

BOOL XAP_Win32Dialog_Language::_onInitDialog(HWND hWnd, WPARAM /*wParam*/, LPARAM /*lParam*/)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	
	
	m_hWnd = hWnd;
	
	SetWindowText(hWnd, pSS->getValue(XAP_STRING_ID_DLG_ULANG_LangTitle));

	// localize controls
	_DS(LANGUAGE_BTN_OK,			DLG_OK);
	_DS(LANGUAGE_BTN_CANCEL,		DLG_Cancel);
	_DS(LANGUAGE_FRM_LANGUAGE,      DLG_ULANG_LangLabel);
		
	HWND hTree = GetDlgItem(hWnd, XAP_RID_DIALOG_LANGUAGE_TREE_LANGUAGE);  		
	DWORD dwColor = GetSysColor(COLOR_WINDOW);	
	UT_RGBColor Color(GetRValue(dwColor),GetGValue(dwColor),GetBValue(dwColor));
	HBITMAP hBitmap = NULL, hBitmapTrans = NULL;
	
	/* create image lists, fill, attach to Treeviews */
	m_hNormIml = ImageList_Create(20, 20,  ILC_COLORDDB, 2, 2);    		              	       	
   	AP_Win32Toolbar_Icons::getBitmapForIcon(hWnd, 20,20, &Color, "SPELLCHECK",  &hBitmap);       		       	
   	AP_Win32Toolbar_Icons::getBitmapForIcon(hWnd, 20,20, &Color, "TRANSPARENTLANG",  &hBitmapTrans);       		
	
	/* Setup tree images */
	ImageList_Add(m_hNormIml,hBitmapTrans, NULL);		
	ImageList_Add(m_hNormIml, hBitmap, NULL);		
	DeleteObject(hBitmap);
	
	SendMessage(hTree, TVM_SETIMAGELIST, TVSIL_NORMAL, (LPARAM)m_hNormIml);		
	SendMessage(hTree, TVM_SETIMAGELIST, TVSIL_STATE, (LPARAM)m_hNormIml);
	
	_fillTreeview(hTree);
	
	gTreeProc = (WHICHPROC) GetWindowLong(hTree, GWL_WNDPROC);
	SetWindowLong(hTree, GWL_WNDPROC, (LONG)s_treeProc);
	SetWindowLong(hTree, GWL_USERDATA, (LONG)this);
	
	CheckDlgButton(hWnd, XAP_RID_DIALOG_LANGUAGE_DOCLANG_CHKBOX, BST_UNCHECKED );

	UT_UTF8String s;
	getDocDefaultLangDescription(s);
	SetDlgItemText(hWnd, XAP_RID_DIALOG_LANGUAGE_DOCLANG_STATIC,
				   AP_Win32App::s_fromUTF8ToWinLocale(s.utf8_str()).c_str());

	getDocDefaultLangCheckboxLabel(s);
	SetDlgItemText(hWnd, XAP_RID_DIALOG_LANGUAGE_DOCLANG_CHKBOX,
				   AP_Win32App::s_fromUTF8ToWinLocale(s.utf8_str()).c_str());
	
	XAP_Win32DialogHelper::s_centerDialog(hWnd);	
			
	return 1;							// 1 == we did not call SetFocus()
}

BOOL XAP_Win32Dialog_Language::_onCommand(HWND hWnd, WPARAM wParam, LPARAM /*lParam*/)
{
	WORD wId = LOWORD(wParam);

	switch (wId)
	{

		case IDCANCEL:						// also XAP_RID_DIALOG_LANGUAGE_BTN_CANCEL
			m_answer = a_CANCEL;
			EndDialog(hWnd,0);
			return 1;

		case IDOK:							// also XAP_RID_DIALOG_LANGUAGE_BTN_OK
			{
				TVITEM tvi;		
				HWND hWndTree = GetDlgItem(hWnd, XAP_RID_DIALOG_LANGUAGE_TREE_LANGUAGE);
		
				// Selected item
				tvi.hItem =  (HTREEITEM)::SendMessage(hWndTree, TVM_GETNEXTITEM, TVGN_CARET, 0);
		
				if (tvi.hItem)
				{				
					// Associated data		 
					tvi.mask = TVIF_PARAM;
					SendMessage(hWndTree, TVM_GETITEM, 0, (LPARAM)&tvi);		
					_setLanguage( m_ppLanguages[tvi.lParam]);
					m_bChangedLanguage = true;
					m_answer = a_OK;
				}
				else
				{
					m_answer = a_CANCEL;
				}	

				EndDialog(hWnd,0);
				return 1;
			}

		case XAP_RID_DIALOG_LANGUAGE_DOCLANG_CHKBOX:
		{
			bool b = (IsDlgButtonChecked(hWnd,XAP_RID_DIALOG_LANGUAGE_DOCLANG_CHKBOX)==BST_CHECKED);
			setMakeDocumentDefault(b);
			return 1;
		}
			
		default:							// we did not handle this notification
			UT_DEBUGMSG(("WM_Command for id %ld\n",wId));
			return 0;						// return zero to let windows take care of it.
	}

	return 0;	
}

