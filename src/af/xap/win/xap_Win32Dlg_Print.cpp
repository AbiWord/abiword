/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
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
#include <stdio.h>
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "xap_Dialog_Id.h"
#include "xap_Win32Dlg_Print.h"
#include "xap_Win32App.h"
#include "xap_Win32FrameImpl.h"
#include "gr_Win32Graphics.h"

/*****************************************************************/

static UINT CALLBACK s_PrintHookProc(
  HWND hdlg,      // handle to the dialog box window
  UINT uiMsg,     // message identifier
  WPARAM wParam,  // message parameter
  LPARAM lParam   // message parameter
  )
{
	// the only thing we want to do at the moment is to hide the
	// selection radio if it is disabled
	if(uiMsg == WM_INITDIALOG)
	{
		PRINTDLG * pDlgInfo = (PRINTDLG*)lParam;
		if(pDlgInfo->Flags & PD_NOSELECTION)
		{
			HWND wPrintSelectionRadio = GetDlgItem(hdlg, rad2);
			ShowWindow(wPrintSelectionRadio, SW_HIDE);
		}
	}
	
	return 0;
}

XAP_Dialog * XAP_Win32Dialog_Print::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	XAP_Win32Dialog_Print * p = new XAP_Win32Dialog_Print(pFactory,id);
	return p;
}

XAP_Win32Dialog_Print::XAP_Win32Dialog_Print(XAP_DialogFactory * pDlgFactory,
										   XAP_Dialog_Id id)
	: XAP_Dialog_Print(pDlgFactory,id)
{
	m_pPersistPrintDlg = (PRINTDLG *)UT_calloc(1,sizeof(PRINTDLG));
	UT_ASSERT(m_pPersistPrintDlg);
	
	memset(m_pPersistPrintDlg,0,sizeof(m_pPersistPrintDlg));
	m_pPersistPrintDlg->lStructSize = sizeof(*m_pPersistPrintDlg);
}

XAP_Win32Dialog_Print::~XAP_Win32Dialog_Print(void)
{
	if (m_pPersistPrintDlg)
	{
		if (m_pPersistPrintDlg->hDevMode)
			GlobalFree(m_pPersistPrintDlg->hDevMode);
		if (m_pPersistPrintDlg->hDevNames)
			GlobalFree(m_pPersistPrintDlg->hDevNames);
		free(m_pPersistPrintDlg);
	}
}

GR_Graphics * XAP_Win32Dialog_Print::getPrinterGraphicsContext(void)
{
	UT_ASSERT(m_answer == a_OK);

	if (!m_pPersistPrintDlg->hDC) return NULL; /* Prevents from passing NULL to GR_Win32Graphics*/

	memset(&m_DocInfo,0,sizeof(m_DocInfo));
	m_DocInfo.cbSize = sizeof(DOCINFO);
	//m_DocInfo.lpszDocName = m_szDocumentPathname; //!TODO Using ANSI function !!!FIXME!!!
	//m_DocInfo.lpszOutput = ((m_bDoPrintToFile) ? m_szPrintToFilePathname : NULL); //!TODO Using ANSI function !!!FIXME!!!
	
	GR_Win32Graphics * pGraphics = new GR_Win32Graphics(m_pPersistPrintDlg->hDC,&m_DocInfo, m_pApp, m_pPersistPrintDlg->hDevMode);
	return pGraphics;
}

void XAP_Win32Dialog_Print::releasePrinterGraphicsContext(GR_Graphics * pGraphics)
{
	GR_Win32Graphics * pWin32Graphics = (GR_Win32Graphics *)pGraphics;
	if (pGraphics)
		delete pGraphics;

	DeleteDC(m_pPersistPrintDlg->hDC);
	m_pPersistPrintDlg->hDC = 0;

	memset(&m_DocInfo, 0, sizeof(m_DocInfo));
}

/*****************************************************************/

void XAP_Win32Dialog_Print::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);

	HWND hwnd = static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow();

	m_pPersistPrintDlg->hwndOwner		= hwnd;
	m_pPersistPrintDlg->nFromPage		= (WORD)m_nFirstPage;
	m_pPersistPrintDlg->nToPage		= (WORD)m_nLastPage;
	m_pPersistPrintDlg->nMinPage		= (WORD)m_nFirstPage;
	m_pPersistPrintDlg->nMaxPage		= (WORD)m_nLastPage;
	m_pPersistPrintDlg->Flags		= PD_ALLPAGES | PD_RETURNDC | PD_ENABLEPRINTHOOK;
	m_pPersistPrintDlg->lpfnPrintHook   = s_PrintHookProc;
	// we do not need this at the moment, but one day it will come handy in the hook procedure
	m_pPersistPrintDlg->lCustData       = (DWORD)this;
		
	if (!m_bEnablePageRange)
		m_pPersistPrintDlg->Flags		|= PD_NOPAGENUMS;
	if (!m_bEnablePrintSelection)
		m_pPersistPrintDlg->Flags		|= PD_NOSELECTION;
	if (m_bEnablePrintToFile)
	{
		if (m_bDoPrintToFile)
			m_pPersistPrintDlg->Flags	|= PD_PRINTTOFILE;
	}
	else
	{
		m_pPersistPrintDlg->Flags		|= PD_HIDEPRINTTOFILE;
	}
	
	if (!m_bPersistValid)				// first time
	{
		// these values are either in the PRINTDLG structure or
		// in the HDEVMODE structure within it depending upon
		// the driver.  since we don't destroy and recreate the
		// structure on each use (and we don't provide our caller
		// access to these fields), we only need to load them the
		// first time.
		
		m_pPersistPrintDlg->nCopies		= (WORD)m_nCopies;
		if (m_bCollate)
			m_pPersistPrintDlg->Flags	|= PD_COLLATE;
	}

	// see if they just want the properties of the printer without
	// bothering the user.
	
	if (m_bPersistValid && m_bBypassActualDialog)
	{
		_extractResults(pFrame);
		if (m_answer == a_OK)
		{
			// create a new hDC for this printer...
			
			DEVMODE * pDevMode = (DEVMODE *)GlobalLock(m_pPersistPrintDlg->hDevMode);
			DEVNAMES * pDevNames = (DEVNAMES *)GlobalLock(m_pPersistPrintDlg->hDevNames);

			TCHAR * p = (TCHAR *)pDevNames;
			m_pPersistPrintDlg->hDC = CreateDC(p + pDevNames->wDriverOffset,
											   p + pDevNames->wDeviceOffset,
											   NULL,
											   pDevMode);
			
			GlobalUnlock(m_pPersistPrintDlg->hDevMode);
			GlobalUnlock(m_pPersistPrintDlg->hDevNames);
		}
	}
	else if (PrintDlg(m_pPersistPrintDlg))		// raise the actual dialog.
	{
		_extractResults(pFrame);
	}
	else
	{
		UT_DEBUGMSG(("Printer dialog failed: reason=0x%x\n", CommDlgExtendedError()));
		m_answer = a_CANCEL;
	}

	return;
}

void XAP_Win32Dialog_Print::_extractResults(XAP_Frame *pFrame)
{
	m_bDoPrintRange		= ((m_pPersistPrintDlg->Flags & PD_PAGENUMS) != 0);
	m_bDoPrintSelection = ((m_pPersistPrintDlg->Flags & PD_SELECTION) != 0);
	m_bDoPrintToFile	= ((m_pPersistPrintDlg->Flags & PD_PRINTTOFILE) != 0);
	m_bCollate			= ((m_pPersistPrintDlg->Flags & PD_COLLATE) != 0);
	m_nCopies			= m_pPersistPrintDlg->nCopies;
	m_nFirstPage		= m_pPersistPrintDlg->nFromPage;
	m_nLastPage			= m_pPersistPrintDlg->nToPage;
	
	if (m_bDoPrintToFile)
	{
		char bufSuggestedName[1030];
		memset(bufSuggestedName,0,sizeof(bufSuggestedName));

		// we construct a suggested pathname for the print-to-file pathname.
		// we append a .print to the string.  it would be better to append
		// a .ps or whatever, but we don't know what the technology/language
		// of the device is....
		
		sprintf(bufSuggestedName,"%s.print",m_szDocumentPathname);
		if (!_getPrintToFilePathname(pFrame,bufSuggestedName))
			goto Fail;
	}

	m_answer = a_OK;
	return;

Fail:
	m_answer = a_CANCEL;
	return;
}
