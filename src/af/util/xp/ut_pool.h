
#ifndef UT_POOL_H
#define UT_POOL_H

#include "ut_types.h"

class UT_StringPool
{
public:
	UT_StringPool();
	char* addString(const char*);

protected:
	int	addBucket();

	struct UT_PoolBucket
	{
		UT_PoolBucket(int iSize);

		char*		pChars;
		int			iCurLen;
		int			iSpace;

		UT_PoolBucket* pNext;
	};

	UT_PoolBucket*	m_pFirstBucket;
};

#endif /* UT_POOL_H */
