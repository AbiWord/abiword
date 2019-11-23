/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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

#pragma once

#include <vector>

#include "ut_types.h"

class ABI_EXPORT UT_Stack
{
public:
	void			push(void * pVoid);
	bool			pop(void ** ppVoid);
	bool			viewTop(void ** ppVoid) const;
	UT_sint32		getDepth(void) const;
	void            clear() {m_vecStack.clear();}

private:
	std::vector<const void*>		m_vecStack;
};

/** A stack to store numbers
 */
class ABI_EXPORT UT_NumberStack
{
public:
	UT_NumberStack (UT_uint32 sizehint = 32);

	void			push (UT_sint32 number);
	bool			pop (UT_sint32 * number = nullptr);
	bool			viewTop (UT_sint32 & number) const;
	UT_sint32		getDepth (void) const;
	void            clear() {m_vecStack.clear();}

private:
	std::vector<UT_sint32>	m_vecStack;
};
