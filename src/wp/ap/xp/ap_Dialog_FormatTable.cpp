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
	m_leftColor(NULL),
	m_rightColor(NULL),
	m_topColor(NULL),
	m_bottomColor(NULL),
	
	m_lineStyle(LS_NORMAL),
	m_leftStyle(NULL),
	m_rightStyle(NULL),
	m_topStyle(NULL),
	m_bottomStyle(NULL),
	
	m_bgColor(NULL),
	m_bgFillStyle(NULL),
	
	m_answer(a_OK),
	m_pFormatTablePreview(NULL),
	m_iCellSource(0),
    m_iCellDestination(0),
	m_iLeftStyle(0),
	m_iRightStyle(0),
	m_iTopStyle(0),
	m_iBottomStyle(0),
    m_iNumRows(0),
    m_iNumCols(0),

	m_pTab(NULL),
	m_pAutoUpdaterMC(NULL),
	m_borderToggled(false),
	m_bDestroy_says_stopupdating(false),
	m_bAutoUpdate_happening_now(false)
{
	UT_String s = UT_String_sprintf("%d", m_lineStyle);	
	
	CLONEP(m_leftStyle, s.c_str());
	CLONEP(m_rightStyle, s.c_str());
	CLONEP(m_topStyle, s.c_str());
	CLONEP(m_bottomStyle, s.c_str());	
	
	if(m_vecProps.getItemCount() > 0)
		m_vecProps.clear();
	  
	if(m_vecPropsRight.getItemCount() > 0)
		m_vecPropsRight.clear();
	  
	if(m_vecPropsBottom.getItemCount() > 0)
		m_vecPropsBottom.clear();
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

void AP_Dialog_FormatTable::addOrReplaceVecProp(UT_Vector &vec,
												const XML_Char * pszProp,
												const XML_Char * pszVal)
{
	UT_sint32 iCount = vec.getItemCount();
	const char * pszV = NULL;
	if(iCount <= 0)
	{
		vec.addItem((void *) pszProp);
		vec.addItem((void *) pszVal);
		return;
	}
	UT_sint32 i = 0;
	for(i=0; i < iCount ; i += 2)
	{
		pszV = (const XML_Char *) vec.getNthItem(i);
		if( (pszV != NULL) && (strcmp( pszV,pszProp) == 0))
			break;
	}
	if(i < iCount)
		vec.setNthItem(i+1, (void *) pszVal, NULL);
	else
	{
		vec.addItem((void *) pszProp);
		vec.addItem((void *) pszVal);
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
	}
}

void AP_Dialog_FormatTable::applyChanges()
{
	if (m_vecProps.getItemCount() == 0)
		return;
	
// 	 Looking for a fix under win32

    FV_View * pView = (FV_View *) m_pApp->getLastFocussedFrame()->getCurrentView();
	const XML_Char ** propsArray  = new const XML_Char * [m_vecProps.getItemCount()+1];
	propsArray[m_vecProps.getItemCount()] = NULL;
	
	UT_sint32 i = m_vecProps.getItemCount();
	UT_sint32 j;
	for(j= 0; j<i; j=j+2)
	{
		propsArray[j] = (XML_Char *) m_vecProps.getNthItem(j);
		propsArray[j+1] = (XML_Char *) m_vecProps.getNthItem(j+1);
	}

	pView->setCellFormat(propsArray);
	delete [] propsArray;
}

void AP_Dialog_FormatTable::finalize(void)
{
	stopUpdater();
	modeless_cleanup();
}

/*!
 * Set the color and style of the toggled button
 */
void AP_Dialog_FormatTable::toggleLineType(toggle_button btn, bool enabled)
{
	UT_String cTmp = UT_String_sprintf("%02x%02x%02x", m_borderColor.m_red, m_borderColor.m_grn, m_borderColor.m_blu);	
	UT_String sTmp = UT_String_sprintf("%d", (enabled ? m_lineStyle : LS_OFF));

	switch (btn)
	{
		case toggle_left:
		{
			CLONEP(m_leftColor, cTmp.c_str());
			CLONEP(m_leftStyle, sTmp.c_str());
			addOrReplaceVecProp(m_vecProps, "left-style", m_leftStyle);
			addOrReplaceVecProp(m_vecProps, "left-color", m_leftColor);
		}
		break;
		case toggle_right:
		{	
			CLONEP(m_rightColor, cTmp.c_str());
			CLONEP(m_rightStyle, sTmp.c_str());
			addOrReplaceVecProp(m_vecProps, "right-style", m_rightStyle);
			addOrReplaceVecProp(m_vecProps, "right-color", m_rightColor);
		}
		break;
		case toggle_top:
		{			
			CLONEP(m_topColor, cTmp.c_str());
			CLONEP(m_topStyle, sTmp.c_str());
			addOrReplaceVecProp(m_vecProps, "top-style", m_topStyle);
			addOrReplaceVecProp(m_vecProps, "top-color", m_topColor);
		}
		break;
		case toggle_bottom:
		{			
			CLONEP(m_bottomColor, cTmp.c_str());
			CLONEP(m_bottomStyle, sTmp.c_str());
			addOrReplaceVecProp(m_vecProps, "bot-style", m_bottomStyle);
			addOrReplaceVecProp(m_vecProps, "bot-color", m_bottomColor);
		}
		break;
	}
	
	m_borderToggled = true;
}

void AP_Dialog_FormatTable::setBorderColor(UT_RGBColor clr)
{
	m_borderColor = clr;
	
	if (m_borderToggled)
		return;

	UT_String s = UT_String_sprintf("%02x%02x%02x", clr.m_red, clr.m_grn, clr.m_blu);	
	
	CLONEP(m_leftColor, s.c_str());
	CLONEP(m_rightColor, s.c_str());
	CLONEP(m_topColor, s.c_str());
	CLONEP(m_bottomColor, s.c_str());	

	addOrReplaceVecProp(m_vecProps, "left-color", m_leftColor);
	addOrReplaceVecProp(m_vecProps, "right-color", m_rightColor);
	addOrReplaceVecProp(m_vecProps, "top-color", m_topColor);
	addOrReplaceVecProp(m_vecProps, "bot-color", m_bottomColor);
	
	addOrReplaceVecProp(m_vecPropsRight, "left-color", m_rightColor);
	addOrReplaceVecProp(m_vecPropsBottom, "top-color", m_bottomColor);
}

void AP_Dialog_FormatTable::setBackgroundColor(UT_RGBColor clr)
{
	UT_String bgcol = UT_String_sprintf("%02x%02x%02x", clr.m_red, clr.m_grn, clr.m_blu);
	UT_String bstmp = UT_String_sprintf("%d", FS_FILL);
	
	CLONEP(m_bgColor, bgcol.c_str());
	CLONEP(m_bgFillStyle, bstmp.c_str());
	
	addOrReplaceVecProp(m_vecProps, "bgcolor", m_bgColor);
	addOrReplaceVecProp(m_vecProps, "bg-style", m_bgFillStyle);
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
	UT_Rect pageRect(0, 0, iWidth, iHeight);

	m_gc->clearArea(pageRect.left, pageRect.top, pageRect.width,
					pageRect.height);
	
	UT_RGBColor black(0, 0, 0);
	UT_RGBColor write(0, 0, 0);
	UT_RGBColor lineColor(0, 0, 0);
	m_gc->setLineWidth(1);
	
	int whiteBorder = 20;
	int cornerLength = 5;

//
//  Draw the cell background
//

	if (m_pFormatTable->m_bgColor)
	{
		UT_parseColor(m_pFormatTable->m_bgColor, lineColor);
		m_gc->fillRect(lineColor, 0 + whiteBorder, 0 + whiteBorder, iWidth - 2*whiteBorder, iHeight - 2*whiteBorder);
	}

//
//  Draw the cell corners
//
	
	m_gc->setColor(UT_RGBColor(127,127,127));
	
	// top left corner
	m_gc->drawLine(pageRect.left + whiteBorder - cornerLength, pageRect.top + whiteBorder,
				   pageRect.left + whiteBorder, pageRect.top + whiteBorder);
	m_gc->drawLine(pageRect.left + whiteBorder, pageRect.top + whiteBorder  - cornerLength,
				   pageRect.left + whiteBorder, pageRect.top + whiteBorder);

	// top right corner
	m_gc->drawLine(pageRect.left + pageRect.width - whiteBorder + cornerLength, pageRect.top + whiteBorder,
				   pageRect.left + pageRect.width - whiteBorder, pageRect.top + whiteBorder);
	m_gc->drawLine(pageRect.left + pageRect.width - whiteBorder, pageRect.top + whiteBorder - cornerLength,
				   pageRect.left + pageRect.width - whiteBorder, pageRect.top + whiteBorder);

	// bottom left corner
	m_gc->drawLine(pageRect.left + whiteBorder - cornerLength, pageRect.top + pageRect.height - whiteBorder,
				   pageRect.left + whiteBorder, pageRect.top + pageRect.height - whiteBorder);
	m_gc->drawLine(pageRect.left + whiteBorder, pageRect.top + pageRect.height - whiteBorder + cornerLength,
				   pageRect.left + whiteBorder, pageRect.top + pageRect.height - whiteBorder);

	// bottom right corner
	m_gc->drawLine(pageRect.left + pageRect.width - whiteBorder + cornerLength, pageRect.top + pageRect.height - whiteBorder,
				   pageRect.left + pageRect.width - whiteBorder, pageRect.top + pageRect.height - whiteBorder);
	m_gc->drawLine(pageRect.left + pageRect.width - whiteBorder, pageRect.top + pageRect.height - whiteBorder + cornerLength,
				   pageRect.left + pageRect.width - whiteBorder, pageRect.top + pageRect.height - whiteBorder);

//
//  Draw the cell borders
//
	UT_String lineStyleString = UT_String_sprintf("%d", LS_OFF);	
	
	// top border
	if (m_pFormatTable->m_topStyle && strcmp(m_pFormatTable->m_topStyle, lineStyleString.c_str()))
	{
		if (m_pFormatTable->m_topColor)
		{
			UT_parseColor(m_pFormatTable->m_topColor, lineColor);
			m_gc->setColor(lineColor);
		}
		else
			m_gc->setColor(black);
		m_gc->drawLine(pageRect.left + whiteBorder, pageRect.top + whiteBorder,
					   pageRect.left + pageRect.width - whiteBorder, pageRect.top + whiteBorder);
	}

	// left border
	if (m_pFormatTable->m_leftStyle && strcmp(m_pFormatTable->m_leftStyle, lineStyleString.c_str()))
	{
		if (m_pFormatTable->m_leftColor)
		{
			UT_parseColor(m_pFormatTable->m_leftColor, lineColor);
			m_gc->setColor(lineColor);
		}
		else
			m_gc->setColor(black);
		m_gc->drawLine(pageRect.left + whiteBorder, pageRect.top + whiteBorder,
					   pageRect.left + whiteBorder, pageRect.top + pageRect.height - whiteBorder);
	}
	
	// right border
	if (m_pFormatTable->m_rightStyle && strcmp(m_pFormatTable->m_rightStyle, lineStyleString.c_str()))
	{
		if (m_pFormatTable->m_rightColor)
		{
			UT_parseColor(m_pFormatTable->m_rightColor, lineColor);
			m_gc->setColor(lineColor);
		}
		else
			m_gc->setColor(black);
		m_gc->drawLine(pageRect.left + pageRect.width - whiteBorder, pageRect.top + whiteBorder,
					   pageRect.left + pageRect.width - whiteBorder, pageRect.top + pageRect.height - whiteBorder);
	}
	
	// bottom border
	if (m_pFormatTable->m_bottomStyle && strcmp(m_pFormatTable->m_bottomStyle, lineStyleString.c_str()))
	{
		if (m_pFormatTable->m_bottomColor)
		{
			UT_parseColor(m_pFormatTable->m_bottomColor, lineColor);
			m_gc->setColor(lineColor);
		}
		else
			m_gc->setColor(black);
		m_gc->drawLine(pageRect.left + whiteBorder, pageRect.top + pageRect.height - whiteBorder,
					   pageRect.left + pageRect.width - whiteBorder, pageRect.top + pageRect.height - whiteBorder);
	}
	
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

