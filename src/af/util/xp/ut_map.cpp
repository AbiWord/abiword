/* AbiWord
 * Copyright (C) 2001 Joaquín Cuenca Abela
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

#include "ut_map.h"
#include "ut_pair.h"
#include "ut_assert.h"

bool ut_map_lexico_lesser(UT_RBTree::key_t x, UT_RBTree::key_t y)
{
	UT_ASSERT(x);
	UT_ASSERT(y);
	
	return ut_lexico_lesser(static_cast<UT_Map::value_t> (x)->first(),
							static_cast<UT_Map::value_t> (y)->first());
}

bool ut_map_lexico_greater(UT_RBTree::key_t x, UT_RBTree::key_t y)
{
	UT_ASSERT(x);
	UT_ASSERT(y);
	
	return ut_lexico_greater(static_cast<UT_Map::value_t> (x)->first(),
							 static_cast<UT_Map::value_t> (y)->first());
}

bool ut_map_lexico_equal(UT_RBTree::key_t x, UT_RBTree::key_t y)
{
	UT_ASSERT(x);
	UT_ASSERT(y);
	
	return ut_lexico_equal(static_cast<UT_Map::value_t> (x)->first(),
						   static_cast<UT_Map::value_t> (y)->first());
}

///////////////////////////////////////////
// UT_Map
///////////////////////////////////////////
static bool lesser(UT_RBTree::key_t x, UT_RBTree::key_t y)
{
	UT_Map::value_t x_ = static_cast<UT_Map::value_t> (x);
	UT_Map::value_t y_ = static_cast<UT_Map::value_t> (y);
	UT_ASSERT(x_);
	UT_ASSERT(y_);
	
	return x_->first() < y_->first();
}

UT_Map::UT_Map(void)
	: m_rbtree(lesser)
{
}

UT_Map::UT_Map(comparator comp)
	: m_rbtree(comp)
{
}

UT_Map::~UT_Map(void)
{
	// that needs at least a comment...
	// The UT_Pair object pointed by each iterator was born as a non
	// const object, so the const_cast is (if not nice) at least deterministic.
	// You should not use something like that outside UT_Map unless you know
	// very well what are you doing.
	// Any operation on rbtree after that (except the destructor) has indeterminated
	// effects (with a bit of luck a segfault).
	Iterator end(m_rbtree.end());
	for (Iterator it(m_rbtree.begin()); it != end; ++it)
		delete const_cast<UT_Pair*> (static_cast<const UT_Pair*> (it.value()));
}

bool
UT_Map::insert(key_t key, data_t data)
{
	return m_rbtree.insert(new UT_Pair(static_cast<pair_type> (key), static_cast<pair_type> (data)));
}

void
UT_Map::erase(key_t key)
{
	Iterator it(m_rbtree.find(key));

	if (it.is_valid())
		erase(it);
}

UT_Map::Iterator
UT_Map::find(key_t key)
{
	UT_Pair* tmp = new UT_Pair(static_cast<pair_type> (key), static_cast<pair_type> (data_t()));
	Iterator retval(m_rbtree.find(tmp));
	delete tmp;
	return retval;
}

UT_Map::Iterator
UT_Map::find_if(key_t key, comparator pred)
{
	UT_Pair* tmp = new UT_Pair(static_cast<pair_type> (key), static_cast<pair_type> (data_t()));
	Iterator retval(m_rbtree.find_if(tmp, pred));
	delete tmp;
	return retval;
}
