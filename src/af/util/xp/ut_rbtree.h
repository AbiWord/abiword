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

class UT_RBTree
{
	struct Node;

public:
	typedef const void* key_t;
	typedef bool (*comparator) (key_t, key_t);

	class Iterator
	{
	public:
		inline Iterator& operator++()
			{ m_pNode = m_pOwner->_next(m_pNode); return *this; }
		inline Iterator operator++(int)
		{
			Iterator tmp(*this);
			m_pNode = m_pOwner->_next(m_pNode);
			return tmp;
		}
		inline Iterator& operator--()
			{ m_pNode = m_pOwner->_prev(m_pNode); return *this; }
		inline Iterator operator--(int)
		{
			Iterator tmp(*this);
			m_pNode = m_pOwner->_prev(m_pNode);
			return tmp;
		}
		inline bool operator==(const Iterator& other)
			{ return (m_pOwner == other.m_pOwner && m_pNode == other.m_pNode); }
		inline bool operator!=(const Iterator& other)
			{ return !(*this == other); }
		key_t value() const;
		inline bool is_valid() const { return m_pNode != 0; }

	private:
		Iterator(UT_RBTree* owner, Node* node = 0) : m_pOwner(owner), m_pNode(node) {}
		inline Node* getNode() { return m_pNode; }
		inline const Node* getNode() const { return m_pNode; }

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

	inline Iterator begin() { return Iterator(this, _first()); }
	inline Iterator end() { return Iterator(this); }
	inline Iterator rbegin() { return Iterator(this, _last()); }
	inline Iterator rend() { return Iterator(this); }
	inline size_t size() { return m_nSize; }

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

	Node* m_pRoot;
	comparator m_comp;
	size_t m_nSize;
	friend class Iterator;
};

bool ut_lexico_lesser(UT_RBTree::key_t x, UT_RBTree::key_t y);
bool ut_lexico_greater(UT_RBTree::key_t x, UT_RBTree::key_t y);
bool ut_lexico_equal(UT_RBTree::key_t x, UT_RBTree::key_t y);

#endif // UT_RBTREE_H
