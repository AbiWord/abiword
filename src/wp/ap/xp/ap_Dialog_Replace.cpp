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
	// GUI
    m_findString = NULL;
	m_replaceString = NULL;
	m_matchCase = UT_FALSE;

	// is this used?
	m_answer = a_VOID;
}

AP_Dialog_Replace::~AP_Dialog_Replace(void)
{
	UT_ASSERT(!m_bInUse);

	if (m_findString)
		FREEP(m_findString);
	if (m_replaceString)
		FREEP(m_replaceString);
}

void AP_Dialog_Replace::useStart(void)
{

	UT_DEBUGMSG(("AP_Dialog_Replace::useStart(void) I've been called\n"));

	AP_Dialog_FramePersistent::useStart();
	
/*
	FREEP(m_szInitialPathname);
	FREEP(m_szFinalPathname);
	m_answer = a_VOID;
	m_bSuggestName = UT_FALSE;
*/
}

void AP_Dialog_Replace::useEnd(void)
{

	UT_DEBUGMSG(("AP_Dialog_Replace::useEnd(void) I've been called\n"));
	AP_Dialog_FramePersistent::useEnd();

	// persistent dialogs don't destroy this data
/*
	FREEP(m_szInitialPathname);
	if (m_answer == a_OK)
	{
		FREEP(m_szPersistPathname);
		m_szPersistPathname = m_szFinalPathname;
		m_szFinalPathname = NULL;
	}
*/
}

AP_Dialog_Replace::tAnswer AP_Dialog_Replace::getAnswer(void) const
{
	// let our caller know if user hit ok, cancel, etc.
	return m_answer;
}

void AP_Dialog_Replace::setView(AV_View * view)
{
	// we can do a static cast from AV_View into FV_View,
	// so we can get WP specific information from it.
	// This could be bad once we introduce an
	// outline view, etc.
	UT_ASSERT(view);

	m_pView = static_cast<FV_View *>(view);
}

UT_Bool AP_Dialog_Replace::findNext(char * string)
{
	UT_ASSERT(string);
	UT_ASSERT(m_pView);

	// convert from char * to unicode
	UT_UCSChar * unicodeString = (UT_UCSChar *) calloc(strlen(string) + 1, sizeof(UT_UCSChar));

	UT_UCSChar * 	d = unicodeString;
	char * 			s = string;
	UT_uint32 		i = 0;
	while (i < strlen(string) && *s != NULL)
		*d++ = *s++;
	*++d = NULL;

	// call view to do the work
	UT_Bool result = m_pView->findNext(unicodeString, UT_TRUE);

	if (unicodeString)
		free(unicodeString);

	return result;
}

UT_Bool AP_Dialog_Replace::findNextAndReplace(char * find, char * replace)
{
	// TODO
	return UT_TRUE;
}
