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

typedef void* pair_type;

#ifndef ABI_OPT_STL

class UT_Pair
{
public:
	UT_Pair(const pair_type first, const pair_type second);
	~UT_Pair();

	const pair_type& first() const  { return m_first; }
	const pair_type& second() const { return m_second; }
	pair_type& first()  { return m_first; }
	pair_type& second() { return m_second; }

private:
	pair_type m_first;
	pair_type m_second;
};

#else /* ABI_OPT_STL */

#include <utility>

class UT_Pair : public XAP_AbiObject
{
public:
	UT_Pair(const pair_type first, const pair_type second);
	~UT_Pair();

	const pair_type& first()  const { m_pair.first; }
	const pair_type& second() const { m_pair.second; }
	pair_type& first()  { return m_pair.first; }
	pair_type& second() { return m_pair.second; }

private:
	std::pair<pair_type, pair_type>	m_pair;
};


#endif /* ABI_OPT_STL */

#endif
