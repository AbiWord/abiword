 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiSource Utilities.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
*/

#include <stdlib.h>
#include <string.h>

#include "ut_pool.h"

#define UT_POOL_BUCKET_SIZE		1024

UT_StringPool::UT_StringPool()
{
	m_pFirstBucket = NULL;
	addBucket();
}

UT_StringPool::~UT_StringPool()
{
	while (m_pFirstBucket)
	{
		UT_PoolBucket* pTmp = m_pFirstBucket->pNext;
		delete m_pFirstBucket;
		m_pFirstBucket = pTmp;
	}
}


UT_StringPool::UT_PoolBucket::UT_PoolBucket(int iSize)
{
	// TODO how do we handle constructor failure here?
	pChars = new char[iSize];
	iSpace = iSize;
	iCurLen = 0;
	pNext = NULL;
}

UT_StringPool::UT_PoolBucket::~UT_PoolBucket()
{
	delete pChars;
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

