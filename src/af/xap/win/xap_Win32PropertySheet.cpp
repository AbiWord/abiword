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
static XAP_Win32PropertySheet* gpThis = NULL;

XAP_Win32PropertySheet::XAP_Win32PropertySheet()
{	
	setApplyButton(false);
	m_lpfnDefSheet = NULL;
	m_pfnDlgProc = s_sheetWndProc;	
	gpThis = this;
}



int CALLBACK XAP_Win32PropertySheet::s_sheetInit(HWND hwnd,  UINT uMsg,  LPARAM lParam)
{
	if (uMsg==PSCB_INITIALIZED)
	{
		gpThis->_onInitDialog(hwnd);				
	
		if (gpThis->m_pfnDlgProc)
		{	
			// override the window procedure 			
			gpThis->m_lpfnDefSheet = (WHICHPROC)GetWindowLong(hwnd, GWL_WNDPROC);
			SetWindowLong(hwnd, GWL_WNDPROC, (LONG)gpThis->m_pfnDlgProc);
			SetWindowLong(hwnd, GWL_USERDATA, (LONG)gpThis);	
		}
	}		
	return 	0;
}

/*

*/
int CALLBACK XAP_Win32PropertySheet::s_sheetWndProc(HWND hWnd, UINT msg, WPARAM wParam,
   LPARAM lParam)
{		
	switch(msg) 
	{		
		case WM_COMMAND:
		{			
			gpThis->_onCommand(hWnd, wParam, lParam);
			break;
		}				
						
		default:
			break;
	}  
	
	return CallWindowProc(gpThis->m_lpfnDefSheet, hWnd, msg, wParam, lParam);       	
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
		
	memset (&m_psh, 0, sizeof(PROPSHEETHEADER));
		
	m_psh.dwSize = sizeof(PROPSHEETHEADER);
	m_psh.dwFlags = PSH_USECALLBACK| PSH_PROPSHEETPAGE;
	m_psh.hwndParent = static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow();
	m_psh.hInstance = pWin32App->getInstance();    
	m_psh.hIcon  = NULL;
	m_psh.pszIcon  = NULL;	
    m_psh.nPages = m_vecPages.getItemCount();
    m_psh.nStartPage = 0;
    m_psh.ppsp = (LPCPROPSHEETPAGE) pPages;
    m_psh.pfnCallback = s_sheetInit;    
    
    if (!m_bApplyButton)
    	m_psh.dwFlags |= PSH_NOAPPLYNOW;
    
	if (nID)
		m_psh.pszCaption  = pSS->getValue(nID);    	
	else
		m_psh.pszCaption  = NULL;

		
	int nRslt = PropertySheet(&m_psh);
    
	delete pPages;    
	return nRslt;
}


