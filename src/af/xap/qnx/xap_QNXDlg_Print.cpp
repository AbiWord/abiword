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
	m_pQNXFrame = NULL;
}

XAP_QNXDialog_Print::~XAP_QNXDialog_Print(void)
{
	if (m_pPrintContext) {
		PpReleasePC(m_pPrintContext);
		m_pPrintContext = NULL;
	}
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
	UT_ASSERT(m_answer == a_OK);
	UT_ASSERT(m_pQNXFrame);
	UT_ASSERT(m_pPrintContext);

	GR_QNXGraphics *gr = (GR_QNXGraphics *)m_pQNXFrame->getGraphics();
	gr->setPrintContext(m_pPrintContext);

	/* Return the same graphics as we used for the screen */
	return m_pQNXFrame->getGraphics();
}

void XAP_QNXDialog_Print::releasePrinterGraphicsContext(GR_Graphics * pContext)
{
	UT_ASSERT(m_pQNXFrame);
	UT_ASSERT(m_pPrintContext);

	GR_QNXGraphics *gr = (GR_QNXGraphics *)m_pQNXFrame->getGraphics();
	gr->setPrintContext(NULL);
	PpPrintReleasePC(m_pPrintContext);
	m_pPrintContext = NULL;
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
	}
	else {
		_raisePrintDialog(pFrame);		
	}
}

void XAP_QNXDialog_Print::_raisePrintDialog(XAP_Frame * pFrame)
{
	if (!m_bPersistValid) {		// first time called
		m_persistPrintDlg.bEnablePrintToFile = m_bEnablePrintToFile;
		m_persistPrintDlg.bEnablePageRange = m_bEnablePageRange;
		m_persistPrintDlg.bEnableSelection = m_bEnablePrintSelection;
		m_persistPrintDlg.nFromPage = m_nFirstPage;
		m_persistPrintDlg.nToPage = m_nLastPage;
		// The first time through, grab the settings and set min and max for range checking
		m_persistPrintDlg.nMinPage = m_nFirstPage;
		m_persistPrintDlg.nMaxPage = m_nLastPage;
	}

	if (m_pPrintContext) {
		PpReleasePC(m_pPrintContext);
	}

	/*TODO: Map the user choices from persistPrintDlg to the current dialog */

	int value;

	m_pPrintContext = PpCreatePC();
	UT_ASSERT(m_pPrintContext);
	value = PtPrintSelection(m_pQNXFrame->getTopLevelWindow(), 		/* Parent widget */
					 		 NULL, 									/* Position on the screen */
					 		 NULL, 									/* Title */
					 		 m_pPrintContext, 						/* Print context */
					 		 Pt_PRINTSEL_ALL_PANES); 				/* Flags */

	m_answer = a_CANCEL;

	if (value == Pt_PRINTSEL_CANCEL) {
		PpReleasePC(m_pPrintContext);
		m_pPrintContext = NULL;
		return;	
	}

	m_answer = a_OK;

	if (value == Pt_PRINTSEL_PRINT || value == Pt_PRINTSEL_PREVIEW) {
		UT_uint32 first = 0, last = 0;
		char *option;
		PhRect_t 	*rect, nrect;
		PhDim_t 	*dim, size;

		nrect.ul.x = nrect.ul.y = 0;
		nrect.lr.x = nrect.lr.y = 0;
		PpSetPC(m_pPrintContext, Pp_PC_MARGINS, &nrect, 0);

		PpGetPC(m_pPrintContext, Pp_PC_NONPRINT_MARGINS, (const void **)&rect);
		UT_DEBUGMSG(("Non-Print Margins are %d,%d %d,%d \n", 
				rect->ul.x, rect->ul.y, rect->lr.x, rect->lr.y));

		PpGetPC(m_pPrintContext, Pp_PC_PAPER_SIZE, (const void **)&dim);
		UT_DEBUGMSG(("Paper size is %d/%d \n", dim->w, dim->h));

#define DPI_LEVEL 72
#if 0	/* Hardcode 612/792 for now */
		size.w = ((dim->w -
				  (rect->ul.x + rect->lr.x)) * DPI_LEVEL) / 1000;
		size.h = ((dim->h -
				  (rect->ul.y + rect->lr.y)) * DPI_LEVEL) / 1000;
#else
		size.w = 612;
		size.h = 792;
#endif
		printf("Source size %d/%d \n", size.w, size.h);
		PpSetPC(m_pPrintContext, Pp_PC_SOURCE_SIZE, &size, 0);

		PpPageRange_t *range = NULL;
		PpGetPC(m_pPrintContext, Pp_PC_PAGE_RANGE, (const void **)&range);
		UT_DEBUGMSG(("Range is set to [%d - %d] \n", 
					(range) ? range->from : -1, (range) ? range->to : -1));
		if (!range || (range->from == 0 && range->to == 0)) {
			m_bDoPrintRange		= UT_FALSE;
			m_bDoPrintSelection = UT_FALSE;
		}
		else if (range->from == -1 && range->to == -1) {
			m_bDoPrintRange		= UT_FALSE;
			m_bDoPrintSelection = UT_TRUE;
		}
		else {	//Must be a range in 1[-2][,3-6][,10-] notation
			m_bDoPrintRange = UT_TRUE;
			m_bDoPrintSelection = UT_FALSE;
			//Punt for now only accept %d-%d format
			first = range->from;
			last = range->to;
			UT_DEBUGMSG(("Got range from %d to %d \n", first, last));
		}

		m_bDoPrintToFile	= UT_FALSE;	//Let photon take care of this
		m_bCollate			= UT_FALSE; //Pp_PC_COLLATING_MODE
		
		PpGetPC(m_pPrintContext, Pp_PC_COPIES, (const void **)&option);
		m_nCopies			= __max(strtoul(option, NULL, 10), 1);
		UT_DEBUGMSG(("Printing %d copies [%s] \n", m_nCopies, option));

		if (m_bDoPrintRange) {
			if (first < m_persistPrintDlg.nMinPage) {
				first = m_persistPrintDlg.nMinPage;
			}

			if (last > m_persistPrintDlg.nMaxPage) {
				last = m_persistPrintDlg.nMaxPage;
			}
			
			m_nFirstPage = __min(first,last);
			m_nLastPage = __max(first,last);
		}
	}
}
