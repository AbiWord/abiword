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

// TODO change the 'int' types to 'UT_[su]int32' whichever is appropriate.

#include "ut_types.h"
#include "ut_vector.h"
#include "ut_assert.h"

#ifndef ABI_OPT_STL

UT_Vector::UT_Vector(int sizehint)
{
	m_iCutoffDouble = sizehint;		/* After this point we stop doubling our allocations */		
	m_iPostCutoffIncrement = 32;	/* We only increment the array by this much after our allocations */
	m_iCount = 0;					/* The number of slots we have filled */
	m_iSpace = 0;					/* The number of slots we have allocated */
	m_pEntries = NULL;				/* The actual array of pointers itself */
}

void UT_Vector::clear()
{
	m_iCount = 0;
	m_iSpace = 0;
	FREEP(m_pEntries);
	m_pEntries = NULL;
}

UT_Vector::~UT_Vector()
{
	FREEP(m_pEntries);
}

UT_uint32 UT_Vector::getItemCount() const
{
	return m_iCount;
}

/*
 This function is called everytime we want to insert a new element but don't have
 enough space.  In this case we grow the array to be _at least_ ndx size.
*/
UT_sint32 UT_Vector::grow(UT_uint32 ndx)
{
	UT_uint32 new_iSpace;
	if(!m_iSpace) {
		new_iSpace = m_iPostCutoffIncrement;
	}
	else if (m_iSpace < m_iCutoffDouble) {
		new_iSpace = m_iSpace * 2;
	}
	else {
		new_iSpace = m_iSpace + m_iPostCutoffIncrement;
	}

	if (new_iSpace < ndx)
	{
		new_iSpace = ndx;
	}

	void ** new_pEntries = (void **)realloc(m_pEntries, new_iSpace * sizeof(void *));
	if (!new_pEntries) {
		return -1;
	}
	//Is this required? We always check Count first anyways.
	//memset(&new_pEntries[m_iSpace], 0, new_iSpace - m_iSpace * sizeof(void *));
	m_iSpace = new_iSpace;
	m_pEntries = new_pEntries;

	return 0;
}

UT_sint32 UT_Vector::insertItemAt(void* p, UT_uint32 ndx)
{
	if (ndx > m_iCount + 1)
		return -1;
	
	if ((m_iCount+1) > m_iSpace)
	{
		UT_sint32 err = grow(0);
		if (err)
		{
			return err;
		}
	}

	// bump the elements -> thataway up to the ndxth position
	memmove(&m_pEntries[ndx+1], &m_pEntries[ndx], (m_iCount - ndx) * sizeof(void *)); 

	m_pEntries[ndx] = p;
	++m_iCount;

	return 0;
}

UT_sint32 UT_Vector::addItem(void* p, UT_uint32 * pIndex)
{
	int err = addItem(p);
	if (!err && pIndex)
		*pIndex = m_iCount-1;
	return err;
}

UT_sint32 UT_Vector::addItem(void* p)
{
	if ((m_iCount+1) > m_iSpace)
	{
		UT_sint32 err = grow(0);
		if (err)
		{
			return err;
		}
	}

	m_pEntries[m_iCount++] = p;

	return 0;
}

void* UT_Vector::getNthItem(UT_uint32 n) const
{
	UT_ASSERT(m_pEntries);
	UT_ASSERT(m_iCount > 0);
	UT_ASSERT(n<m_iCount);

	return m_pEntries[n];
}

UT_sint32 UT_Vector::setNthItem(UT_uint32 ndx, void * pNew, void ** ppOld)
{
	if ((ndx+1) > m_iSpace)
	{
		UT_sint32 err = grow(ndx+1);
		if (err)
		{
			return err;
		}
	}

	if (ppOld)
	{
		*ppOld = m_pEntries[ndx];
	}
	
	m_pEntries[ndx] = pNew;
	if ((ndx+1) > m_iCount)
	{
		m_iCount = ndx + 1;
	}
	
	return 0;
}

void* UT_Vector::getLastItem() const
{
	UT_ASSERT(m_iCount > 0);

	return m_pEntries[m_iCount-1];
}

void* UT_Vector::getFirstItem() const
{
	UT_ASSERT(m_iCount > 0);
	UT_ASSERT(m_pEntries);

	return m_pEntries[0];
}

void UT_Vector::deleteNthItem(UT_uint32 n)
{
	UT_ASSERT(n < m_iCount);
	UT_ASSERT(m_iCount > 0);

	memmove(&m_pEntries[n], &m_pEntries[n+1], (m_iCount - (n + 1)) * sizeof(void*));
	
	m_pEntries[m_iCount-1] = 0;
	m_iCount--;

	return;
}

UT_sint32 UT_Vector::findItem(void* p) const
{
	for (UT_uint32 i=0; i<m_iCount; i++)
	{
		if (m_pEntries[i] == p)
		{
			return (UT_sint32) i;
		}
	}

	return -1;
}

void UT_Vector::qsort(int (*compar)(const void *, const void *))
{
	::qsort(m_pEntries, m_iCount, sizeof(void*), compar);
}

UT_Bool UT_Vector::copy(UT_Vector *pVec)
{
	clear();

	for (UT_uint32 i=0; i < pVec->m_iCount; i++)
	{
		UT_sint32 err;

		err = addItem(pVec->m_pEntries[i]);
		if(err == -1)
			return err;
	}

	return 0;
}

const void* UT_Vector::operator[](UT_uint32 i) const
{
	return this->getNthItem(i);
}

#else /* ABI_OPT_STL */

UT_Vector::UT_Vector(int sizehint)
{
	//Ignore the sizehint
}

void UT_Vector::clear()
{
	m_STLVec.clear();
}

UT_Vector::~UT_Vector()
{
}

UT_uint32 UT_Vector::getItemCount() const
{
	return m_STLVec.size();
}

UT_sint32 UT_Vector::insertItemAt(void* p, UT_uint32 ndx)
{
	m_STLVec.insert(m_STLVec.begin()+ndx, p);
	return 0;
}

UT_sint32 UT_Vector::addItem(void* p, UT_uint32 * pIndex)
{
	int err = addItem(p);
	if (!err && pIndex)
		*pIndex = m_STLVec.size()-1;
	return err;
}

UT_sint32 UT_Vector::addItem(void* p)
{
	m_STLVec.push_back(p);

	return 0;
}

void* UT_Vector::getNthItem(UT_uint32 n) const
{
	return m_STLVec[n];
}

UT_sint32 UT_Vector::setNthItem(UT_uint32 ndx, void * pNew, void ** ppOld)
{
	if (ppOld)
	{
		*ppOld = m_STLVec[ndx];
	}
	
	if (m_STLVec.size() <= ndx)
		m_STLVec.resize (ndx+1);
	m_STLVec[ndx] = pNew;
		
	return 0;
}

void* UT_Vector::getLastItem() const
{
	return m_STLVec.back();
}

void* UT_Vector::getFirstItem() const
{
	return m_STLVec.front();
}

void UT_Vector::deleteNthItem(UT_uint32 n)
{
	m_STLVec.erase(m_STLVec.begin()+n);
}

UT_sint32 UT_Vector::findItem(void* p) const
{
	for (UT_uint32 i=0; i<m_STLVec.size(); i++)
	{
		if (m_STLVec[i] == p)
		{
			return (UT_sint32) i;
		}
	}

	return -1;
}

void UT_Vector::qsort(int (*compar)(const void *, const void *))
{
	sort(m_STLVec.begin(), m_STLVec.end(), compar);
}

UT_Bool UT_Vector::copy(UT_Vector *pVec)
{
	m_STLVec = pVec->m_STLVec;
	return 0;
}

const void* UT_Vector::operator[](UT_uint32 i) const
{
	return m_STLVec[i];
}


#endif /* ABI_OPT_STL */
