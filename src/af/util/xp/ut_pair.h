/* AbiSource Program Utilities
 *
 * Copyright (C) 2001 AbiSource, Inc.
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
#ifndef UT_PAIR_H
#define UT_PAIR_H

#include <stdlib.h>

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif


template <class T, class U> class  ABI_EXPORT UT_Pair
{
public:
	UT_Pair(T first, U second)
		:	m_first(first),
			m_second(second) {}
	virtual ~UT_Pair()
		{ }

	const T first() const  
		{ return m_first; }
	const U second() const 
		{ return m_second; }

private:
	const T m_first;
	const U m_second;
};


#endif
