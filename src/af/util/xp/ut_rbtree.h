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

#ifndef UT_RBTREE_H
#define UT_RBTREE_H

#include <stddef.h>
#ifdef DEBUG
#include <stdio.h>
#endif

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

class ABI_EXPORT UT_RBTree
{
	struct Node;

public:
	typedef const void* key_t;
	typedef bool (*comparator) (key_t, key_t);

	class Iterator
	{
	public:
		Iterator& operator++()
			{ m_pNode = m_pOwner->_next(m_pNode); return *this; }

		Iterator operator++(int)
		{
			Iterator tmp(*this);
			m_pNode = m_pOwner->_next(m_pNode);
			return tmp;
		}

		Iterator& operator--()
			{ m_pNode = m_pOwner->_prev(m_pNode); return *this; }

		Iterator operator--(int)
		{
			Iterator tmp(*this);
			m_pNode = m_pOwner->_prev(m_pNode);
			return tmp;
		}

		bool operator==(const Iterator& other)
			{ return (m_pOwner == other.m_pOwner && m_pNode == other.m_pNode); }

		bool operator!=(const Iterator& other)
			{ return !(*this == other); }

		key_t value() const;

		bool is_valid() const { return m_pNode != 0; }

	private:
		Iterator(UT_RBTree* owner, Node* node = 0) : m_pOwner(owner), m_pNode(node) {}
		Node* getNode() { return m_pNode; }
		const Node* getNode() const { return m_pNode; }

		UT_RBTree* m_pOwner;
		Node* m_pNode;
		friend class UT_RBTree;
	};

	UT_RBTree(comparator comp);
	~UT_RBTree();

	bool insert(key_t item);
	void erase(Iterator& c);

	Iterator find(key_t item);
	Iterator find_if(key_t item, comparator pred);

	Iterator begin() { return Iterator(this, _first()); }
	Iterator end() { return Iterator(this); }
//	Iterator rbegin() { return Iterator(this, _last()); }
//	Iterator rend() { return Iterator(this); }
	size_t size() { return m_nSize; }

#ifdef DEBUG
	void print() const;
	// TODO: This method is const, but I don't have (yet)
	// TODO: a ConstIterator
	bool checkInvariants();
#endif

private:
	UT_RBTree(const UT_RBTree&);
	UT_RBTree& operator= (const UT_RBTree&);

	void _insertBST(Node* pn);
	void _insertFixup(Node* pn);
	void _eraseFixup(Node* pn);
	void _leftRotate(Node* x);
	void _rightRotate(Node* x);

#ifdef DEBUG
	// TODO: This method is const, but I don't have (yet)
	// TODO: a ConstIterator
	int _countBlackNodes(const Iterator& it);
#endif

	static void s_delete_tree(Node* node);

	const Node* _next(const Node* pn) const;
	const Node* _prev(const Node* pn) const;
	const Node* _first() const;
	const Node* _last() const;

	Node* _next(Node* pn);
	Node* _prev(Node* pn);
	Node* _first();
	Node* _last();
	static Node* getLeaf();

	Node* m_pRoot;
	comparator m_comp;
	size_t m_nSize;
	friend class Iterator;
};

bool ut_lexico_lesser(UT_RBTree::key_t x, UT_RBTree::key_t y);
bool ut_lexico_greater(UT_RBTree::key_t x, UT_RBTree::key_t y);
bool ut_lexico_equal(UT_RBTree::key_t x, UT_RBTree::key_t y);

#endif // UT_RBTREE_H
