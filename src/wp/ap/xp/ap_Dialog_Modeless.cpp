/* AbiWord
 * Copyright (C) 2011 AbiSource, Inc.
 * Copyright (C) 2011 Ben Martin
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
#include "ut_std_string.h"
#include "ut_debugmsg.h"
#include "ap_Dialog_Modeless.h"
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


void AP_Dialog_Modeless::ConstructWindowName(void)
{
	const XAP_StringSet * pSS = XAP_App::getApp()->getStringSet();

	std::string s;
	pSS->getValueUTF8(getWindowTitleStringId(), s);

    s = UT_XML_cloneNoAmpersands( s );
	m_WindowName = BuildWindowName( s.c_str() );
}							

AP_Dialog_Modeless::AP_Dialog_Modeless( XAP_DialogFactory * pDlgFactory,
                                        XAP_Dialog_Id id,
                                        const char * helpUrl )
	: XAP_Dialog_Modeless( pDlgFactory, id, helpUrl )
    , m_pView(0)
    , m_answer( a_CLOSE )
{
}

AP_Dialog_Modeless::~AP_Dialog_Modeless(void)
{
    UT_DEBUGMSG(("AP_Dialog_Modeless::~AP_Dialog_Modeless()\n"));
}


void AP_Dialog_Modeless::setStatus( const std::string& /*msg*/ )
{
}


// --------------------------------------------------
// ---------------- boilerplate ---------------------

void
AP_Dialog_Modeless::maybeClosePopupPreviewBubbles()
{
    closePopupPreviewBubbles();
}

void
AP_Dialog_Modeless::closePopupPreviewBubbles()
{
  	FV_View* view = getView();
    m_bubbleBlocker = view->getBubbleBlocker();
}

void
AP_Dialog_Modeless::maybeReallowPopupPreviewBubbles()
{
    m_bubbleBlocker = FV_View_BubbleBlocker();
    UT_DEBUGMSG(("AP_Dialog_Modeless::modeless_cleanup()\n"));
}



AP_Dialog_Modeless::tAnswer AP_Dialog_Modeless::getAnswer(void) const
{
	// let our caller know if user hit ok, cancel, etc.
	return m_answer;
}

void  AP_Dialog_Modeless::setActiveFrame(XAP_Frame * /*pFrame*/)
{
	setView(getView());
	notifyActiveFrame(getActiveFrame());
}

bool AP_Dialog_Modeless::setView(FV_View * /*view*/)
{
	if (getActiveFrame())
		m_pView = (FV_View *) getActiveFrame()->getCurrentView();
	else
		m_pView = NULL;
	return true;
}

FV_View * AP_Dialog_Modeless::getView(void) const
{
	XAP_Frame * pFrame = getActiveFrame();

	if (pFrame)
		return (FV_View *) pFrame->getCurrentView();
	else
		return NULL;
}

