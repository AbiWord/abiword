
#ifndef UTHASH_H
#define UTHASH_H

#include "ut_pool.h"

class UT_HashTable
{
public:

	struct UT_HashEntry
	{
		char*	pszLeft;
		char*	pszRight;
		void*	pData;
	};

	UT_HashTable();
	int addEntry(const char* psLeft, const char* psRight, void* pData);
	UT_HashEntry* getNthEntry(int n);
	UT_HashEntry* findEntry(const char* psLeft);

protected:
	static UT_uint32 hashFunc(const char*);

	int	verifySpaceToAddOneEntry();
	int firstAlloc();
	int grow();
	int calcNewSpace();

	struct UT_HashEntryListNode
	{
		int						iEntry;
		UT_HashEntryListNode*	pNext;
	};

	struct UT_HashBucket
	{
		UT_HashEntryListNode* pHead;
	};

	UT_HashBucket*	m_pBuckets;
	UT_HashEntry*	m_pEntries;
	int				m_iEntrySpace;
	int				m_iEntryCount;
	UT_StringPool	m_pool;
};

#endif /* UTHASH_H */
