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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "xap_Dialog_Id.h"
#include "xap_QNXDlg_Print.h"
#include "xap_QNXDlg_MessageBox.h"
#include "xap_QNXApp.h"
#include "xap_QNXFrame.h"
#include "xap_Strings.h"

/*****************************************************************/
/*****************************************************************/

XAP_Dialog * XAP_QNXDialog_Print::static_constructor(XAP_DialogFactory * pFactory,
													 XAP_Dialog_Id id)
{
	XAP_QNXDialog_Print * p = new XAP_QNXDialog_Print(pFactory,id);
	return p;
}

XAP_QNXDialog_Print::XAP_QNXDialog_Print(XAP_DialogFactory * pDlgFactory,
										   XAP_Dialog_Id id)
	: XAP_Dialog_Print(pDlgFactory,id)
{
	m_pPrintContext = NULL;
}

XAP_QNXDialog_Print::~XAP_QNXDialog_Print(void)
{
}

void XAP_QNXDialog_Print::useStart(void)
{
	XAP_Dialog_Print::useStart();

	if (m_bPersistValid)
	{
		m_persistPrintDlg.bDoPageRange = m_bDoPrintRange;
		m_persistPrintDlg.bDoPrintSelection = m_bDoPrintSelection;
		m_persistPrintDlg.bDoPrintToFile = m_bDoPrintToFile;
		m_persistPrintDlg.bDoCollate = m_bCollate;
	}
}

void XAP_QNXDialog_Print::useEnd(void)
{
	XAP_Dialog_Print::useEnd();

	m_persistPrintDlg.bDoPageRange = m_bDoPrintRange;
	m_persistPrintDlg.bDoPrintSelection = m_bDoPrintSelection;
	m_persistPrintDlg.bDoPrintToFile = m_bDoPrintToFile;
	m_persistPrintDlg.bDoCollate = m_bCollate;
	m_persistPrintDlg.nCopies = m_nCopies;
	m_persistPrintDlg.nFromPage = m_nFirstPage;
	m_persistPrintDlg.nToPage = m_nLastPage;
}

GR_Graphics * XAP_QNXDialog_Print::getPrinterGraphicsContext(void)
{
#if 0
	UT_ASSERT(m_answer == a_OK && m_pPrintContext);

	return m_pPrintContext;
#endif
	return NULL;
}

void XAP_QNXDialog_Print::releasePrinterGraphicsContext(GR_Graphics * pContext)
{
#if 0
	if (m_pPrintContext) {
		PpPrintStop(m_pPrintContext);
		PpPrintClost(m_pPrintContext);
	}
#endif
}

/*****************************************************************/

void XAP_QNXDialog_Print::runModal(XAP_Frame * pFrame)
{
	m_pQNXFrame = static_cast<XAP_QNXFrame *>(pFrame);
	UT_ASSERT(m_pQNXFrame);
	
	// see if they just want the properties of the printer without
	// bothering the user.
	if (m_bPersistValid && m_bBypassActualDialog) {
		m_answer = a_OK;
		_getGraphics();
	}
	else
	{
		_raisePrintDialog(pFrame);		
		if (m_answer == a_OK)
			_getGraphics();
	}

	m_pQNXFrame = NULL;
}

void XAP_QNXDialog_Print::_raisePrintDialog(XAP_Frame * pFrame)
{
		if (!m_bPersistValid)		// first time called
		{
			m_persistPrintDlg.bEnablePrintToFile = m_bEnablePrintToFile;
			m_persistPrintDlg.bEnablePageRange = m_bEnablePageRange;
			m_persistPrintDlg.bEnableSelection = m_bEnablePrintSelection;
			m_persistPrintDlg.nFromPage = m_nFirstPage;
			m_persistPrintDlg.nToPage = m_nLastPage;
			// The first time through, grab the settings and set min and max for range checking
			m_persistPrintDlg.nMinPage = m_nFirstPage;
			m_persistPrintDlg.nMaxPage = m_nLastPage;

		}

#if 0
		char str[30];
		sprintf(str, "%d", m_persistPrintDlg.nFromPage);
		gtk_entry_set_text (GTK_ENTRY (entryFrom), str);
		sprintf(str, "%d", m_persistPrintDlg.nToPage);
		gtk_entry_set_text (GTK_ENTRY (entryTo), str);
#endif

	// get top level window and it's GtkWidget *
	XAP_QNXFrame * frame = static_cast<XAP_QNXFrame *>(pFrame);
	UT_ASSERT(frame);


	int value;

	m_pPrintContext = PpPrintCreatePC();
	UT_ASSERT(m_pPrintContext);

	value = PtPrintSelection(frame->getTopLevelWindow(), 		/* Parent widget */
					 		 NULL, 		/* Position on the screen */
					 		 NULL, 		/* Title */
					 		 m_pPrintContext, 	/* Print context */
					 		 Pt_PRINTSEL_ALL_PANES); /* Flags */
	m_answer = a_CANCEL;

	if (value == Pt_PRINTSEL_CANCEL) {
		PpPrintClose(m_pPrintContext);
		m_pPrintContext = NULL;
		return;	
	}
		

	m_answer = a_OK;

	if (value == Pt_PRINTSEL_PRINT) {
		m_bDoPrintRange		= 0;
		m_bDoPrintSelection = 0;
		m_bDoPrintToFile	= 0;
		m_bCollate			= 0;
		m_nCopies			= 1;

		if (m_bDoPrintRange) {
			UT_uint32 first = 0;
			if (first < m_persistPrintDlg.nMinPage) {
				first = m_persistPrintDlg.nMinPage;
			}

			UT_uint32 last = 0;
			if (last > m_persistPrintDlg.nMaxPage) {
				last = m_persistPrintDlg.nMaxPage;
			}
			
			m_nFirstPage = MyMin(first,last);
			m_nLastPage = MyMax(first,last);
		}
	}

	return;
}

void XAP_QNXDialog_Print::_getGraphics(void)
{
	printf("TODO: _getGraphics \n");
	m_answer = a_CANCEL;

#if 0
	UT_ASSERT(m_answer == a_OK);

	XAP_App * app = m_pQNXFrame->getApp();
	UT_ASSERT(app);
	
	XAP_QNXApp * unixapp = static_cast<XAP_QNXApp *> (app);
	UT_ASSERT(unixapp);

	if (m_bDoPrintToFile)
	{
		printf("TODO: Print to a file \n");
#if 0
		// we construct a suggested pathname for the print-to-file pathname.
		// we append a .print to the string.  it would be better to append
		// a .ps or whatever, but we don't know what the technology/language
		// of the device is....
		
		char bufSuggestedName[1030];
		memset(bufSuggestedName,0,sizeof(bufSuggestedName));

		sprintf(bufSuggestedName,"%s.ps",m_szDocumentPathname);
		if (!_getPrintToFilePathname(m_pQNXFrame,bufSuggestedName))
			goto Fail;

		m_pPSGraphics = new PS_Graphics(m_szPrintToFilePathname, m_szDocumentTitle,
										m_pQNXFrame->getApp()->getApplicationName(),
										fontmgr,
										UT_TRUE);
#endif
		m_answer = a_CANCEL;
		return;	
	}
	else
	{		
		int value;

		m_pPrintContext = PpPrintContextCreatePC();
		UT_ASSERT(m_pPrintContext);

		value = PtPrintSelection(NULL, 		/* Parent widget */
						 		 NULL, 		/* Position on the screen */
						 		 NULL, 		/* Title */
						 		 m_pPrintContext, 	/* Print context */
						 		 Pt_PRINTSEL_ALL_PANES); /* Flags */

		if (value == Pt_PRINTSEL_CANCEL) {
			PpPrintClose(m_pPrintContext);
			m_pPrintContext = NULL;
			m_answer = a_CANCEL;
			return;	
		}
		
	}

	m_answer = a_OK;
	return;
#endif
}
