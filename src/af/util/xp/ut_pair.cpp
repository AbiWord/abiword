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


#include <string.h>
#include "ut_pair.h"

#ifndef ABI_OPT_STL

UT_Pair::UT_Pair(const pair_type first, const pair_type second)
:	m_first(first),
	m_second(second)
{
}

UT_Pair::~UT_Pair()
{
}

#else /* ABI_OPT_STL */

UT_Pair::UT_Pair(const pair_type first, const pair_type second)
:	m_pair(first, second)
{
}

UT_Pair::~UT_Pair()
{
}

#endif /* ABI_OPT_STL */
