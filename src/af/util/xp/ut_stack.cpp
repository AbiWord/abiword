 
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

#include "ut_assert.h"
#include "ut_stack.h"

UT_Bool UT_Stack::push(void * pVoid)
{
	UT_ASSERT(pVoid);
	UT_uint32 error = m_vecStack.addItem(pVoid);
	return (error == 0);
}

UT_Bool UT_Stack::pop(void ** ppVoid)
{
	UT_ASSERT(ppVoid);
	UT_uint32 indexEnd = m_vecStack.getItemCount();
	if (!indexEnd)
		return UT_FALSE;
	*ppVoid = m_vecStack.getLastItem();
	m_vecStack.deleteNthItem(indexEnd-1);
	return UT_TRUE;
}

UT_Bool UT_Stack::viewTop(void ** ppVoid) const
{
	UT_ASSERT(ppVoid);
	UT_uint32 indexEnd = m_vecStack.getItemCount();
	if (!indexEnd)
		return UT_FALSE;
	*ppVoid = m_vecStack.getLastItem();
	return UT_TRUE;
}
