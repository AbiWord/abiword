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

/*****************************************************************/
AP_Dialog * XAP_BeOSDialog_Print::static_constructor(AP_DialogFactory * pFactory,
													 AP_Dialog_Id id)
{
	XAP_BeOSDialog_Print * p = new XAP_BeOSDialog_Print(pFactory,id);
	return p;
}

XAP_BeOSDialog_Print::XAP_BeOSDialog_Print(AP_DialogFactory * pDlgFactory,
										   AP_Dialog_Id id)
	: XAP_Dialog_Print(pDlgFactory,id)
{
	memset(&m_persistPrintDlg, 0, sizeof(m_persistPrintDlg));
}

XAP_BeOSDialog_Print::~XAP_BeOSDialog_Print(void)
{
}

void XAP_BeOSDialog_Print::useStart(void)
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

void XAP_BeOSDialog_Print::useEnd(void)
{
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

GR_Graphics * XAP_BeOSDialog_Print::getPrinterGraphicsContext(void)
{
	UT_ASSERT(m_answer == a_OK);

	return NULL;
}

void XAP_BeOSDialog_Print::releasePrinterGraphicsContext(GR_Graphics * pGraphics)
{
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
		m_answer = a_OK;
		_getGraphics();
	}
	else
	{
		_raisePrintDialog(pFrame);
		if (m_answer == a_OK)
			_getGraphics();
	}

	m_pBeOSFrame = NULL;
	return;
}

void XAP_BeOSDialog_Print::_raisePrintDialog(XAP_Frame * pFrame)
{
	return;
}

void XAP_BeOSDialog_Print::_getGraphics(void)
{
	UT_ASSERT(m_answer == a_OK);
	
	if (m_bDoPrintToFile)
	{
		// we construct a suggested pathname for the print-to-file pathname.
		// we append a .print to the string.  it would be better to append
		// a .ps or whatever, but we don't know what the technology/language
		// of the device is....
		
		char bufSuggestedName[1030];
		memset(bufSuggestedName,0,sizeof(bufSuggestedName));

		sprintf(bufSuggestedName,"%s.ps",m_szDocumentPathname);
		if (!_getPrintToFilePathname(m_pBeOSFrame,bufSuggestedName))
			goto Fail;

		/*
		m_pPSGraphics = new PS_Graphics(m_szPrintToFilePathname, m_szDocumentTitle,
										m_pBeOSFrame->getApp()->getApplicationName(),
										UT_TRUE);
		*/
	}
	else
	{
		// TODO use a POPEN style constructor to get the graphics....
		/*	
		m_pPSGraphics = new PS_Graphics(m_szPrintCommand, m_szDocumentTitle,
										m_pBeOSFrame->getApp()->getApplicationName(),
										UT_FALSE);
		*/
	}

	m_answer = a_OK;
	return;

Fail:
	m_answer = a_CANCEL;
	return;
}

