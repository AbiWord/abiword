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
 

#include "ut_alphahash.h"
#include "ut_string.h"
#include "ut_assert.h"

/*****************************************************************/

UT_AlphaHashTable::UT_AlphaHashTable(int iBuckets)
	: UT_HashTable(iBuckets)
{
}

UT_AlphaHashTable::~UT_AlphaHashTable()
{
	// we don't actually allocate any storage ourselves
	// because we rely on the underlying hash table and
	// vector to do everything.
}

UT_sint32 UT_AlphaHashTable::addEntry(const char* psLeft, const char* psRight, void* pData)
{
	UT_sint32 addHashError = UT_HashTable::addEntry(psLeft,psRight,pData);
	UT_ASSERT(addHashError == 0);

	int ndxEntryAdded = (m_iEntryCount-1);
	
	// find spot for this key in our alphabetically-sorted vector of
	// keys.  ***WE RELY ON THE HASHTABLE TO ACTUALLY STORE THE STRINGS
	// FOR US. WE STORE THE ENTRY INDEX FOR THIS ITEM IN OUR VECTOR.***
	
	UT_uint32 k;
	UT_uint32 kLimit = m_vecAlpha.getItemCount();
	for (k=0; (k < kLimit); k++)
	{
		UT_sint32 ndxK = (UT_sint32)m_vecAlpha.getNthItem(k);
		const char * szK = m_pEntries[ndxK].pszLeft;
		UT_sint32 cmp = UT_stricmp(psLeft,szK);
		if (cmp < 0)					// new key should be before the k-th one.
		{
			UT_sint32 errInsert = m_vecAlpha.insertItemAt((void *)ndxEntryAdded,k);
			UT_ASSERT(errInsert == 0);
			return 0;					// success
		}
		else if (cmp == 0)
		{
			// added duplicate key
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return -1;
		}
		else
		{
			// keep looking for insertion point
		}
	}

	// if we fall out of the loop, we just append it to the end
	// of the vector.

	UT_sint32 addVecError = m_vecAlpha.addItem((void *)ndxEntryAdded);
	UT_ASSERT(addVecError == 0);
	return 0;
}

UT_HashTable::UT_HashEntry * UT_AlphaHashTable::getNthEntryAlpha(int n)
{
	// return the entries in alphabetical order.

	UT_sint32 ndx = (UT_sint32)m_vecAlpha.getNthItem(n);
	return getNthEntry(ndx);
}

