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
	for (UT_uint32 i = 0; i < (unsigned)m_pIgnoreList->getEntryCount(); i++)
	{
		// not careful results checking, etc, in this loop, but it's just for debugging anyhow
		const UT_UCSChar *word = 0;
		char copy[1000];
		enumIgnores(i, &word);
		UT_DEBUGMSG(("AD_Document::~AD_Document(), ignored spell word \"%s\"\n", UT_UCS_strcpy_to_char(copy, word)));
	}

   	// free all of the words on the list first
	clearIgnores();
   	// then free the ignore list
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

UT_Bool AD_Document::appendIgnore(const UT_UCSChar * pWord, UT_uint32 len)
{
	UT_ASSERT(m_pIgnoreList);

	char _key[150], *key = _key;
	if (len > 145) key = new char[len + 1];
	UT_UCSChar *copy = new UT_UCSChar[len + 1];

	for (UT_uint32 i = 0; i < len; i++)
	{
		UT_UCSChar currentChar;
		currentChar = pWord[i];
		// convert smart quote apostrophe to ASCII single quote
		if (currentChar == UCS_RQUOTE) currentChar = '\'';
		key[i] = (char) currentChar;
		copy[i] = currentChar;
	}
	key[len] = 0;
	copy[len] = 0;

	UT_sint32 iRes = -1;

	if (!isIgnore(pWord, len))
	{
		// If it's already on the ignored word list, don't add it again.
		// This can happen if you are looking at a longish document.  You
		// "ignore all" a word, but spell-check doesn't get around to removing
		// the squiggles in the background for a while.  Then, you "ignore all"
		// that word (or another instance of it) again, and ka-bloom, the 
		// hash table stuff asserts on a duplicate entry.
		iRes = m_pIgnoreList->addEntry(key, NULL, (void*) copy);
	}

	if (key != _key) DELETEP(key);

	if (iRes == 0)
		return UT_TRUE;
	else
		return UT_FALSE;
}

UT_Bool AD_Document::isIgnore(const UT_UCSChar * pWord, UT_uint32 len) const
{
	UT_ASSERT(m_pIgnoreList);

	char _key[150], *key = _key;
	if (len > 145) key = new char[len + 1];

	for (UT_uint32 i = 0; i < len; i++)
	{
		UT_UCSChar currentChar;
		currentChar = pWord[i];
		// convert smart quote apostrophe to ASCII single quote
		if (currentChar == UCS_RQUOTE) currentChar = '\'';
		key[i] = (char) currentChar;
	}
	key[len] = 0;

	UT_HashEntry * pHE = m_pIgnoreList->findEntry(key);

	if (key != _key) DELETEP(key);

	if (pHE != NULL)
		return UT_TRUE;
	else 
		return UT_FALSE;
   
}

UT_Bool AD_Document::enumIgnores(UT_uint32 k, const UT_UCSChar **pszWord) const
{
	UT_ASSERT(m_pIgnoreList);

	if ((int)k >= m_pIgnoreList->getEntryCount())
	{
		*pszWord = NULL;
		return UT_FALSE;
	}
   
	UT_HashEntry * pHE = m_pIgnoreList->getNthEntry(k);
	
	UT_ASSERT(pHE);
   
	*pszWord = (UT_UCSChar*) pHE->pData;
	return UT_TRUE;
}

UT_Bool AD_Document::clearIgnores(void)
{
	UT_ASSERT(m_pIgnoreList);
	
	for (int i = 0; i < m_pIgnoreList->getEntryCount(); i++)
	{
		UT_HashEntry * pHE = m_pIgnoreList->getNthEntry(i);
		DELETEP((UT_UCSChar *)pHE->pData);
	}
   
	DELETEP(m_pIgnoreList);
   
	m_pIgnoreList = new UT_AlphaHashTable(11);
   
	UT_ASSERT(m_pIgnoreList);
   
	return UT_TRUE;
}
