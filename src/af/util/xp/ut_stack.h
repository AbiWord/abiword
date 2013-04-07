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



#ifndef UT_STACK_H
#define UT_STACK_H

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif
#include "ut_vector.h"

class ABI_EXPORT UT_Stack
{
public:
	bool			push(void * pVoid);
	bool			pop(void ** ppVoid);
	bool			viewTop(void ** ppVoid) const;
	UT_sint32		getDepth(void) const;
	void            clear() {m_vecStack.clear();}

private:
	UT_Vector		m_vecStack;
};

/* some people have been storing numbers casted to pointers in a UT_Stack;
 * this causes problems sometimes on 64-bit architectures.
 *
 * so, here is a number stack
 */
class ABI_EXPORT UT_NumberStack
{
public:
	UT_NumberStack (UT_uint32 sizehint = 32, UT_uint32 baseincr = 32); // see UT_NumberVector

	bool			push (UT_sint32 number);
	bool			pop (UT_sint32 * number = 0);
	bool			viewTop (UT_sint32 & number) const;
	UT_sint32		getDepth (void) const;
	void            clear() {m_vecStack.clear();}

private:
	UT_NumberVector	m_vecStack;
};

#endif /* UT_STACK_H */
