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
#include "ap_Dialog_Replace.h"

#include "fl_DocLayout.h"
#include "fv_View.h"

#define FREEP(p)	do { if (p) free(p); (p)=NULL; } while (0)
#define DELETEP(p)	do { if (p) delete(p); (p)=NULL; } while (0)

AP_Dialog_Replace::AP_Dialog_Replace(AP_DialogFactory * pDlgFactory, AP_Dialog_Id id)
	: AP_Dialog_FramePersistent(pDlgFactory,id)
{
	_m_findString = NULL;
	_m_replaceString = NULL;
	_m_matchCase = UT_TRUE;

	m_pView = NULL;
	
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
}

void AP_Dialog_Replace::useStart(void)
{
	UT_DEBUGMSG(("AP_Dialog_Replace::useStart(void) I've been called\n"));

	AP_Dialog_FramePersistent::useStart();

	// restore from persistent storage
	if (_m_findString)
		UT_UCS_cloneString(&m_findString, _m_findString);
	if (_m_replaceString)
		UT_UCS_cloneString(&m_replaceString, _m_replaceString);

	m_matchCase = _m_matchCase;
}

void AP_Dialog_Replace::useEnd(void)
{

	UT_DEBUGMSG(("AP_Dialog_Replace::useEnd(void) I've been called\n"));
	AP_Dialog_FramePersistent::useEnd();

	// persistent dialogs don't destroy this data
	if (m_didSomething)
	{
		FREEP(_m_findString);
		if (m_findString)
			UT_UCS_cloneString(&_m_findString, m_findString);
		
		FREEP(_m_replaceString);
		if (m_replaceString)
			UT_UCS_cloneString(&_m_replaceString, m_replaceString);

		_m_matchCase = m_matchCase;
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

	m_pView = static_cast<FV_View *>(view);
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
	UT_ASSERT(m_replaceString);
	
	// so we save our attributes to persistent storage
	m_didSomething = UT_TRUE;
	
	// call view to do the work
	return m_pView->findNext(m_findString, UT_TRUE);
}

UT_Bool AP_Dialog_Replace::findReplace()
{
	UT_ASSERT(m_pView);

	UT_ASSERT(m_findString);
	UT_ASSERT(m_replaceString);
	
	// so we save our attributes to persistent storage
	m_didSomething = UT_TRUE;
	
	// call view to do the work
	return m_pView->findReplace(m_findString, m_replaceString);
}

UT_Bool AP_Dialog_Replace::findReplaceAll()
{
	UT_ASSERT(m_pView);

	UT_ASSERT(m_findString);
	UT_ASSERT(m_replaceString);
	
	// so we save our attributes to persistent storage
	m_didSomething = UT_TRUE;

	
	// call view to do the work
	return m_pView->findReplace(m_findString, m_replaceString);
}
