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

// TODO change the 'int' types to 'UT_[su]int32' whichever is appropriate.

#include "ut_types.h"
#include "ut_vector.h"
#include "ut_assert.h"

UT_Vector::UT_Vector()
{
	m_iCutoffDouble = 128;
	m_iPostCutoffIncrement = 32;
	m_iCount = 0;
	m_iSpace = 0;
	m_pEntries = NULL;
}

void UT_Vector::clear()
{
	m_iCount = 0;
	m_iSpace = 0;
	free(m_pEntries);
	m_pEntries = NULL;
}

UT_Vector::~UT_Vector()
{
	if (m_pEntries)
	{
		free(m_pEntries);
	}
}

UT_uint32 UT_Vector::calcNewSpace()
{
	if (m_iSpace < m_iCutoffDouble)
	{
		if (m_iSpace > 0)
		{
			return m_iSpace * 2;
		}
		else
		{
			return m_iPostCutoffIncrement;
		}
	}
	else
	{
		return m_iSpace + m_iPostCutoffIncrement;
	}
}

UT_uint32 UT_Vector::getItemCount() const
{
	return m_iCount;
}

UT_sint32 UT_Vector::grow()
{
	UT_uint32 new_iSpace = calcNewSpace();

	void ** new_pEntries = (void**) calloc(new_iSpace, sizeof(void*));
	if (!new_pEntries)
	{
		return -1;
	}

	if (m_pEntries && (m_iCount > 0))
	{
		for (UT_uint32 i=0; i<m_iCount; i++)
		{
			new_pEntries[i] = m_pEntries[i];
		}

		free(m_pEntries);
	}

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
		int err = grow();
		if (err)
		{
			return err;
		}
	}

	// bump the elements -> thataway up to the ndxth position
	for (UT_uint32 i = m_iCount; i > ndx; i--)
	{
		m_pEntries[i] = m_pEntries[i - 1];
	}

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
		UT_sint32 err = grow();
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
	if (ndx >= m_iCount)
		return -1;
	if (ppOld)
		*ppOld = m_pEntries[ndx];
	m_pEntries[ndx] = pNew;
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

	for (UT_uint32 k=n; k<m_iCount-1; k++)
	{
		m_pEntries[k] = m_pEntries[k+1];
	}
	
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


