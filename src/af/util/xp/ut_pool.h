 
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

#ifndef UT_POOL_H
#define UT_POOL_H

#include "ut_types.h"

class UT_StringPool
{
public:
	UT_StringPool();
	char* addString(const char*);

protected:
	int	addBucket();

	struct UT_PoolBucket
	{
		UT_PoolBucket(int iSize);

		char*		pChars;
		int			iCurLen;
		int			iSpace;

		UT_PoolBucket* pNext;
	};

	UT_PoolBucket*	m_pFirstBucket;
};

#endif /* UT_POOL_H */
