/* AbiWord
 * Copyright (C) 2003 Martin Sevior
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

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "ap_Dialog_FormatFootnotes.h"

AP_Dialog_FormatFootnotes::AP_Dialog_FormatFootnotes(XAP_DialogFactory * pDlgFactory,
					   XAP_Dialog_Id id)
  : XAP_Dialog_NonPersistent(pDlgFactory,id, "interface/dialogformatfootnotes"), m_answer(a_CANCEL)
{
}

AP_Dialog_FormatFootnotes::~AP_Dialog_FormatFootnotes(void)
{
}

void AP_Dialog_FormatFootnotes::setAnswer(AP_Dialog_FormatFootnotes::tAnswer a)
{
  m_answer = a;
}

AP_Dialog_FormatFootnotes::tAnswer AP_Dialog_FormatFootnotes::getAnswer(void) const
{
  return m_answer;
}

void  AP_Dialog_FormatFootnotes::setFrame(XAP_Frame * pFrame)
{
	m_pFrame = pFrame;
	m_pView = (FV_View *) pFrame->getCurrentView();
	m_pDocLayout = m_pView->getLayout();
	m_pDoc = m_pView->getDocument();
}

void AP_Dialog_FormatFootnotes::setFootnoteType(	FootnoteType iType)
{
 	UT_DEBUGMSG(("setFootnote Type %d \n",iType));
	m_iFootnoteType = iType;
	recalcTextValues();
}

void AP_Dialog_FormatFootnotes::recalcTextValues(void)
{
	m_pDocLayout->getStringFromFootnoteVal(m_sFootnoteVal ,m_iFootnoteVal ,m_iFootnoteType);
	m_pDocLayout->getStringFromFootnoteVal(m_sEndnoteVal ,m_iEndnoteVal ,m_iEndnoteType);
}

void AP_Dialog_FormatFootnotes::setEndnoteType(	FootnoteType iType)
{
 	UT_DEBUGMSG(("setEndnote Type %d \n",iType));
	m_iEndnoteType = iType;
	recalcTextValues();
}

void AP_Dialog_FormatFootnotes::setFootnoteVal(UT_sint32 iVal)
{
	m_iFootnoteVal = iVal;
	recalcTextValues();
}

void AP_Dialog_FormatFootnotes::setEndnoteVal(UT_sint32 iVal)
{
	m_iEndnoteVal = iVal;
	recalcTextValues();
}


FootnoteType AP_Dialog_FormatFootnotes::getFootnoteType(void)
{
	return m_iFootnoteType;
}


FootnoteType AP_Dialog_FormatFootnotes::getEndnoteType(void)
{
	return m_iEndnoteType;
}

UT_sint32 AP_Dialog_FormatFootnotes::getFootnoteVal(void)
{
	return m_iFootnoteVal;
}

UT_sint32 AP_Dialog_FormatFootnotes::getEndnoteVal(void)
{
	return m_iEndnoteVal;
}

void AP_Dialog_FormatFootnotes::getEndnoteValString(UT_String & sVal)
{
	sVal = m_sEndnoteVal;
}


void AP_Dialog_FormatFootnotes::getFootnoteValString(UT_String & sVal)
{
	sVal = m_sFootnoteVal;
}

void AP_Dialog_FormatFootnotes::setRestartFootnoteOnSection(bool bVal)
{
	m_bRestartFootSection = bVal;
}


bool AP_Dialog_FormatFootnotes::getRestartFootnoteOnSection(void)
{
	return m_bRestartFootSection;
}


void AP_Dialog_FormatFootnotes::setRestartFootnoteOnPage(bool bVal)
{
	m_bRestartFootPage = bVal;
}


bool AP_Dialog_FormatFootnotes::getRestartFootnoteOnPage(void)
{
	return m_bRestartFootPage;
}


void AP_Dialog_FormatFootnotes::setRestartEndnoteOnSection(bool bVal)
{
	m_bRestartEndSection = bVal;
}


bool AP_Dialog_FormatFootnotes::getRestartEndnoteOnSection(void)
{
	return m_bRestartEndSection;
}


void AP_Dialog_FormatFootnotes::setPlaceAtDocEnd(bool bVal)
{
	m_bPlaceAtDocEnd = bVal;
}


bool AP_Dialog_FormatFootnotes::getPlaceAtDocEnd(void)
{
	return m_bPlaceAtDocEnd;
}


void AP_Dialog_FormatFootnotes::setPlaceAtSecEnd(bool bVal)
{
	m_bPlaceAtSecEnd = bVal;
}


bool AP_Dialog_FormatFootnotes::getPlaceAtSecEnd(void)
{
	return m_bPlaceAtSecEnd;
}

void  AP_Dialog_FormatFootnotes::setInitialValues(void)
{
	m_iFootnoteVal = m_pDocLayout->getInitialFootVal();
	m_iEndnoteVal = m_pDocLayout->getInitialEndVal();
	m_iFootnoteType = m_pDocLayout->getFootnoteType();
	m_iEndnoteType = m_pDocLayout->getEndnoteType();
	m_bRestartFootSection = m_pDocLayout->getRestartFootOnSection();
	m_bRestartFootPage = m_pDocLayout->getRestartFootOnPage();
	m_bRestartEndSection = m_pDocLayout->getRestartEndOnSection();
	m_bPlaceAtDocEnd = m_pDocLayout->getPlaceEndAtDocEnd();
	m_bPlaceAtSecEnd = m_pDocLayout->getPlaceEndAtSecEnd();
	recalcTextValues();
}

void AP_Dialog_FormatFootnotes::updateDocWithValues(void)
{
	UT_String sFootType;
	UT_String sEndType;
	const gchar * pProps[19] = {"document-footnote-type",NULL,
								  "document-footnote-initial",NULL,
								  "document-footnote-restart-section",NULL,
								  "document-footnote-restart-page",NULL,
								  "document-endnote-type",NULL,
								  "document-endnote-initial",NULL,
								  "document-endnote-restart-section",NULL,
								  "document-endnote-place-endsection",NULL,
								  "document-endnote-place-enddoc",NULL,
								  NULL};
	switch (m_iFootnoteType)
	{
	case FOOTNOTE_TYPE_NUMERIC:
		sFootType = "numeric";
		break;
	case FOOTNOTE_TYPE_NUMERIC_SQUARE_BRACKETS:
		sFootType = "numeric-square-brackets";
		break;
	case FOOTNOTE_TYPE_NUMERIC_PAREN:
		sFootType = "numeric-paren";
		break;
	case FOOTNOTE_TYPE_NUMERIC_OPEN_PAREN:
		sFootType = "numeric-open-paren";
		break;
	case FOOTNOTE_TYPE_UPPER:
		sFootType = "upper";
		break;
	case FOOTNOTE_TYPE_UPPER_PAREN:
		sFootType = "upper-paren";
		break;
	case FOOTNOTE_TYPE_UPPER_OPEN_PAREN:
		sFootType = "upper-paren-open";
		break;
	case FOOTNOTE_TYPE_LOWER:
		sFootType = "lower";
		break;
	case FOOTNOTE_TYPE_LOWER_PAREN:
		sFootType = "lower-paren";
		break;
	case FOOTNOTE_TYPE_LOWER_OPEN_PAREN:
		sFootType = "lower-paren-open";
		break;
	case FOOTNOTE_TYPE_LOWER_ROMAN:
		sFootType = "lower-roman";
		break;
	case FOOTNOTE_TYPE_LOWER_ROMAN_PAREN:
		sFootType = "lower-roman-paren";
		break;
	case FOOTNOTE_TYPE_UPPER_ROMAN:
		sFootType = "upper-roman";
		break;
	case FOOTNOTE_TYPE_UPPER_ROMAN_PAREN:
		sFootType = "upper-roman-paren";
		break;
	default:
		sFootType = "numeric-square-brackets";
		break;
	}
	pProps[1] = sFootType.c_str();
	UT_String sFootInitial;
	UT_String_sprintf(sFootInitial,"%d",m_iFootnoteVal);
	pProps[3] = sFootInitial.c_str();
	if(m_bRestartFootSection)
	{
		pProps[5] = "1";
	}
	else
	{
		pProps[5] = "0";
	}
	if(m_bRestartFootPage)
	{
		pProps[7] = "1";
	}
	else
	{
		pProps[7] = "0";
	}
	switch (m_iEndnoteType)
	{
	case FOOTNOTE_TYPE_NUMERIC:
		sEndType = "numeric";
		break;
	case FOOTNOTE_TYPE_NUMERIC_SQUARE_BRACKETS:
		sEndType = "numeric-square-brackets";
		break;
	case FOOTNOTE_TYPE_NUMERIC_PAREN:
		sEndType = "numeric-paren";
		break;
	case FOOTNOTE_TYPE_NUMERIC_OPEN_PAREN:
		sEndType = "numeric-open-paren";
		break;
	case FOOTNOTE_TYPE_UPPER:
		sEndType = "upper";
		break;
	case FOOTNOTE_TYPE_UPPER_PAREN:
		sEndType = "upper-paren";
		break;
	case FOOTNOTE_TYPE_UPPER_OPEN_PAREN:
		sEndType = "upper-paren-open";
		break;
	case FOOTNOTE_TYPE_LOWER:
		sEndType = "lower";
		break;
	case FOOTNOTE_TYPE_LOWER_PAREN:
		sEndType = "lower-paren";
		break;
	case FOOTNOTE_TYPE_LOWER_OPEN_PAREN:
		sEndType = "lower-paren-open";
		break;
	case FOOTNOTE_TYPE_LOWER_ROMAN:
		sEndType = "lower-roman";
		break;
	case FOOTNOTE_TYPE_LOWER_ROMAN_PAREN:
		sEndType = "lower-roman-paren";
		break;
	case FOOTNOTE_TYPE_UPPER_ROMAN:
		sEndType = "upper-roman";
		break;
	case FOOTNOTE_TYPE_UPPER_ROMAN_PAREN:
		sEndType = "upper-roman-paren";
		break;
	default:
		sEndType = "numeric-square-brackets";
		break;
	}
	pProps[9] = sEndType.c_str();
	UT_String sEndInitial;
	UT_String_sprintf(sEndInitial,"%d",m_iEndnoteVal);
	pProps[11] = sEndInitial.c_str();
	if(m_bRestartEndSection)
	{
		pProps[13] = "1";
	}
	else
	{
		pProps[13] = "0";
	}
	if(m_bPlaceAtSecEnd)
	{
		pProps[15] = "1";
	}
	else
	{
		pProps[15] = "0";
	}
	if(m_bPlaceAtDocEnd)
	{
		pProps[17] = "1";
	}
	else
	{
		pProps[17] = "0";
	}
	m_pDoc->setProperties(pProps);
	m_pDoc->signalListeners(PD_SIGNAL_DOCPROPS_CHANGED_REBUILD);
}	


const FootnoteTypeDesc* 
AP_Dialog_FormatFootnotes::getFootnoteTypeLabelList(void)
{
	return s_FootnoteTypeDesc;
}
