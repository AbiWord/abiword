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
	m_iLocalTick = 0;
	m_paragraphPreview = NULL;
	m_pListsPreview = NULL;
        for(UT_uint32 i=0; i<4; i++)
        {
	      m_pFakeLayout[i] = NULL;
	}
        m_pFakeAuto = NULL;
}

AP_Dialog_Lists::~AP_Dialog_Lists(void)
{
  //	DELETEP(m_paragraphPreview);
 	DELETEP(m_pListsPreview);
        for(UT_uint32 i=0; i<4; i++)
        {
	      DELETEP(m_pFakeLayout[i]);
	}
        DELETEP(m_pFakeAuto);
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

	/* This is Thomas Code. I'll do a fake preview instead
// Martin
 // free any attached preview

	DELETEP(m_paragraphPreview);

	UT_GrowBuf gb;
	UT_Bool hadMem = getBlock()->getBlockBuf(&gb);

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
	*/

	// TODO : any setup of the GC for drawing
}

void AP_Dialog_Lists::event_PreviewAreaExposed(void)
{

  /* Thomas's code
	if (m_paragraphPreview) {
		m_paragraphPreview->draw();
	}
	else {
		UT_ASSERT(0);
	}
  */
  // Martin's code

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

UT_uint32  AP_Dialog_Lists::getTick(void)
{
       return m_iLocalTick;
}

void  AP_Dialog_Lists::setTick(UT_uint32 iTick)
{
       m_iLocalTick = iTick;
}

void  AP_Dialog_Lists::Apply(void)
{
       if(m_newListType == BULLETED_LIST || m_newListType == IMPLIES_LIST)
       {
               UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *) "Symbol");
       }
       else if(m_newListType > DASHED_LIST)
       {
               UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *) "Dingbats");
       }

       if(m_bisCustomized == UT_FALSE)
       {
	      m_iLevel = getBlock()->getLevel();
	      if(m_iLevel == 0 || m_bStartSubList == UT_TRUE)
	      {
		      m_iLevel++;
	      }
	      fillUncustomizedValues();
       }
       if(m_bApplyToCurrent == UT_TRUE && m_isListAtPoint == UT_TRUE)
       {
	      getView()->changeListStyle(getAutoNum(),m_newListType,m_iStartValue,(XML_Char *) m_pszDelim,(XML_Char *) m_pszDecimal, m_pszFont,m_fAlign,m_fIndent);
	      if(getAutoNum() != NULL)
	      {
		      getAutoNum()->update(0);
	              getBlock()->listUpdate();
	      }
	      return;
       }
       if(m_bStartNewList == UT_TRUE)
       { 
	      if(m_isListAtPoint == UT_TRUE || m_newListType == NOT_A_LIST)
	      {
		      getBlock()->StopList();
	              getBlock()->listUpdate();
		      return;
	      }
	      else if (m_bisCustomized == UT_TRUE)
	      {
	              getBlock()->StartList(m_newListType,m_iStartValue,m_pszDelim,m_pszDecimal,m_pszFont,m_fAlign,m_fIndent, 0); 
	              getBlock()->listUpdate();
	              return;
	      }
	      else if (m_bisCustomized == UT_FALSE)
	      {
	      	      getBlock()->StartList(getBlock()->getListStyleString(m_newListType));
		      getBlock()->listUpdate();
	      }
       }
       if(m_bStartSubList == UT_TRUE && m_newListType != NOT_A_LIST )
       { 
	      if(m_isListAtPoint == UT_TRUE)
	      {
		//      getBlock()->StopList();
		      UT_uint32 curlevel = getBlock()->getLevel();
		      UT_uint32 currID = getBlock()->getAutoNum()->getID();
		      curlevel++;
		      if (m_bisCustomized == UT_TRUE)
		      	getBlock()->StartList(m_newListType,m_iStartValue,m_pszDelim,m_pszDecimal,m_pszFont,m_fAlign,m_fIndent, currID); 
		      else if (m_bisCustomized == UT_FALSE)
		      	getBlock()->StartList(getBlock()->getListStyleString(m_newListType));
		      getBlock()->listUpdate();
		      return;
	      }
	      else
	      {
	              fl_BlockLayout * rBlock = getBlock()->getPreviousList();
		      if(rBlock == NULL)
			      return;
		      getBlock()->resumeList(rBlock);
		      return;
	      }

       }
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
       m_fAlign = 0.25 *(float) m_iLevel;
       m_fIndent = -0.25;
       if( m_newListType < BULLETED_LIST)
       {   
               UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *) "NULL");
               UT_XML_strncpy( (XML_Char *) m_pszDecimal, 80, (const XML_Char *) ".");
	       m_iStartValue = 1;
       }
       else
       {
               UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *) "NULL");
               UT_XML_strncpy( (XML_Char *) m_pszDecimal, 80, (const XML_Char *) "NULL");
	       m_iStartValue = 0;
       }	       
       if(m_newListType == BULLETED_LIST || m_newListType == IMPLIES_LIST)
       {
               UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *) "Symbol");
       }
       else if(m_newListType > DASHED_LIST)
       {
               UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *) "Dingbats");
       }
}

void  AP_Dialog_Lists::fillFakeLabels(void)
{
  UT_DEBUGMSG(("SEVIOR: fillFakeLabels m_newListType = %d \n",m_newListType));

       if(m_bisCustomized == UT_FALSE)
       {
	      m_iLevel = getBlock()->getLevel();
	      if(m_iLevel == 0 || m_bStartSubList == UT_TRUE)
	      {
		      m_iLevel++;
	      }
	      PopulateDialogData();
       }
       if(m_newListType == BULLETED_LIST || m_newListType == IMPLIES_LIST)
       {
               UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *) "Symbol");
               UT_XML_strncpy( (XML_Char *) m_pszDelim, 80, (const XML_Char *) "%L");
       }
       else if(m_newListType > DASHED_LIST)
       {
               UT_XML_strncpy( (XML_Char *) m_pszFont, 80, (const XML_Char *) "Dingbats");
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
       // Start by generating 4 fake fl_Layout pointers
       //
       for(i=0; i<4; i++)
       {
	      DELETEP(m_pFakeLayout[i]);
	      m_pFakeLayout[i] = new fl_Layout((PTStruxType) 0 , (PL_StruxDocHandle) NULL ); 
       }
       //
       // Now generate the AutoNum
       //
       DELETEP(m_pFakeAuto);
       m_pFakeAuto = new fl_AutoNum(m_iID, 0, m_newListType, m_newStartValue, m_pszDelim);
       m_pFakeAuto->insertFirstItem(m_pFakeLayout[0], NULL);
       m_pFakeLayout[0]->setAutoNum(m_pFakeAuto);
       for(i=1; i<4; i++)
       {
	      m_pFakeAuto->insertItem(m_pFakeLayout[i],m_pFakeLayout[i-1]);
	      m_pFakeLayout[i]->setAutoNum(m_pFakeAuto);
       }
}

XML_Char * AP_Dialog_Lists::getListLabel(UT_sint32 itemNo)
{
       UT_ASSERT(itemNo < 4);
       XML_Char lab[80];
       const XML_Char * tmp;
       tmp = m_pFakeAuto->getLabel(m_pFakeLayout[itemNo]);
       if(tmp == NULL)
       {
	      return NULL;
       }
       UT_XML_strncpy(lab, 80, tmp);    
       return (XML_Char *) lab;
}

void  AP_Dialog_Lists::fillDialogFromBlock(void)
{
       UT_Vector va,vp;

       if (getBlock()->getPreviousList() != NULL)
       {
	      m_previousListExistsAtPoint = UT_TRUE;
       }
       else
       {
	      m_previousListExistsAtPoint = UT_FALSE;
       }
       getBlock()->getListAttributesVector( &va);
       getBlock()->getListPropertyVector( &vp);
       //
       // do properties first
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
	                 m_fAlign = atof((const XML_Char *) vp.getNthItem(i+1));
		}
		else
		{
                         m_fAlign = 0.25;
		}

		i = findVecItem(&vp,"text-indent");
		if(i >= 0)
		{
                         m_fIndent = atof((const XML_Char *) vp.getNthItem(i+1));
		}
		else
		{
	                 m_fIndent = -0.25;
		}

		if(getAutoNum() != NULL)
		{
	                 UT_XML_strncpy( (XML_Char *) m_pszDelim, 80, getAutoNum()->getDelim());
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
                         UT_XML_strncpy( (XML_Char *) m_pszDecimal, 80, (const XML_Char *) "NULL");
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
       }
       //
       // Now do the Attributes
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
       }
       else
	        m_iID = 0;
       UT_DEBUGMSG(("SEVIOR: In fillDialogfromblock m_iListType = %d m_newListType= %d \n",m_iListType,m_newListType));
}

void  AP_Dialog_Lists::PopulateDialogData(void)
{
       const XAP_StringSet * pSS = m_pApp->getStringSet();
       m_isListAtPoint = getBlock()->isListItem();
       if(m_isListAtPoint == UT_TRUE)
       {   
			fillDialogFromBlock();
       }
       else
       {
			m_newListType = NOT_A_LIST;
			fillUncustomizedValues();
       }
       if(m_isListAtPoint == UT_TRUE)
       {
	      const char * tmp1 =  (char *) getBlock()->getListLabel();
	      strcpy( m_curListLabel, tmp1);
              m_curListLevel = getBlock()->getLevel();
              m_curStartValue = getAutoNum()->getStartValue32();
              m_iStartValue = getAutoNum()->getStartValue32();
	      //              m_iListType =  getAutoNum()->getType();
	      //	      m_newListType =  getAutoNum()->getType();
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
		      strcpy(m_curListType,tmp1);
	      }
       }
       else
       {
			m_iListType = NUMBERED_LIST;
			m_curStartValue = 1;
       }

}

UT_sint32  AP_Dialog_Lists::findVecItem(UT_Vector * v, char * key)
{
       UT_sint32 i = v->getItemCount();
       if(i < 0) 
	 return i;
       UT_sint32 j;
       char * pszV;
       for(j= 0; j<i ;j=j+2)
       {
	       pszV  = (char *) v->getNthItem(j);
	       if( (pszV != NULL) && (strcmp( pszV,key) == 0))
		      break;
       }
       if( j < i )
	      return j;
       else
	      return -1;
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

AV_View * AP_Dialog_Lists::getAvView(void)
{
        XAP_Frame * pFrame =  getActiveFrame();
        return  pFrame->getCurrentView();
}

	
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

AP_Lists_preview::AP_Lists_preview(GR_Graphics * gc, AP_Dialog_Lists * pLists)
	: XAP_Preview(gc)
{
        m_pLists = pLists;
}

AP_Lists_preview::~AP_Lists_preview()
{
}
 
AP_Dialog_Lists * AP_Lists_preview::getLists(void)
{
        return m_pLists;
}

void   AP_Lists_preview::setData(XML_Char * pszFont,float fAlign,float fIndent)
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
	         font = m_gc->findFont(m_pszFont, "normal", "", "normal", "", "16pt");
	}

        m_gc->setFont(font);
        UT_sint32 iDescent = m_gc->getFontDescent();
        UT_sint32 iAscent = m_gc->getFontAscent();
	UT_sint32 iFont = iDescent + iAscent;
	//
	//  clear our screen
	//
	m_gc->clearArea(0, 0, iWidth, iHeight);
	m_gc->setColor(clrBlack);
	UT_sint32 yoff = 5 ;
	UT_sint32 xoff = 5 ;
	UT_sint32 i,yloc,awidth,aheight,maxw;
	UT_sint32 twidth =0;
	UT_sint32 j;
	float z,fwidth;
	aheight = 16;
	fwidth = static_cast<float>(iWidth);
	
	// todo 8.5 should be the page width in inches

	z = (fwidth - 2.0*static_cast<float>(xoff)) /8.5;
        UT_sint32 indent = static_cast<UT_sint32>( z*(m_fAlign+m_fIndent));
	if(indent < 0)
	        indent = 0;
        maxw = 0;
	for(i=0; i<4; i++)
	{
	        XML_Char * lv = getLists()->getListLabel(i);
		UT_sint32 len =0;
		
		if(lv != NULL) 
		{
	  //
	  // This code is here because UT_UCS_copy_char is broken
	  //
		        len = UT_MIN(strlen(lv),51)  ;
			for(j=0; j<=len;j++)
			{
		               ucs_label[j] = (UT_UCSChar) *lv++;
			}
			ucs_label[len] = NULL;
			len = UT_UCS_strlen(ucs_label);
			yloc = yoff + iAscent + (iHeight - 2*yoff -iFont)*i/4;
			m_gc->drawChars(ucs_label,0,len,xoff+indent,yloc);
			twidth = m_gc->measureString(ucs_label,0,len,NULL);
			if(twidth > maxw)
			    maxw = twidth;
		}
	}
	//
	// Draw grey areas to represent text
	//
	UT_sint32 xx,yy;
	if(maxw > 0) 
	        maxw++;
	
	// todo 6.5 should be the page width in inches
        // UT_sint32 vspace = (iHeight - 2*yoff -iFont)*i/16;
	z = (fwidth - 2.0*static_cast<float>(xoff)) /6.5;
        UT_sint32 ialign = static_cast<UT_sint32>( z*m_fAlign);
        xx = xoff + maxw + ialign;
        if(xx < (xoff + maxw + indent))
	        xx = xoff + maxw + indent + 1;
	awidth = iWidth - 2*xoff - xx;
	for(i=0; i<4; i++)
	{
		yloc = yoff + iAscent + (iHeight - 2*yoff -iFont)*i/4;
		for(j=0; j< 2; j++)
		{
		        yy = yloc + 5 + j*21;
			m_gc->fillRect(clrGrey,xx,yy,awidth,aheight);
		}
	}
}





