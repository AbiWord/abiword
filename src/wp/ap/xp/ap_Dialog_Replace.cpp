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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "ap_Features.h"

#include "ut_assert.h"
#include "ut_string.h"
#include "ut_string_class.h"
#include "ut_debugmsg.h"

#include "ap_Dialog_Replace.h"
#include "ap_Dialog_Id.h"
#include "ap_Strings.h"

#include "xap_App.h"
#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"

#include "fl_DocLayout.h"
#include "fv_View.h"


AP_Dialog_Replace::AP_Dialog_Replace(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_Modeless(pDlgFactory,id, "interface/dialogreplace"),
	m_findString(NULL),
	m_replaceString(NULL)
{
	m_pView = NULL;
	m_pFrame = NULL;
	

	// is this used?
	m_answer = a_VOID;
	
	UT_DEBUGMSG(("FODDEX: creating find/replace dialog\n"));
}

AP_Dialog_Replace::~AP_Dialog_Replace(void)
{
	
	// clean up memory
	UT_sint32 i;
	for (i=0; i<m_findList.getItemCount(); i++) 
	{
		UT_UCS4Char* string = static_cast<UT_UCS4Char*>(m_findList.getNthItem(i));
		if (string) 
			FREEP(string);
	}
	for (i=0; i<m_replaceList.getItemCount(); i++) 
	{
		UT_UCS4Char* string = static_cast<UT_UCS4Char*>(m_replaceList.getNthItem(i));
		if (string) 
			FREEP(string);
	}
	FREEP(m_findString);
	FREEP(m_replaceString);
}

void AP_Dialog_Replace::useStart(void)
{
  	UT_DEBUGMSG(("AP_Dialog_Replace::useStart(void) I've been called\n"));

	// restore from view
	m_findString = getFvView()->findGetFindString();
	m_replaceString = getFvView()->findGetReplaceString();
}

void AP_Dialog_Replace::useEnd(void)
{

	UT_DEBUGMSG(("AP_Dialog_Replace::useEnd(void) I've been called\n"));
	
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
	UT_return_val_if_fail (view, false);

	m_pFrame = (XAP_Frame *) getActiveFrame();
	UT_return_val_if_fail (m_pFrame, false);
	
	m_pView = static_cast<FV_View *>(getActiveFrame()->getCurrentView());

	getFvView()->findSetStartAtInsPoint();
	
	return true;
}

AV_View * AP_Dialog_Replace::getView(void) 
{
	return getActiveFrame()->getCurrentView();
}

void  AP_Dialog_Replace::setActiveFrame(XAP_Frame * /*pFrame*/)
{
        setView(getView());
	notifyActiveFrame(getActiveFrame());
}

void  AP_Dialog_Replace::ConstructWindowName(void)
{
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	gchar * tmp = NULL;
	// conditionally set title
	std::string s;
	
	if (m_id == AP_DIALOG_ID_FIND)
	{
		pSS->getValueUTF8(AP_STRING_ID_DLG_FR_FindTitle,s);
		UT_XML_cloneNoAmpersands(tmp, s.c_str());
	}
	else
	{
		pSS->getValueUTF8(AP_STRING_ID_DLG_FR_ReplaceTitle,s);
		UT_XML_cloneNoAmpersands(tmp, s.c_str());	
	}
	BuildWindowName((char *) m_WindowName,(char*)tmp,sizeof(m_WindowName));
	FREEP(tmp);
}

FV_View * AP_Dialog_Replace::getFvView(void) 
{
	return static_cast<FV_View *>(getView());
}

void AP_Dialog_Replace::setFindString(const UT_UCSChar * string)
{
	UT_UCSChar *findString = getFvView()->findGetFindString();  // caller must FREEP
	if (string && findString && UT_UCS4_strcmp(string, findString) != 0)
	{
		// When search parameters change, clear any existing selection to
		// avoid replacement of the previous search string.
		getFvView()->cmdUnselectSelection();
	}
	FREEP(findString);

	getFvView()->findSetFindString(string);
}

/*!
	Caller must FREEP() return value
 */
UT_UCSChar * AP_Dialog_Replace::getFindString(void)
{
	UT_UCSChar * string = NULL;
	string = getFvView()->findGetFindString();
	if (string)
	{
		return string;
	}
	else
	{
		if (UT_UCS4_cloneString_char(&string, "")) {
			return string;
		}
	}
	return NULL;
}

void AP_Dialog_Replace::setReplaceString(const UT_UCSChar * string)
{
	UT_DEBUGMSG(("AP_dlg_replace::setReplaceString()\n"));

	getFvView()->findSetReplaceString(string);
}

UT_UCSChar * AP_Dialog_Replace::getReplaceString(void)
{
	UT_UCSChar * string = NULL;
	UT_UCSChar * replaceString = getFvView()->findGetReplaceString();
	
	if (replaceString)
	{
		return replaceString;
	}
	else
	{
		if (UT_UCS4_cloneString_char(&string, ""))
			return string;
	}

	return NULL;
}
	
void AP_Dialog_Replace::setMatchCase(bool match)
{
	UT_DEBUGMSG(("AP_dlg_replace::setMatchCase(%d)\n", match));

	if (match != getFvView()->findGetMatchCase())
	{
	  // When search parameters change, clear any existing selection to
	  // avoid replacement of the previous search string.
	  if ( !getFvView()->isSelectionEmpty() )
	    getFvView()->cmdUnselectSelection();
	}

	getFvView()->findSetMatchCase(match);
}

bool	AP_Dialog_Replace::getMatchCase(void)
{
	UT_DEBUGMSG(("AP_dlg_replace::getMatchCase()\n"));
	return getFvView()->findGetMatchCase();
}

void AP_Dialog_Replace::setReverseFind(bool newValue)
{
	UT_DEBUGMSG(("AP_dlg_replace::setReverseFind(%d)\n", newValue));

	getFvView()->findSetReverseFind(newValue);
}

bool AP_Dialog_Replace::getReverseFind(void)
{
	UT_DEBUGMSG(("AP_dlg_replace::getReverseFind()\n"));
	return getFvView()->findGetReverseFind();
}

void AP_Dialog_Replace::setWholeWord(bool newValue)
{
	UT_DEBUGMSG(("AP_dlg_replace::setWholeWord(%d)\n", newValue));

	getFvView()->findSetWholeWord(newValue);
}

bool AP_Dialog_Replace::getWholeWord(void)
{
	UT_DEBUGMSG(("AP_dlg_replace::getWholeWord()\n"));
	return getFvView()->findGetWholeWord();
}

// --------------------------- Action Functions -----------------------------

bool AP_Dialog_Replace::findPrev()
{
	UT_DEBUGMSG(("AP_dlg_replace::findPrev()\n"));
	
	bool bDoneEntireDocument = false;

	// call view to do the work
	bool result = getFvView()->findPrev(bDoneEntireDocument);

	if (bDoneEntireDocument == true)
		_messageFinishedFind();

	return result;
}

bool AP_Dialog_Replace::findNext()
{
	
	// manage the list of find strings
	UT_UCSChar* findString = getFindString();
	UT_UCSChar* replaceString = getReplaceString();

	bool var1 = _manageList(&m_findList, findString);
	bool var2 = _manageList(&m_replaceList, replaceString);
	if (var1 || var2)
	{
		_updateLists();
	}
	FREEP(findString);
	FREEP(replaceString);
	
	bool bDoneEntireDocument = false;

	// call view to do the work
	bool result = getFvView()->findNext(bDoneEntireDocument);

	if (bDoneEntireDocument == true)
		_messageFinishedFind();

	return result;
}

bool AP_Dialog_Replace::findReplaceReverse()
{
	
	bool bDoneEntireDocument = false;

	
	// call view to do the work
	bool result = getFvView()->findReplaceReverse(bDoneEntireDocument);


	if (bDoneEntireDocument == true)
		_messageFinishedFind();

	return result;
}

bool AP_Dialog_Replace::findReplace()
{
	
	// manage the list of find strings
	UT_UCSChar* findString = getFindString();
	UT_UCSChar* replaceString = getReplaceString();

	// manage the list of find & replace strings
	bool var1 = _manageList(&m_findList, findString);
	bool var2 = _manageList(&m_replaceList, replaceString);
	if (var1 || var2)
	{
		_updateLists();
	}
	
	bool bDoneEntireDocument = false;
	
	// call view to do the work
	bool result = getFvView()->findReplace(bDoneEntireDocument);

	if (bDoneEntireDocument == true)
		_messageFinishedFind();

	return result;
}

bool AP_Dialog_Replace::findReplaceAll()
{

	// manage the list of find & replace strings
	UT_UCSChar* findString = getFindString();
	UT_UCSChar* replaceString = getReplaceString();

	bool var1 = _manageList(&m_findList, findString);
	bool var2 = _manageList(&m_replaceList, replaceString);
	if (var1 || var2)
	{
		_updateLists();
	}
	
	FREEP(findString);
	FREEP(replaceString);
	
	// call view to do the work
	UT_uint32 numReplaced = getFvView()->findReplaceAll();

	_messageFinishedReplace(numReplaced);

	return true;
}

void AP_Dialog_Replace::_messageFinishedFind(void)
{
	getActiveFrame()->showMessageBox(AP_STRING_ID_DLG_FR_FinishedFind,
					 XAP_Dialog_MessageBox::b_O,
					 XAP_Dialog_MessageBox::a_OK);
}

void AP_Dialog_Replace::_messageFinishedReplace(UT_uint32 numReplaced)
{
	XAP_Dialog_MessageBox * message = 
	  getActiveFrame()->createMessageBox(AP_STRING_ID_DLG_FR_FinishedReplace,
					     XAP_Dialog_MessageBox::b_O,
					     XAP_Dialog_MessageBox::a_OK,
					     numReplaced);
	getActiveFrame()->showMessageBox(message);
}

bool AP_Dialog_Replace::_manageList(UT_GenericVector<UT_UCS4Char*>* list, UT_UCSChar* string)
{
	UT_UCS4String us(string);
	UT_sint32 i = 0;
	bool found = false;
	
	UT_DEBUGMSG(("FODDEX: AP_Dialog_Replace::_manageList: called\n"));

	// check if the current string is already in the list
	for (i=0; i<list->getItemCount(); i++) 
	{
		if (!UT_UCS4_strcmp(string, list->getNthItem(i)))
		{
			found = true;
			break;
		}
	}
	
	UT_UCSChar * clone = NULL;		
	if (UT_UCS4_cloneString(&clone, string))
	{
		if (!found) 
		{
			// if not present, just add it to the internal list
			list->insertItemAt(clone, 0);
			UT_DEBUGMSG(("FODDEX: adding '%s' to list\n", us.utf8_str()));
			return true;
		} else {
			// g_free the old string
			UT_UCSChar* temp = static_cast<UT_UCSChar*>(list->getNthItem(i));
			if (temp) FREEP(temp);
			// remove the reference from the list
			list->deleteNthItem(i);
			// add it again to the top of the list
			list->insertItemAt(clone, 0);
		}
	} else {
		UT_DEBUGMSG(("FODDEX: warning, failed to clone UCS4 string: '%s'\n", us.utf8_str()));
		return false;
	}
	
	// cloning failed or the string already existed, so do nothing
	UT_DEBUGMSG(("FODDEX: string '%s' already in list\n", us.utf8_str()));
	return false;
}
