/* AbiSuite
 * Copyright (C) Jordi Mas i Hern�ndez
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

#include <string.h>
#include "ut_misc.h"
#include "barbarisms.h"
#include "ut_debugmsg.h"
#include "ut_hash.h"
#include "ut_string.h"
#include "ut_stringbuf.h"
#include "ut_string_class.h"
#include "xap_App.h"

BarbarismChecker::BarbarismChecker()
{
	m_pCurVector = NULL;
}

BarbarismChecker::~BarbarismChecker()
{	  
	UT_StringPtrMap::UT_Cursor _hc1(&m_map);		

    for (UT_Vector* pVec = (UT_Vector*) _hc1.first(); _hc1.is_valid(); pVec = (UT_Vector*) _hc1.next() ) 
	{ 
		if (pVec)									
		{
			for (UT_uint32 i=0; i < pVec->getItemCount(); i++)
				delete (char *)pVec->getNthItem(i);
				
			delete pVec;			
		}
	} 	
}


/*
	Takes a language code and builds the barbarism full filename.
*/
bool BarbarismChecker::load(const char *szLang)
{
	UT_String	fullPath;
	UT_XML 		parser;

	if (!szLang || !*szLang)
		return false;

	m_sLang = szLang;

	fullPath = XAP_App::getApp()->getAbiSuiteLibDir();
#if defined(WIN32)
	fullPath += "\\dictionary\\";
#else
	fullPath += "/dictionary/";
#endif
	fullPath += szLang;
	fullPath += "-barbarism.xml";

	parser.setListener (this);

	if ((parser.parse (fullPath.c_str())) != UT_OK)
		return false; // cannot parse the file

	UT_DEBUGMSG(("SPELL: barbar %s loaded %u\n", szLang, m_map.size()));

	return true;
}

/**
 * True if word is a barbarsim, false if not
 */
bool BarbarismChecker::checkWord(const UT_UCSChar * word32, size_t length)
{
	UT_UTF8String stUTF8;
	stUTF8.appendUCS4(word32, length);

	// TODO: capitalization issues

	bool bResult = (m_map.pick(stUTF8.utf8_str()) != NULL);

	return bResult;
}

/*
	Looks for an exact case match of the suggestion
	Returns true if word is a barbarism
*/
bool BarbarismChecker::suggestExactWord(const UT_UCSChar *word32, size_t length, UT_Vector* pVecsugg)
{
	const char* pUTF8;
	const UT_UCS4Char *pWord;
	UT_UTF8String stUTF8;
	UT_UCS4Char *suggest32;
	int nSize;

	stUTF8.appendUCS4(word32, length);

	pUTF8 = stUTF8.utf8_str();

	UT_Vector* vec = static_cast<UT_Vector*>(const_cast<void *>(m_map.pick(pUTF8)));
	if (!vec)
		return false;

	const UT_uint32 nItems = vec->getItemCount();

	if (!nItems)
		return false;

	for (UT_uint32 iItem = nItems; iItem; --iItem)
	{
		pWord = static_cast<UT_UCS4Char *>(vec->getNthItem(iItem - 1));
		nSize = sizeof(UT_UCS4Char) * (UT_UCS4_strlen(pWord) + 1);
		suggest32 = static_cast<UT_UCS4Char*>(malloc(nSize));
		memcpy (suggest32, pWord, nSize);
		
		pVecsugg->insertItemAt(static_cast<void *>(suggest32), 0);
	}

	return true;
}

/*
	Suggest a word with all possible case combinations

	- If it's lower case, we just look for the lower case word
	- If it has the first letter upper case, we look for an exact match and for lower case
	- If it's upper case, we look for the exact match and lower case // not implemented yet

	Returns true if word is a barbarism
*/
bool BarbarismChecker::suggestWord(const UT_UCSChar *word32, size_t length, UT_Vector* pVecsugg)
{
	bool bIsBarbarism = false;
	bool bIsLower = true;
	bool bIsUpperLower = false;
	size_t len;
	UT_UCSChar* pStr;

	if (!length)
		return false;

	/*
		If the word is lower case we just look at the lower case
	*/
	len=length;
	pStr = const_cast<UT_UCSChar *>(word32);
	for (; len; pStr++, len--)
	{
		if (!UT_UCS4_islower(*pStr))
		{
			bIsLower=false;
			break;
		}
	}
	if (bIsLower)
		return suggestExactWord(word32, length, pVecsugg);

	/*
		If the word has the first char upper case and the rest lower case
	*/
	if (UT_UCS4_isupper(*word32))
	{
		UT_UCSChar* pStr = const_cast<UT_UCSChar *>(word32);
		pStr++;
		len=length;
		if (len)
			len--;
		/* After the first character, the rest should be lower case */
		for (;len;pStr++, len--)
		{
			if (!UT_UCS4_islower(*pStr))
				break;
		}
		if (!len)
			bIsUpperLower = true;
	}

	if (bIsUpperLower)
	{
		UT_UCS4Char* wordsearch;

		UT_UCS4_cloneString(&wordsearch, word32);

		/* Convert word into lowercase (only need the first char) */
		wordsearch[0] = UT_UCS4_tolower(wordsearch[0]);

		if ((bIsBarbarism = suggestExactWord(wordsearch,  length, pVecsugg)))
		{
			const UT_uint32 nItems = pVecsugg->getItemCount();
			UT_UCSChar* pSug;

			/* Make the first letter of all the results uppercase */
			for (UT_uint32 iItem = nItems; iItem; --iItem)
			{
				pSug = static_cast<UT_UCSChar *>(pVecsugg->getNthItem(iItem - 1));
				*pSug = UT_UCS4_toupper(*pSug);
			}
		}

		if (wordsearch)
			free(wordsearch);
	}

	return bIsBarbarism;
}

/*
	Called by the parser. We build the barbarism list here

	Barbarism (the index of the map) is stored in UTF-8 because the map index
	and the suggestions are in UT_UCSChar

*/
void BarbarismChecker::startElement(const XML_Char *name, const XML_Char **atts)
{
	// TODO: find out the correct arguments for UT_getAttribute...

	if (strcmp(name, "barbarism")==0)
	{
		
		const char * word = UT_getAttribute ("word", atts);
		if (word != NULL)
		{
			m_pCurVector = new UT_Vector();
			m_map.insert (word, m_pCurVector);
		}
		else
			m_pCurVector = NULL;
			
	}
	else if (strcmp(name, "suggestion")==0)
	{
		if (m_pCurVector)
		{
			const char* pUTF8 = UT_getAttribute ("word", atts);
			if (pUTF8 == NULL)
				return;

			size_t			length = strlen (pUTF8);
			int				nUSC4Len = 0;
			UT_UCS4String	usc4;

			while (true)
			{
				UT_UCS4Char ch4 = UT_UCS4Stringbuf::UTF8_to_UCS4 (pUTF8, length);
				if (ch4 == 0)
					break;
				nUSC4Len++;
				usc4+=ch4;
			}

			const UT_UCS4Char* pData = usc4.ucs4_str();

			UT_UCS4Char *word32 = new UT_UCS4Char[nUSC4Len+1];
			memcpy (word32, pData, (nUSC4Len+1)*sizeof(UT_UCS4Char));

			// insert suggestions at beginning
			// this preserves the order in the xml file
			// and puts them ahead of the regular suggestions
			m_pCurVector->insertItemAt(word32, 0);
		}
	}
}

