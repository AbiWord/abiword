/* AbiSource Application Framework
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

//#include "ap_UnixDialog_All.h"	// why is this referenced in an XP file?
#include "ap_Dialog_Replace.h"

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dialog_MessageBox.h"

#include "fl_DocLayout.h"
#include "fv_View.h"

#define FREEP(p)	do { if (p) free(p); (p)=NULL; } while (0)
#define DELETEP(p)	do { if (p) delete(p); (p)=NULL; } while (0)

AP_Dialog_Replace::AP_Dialog_Replace(AP_DialogFactory * pDlgFactory, AP_Dialog_Id id)
	: AP_Dialog_FramePersistent(pDlgFactory,id)
{
	persist_findString = NULL;
	persist_replaceString = NULL;
	persist_matchCase = UT_TRUE;

	m_pView = NULL;
	m_pFrame = NULL;
	
	m_findString = NULL;
	m_replaceString = NULL;
	m_matchCase = UT_TRUE;

	m_didSomething = UT_FALSE;

	// is this used?
	m_answer = a_VOID;
}

AP_Dialog_Replace::~AP_Dialog_Replace(void)
{
	UT_ASSERT(!m_bInUse);

	FREEP(m_findString);
	FREEP(m_replaceString);

	FREEP(persist_findString);
	FREEP(persist_replaceString);
}

void AP_Dialog_Replace::useStart(void)
{
	UT_DEBUGMSG(("AP_Dialog_Replace::useStart(void) I've been called\n"));

	AP_Dialog_FramePersistent::useStart();

	// restore from persistent storage
	if (persist_findString)
		UT_UCS_cloneString(&m_findString, persist_findString);
	if (persist_replaceString)
		UT_UCS_cloneString(&m_replaceString, persist_replaceString);

	m_matchCase = persist_matchCase;
}

void AP_Dialog_Replace::useEnd(void)
{

	UT_DEBUGMSG(("AP_Dialog_Replace::useEnd(void) I've been called\n"));
	AP_Dialog_FramePersistent::useEnd();

	// Let the view know it doens't need to maintain it's
	// "find" or "replace" session-oriented counters
	UT_ASSERT(m_pView);
	m_pView->findReset();
	
	// persistent dialogs don't destroy this data
	if (m_didSomething)
	{
		FREEP(persist_findString);
		if (m_findString)
			UT_UCS_cloneString(&persist_findString, m_findString);
		
		FREEP(persist_replaceString);
		if (m_replaceString)
			UT_UCS_cloneString(&persist_replaceString, m_replaceString);

		persist_matchCase = m_matchCase;
	}
}

AP_Dialog_Replace::tAnswer AP_Dialog_Replace::getAnswer(void) const
{
	// let our caller know if user hit ok, cancel, etc.
	return m_answer;
}

// --------------------------- Setup Functions -----------------------------

UT_Bool AP_Dialog_Replace::setView(AV_View * view)
{
	// we can do a static cast from AV_View into FV_View,
	// so we can get WP specific information from it.
	// This could be bad once we introduce an
	// outline view, etc.
	UT_ASSERT(view);

	m_pFrame = (AP_Frame *) view->getParentData();
	UT_ASSERT(m_pFrame);
	
	m_pView = static_cast<FV_View *>(view);

	// must resize doc positions so we're within bounds
	// for the whole find
	m_pView->findReset();

	return UT_TRUE;
}

AV_View * AP_Dialog_Replace::getView(void) const
{
	return m_pView;
}

UT_Bool AP_Dialog_Replace::setFindString(const UT_UCSChar * string)
{
	FREEP(m_findString);
	return UT_UCS_cloneString(&m_findString, string);
}

UT_UCSChar * AP_Dialog_Replace::getFindString(void)
{
	UT_UCSChar * string = NULL;
	if (m_findString)
	{
		if (UT_UCS_cloneString(&string, m_findString))
			return string;
	}
	else
	{
		if (UT_UCS_cloneString_char(&string, ""))
			return string;
	}
	return NULL;
}

UT_Bool AP_Dialog_Replace::setReplaceString(const UT_UCSChar * string)
{
	FREEP(m_replaceString);
	return UT_UCS_cloneString(&m_replaceString, string);
}

UT_UCSChar * AP_Dialog_Replace::getReplaceString(void)
{
	UT_UCSChar * string = NULL;
	if (m_replaceString)
	{
		if (UT_UCS_cloneString(&string, m_replaceString))
			return string;
	}
	else
	{
		if (UT_UCS_cloneString_char(&string, ""))
			return string;
	}

	return NULL;
}
	
UT_Bool AP_Dialog_Replace::setMatchCase(UT_Bool match)
{
	m_matchCase = match;
	return UT_TRUE;
}

UT_Bool	AP_Dialog_Replace::getMatchCase(void)
{
	return m_matchCase;
}

// --------------------------- Action Functions -----------------------------

UT_Bool AP_Dialog_Replace::findNext()
{
	UT_ASSERT(m_pView);

	UT_ASSERT(m_findString);
	//UT_ASSERT(m_replaceString);
	
	// so we save our attributes to persistent storage
	m_didSomething = UT_TRUE;

	UT_Bool bDoneEntireDocument = UT_FALSE;

	// update the view's automatic "find next" string
	m_pView->findSetNextString(m_findString);
	
	// call view to do the work
	UT_Bool result = m_pView->findNext(m_findString, UT_TRUE, NULL, &bDoneEntireDocument);

	if (bDoneEntireDocument == UT_TRUE)
		_messageFinishedFind();

	return result;
}

UT_Bool AP_Dialog_Replace::findReplace()
{
	UT_ASSERT(m_pView);

	UT_ASSERT(m_findString);
	UT_ASSERT(m_replaceString);
	
	// so we save our attributes to persistent storage
	m_didSomething = UT_TRUE;

	UT_Bool bDoneEntireDocument = UT_FALSE;
	
	// update the view's automatic "find next" string
	m_pView->findSetNextString(m_findString);
	
	// call view to do the work
	UT_Bool result = m_pView->findReplace(m_findString, m_replaceString, NULL, &bDoneEntireDocument);

	if (bDoneEntireDocument == UT_TRUE)
		_messageFinishedFind();

	return result;
}

UT_Bool AP_Dialog_Replace::findReplaceAll()
{
	UT_ASSERT(m_pView);

	UT_ASSERT(m_findString);
	UT_ASSERT(m_replaceString);
	
	// so we save our attributes to persistent storage
	m_didSomething = UT_TRUE;

	UT_Bool bDoneEntireDocument = UT_FALSE;
	
	// update the view's automatic "find next" string
	m_pView->findSetNextString(m_findString);
	
	// call view to do the work
	UT_uint32 numReplaced = m_pView->findReplaceAll(m_findString, m_replaceString, NULL, &bDoneEntireDocument);

	if (bDoneEntireDocument == UT_TRUE)
		_messageFinishedReplace(numReplaced);

	return UT_TRUE;
}

void AP_Dialog_Replace::_messageFinishedFind(void)
{
	_messageBox("AbiWord has finished searching the document.");
}

void AP_Dialog_Replace::_messageFinishedReplace(UT_uint32 numReplaced)
{
	char message[512];

	sprintf(message, "AbiWord has finished its search of the document and has made %ld replacements.", numReplaced);
	_messageBox(message);
}

void AP_Dialog_Replace::_messageBox(char * message)
{
	AP_DialogFactory * pDialogFactory
		= (AP_DialogFactory *)(m_pFrame->getDialogFactory());

	AP_Dialog_MessageBox * pDialog
		= (AP_Dialog_MessageBox *)(pDialogFactory->requestDialog(XAP_DIALOG_ID_MESSAGE_BOX));
	UT_ASSERT(pDialog);

	pDialog->setMessage(message);
	pDialog->setButtons(AP_Dialog_MessageBox::b_O);
	pDialog->setDefaultAnswer(AP_Dialog_MessageBox::a_OK);

	pDialog->runModal(m_pFrame);

	pDialogFactory->releaseDialog(pDialog);
}
