/* AbiSource Application Framework
 * Copyright (C) 1998,1999 AbiSource, Inc.
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
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "xad_Document.h"
#include "ut_string.h"
#include "ut_alphahash.h"

AD_Document::AD_Document()
{
	m_iRefCount = 1;
	m_szFilename = NULL;

	// TODO do we need to auto-increase the bucket count,
   	// TODO if the ignore list gets long?
	m_pIgnoreList = new UT_AlphaHashTable(11);
}

AD_Document::~AD_Document()
{
	UT_ASSERT(m_iRefCount == 0);

   	// free all of the words on the list first
	for (int i = 0; i < m_pIgnoreList->getEntryCount(); i++) {
	   	UT_HashEntry * pHE = m_pIgnoreList->getNthEntry(i);
	   	FREEP(pHE->pData);
	}
   	// the free the ignore list
	DELETEP(m_pIgnoreList);

   	// NOTE: let subclass clean up m_szFilename, so it matches the alloc mechanism
}

void AD_Document::ref(void)
{
	UT_ASSERT(m_iRefCount > 0);

	m_iRefCount++;
}

void AD_Document::unref(void)
{
	UT_ASSERT(m_iRefCount > 0);

	if (--m_iRefCount == 0)
	{
		delete this;
	}
}

const char * AD_Document::getFilename(void) const
{
	return m_szFilename;
}


// Methods for maintaining document-wide "Ignore All" list

bool AD_Document::appendIgnore(const UT_UCSChar * pWord, UT_uint32 len)
{
	UT_ASSERT(m_pIgnoreList);

	char * key = (char *) calloc(len+1, sizeof(char));
	UT_UCSChar * copy = (UT_UCSChar *) calloc(len+1, sizeof(UT_UCSChar));

	if (!key || !copy)
	{
		UT_DEBUGMSG(("mem failure adding word to dictionary\n"));
		FREEP(key);
		FREEP(copy);
		return false;
	}

	for (UT_uint32 i = 0; i < len; i++)
	{
		UT_UCSChar currentChar;
		currentChar = pWord[i];
		// convert smart quote apostrophe to ASCII single quote
		if (currentChar == UCS_RQUOTE) currentChar = '\'';
		key[i] = (char) currentChar;
		copy[i] = currentChar;
	}

	UT_sint32 iRes = m_pIgnoreList->addEntry(key, NULL, (void*) copy);

	FREEP(key);

	if (iRes == 0)
		return true;
	else
		return false;
}

bool AD_Document::isIgnore(const UT_UCSChar * pWord, UT_uint32 len) const
{
	UT_ASSERT(m_pIgnoreList);

	char * key = (char*) calloc(len+1, sizeof(char));
	if (!key)
	{
		UT_DEBUGMSG(("mem failure looking up word in ignore all list\n"));
		FREEP(key);
		return false;
	}

	for (UT_uint32 i = 0; i < len; i++)
	{
		key[i] = (char) pWord[i];
	}

	UT_HashEntry * pHE = m_pIgnoreList->findEntry(key);

	FREEP(key);

	if (pHE != NULL)
		return true;
	else 
		return false;
   
}

  
bool AD_Document::enumIgnores(UT_uint32 k, const UT_UCSChar * pszWord) const
{
         UT_ASSERT(m_pIgnoreList);
  
	 if ((int)k >= m_pIgnoreList->getEntryCount())
         {
	          pszWord = NULL;
		  return false;
	 }
   
	 UT_HashEntry * pHE = m_pIgnoreList->getNthEntry(k);
     
	 UT_ASSERT(pHE);
     
	 pszWord = (UT_UCSChar*) pHE->pData;

	 return true;
}
  
bool AD_Document::clearIgnores(void)
{
          UT_ASSERT(m_pIgnoreList);
     
	  for (int i = 0; i < m_pIgnoreList->getEntryCount(); i++) 
	  {
	           UT_HashEntry * pHE = m_pIgnoreList->getNthEntry(i);
		   FREEP(pHE->pData);
	  }
     
	  DELETEP(m_pIgnoreList);
	  
	  m_pIgnoreList = new UT_AlphaHashTable(11);
   
	  UT_ASSERT(m_pIgnoreList);
     
	  return true;
}


