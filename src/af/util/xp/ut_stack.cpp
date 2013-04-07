/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Program Utilities
 * Copyright (C) 1998-2000 AbiSource, Inc.
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
 


#include "ut_assert.h"
#include "ut_stack.h"

bool UT_Stack::push(void * pVoid)
{
	UT_uint32 error = m_vecStack.addItem(pVoid);
	return (error == 0);
}

bool UT_Stack::pop(void ** ppVoid)
{
	UT_ASSERT(ppVoid);
	UT_uint32 indexEnd = m_vecStack.getItemCount();
	if (!indexEnd) {
		*ppVoid = 0;
		return false;
	}
	*ppVoid = const_cast<void*>(m_vecStack.getLastItem());
	m_vecStack.deleteNthItem(indexEnd-1);
	return true;
}

bool UT_Stack::viewTop(void ** ppVoid) const
{
	UT_ASSERT(ppVoid);
	UT_uint32 indexEnd = m_vecStack.getItemCount();
	if (!indexEnd) {
		*ppVoid = 0;
		return false;
	}
	*ppVoid = const_cast<void*>(m_vecStack.getLastItem());
	return true;
}

UT_sint32 UT_Stack::getDepth(void) const
{
	return m_vecStack.getItemCount();
}

UT_NumberStack::UT_NumberStack (UT_uint32 sizehint, UT_uint32 baseincr) :
	m_vecStack(sizehint,baseincr)
{
	// 
}

bool UT_NumberStack::push (UT_sint32 number)
{
	return (m_vecStack.addItem (number) == 0);
}

bool UT_NumberStack::pop (UT_sint32 * number)
{
	if (!m_vecStack.getItemCount ()) return false;

	if (number) *number = m_vecStack.getLastItem ();

	return m_vecStack.pop_back ();
}

bool UT_NumberStack::viewTop (UT_sint32 & number) const
{
	if (!m_vecStack.getItemCount ()) return false;

	number = m_vecStack.getLastItem ();

	return true;
}

UT_sint32 UT_NumberStack::getDepth () const
{
	return m_vecStack.getItemCount ();
}
