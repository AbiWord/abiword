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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ut_assert.h"
#include "ut_string.h"
#include "ut_debugmsg.h"

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

#include "ap_Dialog_FormatTable.h"

AP_Dialog_FormatTable::AP_Dialog_FormatTable(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_Modeless(pDlgFactory,id,"interface/dialogwordcount.html"),
	  m_answer(a_OK),
	  m_iCellSource(0),
	  m_iCellDestination(0),
	  m_lineType(radio_left),
	  m_iLeftStyle(0),
	  m_iRightStyle(0),
	  m_iTopStyle(0),
	  m_iBottomStyle(0),
	  m_iNumRows(0),
	  m_iNumCols(0),
	  m_pTab(NULL),
	  m_pAutoUpdaterMC(NULL),
      m_bDestroy_says_stopupdating(false),
      m_bAutoUpdate_happening_now(false),
	  m_pFormatTablePreview(NULL),
      m_leftColor(NULL),
      m_rightColor(NULL),
      m_topColor(NULL),
      m_bottomColor(NULL)	
{
      if(m_vecProps.getItemCount() > 0)
		  m_vecProps.clear();
}

AP_Dialog_FormatTable::~AP_Dialog_FormatTable(void)
{
	stopUpdater();
	DELETEP(m_pFormatTablePreview);
}

AP_Dialog_FormatTable::tAnswer AP_Dialog_FormatTable::getAnswer(void) const
{
	return m_answer;
}

void AP_Dialog_FormatTable::setActiveFrame(XAP_Frame *pFrame)
{
	notifyActiveFrame(getActiveFrame());
}

void AP_Dialog_FormatTable::ConstructWindowName(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	XML_Char * tmp = NULL;
	UT_uint32 title_width = 26;
	UT_XML_cloneNoAmpersands(tmp, pSS->getValue(AP_STRING_ID_DLG_FormatTableTitle));
	BuildWindowName((char *) m_WindowName,(char*)tmp,title_width);
	FREEP(tmp);
}

void AP_Dialog_FormatTable::startUpdater(void)
{
	m_bDestroy_says_stopupdating = false;
	m_bAutoUpdate_happening_now = false;
	GR_Graphics *pG = NULL;
	m_pAutoUpdaterMC =  UT_Timer::static_constructor(autoUpdateMC,this,pG);
	m_pAutoUpdaterMC->set(500);
	m_pAutoUpdaterMC->start();
}

void AP_Dialog_FormatTable::stopUpdater(void)
{
	if(m_pAutoUpdaterMC == NULL)
	{
		return;
	}
	m_bDestroy_says_stopupdating = true;
	while (m_bAutoUpdate_happening_now == true) 
	  ;
	m_pAutoUpdaterMC->stop();
	DELETEP(m_pAutoUpdaterMC);
	m_pAutoUpdaterMC = NULL;
}
/*!
 * Autoupdater of the dialog.
 */
void AP_Dialog_FormatTable::autoUpdateMC(UT_Worker * pTimer)
{

	UT_ASSERT(pTimer);
	
// this is a static callback method and does not have a 'this' pointer

	AP_Dialog_FormatTable * pDialog = (AP_Dialog_FormatTable *) pTimer->getInstanceData();
	// Handshaking code

	if( pDialog->m_bDestroy_says_stopupdating != true)
	{
		pDialog->m_bAutoUpdate_happening_now = true;
		pDialog->setAllSensitivities();
		pDialog->m_bAutoUpdate_happening_now = false;
	}
}        

void AP_Dialog_FormatTable::addOrReplaceVecProp(const XML_Char * pszProp,
												const XML_Char * pszVal)
{
	UT_sint32 iCount = m_vecProps.getItemCount();
	const char * pszV = NULL;
	if(iCount <= 0)
	{
		m_vecProps.addItem((void *) pszProp);
		m_vecProps.addItem((void *) pszVal);
		return;
	}
	UT_sint32 i = 0;
	for(i=0; i < iCount ; i += 2)
	{
		pszV = (const XML_Char *) m_vecProps.getNthItem(i);
		if( (pszV != NULL) && (strcmp( pszV,pszProp) == 0))
			break;
	}
	if(i < iCount)
		m_vecProps.setNthItem(i+1, (void *) pszVal, NULL);
	else
	{
		m_vecProps.addItem((void *) pszProp);
		m_vecProps.addItem((void *) pszVal);
	}
	return;
}

/*! 
 * This method sets the sensitivity of the radio buttons to top/bottom/left/right line buttons
 *
 * Call this right after contructing the widget and before dropping into the main loop.
 */
void AP_Dialog_FormatTable::setAllSensitivities(void)
{
    FV_View * pView = (FV_View *) m_pApp->getLastFocussedFrame()->getCurrentView();
	m_iCellSource = pView->getPoint();

	setSensitivity(pView->isInTable());

	if (pView->isInTable())
	{
		pView->getCellLineStyle(m_iCellSource,&m_iLeftStyle,&m_iRightStyle,&m_iTopStyle,&m_iBottomStyle);
		
	/*	setSensitivity(radio_top, m_iTopStyle != 0); // style is -1 if not set at all...
		setSensitivity(radio_bottom, m_iBottomStyle != 0);
		setSensitivity(radio_left, m_iLeftStyle != 0);
		setSensitivity(radio_right, m_iRightStyle != 0);*/
	}
}

void AP_Dialog_FormatTable::applyChanges()
{
	if (m_vecProps.getItemCount() == 0)
		return;
	
    FV_View * pView = (FV_View *) m_pApp->getLastFocussedFrame()->getCurrentView();
	const XML_Char * propsArray[m_vecProps.getItemCount()+1];
	propsArray[m_vecProps.getItemCount()] = NULL;
	
	UT_sint32 i = m_vecProps.getItemCount();
	UT_sint32 j;
	for(j= 0; j<i; j=j+2)
	{
		propsArray[j] = (XML_Char *) m_vecProps.getNthItem(j);
		propsArray[j+1] = (XML_Char *) m_vecProps.getNthItem(j+1);
	}

	pView->setCellFormat(propsArray);
}

void AP_Dialog_FormatTable::finalize(void)
{
	stopUpdater();
	modeless_cleanup();
}

/*!
 * Set the merge Type
 */
void AP_Dialog_FormatTable::setLineType(lineEnable iLineType)
{
	m_lineType = iLineType;
}

void AP_Dialog_FormatTable::setBorderColor(UT_RGBColor clr)
{
	UT_String s = UT_String_sprintf("%02x%02x%02x", clr.m_red, clr.m_grn, clr.m_blu);
	
	CLONEP(m_leftColor, s.c_str());
	CLONEP(m_rightColor, s.c_str());
	CLONEP(m_topColor, s.c_str());
	CLONEP(m_bottomColor, s.c_str());
	
	addOrReplaceVecProp("left-color", m_leftColor);
	addOrReplaceVecProp("right-color", m_rightColor);
	addOrReplaceVecProp("top-color", m_topColor);
	addOrReplaceVecProp("bot-color", m_bottomColor);
}

/*!
 * Call this method after pressing OK to read out the radio buttons and store results in the
 * output variables
 *
 * It actuall calculates the positions of the source and destination positions of the cells
 * to be merged.
 */
void AP_Dialog_FormatTable::_generateSrcDest(void)
{
	/*PT_DocPosition swap = 0;
	FV_View * pView = (FV_View *) m_pApp->getLastFocussedFrame()->getCurrentView();
	if(m_lineType == radio_left)
	{
		m_iCellDestination = pView->findCellPosAt(m_iCellSource,m_iTop,m_iLeft-1)+1;
	}
	if(m_lineType == radio_right)
	{
		m_iCellDestination = pView->findCellPosAt(m_iCellSource,m_iTop,m_iRight)+1;
	}
	if(m_lineType == radio_top)
	{
		m_iCellDestination = pView->findCellPosAt(m_iCellSource,m_iTop-1,m_iLeft)+1;
	}
	if(m_lineType == radio_bottom)
	{
		m_iCellDestination = pView->findCellPosAt(m_iCellSource,m_iBot,m_iLeft)+1;
	}
	if(m_iCellDestination > m_iCellSource)
	{
		swap = m_iCellDestination;
		m_iCellDestination = m_iCellSource;
		m_iCellSource = swap;
	}
	return;*/
}

PT_DocPosition AP_Dialog_FormatTable::getCellSource(void)
{
	return m_iCellSource;
}

PT_DocPosition AP_Dialog_FormatTable::getCellDestination(void)
{
	return m_iCellDestination;
}

void AP_Dialog_FormatTable::_createPreviewFromGC(GR_Graphics * gc,
											     UT_uint32 width,
											     UT_uint32 height)
{
	UT_ASSERT(gc);

	m_pFormatTablePreview = new AP_FormatTable_preview(gc,this);
	UT_ASSERT(m_pFormatTablePreview);

	m_pFormatTablePreview->setWindowSize(width, height);
}



//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

AP_FormatTable_preview::AP_FormatTable_preview(GR_Graphics * gc, AP_Dialog_FormatTable * pFormatTable)
	: XAP_Preview(gc)
{
	m_pFormatTable = pFormatTable;
}

AP_FormatTable_preview::~AP_FormatTable_preview()
{
}

void AP_FormatTable_preview::draw(void)
{
	UT_sint32 iWidth = getWindowWidth();
	UT_sint32 iHeight = getWindowHeight();
    //double maxHeightPercent = m_pColumns->getMaxHeightPercent();
	//double SpacePercent = m_pColumns->getSpaceAfterPercent();
	UT_Rect pageRect(5, 5, iWidth - 10, iHeight - 10);

	m_gc->fillRect(GR_Graphics::CLR3D_Background, 0, 0, iWidth, iHeight);
	m_gc->clearArea(pageRect.left, pageRect.top, pageRect.width,
					pageRect.height);

	
	UT_RGBColor lineColor(0, 0, 0);
	m_gc->setLineWidth(1);
	
	if (m_pFormatTable->m_topColor)
		UT_parseColor(m_pFormatTable->m_topColor, lineColor);
	m_gc->setColor(lineColor);
	m_gc->drawLine(pageRect.left+10, pageRect.top+10,
				   pageRect.left + pageRect.width - 10, pageRect.top + 10);
	
	if (m_pFormatTable->m_leftColor)
		UT_parseColor(m_pFormatTable->m_leftColor, lineColor);
	m_gc->setColor(lineColor);
	m_gc->drawLine(pageRect.left + 10, pageRect.top + 10,
				   pageRect.left + 10, pageRect.top + pageRect.height - 10);
				   
	if (m_pFormatTable->m_rightColor)
		UT_parseColor(m_pFormatTable->m_rightColor, lineColor);
	m_gc->setColor(lineColor);
	m_gc->drawLine(pageRect.left + pageRect.width - 10, pageRect.top + 10,
				   pageRect.left + pageRect.width - 10, pageRect.top + pageRect.height - 10);
				   
	if (m_pFormatTable->m_bottomColor)	
		UT_parseColor(m_pFormatTable->m_bottomColor, lineColor);
	m_gc->setColor(lineColor);
	m_gc->drawLine(pageRect.left + 10, pageRect.top + pageRect.height - 10,
				   pageRect.left + pageRect.width - 10, pageRect.top + pageRect.height - 10);
	
	//pageRect.top += 5;
	//pageRect.height -= 5;
	//m_previewDrawer.draw(m_gc, pageRect);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void AP_FormatTable_preview_drawer::draw(GR_Graphics *gc, UT_Rect &rect)
{

	/*UT_sint32 iHalfColumnGap = rect.width / 20;

	UT_sint32 y_start = rect.top + iHalfColumnGap;
	UT_sint32 y_end = rect.top + rect.height - iHalfColumnGap;

	UT_sint32 y_step = (y_end - y_start) / iLinesToDraw;
	y_step = 2;
	UT_DEBUGMSG(("SEVIOR: maxheightpercent = %f \n",maxHeightPercent));
	maxHeightPercent = maxHeightPercent/100.0;
	SpacePercent = SpacePercent/100.0;
	if(maxHeightPercent < 0.01)
	{
		maxHeightPercent = 1.1;
	}
	gc->setLineWidth(1);
	UT_RGBColor Line_color(0, 0, 0);
	gc->setColor(Line_color);

	rect.left += iHalfColumnGap;
	rect.width -= 2 * iHalfColumnGap;
	double d_ysize = (double) (y_end - y_start);
	UT_sint32 iSpace = static_cast<UT_sint32>(SpacePercent* d_ysize);
	if(iSpace < y_step)
	{
		iSpace = y_step;
	}
	UT_sint32 maxHeight = static_cast<UT_sint32>(maxHeightPercent * d_ysize);
	for (UT_sint32 i = 1; i <= iColumns; i++)
	{
		UT_sint32 curskip = 0;
		for(UT_sint32 y = y_start; y < y_end; y += y_step)
		{
			UT_sint32 xLeft, xRight;

			// a little bit of math to avoid/replace a (nasty) switch statement
			xLeft = rect.left + iHalfColumnGap + ((i-1) * rect.width/iColumns);
			xRight = rect.left - iHalfColumnGap + (i * rect.width / iColumns);
			curskip += y_step;
			if(curskip >= maxHeight )
			{
					curskip = 0;
					y += iSpace;
			}
			gc->drawLine(xLeft, y, xRight, y);
		}
	}

	if(bLineBetween)
	{
		// a bit of math to avoid/replace a (nasty) switch statement
		for (UT_sint32 j = 2; j <= iColumns; j++)
		{
			UT_sint32 x;

			x = rect.left + (j-1) * rect.width / iColumns;
			gc->drawLine(x, y_start, x, y_end);
		}
	}*/
}

