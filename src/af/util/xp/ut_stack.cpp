/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t; -*- */

/* AbiSource Program Utilities
 * Copyright (C) 1998-2000 AbiSource, Inc.
 *
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

void UT_Stack::push(void * pVoid)
{
	m_vecStack.push_back(pVoid);
}

bool UT_Stack::pop(void ** ppVoid)
{
	UT_ASSERT(ppVoid);
	UT_uint32 indexEnd = m_vecStack.size();
	if (!indexEnd) {
		*ppVoid = nullptr;
		return false;
	}
	*ppVoid = const_cast<void*>(m_vecStack.back());
	m_vecStack.pop_back();
	return true;
}

bool UT_Stack::viewTop(void ** ppVoid) const
{
	UT_ASSERT(ppVoid);
	UT_uint32 indexEnd = m_vecStack.size();
	if (!indexEnd) {
		*ppVoid = nullptr;
		return false;
	}
	*ppVoid = const_cast<void*>(m_vecStack.back());
	return true;
}

UT_sint32 UT_Stack::getDepth(void) const
{
	return m_vecStack.size();
}

UT_NumberStack::UT_NumberStack(UT_uint32 sizehint)
{
	//
	m_vecStack.reserve(sizehint);
}

void UT_NumberStack::push(UT_sint32 number)
{
	m_vecStack.push_back(number);
}

bool UT_NumberStack::pop(UT_sint32 * number)
{
	if (m_vecStack.empty()) {
		return false;
	}

	if (number) {
		*number = m_vecStack.back();
	}

	m_vecStack.pop_back();
	return true;
}

bool UT_NumberStack::viewTop(UT_sint32 & number) const
{
	if (m_vecStack.empty()) {
		return false;
	}

	number = m_vecStack.back();

	return true;
}

UT_sint32 UT_NumberStack::getDepth () const
{
	return m_vecStack.size();
}
