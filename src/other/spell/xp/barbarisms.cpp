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
#include "ispell.h"
#include <string.h>
#include "ut_hash.h"
#include "ut_string.h"
#include "ut_stringbuf.h"
#include "ut_string_class.h"


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
bool	Barbarisms::suggestExactWord(const UT_UCSChar *word32, size_t length,	UT_Vector* pVecsugg, ispell_state_t* state)
{	
	size_t len_in, len_out;
	const UT_UCS4Char *pWord;	
	UT_UTF8String stUTF8;
	UT_UCS4Char *suggest32;
	int nSize;
	 
	stUTF8.appendUCS4(word32, length);
	
	const char* pUTF8 =  stUTF8.utf8_str();

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
bool	Barbarisms::suggestWord(const UT_UCSChar *word32, size_t length,	UT_Vector* pVecsugg, ispell_state_t* state)
{		
	
	ichar_t  word16[INPUTWORDLEN + MAXAFFIXLEN];
	char  word8[INPUTWORDLEN + MAXAFFIXLEN];
	ichar_t* pStr16 = word16;	
	bool bIsLower = true;
	bool bIsUpperLower = false;	
	
	/* word 32 to word 8*/
	size_t len_in, len_out;
	const char *In = (const char *)word32;
	char *Out = word8;
	len_in = length * sizeof(UT_UCSChar);
	len_out = sizeof( word8 ) - 1;
	UT_iconv(state->translate_in, &In, &len_in, &Out, &len_out);
	*Out = '\0';
	
	/* The vector should be empty because we want our suggestions first */
	UT_ASSERT (pVecsugg->getItemCount()==0);
	
	/*	
		If the word is lower case we just look the lower case 		
	*/		
	strtoichar(state, word16, word8, sizeof(word16), 0);
	
	for (;*pStr16;pStr16++)
	{
		if (!mylower(state, *pStr16))
		{
			bIsLower=false;
			break;
		}		
	}	
	if (bIsLower)
		return suggestExactWord(word32,  length, pVecsugg, state);
	
	/*	
		If the word has the first char upper case and the rest lower case		
	*/				
	if (myupper(state, *word16))
	{
		pStr16 = word16;
		pStr16++;
		/* After the first character, the rest should be lower case*/	
		for (;*pStr16;pStr16++)
		{
			if (!mylower(state, *pStr16))
				break;		
		}				
		if (!*pStr16)
			bIsUpperLower = true;
	}
	
	if (bIsUpperLower)
	{
		ichar_t 	word16lwr[INPUTWORDLEN + MAXAFFIXLEN];
		char		word8lwr[INPUTWORDLEN + MAXAFFIXLEN];
		UT_UCS4Char	word32[INPUTWORDLEN + MAXAFFIXLEN];
		
		/* Convert word into lowercase (only need the first char) */
		strtoichar(state, word16lwr, word8, sizeof(word16lwr), 0);
		word16lwr[0] = mytolower(state, word16lwr[0]);
		ichartostr (state, word8lwr, word16lwr, INPUTWORDLEN + MAXAFFIXLEN, INPUTWORDLEN + MAXAFFIXLEN);
		
		/* Convert to word8 to word 32*/
		size_t len_in, len_out;
		const char *In = word8lwr;
		char *Out = (char *)word32;

		len_in = strlen(word8lwr);
		len_out = sizeof(UT_UCS4Char) * (len_in+1);
		UT_iconv(state->translate_out, &In, &len_in, &Out, &len_out);
		*((UT_UCS4Char *)Out) = 0;
		
		if (suggestExactWord(word32,  length, pVecsugg, state))
		{
			const UT_uint32 nItems = pVecsugg->getItemCount();				
			char*	pSug;
			
			/*	Make the first letter of all the results uppercase	*/
			for (UT_uint32 iItem = nItems; iItem; --iItem)
			{
				pSug = (char *) pVecsugg->getNthItem(iItem - 1);		
				strtoichar(state, word16lwr, pSug, sizeof(word16lwr), 0);
				word16lwr[0] = mytoupper(state, word16lwr[0]);
				ichartostr (state, pSug, word16lwr, INPUTWORDLEN + MAXAFFIXLEN, INPUTWORDLEN + MAXAFFIXLEN);
			}				
		}
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
				UT_UCS2Char	ch2;
				const char*	pUTF8 = a[1];				
				size_t	length = strlen(a[1]);
				int		nUSC2Len = 0;
				UT_UCS2String	usc2;
				
				while (true)
				{
					ch2 = UT_UCS2Stringbuf::UTF8_to_UCS2 (pUTF8, length);
					if (ch2 == 0) break;
					nUSC2Len++;
					usc2+=ch2;
				}			
				
				const UT_UCS4Char* pData =  usc2.ucs4_str();
								
				UT_UCS4Char *word16 = new UT_UCS4Char[nUSC2Len+1];
				memcpy (word16, pData, (nUSC2Len+1)*sizeof(UT_UCS4Char));
				m_pCurVector->addItem(word16);				

				UT_DEBUGMSG(( "Barbarisms::startElement->Suggestion->%s\n", a[1]));
			}
		}
	}
}

