 
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
	int getEntryCount(void);
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
