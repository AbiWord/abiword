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
 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ut_assert.h"
#include "ut_types.h"
#include "ut_bytebuf.h"

#define DEFAULT_CHUNK		1024
#define MIN_CHUNK			256

UT_ByteBuf::UT_ByteBuf(UT_uint32 iChunk)
{
	if (iChunk < MIN_CHUNK)
		iChunk = DEFAULT_CHUNK;
	m_iChunk = iChunk;

	// since constructors cannot report failure, we defer
	// the malloc until the first use.

	m_pBuf = 0;
	m_iSize = 0;
	m_iSpace = 0;
}

UT_ByteBuf::~UT_ByteBuf()
{
	if (m_pBuf)
		free(m_pBuf);
}

bool UT_ByteBuf::_byteBuf(UT_uint32 spaceNeeded)
{
	// expand the buffer if necessary to accomidate the requested space.
	// round up to the next multiple of the chunk size.
	
	UT_uint32 newSize = ((m_iSize+spaceNeeded+m_iChunk-1)/m_iChunk)*m_iChunk;
	UT_Byte * pNew = (UT_Byte *)calloc(newSize,sizeof(*m_pBuf));
	if (!pNew)
		return false;
	
	if (m_pBuf)
	{
		memmove(pNew,m_pBuf,m_iSize*sizeof(*m_pBuf));
		free(m_pBuf);
	}

	m_pBuf = pNew;
	m_iSpace = newSize;

	return true;
}

bool UT_ByteBuf::append(const UT_Byte * pValue, UT_uint32 length)
{
	return ins(m_iSize,pValue,length);
}

bool UT_ByteBuf::ins(UT_uint32 position, const UT_Byte * pValue, UT_uint32 length)
{
	// insert the given buffer into the bytebuf

	if (!length)
	{
		UT_ASSERT(!pValue);
		return true;
	}
	
	UT_ASSERT(pValue);
	
	if (m_iSpace-m_iSize < length)
		if (!_byteBuf(length))
			return false;

	if (m_iSize > position)
		memmove(m_pBuf+position+length,m_pBuf+position,(m_iSize-position)*sizeof(*m_pBuf));
	m_iSize += length;
	memmove(m_pBuf+position,pValue,length*sizeof(*m_pBuf));

	return true;
}

bool UT_ByteBuf::ins(UT_uint32 position, UT_uint32 length)
{
	// insert zeroed space into the bytebuf

	if (!length)
		return true;
	
	if (m_iSpace-m_iSize < length)
		if (!_byteBuf(length))
			return false;

	if (m_iSize > position)
		memmove(m_pBuf+position+length,m_pBuf+position,(m_iSize-position)*sizeof(*m_pBuf));
	m_iSize += length;
	memset(m_pBuf+position,0,length*sizeof(*m_pBuf));

	return true;
}

bool UT_ByteBuf::del(UT_uint32 position, UT_uint32 amount)
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
		m_pBuf = (UT_Byte *)realloc(m_pBuf, newSpace*sizeof(*m_pBuf));  //Re-allocate to the smaller size
		m_iSpace = newSpace; //update m_iSpace to the new figure
	}

	
	return true;
}

UT_uint32 UT_ByteBuf::getLength(void) const
{
	// return the number of items in the buffer

	return m_iSize;
}

const UT_Byte * UT_ByteBuf::getPointer(UT_uint32 position) const
{
	// return a read-only pointer to the buffer
	
	if (!m_pBuf || !m_iSize)
		return 0;
	UT_ASSERT(position < m_iSize);
	return m_pBuf+position;
}

bool UT_ByteBuf::overwrite(UT_uint32 position, UT_Byte * pValue, UT_uint32 length)
{
	// overwrite the current cells at the given position for the given length.

	if (!length)
	{
		UT_ASSERT(!pValue);
		return true;
	}
	
	UT_ASSERT(pValue);

	if (m_iSpace < position+length)
		if (!_byteBuf(position+length-m_iSpace))
			return false;

	memmove(m_pBuf+position,pValue,length*sizeof(*m_pBuf));
	return true;
}

void UT_ByteBuf::truncate(UT_uint32 position)
{
	if (position < m_iSize)
		m_iSize = position;

	UT_uint32 newSpace = ((m_iSize+m_iChunk-1)/m_iChunk)*m_iChunk; //Calculate the new space needed
	if (newSpace != m_iSpace)
	{
		m_pBuf = (UT_Byte *)realloc(m_pBuf, newSpace*sizeof(*m_pBuf));  //Re-allocate to the smaller size
		m_iSpace = newSpace; //update m_iSpace to the new figure
	}
}

bool UT_ByteBuf::insertFromFile(UT_uint32 iPosition, FILE * fp)
{
	UT_uint32 iLengthOfFile = ftell (fp);

	if (0 != fseek(fp, 0, SEEK_SET))
	{
		return false;
	}

	// create a lot of space initialized to 0s
	this->ins (iPosition, iLengthOfFile);

	UT_Byte *pBuf = m_pBuf + iPosition;	
	UT_uint32 iBytesRead = 0;

	while (iBytesRead < iLengthOfFile)
	{
		iBytesRead += fread (pBuf + iBytesRead, sizeof(UT_Byte), 
							 iLengthOfFile - iBytesRead, fp);
	
	}
	return true;
}

bool UT_ByteBuf::insertFromFile(UT_uint32 iPosition, const char* pszFileName)
{
	UT_ASSERT(pszFileName && pszFileName[0]);
	
	FILE* fp = fopen(pszFileName, "rb");
	if (!fp)
	{
		return false;
	}

	if (0 != fseek(fp, 0, SEEK_END))
	{
		fclose(fp);
		return false;
	}

	bool b = this->insertFromFile (iPosition, fp);

	fclose(fp);
	return b;
}

bool UT_ByteBuf::writeToFile(const char* pszFileName)
{
	UT_ASSERT(pszFileName && pszFileName[0]);
	
	FILE* fp = fopen(pszFileName, "wb");
	if (!fp)
	{
		return false;
	}

	UT_uint32 iBytesWritten = fwrite(m_pBuf, 1, m_iSize, fp);
	if (iBytesWritten != m_iSize)
	{
		fclose(fp);
		return false;
	}

	fclose(fp);

	return true;
}
