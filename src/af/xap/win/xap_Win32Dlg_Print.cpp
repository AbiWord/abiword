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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
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
#include "xav_View.h"
#include "gr_Win32Graphics.h"
#include "xap_Win32DialogBase.h"

/*****************************************************************/
static UINT_PTR CALLBACK s_PrintHookProc(
  HWND hdlg,      // handle to the dialog box window
  UINT uiMsg,     // message identifier
  WPARAM wParam,  // message parameter
  LPARAM lParam   // message parameter
  )                                                                             
{
	XAP_Win32Dialog_Print * pThis = NULL;
	
	if(uiMsg == WM_INITDIALOG)
	{
		PRINTDLGW * pDlgInfo = (PRINTDLGW*)lParam;
		pThis = (XAP_Win32Dialog_Print*)pDlgInfo->lCustData;
		SetWindowLongPtrW(hdlg, DWLP_USER, (LONG_PTR) pThis);

		// reset the 'closed' flag which indicates that the dialog should be considered
		// 'closed' rather than aborted
		pThis->setClosed(false);

		// hide the selection radio since we do not support selection printing
		if(pDlgInfo->Flags & PD_NOSELECTION)
		{
			HWND wPrintSelectionRadio = GetDlgItem(hdlg, rad2);
			ShowWindow(wPrintSelectionRadio, SW_HIDE);
		}

		// remember the original printer selected; we use this to decide if upon closure
		// we should notify associated views to rebuild to reflect the new font metrics
		UT_uint32 iPrinter = SendDlgItemMessageW(hdlg, cmb4, CB_GETCURSEL, 0, 0);
		pThis->setOrigPrinter(iPrinter);
		pThis->setNewPrinter(iPrinter);
	}
	else
	{
		pThis = (XAP_Win32Dialog_Print*)GetWindowLongPtrW(hdlg, DWLP_USER);
	}
	
	if(uiMsg == WM_COMMAND)
	{
		// if the command indicates that the user changed printer, we change the Cancel
		// button to Close; make sure we have valid this pointer.
		if((int)LOWORD(wParam) == cmb4 && HIWORD(wParam) == CBN_SELCHANGE)
		{
			pThis->setNewPrinter(SendDlgItemMessageW(hdlg, cmb4, CB_GETCURSEL, 0, 0));

			XAP_App*                    pApp      = XAP_App::getApp();
			const XAP_StringSet*  pSS         = pApp->getStringSet();
			
			if(pThis->getNewPrinter() != pThis->getOrigPrinter())
			{
			    XAP_Win32DialogBase::setDlgItemText(hdlg,IDCANCEL,pSS->getValue(XAP_STRING_ID_DLG_Close));
			}
			else
			{
				// the user set the printer back to what it used to be -- revert back to
				// cancel button
			    XAP_Win32DialogBase::setDlgItemText(hdlg,IDCANCEL,pSS->getValue(XAP_STRING_ID_DLG_Cancel));
			}
		}
		else if((int)LOWORD(wParam) == IDCANCEL && HIWORD(wParam) == 0)
		{
			// the user presed the Cancel/Close button; see if it is Cancel or Close
			if(pThis->getNewPrinter() != pThis->getOrigPrinter())
			{
				// Different printer is selected to the one we started with, this is Close
				// scenario -- eat this message and simulate click on the OK button
				// instead so that the default dlg procedure fills the PRINTDLG struct
				// with the data we need; set the m_bClosed flag, so we can differentiate
				// this from the user pressing the OK button
				pThis->setClosed(true);
				PostMessageW(hdlg, WM_COMMAND, IDOK, 0);

				return 1; // eat the message
			}
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
										   XAP_Dialog_Id id):
	XAP_Dialog_Print(pDlgFactory,id),
	m_iOrigPrinter(0),
	m_iNewPrinter(0),
	m_bClosed(false)
{
	m_pPersistPrintDlg = (PRINTDLGW *)UT_calloc(1,sizeof(PRINTDLGW));
	UT_return_if_fail(m_pPersistPrintDlg);

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
		g_free(m_pPersistPrintDlg);
	}
}

GR_Graphics * XAP_Win32Dialog_Print::getPrinterGraphicsContext(void)
{
	UT_ASSERT(m_answer == a_OK);

	if (!m_pPersistPrintDlg || !m_pPersistPrintDlg->hDC)
		return NULL; /* Prevents from passing NULL to GR_Win32Graphics*/

	memset(&m_DocInfo,0,sizeof(m_DocInfo));
	m_DocInfo.cbSize = sizeof(DOCINFOW);
	m_docName.fromUTF8 (m_szDocumentPathname);
	m_DocInfo.lpszDocName = m_docName.c_str();
	
    if (m_bDoPrintToFile) {
		m_fileName.fromUTF8 (m_szPrintToFilePathname);
		m_DocInfo.lpszOutput = m_fileName.c_str();
	} else {
		m_DocInfo.lpszOutput = NULL;	
	}	

	GR_Win32AllocInfo ai(m_pPersistPrintDlg->hDC,&m_DocInfo, m_pPersistPrintDlg->hDevMode);
	GR_Win32Graphics *pGr = (GR_Win32Graphics *)XAP_App::getApp()->newGraphics(ai);
	UT_ASSERT(pGr);
	
	return pGr;
}

void XAP_Win32Dialog_Print::releasePrinterGraphicsContext(GR_Graphics * pGraphics)
{
	if (pGraphics)
		delete pGraphics;

#if 0
	// Do not delete the DC, since the original view will use it to do its layout and will
	// delete it when no longer needed. Tomas
	DeleteDC(m_pPersistPrintDlg->hDC);
#endif
	if(m_pPersistPrintDlg)
		m_pPersistPrintDlg->hDC = 0;

	memset(&m_DocInfo, 0, sizeof(m_DocInfo));
}

/*****************************************************************/

void XAP_Win32Dialog_Print::runModal(XAP_Frame * pFrame)
{
	UT_return_if_fail(pFrame);
	UT_return_if_fail(m_pPersistPrintDlg);

	HWND hwnd = static_cast<XAP_Win32FrameImpl*>(pFrame->getFrameImpl())->getTopLevelWindow();

	m_pPersistPrintDlg->hwndOwner	= hwnd;
	m_pPersistPrintDlg->nFromPage	= (WORD)m_nFirstPage;
	m_pPersistPrintDlg->nToPage		= (WORD)m_nLastPage;
	m_pPersistPrintDlg->nMinPage	= (WORD)m_nFirstPage;
	m_pPersistPrintDlg->nMaxPage	= (WORD)m_nLastPage;
	m_pPersistPrintDlg->Flags		= PD_ALLPAGES | PD_RETURNDC | PD_ENABLEPRINTHOOK;
	m_pPersistPrintDlg->lpfnPrintHook   = s_PrintHookProc;
	m_pPersistPrintDlg->lCustData       = (INT_PTR)this;
		
	if (!m_bEnablePageRange)
		m_pPersistPrintDlg->Flags	|= PD_NOPAGENUMS;
	if (!m_bEnablePrintSelection)
		m_pPersistPrintDlg->Flags	|= PD_NOSELECTION;
	if (m_bEnablePrintToFile)
	{
		if (m_bDoPrintToFile)
			m_pPersistPrintDlg->Flags	|= PD_PRINTTOFILE;
	}
	else
	{
		m_pPersistPrintDlg->Flags	|= PD_HIDEPRINTTOFILE;
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
			
			DEVMODEW * pDevMode = (DEVMODEW *)GlobalLock(m_pPersistPrintDlg->hDevMode);
			DEVNAMES * pDevNames = (DEVNAMES *)GlobalLock(m_pPersistPrintDlg->hDevNames);
			UT_return_if_fail(pDevNames); //GlobalLock can return NULL

			wchar_t * p = (wchar_t *)pDevNames;
			m_pPersistPrintDlg->hDC = CreateDCW(p + pDevNames->wDriverOffset,
											   p + pDevNames->wDeviceOffset,
											   NULL,
											   pDevMode);
			
			GlobalUnlock(m_pPersistPrintDlg->hDevMode);
			GlobalUnlock(m_pPersistPrintDlg->hDevNames);

			// notify layout of printer (this results in redoing doc layout !!!)
			GR_Win32Graphics* pG=(GR_Win32Graphics*)XAP_App::getApp()->getLastFocussedFrame()->getCurrentView()->getGraphics();
			UT_return_if_fail( pG );
			pG->setPrintDC(m_pPersistPrintDlg->hDC);
		}
	}
	else if (PrintDlgW(m_pPersistPrintDlg))		// raise the actual dialog.
	{
		_extractResults(pFrame);

		// notify layout of printer (this results in redoing doc layout !!!)
		GR_Win32Graphics* pG=(GR_Win32Graphics*)XAP_App::getApp()->getLastFocussedFrame()->getCurrentView()->getGraphics();
		UT_return_if_fail( pG );
		pG->setPrintDC(m_pPersistPrintDlg->hDC);

		// we need to differentiate between actual OK being pressed by user and simulated
		// by our hook procedure -- in the later case we want to return a_CANCEL so that
		// the document does not get sent to the printer. We will also set m_bPersistValid
		// as if OK was clicked
		if(m_bClosed)
		{
			m_answer = a_CANCEL;
			m_bPersistValid = true;
		}
		
	}
	else
	{
		// The user clicked the cancel button and did not change the printer, there is
		// nothing for us to do
		m_answer = a_CANCEL;
	}

	return;
}

void XAP_Win32Dialog_Print::_extractResults(XAP_Frame *pFrame)
{
	UT_return_if_fail(m_pPersistPrintDlg);
	UT_return_if_fail(m_pPersistPrintDlg->hDevMode!=NULL);

	m_bDoPrintRange		= ((m_pPersistPrintDlg->Flags & PD_PAGENUMS) != 0);
	m_bDoPrintSelection = ((m_pPersistPrintDlg->Flags & PD_SELECTION) != 0);
	m_bDoPrintToFile	= ((m_pPersistPrintDlg->Flags & PD_PRINTTOFILE) != 0);	
	m_nFirstPage		= m_pPersistPrintDlg->nFromPage;
	m_nLastPage			= m_pPersistPrintDlg->nToPage;

	// Most Win32 printer drivers support multicopies and collating, 
	//however we want Abi to do both by himself, like in the rest of platforms		
	DEVMODEW *pDevMode=(DEVMODEW *)GlobalLock(m_pPersistPrintDlg->hDevMode);
	UT_return_if_fail(pDevMode); //GlobalLock can return NULL
	m_nCopies = pDevMode->dmCopies;
	m_bCollate	= ((pDevMode->dmCollate  & DMCOLLATE_TRUE) != 0);		
	pDevMode->dmCopies = 1;
	pDevMode->dmCollate = DMCOLLATE_FALSE;
	pDevMode->dmFields = DM_COPIES | DM_COLLATE;
	GlobalUnlock(m_pPersistPrintDlg->hDevMode);

	// any changes to the DEVMODE structure must be followed by this call before the
	// modified structure can be used !!!
	GR_Win32Graphics::fixDevMode(m_pPersistPrintDlg->hDevMode);

	pDevMode=(DEVMODEW *)GlobalLock(m_pPersistPrintDlg->hDevMode);
	ResetDCW(m_pPersistPrintDlg->hDC,pDevMode);
	GlobalUnlock(pDevMode);
	
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

