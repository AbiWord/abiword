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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */
 


#include <stdlib.h>
#include <string.h>

#include "ut_assert.h"
#include "ut_types.h"
#include "ut_growbuf.h"

#define DEFAULT_CHUNK		1024*10
#define MIN_CHUNK			256

UT_GrowBuf::UT_GrowBuf(UT_uint32 iChunk)
  : m_pBuf(0), m_iSize(0), m_iSpace(0)
{
	if (iChunk < MIN_CHUNK)
		iChunk = DEFAULT_CHUNK;
	m_iChunk = iChunk;

	// we defer the g_try_malloc until the first use.
}

UT_GrowBuf::~UT_GrowBuf()
{
  FREEP(m_pBuf);
}

bool UT_GrowBuf::_growBuf(UT_uint32 spaceNeeded)
{
	// expand the buffer if necessary to accomidate the requested space.
	// round up to the next multiple of the chunk size.
	
	UT_uint32 newSize = ((m_iSize+spaceNeeded+m_iChunk-1)/m_iChunk)*m_iChunk;
	UT_GrowBufElement * pNew = static_cast<UT_GrowBufElement *>(UT_calloc(newSize,sizeof(*m_pBuf))); // Why not use g_try_realloc ? - fjf
	if (!pNew)
		return false;
	
	if (m_pBuf)
	{
		memmove(pNew,m_pBuf,m_iSize*sizeof(*m_pBuf));
		g_free(m_pBuf);
	}

	m_pBuf = pNew;
	m_iSpace = newSize;

	return true;
}
		
bool UT_GrowBuf::append(const UT_GrowBufElement * pValue, UT_uint32 length)
{
	return ins(m_iSize,pValue,length);
}

bool UT_GrowBuf::ins(UT_uint32 position, const UT_GrowBufElement * pValue, UT_uint32 length)
{
	// insert the given buffer into the growbuf

	if (!length)
	{
		UT_ASSERT(!pValue);
		return true;
	}
	
	UT_ASSERT(pValue);
	
	if (position > m_iSize)
	{
		// Situation here: We're inserting after the end of the
		// buffer that we've previously used (i.e. m_iSize).
		// Treat the request as if we were starting at m_iSize,
		// subtracting from position and adding to length.
		UT_uint32 slack = position - m_iSize;
		position = m_iSize; length += slack;
	}

	if (m_iSpace-m_iSize < length)
		if (!_growBuf(length))
			return false;

	if (m_iSize > position)
		memmove(m_pBuf+position+length,m_pBuf+position,(m_iSize-position)*sizeof(*m_pBuf));
	m_iSize += length;
	memmove(m_pBuf+position,pValue,length*sizeof(*m_pBuf));

	return true;
}

bool UT_GrowBuf::ins(UT_uint32 position, UT_uint32 length)
{
	// insert zeroed space into the growbuf

	if (!length)
		return true;
	
	if (position > m_iSize)
	{
		// Situation here: We're inserting after the end of the
		// buffer that we've previously used (i.e. m_iSize).
		// Treat the request as if we were starting at m_iSize,
		// subtracting from position and adding to length.
		UT_uint32 slack = position - m_iSize;
		position = m_iSize; length += slack;
	}

	if (m_iSpace-m_iSize < length)
		if (!_growBuf(length))
			return false;

	if (m_iSize > position)
		memmove(m_pBuf+position+length,m_pBuf+position,(m_iSize-position)*sizeof(*m_pBuf));
	m_iSize += length;
	memset(m_pBuf+position,0,length*sizeof(*m_pBuf));

	return true;
}

bool UT_GrowBuf::del(UT_uint32 position, UT_uint32 amount)
{
	if (!amount)
		return true;

	if (!m_pBuf)
		return false;
	UT_ASSERT(position < m_iSize);
	UT_ASSERT(position+amount <= m_iSize);
	
	memmove(m_pBuf+position,m_pBuf+position+amount,(m_iSize-position-amount)*sizeof(*m_pBuf));
	m_iSize -= amount;

	UT_uint32 newSpace = ((m_iSize+m_iChunk-1)/m_iChunk)*m_iChunk; //Calculate the new space needed
	if (newSpace != m_iSpace)
	{
		m_pBuf = static_cast<UT_GrowBufElement *>(g_try_realloc(m_pBuf, newSpace*sizeof(*m_pBuf)));  //Re-allocate to the smaller size
		m_iSpace = newSpace; //update m_iSpace to the new figure
	}
	
	return true;
}

UT_uint32 UT_GrowBuf::getLength(void) const
{
	// return the number of items in the buffer

	return m_iSize;
}

UT_GrowBufElement * UT_GrowBuf::getPointer(UT_uint32 position) const
{
	// return a read-only pointer to the buffer
	
	if (!m_pBuf || !m_iSize)
		return 0;
	UT_ASSERT(position < m_iSize);
	return m_pBuf+position;
}

bool UT_GrowBuf::overwrite(UT_uint32 position, UT_GrowBufElement * pValue, UT_uint32 length)
{
	// overwrite the current cells at the given position for the given length.

	if (!length)
	{
		UT_ASSERT(!pValue);
		return true;
	}
	
	UT_ASSERT(pValue);

	if (m_iSpace < position+length)
		if (!_growBuf(position+length-m_iSpace))
			return false;

	memmove(m_pBuf+position,pValue,length*sizeof(*m_pBuf));
	return true;
}

void UT_GrowBuf::truncate(UT_uint32 position)
{
	if ((m_pBuf == 0) && (position == 0))
		return;

	if (position < m_iSize)
		m_iSize = position;

	UT_uint32 newSpace = ((m_iSize+m_iChunk-1)/m_iChunk)*m_iChunk; //Calculate the new space needed
	if (newSpace == 0) newSpace = m_iChunk; // In case of UT_GrowBuf::truncate (0)
	if (newSpace != m_iSpace)
	{
		m_pBuf = static_cast<UT_GrowBufElement *>(g_try_realloc(m_pBuf, newSpace*sizeof(*m_pBuf)));  //Re-allocate to the smaller size
		m_iSpace = newSpace; //update m_iSpace to the new figure
	}
}

