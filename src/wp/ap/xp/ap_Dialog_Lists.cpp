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

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"

#include "fl_DocLayout.h"
#include "fv_View.h"

#include "fl_DocLayout.h"
#include "fl_BlockLayout.h"
#include "ap_Preview_Paragraph.h"


AP_Dialog_Lists::AP_Dialog_Lists(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_Modeless(pDlgFactory, id)
{
  	m_pView = NULL;
	m_answer = a_CLOSE;
	m_bStartList = UT_FALSE;
	m_bStopList = UT_FALSE;
	m_bChangeStartValue = UT_FALSE;

	m_paragraphPreview = NULL;
}

AP_Dialog_Lists::~AP_Dialog_Lists(void)
{
	DELETEP(m_paragraphPreview);
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

void AP_Dialog_Lists::_createPreviewFromGC(GR_Graphics * gc,
										   UT_uint32 width,
										   UT_uint32 height)
{
	UT_ASSERT(gc);

	// free any attached preview
	DELETEP(m_paragraphPreview);

	// platform's runModal should have set this
	UT_ASSERT(getActiveFrame());

	AV_View * baseview = getActiveFrame()->getCurrentView();
	UT_ASSERT(baseview);

	FV_View * view = static_cast<FV_View *> (baseview);

	FL_DocLayout * dl = view->getLayout();
	UT_ASSERT(dl);

	fl_BlockLayout * bl = dl->findBlockAtPosition((PT_DocPosition) view->getPoint());
	UT_ASSERT(bl);

	UT_GrowBuf gb;
	UT_Bool hadMem = bl->getBlockBuf(&gb);

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

	// TODO : any setup of the GC for drawing
}

void AP_Dialog_Lists::event_PreviewAreaExposed(void)
{
	if (m_paragraphPreview) {
		m_paragraphPreview->draw();
	}
	else {
		UT_ASSERT(0);
	}
}

void  AP_Dialog_Lists::StartList(void)
{
       getBlock()->listUpdate();
       getView()->cmdStartList(getBlock()->getListStyleString(m_iListType));
}


void  AP_Dialog_Lists::StopList(void)
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

void  AP_Dialog_Lists::Apply(void)
{
       XML_Char* style;
       if(m_bChangeStartValue == UT_TRUE)
       {
              getAutoNum()->setAsciiOffset(0);
	      style = getBlock()->getListStyleString(m_iListType);
	      getView()->changeListStyle(getAutoNum(),style);
       	      getAutoNum()->setStartValue(m_curStartValue);
	      getAutoNum()->setFormat((XML_Char*)m_newListType);
       	      getAutoNum()->update(0);
	      getBlock()->listUpdate();
	      return;
       }
       if(m_bStopList == UT_TRUE)
       { 
	      StopList();
	      return;
       }
       if(m_bStartList == UT_TRUE)
       { 
	      StartList(); //Start with a numbered list then change the format
	                   //and starting value
	      switch(m_iListType)
	      {
	      case NUMBERED_LIST:  // Numbered List
	      
		       getAutoNum()->setStartValue(m_newStartValue);
     		       getAutoNum()->setFormat((XML_Char*)m_newListType);
		       break;
	      case LOWERCASE_LIST:  // Lowe Case List
		       getAutoNum()->setStartValue(m_newStartValue); 
     		       getAutoNum()->setFormat((XML_Char*)m_newListType);
		       break;
	      case UPPERCASE_LIST:  // Upper Case List
		       getAutoNum()->setStartValue(m_newStartValue); 
     		       getAutoNum()->setFormat((XML_Char*)m_newListType);
		       break;
	      case BULLETED_LIST:  // Bulleted List
		       getAutoNum()->setFormat((XML_Char*)m_newListType);
		       break;
	      default:
		       UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	      }
	      getBlock()->listUpdate();
	      return;
       }
       if(m_bresumeList == UT_TRUE)
       {
	      fl_BlockLayout * rBlock = getBlock()->getPreviousList();
	      if(rBlock == NULL)
		       return;
	      getBlock()->resumeList(rBlock);
	      return;
       }
}


void  AP_Dialog_Lists::PopulateDialogData(void)
{
       const XAP_StringSet * pSS = m_pApp->getStringSet();
       if (getBlock()->getPreviousList() != NULL)
       {
	      m_previousListExistsAtPoint = UT_TRUE;
       }
       else
       {
	      m_previousListExistsAtPoint = UT_FALSE;
       }
       m_isListAtPoint = getBlock()->isListItem();
       if(m_isListAtPoint == UT_TRUE)
       {
	      const char * tmp1 =  (char *) getBlock()->getListLabel();
	      strcpy( m_curListLabel, tmp1);
              m_curListLevel = getBlock()->getLevel();
              m_curStartValue = getAutoNum()->getStartValue32();
	      char * tmp2 = getAutoNum()->getType();
	      m_iListType = getBlock()->decodeListType(tmp2);
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
	      else if(m_iListType == BULLETED_LIST)
	      {
		      strcpy(m_curListType,pSS->getValue(AP_STRING_ID_DLG_Lists_Bullet_List));
	      }
	      else
	      {
		      strcpy(m_curListType,tmp2);
	      }
       }
       else
       {
              m_iListType = NUMBERED_LIST;
	      m_curStartValue = 1;
       }

}

UT_Bool  AP_Dialog_Lists::isLastOnLevel(void)
{
       return getAutoNum()->isLastOnLevel(getBlock());
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

void  AP_Dialog_Lists::setActiveFrame(XAP_Frame *pFrame)
{
        setView(getView());
	notifyActiveFrame(getActiveFrame());
}

UT_Bool AP_Dialog_Lists::setView(FV_View * view)
{
        m_pView =  (FV_View *) getActiveFrame()->getCurrentView();
	return UT_TRUE;
}

FV_View * AP_Dialog_Lists::getView(void)
{
        XAP_Frame * pFrame =  getActiveFrame();
        return  (FV_View *) pFrame->getCurrentView();
}







