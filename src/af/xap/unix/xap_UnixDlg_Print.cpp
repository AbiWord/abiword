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
#include <stdlib.h>
#include <string.h>
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "xap_Dialog_Id.h"
#include "ap_UnixDialog_Print.h"
#include "ap_UnixApp.h"
#include "ap_UnixFrame.h"
#include "ps_Graphics.h"

#define DELETEP(p)	do { if (p) delete(p); (p)=NULL; } while (0)

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
	DELETEP(m_pPSGraphics);
}

DG_Graphics * AP_UnixDialog_Print::getPrinterGraphicsContext(void)
{
	UT_ASSERT(m_answer == a_OK);

	return m_pPSGraphics;
}

void AP_UnixDialog_Print::releasePrinterGraphicsContext(DG_Graphics * pGraphics)
{
	UT_ASSERT(pGraphics == m_pPSGraphics);
	
	DELETEP(m_pPSGraphics);
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
#endif
	
	// see if they just want the properties of the printer without
	// bothering the user.
	
	if (m_bPersistValid && m_bBypassActualDialog)
	{
		_extractResults();
	}
	else if (_raisePrintDialog())
	{
		_extractResults();
	}
	else
	{
		m_answer = a_CANCEL;
	}

	m_pUnixFrame = NULL;
	return;
}

UT_Bool AP_UnixDialog_Print::_raisePrintDialog(void)
{
	// raise the actual dialog and wait for an answer.
	// return true if they hit ok.

	return UT_TRUE;
}


void AP_UnixDialog_Print::_extractResults(void)
{
#if 0
	m_bDoPrintRange		= ((m_pPersistPrintDlg->Flags & PD_PAGENUMS) != 0);
	m_bDoPrintSelection = ((m_pPersistPrintDlg->Flags & PD_SELECTION) != 0);
	m_bDoPrintToFile	= ((m_pPersistPrintDlg->Flags & PD_PRINTTOFILE) != 0);
	m_bCollate			= ((m_pPersistPrintDlg->Flags & PD_COLLATE) != 0);
	m_nCopies			= m_pPersistPrintDlg->nCopies;
	m_nFirstPage		= m_pPersistPrintDlg->nFromPage;
	m_nLastPage			= m_pPersistPrintDlg->nToPage;
#else
	m_bDoPrintToFile = UT_TRUE;
#endif
	
	if (m_bDoPrintToFile)
	{
		char bufSuggestedName[1030];
		memset(bufSuggestedName,0,sizeof(bufSuggestedName));

		// we construct a suggested pathname for the print-to-file pathname.
		// we append a .print to the string.  it would be better to append
		// a .ps or whatever, but we don't know what the technology/language
		// of the device is....
		
		sprintf(bufSuggestedName,"%s.ps",m_szDocumentPathname);
		if (!_getPrintToFilePathname(m_pUnixFrame,bufSuggestedName))
			goto Fail;
	}

	// we're going to remember the application name here because PS_Graphics
	// needs it on the constructor

	m_pPSGraphics = new PS_Graphics(m_szPrintToFilePathname,
									m_szDocumentTitle,
									m_pUnixFrame->getApp()->getApplicationName());
	UT_ASSERT(m_pPSGraphics);
	
	m_answer = a_OK;
	return;

Fail:
	m_answer = a_CANCEL;
	return;
}

