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
	: XAP_Dialog_Modeless(pDlgFactory,id),
	
	  m_lineStyle(LS_NORMAL),
	  m_bgFillStyle(NULL),
	  m_answer(a_OK),
	  m_pFormatTablePreview(NULL),
	  m_bSettingsChanged(false),

	  m_pAutoUpdaterMC(NULL),
	  m_borderToggled(false),
	  m_bDestroy_says_stopupdating(false),
	  m_bAutoUpdate_happening_now(false),
	  m_iOldPos(0)
{
	if(m_vecProps.getItemCount() > 0)
		m_vecProps.clear();
	  
	if(m_vecPropsAdjRight.getItemCount() > 0)
		m_vecPropsAdjRight.clear();
	  
	if(m_vecPropsAdjBottom.getItemCount() > 0)
		m_vecPropsAdjBottom.clear();
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
	BuildWindowName(static_cast<char *>(m_WindowName),static_cast<char*>(tmp),title_width);
	FREEP(tmp);
}

void AP_Dialog_FormatTable::startUpdater(void)
{
	m_bDestroy_says_stopupdating = false;
	m_bAutoUpdate_happening_now = false;
	GR_Graphics *pG = NULL;
	m_pAutoUpdaterMC =  UT_Timer::static_constructor(autoUpdateMC,this,pG);
	m_pAutoUpdaterMC->set(100); // use a fast time, so the dialogs behaviour looks "snappy"
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
 Autoupdater of the dialog.
 */
void AP_Dialog_FormatTable::autoUpdateMC(UT_Worker * pTimer)
{
	UT_ASSERT(pTimer);
	
	// get the dialog pointer from the timer instance data
	AP_Dialog_FormatTable * pDialog = static_cast<AP_Dialog_FormatTable *>(pTimer->getInstanceData());

	// Handshaking code
	if( pDialog->m_bDestroy_says_stopupdating != true)
	{
		pDialog->m_bAutoUpdate_happening_now = true;
		pDialog->setAllSensitivities();
		pDialog->setCurCellProps();
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
		char * prop = NULL;
		char * val = NULL;
		CLONEP(prop, pszProp);
		CLONEP(val, pszVal);
		vec.addItem(static_cast<void *>(prop));
		vec.addItem(static_cast<void *>(val));
		return;
	}
	UT_sint32 i = 0;
	for(i=0; i < iCount ; i += 2)
	{
		pszV = reinterpret_cast<const XML_Char *>(vec.getNthItem(i));
		if( (pszV != NULL) && (strcmp( pszV,pszProp) == 0))
			break;
	}
	if(i < iCount)
	{
		char * pVal = static_cast<char *>(vec.getNthItem(i+1));
		FREEP(pVal);
		char * val = NULL;
		CLONEP(val, pszVal);
		vec.setNthItem(i+1, static_cast<void *>(val), NULL);
	}
	else
	{
		char * prop = NULL;
		char * val = NULL;
		CLONEP(prop, pszProp);
		CLONEP(val, pszVal);
		vec.addItem(static_cast<void *>(prop));
		vec.addItem(static_cast<void *>(val));
	}
	return;
}

void AP_Dialog_FormatTable::getVecProp(UT_Vector &vec,
									   const XML_Char * pszProp,
									   const XML_Char * &pszVal)
{
	UT_sint32 iCount = vec.getItemCount();
	const char * pszV = NULL;
	if(iCount <= 0)
	{
		return;
	}
	UT_sint32 i = 0;
	for(i=0; i < iCount ; i += 2)
	{
		pszV = reinterpret_cast<const XML_Char *>(vec.getNthItem(i));
		if( (pszV != NULL) && (strcmp( pszV,pszProp) == 0))
			break;
	}
	if(i < iCount)
	{
		pszVal = reinterpret_cast<const XML_Char *>(vec.getNthItem(i+1));
	}
	return;
}

/*!
 Removes the key,value pair  (pszProp,pszVal) given by pszProp
 from the Vector of all properties of the current format.
 If the Property does not exists nothing happens
 \param UT_Vector &vec the vector to remove the pair from
 \param const XML_Char * pszProp the property name
*/
void AP_Dialog_FormatTable::removeVecProp(UT_Vector &vec, const XML_Char * pszProp)
{
	UT_sint32 iCount = vec.getItemCount();
	const char * pszV = NULL;
	if(iCount <= 0)
	{
		return;
	}
	UT_sint32 i = 0;
	for(i=0; i < iCount ; i += 2)
	{
		pszV = reinterpret_cast<const XML_Char *>(vec.getNthItem(i));
		if( (pszV != NULL) && (strcmp( pszV,pszProp) == 0))
			break;
	}
	if(i < iCount)
	{
		char * pSP = static_cast<char *>(vec.getNthItem(i));
		char * pSV = static_cast<char *>(vec.getNthItem(i+1));
		FREEP(pSP);
		FREEP(pSV);
		vec.deleteNthItem(i+1);
		vec.deleteNthItem(i);
	}
	return;
}

/*! 
 Sets the sensitivity of the radio buttons to top/bottom/left/right line buttons
 Call this right after contructing the widget and before dropping into the main loop.
 */
void AP_Dialog_FormatTable::setAllSensitivities(void)
{
    FV_View * pView = static_cast<FV_View *>(m_pApp->getLastFocussedFrame()->getCurrentView());
	setSensitivity(pView->isInTable());
}

void AP_Dialog_FormatTable::setCurCellProps(void)
{
	
	FV_View * pView = static_cast<FV_View *>(m_pApp->getLastFocussedFrame()->getCurrentView());

	if (m_bSettingsChanged)
		return;
	m_iOldPos = pView->getPoint();

	removeVecProp (m_vecProps, "bg-style");
	removeVecProp (m_vecProps, "bgcolor");

	XML_Char * bgColor = NULL;
	if (pView->getCellBGColor (bgColor))
	{
		addOrReplaceVecProp (m_vecProps, "background-color", bgColor);
	}
	else
	{
		removeVecProp (m_vecProps, "background-color");
	}
	
	// draw the preview with the changed properties
	if(m_pFormatTablePreview)
		m_pFormatTablePreview->draw();
	
	//m_bCurCellFormatDisplayed = TODO;
}

void AP_Dialog_FormatTable::applyChanges()
{
	UT_DEBUGMSG(("Doing apply changes number props %d \n",m_vecProps.getItemCount()));
	if (m_vecProps.getItemCount() == 0)
		return;

    FV_View * pView = static_cast<FV_View *>(m_pApp->getLastFocussedFrame()->getCurrentView());
	const XML_Char ** propsArray  = new const XML_Char * [m_vecProps.getItemCount()+1];
	propsArray[m_vecProps.getItemCount()] = NULL;
	
	UT_sint32 i = m_vecProps.getItemCount();
	UT_sint32 j;
	for(j= 0; j<i; j=j+2)
	{
		propsArray[j] = static_cast<XML_Char *>(m_vecProps.getNthItem(j));
		propsArray[j+1] = static_cast<XML_Char *>(m_vecProps.getNthItem(j+1));
	}

	pView->setCellFormat(propsArray);
	delete [] propsArray;
	
	m_bSettingsChanged = false;
}

void AP_Dialog_FormatTable::finalize(void)
{
	stopUpdater();
	modeless_cleanup();
}

/*!
 Set the color and style of the toggled button
 */
void AP_Dialog_FormatTable::toggleLineType(toggle_button btn, bool enabled)
{
	UT_String cTmp = UT_String_sprintf("%02x%02x%02x", m_borderColor.m_red, m_borderColor.m_grn, m_borderColor.m_blu);	
	UT_String sTmp = UT_String_sprintf("%d", (enabled ? m_lineStyle : LS_OFF));

	switch (btn)
	{
		case toggle_left:
		{
			addOrReplaceVecProp(m_vecProps, "left-style", sTmp.c_str());
			addOrReplaceVecProp(m_vecProps, "left-color", cTmp.c_str());
		}
		break;
		case toggle_right:
		{	
			addOrReplaceVecProp(m_vecProps, "right-style", sTmp.c_str());
			addOrReplaceVecProp(m_vecProps, "right-color", cTmp.c_str());
		}
		break;
		case toggle_top:
		{			
			addOrReplaceVecProp(m_vecProps, "top-style", sTmp.c_str());
			addOrReplaceVecProp(m_vecProps, "top-color", cTmp.c_str());
		}
		break;
		case toggle_bottom:
		{			
			addOrReplaceVecProp(m_vecProps, "bot-style", sTmp.c_str());
			addOrReplaceVecProp(m_vecProps, "bot-color", cTmp.c_str());
		}
		break;
	}
	
	m_borderToggled = true;
	m_bSettingsChanged = true;
}

void AP_Dialog_FormatTable::setBorderColor(UT_RGBColor clr)
{
	m_borderColor = clr;
	
	if (m_borderToggled)
		return;

	UT_String s = UT_String_sprintf("%02x%02x%02x", clr.m_red, clr.m_grn, clr.m_blu);	

	addOrReplaceVecProp(m_vecProps, "left-color", s.c_str());
	addOrReplaceVecProp(m_vecProps, "right-color", s.c_str());
	addOrReplaceVecProp(m_vecProps, "top-color", s.c_str());
	addOrReplaceVecProp(m_vecProps, "bot-color", s.c_str());
	
	addOrReplaceVecProp(m_vecPropsAdjRight, "left-color", s.c_str());
	addOrReplaceVecProp(m_vecPropsAdjBottom, "top-color", s.c_str());
	
	m_bSettingsChanged = true;
}

void AP_Dialog_FormatTable::setBGColor(UT_RGBColor clr)
{
	UT_String bgcol = UT_String_sprintf("%02x%02x%02x", clr.m_red, clr.m_grn, clr.m_blu);

	removeVecProp (m_vecProps, "bg-style");
	removeVecProp (m_vecProps, "bgcolor");

	if (clr.isTransparent ())
		removeVecProp (m_vecProps, "background-color");
	else
		addOrReplaceVecProp (m_vecProps, "background-color", bgcol.c_str ());

	m_bSettingsChanged = true;
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

bool AP_Dialog_FormatTable::getTopToggled()
{
	const XML_Char * pszStyle = NULL;
	UT_String lineStyleString = UT_String_sprintf("%d", LS_OFF);	

	getVecProp(m_vecProps, "top-style", pszStyle);

	if ((pszStyle && strcmp(pszStyle, lineStyleString.c_str())) || 
		!pszStyle)
		return true;
	else
		return false;
}

bool	AP_Dialog_FormatTable::getBottomToggled()
{
	const XML_Char * pszStyle = NULL;
	UT_String lineStyleString = UT_String_sprintf("%d", LS_OFF);	

	getVecProp(m_vecProps, "bot-style", pszStyle);

	if ((pszStyle && strcmp(pszStyle, lineStyleString.c_str())) || 
		!pszStyle)
		return true;
	else
		return false;
	
}

bool	AP_Dialog_FormatTable::getRightToggled()
{
	const XML_Char * pszStyle = NULL;
	UT_String lineStyleString = UT_String_sprintf("%d", LS_OFF);	

	getVecProp(m_vecProps, "right-style", pszStyle);

	if ((pszStyle && strcmp(pszStyle, lineStyleString.c_str())) || 
		!pszStyle)
		return true;
	else
		return false;
}

bool	AP_Dialog_FormatTable::getLeftToggled()
{

	const XML_Char * pszStyle = NULL;
	UT_String lineStyleString = UT_String_sprintf("%d", LS_OFF);	

	getVecProp(m_vecProps, "left-style", pszStyle);

	if ((pszStyle && strcmp(pszStyle, lineStyleString.c_str())) || 
		!pszStyle)
		return true;
	else
		return false;
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
	UT_sint32 iWidth = m_gc->tlu (getWindowWidth());
	UT_sint32 iHeight = m_gc->tlu (getWindowHeight());
	UT_Rect pageRect(m_gc->tlu(7), m_gc->tlu(7), iWidth - m_gc->tlu(14), iHeight - m_gc->tlu(14));	
	
	m_gc->fillRect(GR_Graphics::CLR3D_Background, 0, 0, iWidth, iHeight);
	m_gc->clearArea(pageRect.left, pageRect.top, pageRect.width, pageRect.height);	
	
	
	UT_RGBColor tmpCol;
	
	UT_RGBColor black(0, 0, 0);
	m_gc->setLineWidth(m_gc->tlu(1));
	
	int border = m_gc->tlu(20);
	int cornerLength = m_gc->tlu(5);

//
//  Draw the cell background
//
	
	const XML_Char * pszBGCol = NULL;
	m_pFormatTable->getVecProp (m_pFormatTable->m_vecProps,
				    static_cast<const XML_Char *>("background-color"), pszBGCol);
	if (pszBGCol == NULL)
		m_pFormatTable->getVecProp (m_pFormatTable->m_vecProps,
					    static_cast<const XML_Char *>("bgcolor"), pszBGCol);
	if (pszBGCol && *pszBGCol)
	{
		UT_parseColor(pszBGCol, tmpCol);
		m_gc->fillRect(tmpCol, pageRect.left + border, pageRect.top + border, pageRect.width - 2*border, pageRect.height - 2*border);
	}

//
//  Draw the cell corners
//
	
	m_gc->setColor(UT_RGBColor(127,127,127));
	
	// top left corner
	m_gc->drawLine(pageRect.left + border - cornerLength, pageRect.top + border,
				   pageRect.left + border, pageRect.top + border);
	m_gc->drawLine(pageRect.left + border, pageRect.top + border  - cornerLength,
				   pageRect.left + border, pageRect.top + border);

	// top right corner
	m_gc->drawLine(pageRect.left + pageRect.width - border + cornerLength, pageRect.top + border,
				   pageRect.left + pageRect.width - border, pageRect.top + border);
	m_gc->drawLine(pageRect.left + pageRect.width - border, pageRect.top + border - cornerLength,
				   pageRect.left + pageRect.width - border, pageRect.top + border);

	// bottom left corner
	m_gc->drawLine(pageRect.left + border - cornerLength, pageRect.top + pageRect.height - border,
				   pageRect.left + border, pageRect.top + pageRect.height - border);
	m_gc->drawLine(pageRect.left + border, pageRect.top + pageRect.height - border + cornerLength,
				   pageRect.left + border, pageRect.top + pageRect.height - border);

	// bottom right corner
	m_gc->drawLine(pageRect.left + pageRect.width - border + cornerLength, pageRect.top + pageRect.height - border,
				   pageRect.left + pageRect.width - border, pageRect.top + pageRect.height - border);
	m_gc->drawLine(pageRect.left + pageRect.width - border, pageRect.top + pageRect.height - border + cornerLength,
				   pageRect.left + pageRect.width - border, pageRect.top + pageRect.height - border);

//
//  Draw the cell borders
//

	UT_String lineStyleString = UT_String_sprintf("%d", LS_OFF);	
	
	// top border
	const XML_Char * pszTopStyle = NULL;
	m_pFormatTable->getVecProp(m_pFormatTable->m_vecProps, "top-style", pszTopStyle);
	if ((pszTopStyle && strcmp(pszTopStyle, lineStyleString.c_str())) || 
		!pszTopStyle)
	{
		const XML_Char * pszTopColor = NULL;
		m_pFormatTable->getVecProp(m_pFormatTable->m_vecProps, "top-color", pszTopColor);
		if (pszTopColor)
		{
			UT_parseColor(pszTopColor, tmpCol);
			m_gc->setColor(tmpCol);
		}
		else
			m_gc->setColor(black);
		m_gc->drawLine(pageRect.left + border, pageRect.top + border,
					   pageRect.left + pageRect.width - border, pageRect.top + border);
	}

	// left border
	const XML_Char * pszLeftStyle = NULL;
	m_pFormatTable->getVecProp(m_pFormatTable->m_vecProps, "left-style", pszLeftStyle);
	if ((pszLeftStyle && strcmp(pszLeftStyle, lineStyleString.c_str())) || 
		!pszLeftStyle)
	{
		const XML_Char * pszLeftColor = NULL;
		m_pFormatTable->getVecProp(m_pFormatTable->m_vecProps, "left-color", pszLeftColor);
		if (pszLeftColor)
		{
			UT_parseColor(pszLeftColor, tmpCol);
			m_gc->setColor(tmpCol);
		}
		else
			m_gc->setColor(black);
		m_gc->drawLine(pageRect.left + border, pageRect.top + border,
					   pageRect.left + border, pageRect.top + pageRect.height - border);
	}
	
	// right border
	const XML_Char * pszRightStyle = NULL;
	m_pFormatTable->getVecProp(m_pFormatTable->m_vecProps, "right-style", pszRightStyle);
	if ((pszRightStyle && strcmp(pszRightStyle, lineStyleString.c_str())) || 
		!pszRightStyle)
	{
		const XML_Char * pszRightColor = NULL;
		m_pFormatTable->getVecProp(m_pFormatTable->m_vecProps, "right-color", pszRightColor);
		if (pszRightColor)
		{
			UT_parseColor(pszRightColor, tmpCol);
			m_gc->setColor(tmpCol);
		}
		else
			m_gc->setColor(black);
		m_gc->drawLine(pageRect.left + pageRect.width - border, pageRect.top + border,
					   pageRect.left + pageRect.width - border, pageRect.top + pageRect.height - border);
	}
	
	// bottom border
	const XML_Char * pszBottomStyle = NULL;
	m_pFormatTable->getVecProp(m_pFormatTable->m_vecProps, "bot-style", pszBottomStyle);
	if ((pszBottomStyle && strcmp(pszBottomStyle, lineStyleString.c_str())) || 
		!pszBottomStyle)
	{
		const XML_Char * pszBottomColor = NULL;
		m_pFormatTable->getVecProp(m_pFormatTable->m_vecProps, "bot-color", pszBottomColor);
		if (pszBottomColor)
		{
			UT_parseColor(pszBottomColor, tmpCol);
			m_gc->setColor(tmpCol);
		}
		else
			m_gc->setColor(black);
		m_gc->drawLine(pageRect.left + border, pageRect.top + pageRect.height - border,
					   pageRect.left + pageRect.width - border, pageRect.top + pageRect.height - border);
	}
}




