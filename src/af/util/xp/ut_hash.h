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
 


#ifndef UTHASH_H
#define UTHASH_H

#include "ut_types.h"
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

	UT_HashTable(int iBuckets);
	UT_sint32 addEntry(const char* psLeft, const char* psRight, void* pData);
	UT_sint32 setEntry(UT_HashTable::UT_HashEntry* pEntry, const char* pszRight, void* pData);
	~UT_HashTable();
	int getEntryCount(void);
	UT_HashEntry* getNthEntry(int n);
	UT_HashEntry* findEntry(const char* psLeft);

protected:
	UT_uint32 hashFunc(const char*);

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

	int				m_iBuckets;
	UT_HashBucket*	m_pBuckets;
	UT_HashEntry*	m_pEntries;
	int				m_iEntrySpace;
	int				m_iEntryCount;
	UT_StringPool	m_pool;
};

// NB: this macro is useful only in destructors
#define UT_HASH_PURGEDATA(d, h)							\
	do	{	int uth_max = h.getEntryCount();			\
			UT_HashTable::UT_HashEntry* uth_e;			\
			for (int uth=uth_max-1; uth>=0; uth--)		\
			{											\
				uth_e = h.getNthEntry(uth);				\
				UT_ASSERT(uth_e);						\
				if (uth_e)								\
				{										\
					d* uth_p = (d*) uth_e->pData;		\
					if (uth_p)							\
						delete uth_p;					\
				}										\
			}											\
	} while (0)

#endif /* UTHASH_H */
