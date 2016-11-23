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



#ifndef UT_BYTEBUF_H
#define UT_BYTEBUF_H

/*****************************************************************
** A buffer class which can grow and shrink
*****************************************************************/

#include <stdio.h>
#include <memory>
#include <gsf/gsf-input.h>

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

/* sigh... total hack here. Solaris does the following in unistd.h
 * #define truncate truncate64
 * I really hate/fear this kind of interaction of preprocessor and
 * C++ method name.
 * Anyway, I'm going to redefine truncate here; make sure to include
 * this file *after* unistd.h, however... - fjf
 */
#ifndef abi_std_truncate
#define abi_std_truncate truncate
#endif
#ifdef truncate
#undef truncate
#endif

class ABI_EXPORT UT_ByteBuf
{
public:
	UT_ByteBuf(UT_uint32 iChunk = 0);
	~UT_ByteBuf();

	bool				append(const UT_Byte * pValue, UT_uint32 length);
	bool				ins(UT_uint32 position, const UT_Byte * pValue, UT_uint32 length);
	bool				ins(UT_uint32 position, UT_uint32 length);
	bool				del(UT_uint32 position, UT_uint32 amount);
	bool				overwrite(UT_uint32 position, const UT_Byte * pValue, UT_uint32 length);
	void				truncate(UT_uint32 position);
	UT_uint32			getLength(void) const;
	const UT_Byte *		getPointer(UT_uint32 position) const;				/* temporary use only */
	bool				writeToFile(const char* pszFileName) const;
	bool				writeToURI(const char* pszURI) const;
	bool				insertFromFile(UT_uint32 iPosition, const char* pszFilename);
	bool                            insertFromURI(UT_uint32 iPosition, const char* pszURI);
	bool                            insertFromInput(UT_uint32 iPosition, GsfInput * fp);
	bool                insertFromFile(UT_uint32 iPosition, FILE * fp);
private:
	bool				_byteBuf(UT_uint32 spaceNeeded);

	UT_Byte *			m_pBuf;
	UT_uint32			m_iSize;			/* amount currently used */
	UT_uint32			m_iSpace;			/* space currently allocated */
	UT_uint32			m_iChunk;			/* unit for g_try_realloc */
};

typedef std::shared_ptr<UT_ByteBuf> UT_ByteBufPtr;
typedef std::shared_ptr<const UT_ByteBuf> UT_ConstByteBufPtr;

#endif /* UT_BYTEBUF_H */
