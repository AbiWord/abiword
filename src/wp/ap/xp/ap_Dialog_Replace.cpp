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

#include "ap_Dialog_Replace.h"
#include "ap_Dialog_Id.h"
#include "ap_Strings.h"

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"

#include "fl_DocLayout.h"
#include "fv_View.h"


AP_Dialog_Replace::AP_Dialog_Replace(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_Modeless(pDlgFactory,id, "interface/dialogreplace.html")
{
	persist_findString = NULL;
	persist_replaceString = NULL;
	persist_matchCase = true;

	m_pView = NULL;
	m_pFrame = NULL;
	
	m_findString = NULL;
	m_replaceString = NULL;
	m_matchCase = false;

	m_didSomething = false;

	// is this used?
	m_answer = a_VOID;
}

AP_Dialog_Replace::~AP_Dialog_Replace(void)
{
  //	UT_ASSERT(!m_bInUse);

	FREEP(m_findString);
	FREEP(m_replaceString);

	FREEP(persist_findString);
	FREEP(persist_replaceString);
}

void AP_Dialog_Replace::useStart(void)
{
  	UT_DEBUGMSG(("AP_Dialog_Replace::useStart(void) I've been called\n"));

	// restore from persistent storage
	if (persist_findString)
		UT_UCS4_cloneString(&m_findString, persist_findString);
	if (persist_replaceString)
		UT_UCS4_cloneString(&m_replaceString, persist_replaceString);

	m_matchCase = persist_matchCase;

}

void AP_Dialog_Replace::useEnd(void)
{

	UT_DEBUGMSG(("AP_Dialog_Replace::useEnd(void) I've been called\n"));
	
	// persistent dialogs don't destroy this data
	if (m_didSomething)
	{
		FREEP(persist_findString);
		if (m_findString)
			UT_UCS4_cloneString(&persist_findString, m_findString);
		
		FREEP(persist_replaceString);
		if (m_replaceString)
			UT_UCS4_cloneString(&persist_replaceString, m_replaceString);

		persist_matchCase = m_matchCase;
	}
}

AP_Dialog_Replace::tAnswer AP_Dialog_Replace::getAnswer(void) const
{
	// let our caller know if user hit ok, cancel, etc.
	return m_answer;
}

// --------------------------- Setup Functions -----------------------------

bool AP_Dialog_Replace::setView(AV_View * view)
{
	// we can do a static cast from AV_View into FV_View,
	// so we can get WP specific information from it.
	// This could be bad once we introduce an
	// outline view, etc.
	UT_ASSERT(view);

	m_pFrame = (XAP_Frame *) getActiveFrame();
	UT_ASSERT(m_pFrame);
	
	m_pView = static_cast<FV_View *>(getActiveFrame()->getCurrentView());

	getFvView()->findSetStartAtInsPoint();
	
	return true;
}

AV_View * AP_Dialog_Replace::getView(void) 
{
	return getActiveFrame()->getCurrentView();
}

void  AP_Dialog_Replace::setActiveFrame(XAP_Frame *pFrame)
{
        setView(getView());
	notifyActiveFrame(getActiveFrame());
}

void  AP_Dialog_Replace::ConstructWindowName(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	XML_Char * tmp = NULL;
        UT_uint32 title_width = 0;
	// conditionally set title
	if (m_id == AP_DIALOG_ID_FIND)
	{
                UT_XML_cloneNoAmpersands(tmp, pSS->getValue(AP_STRING_ID_DLG_FR_FindTitle));
                title_width = 30;
	}
	else
	{
                UT_XML_cloneNoAmpersands(tmp, pSS->getValue(AP_STRING_ID_DLG_FR_ReplaceTitle));	
                title_width = 60;
	}
        BuildWindowName((char *) m_WindowName,(char*)tmp,title_width);
        FREEP(tmp);
}

FV_View * AP_Dialog_Replace::getFvView(void) 
{
	return static_cast<FV_View *>(getView());
}

bool AP_Dialog_Replace::setFindString(const UT_UCSChar * string)
{
	if (string && m_findString && UT_UCS4_strcmp(string, m_findString) != 0)
	{
		// When search parameters change, clear any existing selection to
		// avoid replacement of the previous search string.
		getFvView()->cmdUnselectSelection();
	}

	FREEP(m_findString);
	return UT_UCS4_cloneString(&m_findString, string);
}

UT_UCSChar * AP_Dialog_Replace::getFindString(void)
{
	UT_UCSChar * string = NULL;
	if (m_findString)
	{
		if (UT_UCS4_cloneString(&string, m_findString))
			return string;
	}
	else
	{
		if (UT_UCS4_cloneString_char(&string, ""))
			return string;
	}
	return NULL;
}

bool AP_Dialog_Replace::setReplaceString(const UT_UCSChar * string)
{
	FREEP(m_replaceString);
	return UT_UCS4_cloneString(&m_replaceString, string);
}

UT_UCSChar * AP_Dialog_Replace::getReplaceString(void)
{
	UT_UCSChar * string = NULL;
	if (m_replaceString)
	{
		if (UT_UCS4_cloneString(&string, m_replaceString))
			return string;
	}
	else
	{
		if (UT_UCS4_cloneString_char(&string, ""))
			return string;
	}

	return NULL;
}
	
bool AP_Dialog_Replace::setMatchCase(bool match)
{
	if (match != m_matchCase)
	{
		// When search parameters change, clear any existing selection to
		// avoid replacement of the previous search string.
		getFvView()->cmdUnselectSelection();
	}

	m_matchCase = match;
	return true;
}

bool	AP_Dialog_Replace::getMatchCase(void)
{
	return m_matchCase;
}

// --------------------------- Action Functions -----------------------------

bool AP_Dialog_Replace::findNext()
{
	UT_ASSERT(m_findString);
	//UT_ASSERT(m_replaceString);
	
	// so we save our attributes to persistent storage
	m_didSomething = true;

	bool bDoneEntireDocument = false;

	// update the view's automatic "find next" string
	getFvView()->findSetNextString(m_findString, m_matchCase);
	
	// call view to do the work
	bool result = getFvView()->findNext(m_findString, m_matchCase, 
										bDoneEntireDocument);

	if (bDoneEntireDocument == true)
		_messageFinishedFind();

	return result;
}

bool AP_Dialog_Replace::findReplace()
{

	UT_ASSERT(m_findString);
	UT_ASSERT(m_replaceString);
	
	// so we save our attributes to persistent storage
	m_didSomething = true;

	bool bDoneEntireDocument = false;
	
	// update the view's automatic "find next" string
	getFvView()->findSetNextString(m_findString, m_matchCase);
	
	// call view to do the work
	bool result = getFvView()->findReplace(m_findString, m_replaceString,
										   m_matchCase, bDoneEntireDocument);

	if (bDoneEntireDocument == true)
		_messageFinishedFind();

	return result;
}

bool AP_Dialog_Replace::findReplaceAll()
{

	UT_ASSERT(m_findString);
	UT_ASSERT(m_replaceString);
	
	// so we save our attributes to persistent storage
	m_didSomething = true;

	// update the view's automatic "find next" string
	getFvView()->findSetNextString(m_findString, m_matchCase);
	
	// call view to do the work
	UT_uint32 numReplaced = getFvView()->findReplaceAll(m_findString,
													m_replaceString,
													m_matchCase);

	_messageFinishedReplace(numReplaced);

	return true;
}

void AP_Dialog_Replace::_messageFinishedFind(void)
{
  const XAP_StringSet * pSS = m_pApp->getStringSet();
	_messageBox(pSS->getValue(AP_STRING_ID_DLG_FR_FinishedFind));
}

void AP_Dialog_Replace::_messageFinishedReplace(UT_uint32 numReplaced)
{
	char message[512];
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	sprintf(message, pSS->getValue(AP_STRING_ID_DLG_FR_FinishedReplace), 
		numReplaced);
	_messageBox(message);
}

void AP_Dialog_Replace::_messageBox(const char * message)
{
	getActiveFrame()->showMessageBox(message,
								XAP_Dialog_MessageBox::b_O,
								XAP_Dialog_MessageBox::a_OK);
}
