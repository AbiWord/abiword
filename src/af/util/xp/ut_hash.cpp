/* AbiSource Program Utilities
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
#include <string.h>

#include "ut_hash.h"
#include "ut_string.h"
#include "ut_assert.h"

#define CHUNK_NUM_ENTRIES		8

struct ut_HashEntryListNode
{
	int						iEntry;
	ut_HashEntryListNode*	pNext;
};

struct ut_HashBucket
{
	ut_HashEntryListNode* pHead;
};

/*****************************************************************
** WARNING: if we ever put in code to properly delete
** WARNING: things from the m_pEntries array, we will
** WARNING: need to update ut_alphahash.cpp.
*****************************************************************/

UT_HashTable::UT_HashTable(int iBuckets) 
	: m_iBuckets (iBuckets), m_pBuckets (0), 
	m_pEntries (0), m_iEntrySpace (0), m_iEntryCount (0),
	m_pool ()
{
	UT_ASSERT(iBuckets >= 3);		// NB: must be prime
}

UT_HashTable::~UT_HashTable()
{
	// in case this never got used
	if (!m_pBuckets)
		return;

	for (int i=0; i < m_iBuckets; i++)
	{
		ut_HashEntryListNode* pHELN = m_pBuckets[i].pHead;

		while (pHELN)
		{
			ut_HashEntryListNode* pTmp = pHELN->pNext;
			delete pHELN;
			pHELN = pTmp;
		}
	}
	
	free(m_pBuckets);
	free(m_pEntries);
}

UT_sint32 UT_HashTable::addEntry(const char* pszLeft, 
								 const char* pszRight, void* pData)
{
	UT_ASSERT(pszLeft);

	// TODO check to see if the entry is already there.
	// Done! Sevior 16/5/2001
	UT_HashEntry * pEntry = findEntry(pszLeft);
	if(pEntry != NULL)
	{
		pEntry->pszRight = m_pool.addString(pszRight);
		pEntry->pData = pData;
		return 0;
	}

	if (0 != verifySpaceToAddOneEntry())
	{
		return -1;
	}

	// TODO the following are essentially memory allocations which can fail.  check them
	m_pEntries[m_iEntryCount].pszLeft = m_pool.addString(pszLeft);

	if (pszRight)
		m_pEntries[m_iEntryCount].pszRight = m_pool.addString(pszRight);

	m_pEntries[m_iEntryCount].pData = pData;

	int iBucket = hashFunc(pszLeft);

	ut_HashEntryListNode* pHELN = new ut_HashEntryListNode();
	pHELN->iEntry = m_iEntryCount;
	pHELN->pNext = m_pBuckets[iBucket].pHead;
	m_pBuckets[iBucket].pHead = pHELN;
	m_iEntryCount++;

	return 0;
}

UT_sint32 UT_HashTable::setEntry(UT_HashEntry* pEntry, 
								 const char* pszRight, void* pData)
{
	if (pszRight)
		pEntry->pszRight = m_pool.addString(pszRight);	// TODO this can fail, right?

	pEntry->pData = pData;

	return 0;
}

UT_HashEntry* UT_HashTable::findEntry(const char* pszLeft) const
{
	if (!m_pBuckets)
	{
		return NULL;
	}

	if (!pszLeft)
	{
		return NULL;
	}

	UT_ASSERT(m_pBuckets);
	int iBucket = hashFunc(pszLeft);
	ut_HashEntryListNode* pHELN = m_pBuckets[iBucket].pHead;
	while (pHELN)
	{
		UT_HashEntry* pEntry = m_pEntries + pHELN->iEntry;

		if (!pEntry->pszLeft)
			continue;

		if (0 == strcmp(pEntry->pszLeft, pszLeft))
		{
			return pEntry;
		}

		pHELN = pHELN->pNext;
	}

	return NULL;
}

void UT_HashTable::removeEntry (const char * pszLeft)
{
	if (!m_pBuckets)
	{
		return;
	}

	UT_HashEntry * pEntry = findEntry (pszLeft);
	if (!pEntry)
	{
		return;
	}

	// the ht is in *dire* need of a rewrite!!!!!!!!!!!
	// this method is certainly not correct

	setEntry (pEntry, NULL, NULL);
	pEntry->pszLeft = 0;

	return;
}

UT_HashEntry* UT_HashTable::getNthEntry(int n) const
{
	UT_ASSERT((n>=0) && (n<m_iEntryCount));

	return m_pEntries + n;
}

int UT_HashTable::firstAlloc()
{
	UT_ASSERT(!m_pEntries);

	m_pBuckets = (ut_HashBucket*) calloc(m_iBuckets, sizeof(ut_HashBucket));
	if (!m_pBuckets)
	{
		return -1;
	}

	m_iEntryCount = 0;
	m_iEntrySpace = CHUNK_NUM_ENTRIES;
	m_pEntries = (UT_HashEntry*) calloc(m_iEntrySpace, sizeof(UT_HashEntry));
	if (!m_pEntries)
	{
		free(m_pBuckets);
		m_pBuckets = NULL;

		m_iEntrySpace = 0;

		return -1;
	}

	return 0;
}

int UT_HashTable::grow()
{
	int iNewSpace = calcNewSpace();
	UT_HashEntry *pNewEntries = (UT_HashEntry *) calloc(iNewSpace, 
														sizeof(UT_HashEntry));
	if (!pNewEntries)
	{
		return -1;
	}

	memcpy(pNewEntries, m_pEntries, m_iEntryCount*sizeof(UT_HashEntry));
	free(m_pEntries);
	m_iEntrySpace = iNewSpace;
	m_pEntries = pNewEntries;

	return 0;
}

int UT_HashTable::calcNewSpace()
{
	return m_iEntrySpace + CHUNK_NUM_ENTRIES;
}

int UT_HashTable::verifySpaceToAddOneEntry()
{
	if (!m_pBuckets)
	{
		if (0 != firstAlloc())
		{
			return -1;
		}
	}

	if ((m_iEntryCount+1) > m_iEntrySpace)
	{
		return grow();
	}

	return 0;
}

UT_uint32 UT_HashTable::hashFunc(const char* key) const
{
	UT_ASSERT(key);

	UT_uint32 sum = 0;

#if 1
	// Glib impl.
	const char *p = key;
	sum = *p;
 
	for (p += 1; *p != '\0'; p++)
		sum = (sum << 5) - sum + *p;
	
#else
	// Abi impl.
	unsigned char* q = (unsigned char*) key;

	int len = 0;
	while (*q)
	{
		len++;
		sum += *q;
		q++;
	}

	sum = sum * len + len;
#endif

	sum = sum % m_iBuckets;

	UT_ASSERT(sum >= 0);
	UT_ASSERT(sum < m_iBuckets);

	return sum;
}
