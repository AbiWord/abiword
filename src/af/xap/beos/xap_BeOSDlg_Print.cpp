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
#include <fstream.h>
#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "xap_Dialog_Id.h"
#include "xap_BeOSDlg_Print.h"
#include "xap_BeOSApp.h"
#include "xap_BeOSFrame.h"
#include "gr_BeOSGraphics.h"

#include <PrintJob.h>

XAP_Dialog * XAP_BeOSDialog_Print::static_constructor(XAP_DialogFactory * pFactory, 
						      XAP_Dialog_Id id)
{
	XAP_BeOSDialog_Print * p = new XAP_BeOSDialog_Print(pFactory,id);
	return p;
}

XAP_BeOSDialog_Print::XAP_BeOSDialog_Print(XAP_DialogFactory * pDlgFactory,
					   XAP_Dialog_Id id)
	: XAP_Dialog_Print(pDlgFactory,id)
{
	memset(&m_persistPrintDlg, 0, sizeof(m_persistPrintDlg));
	m_pBeOSFrame = NULL;
}

XAP_BeOSDialog_Print::~XAP_BeOSDialog_Print(void)
{
}

void XAP_BeOSDialog_Print::useStart(void)
{
	XAP_Dialog_Print::useStart();

	if (m_bPersistValid)
	{
		printf("PRINT: UseStart ... persist valid \n");
		m_persistPrintDlg.bDoPageRange = m_bDoPrintRange;
		m_persistPrintDlg.bDoPrintSelection = m_bDoPrintSelection;
		m_persistPrintDlg.bDoPrintToFile = m_bDoPrintToFile;
		m_persistPrintDlg.bDoCollate = m_bCollate;
	}
}

void XAP_BeOSDialog_Print::useEnd(void)
{
	printf("PRINT: UseEnd ... \n");
	XAP_Dialog_Print::useEnd();

	m_persistPrintDlg.bDoPageRange = m_bDoPrintRange;
	m_persistPrintDlg.bDoPrintSelection = m_bDoPrintSelection;
	m_persistPrintDlg.bDoPrintToFile = m_bDoPrintToFile;
	m_persistPrintDlg.bDoCollate = m_bCollate;
	m_persistPrintDlg.nCopies = m_nCopies;
	m_persistPrintDlg.nFromPage = m_nFirstPage;
	m_persistPrintDlg.nToPage = m_nLastPage;

	UT_cloneString(m_persistPrintDlg.szPrintCommand, m_szPrintCommand);
}

GR_Graphics * XAP_BeOSDialog_Print::getPrinterGraphicsContext(void) {
	//Should I create a new context for this ???
	printf("PRINT: Get PrinterGraphicsContext Frame 0x%x\n", m_pBeOSFrame);
	UT_ASSERT(m_answer == a_OK);
	UT_ASSERT(m_pBeOSFrame);

	printf("PRINT: Returning Graphics 0x%x \n", m_pBeOSFrame->Graphics());
	return(m_pBeOSFrame->Graphics());
}

void XAP_BeOSDialog_Print::releasePrinterGraphicsContext(GR_Graphics * pGraphics) {
	printf("PRINT: Release PrinterGraphicsContext \n");
}

/*****************************************************************/

void XAP_BeOSDialog_Print::runModal(XAP_Frame * pFrame)
{
	m_pBeOSFrame = static_cast<XAP_BeOSFrame *>(pFrame);
	UT_ASSERT(m_pBeOSFrame);
	
	// see if they just want the properties of the printer without
	// bothering the user.
	if (m_bPersistValid && m_bBypassActualDialog)
	{
		printf("PRINT: Run modal with bypass/persist active \n");
		m_answer = a_OK;
	}
	else
	{
		printf("PRINT: Run modal with bypass/persist inactive \n");
		_raisePrintDialog(pFrame);
	}

	return;
}

void XAP_BeOSDialog_Print::_raisePrintDialog(XAP_Frame * pFrame)
{
	BPrintJob 	*job = new BPrintJob("Thomas Add Document Name");
	GR_BeOSGraphics *gr = (GR_BeOSGraphics *)m_pBeOSFrame->Graphics();
	BMessage 	*msg;

	UT_ASSERT(job);
	UT_ASSERT(gr);
	
	if (!(msg = gr->GetPrintSettings())) {
		msg = new BMessage();
	}
	else {
		job->SetSettings(msg);
	}

	//Get the user to configure the page
	if (job->ConfigPage() != B_OK) {
		delete job;
		m_answer = a_CANCEL;
		return;
	}
	msg = job->Settings();

	//Configure the print job
	if (job->ConfigJob() != B_OK) {
		delete job;
		m_answer = a_CANCEL;
		return;
	}

	printf("PRINT: Status of the variables: \n");
	printf("m_EnablePrintToFile %d \n", m_bEnablePrintToFile);	
	printf("m_EnablePageRange %d \n", m_bEnablePageRange);	
	printf("m_EnablePrintSelection %d \n", m_bEnablePrintSelection);	
	printf("First %d - Last %d page \n", m_nFirstPage, m_nLastPage);	
	m_nFirstPage = max_c(job->FirstPage(), m_nFirstPage);
	m_nLastPage = min_c(job->LastPage(), m_nLastPage);

	printf("REAL First %d - Last %d page \n", m_nFirstPage, m_nLastPage);	
	UT_cloneString(m_szPrintCommand, "I'm not here");

	gr->SetPrintSettings(msg);
	gr->SetPrintJob(job);

	m_answer = a_OK;
}

