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
#include "ap_Dialog_Goto.h"

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dialog_MessageBox.h"

#include "fl_DocLayout.h"
#include "fv_View.h"

#define FREEP(p)	do { if (p) free(p); (p)=NULL; } while (0)
#define DELETEP(p)	do { if (p) delete(p); (p)=NULL; } while (0)

AP_Dialog_Goto::AP_Dialog_Goto(AP_DialogFactory * pDlgFactory, AP_Dialog_Id id)
	: AP_Dialog_FramePersistent(pDlgFactory,id)
{
	_m_pageNumberString = NULL;

	m_pView = NULL;
	m_pFrame = NULL;
	
	m_pageNumberString = NULL;

	m_didSomething = UT_FALSE;

	// is this used?
	m_answer = a_VOID;
}

AP_Dialog_Goto::~AP_Dialog_Goto(void)
{
	UT_ASSERT(!m_bInUse);

	FREEP(m_pageNumberString);
}

void AP_Dialog_Goto::useStart(void)
{
	UT_DEBUGMSG(("AP_Dialog_Goto::useStart(void) I've been called\n"));

	AP_Dialog_FramePersistent::useStart();

	// restore from persistent storage
	if (_m_pageNumberString)
		UT_UCS_cloneString(&m_pageNumberString, _m_pageNumberString);
}

void AP_Dialog_Goto::useEnd(void)
{

	UT_DEBUGMSG(("AP_Dialog_Goto::useEnd(void) I've been called\n"));
	AP_Dialog_FramePersistent::useEnd();

	// Let the view know it doens't need to maintain it's
	// "find" or "replace" session-oriented counters
	UT_ASSERT(m_pView);
	m_pView->findReset();
	
	// persistent dialogs don't destroy this data
	if (m_didSomething)
	{
		FREEP(_m_pageNumberString);
		if (m_pageNumberString)
			UT_UCS_cloneString(&_m_pageNumberString, m_pageNumberString);
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

	m_pFrame = (AP_Frame *) view->getParentData();
	UT_ASSERT(m_pFrame);
	
	m_pView = static_cast<FV_View *>(view);

	// must resize doc positions so we're within bounds
	// for the whole find
	m_pView->findReset();

	return UT_TRUE;
}

AV_View * AP_Dialog_Goto::getView(void) const
{
	return m_pView;
}

UT_Bool AP_Dialog_Goto::setPageNumberString(const UT_UCSChar * string)
{
	FREEP(m_pageNumberString);
	return UT_UCS_cloneString(&m_pageNumberString, string);
}

UT_UCSChar * AP_Dialog_Goto::getPageNumberString(void)
{
	UT_UCSChar * string = NULL;
	if (m_pageNumberString)
	{
		if (UT_UCS_cloneString(&string, m_pageNumberString))
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

UT_Bool AP_Dialog_Goto::gotoPage()
{
	UT_ASSERT(m_pView);

	UT_ASSERT(m_pageNumberString);

	// so we save our attributes to persistent storage
	m_didSomething = UT_TRUE;

	// TODO:  We need a Unicode atol/strtol.

	char * numberString = (char *) calloc(UT_UCS_strlen(m_pageNumberString) + 1, sizeof(char));
	UT_ASSERT(numberString);
	
	UT_UCS_strcpy_to_char(numberString, m_pageNumberString);
	
	UT_uint32 pageNumber = atol(numberString);
	FREEP(numberString);
	
	// call view to do the work
	return m_pView->gotoPage(pageNumber);
}
