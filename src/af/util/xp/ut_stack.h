/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */ 


#ifndef UT_STACK_H
#define UT_STACK_H

#include "ut_types.h"
#include "ut_vector.h"

class UT_Stack
{
public:
	UT_Bool			push(void * pVoid);
	UT_Bool			pop(void ** ppVoid);
	UT_Bool			viewTop(void ** ppVoid) const;
	UT_uint32		getDepth(void) const;

protected:
	UT_Vector		m_vecStack;
};

#endif /* UT_STACK_H */
