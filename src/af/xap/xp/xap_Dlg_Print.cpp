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


/******************************************************************/

XAP_Dialog_Print::XAP_Dialog_Print(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_AppPersistent(pDlgFactory,id)
{
	m_bPersistValid = false;
	m_persistNrCopies = 1;
	m_persistCollate = false;
	m_persistColorSpace = GR_Graphics::GR_COLORSPACE_COLOR;	/* full color is default */
	m_persistPrintToFile = false;

	m_szDocumentTitle = NULL;
	m_szDocumentPathname = NULL;
	m_szPrintToFilePathname = NULL;
	m_szPrintCommand = NULL;
	m_bBypassActualDialog = false;
	m_bEnablePageRange = false;
	m_bEnablePrintSelection = false;
	m_bEnablePrintToFile = false;
	m_nFirstPage = 0;
	m_nLastPage = 0;

	m_pageSize = 0;
	m_answer = a_VOID;
}

XAP_Dialog_Print::~XAP_Dialog_Print(void)
{
	UT_ASSERT(!m_bInUse);
}

void XAP_Dialog_Print::useStart(void)
{
	XAP_Dialog_AppPersistent::useStart();
	
	FREEP(m_szDocumentTitle);
	FREEP(m_szDocumentPathname);
	FREEP(m_szPrintToFilePathname);

	m_bBypassActualDialog = false;
	m_bEnablePageRange = false;
	m_bEnablePrintSelection = false;
	m_bEnablePrintToFile = false;
	m_nFirstPage = 0;
	m_nLastPage = 0;

	m_nCopies = ((m_bPersistValid) ? m_persistNrCopies : 1);
	m_bCollate = ((m_bPersistValid) ? m_persistCollate : true);
	m_cColorSpace = ((m_bPersistValid) ? m_persistColorSpace : GR_Graphics::GR_COLORSPACE_COLOR);
	m_bDoPrintToFile = ((m_bPersistValid) ? m_persistPrintToFile : false);
	
	m_answer = a_VOID;
}

void XAP_Dialog_Print::useEnd(void)
{
	XAP_Dialog_AppPersistent::useEnd();

	FREEP(m_szDocumentTitle);
	FREEP(m_szDocumentPathname);
	FREEP(m_szPrintToFilePathname);
	FREEP(m_pageSize);

	if (m_answer == a_OK)
	{
		m_bPersistValid = true;
		m_persistNrCopies = m_nCopies;
		m_persistCollate = m_bCollate;
		m_persistColorSpace = m_cColorSpace;
		m_persistPrintToFile = m_bDoPrintToFile;
	}
	
	// the platform sub-classes should store all persistent fields
	// if (m_answer == a_OK) in their implementations.
}

XAP_Dialog_Print::tAnswer XAP_Dialog_Print::getAnswer(void) const
{
	// let our caller know if user hit ok or cancel.
	
	return m_answer;
}

void XAP_Dialog_Print::setPaperSize(const char * szPageSize)
{
	FREEP(m_pageSize);
	if (szPageSize && *szPageSize)
		UT_cloneString(m_pageSize,szPageSize);
}

void XAP_Dialog_Print::setDocumentTitle(const char * szDocTitle)
{
	FREEP(m_szDocumentTitle);
	if (szDocTitle && *szDocTitle)
		UT_cloneString(m_szDocumentTitle,szDocTitle);
}

void XAP_Dialog_Print::setDocumentPathname(const char * szDocPath)
{
	FREEP(m_szDocumentPathname);
	if (szDocPath && *szDocPath)
		UT_cloneString(m_szDocumentPathname,szDocPath);
}

void XAP_Dialog_Print::setEnablePageRangeButton(bool bEnable,
											   UT_uint32 nFirst,
											   UT_uint32 nLast)
{
	m_bEnablePageRange	= bEnable;
	m_nFirstPage		= ((bEnable) ? nFirst : 0);
	m_nLastPage			= ((bEnable) ? nLast : 0);
}

void XAP_Dialog_Print::setEnablePrintSelection(bool bEnable)
{
	m_bEnablePrintSelection = bEnable;
}

void XAP_Dialog_Print::setEnablePrintToFile(bool bEnable)
{
	m_bEnablePrintToFile = bEnable;
}

void XAP_Dialog_Print::setTryToBypassActualDialog(bool bEnable)
{
	// allow the caller to bypass the actual dialog (useful for the
	// print button on the toolbar (which should try to reuse as much
	// state from the previous use as possible)).  we only let this
	// happen if they ask for it and we have valid persistence data.
	
	m_bBypassActualDialog = (bEnable && m_bPersistValid);
}

bool XAP_Dialog_Print::getDoPrintRange(UT_uint32 * pnFirst, UT_uint32 * pnLast) const
{
	UT_ASSERT(m_answer == a_OK);

	// we set the page ranges whether they checked the print-to-file button or not.
	
	if (pnFirst)
		*pnFirst = m_nFirstPage;
	if (pnLast)
		*pnLast = m_nLastPage;

	return m_bDoPrintRange;
}

bool XAP_Dialog_Print::getDoPrintSelection(void) const
{
	UT_ASSERT(m_answer == a_OK);

	return m_bDoPrintSelection;
}

bool XAP_Dialog_Print::getDoPrintToFile(const char *) const
{
	UT_ASSERT(m_answer == a_OK);

	return m_bDoPrintToFile;
}

UT_uint32 XAP_Dialog_Print::getNrCopies(void) const
{
	UT_ASSERT(m_answer == a_OK);

	return m_nCopies;
}

bool XAP_Dialog_Print::getCollate(void) const
{
	UT_ASSERT(m_answer == a_OK);

	return m_persistCollate;
}

GR_Graphics::ColorSpace XAP_Dialog_Print::getColorSpace(void) const
{
	UT_ASSERT(m_answer == a_OK);

	return m_persistColorSpace;
}

bool XAP_Dialog_Print::_getPrintToFilePathname(XAP_Frame * pFrame,
												 const char * szSuggestedName)
{
	UT_ASSERT(pFrame);
	UT_ASSERT(szSuggestedName && *szSuggestedName);

	XAP_Dialog_Id id = XAP_DIALOG_ID_PRINTTOFILE;
	
	XAP_DialogFactory * pDialogFactory
		= (XAP_DialogFactory *)(pFrame->getDialogFactory());

	XAP_Dialog_FileOpenSaveAs * pDialog
		= (XAP_Dialog_FileOpenSaveAs *)(pDialogFactory->requestDialog(id));
	UT_ASSERT(pDialog);

	pDialog->setCurrentPathname(szSuggestedName);
	pDialog->setSuggestFilename(true);

	{
		// TODO : FIX THIS!  Make this pull dynamic types from the export
		// TODO : filter list (creat that while you're at it).

		// TODO : Right now we can just feed the dialog some static filters
		// TODO : that will be ignored by Windows and BeOS but will be required
		// TODO : by Unix.

		UT_uint32 filterCount = 1;

		const char ** szDescList = (const char **) calloc(filterCount + 1,
														  sizeof(char *));
		const char ** szSuffixList = (const char **) calloc(filterCount + 1,
															sizeof(char *));
		// HACK : this should be IEFileType
		UT_sint32 * nTypeList = (UT_sint32 *) calloc(filterCount + 1,
													 sizeof(UT_sint32));

		szDescList[0] = "PostScript 2.0";
		szSuffixList[0] = "ps";
		nTypeList[0] = 0;

		pDialog->setFileTypeList(szDescList, szSuffixList, (const UT_sint32 *) nTypeList);
	}

	pDialog->runModal(pFrame);

	XAP_Dialog_FileOpenSaveAs::tAnswer ans = pDialog->getAnswer();
	bool bOK = (ans == XAP_Dialog_FileOpenSaveAs::a_OK);

	if (bOK)
		UT_cloneString(m_szPrintToFilePathname,pDialog->getPathname());
	
	pDialogFactory->releaseDialog(pDialog);

	return bOK;
}
