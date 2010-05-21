/* AbiWord
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */


#ifndef PF_FRAGMENTS_H
#define PF_FRAGMENTS_H

/*!
 pf_Fragments is a container for all of the pf_Frag
 derrived objects.  pf_Fragments provides a searchable,
 efficient ordering on the document fragments.

 Currently this consists of a simple doubly-linked list.
 We may need to add a tree structure on top of it, if we
 need to do various types of searches.
*/

#include <stdio.h>
#include "ut_vector.h"
#include "ut_types.h"
class pf_Frag;

class ABI_EXPORT pf_Fragments
{
  friend class pf_Frag;
public:
	pf_Fragments();
	~pf_Fragments();

	void					appendFrag(pf_Frag * pf);
	void					insertFrag(pf_Frag * pfPlace, pf_Frag * pfNew);
	void					insertFragBefore(pf_Frag * pfPlace, pf_Frag * pfNew);
	void					unlinkFrag(pf_Frag * pf);
	pf_Frag *               findFirstFragBeforePos(PT_DocPosition pos) const;

	pf_Frag *				getFirst() const;
	pf_Frag *				getLast() const;
	void                                    verifyDoc(void) const;
#ifdef PT_TEST
	void					__dump(FILE * fp) const;
#endif

	class  Node
	{
	public:
	  enum Color { red, black };
	  Node(void);
	  Node(Color c);
	  Node(Color c, pf_Frag * pf, Node * l, Node * r, Node * p);
	  ~Node(void);
	  Color color;
	  pf_Frag* item;
	  Node* left;
	  Node* right;
	  Node* parent;

#ifdef DEBUG
	  void         print(void);
#endif
	};

	
	class Iterator
	{
	public:
		Iterator() : m_pOwner(NULL), m_pNode(NULL) {}
		
		Iterator& operator++()
		{
			m_pNode = const_cast<Node*>(m_pOwner->_next(m_pNode));
			return *this;
		}

		Iterator operator++(int)
		{
			Iterator tmp(*this);
			m_pNode = const_cast<Node*>(m_pOwner->_next(m_pNode));
			return tmp;
		}

		Iterator& operator--()
		{
			m_pNode = const_cast<Node*>(m_pOwner->_prev(m_pNode));
			return *this;
		}

		Iterator operator--(int)
		{
			Iterator tmp(*this);
			m_pNode = const_cast<Node*>(m_pOwner->_prev(m_pNode));
			return tmp;
		}

		bool operator==(const Iterator other)
		{ return (m_pOwner == other.m_pOwner && m_pNode == other.m_pNode); }

		bool operator!=(const Iterator other)
		{ return !(*this == other); }

		const pf_Frag* value() const;
		pf_Frag* value();
		
		bool is_valid() const
		{ return m_pNode != 0; }

	private:
		Iterator(const pf_Fragments* owner, Node* node = 0) : m_pOwner(owner), m_pNode(node) {}
		const Node* getNode() const { return m_pNode; }
		Node* getNode() { return m_pNode; }

		const pf_Fragments* m_pOwner;
		Node* m_pNode;
		friend class pf_Fragments;
		friend class pf_Frag;
	};

	Iterator find(PT_DocPosition pos) const; // throws ()


private:
	inline pf_Frag*			getCache() const { return m_pCache; }
	inline void				setCache(pf_Frag* pf) const { m_pCache = pf; }

	pf_Frag *				m_pFirst;
	pf_Frag *				m_pLast;
	mutable pf_Frag*		m_pCache;

	Iterator insertRoot(pf_Frag* new_piece); // throws std::bad_alloc (strong)
	Iterator insertLeft(pf_Frag* new_piece, Iterator it); // throws std::bad_alloc (strong)
	Iterator insertRight(pf_Frag* new_piece, Iterator it); // throws std::bad_alloc (strong)
	void erase(Iterator it); // throws ()
	void fixSize(Iterator it); // throws ()
	PT_DocPosition documentPosition(const Iterator it) const; // throws ()
	void changeSize(int delta); // throws ()

	Iterator begin() { return Iterator(this, _first()); }
	Iterator end() { return Iterator(this); }
	
	size_t size() const { return m_nSize; }
	PT_DocPosition sizeDocument() const { return m_nDocumentSize; }

#ifdef DEBUG
	void print() const;
	bool checkInvariants() const;
	bool checkSizeInvariant(const Node* pn, PT_DocPosition* size) const;
#endif

	void _insertBST(Node* pn);
	void _insertFixup(Node* pn);
	void _eraseFixup(Node* pn);
	void _leftRotate(Node* x);
	void _rightRotate(Node* x);
	PT_DocPosition _calculateSize(Node* x);
	
#ifdef DEBUG
	int _countBlackNodes(const Iterator it) const;
#endif

	void delete_tree(Node* node);

	const Node* _next(const Node* pn) const;
	const Node* _prev(const Node* pn) const;
	const Node* _first() const;
	const Node* _last() const;

	Node* _next(Node* pn);
	Node* _prev(Node* pn);
	Node* _first();
	Node* _last();

	Node* m_pLeaf;
	Node* m_pRoot;
	size_t m_nSize;
	PT_DocPosition m_nDocumentSize;

	friend class Iterator;

};

#endif /* PF_FRAGMENTS_H */
