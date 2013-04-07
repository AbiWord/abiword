/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2009 Hubert Figuiere 
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
#include "ap_Dialog_Goto.h"
#include "ap_Strings.h"

#include "xap_App.h"
#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"

#include "fl_DocLayout.h"
#include "fv_View.h"
#include "fl_BlockLayout.h"
#include "xap_Frame.h"
#include "pd_Document.h"

const char * AP_Dialog_Goto::s_pJumpTargets[] = {
	NULL,
	NULL,
	NULL,
//	"Picture",  TODO
	NULL,
	NULL,
	NULL
};


AP_Dialog_Goto::AP_Dialog_Goto(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_Modeless(pDlgFactory, id, "interface/dialoggoto")
{
  	m_pView = NULL;
	m_answer = a_CLOSE;
	if (s_pJumpTargets[0] == NULL)
	    _setupJumpTargets();
}

AP_Dialog_Goto::~AP_Dialog_Goto(void)
{
}

void AP_Dialog_Goto::_setupJumpTargets(void)
{
    const XAP_StringSet * pSS = m_pApp->getStringSet();

    s_pJumpTargets[0] = ::g_strdup(pSS->getValue (AP_STRING_ID_DLG_Goto_Target_Page));
    s_pJumpTargets[1] = ::g_strdup(pSS->getValue (AP_STRING_ID_DLG_Goto_Target_Line));
    s_pJumpTargets[2] = ::g_strdup(pSS->getValue (AP_STRING_ID_DLG_Goto_Target_Bookmark));
    //s_pJumpTargets[2] = ::g_strdup(pSS->getValue (AP_STRING_ID_DLG_Goto_Target_Picture)); //TODO
    s_pJumpTargets[3] = ::g_strdup(pSS->getValue (AP_STRING_ID_DLG_Goto_Target_XMLid));    
    s_pJumpTargets[4] = ::g_strdup(pSS->getValue (AP_STRING_ID_DLG_Goto_Target_Annotation));    

}

const char ** AP_Dialog_Goto::getJumpTargets(void)
{
	return (s_pJumpTargets);
}

AP_Dialog_Goto::tAnswer AP_Dialog_Goto::getAnswer(void) const
{
	// let our caller know if user hit ok, cancel, etc.
	return m_answer;
}


void AP_Dialog_Goto::ConstructWindowName(void)
{
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();
	gchar * tmp = NULL;

	std::string s;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Goto_Title, s);
	
	UT_XML_cloneNoAmpersands(tmp, s.c_str());
	BuildWindowName(m_WindowName, tmp, sizeof(m_WindowName));
	FREEP(tmp);
}							


void  AP_Dialog_Goto::setActiveFrame(XAP_Frame * /*pFrame*/)
{
	setView(getView());
	notifyActiveFrame(getActiveFrame());
}

// Actions
void AP_Dialog_Goto::performGoto(AP_JumpTarget target, const char * value) const
{
	UT_DEBUGMSG (("performGoto target='%d' number='%s'\n", target, value));
	m_pView->gotoTarget (target, value);
}

std::string AP_Dialog_Goto::performGotoNext(AP_JumpTarget target, UT_sint32 idx) const
{
	std::string dest;
	if(target == AP_JUMPTARGET_BOOKMARK) {
		if(!getExistingBookmarksCount()) {
			return dest;
		}
		if(idx >= 0) {
			idx++;
			// wrap it
			if(idx >= getExistingBookmarksCount()) {
				idx = 0;
			}
		}
		else {
			idx = 0;
		}
		dest = getNthExistingBookmark(idx);
		m_pView->gotoTarget (target, dest.c_str());	
	}
	else {
		m_pView->gotoTarget (target, "+1");
	}
	return dest;
}


std::string AP_Dialog_Goto::performGotoPrev(AP_JumpTarget target, UT_sint32 idx) const
{
	std::string dest;
	if(target == AP_JUMPTARGET_BOOKMARK) {
		if(!getExistingBookmarksCount()) {
			return dest;
		}
		idx--;
		if(idx < 0) {
			idx = getExistingBookmarksCount();
			if(idx) {
				idx--;
			}
		}
		dest = getNthExistingBookmark(idx);
		m_pView->gotoTarget (target, dest.c_str());
	}
	else {
		m_pView->gotoTarget (target, "-1");
	}
	return dest;
}



PD_DocumentRDFHandle
AP_Dialog_Goto::getRDF()
{
    FV_View* view = getView();
    PD_Document* doc = view->getDocument();
    PD_DocumentRDFHandle rdf = doc->getDocumentRDF();
    return rdf;
}
 

// --------------------------- Setup Functions -----------------------------
bool AP_Dialog_Goto::setView(FV_View * /*view*/)
{
	m_pView =  (FV_View *) getActiveFrame()->getCurrentView();
	return true;
}

FV_View * AP_Dialog_Goto::getView(void) const
{
	XAP_Frame * pFrame =  getActiveFrame();
	return  (FV_View *) pFrame->getCurrentView();
}

UT_sint32 AP_Dialog_Goto::getExistingBookmarksCount() const
{
	UT_return_val_if_fail (m_pView, 0);
	return m_pView->getDocument()->getBookmarkCount();
}

const std::string & AP_Dialog_Goto::getNthExistingBookmark(UT_sint32 n) const
{
	UT_ASSERT(m_pView);
	return m_pView->getDocument()->getNthBookmark(n);
}
