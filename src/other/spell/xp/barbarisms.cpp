/* AbiSuite
 * Copyright (C) Jordi Mas i Hernàndez
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

#include "barbarisms.h"
#include "ut_debugmsg.h"
#include "ut_hash.h"
#include "ut_string.h"
#include "ut_stringbuf.h"
#include "ut_string_class.h"
#include <string.h>

Barbarisms::Barbarisms()
{
	m_pCurVector = NULL;	
}

Barbarisms::~Barbarisms()
{
	UT_HASH_PURGEDATA(UT_Vector*, &m_map, delete);
}


/* 
	Takes a full dictionary filename (.hash) and builds the barbarism full 
	filename. The barbarism file is expected to be in the same directory, using 
	the same name, and just with a different file extension		
*/		
bool Barbarisms::load(const char *szHash)
{			
	char 	szHashExt[] = ".hash";
	UT_XML 	parser;	
	char	szBarFile[MAX_PATHNM];	
		
	if (strlen(szHash)<strlen(szHashExt))	return false;		

	// Takes dictionary file names and builds the barbarism file
	memset(szBarFile, 0, MAX_PATHNM);
	strncpy (szBarFile, szHash, strlen(szHash)-strlen(szHashExt));
	strcat (szBarFile, "-barbarism.xml");	
				
	parser.setListener (this);	
	
	if ((parser.parse (szBarFile) != UT_OK))
		return false; // cannot parse the file

	UT_DEBUGMSG(( "Barbarisms::load->Read %u elements into the map\n", m_map.size()));				
	
	return true;
}



/*
	Looks for a exact case maching of the suggestion
*/
bool	Barbarisms::suggestExactWord(const UT_UCSChar *word32, size_t length,	UT_Vector* pVecsugg)
{		
	const char* pUTF8;
	const UT_UCS4Char *pWord;	
	UT_UTF8String stUTF8;
	UT_UCS4Char *suggest32;
	int nSize;
			 
	stUTF8.appendUCS4(word32, length);
	
	pUTF8 =  stUTF8.utf8_str();

	UT_Vector* vec  =(UT_Vector*) m_map.pick(pUTF8);	
	if (!vec) return false;
	
	const UT_uint32 nItems = vec->getItemCount();	
	
	if (!nItems) return false;
	
	for (UT_uint32 iItem = nItems; iItem; --iItem)
	{
		pWord = (const UT_UCS4Char * ) vec->getNthItem(iItem - 1);					
		nSize = sizeof(UT_UCS4Char) * (UT_UCS4_strlen(pWord) + 1);
		suggest32 = (UT_UCS4Char*) malloc(nSize);		
		memcpy (suggest32, pWord, nSize);
		pVecsugg->addItem((void *)suggest32);					
	}			

	return true;
}

/*
	Suggest a word with all possible case combinations
	
	- If it's lower case, we just look for the lower case word
	- If it has the first letter upper case, we look for an exact macht and for lower case 
	- If it's upper case, we look for the exact mach and lower case // not implemented yet
				
*/
bool	Barbarisms::suggestWord(const UT_UCSChar *word32, size_t length,	UT_Vector* pVecsugg)
{			
	bool bIsLower = true;
	bool bIsUpperLower = false;	
	size_t len;
	UT_UCSChar* pStr;

	if (!length) return false;	
	
	/* The vector should be empty because we want our suggestions first */
	UT_ASSERT (pVecsugg->getItemCount()==0);
	
	/*	
		If the word is lower case we just look the lower case 		
	*/	
	len=length;	
	pStr = (UT_UCSChar *) word32;
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
		UT_UCSChar* pStr = (UT_UCSChar *)word32;
		pStr++;
		len=length;	
		if (len) len--;
		/* After the first character, the rest should be lower case*/	
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
		UT_UCS4Char*	wordsearch;
		
		UT_UCS4_cloneString(&wordsearch, word32);
		
		/* Convert word into lowercase (only need the first char) */
		wordsearch[0] = UT_UCS4_tolower(wordsearch[0]);		
				
		if (suggestExactWord(wordsearch,  length, pVecsugg))
		{
			const UT_uint32 nItems = pVecsugg->getItemCount();				
			UT_UCSChar*	pSug;
			
			/*	Make the first letter of all the results uppercase	*/
			for (UT_uint32 iItem = nItems; iItem; --iItem)
			{
				pSug =  (UT_UCSChar *)pVecsugg->getNthItem(iItem - 1);						
				*pSug = UT_UCS4_toupper(*pSug);						
			}				
		}
		
		if (wordsearch) free(wordsearch);
	}
	
	
			
	return 0;
}

/*
	Called by the parser. We build the barbarism list here
	
	Barbarism (the index of the map) is stored in UTF8 because the map index 
	and the suggestions in UT_UCSChard
	
*/
void	Barbarisms::startElement(const XML_Char *name, const XML_Char **atts)
{	
	const XML_Char ** a = atts;
	
	if (strcmp(name, "barbarism")==0)
	{				
		m_pCurVector =  new UT_Vector();	
		m_map.insert (strdup(a[1]), m_pCurVector);		

		UT_DEBUGMSG(( "Barbarisms::startElement->Barbarism->%s \n", a[1]));
	}
	else
	{
		if (strcmp(name, "suggestion")==0)
		{	
			if (m_pCurVector)
			{						
				UT_UCS4Char	ch4;
				const char*	pUTF8 = a[1];				
				size_t	length = strlen(a[1]);
				int		nUSC4Len = 0;
				UT_UCS4String	usc4;
				
				while (true)
				{
					ch4 = UT_UCS4Stringbuf::UTF8_to_UCS4 (pUTF8, length);
					if (ch4 == 0) break;
					nUSC4Len++;
					usc4+=ch4;
				}			
				
				const UT_UCS4Char* pData =  usc4.ucs4_str();
								
				UT_UCS4Char *word16 = new UT_UCS4Char[nUSC4Len+1];
				memcpy (word16, pData, (nUSC4Len+1)*sizeof(UT_UCS4Char));
				m_pCurVector->addItem(word16);				

				UT_DEBUGMSG(( "Barbarisms::startElement->Suggestion->%s\n", a[1]));
			}
		}
	}
}

