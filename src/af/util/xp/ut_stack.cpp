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
 


#include "ut_assert.h"
#include "ut_stack.h"

UT_Bool UT_Stack::push(void * pVoid)
{
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

UT_uint32 UT_Stack::getDepth(void) const
{
	return m_vecStack.getItemCount();
}
