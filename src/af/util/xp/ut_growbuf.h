 
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

#ifndef UT_GROWBUF_H
#define UT_GROWBUF_H

/*****************************************************************
** A buffer class which can grow and shrink
*****************************************************************/

#include "ut_types.h"

class UT_GrowBuf
{
public:
	UT_GrowBuf(UT_uint32 iChunk = 0);
	~UT_GrowBuf();

	UT_Bool				ins(UT_uint32 position, const UT_uint16 * pValue, UT_uint32 length);
	UT_Bool				ins(UT_uint32 position, UT_uint32 length);
	UT_Bool				del(UT_uint32 position, UT_uint32 amount);
	UT_Bool				overwrite(UT_uint32 position, UT_uint16 * pValue, UT_uint32 length);
	void				truncate(UT_uint32 position);
	UT_uint32			getLength(void) const;
	UT_uint16 *			getPointer(UT_uint32 position) const;				/* temporary use only */
	
protected:
	UT_Bool				_growBuf(UT_uint32 spaceNeeded);

	UT_uint16 *			m_pBuf;
	UT_uint32			m_iSize;			/* amount currently used */
	UT_uint32			m_iSpace;			/* space currently allocated */
	UT_uint32			m_iChunk;			/* unit for realloc */
};

#endif /* UT_GROWBUF_H */
