/* -*- c-basic-offset: 4; tab-width: 4; indent-tabs-mode: t -*- */
/* AbiWord
 * Copyright (C) 2004 Hubert Figuiere
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


/* This file implements RTF parsing routines */

#include "ut_debugmsg.h"

#include "ie_imp_RTF.h"
#include "ie_imp_RTFParse.h"
#include "ie_imp_RTFKeywords.h"

bool IE_Imp_RTF::keywordSorted = false;

static int kwsortcomparator(const void *v1, const void *v2)
{
	return strcmp (((const _rtf_keyword *)v1)->keyword, 
				   ((const _rtf_keyword *)v2)->keyword);
}


void IE_Imp_RTF::_initialKeywordSort(void)
{
	UT_DEBUGMSG(("RTF: initial sorting of keywords..."));
	qsort (const_cast<_rtf_keyword*>(rtfKeywords), 
		   sizeof (rtfKeywords) / sizeof(_rtf_keyword) , 
		   sizeof(_rtf_keyword), &kwsortcomparator);
	IE_Imp_RTF::keywordSorted = true;
	UT_DEBUGMSG(("done.\n"));
}


// Pushes the current state RTF state onto the state stack
//
bool IE_Imp_RTF::PushRTFState(void)
{
	// Create a new object to store the state in
	RTFStateStore* pState = new RTFStateStore;
	if (pState == NULL)	{
	    UT_DEBUGMSG (("PushRTFState(): no state\n"));
	    return false;
	}
	*pState = m_currentRTFState;
	m_stateStack.push(pState);

	// Reset the current state
	m_currentRTFState.m_internalState = RTFStateStore::risNorm;
	return true;
}



// Pops the state off the top of the RTF stack and set the current state to it.
//
bool IE_Imp_RTF::PopRTFState(void)
{
	RTFStateStore* pState = NULL;
	m_stateStack.pop(reinterpret_cast<void**>(&pState));

	if (pState != NULL)	{
		bool ok = FlushStoredChars();
		m_currentRTFState = *pState;
		delete pState;

		m_currentRTFState.m_unicodeInAlternate = 0;
		return ok;
	}
	else {
		UT_return_val_if_fail(pState != NULL, false);	// state stack should not be empty
		return true; // was false
	}
}

static int kwcompar(const void * v1, const void* v2)
{
	const char *kw = (const char *)v1;
	const _rtf_keyword *kwelem = (const _rtf_keyword *)v2;
	return strcmp(kw, kwelem->keyword);
}

/*!
  Return the keyword ID for the given keyword

  
*/
RTF_KEYWORD_ID IE_Imp_RTF::KeywordToID(const char * keyword)
{
	const _rtf_keyword *kwelem  = (_rtf_keyword *)bsearch (keyword, 
														   rtfKeywords, 
														   sizeof(rtfKeywords) / sizeof(rtfKeywords[0]),
														   sizeof(rtfKeywords[0]), &kwcompar);
	if (kwelem) {
		return kwelem->id;
	}
	return RTF_UNKNOWN_KEYWORD;
}
