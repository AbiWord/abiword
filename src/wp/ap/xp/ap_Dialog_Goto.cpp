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
#include "ap_Dialog_Goto.h"

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"

#include "fl_DocLayout.h"
#include "fv_View.h"

AP_Dialog_Goto::AP_Dialog_Goto(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_FramePersistent(pDlgFactory,id)
{
	persist_targetData = NULL;
	persist_targetType = FV_JUMPTARGET_PAGE;
		
	m_pView = NULL;

	m_targetType = FV_JUMPTARGET_PAGE;
	m_targetData = NULL;

	m_didSomething = UT_FALSE;

	// is this used?
	m_answer = a_VOID;
}

AP_Dialog_Goto::~AP_Dialog_Goto(void)
{
	UT_ASSERT(!m_bInUse);

	FREEP(m_targetData);
	
	FREEP(persist_targetData);
}

void AP_Dialog_Goto::useStart(void)
{
	UT_DEBUGMSG(("AP_Dialog_Goto::useStart(void) I've been called\n"));

	XAP_Dialog_FramePersistent::useStart();

	// restore from persistent storage
	if (persist_targetData)
		UT_UCS_cloneString(&m_targetData, persist_targetData);
}

void AP_Dialog_Goto::useEnd(void)
{

	UT_DEBUGMSG(("AP_Dialog_Goto::useEnd(void) I've been called\n"));
	XAP_Dialog_FramePersistent::useEnd();

	// persistent dialogs don't destroy this data
	if (m_didSomething)
	{
		FREEP(persist_targetData);
		if (m_targetData)
			UT_UCS_cloneString(&persist_targetData, m_targetData);
	}
}

AP_Dialog_Goto::tAnswer AP_Dialog_Goto::getAnswer(void) const
{
	// let our caller know if user hit ok, cancel, etc.
	return m_answer;
}

// --------------------------- Setup Functions -----------------------------

UT_Bool AP_Dialog_Goto::setView(AV_View * view)
{
	// we can do a static cast from AV_View into FV_View,
	// so we can get WP specific information from it.
	// This could be bad once we introduce an
	// outline view, etc.
	UT_ASSERT(view);

	m_pView = static_cast<FV_View *>(view);

	return UT_TRUE;
}

AV_View * AP_Dialog_Goto::getView(void) const
{
	return m_pView;
}

UT_Bool	AP_Dialog_Goto::setTargetType(FV_JumpTarget target)
{
	m_targetType = target;
	return UT_TRUE;
}

FV_JumpTarget AP_Dialog_Goto::getTargetType(void)
{
	return m_targetType;
}

UT_Bool AP_Dialog_Goto::setTargetData(const UT_UCSChar * string)
{
	FREEP(m_targetData);
	return UT_UCS_cloneString(&m_targetData, string);
}

UT_UCSChar * AP_Dialog_Goto::getTargetData(void)
{
	UT_UCSChar * string = NULL;
	if (m_targetData)
	{
		if (UT_UCS_cloneString(&string, m_targetData))
			return string;
	}
	else
	{
		if (UT_UCS_cloneString_char(&string, ""))
			return string;
	}
	return NULL;
}

// --------------------------- Action Functions -----------------------------

UT_Bool AP_Dialog_Goto::gotoTarget(void)
{
	UT_ASSERT(m_pView);

	UT_ASSERT(m_targetData);

	// so we save our attributes to persistent storage
	m_didSomething = UT_TRUE;

	// call view to do the work
	return m_pView->gotoTarget(m_targetType, m_targetData);
}
