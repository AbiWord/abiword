/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2000 AbiSource, Inc.
 * Copyright (C) 2005 Hubert Figuiere
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

#include "ap_Strings.h"
#include "xap_App.h"
#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"

#include "ap_Dialog_WordCount.h"

AP_Dialog_WordCount::AP_Dialog_WordCount(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_Modeless(pDlgFactory,id, "interface/dialogwordcount")
{
	m_answer = a_OK;
	memset(&m_count,0,sizeof(m_count));
}

AP_Dialog_WordCount::~AP_Dialog_WordCount(void)
{
}

void AP_Dialog_WordCount::localizeDialog(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet ();
	std::string str;

	pSS->getValueUTF8(AP_STRING_ID_DLG_WordCount_WordCountTitle, str);
	setWidgetLabel(DIALOG_WID, str); 

 	pSS->getValueUTF8(AP_STRING_ID_DLG_WordCount_Words, str);
	setWidgetLabel(WORDS_LBL_WID, str); 
	pSS->getValueUTF8(AP_STRING_ID_DLG_WordCount_Words_No_Notes, str);
	setWidgetLabel(WORDSNF_LBL_WID, str);
	pSS->getValueUTF8(AP_STRING_ID_DLG_WordCount_Pages, str);
	setWidgetLabel(PAGES_LBL_WID, str);
	pSS->getValueUTF8(AP_STRING_ID_DLG_WordCount_Characters_Sp, str);
	setWidgetLabel(CHARSP_LBL_WID, str);
	pSS->getValueUTF8(AP_STRING_ID_DLG_WordCount_Characters_No, str);
	setWidgetLabel(CHARNSP_LBL_WID, str);
	pSS->getValueUTF8(AP_STRING_ID_DLG_WordCount_Lines, str);
	setWidgetLabel(LINES_LBL_WID, str);
	pSS->getValueUTF8(AP_STRING_ID_DLG_WordCount_Paragraphs, str);
	setWidgetLabel(PARA_LBL_WID, str);
}


void AP_Dialog_WordCount::updateDialogData(void)
{
	setWidgetValueInt(WORDS_VAL_WID, m_count.word);
	setWidgetValueInt(WORDSNF_VAL_WID, m_count.words_no_notes);
	setWidgetValueInt(PARA_VAL_WID, m_count.para);
	setWidgetValueInt(CHARSP_VAL_WID, m_count.ch_sp);
	setWidgetValueInt(CHARNSP_VAL_WID, m_count.ch_no);
	setWidgetValueInt(LINES_VAL_WID, m_count.line);
	setWidgetValueInt(PAGES_VAL_WID, m_count.page);

	setWidgetLabel (TITLE_LBL_WID, getActiveFrame()->getTitle());
}



AP_Dialog_WordCount::tAnswer AP_Dialog_WordCount::getAnswer(void) const
{
	return m_answer;
}

FV_DocCount AP_Dialog_WordCount::getCount(void) const
{
	return m_count;
}

void AP_Dialog_WordCount::setCount(FV_DocCount nCount)
{
	m_count = nCount;
}

void AP_Dialog_WordCount::setCountFromActiveFrame(void)
{
	if(!getActiveFrame())
		return;

	FV_View * pview = static_cast<FV_View *>(getActiveFrame()->getCurrentView());
	if(!pview->isLayoutFilling())
	{
		setCount(pview->countWords(true));
	}
}


void AP_Dialog_WordCount::ConstructWindowName(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	gchar * tmp = NULL;
	UT_XML_cloneNoAmpersands(tmp, pSS->getValue(AP_STRING_ID_DLG_WordCount_WordCountTitle));
	BuildWindowName(static_cast<char *>(m_WindowName),static_cast<char*>(tmp),sizeof(m_WindowName));
	FREEP(tmp);
}

void AP_Dialog_WordCount::setActiveFrame(XAP_Frame* /*pFrame*/)
{
	notifyActiveFrame(getActiveFrame());
}
