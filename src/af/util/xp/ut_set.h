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

#ifndef UT_SET_H
#define UT_SET_H

#include "ut_rbtree.h"

class UT_Set
{
public:
	typedef UT_RBTree::key_t key_t;
	typedef UT_RBTree::Iterator Iterator;
	typedef UT_RBTree::comparator comparator;

	UT_Set();
	UT_Set(comparator comp);

	inline bool insert(key_t item) { return m_rbtree.insert(item); }
	inline void erase(Iterator& c) { m_rbtree.erase(c); }

	inline UT_Set::Iterator find(key_t item) { return m_rbtree.find(item); }
	inline UT_Set::Iterator find_if(key_t item, comparator pred) { return m_rbtree.find_if(item, pred); }

	inline UT_Set::Iterator begin() { return m_rbtree.begin(); }
	inline UT_Set::Iterator end() { return m_rbtree.end(); }
	inline UT_Set::Iterator rbegin() { return m_rbtree.rbegin(); }
	inline UT_Set::Iterator rend() { return m_rbtree.rend(); }
	inline size_t size() { return m_rbtree.size(); }

#ifdef DEBUG
	inline void print() const { m_rbtree.print(); }
#endif

private:
	UT_Set(const UT_Set&);
	UT_Set& operator= (const UT_Set&);

	UT_RBTree m_rbtree;
};

#endif // UT_SET_H
