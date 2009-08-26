/* AbiSource Application Framework
 * Copyright (C) 2009 J.M. Maurer <uwog@uwog.net>
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

#include <windows.h>

#include "xap_Frame.h"
#include "xap_Frame.h"
#include "ut_debugmsg.h"
#include "ap_Win32Preview_Annotation.h"
#include "xap_Win32DialogHelper.h"

AP_Win32Preview_Annotation::AP_Win32Preview_Annotation(XAP_DialogFactory * pDlgFactory,XAP_Dialog_Id id)
	: AP_Preview_Annotation(pDlgFactory,id)
{
	UT_DEBUGMSG(("AP_Win32Preview_Annotation: Preview annotation for Unix platform\n"));
}

AP_Win32Preview_Annotation::~AP_Win32Preview_Annotation(void)
{
	UT_DEBUGMSG(("Preview Annotation deleted %p \n",this));
	destroy();
}

void AP_Win32Preview_Annotation::runModeless(XAP_Frame * pFrame)
{
	UT_DEBUGMSG(("Preview Annotation runModeless %p \n",this));
	setActiveFrame(pFrame);
	
	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
}

void AP_Win32Preview_Annotation::activate(void)
{
	UT_DEBUGMSG(("AP_Win32Preview_Annotation::activate()\n"));

	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
}

void AP_Win32Preview_Annotation::draw(void)
{
	UT_DEBUGMSG(("Stubbed out AP_Win32Preview_Annotation::draw()\n"));
}

XAP_Dialog * AP_Win32Preview_Annotation::static_constructor(XAP_DialogFactory * pFactory, XAP_Dialog_Id id)
{
	return new AP_Win32Preview_Annotation(pFactory,id);
}

void AP_Win32Preview_Annotation::_constructWindow(void)
{
	XAP_App::getApp()->rememberModelessId(getDialogId(), static_cast<XAP_Dialog_Modeless *>(this));
	UT_DEBUGMSG(("Contructing Window width %d height %d left %d top %d \n",m_width,m_height,m_left,m_top));

	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
}

void AP_Win32Preview_Annotation::destroy(void)
{
	modeless_cleanup();

	UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
}



