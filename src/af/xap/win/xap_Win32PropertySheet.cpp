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

#include <windows.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"
#include "xap_Win32PropertySheet.h"
#include "xap_Win32DialogHelper.h"


/*

	XAP_Win32PropertyPage
	
*/



XAP_Win32PropertyPage::XAP_Win32PropertyPage()
{
	m_pfnDlgProc = s_pageWndProc;	
	m_pParent = NULL;
}


int CALLBACK XAP_Win32PropertyPage::s_pageWndProc(HWND hWnd, UINT msg, WPARAM wParam,
   LPARAM lParam)
{	
	NMHDR* pNMHDR;
	
	switch(msg) 
	{
		case WM_INITDIALOG:
		{	
			PROPSHEETPAGE*	pStruct = (PROPSHEETPAGE*)lParam;  		
			XAP_Win32PropertyPage *pThis = (XAP_Win32PropertyPage *)pStruct->lParam;
			SetWindowLong(hWnd,DWL_USER,pStruct->lParam);
			pThis->m_hWnd = hWnd;
			pThis->_onInitDialog();
			return 0;
		}		

		case WM_NOTIFY:
		{
			pNMHDR = (NMHDR*)lParam;					
			if (pNMHDR->code==PSN_KILLACTIVE)
			{
				XAP_Win32PropertyPage *pThis = (XAP_Win32PropertyPage *)GetWindowLong(hWnd,DWL_USER);
				pThis->_onKillActive();
			}
			break;
		}		
		
		case WM_COMMAND:
		{
			XAP_Win32PropertyPage *pThis = (XAP_Win32PropertyPage *)GetWindowLong(hWnd,DWL_USER);
			pThis->_onCommand(hWnd, wParam, lParam);
			break;
		}				
						
		default:
			break;
	}  
        
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

void XAP_Win32PropertyPage::createPage(XAP_Win32App* pWin32App, WORD wRscID,
	XAP_String_Id	nID /* =0 */)
{	
	m_pWin32App = pWin32App;
	LPCTSTR lpTemplate = MAKEINTRESOURCE(wRscID);	
	const XAP_StringSet * pSS = getApp()->getStringSet();									
	
	m_page.pszTitle = pSS->getValue(nID);
	
	m_page.dwSize = sizeof(PROPSHEETPAGE);
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
		m_page.pszTitle = pSS->getValue(nID);
	}	
	else
		m_page.pszTitle = NULL;
    
	m_hdle = CreatePropertySheetPage(&m_page);
	
}

/*

	XAP_Win32PropertySheet
	
*/


XAP_Win32PropertySheet::XAP_Win32PropertySheet()
{	
	setApplyButton(false);
	m_lpfnDefSheet = NULL;
	m_pfnDlgProc = s_sheetWndProc;	
	m_pCallback = NULL;
}




/*

*/
int CALLBACK XAP_Win32PropertySheet::s_sheetWndProc(HWND hWnd, UINT msg, WPARAM wParam,  LPARAM lParam)
{			
	XAP_Win32PropertySheet *pThis = (XAP_Win32PropertySheet*)GetWindowLong(hWnd, GWL_USERDATA);			
	
	switch(msg) 
	{			
	    case WM_SYSCOMMAND:  
		{
			// Support the closing button
			if (wParam==SC_CLOSE)				    	
			{
               SendMessage (hWnd, WM_COMMAND, IDCANCEL, 0L);
               return 0;
            }			
		}
			
		case WM_DESTROY:
		{			 			
			PostQuitMessage(0);
			return 0;
		}

		case WM_COMMAND:
		{	
			if (!pThis->_onCommand(hWnd, wParam, lParam)) 	
				return 0; // Already processed							
			
			if (LOWORD(wParam)==IDOK)
			{
				pThis->m_nRslt=IDOK;				
				pThis->destroy();
				return 0;
			}				
				
			if (LOWORD(wParam)==IDCANCEL)
			{				
				pThis->m_nRslt=IDCANCEL;
				pThis->destroy();
				return 0;
			}			

			break;
		}				
						
		default:
			break;
	}  
	
	return CallWindowProc(pThis->m_lpfnDefSheet, hWnd, msg, wParam, lParam);       	
}


PROPSHEETPAGE* XAP_Win32PropertySheet::_buildPageArray()
{
			
	PROPSHEETPAGE *pArPages = (PROPSHEETPAGE *) new PROPSHEETPAGE[m_vecPages.getItemCount()];
	PROPSHEETPAGE *pCurPage;
	pCurPage = pArPages;
	XAP_Win32PropertyPage* pPage;
	
	UT_sint32 count = m_vecPages.getItemCount();
	UT_sint32 i= 0;
	for(i=0; i< count; pCurPage++, i++)
	{			
		pPage = (XAP_Win32PropertyPage*)m_vecPages.getNthItem(i);		
		memcpy (pCurPage,  pPage->getStruct(), sizeof(PROPSHEETPAGE));
	}
	
	return pArPages;	
}


void XAP_Win32PropertySheet::addPage(XAP_Win32PropertyPage* pPage)
{
	m_vecPages.addItem(pPage);
}



int XAP_Win32PropertySheet::runModal(XAP_Win32App* pWin32App, XAP_Frame * pFrame, XAP_String_Id nID/* = 0*/)
{		
	PROPSHEETPAGE*	pPages = _buildPageArray();
	const XAP_StringSet * pSS = pWin32App->getStringSet();
	MSG msg;	
	m_nRslt = IDCANCEL;
		
	memset (&m_psh, 0, sizeof(PROPSHEETHEADER));
		
	m_psh.dwSize = sizeof(PROPSHEETHEADER);
	m_psh.dwFlags = PSH_PROPSHEETPAGE;
	m_psh.hwndParent = static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow();
	m_psh.hInstance = pWin32App->getInstance();    
	m_psh.hIcon  = NULL;
	m_psh.pszIcon  = NULL;	
    m_psh.nPages = m_vecPages.getItemCount();
    m_psh.nStartPage = 0;
    m_psh.ppsp = (LPCPROPSHEETPAGE) pPages;
    m_psh.pfnCallback = m_pCallback;    
    
    if (m_pCallback)
     	m_psh.dwFlags |= PSH_USECALLBACK;    	
    
    if (!m_bApplyButton)
    	m_psh.dwFlags |= PSH_NOAPPLYNOW;
    
	if (nID)
		m_psh.pszCaption  = pSS->getValue(nID);    	
	else
		m_psh.pszCaption  = NULL;

	/*
		
	*/
	m_psh.dwFlags |= PSH_MODELESS;	
	m_hWnd = (HWND)::PropertySheet(&m_psh);	
	EnableWindow(m_psh.hwndParent, FALSE);

	/* Subclassing */
	m_lpfnDefSheet = (WHICHPROC)GetWindowLong(m_hWnd, GWL_WNDPROC);
	SetWindowLong(m_hWnd, GWL_USERDATA, (LONG)this);	
	SetWindowLong(m_hWnd, GWL_WNDPROC, (LONG)m_pfnDlgProc);
		
	_onInitDialog(m_hWnd);		

	while (GetMessage(&msg, NULL, 0, 0))
	{				
		if(m_hWnd && PropSheet_IsDialogMessage(m_hWnd, &msg))
			continue;				
					
		TranslateMessage(&msg);
		DispatchMessage(&msg);		
	}	
	
	destroy();
		
	delete pPages;    
	return m_nRslt;
}

/*

*/
void XAP_Win32PropertySheet::destroy(void)
{
	if (::IsWindow(m_hWnd))
	{	
		EnableWindow(m_psh.hwndParent, TRUE);
		SetActiveWindow(m_psh.hwndParent);
		SetFocus(m_psh.hwndParent);
		DestroyWindow(m_hWnd);
	}
	return;	
}

