/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Program Utilities
 * Copyright (C) 1998-2003 AbiSource, Inc.
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

#include "ut_types.h"
#include "ut_vector.h"
#include "ut_assert.h"

UT_Vector::UT_Vector(UT_uint32 sizehint)
  : m_pEntries(NULL), m_iCount(0), m_iSpace(0),
    m_iCutoffDouble(sizehint), m_iPostCutoffIncrement(32)
{
}

UT_Vector::UT_Vector(const UT_Vector& utv)
{
	m_iCutoffDouble = utv.m_iCutoffDouble;
	m_iPostCutoffIncrement = utv.m_iPostCutoffIncrement;
	copy(&utv);
}

UT_Vector& UT_Vector::operator=(const UT_Vector& utv)
{
	if(this != &utv)
	{
		m_iCutoffDouble = utv.m_iCutoffDouble;
		m_iPostCutoffIncrement = utv.m_iPostCutoffIncrement;
		copy(&utv);
	}
	return *this;
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

	void ** new_pEntries = static_cast<void **>(realloc(m_pEntries, new_iSpace * sizeof(void *)));
	if (!new_pEntries) {
		return -1;
	}
	//Is this required? We always check Count first anyways.
	// TMN: Unfortunately it is, since the class GR_CharWidths
	// uses UT_Vector as a sparse array!
	memset(&new_pEntries[m_iSpace], 0, (new_iSpace - m_iSpace) * sizeof(void *));
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

UT_sint32 UT_Vector::addItem(const void* p, UT_uint32 * pIndex)
{
	UT_sint32 err = addItem(p);
	if (!err && pIndex)
		*pIndex = m_iCount-1;
	return err;
}

UT_sint32 UT_Vector::addItem(const void* p)
{
	if ((m_iCount+1) > m_iSpace)
	{
		UT_sint32 err = grow(0);
		if (err)
		{
			return err;
		}
	}

	m_pEntries[m_iCount++] = (void *)p;  /*** bad, cast away const so we can build again ***/

	return 0;
}

/** It returns true if there were no errors, false elsewhere */
bool UT_Vector::pop_back()
{
	if (m_iCount > 0)
	{
		--m_iCount;
		return true;
	}
	else
		return false;
}

UT_sint32 UT_Vector::setNthItem(UT_uint32 ndx, void * pNew, void ** ppOld)
{
	const UT_uint32 old_iSpace = m_iSpace;

	// skip realloc in cases where we are removing an entry, as its probably an error
	UT_return_val_if_fail(!((ndx >= m_iSpace) && (pNew == NULL) && (ppOld == NULL)), -1)

	if (ndx >= m_iSpace)
	{
		const UT_sint32 err = grow(ndx+1);
		if (err)
		{
			return err;
		}
	}

	if (ppOld)
	{
		*ppOld = (ndx < old_iSpace) ? m_pEntries[ndx] : 0;
	}
	
	m_pEntries[ndx] = pNew;
	if (ndx >= m_iCount)
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
			return static_cast<UT_sint32>(i);
		}
	}

	return -1;
}

void UT_Vector::qsort(int (*compar)(const void *, const void *))
{
	::qsort(m_pEntries, m_iCount, sizeof(void*), compar);
}

bool UT_Vector::copy(const UT_Vector *pVec)
{
	clear();

	for (UT_uint32 i=0; i < pVec->m_iCount; i++)
	{
		UT_sint32 err;

		err = addItem(pVec->m_pEntries[i]);
		if(err == -1)
			return (err ? true : false);
	}

	return 0;
}

const void* UT_Vector::operator[](UT_uint32 i) const
{
	return this->getNthItem(i);
}



UT_NumberVector::UT_NumberVector (UT_uint32 sizehint, UT_uint32 baseincr) :
	m_pEntries(0),
	m_iCount(0),
	m_iSpace(0),
	m_iInitialSize(sizehint),
	m_iIncrement(baseincr)
{
	// 
}

UT_NumberVector::UT_NumberVector (const UT_NumberVector & NV) :
	m_pEntries(0),
	m_iCount(0),
	m_iSpace(0),
	m_iInitialSize(NV.m_iInitialSize),
	m_iIncrement(NV.m_iIncrement)
{
	copy (NV);
}

UT_NumberVector::~UT_NumberVector ()
{
	clear ();
}

UT_NumberVector & UT_NumberVector::operator= (const UT_NumberVector & NV)
{
	copy (NV);
	return *this;
}

bool UT_NumberVector::copy (const UT_NumberVector & NV)
{
	clear (false);

	if (NV.m_iCount > m_iSpace)
		if (grow (NV.m_iCount) == -1)
			return false; // :-(

	if (NV.m_iCount)
		{
			m_iCount = NV.m_iCount;
			memcpy (m_pEntries, NV.m_pEntries, m_iCount * sizeof (UT_sint32));
		}
	return true;
}

void UT_NumberVector::clear (bool free_memory)
{
	if (free_memory)
		{
			FREEP(m_pEntries);
			m_iSpace = 0;
		}
	m_iCount = 0;
}

/* addItem() returns 0 on success, -1 on failure:
 */
UT_sint32 UT_NumberVector::addItem (UT_sint32 number)
{
	if (grow (m_iCount + 1) == -1)
		return -1; // :-(

	m_pEntries[m_iCount++] = number;

	return 0;
}

UT_sint32 UT_NumberVector::getNthItem (UT_uint32 index) const
{
	UT_ASSERT(index < m_iCount);
	return ((index < m_iCount) ? m_pEntries[index] : 0);
}

/* setNthItem() and insertItemAt() return 0 on success, -1 on failure:
 */
UT_sint32 UT_NumberVector::setNthItem (UT_uint32 index, UT_sint32 new_number, UT_sint32 * old_number)
{
	if (index >= m_iCount)
		{
			if (grow (index + 1) == -1)
				return -1; // :-(

			m_iCount = index + 1;
		}

	if (old_number) *old_number = m_pEntries[index];

	m_pEntries[index] = new_number;

	return 0;
}

UT_sint32 UT_NumberVector::insertItemAt (UT_sint32 number, UT_uint32 index)
{
	UT_ASSERT(index <= m_iCount);
	if (index > m_iCount)
		return -1;

	if (grow (m_iCount + 1) == -1)
		return -1; // :-(

	if (index <  m_iCount)
		memmove (m_pEntries + index + 1, m_pEntries + index, (m_iCount - index) * sizeof (UT_sint32));

	++m_iCount;

	m_pEntries[index] = number;

	return 0;
}

void UT_NumberVector::deleteNthItem (UT_uint32 index)
{
	if (index >= m_iCount) return;

	--m_iCount;

	if (index <  m_iCount)
		memmove (m_pEntries + index, m_pEntries + index + 1, (m_iCount - index) * sizeof (UT_sint32));

	m_pEntries[m_iCount] = 0;
}

/* findItem() returns index >= 0 of first instance of number, or -1 if not found:
 */
UT_sint32 UT_NumberVector::findItem (UT_sint32 number) const
{
	UT_sint32 retval = -1;

	for (UT_uint32 index = 0; index < m_iCount; index++)
		if (m_pEntries[index] == number)
			{
				retval = static_cast<UT_sint32>(index);
				break;
			}
	return retval;
}

/* grow() returns 0 on success, -1 on failure:
 */
UT_sint32 UT_NumberVector::grow (UT_uint32 requirement)
{
	if (requirement <= m_iSpace) return 0;

	UT_uint32 new_space = m_iInitialSize;
	if (new_space < requirement)
		{
			new_space = (requirement - m_iInitialSize) / m_iIncrement;
			new_space = m_iInitialSize + new_space * m_iIncrement;

			if (new_space < requirement)
				new_space += m_iIncrement;
		}
	if (m_pEntries == 0)
		{
			m_pEntries = reinterpret_cast<UT_sint32 *>(malloc (new_space * sizeof (UT_sint32)));
			UT_ASSERT(m_pEntries);
			if (m_pEntries == 0)
				return -1;
			memset (m_pEntries, 0, new_space * sizeof (UT_sint32));
		}
	else
		{
			UT_sint32 * more = 0;
			more = reinterpret_cast<UT_sint32 *>(realloc (m_pEntries, new_space * sizeof (UT_sint32)));
			UT_ASSERT(more);
			if (more == 0)
				return -1;
			m_pEntries = more;
			memset (m_pEntries + m_iSpace, 0, (new_space - m_iSpace) * sizeof (UT_sint32));
		}
	m_iSpace = new_space;

	return 0;
}
