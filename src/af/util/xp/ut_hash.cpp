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
#include "ut_assert.h"

#define CHUNK_NUM_ENTRIES		8

UT_HashTable::UT_HashTable(int iBuckets) : m_pool()
{
	UT_ASSERT(iBuckets>=3);		// NB: must be prime

	m_iBuckets = iBuckets;
	m_pBuckets = NULL;
	m_pEntries = NULL;
	m_iEntrySpace = 0;
	m_iEntryCount = 0;
}

UT_HashTable::~UT_HashTable()
{
	// in case this never got used
	if (!m_pBuckets)
		return;

	for (int i=0; i < m_iBuckets; i++)
	{
		UT_HashEntryListNode* pHELN = m_pBuckets[i].pHead;

		while (pHELN)
		{
			UT_HashEntryListNode* pTmp = pHELN->pNext;
			delete pHELN;
			pHELN = pTmp;
		}
	}
	
	free(m_pBuckets);
	free(m_pEntries);
}

UT_sint32 UT_HashTable::addEntry(const char* pszLeft, const char* pszRight, void* pData)
{
	UT_ASSERT(pszLeft);
	UT_ASSERT(pszRight || pData);

	// TODO check to see if the entry is already there.

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

	UT_HashEntryListNode* pHELN = new UT_HashEntryListNode();
	pHELN->iEntry = m_iEntryCount;
	pHELN->pNext = m_pBuckets[iBucket].pHead;
	m_pBuckets[iBucket].pHead = pHELN;
	m_iEntryCount++;

	return 0;
}

UT_sint32 UT_HashTable::setEntry(UT_HashTable::UT_HashEntry* pEntry, const char* pszRight, void* pData)
{
	if (pszRight)
		pEntry->pszRight = m_pool.addString(pszRight);	// TODO this can fail, right?
	
	pEntry->pData = pData;

	return 0;
}

UT_HashTable::UT_HashEntry* UT_HashTable::findEntry(const char* pszLeft)
{
	if (!m_pBuckets)
	{
		return NULL;
	}

	UT_ASSERT(m_pBuckets);
	int iBucket = hashFunc(pszLeft);
	UT_HashEntryListNode* pHELN = m_pBuckets[iBucket].pHead;
	while (pHELN)
	{
		UT_HashEntry* pEntry = m_pEntries + pHELN->iEntry;
		if (0 == strcmp(pEntry->pszLeft, pszLeft))
		{
			return pEntry;
		}

		pHELN = pHELN->pNext;
	}

	return NULL;
}

int UT_HashTable::getEntryCount(void)
{
	return m_iEntryCount;
}

UT_HashTable::UT_HashEntry* UT_HashTable::getNthEntry(int n)
{
	UT_ASSERT((n>=0) && (n<m_iEntryCount));

	return m_pEntries + n;
}

int UT_HashTable::firstAlloc()
{
	UT_ASSERT(!m_pEntries);

	m_pBuckets = (UT_HashTable::UT_HashBucket*) calloc(m_iBuckets, sizeof(UT_HashTable::UT_HashBucket));
	if (!m_pBuckets)
	{
		return -1;
	}

	m_iEntryCount = 0;
	m_iEntrySpace = CHUNK_NUM_ENTRIES;
	m_pEntries = (UT_HashTable::UT_HashEntry*) calloc(m_iEntrySpace, sizeof(UT_HashTable::UT_HashEntry));
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
	UT_HashTable::UT_HashEntry *pNewEntries = (UT_HashTable::UT_HashEntry *) calloc(iNewSpace, sizeof(UT_HashTable::UT_HashEntry));
	if (!pNewEntries)
	{
		return -1;
	}

	memcpy(pNewEntries, m_pEntries, m_iEntryCount*sizeof(UT_HashTable::UT_HashEntry));
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

UT_uint32 UT_HashTable::hashFunc(const char* p)
{
	UT_ASSERT(p);
	unsigned char* q = (unsigned char*) p;

	int len = 0;
	int sum = 0;
	while (*q)
	{
		len++;
		sum += *q;
		q++;
	}

	sum = sum * len + len;

	sum = sum % m_iBuckets;

	UT_ASSERT(sum>=0);
	UT_ASSERT(sum < m_iBuckets);

	return sum;
}
