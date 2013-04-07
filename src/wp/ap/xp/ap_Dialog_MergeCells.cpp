/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ap_Features.h"

#include "ut_assert.h"
#include "ut_string.h"
#include "ut_debugmsg.h"

#include "xap_App.h"
#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"
#include "fv_View.h"
#include "pd_Document.h"
#include "pt_Types.h"
#include "fp_Line.h"
#include "fp_Run.h"
#include "fp_ContainerObject.h"
#include "fp_TableContainer.h"
#include "fl_TableLayout.h"
#include "fl_BlockLayout.h"
#include "fl_DocLayout.h"
#include "ut_timer.h"

#include "ap_Dialog_MergeCells.h"
#include "ap_Strings.h"

AP_Dialog_MergeCells::AP_Dialog_MergeCells(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_Modeless(pDlgFactory,id, "interface/dialogmergecells"),
	  m_answer(a_OK),
	  m_iCellSource(0),
	  m_iCellDestination(0),
	  m_mergeType(radio_left),
	  m_iLeft(0),
	  m_iRight(0),
	  m_iTop(0),
	  m_iBot(0),
	  m_iNumRows(0),
	  m_iNumCols(0),
	  m_pTab(NULL),
	  m_pAutoUpdaterMC(NULL),
      m_bDestroy_says_stopupdating(false),
      m_bAutoUpdate_happening_now(false)

{
}

AP_Dialog_MergeCells::~AP_Dialog_MergeCells(void)
{
	stopUpdater();
}

AP_Dialog_MergeCells::tAnswer AP_Dialog_MergeCells::getAnswer(void) const
{
	return m_answer;
}

void    AP_Dialog_MergeCells::setActiveFrame(XAP_Frame * /*pFrame*/)
{
	notifyActiveFrame(getActiveFrame());
}

void AP_Dialog_MergeCells::ConstructWindowName(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	gchar * tmp = NULL;
	UT_XML_cloneNoAmpersands(tmp, pSS->getValue(AP_STRING_ID_DLG_MergeCellsTitle));
	BuildWindowName(static_cast<char *>(m_WindowName),static_cast<char*>(tmp),sizeof(m_WindowName));
	FREEP(tmp);
}

void AP_Dialog_MergeCells::startUpdater(void)
{
	m_bDestroy_says_stopupdating = false;
	m_bAutoUpdate_happening_now = false;
	m_pAutoUpdaterMC =  UT_Timer::static_constructor(autoUpdateMC,this);
	m_pAutoUpdaterMC->set(500);
	m_pAutoUpdaterMC->start();
}

void AP_Dialog_MergeCells::stopUpdater(void)
{
	if(m_pAutoUpdaterMC == NULL)
	{
		return;
	}
	m_bDestroy_says_stopupdating = true;
	m_pAutoUpdaterMC->stop();
	DELETEP(m_pAutoUpdaterMC);
	m_pAutoUpdaterMC = NULL;
}

/*!
 * Autoupdater of the dialog.
 */
void AP_Dialog_MergeCells::autoUpdateMC(UT_Worker * pTimer)
{
	UT_return_if_fail(pTimer);
	
// this is a static callback method and does not have a 'this' pointer

	AP_Dialog_MergeCells * pDialog = static_cast<AP_Dialog_MergeCells *>(pTimer->getInstanceData());

	if (pDialog->m_bDestroy_says_stopupdating != true)
	{
		FV_View * pView = 0;
		PD_Document * pDoc = NULL;

		if (XAP_Frame * pFrame = pDialog->getApp()->getLastFocussedFrame())
		{
			pView = static_cast<FV_View *>(pFrame->getCurrentView());
		}
		if (pView)
		{
			pDoc = pView->getDocument();
		}
		if (!pView || (pDoc && !pDoc->isPieceTableChanging()))
		{
			pDialog->m_bAutoUpdate_happening_now = true;
			pDialog->setAllSensitivities();
			pDialog->m_bAutoUpdate_happening_now = false;
		}
	}
}        

/*! 
 * This method sets the sensitivity of the radio buttons to above/below/left/right merges
 * Because we can't merge to the left of column zero for example.
 *
 * Call this right after contructing the widget and before dropping into the main loop.
 */
void AP_Dialog_MergeCells::setAllSensitivities(void)
{
	FV_View * pView = 0;

	if (XAP_Frame * pFrame = getApp()->getLastFocussedFrame())
	{
		pView = static_cast<FV_View *>(pFrame->getCurrentView());
	}
	if (!pView)
	{
		setSensitivity(radio_left,  false);
		setSensitivity(radio_right, false);
		setSensitivity(radio_above, false);
		setSensitivity(radio_below, false);
		return;
	}
	if (!pView->isInTable())
	{
		setSensitivity(radio_left,  false);
		setSensitivity(radio_right, false);
		setSensitivity(radio_above, false);
		setSensitivity(radio_below, false);
		return;
	}

	PT_DocPosition iCurPos = pView->getPoint();
	m_iCellSource = iCurPos;
	pView->getCellParams(iCurPos,&m_iLeft,&m_iRight,&m_iTop,&m_iBot);
//
// Now find the number of rows and columns inthis table. This is easiest to
// get from the table container
//
	fl_BlockLayout * pBL =	pView->getLayout()->findBlockAtPosition(iCurPos);
	fp_Run * pRun;
	UT_sint32 xPoint,yPoint,xPoint2,yPoint2,iPointHeight;
	bool bDirection;
	pRun = pBL->findPointCoords(iCurPos, false, xPoint,
							    yPoint, xPoint2, yPoint2,
							    iPointHeight, bDirection);

	UT_return_if_fail(pRun);

	fp_Line * pLine = pRun->getLine();
	UT_return_if_fail(pLine);

	fp_Container * pCon = pLine->getContainer();
	UT_return_if_fail(pCon);
	if(pCon->getContainerType() != FP_CONTAINER_CELL)
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		setSensitivity(radio_above,false);
		setSensitivity(radio_below,false);
		setSensitivity(radio_left,false);
		setSensitivity(radio_right,false);
		return;
	}
	fp_TableContainer * pTab = static_cast<fp_TableContainer *>(pCon->getContainer());
	UT_return_if_fail(pTab);
	UT_return_if_fail(pTab->getContainerType() == FP_CONTAINER_TABLE);
	m_pTab = pTab;
	m_iNumRows = pTab->getNumRows();
	m_iNumCols = pTab->getNumCols();
	if(m_iTop >0)
	{
		setSensitivity(radio_above,true);
	}
	else
	{
		setSensitivity(radio_above,false);
	}
	if(m_iBot < m_iNumRows)
	{
		setSensitivity(radio_below,true);
	}
	else
	{
		setSensitivity(radio_below,false);
	}
	if(m_iLeft > 0)
	{
		setSensitivity(radio_left,true);
	}
	else
	{
		setSensitivity(radio_left,false);
	}
	if(m_iRight < m_iNumCols)
	{
		setSensitivity(radio_right,true);
	}
	else
	{
		setSensitivity(radio_right,false);
	}
}

void AP_Dialog_MergeCells::finalize(void)
{
	stopUpdater();
	modeless_cleanup();
}

/*!
 * Set the merge Type
 */
void AP_Dialog_MergeCells::setMergeType( mergeWithCell iMergeType)
{
	m_mergeType = iMergeType;
}
/*!
 * Call this method after pressing OK to read out the radio buttons and store results in the
 * output variables
 *
 * It actuall calculates the positions of the source and destination positions of the cells
 * to be merged.
 */
void AP_Dialog_MergeCells::_generateSrcDest(void)
{
	PT_DocPosition swap = 0;
	FV_View * pView = static_cast<FV_View *>(m_pApp->getLastFocussedFrame()->getCurrentView());
	if(m_mergeType == radio_left)
	{
		m_iCellDestination = pView->findCellPosAt(m_iCellSource,m_iTop,m_iLeft-1)+1;
	}
	if(m_mergeType == radio_right)
	{
		m_iCellDestination = pView->findCellPosAt(m_iCellSource,m_iTop,m_iRight)+1;
	}
	if(m_mergeType == radio_above)
	{
		m_iCellDestination = pView->findCellPosAt(m_iCellSource,m_iTop-1,m_iLeft)+1;
	}
	if(m_mergeType == radio_below)
	{
		m_iCellDestination = pView->findCellPosAt(m_iCellSource,m_iBot,m_iLeft)+1;
	}
	if(m_iCellDestination > m_iCellSource)
	{
		swap = m_iCellDestination;
		m_iCellDestination = m_iCellSource;
		m_iCellSource = swap;
	}
	return;
}

/*!
 * Method to actually do the cell merge.
 */
void AP_Dialog_MergeCells::onMerge(void)
{
	FV_View * pView = 0;

	if (XAP_Frame * pFrame = getApp()->getLastFocussedFrame())
	{
		pView = static_cast<FV_View *>(pFrame->getCurrentView());
	}
	if (pView)
	{
		_generateSrcDest();
		pView->cmdMergeCells(m_iCellSource, m_iCellDestination);
	}
	setAllSensitivities();
}

PT_DocPosition AP_Dialog_MergeCells::getCellSource(void)
{
	return m_iCellSource;
}

PT_DocPosition AP_Dialog_MergeCells::getCellDestination(void)
{
	return m_iCellDestination;
}
