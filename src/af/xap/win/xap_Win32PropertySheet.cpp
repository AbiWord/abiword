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

#include <windows.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"
#include "xap_Win32PropertySheet.h"
#include "xap_Win32DialogHelper.h"
#include "xav_View.h"
#include "xad_Document.h"
#include "ap_FrameData.h"
#include "ap_Dialog_Id.h"



/*

	XAP_Win32PropertyPage
	
*/



XAP_Win32PropertyPage::XAP_Win32PropertyPage()
{
	m_pfnDlgProc = s_pageWndProc;	
	m_pParent = NULL;
	m_hdle = NULL;
}

XAP_Win32PropertyPage::~XAP_Win32PropertyPage()
{
	if (m_hdle)
		DestroyPropertySheetPage(m_hdle);	
}

void XAP_Win32PropertyPage::setChanged (bool bChanged)
{
	HWND hWnd = GetParent(m_hDlg);
	SendMessageW(hWnd, bChanged ? PSM_CHANGED : PSM_UNCHANGED, (WPARAM)m_hDlg, 0);	
}

INT_PTR CALLBACK XAP_Win32PropertyPage::s_pageWndProc(HWND hWnd, UINT msg, WPARAM wParam,
   LPARAM lParam)
{	
	NMHDR* pNMHDR;
	
	switch(msg) 
	{
		case WM_INITDIALOG:
		{	
			PROPSHEETPAGEW*	pStruct = (PROPSHEETPAGEW*)lParam;  		
			XAP_Win32PropertyPage *pThis = (XAP_Win32PropertyPage *)pStruct->lParam;
			SetWindowLongPtrW(hWnd,DWLP_USER,pStruct->lParam);
			pThis->m_hDlg = hWnd;
			pThis->_onInitDialog();
			return 0;
		}		
		

		case WM_NOTIFY:
		{
			XAP_Win32PropertyPage *pThis = (XAP_Win32PropertyPage *)GetWindowLongPtrW(hWnd,DWLP_USER);

			if (pThis)
				pThis->_onNotify((LPNMHDR) lParam, wParam); 
			
			pNMHDR = (NMHDR*)lParam;					
			if (pNMHDR->code==PSN_KILLACTIVE)
			{
				pThis = (XAP_Win32PropertyPage *)GetWindowLongPtrW(hWnd,DWLP_USER);
				pThis->_onKillActive();
			}
			break;
		}		
		
		case WM_COMMAND:
		{
			XAP_Win32PropertyPage *pThis = (XAP_Win32PropertyPage *)GetWindowLongPtrW(hWnd,DWLP_USER);

			if (pThis)
				pThis->_onCommand(hWnd, wParam, lParam); 	

			return 0; // Already processed			
		}				
						
		default:
			break;
	}  
        
	return DefWindowProcW(hWnd, msg, wParam, lParam);
}


void XAP_Win32PropertyPage::createPage(XAP_Win32App* pWin32App, WORD wRscID,
	XAP_String_Id	nID /* =0 */)
{	
	m_pWin32App = pWin32App;
	LPCWSTR lpTemplate = MAKEINTRESOURCEW(wRscID);	
	const XAP_StringSet * pSS = getApp()->getStringSet();									

	m_page.dwSize = sizeof(PROPSHEETPAGEW);
	m_page.dwFlags = PSP_DEFAULT;
	m_page.hInstance = pWin32App->getInstance();
	m_page.hIcon  = NULL;
	m_page.pszIcon  = NULL;	
	m_page.pszTemplate = lpTemplate;
	m_page.pfnDlgProc = m_pfnDlgProc;
	m_page.lParam = (LPARAM) this;
	m_page.pfnCallback = NULL;
	m_page.pcRefParent  = NULL;
	
	if (nID)
	{
		m_page.dwFlags = m_page.dwFlags | PSP_USETITLE;
		m_title.fromUTF8 (pSS->getValue(nID));
		m_page.pszTitle = m_title.c_str();
	}	
	else
		m_page.pszTitle = NULL;
    
	m_hdle = CreatePropertySheetPageW(&m_page);	
	
}

/*

	XAP_Win32PropertySheet
	
*/


XAP_Win32PropertySheet::XAP_Win32PropertySheet()
{	
	setOkButton(false);
	setApplyButton(true);
	setCancelButton(true);
	m_lpfnDefSheet = NULL;
	m_pfnDlgProc = s_sheetWndProc;	
	m_pCallback = NULL;
	m_pages = NULL;
	m_modeless = false;
}



/*

*/
INT_PTR CALLBACK XAP_Win32PropertySheet::s_sheetWndProc(HWND hWnd, UINT msg, WPARAM wParam,  LPARAM lParam)
{			
	XAP_Win32PropertySheet *pThis = (XAP_Win32PropertySheet*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);			
	
	switch(msg) 
	{			
	    case WM_SYSCOMMAND:  
		{
			// Support the closing button
			if (wParam==SC_CLOSE)
			{
				if (!pThis->m_modeless)
				{
			   		SendMessageW (hWnd, WM_COMMAND, IDCANCEL, 0L);
			   		return 0;
			   	}
			   	else			   
				{				
					pThis->destroy();
					return 0;							
				}			
			}	            		
            break;
		}
			
		
		case WM_DESTROY:
		{		
			if (!pThis->m_modeless)
				PostQuitMessage(0);

			pThis->cleanup ();

			return 0;
		} 

		case WM_COMMAND:
		{	
			if (!pThis->_onCommand(hWnd, wParam, lParam)) 	
				return 0; // Already processed

			if (LOWORD(wParam)==ID_APPLY)
			{
				for(UT_sint32 i = 0; i < pThis->m_vecPages.getItemCount(); i++)
				{			
					XAP_Win32PropertyPage* pPage = (XAP_Win32PropertyPage*)pThis->m_vecPages.getNthItem(i);		
					pPage->_onApply();					
				}				

				pThis->_onApply();
				pThis->m_nRslt=ID_APPLY;
				return 0;
			}
			
			
			if (LOWORD(wParam)==IDOK)
			{
				for(UT_sint32 i = 0; i < pThis->m_vecPages.getItemCount(); i++)
				{			
					XAP_Win32PropertyPage* pPage = (XAP_Win32PropertyPage*)pThis->m_vecPages.getNthItem(i);		
					pPage->_onOK();					
				}				

				pThis->_onOK();
				pThis->m_nRslt=IDOK;

				if (!pThis->m_modeless)
					pThis->destroy();

				return 0;
			}				
				
			if (LOWORD(wParam)==IDCANCEL)
			{				
				pThis->_onCancel();
				pThis->m_nRslt=IDCANCEL;
				pThis->destroy();
				return 0;
			}			

			break;
		}				
						
		default:
			break;
	}  
	
	return CallWindowProcW(pThis->m_lpfnDefSheet, hWnd, msg, wParam, lParam);       	
}


PROPSHEETPAGEW* XAP_Win32PropertySheet::_buildPageArray()
{
			
	PROPSHEETPAGEW *pArPages = (PROPSHEETPAGEW *) new PROPSHEETPAGEW[m_vecPages.getItemCount()];
	PROPSHEETPAGEW *pCurPage;
	pCurPage = pArPages;
	XAP_Win32PropertyPage* pPage;
	
	UT_sint32 count = m_vecPages.getItemCount();
	UT_sint32 i= 0;
	for(i=0; i< count; pCurPage++, i++)
	{			
		pPage = (XAP_Win32PropertyPage*)m_vecPages.getNthItem(i);		
		memcpy (pCurPage,  pPage->getStruct(), sizeof(PROPSHEETPAGEW));
	}
	
	return pArPages;	
}


void XAP_Win32PropertySheet::addPage(XAP_Win32PropertyPage* pPage)
{
	m_vecPages.addItem(pPage);
}


int XAP_Win32PropertySheet::runModal(XAP_Win32App* pWin32App, XAP_Frame * pFrame, XAP_String_Id nID/* = 0*/)
{	
	MSG msg;	
    UT_Win32LocaleString title;
	m_pages = _buildPageArray();
	const XAP_StringSet * pSS = pWin32App->getStringSet();
		
	m_nRslt = IDCANCEL;
		
	memset (&m_psh, 0, sizeof(PROPSHEETHEADERW));		
		
	m_psh.dwSize = sizeof(PROPSHEETHEADERW);
	m_psh.dwFlags = PSH_PROPSHEETPAGE;
	m_psh.hwndParent = static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow();
	m_psh.hInstance = pWin32App->getInstance();    
	m_psh.hIcon  = NULL;
	m_psh.pszIcon  = NULL;	
    m_psh.nPages = m_vecPages.getItemCount();
    m_psh.nStartPage = 0;
    m_psh.ppsp = (LPCPROPSHEETPAGEW) m_pages;
    m_psh.pfnCallback = m_pCallback;    
    
    if (m_pCallback)
     	m_psh.dwFlags |= PSH_USECALLBACK;    	
    
    if (!m_bApplyButton)
    	m_psh.dwFlags |= PSH_NOAPPLYNOW;
    
	if (nID)
	{
		title.fromUTF8 (pSS->getValue(nID));
		m_psh.pszCaption  = title.c_str();
	}
	else
		m_psh.pszCaption  = NULL;

	/*
		
	*/
	m_psh.dwFlags |= PSH_MODELESS;	
	m_hWnd = (HWND)::PropertySheetW(&m_psh);	
	EnableWindow(m_psh.hwndParent, FALSE);

	/* Subclassing */
	m_lpfnDefSheet = (WHICHPROC)GetWindowLongPtrW(m_hWnd, GWLP_WNDPROC);
	SetWindowLongPtrW(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);	
	SetWindowLongPtrW(m_hWnd, GWLP_WNDPROC, (LONG_PTR)m_pfnDlgProc);
		
	_onInitDialog(m_hWnd);		

	while (GetMessageW(&msg, NULL, 0, 0))
	{				
		if(m_hWnd && PropSheet_IsDialogMessage(m_hWnd, &msg))
			continue;				
					
		TranslateMessage(&msg);
		DispatchMessageW(&msg);		
	}	
	
	destroy();	
	return m_nRslt;
}

int XAP_Win32PropertySheet::runModeless (XAP_Win32App* pWin32App, XAP_Frame * pFrame, XAP_String_Id nID/* = 0*/)
{		
	m_pages = _buildPageArray();
    UT_Win32LocaleString title;
	const XAP_StringSet * pSS = pWin32App->getStringSet();	
	m_nRslt = IDCANCEL;
	m_modeless = true;
		
	memset (&m_psh, 0, sizeof(PROPSHEETHEADERW));		
		
	m_psh.dwSize = sizeof(PROPSHEETHEADERW);
	m_psh.dwFlags = PSH_PROPSHEETPAGE;
	m_psh.hwndParent = static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow();
	m_psh.hInstance = pWin32App->getInstance();    
	m_psh.hIcon  = NULL;
	m_psh.pszIcon  = NULL;	
    m_psh.nPages = m_vecPages.getItemCount();
    m_psh.nStartPage = 0;
    m_psh.ppsp = (LPCPROPSHEETPAGEW) m_pages;
    m_psh.pfnCallback = m_pCallback;    
    
    if (m_pCallback)
    	m_psh.dwFlags |= PSH_USECALLBACK;    	
    
    if (!m_bApplyButton)
    	m_psh.dwFlags |= PSH_NOAPPLYNOW;
    
	if (nID)
		{
		title.fromUTF8 (pSS->getValue(nID));
		m_psh.pszCaption  = title.c_str();
	}  	
	else
		m_psh.pszCaption  = NULL;

	
	m_psh.dwFlags |= PSH_MODELESS;	
	m_hWnd = (HWND)::PropertySheetW(&m_psh);		

	if (m_bApplyButton) {		
		XAP_Win32DialogBase::setDlgItemText (m_hWnd, ID_APPLY, pSS->getValue(XAP_STRING_ID_DLG_Apply));	
	}
	XAP_Win32DialogBase::setDlgItemText (m_hWnd, IDCANCEL, pSS->getValue(XAP_STRING_ID_DLG_Cancel));

	/* Subclassing */

	m_lpfnDefSheet = (WHICHPROC)GetWindowLongPtrW(m_hWnd, GWLP_WNDPROC);
	SetWindowLongPtrW(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);	
	SetWindowLongPtrW(m_hWnd, GWLP_WNDPROC, (LONG_PTR)m_pfnDlgProc);
		
	_onInitDialog(m_hWnd);	
	XAP_Win32DialogHelper::s_centerDialog(m_hWnd);

	if (!m_bOkButton)
		ShowWindow (GetDlgItem (m_hWnd, IDOK), FALSE);

	if (!m_bCancelButton)
		ShowWindow (GetDlgItem (m_hWnd, IDCANCEL), FALSE);
	
	return 0;
}

/*

*/
void XAP_Win32PropertySheet::destroy(void)
{	
	if (::IsWindow(m_hWnd))
	{	
		if (!m_modeless)
			EnableWindow(m_psh.hwndParent, TRUE);		

		DestroyWindow(m_hWnd);
	}

	cleanup ();	
	
}

void XAP_Win32PropertySheet::cleanup(void)
{	
	if (m_pages) {
		delete m_pages;
		m_pages = NULL;
	}	
}
