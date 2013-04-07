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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
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
#define _DS(c,s)	setDlgItemText(XAP_RID_DIALOG_##c,pSS->getValue(XAP_STRING_ID_##s))


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
	UT_ASSERT(m_id == XAP_DIALOG_ID_LANGUAGE);
    createModal(pFrame, MAKEINTRESOURCEW(XAP_RID_DIALOG_LANGUAGE));    
}

/*
	Fills up the tree with the languages names
*/
void  XAP_Win32Dialog_Language::_fillTreeview(HWND hTV) 
{
	TV_INSERTSTRUCTW tvins;
	TV_ITEMW tvi;
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
		UT_Win32LocaleString str;
		str.fromUTF8 (sLang);
		tvi.pszText = (wchar_t *) str.c_str();
		tvi.cchTextMax = str.length();
		tvi.lParam=i;
		tvins.item = tvi;
		hItem = (HTREEITEM)SendMessageW(hTV, TVM_INSERTITEMW, 0, (LPARAM)&tvins);
		
		if (strcmp(m_pLanguage, sLang)==0)
			hSel = hItem;	
	}
	
	::SendMessageW(hTV, TVM_SELECTITEM, TVGN_CARET,  (LONG_PTR)hSel);	
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
		pThis = (XAP_Win32Dialog_Language *)GetWindowLongPtrW(hWnd,GWLP_USERDATA);
		TVITEM tvi;		
		
		// Selected item
		tvi.hItem =  (HTREEITEM)::SendMessageW(hWnd, TVM_GETNEXTITEM, TVGN_CARET, 0);
				
		if (tvi.hItem)
		{
			// Associated data		 
			tvi.mask = TVIF_PARAM;
			SendMessageW(hWnd, TVM_GETITEMW, 0, (LPARAM)&tvi);
			pThis->_setLanguageAndExit(tvi.lParam);			
		}
		return 1;						
	}	
		
	return CallWindowProcW(gTreeProc,  hWnd, msg, wParam, lParam);
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
	
	
	setDialogTitle (pSS->getValue(XAP_STRING_ID_DLG_ULANG_LangTitle));

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
   	XAP_Win32Toolbar_Icons::getBitmapForIcon(hWnd, 20,20, &Color, "SPELLCHECK",  &hBitmap);       		       	
   	XAP_Win32Toolbar_Icons::getBitmapForIcon(hWnd, 20,20, &Color, "TRANSPARENTLANG",  &hBitmapTrans);       		
	
	/* Setup tree images */
	ImageList_Add(m_hNormIml,hBitmapTrans, NULL);		
	ImageList_Add(m_hNormIml, hBitmap, NULL);		
	DeleteObject(hBitmap);
	
	SendMessageW(hTree, TVM_SETIMAGELIST, TVSIL_NORMAL, (LPARAM)m_hNormIml);		
	SendMessageW(hTree, TVM_SETIMAGELIST, TVSIL_STATE, (LPARAM)m_hNormIml);
	
	_fillTreeview(hTree);
	
	gTreeProc = (WHICHPROC) GetWindowLongPtrW(hTree, GWLP_WNDPROC);
	SetWindowLongPtrW(hTree, GWLP_WNDPROC, (LONG_PTR)s_treeProc);
	SetWindowLongPtrW(hTree, GWLP_USERDATA, (LONG_PTR)this);
	
	CheckDlgButton(hWnd, XAP_RID_DIALOG_LANGUAGE_DOCLANG_CHKBOX, BST_UNCHECKED );

	UT_UTF8String s;
	getDocDefaultLangDescription(s);
	setDlgItemText(XAP_RID_DIALOG_LANGUAGE_DOCLANG_STATIC, s.utf8_str());

	getDocDefaultLangCheckboxLabel(s);
	setDlgItemText(XAP_RID_DIALOG_LANGUAGE_DOCLANG_CHKBOX, s.utf8_str());
	
    centerDialog();
			
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
				TVITEMW tvi;		
				HWND hWndTree = GetDlgItem(hWnd, XAP_RID_DIALOG_LANGUAGE_TREE_LANGUAGE);
		
				// Selected item
				tvi.hItem =  (HTREEITEM)::SendMessageW(hWndTree, TVM_GETNEXTITEM, TVGN_CARET, 0);
		
				if (tvi.hItem)
				{				
					// Associated data		 
					tvi.mask = TVIF_PARAM;
					SendMessageW(hWndTree, TVM_GETITEMW, 0, (LPARAM)&tvi);		
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

