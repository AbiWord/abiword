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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_debugmsg.h"
#include "xap_Dialog_Id.h"
#include "xap_Dlg_Print.h"
#include "xap_Frame.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_FileOpenSaveAs.h"


#define FREEP(p)	do { if (p) free(p); (p)=NULL; } while (0)
#define DELETEP(p)	do { if (p) delete(p); (p)=NULL; } while (0)

/******************************************************************/

AP_Dialog_Print::AP_Dialog_Print(AP_DialogFactory * pDlgFactory, AP_Dialog_Id id)
	: AP_Dialog_AppPersistent(pDlgFactory,id)
{
	m_bPersistValid = UT_FALSE;
	m_persistNrCopies = 1;
	m_persistCollate = UT_FALSE;
	m_persistPrintToFile = UT_FALSE;

	m_szDocumentTitle = NULL;
	m_szDocumentPathname = NULL;
	m_szPrintToFilePathname = NULL;
	m_szPrintCommand = NULL;
	m_bBypassActualDialog = UT_FALSE;
	m_bEnablePageRange = UT_FALSE;
	m_bEnablePrintSelection = UT_FALSE;
	m_bEnablePrintToFile = UT_FALSE;
	m_nFirstPage = 0;
	m_nLastPage = 0;

	m_answer = a_VOID;
}

AP_Dialog_Print::~AP_Dialog_Print(void)
{
	UT_ASSERT(!m_bInUse);
}

void AP_Dialog_Print::useStart(void)
{
	AP_Dialog_AppPersistent::useStart();
	
	FREEP(m_szDocumentTitle);
	FREEP(m_szDocumentPathname);
	FREEP(m_szPrintToFilePathname);

	m_bBypassActualDialog = UT_FALSE;
	m_bEnablePageRange = UT_FALSE;
	m_bEnablePrintSelection = UT_FALSE;
	m_bEnablePrintToFile = UT_FALSE;
	m_nFirstPage = 0;
	m_nLastPage = 0;

	m_nCopies = ((m_bPersistValid) ? m_persistNrCopies : 1);
	m_bCollate = ((m_bPersistValid) ? m_persistCollate : UT_TRUE);
	m_bDoPrintToFile = ((m_bPersistValid) ? m_persistPrintToFile : UT_FALSE);
	
	m_answer = a_VOID;
}

void AP_Dialog_Print::useEnd(void)
{
	AP_Dialog_AppPersistent::useEnd();

	FREEP(m_szDocumentTitle);
	FREEP(m_szDocumentPathname);
	FREEP(m_szPrintToFilePathname);

	if (m_answer == a_OK)
	{
		m_bPersistValid = UT_TRUE;
		m_persistNrCopies = m_nCopies;
		m_persistCollate = m_bCollate;
		m_persistPrintToFile = m_bDoPrintToFile;
	}
	
	// the platform sub-classes should store all persistent fields
	// if (m_answer == a_OK) in their implementations.
}

AP_Dialog_Print::tAnswer AP_Dialog_Print::getAnswer(void) const
{
	// let our caller know if user hit ok or cancel.
	
	return m_answer;
}

void AP_Dialog_Print::setDocumentTitle(const char * szDocTitle)
{
	FREEP(m_szDocumentTitle);
	if (szDocTitle && *szDocTitle)
		UT_cloneString(m_szDocumentTitle,szDocTitle);
}

void AP_Dialog_Print::setDocumentPathname(const char * szDocPath)
{
	FREEP(m_szDocumentPathname);
	if (szDocPath && *szDocPath)
		UT_cloneString(m_szDocumentPathname,szDocPath);
}

void AP_Dialog_Print::setEnablePageRangeButton(UT_Bool bEnable,
											   UT_uint32 nFirst,
											   UT_uint32 nLast)
{
	m_bEnablePageRange	= bEnable;
	m_nFirstPage		= ((bEnable) ? nFirst : 0);
	m_nLastPage			= ((bEnable) ? nLast : 0);
}

void AP_Dialog_Print::setEnablePrintSelection(UT_Bool bEnable)
{
	m_bEnablePrintSelection = bEnable;
}

void AP_Dialog_Print::setEnablePrintToFile(UT_Bool bEnable)
{
	m_bEnablePrintToFile = bEnable;
}

void AP_Dialog_Print::setTryToBypassActualDialog(UT_Bool bEnable)
{
	// allow the caller to bypass the actual dialog (useful for the
	// print button on the toolbar (which should try to reuse as much
	// state from the previous use as possible)).  we only let this
	// happen if they ask for it and we have valid persistence data.
	
	m_bBypassActualDialog = (bEnable && m_bPersistValid);
}

UT_Bool AP_Dialog_Print::getDoPrintRange(UT_uint32 * pnFirst, UT_uint32 * pnLast) const
{
	UT_ASSERT(m_answer == a_OK);

	// we set the page ranges whether they checked the print-to-file button or not.
	
	if (pnFirst)
		*pnFirst = m_nFirstPage;
	if (pnLast)
		*pnLast = m_nLastPage;

	return m_bDoPrintRange;
}

UT_Bool AP_Dialog_Print::getDoPrintSelection(void) const
{
	UT_ASSERT(m_answer == a_OK);

	return m_bDoPrintSelection;
}

UT_Bool AP_Dialog_Print::getDoPrintToFile(const char *) const
{
	UT_ASSERT(m_answer == a_OK);

	return m_bDoPrintToFile;
}

UT_uint32 AP_Dialog_Print::getNrCopies(void) const
{
	UT_ASSERT(m_answer == a_OK);

	return m_nCopies;
}

UT_Bool AP_Dialog_Print::getCollate(void) const
{
	UT_ASSERT(m_answer == a_OK);

	return m_persistCollate;
}

UT_Bool AP_Dialog_Print::_getPrintToFilePathname(XAP_Frame * pFrame,
												 const char * szSuggestedName)
{
	UT_ASSERT(pFrame);
	UT_ASSERT(szSuggestedName && *szSuggestedName);

	AP_Dialog_Id id = XAP_DIALOG_ID_PRINTTOFILE;
	
	AP_DialogFactory * pDialogFactory
		= (AP_DialogFactory *)(pFrame->getDialogFactory());

	AP_Dialog_FileOpenSaveAs * pDialog
		= (AP_Dialog_FileOpenSaveAs *)(pDialogFactory->requestDialog(id));
	UT_ASSERT(pDialog);

	pDialog->setCurrentPathname(szSuggestedName);
	pDialog->setSuggestFilename(UT_TRUE);

	pDialog->runModal(pFrame);

	AP_Dialog_FileOpenSaveAs::tAnswer ans = pDialog->getAnswer();
	UT_Bool bOK = (ans == AP_Dialog_FileOpenSaveAs::a_OK);

	if (bOK)
		UT_cloneString(m_szPrintToFilePathname,pDialog->getPathname());
	
	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}
