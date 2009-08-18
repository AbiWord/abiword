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


#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "pf_Frag.h"
#include "pf_Fragments.h"
#include "ut_debugmsg.h"
#include "pf_Frag_Strux.h"

///////////////////////////////////////////
// Node
///////////////////////////////////////////


#ifdef DEBUG
void pf_Fragments::Node::print()
{
  if (!item)
    return;
				
  printf("%c (%i, %i) \n", color == red ? 'R' : 'B', item->m_leftTreeLength, item->m_length);

  if (left && left->item)
  {
      printf("--- left tree:");
      left->print();
  }

  if (right && right->item)
  {
      printf("--- right tree:");
      right->print();
  }
}
#endif

//////////////////////////////////////////////
// End Node Defn.
//////////////////////////////////////////////

pf_Fragments::pf_Fragments()
	: m_pFirst(0),
	  m_pLast(0),
	  m_bAreFragsClean(false),
	  m_pCache(0),
	  m_pLeaf(new Node(Node::black)),
	  m_pRoot(m_pLeaf),
	  m_nSize(0),
	  m_nDocumentSize(0)
	  
{
}

pf_Fragments::~pf_Fragments()
{
        if (m_pRoot != m_pLeaf)
		delete_tree(m_pRoot);

	delete m_pLeaf;

	//Comment this out later.
	while (m_pFirst)
	{
		pf_Frag* pNext = m_pFirst->getNext();
		delete m_pFirst;
		m_pFirst = pNext;
	}
	
	m_pLast = NULL;
}

void pf_Fragments::appendFrag(pf_Frag * pf)
{
	// append a frag to the end of the list
	
	UT_return_if_fail (pf);

	if ( m_pRoot == m_pLeaf ) //If tree is empty.
	{
		insertRoot(pf);	
	}
	else 
	{
		//Since this fragment is at the end of the document,
		//we find the last piece and insert it to its right.
		Iterator lastIt = find(sizeDocument()-1);
		insertRight(pf, lastIt);
	}

	return;
}

/*
 * This function has O(log(n)) time complexity.
 */ 
pf_Frag * pf_Fragments::getFirst() const
{
	return find( 0 ).value();
}


/*
 * This function has O(log(n)) time complexity.
 */ 
pf_Frag * pf_Fragments::getLast() const
{
	return find( sizeDocument() - 1 ).value();
}

void pf_Fragments::insertFrag(pf_Frag * pfPlace, pf_Frag * pfNew)
{
	// insert the new fragment after the given fragment.
	UT_return_if_fail (pfPlace);
	UT_return_if_fail (pfNew);
	UT_return_if_fail (pfPlace->_getNode());

	xxx_UT_DEBUGMSG(("Inserting frag %x of type %d after frag %x of type %d\n",pfNew,pfNew->getType(),pfPlace,pfPlace->getType()));

	Iterator it(this,pfPlace->_getNode());
	insertRight(pfNew, it);
}

void pf_Fragments::insertFragBefore(pf_Frag * pfPlace, pf_Frag * pfNew)
{
	// insert the new fragment after the given fragment.
	UT_return_if_fail (pfPlace);
	UT_return_if_fail (pfNew);
	UT_return_if_fail (pfPlace->_getNode());

	xxx_UT_DEBUGMSG(("Inserting frag %x of type %d after frag %x of type %d\n",pfNew,pfNew->getType(),pfPlace,pfPlace->getType()));

	pfNew->setPos(pfPlace->getPos() - 1);
	Iterator it(this,pfPlace->_getNode());
	insertLeft(pfNew, it);
}

void pf_Fragments::unlinkFrag(pf_Frag * pf)
{
	// NOTE:  it is the caller's responsibility to delete pf if appropriate.
	
	UT_return_if_fail (pf->getType() != pf_Frag::PFT_EndOfDoc);

	Iterator it = find(pf->getPos());
	erase(it);
}

/*!
 * This method clears out and repopulates the vector of pointers to fragments.
 * It also sets the doc Positions of all the fragments.
 */
void pf_Fragments::cleanFrags(void) const
{
	/*
	 * ADITYA: In the new implementation with an rb-tree, this operation
	 * need not do anything. Thus we return immediately.
	 */
	return;

	if (m_vecFrags.getItemCount() > 0)
		m_vecFrags.clear();

	pf_Frag * pfLast = NULL;
	PT_DocPosition sum = 0;
	for (pf_Frag * pf = getFirst(); (pf); pf=pf->getNext())
	{
		pf->setPos(sum);
		sum += pf->getLength();
		pfLast = pf;
		m_vecFrags.addItem((void *) pf);
	}
	
	UT_return_if_fail (pfLast /*&& (pfLast->getType() == pf_Frag::PFT_EndOfDoc)*/);
	xxx_UT_DEBUGMSG(("Found %d Frags dopos at end = %d \n",m_vecFrags.getItemCount(),getLast()->getPos()));
	m_bAreFragsClean = true;
	setCache(NULL);
}

pf_Frag * pf_Fragments::getNthFrag(UT_uint32 nthFrag) const
{
	if (areFragsDirty())
	{
		xxx_UT_DEBUGMSG(("JCA: getNthFrag (%d): Cleanning fragments ( O(n) complexity! )\n", nthFrag));
		cleanFrags();
	}
	else 
	{
		xxx_UT_DEBUGMSG(("JCA: getNthFrag (%d): Don't need to clean fragments\n", nthFrag));
	}
	
	if (m_vecFrags.getItemCount() > 0)
	{
		xxx_UT_DEBUGMSG(("JCA: getNthFrag (%d): returning frag %p\n", nthFrag, m_vecFrags.getNthItem(nthFrag)));
		return (pf_Frag *) m_vecFrags.getNthItem(nthFrag);
	}

	return NULL;
}


/*!
 * Search the rb-tree to find the first frag before pos.
 * @param PT_DocPosition we want to find for.
 * @returns pf_Frag * pointer to the Frag with position immediately before pos
*/
pf_Frag * pf_Fragments::findFirstFragBeforePos(PT_DocPosition pos) const
{       
	if (pos >= sizeDocument())
		return NULL;

	Iterator it = find(pos);
	return it.value();
}


UT_uint32 pf_Fragments::getFragNumber(const pf_Frag * pf) const
{
	if (areFragsDirty())
		cleanFrags();

	return m_vecFrags.findItem((void *) pf);
}

UT_uint32 pf_Fragments::getNumberOfFrags() const
{
	if (areFragsDirty())
	{
		cleanFrags();
	}
	return m_vecFrags.getItemCount();
}


/* Methods that implement the rb-tree */



///////////////////////////////////////////
// Iterator
///////////////////////////////////////////

pf_Frag*
pf_Fragments::Iterator::value()
{
	return m_pNode->item;
}

///////////////////////////////////////////
// pf_Fragments
///////////////////////////////////////////


#if 0
void
pf_Fragments::_insertBST(Node* pNewNode, PT_DocPosition pos)
{
	Node* pNode = m_pRoot;
	
	UT_ASSERT(pNewNode);

	while (pNode != m_pLeaf)
	{
		if (m_comp(pNewNode->item, pNode->item))
		{
			if (pNode->left != m_pLeaf)
				pNode = pNode->left;
			else
			{
				pNewNode->parent = pNode;
				pNode->left = pNewNode;
				break;
			}
		}
		else
		{
			if (pNode->right != m_pLeaf)
				pNode = pNode->right;
			else
			{
				pNewNode->parent = pNode;
				pNode->right = pNewNode;
				break;
			}
		}
	}

	if (pNode == m_pLeaf)
	{
		m_pRoot = pNewNode;
		m_pRoot->color = Node::black;
	}
}
#endif

void
pf_Fragments::_insertFixup(Node* x)
{
	Node* y;
	UT_ASSERT(x);

	Iterator it(this, x);
	fixSize(it);
	
	while ((x != m_pRoot) && (x->parent->color == Node::red))
	{
		if (x->parent == x->parent->parent->left)
		{
			/* If x's parent is a left, y is x's right 'uncle' */
			y = x->parent->parent->right;
		    if (y && y->color == Node::red)
			{
				/*       bz          rz
				 *       / \         / \
				 *     rw  ry  =>  bw  by
				 *     /           /
				 *   rx          rx
				 */
				/* case 1 - change the colours */
				x->parent->color = Node::black;
				y->color = Node::black;
				x->parent->parent->color = Node::red;
				/* Move x up the tree */
				x = x->parent->parent;
			}
			else
			{
				/* y is a black node */
				if (x == x->parent->right)
				{
					/*       bz            bz
					 *       / \           / \
					 *     rw  by  =>    rx  by
					 *     / \           / \
					 *   b1  rx        rw  b3
					 *       / \       / \
 					 *     b2   b3   b1  b2
					 */
					/* and x is to the right */ 
					/* case 2 - move x up and rotate */
					x = x->parent;
					_leftRotate(x);
				}
				/* case 3 */
				x->parent->color = Node::black;
				x->parent->parent->color = Node::red;
				_rightRotate(x->parent->parent);
			}
		}
		else
		{
			/* If x's parent is a left, y is x's right 'uncle' */
			y = x->parent->parent->left;
			if (y && y->color == Node::red)
			{
				/* case 1 - change the colors */
				x->parent->color = Node::black;
				y->color = Node::black;
				x->parent->parent->color = Node::red;
				/* Move x up the tree */
				x = x->parent->parent;
			}
			else
			{
				/* y is a black node */
				if (x == x->parent->left)
				{
					/* and x is to the left */ 
					/* case 2 - move x up and rotate */
					x = x->parent;
					_rightRotate(x);
				}
				/* case 3 */
				x->parent->color = Node::black;
				x->parent->parent->color = Node::red;
				_leftRotate(x->parent->parent);
			}
		}
	}

	/* Colour the root black */
	m_pRoot->color = Node::black;
}

#if 0
Iterator
pf_Fragments::insert(pf_Frag* new_piece)
{
	Node* pNode = it.getNode();
	Node* pNewNode = new Node(Node::red, new_piece, m_pLeaf, m_pLeaf, 0);

	++m_nSize;
	_insertBST(pNewNode);
	_insertFixup(pNewNode);

	return Iterator(this, pNewNode);
}
#endif

/**
 * Insert a new piece as the root of the tree.  The tree should be empty before performing
 * this operation.
 *
 * @param new_piece the piece to become root
 */
pf_Fragments::Iterator
pf_Fragments::insertRoot(pf_Frag* new_piece)
{
	Iterator it_null(this, 0);
	return insertRight(new_piece, it_null);
}

/**
 * Insert a new piece to the right of a given node.
 * This operation has the exceptional guarantee strong.
 *
 * @param new_piece the piece to insert
 * @param it iterator to the reference node
 */
pf_Fragments::Iterator
pf_Fragments::insertRight(pf_Frag* new_piece, Iterator& it)
{
	Node* pNode = it.getNode();

	UT_ASSERT(m_nSize == 0 || it.is_valid());
	
	Node* pNewNode = new Node(Node::red, new_piece, m_pLeaf, m_pLeaf, 0);

	new_piece->setLeftTreeLength(0);
	
	++m_nSize;
	m_nDocumentSize += new_piece->getLength();

	if (!it.is_valid())
		m_pRoot = pNewNode;
	else if (pNode->right == m_pLeaf)
	{
		pNode->right = pNewNode;
		pNewNode->parent = pNode;
	}
	else
	{
		++it;
		pNode = it.getNode();

		UT_ASSERT(it.is_valid());
		UT_ASSERT(pNode->left == m_pLeaf);

		pNode->left = pNewNode;
		pNewNode->parent = pNode;
	}

	_insertFixup(pNewNode);
	new_piece->_setNode(pNewNode);
	return Iterator(this, pNewNode);
}

/**
 * Insert a new piece to the left of a given node.
 * This operation has the exceptional guarantee strong.
 *
 * @param new_piece the piece to insert
 * @param it iterator to the reference node
 */
pf_Fragments::Iterator
pf_Fragments::insertLeft(pf_Frag* new_piece, Iterator& it)
{
	Node* pNode = it.getNode();

	UT_ASSERT(m_nSize == 0 || it.is_valid());
	
	Node* pNewNode = new Node(Node::red, new_piece, m_pLeaf, m_pLeaf, 0);

	new_piece->setLeftTreeLength(0);
	
	++m_nSize;
	m_nDocumentSize += new_piece->getLength();

	if (!it.is_valid())
		m_pRoot = pNewNode;
	else if (pNode->left == m_pLeaf)
	{
		pNode->left = pNewNode;
		pNewNode->parent = pNode;
	}
	else
	{
		--it;
		pNode = it.getNode();

		UT_ASSERT(it.is_valid());
		UT_ASSERT(pNode->right == m_pLeaf);

		pNode->right = pNewNode;
		pNewNode->parent = pNode;
	}

	_insertFixup(pNewNode);
	new_piece->_setNode(pNewNode);
	return Iterator(this, pNewNode);
}

/**
 * Erase a piece from the tree.
 * This operation has the exceptional guarantee "no throw".
 *
 * @param it iterator to the node to erase.
 */
void
pf_Fragments::erase(Iterator& it)
{
	if (!it.is_valid())
		return;

	Node* pNode = it.getNode();
	UT_ASSERT(pNode);

	--m_nSize;
	m_nDocumentSize -= pNode->item->getLength();
	
	Node* y = (pNode->left == m_pLeaf || pNode->right == m_pLeaf) ?
		pNode :
		_prev(pNode);

	UT_ASSERT(y->left == m_pLeaf || y->right == m_pLeaf);
	Node* son = y->left != m_pLeaf ? y->left : y->right;
	UT_ASSERT(son);
	son->parent = y->parent;

	if (!y->parent)
		m_pRoot = son;
	else
		if (y->parent->left == y)
			y->parent->left = son;
		else
			y->parent->right = son;

	if (y != pNode)
	{
		/* ADITYA: To conform with the existing interface, we
		  do not delete the pf_Frag object - it is responsibility of
		  the caller of unlinkFrag().
		  
		  Thus, the line below is commented out.
		*/
		//delete pNode->item;
		pNode->item = y->item;
		Iterator it_temp(this, pNode);
		fixSize(it_temp);
	}

	if (son->parent)
	{
		/* and fix the whole left branch of the tree */
		Iterator it_temp(this, son);
		fixSize(it_temp);
	}
		
	if (y->color == Node::black)
		_eraseFixup(son);

	delete y;
}

/**
 * If the size of a tree's node changed, we should update the m_lengthLeft field
 * of all its right parents.  This function does that fix.  You should pass
 * an iterator to the node that changed its size.  It assumes that the sizes
 * of the tree are all right except for a change in the size of the node passed
 * as argument.
 *
 * @param it Iterator to the node that changed its size
 */
void
pf_Fragments::fixSize(Iterator& it)
{
	UT_ASSERT(it.is_valid());
	Node* pn = it.getNode();
	pf_Frag* item;
	int delta = 0;
   
	if (pn == m_pRoot)
		return;

	/* if the two sons of the left son are the same one, then they are
	 * two leafs, and maybe we've gone one node too high when we're climbing
	 * the tree (because pn was == pn->parent->right, but the change was in
	 * pn->parent->left).  We can't thus trust the return value of _calculateSize,
	 * and we should do this calculation at hand (it's just pn->left->item->m_length) */
	if (pn->parent->left == pn->parent->right && pn->parent->item)
	{
		pn = pn->parent;
		delta = - (int) pn->item->getLeftTreeLength();
		pn->item->getLeftTreeLength();
	}

	if (delta == 0)
	{
		/* go up to the head of the first left subtree that contains the node that changed. */
		while (pn != m_pRoot && pn == pn->parent->right)
			pn = pn->parent;
	
		if (pn == m_pRoot)
			return;

		pn = pn->parent;
	
		/* Fix the m_lengthLeft of this head */
		delta = _calculateSize(pn->left) - pn->item->getLeftTreeLength();
		pn->item->accLeftTreeLength( delta );
	}

	/* if the m_lengthLeft of that head changed, propagate the change to our parents */
	while (pn != m_pRoot && delta != 0)
	{
		item = pn->item;
	
		if (pn->parent->left == pn)
			pn->parent->item->accLeftTreeLength(delta);

		pn = pn->parent;
	}
}

void
pf_Fragments::_eraseFixup(Node* x)
{
	while (x != m_pRoot && x->color == Node::black)
	{
		if (x == x->parent->left)
		{
			Node *tmp = x->parent->right;
			if (tmp->color == Node::red)
			{
				tmp->color = Node::black;
				x->parent->color = Node::red;
				_leftRotate(x->parent);
				tmp = x->parent->right;
			}
			if (tmp->left->color == Node::black && tmp->right->color == Node::black)
			{
				tmp->color = Node::red;
				x = x->parent;
			}
			else
			{
				if (tmp->right->color == Node::black)
				{
					tmp->left->color = Node::black;
					tmp->color = Node::red;
					_rightRotate(tmp);
					tmp = x->parent->right;
				}
				tmp->color = x->parent->color;
				x->parent->color = Node::black;
				tmp->right->color = Node::black;
				_leftRotate(x->parent);
				x = m_pRoot;
			}
		}
		else
		{
			Node* tmp = x->parent->left;
			if (tmp->color == Node::red)
			{
				tmp->color = Node::black;
				x->parent->color = Node::red;
				_rightRotate(x->parent);
				tmp = x->parent->left;
			}
			if (tmp->right->color == Node::black && tmp->left->color == Node::black)
			{
				tmp->color = Node::red;
				x = x->parent;
            }
			else
			{
				if (tmp->left->color == Node::black)
				{
					tmp->right->color = Node::black;
					tmp->color = Node::red;
					_leftRotate(tmp);
					tmp = x->parent->left;
				}
				tmp->color = x->parent->color;
				x->parent->color = Node::black;
				tmp->left->color = Node::black;
				_rightRotate(x->parent);
				x = m_pRoot;
			}
		}
	}
	x->color = Node::black;
}

pf_Fragments::Iterator
pf_Fragments::find(PT_DocPosition pos) const
{
	Node* x = m_pRoot;

	while (x != m_pLeaf)
	{
		pf_Frag* p = x->item;
		
		if (p->getLeftTreeLength() > pos)
			x = x->left;
		else if (p->getLeftTreeLength() + p->getLength() > pos)
			return Iterator(this, x);
		else
		{
			pos -= p->getLeftTreeLength() + p->getLength();
			x = x->right;
		}
	}

	UT_ASSERT(pos >= sizeDocument());
	return Iterator(this, 0);
}

PT_DocPosition
pf_Fragments::documentPosition(const Iterator& it) const
{
	UT_ASSERT(it.is_valid());

	const Node* pn = it.getNode();
	PT_DocPosition pos = pn->item->getLeftTreeLength();
	while (pn != m_pRoot)
	{
		if (pn->parent->right == pn)
			pos += pn->parent->item->getLeftTreeLength() + pn->parent->item->getLength();

		pn = pn->parent;
	}

	return pos;
}

void
pf_Fragments::changeSize(int delta)
{
	UT_ASSERT(delta < 0 ? ((int) m_nDocumentSize >= -delta) : true);

	m_nDocumentSize += delta;
}

const pf_Fragments::Node*
pf_Fragments::_prev(const Node* pn) const
{
	UT_ASSERT(pn != NULL);

	if (pn != m_pLeaf)
	{
		if (pn->left != m_pLeaf)
		{
			pn = pn->left;

			while(pn->right != m_pLeaf)
				pn = pn->right;
		}
		else
		{
			while(pn->parent)
			{
				if (pn->parent->right == pn)
					return pn->parent;
				else
					pn = pn->parent;
			}

			return 0;
		}
	}

	return pn;
}

pf_Fragments::Node*
pf_Fragments::_prev(Node* pn)
{
	return const_cast<Node*> (_prev(const_cast<const Node*> (pn)));
}

const pf_Fragments::Node*
pf_Fragments::_next(const Node* pn) const
{
	UT_ASSERT(pn != NULL);

	if (pn != m_pLeaf)
	{
		if (pn->right != m_pLeaf)
		{
			pn = pn->right;

			while(pn->left != m_pLeaf)
				pn = pn->left;
		}
		else
		{
			while(pn->parent)
			{
				if (pn->parent->left == pn)
					return pn->parent;
				else
					pn = pn->parent;
			}

			return 0;
		}
	}

	return pn;
}

pf_Fragments::Node*
pf_Fragments::_next(Node* pn)
{
	return const_cast<Node*> (_next(const_cast<const Node*> (pn)));
}

const pf_Fragments::Node*
pf_Fragments::_first() const
{
	Node* pn = m_pRoot;
	
	if (pn == m_pLeaf)
		return 0;

	while(pn->left != m_pLeaf)
		pn = pn->left;

	return pn;
}

pf_Fragments::Node*
pf_Fragments::_first()
{
	Node* pn = m_pRoot;
	
	if (pn == m_pLeaf)
		return 0;

	while(pn->left != m_pLeaf)
		pn = pn->left;

	return pn;
}

const pf_Fragments::Node*
pf_Fragments::_last() const
{
	Node* pn = m_pRoot;
	
	if (pn == m_pLeaf)
		return 0;

	while(pn->right != m_pLeaf)
		pn = pn->right;

	return pn;
}

pf_Fragments::Node*
pf_Fragments::_last()
{
	Node* pn = m_pRoot;
	
	if (pn == m_pLeaf)
		return 0;

	while(pn->right != m_pLeaf)
		pn = pn->right;

	return pn;
}

/**
 * This method rotates a tree to the left.
 *
 * Let be 2 & 4 nodes of a tree.  Let be 1, 3, 5 subtrees.
 * This operation performs the rotation that appears in the
 * this ASCII picture:
 *
 *        2                4
 *       / \              / \
 *      1   4     =>     2   5
 *         / \          / \
 *        3   5        1   3
 *
 * @params x is the head of the tree (node 2 in the picture).
 */
void
pf_Fragments::_leftRotate(Node* x)
{
 	Node* y;
	y = x->right;

	/* correct the sizes */
	y->item->accLeftTreeLength(x->item->getLength() + x->item->getLeftTreeLength());
	
    /* Turn y's left sub-tree into x's right sub-tree */
    x->right = y->left;

    if (y->left != m_pLeaf)
        y->left->parent = x;

    /* y's new parent was x's parent */
    y->parent = x->parent;
    /* Set the parent to point to y instead of x */
    /* First see whether we're at the root */
    if (!x->parent)
		m_pRoot = y;
    else
        if (x == x->parent->left)
            /* x was on the left of its parent */
            x->parent->left = y;
        else
            /* x must have been on the right */
            x->parent->right = y;
    /* Finally, put x on y's left */
    y->left = x;
    x->parent = y;
}

/**
 * This method rotates a tree to the left.
 *
 * Let be 2 & 4 nodes of a tree.  Let be 1, 3, 5 subtrees.
 * This operation performs the rotation that appears in the
 * this ASCII picture:
 *
 *        4                2
 *       / \              / \
 *      2   5     =>     1   4
 *     / \                  / \
 *    1   3                3   5
 *
 * @params x is the head of the tree (node 4 in the picture).
 */
void
pf_Fragments::_rightRotate(Node* x)
{
	Node* y;
	y = x->left;

	/* correct the sizes */
	x->item->accLeftTreeLength( - ( y->item->getLength() + y->item->getLeftTreeLength() ) );
	
	x->left = y->right;

	if (y->right != m_pLeaf)
		y->right->parent = x;

	y->parent = x->parent;

	if (!x->parent)
		m_pRoot = y;
	else
		if (x == x->parent->right)
			x->parent->right = y;
		else
			x->parent->left = y;

	y->right = x;
	x->parent = y;
}

/**
 * This method calculates the cumulated size of all the
 * nodes that are in the subtree that has "x" as head.
 *
 * This operation is performed in O(log(n)), where n is
 * the number of nodes in the subtree.
 *
 * @params x is the head of the subtree
 */
PT_DocPosition
pf_Fragments::_calculateSize(Node* x)
{
	UT_ASSERT(x != NULL);

	if (x == m_pLeaf)
		return 0;
	
	return x->item->getLeftTreeLength() + x->item->getLength() + _calculateSize(x->right);
}

void
pf_Fragments::delete_tree(Node* node)
{
	if (node->left != m_pLeaf)
		delete_tree(node->left);

	if (node->right != m_pLeaf)
		delete_tree(node->right);

	delete node->item;
	delete node;
}

#ifdef DEBUG
void
pf_Fragments::print() const
{
	if (m_pRoot != m_pLeaf)
		m_pRoot->print();
	else
		printf("empty set\n");
}

// Iterator should be a ConstIterator
int
pf_Fragments::_countBlackNodes(const Iterator& it) const
{
	int retval = 0;
	const Node* pn = it.getNode();
	UT_ASSERT(it.is_valid());

	do
	{
		if (pn->color == Node::black)
			++retval;
		else  // we're red
			if (pn->parent && pn->parent->color == Node::red) // and also our father!!
			{
				printf("ERROR: We've found a red node that has a red father.\n");
				return -1;
			}

		pn = pn->parent;
	}
	while (pn != 0);

	return retval;
}

bool
pf_Fragments::checkInvariants() const
{
	int nb_blacks = 0;

	// These iterators should be ConstIterators
	// ConstIterator end(end());
	// ConstIterator it(begin());
	Iterator end(const_cast<pf_Fragments* const> (this));
	Iterator it(const_cast<pf_Fragments* const> (this), const_cast<Node*> (_first()));

	if (it != end)
		nb_blacks = _countBlackNodes(it++);

	if (nb_blacks < 0)
		return false;

	if (!checkSizeInvariant(m_pRoot, NULL))
		return false;
	
	for (; it != end; ++it)
	{
		Node *pn = it.getNode();
		
		if (pn->left == m_pLeaf && pn->right == m_pLeaf &&
			nb_blacks != _countBlackNodes(it))
		{
			printf("ERROR: We've found two paths that pass through a different number of black nodes. "
				   "The first path pass through %d nodes and the second one pass through %d nodes.\n", nb_blacks, _countBlackNodes(it));
			return false;
		}
	}

	return true;
}

bool
pf_Fragments::checkSizeInvariant(const Node* pn, PT_DocPosition* size) const
{
	PT_DocPosition m_lengthLeft, size_right;
	
	if (pn == m_pLeaf)
	{
		if (size)
			*size = 0;

		return true;
	}

	if (!checkSizeInvariant(pn->left, &m_lengthLeft))
		return false;
	
	if (!checkSizeInvariant(pn->right, &size_right))
		return false;

	if (pn->item->m_leftTreeLength != m_lengthLeft)
	{
		printf("A node reported as m_lengthLeft [%u], but the real size of its left subtree is [%u].\n", pn->item->m_leftTreeLength, m_lengthLeft);
		return false;
	}

	if (size)
		*size = pn->item->m_length + m_lengthLeft + size_right;

	return true;
}

#endif
