
#include <stdlib.h>
#include <string.h>

#include "ut_pool.h"

#define UT_POOL_BUCKET_SIZE		1024

UT_StringPool::UT_StringPool()
{
	m_pFirstBucket = NULL;
	addBucket();
}

UT_StringPool::UT_PoolBucket::UT_PoolBucket(int iSize)
{
	// TODO how do we handle contructor failure here?
	pChars = new char[iSize];
	iSpace = iSize;
	iCurLen = 0;
	pNext = NULL;
}

int UT_StringPool::addBucket()
{
	UT_PoolBucket* pBuck = new UT_PoolBucket(1024);
	pBuck->pNext = m_pFirstBucket;
	m_pFirstBucket = pBuck;

	return 0; // TODO return code
}

char* UT_StringPool::addString(const char* p)
{
	int len = strlen(p);

	if ((m_pFirstBucket->iCurLen + len + 1) > m_pFirstBucket->iSpace)
	{
		if (0 != addBucket())
		{
			return NULL; // out of mem
		}
	}

	char*	pResult = m_pFirstBucket->pChars + m_pFirstBucket->iCurLen;

	for (int i=0; i<=len; i++)		// does the zero terminator too
	{
		m_pFirstBucket->pChars[m_pFirstBucket->iCurLen++] = p[i];
	}

	return pResult;
}

