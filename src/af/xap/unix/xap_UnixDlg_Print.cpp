/* AbiWord
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

#include <stdio.h>
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "xap_Dialog_Id.h"
#include "ap_UnixDialog_Print.h"
#include "ap_UnixApp.h"
#include "ap_UnixFrame.h"
#include "gr_UnixGraphics.h"

/*****************************************************************/
AP_Dialog * AP_UnixDialog_Print::static_constructor(AP_DialogFactory * pFactory,
													 AP_Dialog_Id id)
{
	AP_UnixDialog_Print * p = new AP_UnixDialog_Print(pFactory,id);
	return p;
}

AP_UnixDialog_Print::AP_UnixDialog_Print(AP_DialogFactory * pDlgFactory,
										   AP_Dialog_Id id)
	: AP_Dialog_Print(pDlgFactory,id)
{
}

AP_UnixDialog_Print::~AP_UnixDialog_Print(void)
{
}

DG_Graphics * AP_UnixDialog_Print::getPrinterGraphicsContext(void)
{
	UT_ASSERT(m_answer == a_OK);
#if 0
	UnixGraphics * pGraphics = NULL;	// TODO.....
	
	return pGraphics;
#else
	return 0;
#endif
}

void AP_UnixDialog_Print::releasePrinterGraphicsContext(DG_Graphics * pGraphics)
{
#if 0
	UnixGraphics * pUnixGraphics = (UnixGraphics *)pGraphics;
	if (pGraphics)
		delete pGraphics;
#endif
}

/*****************************************************************/

void AP_UnixDialog_Print::runModal(AP_Frame * pFrame)
{
	m_pUnixFrame = static_cast<AP_UnixFrame *>(pFrame);
	UT_ASSERT(m_pUnixFrame);

#if 0
	m_pPersistPrintDlg->hwndOwner		= hwnd;
	m_pPersistPrintDlg->nFromPage		= (WORD)m_nFirstPage;
	m_pPersistPrintDlg->nToPage			= (WORD)m_nLastPage;
	m_pPersistPrintDlg->nMinPage		= (WORD)m_nFirstPage;
	m_pPersistPrintDlg->nMaxPage		= (WORD)m_nLastPage;
	m_pPersistPrintDlg->Flags			= PD_ALLPAGES | PD_RETURNDC;
	if (!m_bEnablePageRange)
		m_pPersistPrintDlg->Flags		|= PD_NOPAGENUMS;
	if (!m_bEnablePrintSelection)
		m_pPersistPrintDlg->Flags		|= PD_NOSELECTION;
	if (!m_bEnablePrintToFile)
		m_pPersistPrintDlg->Flags		|= PD_HIDEPRINTTOFILE;
		
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
		_extractResults();
	}
	else if (PrintDlg(m_pPersistPrintDlg))		// raise the actual dialog.
	{
		_extractResults();
	}
	else
#endif
	{
		m_answer = a_CANCEL;
	}

	m_pUnixFrame = NULL;
	return;
}

#if 0
void AP_UnixDialog_Print::_extractResults(void)
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
		
		sprintf(bufSuggestedName,"%s.ps",m_szDocumentPathname);
		if (!_getPrintToFilePathname(m_pWin32Frame,bufSuggestedName))
			goto Fail;
	}

	m_answer = a_OK;
	return;

Fail:
	m_answer = a_CANCEL;
	return;
}
#endif

