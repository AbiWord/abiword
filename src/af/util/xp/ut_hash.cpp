
#include <stdlib.h>
#include <string.h>

#include "ut_hash.h"
#include "ut_assert.h"

#define CHUNK_NUM_ENTRIES		8
#define NUM_BUCKETS				511

UT_HashTable::UT_HashTable() : m_pool()
{
	m_pBuckets = NULL;
	m_pEntries = NULL;
	m_iEntrySpace = 0;
	m_iEntryCount = 0;
}

int UT_HashTable::addEntry(const char* pszLeft, const char* pszRight, void* pData)
{
	UT_ASSERT(pszLeft);
	UT_ASSERT(pszRight);

	// TODO check to see if the entry is already there.

	if (0 != verifySpaceToAddOneEntry())
	{
		return -1;
	}

	// TODO the following are essentially memory allocations which can fail.  check them
	m_pEntries[m_iEntryCount].pszLeft = m_pool.addString(pszLeft);
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

UT_HashTable::UT_HashEntry* UT_HashTable::getNthEntry(int n)
{
	UT_ASSERT((n>=0) && (n<m_iEntryCount));

	return m_pEntries + n;
}

int UT_HashTable::firstAlloc()
{
	UT_ASSERT(!m_pEntries);

	m_pBuckets = (UT_HashTable::UT_HashBucket*) calloc(NUM_BUCKETS, sizeof(UT_HashTable::UT_HashBucket));
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

	sum = sum % NUM_BUCKETS;

	UT_ASSERT(sum>=0);
	UT_ASSERT(sum < NUM_BUCKETS);

	return sum;
}
