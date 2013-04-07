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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */


/* This file implements RTF parsing routines */

#include "ut_debugmsg.h"

#include "ie_imp_RTF.h"
#include "ie_imp_RTFParse.h"
#include "ie_imp_RTFKeywords.h"


bool 
IE_Imp_RTFGroupParser::tokenError(IE_Imp_RTF * /*ie*/)
{
//	UT_ASSERT_NOT_REACHED();
	UT_DEBUGMSG(("tokenError() reached\n"));
	return true;
}


bool 
IE_Imp_RTFGroupParser::tokenKeyword(IE_Imp_RTF * /*ie*/, RTF_KEYWORD_ID /*kwID*/, 
									UT_sint32 /*param*/, bool /*paramUsed*/)
{
	return true;
}

bool 
IE_Imp_RTFGroupParser::tokenOpenBrace(IE_Imp_RTF * /*ie*/)
{
	m_nested++;
	return true;
}


bool 
IE_Imp_RTFGroupParser::tokenCloseBrace(IE_Imp_RTF * /*ie*/)
{
	m_nested--;
	return true;
}


bool 
IE_Imp_RTFGroupParser::tokenData(IE_Imp_RTF * /*ie*/, UT_UTF8String & /*data*/)
{
	return true;
}


bool 
IE_Imp_RTFGroupParser::finalizeParse(void)
{
	return true;
}


bool IE_Imp_RTF::keywordSorted = false;

static int kwsortcomparator(const void *v1, const void *v2)
{
	return strcmp(((const _rtf_keyword *)v1)->keyword, 
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
	xxx_UT_DEBUGMSG(("Push RTF state \n"));
	// Create a new object to store the state in
	RTFStateStore* pState = new RTFStateStore;
	if (pState == NULL)	{
	    UT_DEBUGMSG(("PushRTFState(): no state\n"));
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
	xxx_UT_DEBUGMSG(("Pop RTF state depth %d \n",m_stateStack.getDepth()));
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
		UT_DEBUGMSG(("RTF ERROR: pState is NULL! Will try to recover."));
		return false;
	}
}


/*!
  This is the standard keyword parser

  \param parser the IE_Imp_RTFGroupParser subclass that will do the 
  real syntaxic parsing.
*/
bool 
IE_Imp_RTF::StandardKeywordParser(IE_Imp_RTFGroupParser *parser)
{
	RTFTokenType tokenType;
	unsigned char keyword[MAX_KEYWORD_LEN];
	UT_sint32 parameter = 0;
	bool paramUsed = false;	
	RTF_KEYWORD_ID keywordID;
	bool finalBrace = false;

	do
	{
		tokenType = NextToken (keyword, &parameter, &paramUsed, 
							   MAX_KEYWORD_LEN,false);
		switch (tokenType)
		{
		case RTF_TOKEN_ERROR:
			return parser->tokenError(this);
			break;
		case RTF_TOKEN_KEYWORD:
		{
			xxx_UT_DEBUGMSG(("IE_Imp_RTF::StandardKeywordParser() %s\n", keyword));
			keywordID = KeywordToID(reinterpret_cast<char *>(keyword));
			parser->tokenKeyword(this, keywordID, parameter, paramUsed);
			break;
		}
		case RTF_TOKEN_OPEN_BRACE:
			UT_DEBUGMSG(("Nesting %d ++ <%p>\n", parser->nested(), parser));
			parser->tokenOpenBrace(this);
			break;
		case RTF_TOKEN_CLOSE_BRACE:
			parser->tokenCloseBrace(this);
			UT_DEBUGMSG(("Nesting %d -- <%p>\n", parser->nested(), parser));
			// oh oh we catched the last brace from the group
			if (parser->nested() == 0) {
				finalBrace = true;
			}
			break;
		case RTF_TOKEN_DATA:	//Ignore data
		{
			SkipBackChar(*keyword);
			UT_UTF8String data;
			HandlePCData(data);
			parser->tokenData(this, data);
			break;
		}
		default:
			break;
		}
	} while (!finalBrace);

	/* 
	   we must put it back into the flow because we are only supposed to 
	   handle the content of the group.
	 */
	SkipBackChar('}');
	return parser->finalizeParse();
}




/*!
  Comparator for keyword bsearch
*/
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



