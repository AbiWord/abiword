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
	UT_Map::value_t x_ = static_cast<UT_Map::value_t> (x);
	UT_Map::value_t y_ = static_cast<UT_Map::value_t> (y);
	UT_ASSERT(x_);
	UT_ASSERT(y_);
	
	return ut_lexico_lesser(x_->first(), y_->first());
}

bool ut_map_lexico_greater(UT_RBTree::key_t x, UT_RBTree::key_t y)
{
	UT_Map::value_t x_ = static_cast<UT_Map::value_t> (x);
	UT_Map::value_t y_ = static_cast<UT_Map::value_t> (y);
	UT_ASSERT(x_);
	UT_ASSERT(y_);
	
	return ut_lexico_greater(x_->first(), y_->first());
}

bool ut_map_lexico_equal(UT_RBTree::key_t x, UT_RBTree::key_t y)
{
	UT_Map::value_t x_ = static_cast<UT_Map::value_t> (x);
	UT_Map::value_t y_ = static_cast<UT_Map::value_t> (y);
	UT_ASSERT(x_);
	UT_ASSERT(y_);
	
	return ut_lexico_equal(x_->first(), y_->first());
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

UT_Map::UT_Map()
	: m_rbtree(lesser)
{
}

UT_Map::UT_Map(comparator comp)
	: m_rbtree(comp)
{
}
