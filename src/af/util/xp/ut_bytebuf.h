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
 


#ifndef UT_BYTEBUF_H
#define UT_BYTEBUF_H

/*****************************************************************
** A buffer class which can grow and shrink
*****************************************************************/

#include "ut_types.h"

class UT_ByteBuf
{
public:
	UT_ByteBuf(UT_uint32 iChunk = 0);
	~UT_ByteBuf();

	UT_Bool				ins(UT_uint32 position, const UT_Byte * pValue, UT_uint32 length);
	UT_Bool				ins(UT_uint32 position, UT_uint32 length);
	UT_Bool				del(UT_uint32 position, UT_uint32 amount);
	UT_Bool				overwrite(UT_uint32 position, UT_Byte * pValue, UT_uint32 length);
	void				truncate(UT_uint32 position);
	UT_uint32			getLength(void) const;
	const UT_Byte *		getPointer(UT_uint32 position) const;				/* temporary use only */
	UT_Bool				writeToFile(const char* pszFileName);
	
protected:
	UT_Bool				_byteBuf(UT_uint32 spaceNeeded);

	UT_Byte *			m_pBuf;
	UT_uint32			m_iSize;			/* amount currently used */
	UT_uint32			m_iSpace;			/* space currently allocated */
	UT_uint32			m_iChunk;			/* unit for realloc */
};

#endif /* UT_BYTEBUF_H */
