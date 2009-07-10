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

char * AP_Dialog_Goto::s_pJumpTargets[] = {
	NULL,
	NULL,
	NULL,
//	"Picture",  TODO
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

}

char ** AP_Dialog_Goto::getJumpTargets(void)
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
	const XAP_StringSet * pSS = m_pApp->getStringSet();
	gchar * tmp = NULL;
	UT_uint32 title_width = 33;

	UT_UTF8String s;
	pSS->getValueUTF8(AP_STRING_ID_DLG_Goto_Title,s);
	
	UT_XML_cloneNoAmpersands(tmp, s.utf8_str());
	BuildWindowName((char *) m_WindowName,(char*)tmp,title_width);
	FREEP(tmp);
}							


void  AP_Dialog_Goto::setActiveFrame(XAP_Frame * /*pFrame*/)
{
	setView(getView());
	notifyActiveFrame(getActiveFrame());
}


// --------------------------- Setup Functions -----------------------------
bool AP_Dialog_Goto::setView(FV_View * /*view*/)
{
	m_pView =  (FV_View *) getActiveFrame()->getCurrentView();
	return true;
}

FV_View * AP_Dialog_Goto::getView(void)
{
	XAP_Frame * pFrame =  getActiveFrame();
	return  (FV_View *) pFrame->getCurrentView();
}

UT_uint32 AP_Dialog_Goto::getExistingBookmarksCount() const
{
	UT_return_val_if_fail (m_pView, 0);
	return m_pView->getDocument()->getBookmarkCount();
}

const gchar * AP_Dialog_Goto::getNthExistingBookmark(UT_uint32 n) const
{
	UT_return_val_if_fail (m_pView, NULL);
	return m_pView->getDocument()->getNthBookmark(n);
}
