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
#include "ap_Dialog_Lists.h"
#include "ap_Strings.h"
#include "ut_string.h"
#include "ut_string_class.h"

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"

#include "fl_DocLayout.h"
#include "fv_View.h"
#include "pd_Document.h"
#include "fl_DocLayout.h"
#include "fl_BlockLayout.h"
#include "ap_Preview_Paragraph.h"
#include "xad_Document.h"

AP_Dialog_Lists::AP_Dialog_Lists(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
:	XAP_Dialog_Modeless(pDlgFactory, id),
	m_pView(0),
	m_answer(a_CLOSE),
	m_isListAtPoint(false),
	m_previousListExistsAtPoint(false),
	m_newListType(NOT_A_LIST),
	m_fAlign(0),
	m_fIndent(0),
	m_iLevel(0),
	m_iStartValue(0),
	m_iWidth(0),
	m_iHeight(0),
	m_iLocalTick(0),
	m_curStartValue(0),
	m_newStartValue(0),
	m_curListLevel(0),
	m_newListLevel(0),
	m_iID(0),
	m_iListType(NOT_A_LIST),
	m_bStartList(false),
	m_bStopList(false),
	m_bChangeStartValue(false),
	m_bresumeList(0),
	m_bStartNewList(0),
	m_bApplyToCurrent(0),
	m_bStartSubList(0),
	m_bResumeList(0),
	m_bisCustomized(0),
	m_bguiChanged(false),
	m_paragraphPreview(0),
	m_pListsPreview(0),
	m_pFakeAuto(0),
	m_bDirty(false),
    m_bIsModal(false)
{
	for(UT_uint32 i=0; i<4; i++)
	{
		m_pFakeLayout[i] = NULL;
		m_pFakeSdh[i] = NULL;
	}

	m_WindowName[0] = '\0';
	m_curListType[0] = '\0';
	m_curListLabel[0] = '\0';
	m_newListLabel[0] = '\0';
	m_pszDelim[0] = '\0';
	m_pszDecimal[0] = '\0';
	m_pszFont[0] = '\0';

}

AP_Dialog_Lists::~AP_Dialog_Lists(void)
{
//	DELETEP(m_paragraphPreview);
 	DELETEP(m_pListsPreview);
	for(UT_uint32 i=0; i<4; i++)
	{
		DELETEP(m_pFakeLayout[i]);
	}
	// What do we do about the fakeAutoNum in the Document pDoc?
	// Maybe we need another constrcutor

	DELETEP(m_pFakeAuto);
	
	UNREFP(m_pFakeDoc);
}

AP_Dialog_Lists::tAnswer AP_Dialog_Lists::getAnswer(void) const
{
	// let our caller know if user hit ok, cancel, etc.
	return m_answer;
}

/************************************************************************/

// how many characters do we want to pull from the current paragraph
// to fill our preview

#define NUM_CHARS_FOR_SAMPLE 100

void AP_Dialog_Lists::_createPreviewFromGC(GR_Graphics* gc,
										   UT_uint32 width,
										   UT_uint32 height)
{
	UT_ASSERT(gc);

	m_iWidth = width;
	m_iHeight = height;

// free any attached preview

	DELETEP(m_pListsPreview);
	
	m_pListsPreview = new AP_Lists_preview(gc, this);
	UT_ASSERT(m_pListsPreview);
	
	m_pListsPreview->setWindowSize(width, height);
	//
	// Generate the fake layout pointers and autonum we need for the 
	// preview
	//
	generateFakeLabels();
	m_isListAtPoint = getBlock()->isListItem();
	if(m_isListAtPoint == false)
	{
		m_newListType = NOT_A_LIST;
	}

	// Mike: I added the "#if 0" to make it perfectly clear that
	// this code is not currently used.
	// Please remove this comment if this code gets reactivated.
#if 0
	// This is Thomas Code. I'll do a fake preview instead
// Martin
// free any attached preview

	DELETEP(m_paragraphPreview);

	UT_GrowBuf gb;
	bool hadMem = getBlock()->getBlockBuf(&gb);

	UT_UCSChar * tmp = NULL;
	if (hadMem && gb.getLength() > 0)
	{
		gb.truncate(NUM_CHARS_FOR_SAMPLE);
		UT_UCS_cloneString(&tmp, (UT_UCSChar *) gb.getPointer(0));
	}
	else
	{
		const XAP_StringSet * pSS = m_pApp->getStringSet();
	
		// if the paragraph was empty, use our sample
		UT_UCS_cloneString_char(&tmp, pSS->getValue(AP_STRING_ID_DLG_Para_PreviewSampleFallback));
	}

	m_paragraphPreview = new AP_Preview_Paragraph(gc, tmp, this);

	FREEP(tmp);
	
	UT_ASSERT(m_paragraphPreview);
	
	m_paragraphPreview->setWindowSize(width, height);
#endif

	// TODO : any setup of the GC for drawing
}

void AP_Dialog_Lists::event_PreviewAreaExposed(void)
{
	if (m_pListsPreview) 
	{
		fillFakeLabels();
		m_pListsPreview->draw();
	}
	else 
	{
		UT_ASSERT(0);
	}
}

void AP_Dialog_Lists::StartList(void)
{
	UT_ASSERT(!IS_NONE_LIST_TYPE(m_iListType));
	getBlock()->listUpdate();
	const XML_Char* pStyle = getBlock()->getListStyleString(m_iListType);
	UT_ASSERT(pStyle);
	getView()->cmdStartList(pStyle);
}


void AP_Dialog_Lists::StopList(void)
{
	getBlock()->listUpdate();
	getView()->cmdStopList();
}

fl_AutoNum * AP_Dialog_Lists::getAutoNum(void)
{
	return getBlock()->getAutoNum();
}


fl_BlockLayout * AP_Dialog_Lists::getBlock(void)
{
	return getView()->getCurrentBlock();
}

UT_uint32 AP_Dialog_Lists::getTick(void)
{
	return m_iLocalTick;
}

void AP_Dialog_Lists::setTick(UT_uint32 iTick)
{
	m_iLocalTick = iTick;
}

void AP_Dialog_Lists::Apply(void)
{
	XML_Char szStart[20];
	if(m_newListType == BULLETED_LIST || m_newListType == IMPLIES_LIST)
	{
		UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *) "Symbol");
	}
	else if(m_newListType > DASHED_LIST)
	{
		UT_XML_strncpy( (XML_Char *) m_pszFont, 80, _getDingbatsFontName());
	}

	if(isModal())
	{
//
// Fill out output vector with XML_Char * strings to be accessed via the calling
// function.
//
		if(m_OutProps.getItemCount() > 0)
			m_OutProps.clear();
		sprintf(szStart,"%d",m_iStartValue);
		m_OutProps.addItem((void *) "start-value"); 
		m_Output[0] = (XML_Char *) szStart;
		m_OutProps.addItem((void *) m_Output[0].c_str());
		m_OutProps.addItem((void *) "list-style"); 
		m_Output[1] = getBlock()->getListStyleString(m_newListType);
		m_OutProps.addItem((void *) m_Output[1].c_str());
		m_OutProps.addItem((void *) "list-delim");
		m_OutProps.addItem((void *)  m_pszDelim);
		m_OutProps.addItem((void *) "list-decimal");
		m_OutProps.addItem((void *) m_pszDecimal);
		m_OutProps.addItem((void *) "field-font");
		m_OutProps.addItem((void *) m_pszFont);
		m_OutProps.addItem((void *) "margin-left");
		m_Output[2] =	UT_convertInchesToDimensionString(DIM_IN, m_fAlign, 0);
		m_OutProps.addItem((void *) m_Output[2].c_str());

		m_OutProps.addItem((void *) "text-indent");
		m_Output[3] = UT_convertInchesToDimensionString(DIM_IN, m_fIndent, 0);
		m_OutProps.addItem((void *) m_Output[3].c_str());
		return;
	}
	if(m_bApplyToCurrent == true && m_isListAtPoint == true &&  m_newListType != NOT_A_LIST)
	{
		getView()->changeListStyle(getAutoNum(),m_newListType,m_iStartValue,(XML_Char *) m_pszDelim,(XML_Char *) m_pszDecimal, m_pszFont,m_fAlign,m_fIndent);
		if(getAutoNum() != NULL)
		{
			getAutoNum()->update(0);
			// getBlock()->listUpdate();
		}
		clearDirty();
		return;
	}
	else if ( m_isListAtPoint == true &&  m_newListType == NOT_A_LIST)
	{
		if(getBlock()->isListItem() == true)
		{
			getBlock()->StopList();
		}
		clearDirty();
		return;
	}
	if(m_bStartNewList == true)
	{ 
		if(m_isListAtPoint == true && m_newListType == NOT_A_LIST)
		{
			if(getBlock()->isListItem() == true)
			{
				getBlock()->StopList();
			}
			clearDirty();
			return;
		}
		else if ( m_isListAtPoint != true && m_newListType != NOT_A_LIST )
		{
		        getBlock()->getDocument()->disableListUpdates();
			getBlock()->StartList(m_newListType,m_iStartValue,m_pszDelim,m_pszDecimal,m_pszFont,m_fAlign,m_fIndent, 0,1); 
			getBlock()->getDocument()->enableListUpdates();
			getBlock()->getDocument()->updateDirtyLists();
			clearDirty();
			return;
		}
		else if( m_newListType != NOT_A_LIST )
		{
			UT_uint32 curlevel = getBlock()->getLevel();
			UT_uint32 currID = getBlock()->getAutoNum()->getID();
			curlevel++;
		        getBlock()->getDocument()->disableListUpdates();
			getBlock()->StartList(m_newListType,m_iStartValue,m_pszDelim,m_pszDecimal,m_pszFont,m_fAlign,m_fIndent, currID,curlevel);
			getBlock()->getDocument()->enableListUpdates();
			getBlock()->getDocument()->updateDirtyLists();
			clearDirty();
			return;
		}
	}
	if(m_bStartSubList == true &&  m_isListAtPoint != true )
	{ 
		fl_BlockLayout * rBlock = getBlock()->getPreviousListOfSameMargin();
		if(rBlock == NULL)
		{
			clearDirty();
			return;
		}
		getBlock()->resumeList(rBlock);
		getBlock()->getDocument()->enableListUpdates();
		getBlock()->getDocument()->updateDirtyLists();
		clearDirty();
		return;
	}
	clearDirty();
}


void  AP_Dialog_Lists::fillUncustomizedValues(void)
{
  //
  // This function loads the standard values into Delim, decimal, format
  // m_fAlign, m_iLevel and m_iStarValue based on m_newListType
  //
  // m_fAlign and m_fIndent should be in inches
  //
	if(m_newListType == NOT_A_LIST)
	{
		UT_XML_strncpy( (XML_Char *) m_pszDelim, 80, (const XML_Char *) "%L");
		m_fAlign = 0.0;
		m_fIndent = 0.0;
		m_iLevel = 0;
		UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *) "NULL");
		UT_XML_strncpy( (XML_Char *) m_pszDecimal, 80, (const XML_Char *) ".");
		m_iStartValue = 1;
	}	       

	if(m_iLevel <= 0)
	{
		m_iLevel = 1;
	}
	
	UT_XML_strncpy( (XML_Char *) m_pszDelim, 80, (const XML_Char *) "%L");
	m_fAlign =  (float)(LIST_DEFAULT_INDENT * m_iLevel);
	m_fIndent = (float)-LIST_DEFAULT_INDENT_LABEL;

	if( m_newListType == NUMBERED_LIST)
	{   
		UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *) "NULL");
		UT_XML_strncpy( (XML_Char *) m_pszDecimal, 80, (const XML_Char *) ".");
		m_iStartValue = 1;
		UT_XML_strncpy( (XML_Char *) m_pszDelim, 80, (const XML_Char *) "%L.");
	}
	else if( m_newListType == LOWERCASE_LIST)
	{   
		UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *) "NULL");
		UT_XML_strncpy( (XML_Char *) m_pszDecimal, 80, (const XML_Char *) ".");
		m_iStartValue = 1;
		UT_XML_strncpy( (XML_Char *) m_pszDelim, 80, (const XML_Char *) "%L)");
	}
	else if( m_newListType == UPPERCASE_LIST)
	{   
		UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *) "NULL");
		UT_XML_strncpy( (XML_Char *) m_pszDecimal, 80, (const XML_Char *) ".");
		m_iStartValue = 1;
		UT_XML_strncpy( (XML_Char *) m_pszDelim, 80, (const XML_Char *) "%L)");
	}
	else if( m_newListType < BULLETED_LIST)
	{   
		UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *) "NULL");
		UT_XML_strncpy( (XML_Char *) m_pszDecimal, 80, (const XML_Char *) ".");
		m_iStartValue = 1;
		UT_XML_strncpy( (XML_Char *) m_pszDelim, 80, (const XML_Char *) "%L");
	}
	else
	{
		UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *) "NULL");
		UT_XML_strncpy( (XML_Char *) m_pszDecimal, 80, (const XML_Char *) ".");
		m_iStartValue = 0;
	}	       
	if(m_newListType == BULLETED_LIST || m_newListType == IMPLIES_LIST)
	{
		UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *) "Symbol");
	}
	else if (m_newListType == NOT_A_LIST)
	{
		UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *) "NULL");
	}
	else if(m_newListType > DASHED_LIST)
	{
		UT_XML_strncpy( (XML_Char *) m_pszFont, 80, _getDingbatsFontName());
	}
}

void  AP_Dialog_Lists::fillFakeLabels(void)
{

	if(m_bisCustomized == false && !isModal())
	{
		m_iLevel = getBlock()->getLevel();
		if(m_iLevel == 0 || m_bStartSubList == true)
		{
			m_iLevel++;
		}
		PopulateDialogData();
		if(m_bguiChanged == false)
			m_newListType = m_iListType;
		m_bguiChanged = false;
	}
	if(m_newListType == BULLETED_LIST || m_newListType == IMPLIES_LIST)
	{
		UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *) "Symbol");
		UT_XML_strncpy( (XML_Char *) m_pszDelim, 80, (const XML_Char *) "%L");
	}
	else if (m_newListType == NOT_A_LIST)
	{
		UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *) "NULL");
		UT_XML_strncpy( (XML_Char *) m_pszDelim, 80, (const XML_Char *) "%L");
	}
	else if(m_newListType > DASHED_LIST)
	{
		UT_XML_strncpy( (XML_Char *) m_pszFont, 80, _getDingbatsFontName());
		UT_XML_strncpy( (XML_Char *) m_pszDelim, 80, (const XML_Char *) "%L");
	}
	m_pFakeAuto->setListType(m_newListType);
	m_pFakeAuto->setDelim(m_pszDelim);
	m_pFakeAuto->setDecimal(m_pszDecimal);
	m_pFakeAuto->setStartValue(m_iStartValue);
	m_pListsPreview->setData(m_pszFont,m_fAlign,m_fIndent);
}

void  AP_Dialog_Lists::generateFakeLabels(void)
{
  //
  // This routine generates it's own AutoNum's and Layout pointers 
  // for use in the preview
  //
	UT_uint32 i;
	//
	// Start by generating 4 fake (PL_StruxDocHandle and fl_Layout pointers
	//
	// Jeeze gotta generate a fake void * pointer!! Try this hack.
	//
	XAP_App * fakeApp = getApp();
	for(i=0; i<4; i++)
	{
		DELETEP(m_pFakeLayout[i]);
		m_pFakeSdh[i] = (PL_StruxDocHandle) fakeApp++;
		m_pFakeLayout[i] = new fl_Layout((PTStruxType) 0 , (PL_StruxDocHandle) m_pFakeSdh[i] ); 
	}
	//
	// Now generate the AutoNum
	//
	DELETEP(m_pFakeAuto);
	m_pFakeDoc = new PD_Document(fakeApp);
	m_pFakeAuto = new fl_AutoNum(m_iID, 0, m_newListType, m_newStartValue, m_pszDelim, m_pszDecimal, (PD_Document *) m_pFakeDoc);
	m_pFakeAuto->insertFirstItem(m_pFakeSdh[0], NULL,1);
	m_pFakeLayout[0]->setAutoNum(m_pFakeAuto);
	for(i=1; i<4; i++)
	{
		m_pFakeAuto->insertItem(m_pFakeSdh[i],m_pFakeSdh[i-1]);
		m_pFakeLayout[i]->setAutoNum(m_pFakeAuto);
	}
}


UT_UCSChar * AP_Dialog_Lists::getListLabel(UT_sint32 itemNo)
{
	UT_ASSERT(itemNo < 4);
	static UT_UCSChar lab[80];
	const UT_UCSChar * tmp  = m_pFakeAuto->getLabel(m_pFakeSdh[itemNo]);
	if(tmp == NULL)
	{
		return NULL;
	}
	UT_sint32 cnt = UT_MIN(UT_UCS_strlen(tmp),80);
	UT_sint32 i;
	for(i =0; i<= cnt; i++)
		lab[i] = *tmp++;
	return lab;
}

void AP_Dialog_Lists::fillDialogFromVector( UT_Vector * vp)
{
	UT_sint32 i;
	if(vp->getItemCount() > 0)
	{
		i = findVecItem(vp,"start-value");
		if(i >= 0)
		{
			m_iStartValue = atoi( (const XML_Char *) vp->getNthItem(i+1));
		}
		else
		{
			m_iStartValue = 1;
		}

		i = findVecItem(vp,"margin-left");
		if(i>=0)
		{
			m_fAlign = (float)atof((const XML_Char *) vp->getNthItem(i+1));
		}
		else
		{
			m_fAlign = (float)LIST_DEFAULT_INDENT;
		}

		i = findVecItem(vp,"text-indent");
		if(i >= 0)
		{
			m_fIndent = (float)atof((const XML_Char *) vp->getNthItem(i+1));
		}
		else
		{
			m_fIndent = (float)-LIST_DEFAULT_INDENT_LABEL;
		}

		i = findVecItem(vp,"list-delim");
		if( i>= 0)
		{
			UT_XML_strncpy( (XML_Char *) m_pszDelim, 80, (const XML_Char *) vp->getNthItem(i+1));
		}
		else
		{
			UT_XML_strncpy( (XML_Char *) m_pszDelim, 80, (const XML_Char *) "%L");
		}
		i = findVecItem(vp,"list-decimal");
		if( i>= 0)
		{
			UT_XML_strncpy( (XML_Char *) m_pszDecimal, 80, (const XML_Char *) vp->getNthItem(i+1));
		}
		else
		{
			UT_XML_strncpy( (XML_Char *) m_pszDecimal, 80, (const XML_Char *) ".");
		}

		i = findVecItem(vp,"field-font");
		if( i>= 0)
		{
			UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *) vp->getNthItem(i+1));
		}
		else
		{
			UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *) "NULL");
		}
		i = findVecItem(vp,"list-style");
		if( i>= 0)
		{
			m_iListType = getBlock()->getListTypeFromStyle( (const XML_Char *) vp->getNthItem(i+1));
			m_newListType = m_iListType;
		}
		else
		{
			m_iListType = NOT_A_LIST;
			m_newListType = m_iListType;
		}
	}
}

void AP_Dialog_Lists::fillDialogFromBlock(void)
{
	UT_Vector va,vp;

	if (getBlock()->getPreviousList() != NULL)
	{
		m_previousListExistsAtPoint = true;
	}
	else
	{
		m_previousListExistsAtPoint = false;
	}
	getBlock()->getListAttributesVector( &va);
	getBlock()->getListPropertyVector( &vp);
	//
	// First do properties.
	//
	UT_sint32 i;
	if(vp.getItemCount() > 0)
	{
		i = findVecItem(&vp,"start-value");
		if(i >= 0)
		{
			m_iStartValue = atoi( (const XML_Char *) vp.getNthItem(i+1));
		}
		else
		{
			m_iStartValue = 1;
		}

		i = findVecItem(&vp,"margin-left");
		if(i>=0)
		{
			m_fAlign = (float)atof((const XML_Char *) vp.getNthItem(i+1));
		}
		else
		{
			m_fAlign = (float)LIST_DEFAULT_INDENT;
		}

		i = findVecItem(&vp,"text-indent");
		if(i >= 0)
		{
			m_fIndent = (float)atof((const XML_Char *) vp.getNthItem(i+1));
		}
		else
		{
			m_fIndent = (float)-LIST_DEFAULT_INDENT_LABEL;
		}

		i = findVecItem(&vp,"list-delim");
		if(i >=0 )
		{
			UT_XML_strncpy( (XML_Char *) m_pszDelim, 80, (const XML_Char *) vp.getNthItem(i+1));
		}
		else
		{
			UT_XML_strncpy( (XML_Char *) m_pszDelim, 80, (const XML_Char *) "%L");
		}

		i = findVecItem(&vp,"list-decimal");
		if( i>= 0)
		{
			UT_XML_strncpy( (XML_Char *) m_pszDecimal, 80, (const XML_Char *) vp.getNthItem(i+1));
		}
		else
		{
			UT_XML_strncpy( (XML_Char *) m_pszDecimal, 80, (const XML_Char *) ".");
		}

		i = findVecItem(&vp,"field-font");
		if( i>= 0)
		{
			UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *) vp.getNthItem(i+1));
		}
		else
		{
			UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *) "NULL");
		}
		i = findVecItem(&vp,"list-style");
		if( i>= 0)
		{
			m_iListType = getBlock()->getListTypeFromStyle( (const XML_Char *) vp.getNthItem(i+1));
		}
		else
		{
			m_iListType = NUMBERED_LIST;
		}
	}
	//
	// Now Do the Attributes first
	//
	if(va.getItemCount()>0)
	{	
		i = findVecItem(&va,"style");
		if( i>= 0)
		{
			m_iListType = getBlock()->getListTypeFromStyle( (const XML_Char *) va.getNthItem(i+1));
		}
		else
		{
			m_iListType = NUMBERED_LIST;
		}

		i = findVecItem(&va,"level");
		if( i>= 0)
		{
			m_iLevel = atoi((const XML_Char *) va.getNthItem(i+1));
		}
		else
		{
			m_iLevel = 1;
		}
	}
	if(getAutoNum() != NULL)
	{
		m_iID = getAutoNum()->getID();
		m_iListType = getAutoNum()->getType();
		UT_XML_strncpy( (XML_Char *) m_pszDecimal, 80, (const XML_Char *) getAutoNum()->getDecimal());
		
	}
	else
	{	       
		m_iID = 0;
		m_iListType = NOT_A_LIST;
	}
}

void AP_Dialog_Lists::PopulateDialogData(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	m_isListAtPoint = getBlock()->isListItem();
	if(m_isListAtPoint == true)
	{
		fillDialogFromBlock();
	}
	else
	{
		//	m_newListType = NOT_A_LIST;
		fillUncustomizedValues();
	}
	if(m_isListAtPoint == true)
	{
		const UT_UCSChar * tmp1 =  getBlock()->getListLabel();
		if(tmp1 != NULL)
		{
			UT_sint32 cnt = UT_MIN(UT_UCS_strlen(tmp1),80);
			UT_sint32 i;
			for(i =0; i<=cnt; i++)
				m_curListLabel[i] = *tmp1++;
		}
		m_curListLevel = getBlock()->getLevel();
		m_curStartValue = getAutoNum()->getStartValue32();
		m_iStartValue = getAutoNum()->getStartValue32();
		m_iListType = getAutoNum()->getType();
		//	m_newListType = getAutoNum()->getType();
		if(m_iListType == NUMBERED_LIST)
		{
			strcpy(m_curListType,pSS->getValue(AP_STRING_ID_DLG_Lists_Numbered_List));
		}
		else if(m_iListType == LOWERCASE_LIST)
		{
			strcpy(m_curListType,pSS->getValue(AP_STRING_ID_DLG_Lists_Lower_Case_List));
		}
		else if(m_iListType == UPPERCASE_LIST)
		{
			strcpy(m_curListType,pSS->getValue(AP_STRING_ID_DLG_Lists_Upper_Case_List));
		}
		else if(m_iListType == UPPERROMAN_LIST)
		{
			strcpy(m_curListType,pSS->getValue(AP_STRING_ID_DLG_Lists_Upper_Roman_List));
		}
		else if(m_iListType == LOWERROMAN_LIST)
		{
			strcpy(m_curListType,pSS->getValue(AP_STRING_ID_DLG_Lists_Lower_Roman_List));
		}
		else if(m_iListType == BULLETED_LIST)
		{
			strcpy(m_curListType,pSS->getValue(AP_STRING_ID_DLG_Lists_Bullet_List));
		}
		else if(m_iListType == DASHED_LIST)
		{
			strcpy(m_curListType,pSS->getValue(AP_STRING_ID_DLG_Lists_Dashed_List));
		}
		else
		{
			strcpy(m_curListType,pSS->getValue(AP_STRING_ID_DLG_Lists_Numbered_List ));
		}
	}
	else
	{
		m_iListType = NOT_A_LIST;
		m_curStartValue = 1;
	}
}

UT_uint32 AP_Dialog_Lists::getID(void)
{
       if(getBlock()->isListItem() == false)
       {
	       return 0;
       }
       else
       {
	       return getAutoNum()->getID();
       }
}

/*!
 * This method returns the index to the value corresponding to the 
 * key in this props vector
 */
UT_sint32  AP_Dialog_Lists::findVecItem(UT_Vector * v, char * key)
{
	UT_sint32 i = v->getItemCount();
	if(i < 0) 
		return i;
	UT_sint32 j;
	const char * pszV = NULL;
	for(j= 0; j<i ;j=j+2)
	{
		pszV = (char *) v->getNthItem(j);
		if( (pszV != NULL) && (strcmp( pszV,key) == 0))
			break;
	}
	if( j < i && pszV)
		return j;
	else
		return -1;
}

bool AP_Dialog_Lists::isLastOnLevel(void)
{
	return getAutoNum()->isLastOnLevel(getBlock()->getStruxDocHandle());
}



// --------------------------- Generic Functions -----------------------------


void AP_Dialog_Lists::ConstructWindowName(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	XML_Char * tmp = NULL;
	UT_uint32 title_width = 33;
	UT_XML_cloneNoAmpersands(tmp, pSS->getValue(AP_STRING_ID_DLG_Lists_Title));
	BuildWindowName((char *) m_WindowName,(char*)tmp,title_width);
	FREEP(tmp);
}

void AP_Dialog_Lists::setActiveFrame(XAP_Frame *pFrame)
{
	setView(getView());
	notifyActiveFrame(getActiveFrame());
}

bool AP_Dialog_Lists::setView(FV_View * view)
{
	m_pView = (FV_View *) getActiveFrame()->getCurrentView();
	return true;
}

FV_View * AP_Dialog_Lists::getView(void)
{
	XAP_Frame * pFrame = getActiveFrame();
	return (FV_View *) pFrame->getCurrentView();
}

AV_View * AP_Dialog_Lists::getAvView(void)
{
	XAP_Frame * pFrame = getActiveFrame();
	return pFrame->getCurrentView();
}

const XML_Char* AP_Dialog_Lists::_getDingbatsFontName() const
{
	return "Dingbats";
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

AP_Lists_preview::AP_Lists_preview(GR_Graphics * gc, AP_Dialog_Lists * pLists)
:	XAP_Preview(gc),
	m_pLists(pLists),
	m_fAlign(0.0f),
	m_fIndent(0.0f),
	m_iLine_height(0),
	m_bFirst(true)
{
	m_pszFont[0] = '\0';
	m_iLine_pos[0] = 0;
}

AP_Lists_preview::~AP_Lists_preview()
{
}

AP_Dialog_Lists * AP_Lists_preview::getLists(void)
{
	return m_pLists;
}

void AP_Lists_preview::setData(XML_Char * pszFont,float fAlign,float fIndent)
{
	UT_XML_strncpy(m_pszFont, 80, (const XML_Char *) pszFont);
	m_fAlign = fAlign;
	m_fIndent = fIndent;
}


void AP_Lists_preview::draw(void)
{
	UT_RGBColor clrGrey = UT_RGBColor(128,128,128);
	UT_RGBColor clrBlack = UT_RGBColor(0,0,0);
	UT_sint32 iWidth = getWindowWidth();
	UT_sint32 iHeight = getWindowHeight();
	UT_UCSChar ucs_label[50];
	//
	// we draw at 16 points in this preview
	//
	GR_Font * font;
	if(strcmp(m_pszFont,"NULL")== 0)
	{
		font = m_gc->findFont("Times New Roman", "normal", "", "normal", "", "16pt");
	}
	else
	{
		font = m_gc->findFont((char *)m_pszFont, "normal", "", "normal", "", "16pt");
	}

	m_gc->setFont(font);
	UT_sint32 iDescent = m_gc->getFontDescent();
	UT_sint32 iAscent = m_gc->getFontAscent();
	UT_sint32 iFont = iDescent + iAscent;
	m_iLine_height = iFont;
	//
	// clear our screen
	//
	if(m_bFirst == true)
	{
	         m_gc->clearArea(0, 0, iWidth, iHeight);
	}
	m_gc->setColor(clrBlack);
	UT_sint32 yoff = 5 ;
	UT_sint32 xoff = 5 ;
	UT_sint32 i,ii,yloc,awidth,aheight,maxw;
	UT_sint32 twidth =0;
	UT_sint32 j,xy;
	float z,fwidth;
	// todo 6.5 should be the page width in inches
	float pagew = 2.0;
	aheight = 16;
	fwidth = static_cast<float>(iWidth);
	
	z = (float)((fwidth - 2.0*static_cast<float>(xoff)) /pagew);
        UT_sint32 indent = static_cast<UT_sint32>( z*(m_fAlign+m_fIndent));

	if(indent < 0)
		indent = 0;
	maxw = 0;
	for(i=0; i<4; i++)
	{
		UT_UCSChar * lv = getLists()->getListLabel(i);
		UT_sint32 len =0;

		if(lv != NULL) 
		{
			//
			// This code is here because UT_UCS_copy_char is broken
			//
			len = UT_MIN(UT_UCS_strlen(lv),51);
			for(j=0; j<=len;j++)
			{
				ucs_label[j] = *lv++;
			}
			ucs_label[len] = NULL;
			len = UT_UCS_strlen(ucs_label);
			yloc = yoff + iAscent + (iHeight - 2*yoff -iFont)*i/4;
			//    m_gc->drawChars(ucs_label,0,len,xoff+indent,yloc);
			twidth = m_gc->measureString(ucs_label,0,len,NULL);
			if(twidth > maxw)
				maxw = twidth;
		}
	}
	//
	// Work out where to put grey areas to represent text
	//
	UT_sint32 xx,yy;
	if(maxw > 0) 
		maxw++;
	
        // UT_sint32 vspace = (iHeight - 2*yoff -iFont)*i/16;
	z = (float)((fwidth - 2.0*static_cast<float>(xoff)) /(float)pagew);
	UT_sint32 ialign = static_cast<UT_sint32>( z*m_fAlign);
	xx = xoff + ialign;
	xy = xoff + ialign;
	if(xx < (xoff + maxw + indent))
		xy = xoff + maxw + indent + 1;
	ii = 0;

	for(i=0; i<4; i++)
	{
		yloc = yoff + iAscent + (iHeight - 2*yoff -iFont)*i/4;
		for(j=0; j< 2; j++)
		{
			yy = yloc + 5 + j*21;
			m_iLine_pos[ii++] = yy;
		}
	}
	//
	// Now finally draw the preview
	//
	for(i=0; i<8; i++)
	{
		//
		// First clear the line
		//
		m_gc->clearArea(0, m_iLine_pos[i], iWidth, iHeight);
		if((i & 1) == 0)
		{
			//
			// Draw the text
			//
			UT_UCSChar * lv = getLists()->getListLabel(i/2);
			UT_sint32 len =0;
			
			if(lv != NULL) 
			{
				len = UT_MIN(UT_UCS_strlen(lv),51);
				for(j=0; j<=len;j++)
				{
					ucs_label[j] = *lv++;
				}
				ucs_label[len] = NULL;
				len = UT_UCS_strlen(ucs_label);
				yloc = yoff + iAscent + (iHeight - 2*yoff -iFont)*i/8;
				m_gc->drawChars(ucs_label,0,len,xoff+indent,yloc);
				yy = m_iLine_pos[i];
				awidth = iWidth - 2*xoff - xy;
				m_gc->fillRect(clrGrey,xy,yy,awidth,aheight);
			}
			else
			{
				yy = m_iLine_pos[i];
				awidth = iWidth - 2*xoff - xy;
				m_gc->fillRect(clrGrey,xy,yy,awidth,aheight);
			}
		}
		else
		{
			yy = m_iLine_pos[i];
			awidth = iWidth - 2*xoff - xx;
			m_gc->fillRect(clrGrey,xx,yy,awidth,aheight);
		}
	}
}

