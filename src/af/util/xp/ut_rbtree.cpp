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

#include "ut_rbtree.h"
#include "ut_assert.h"
#include <string.h>
#ifdef DEBUG
#include <stdio.h>
#endif

///////////////////////////////////////////
// Node
///////////////////////////////////////////

struct UT_RBTree::Node
{
	enum Color { red, black } color;
	UT_RBTree::key_t item;
	Node* left;
	Node* right;
	Node* parent;

	Node() : color(red), item(0), left(0), right(0), parent(0) {}

#ifdef DEBUG
	void print()
	{
		printf("============ @ %p\n", static_cast<void*> (this));
		printf("Color: %s\n", color == red ? "red" : "black");
		printf("Value: %p\n", item);
		printf("left: %p\n", static_cast<void*> (left));
		printf("Rigth: %p\n", static_cast<void*> (right));
		if (left)
			left->print();
		if (right)
			right->print();
	}
#endif
};

///////////////////////////////////////////
// Iterator
///////////////////////////////////////////

UT_RBTree::key_t UT_RBTree::Iterator::value() const
{
	return m_pNode->item;
}

///////////////////////////////////////////
// UT_RBTree
///////////////////////////////////////////
UT_RBTree::UT_RBTree(UT_RBTree::comparator comp)
	: m_pRoot(0),
	  m_comp(comp),
	  m_nSize(0)
{
}

UT_RBTree::~UT_RBTree()
{
	if (m_pRoot)
		s_delete_tree(m_pRoot);
}

void
UT_RBTree::_insertBST(Node* pNewNode)
{
	Node* pNode = m_pRoot;
	UT_ASSERT(pNewNode);

	while(pNode)
	{
		if (m_comp(pNewNode->item, pNode->item))
		{
			if (pNode->left)
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
			if (pNode->right)
				pNode = pNode->right;
			else
			{
				pNewNode->parent = pNode;
				pNode->right = pNewNode;
				break;
			}
		}
	}

	if (!pNode)
	{
		m_pRoot = pNewNode;
		m_pRoot->color = Node::black;
	}
}

void
UT_RBTree::_insertFixup(Node* x)
{
	Node* y;
	UT_ASSERT(x);

	while ((x != m_pRoot) && (x->parent->color == Node::red))
	{
		if (x->parent == x->parent->parent->left)
		{
			/* If x's parent is a left, y is x's right 'uncle' */
			y = x->parent->parent->right;
		    if (y->color == Node::red)
			{
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
    /* Colour the root black */
    m_pRoot->color = Node::black;
	}
}

bool
UT_RBTree::insert(const void* item)
{
	Node* pnode = new Node;

	if (!pnode)
		return false;

	++m_nSize;
	pnode->item = item;
	_insertBST(pnode);
	_insertFixup(pnode);

	return true;
}

void
UT_RBTree::erase(Iterator& c)
{
	if (!c.is_valid())
		return;

	Node* pNode = c.getNode();
	--m_nSize;

	UT_ASSERT(pNode);
	Node* y;

	if (pNode->left == 0 || pNode->right == 0)
		y = pNode;
	else
		y = _prev(pNode);

	UT_ASSERT(y->left == 0 || y->right == 0);
	Node* son = y->left ? y->left : y->right;
	if (son)
		son->parent = y->parent;

	if (!y->parent)
		m_pRoot = son;
	else
		if (y->parent->left == y)
			y->parent->left = son;
		else
			y->parent->right = son;

	if (y != pNode)
		pNode->item = y->item;

	if (y->color == Node::black && son)
		_eraseFixup(son);

	delete y;
}

void
UT_RBTree::_eraseFixup(UT_RBTree::Node *x)
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
            Node *tmp = x->parent->left;
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

UT_RBTree::Iterator
UT_RBTree::find(const void* item)
{
	Node* x = m_pRoot;
	while(x)
	{
		if (item == x->item)
			return Iterator(this, x);

		if (m_comp(item, x->item))
			x = x->left;
		else
			x = x->right;
	}

	return Iterator(this, 0);
}

UT_RBTree::Iterator
UT_RBTree::find_if(const void* item, comparator pred)
{
	Node* x = m_pRoot;
	while(x)
	{
		if (pred(item, x->item))
			return Iterator(this, x);

		if (m_comp(item, x->item))
			x = x->left;
		else
			x = x->right;
	}

	return Iterator(this, 0);
}

const UT_RBTree::Node*
UT_RBTree::_prev(const UT_RBTree::Node* pn) const
{
	if (pn)
	{
		if (pn->left)
		{
			pn = pn->left;

			while(pn->right)
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

UT_RBTree::Node*
UT_RBTree::_prev(UT_RBTree::Node* pn)
{
	if (pn)
	{
		if (pn->left)
		{
			pn = pn->left;

			while(pn->right)
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

const UT_RBTree::Node*
UT_RBTree::_next(const UT_RBTree::Node* pn) const
{
	if (pn)
	{
		if (pn->right)
		{
			pn = pn->right;

			while(pn->left)
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

UT_RBTree::Node*
UT_RBTree::_next(UT_RBTree::Node* pn)
{
	if (pn)
	{
		if (pn->right)
		{
			pn = pn->right;

			while(pn->left)
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

const UT_RBTree::Node*
UT_RBTree::_first() const
{
	Node* pn = m_pRoot;

	while(pn && pn->left)
		pn = pn->left;

	return pn;
}

UT_RBTree::Node*
UT_RBTree::_first()
{
	Node* pn = m_pRoot;

	while(pn && pn->left)
		pn = pn->left;

	return pn;
}

const UT_RBTree::Node*
UT_RBTree::_last() const
{
	Node* pn = m_pRoot;

	while(pn && pn->left)
		pn = pn->left;

	return pn;
}

UT_RBTree::Node*
UT_RBTree::_last()
{
	Node* pn = m_pRoot;

	while(pn && pn->left)
		pn = pn->left;

	return pn;
}

void
UT_RBTree::_leftRotate(UT_RBTree::Node* x)
{
 	Node* y;
	y = x->right;
    /* Turn y's left sub-tree into x's right sub-tree */
    x->right = y->left;

    if (y->left)
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

void
UT_RBTree::_rightRotate(UT_RBTree::Node* x)
{
	Node* y;
	y = x->left;
	x->left = y->right;

	if (y->right)
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

void
UT_RBTree::s_delete_tree(UT_RBTree::Node* node)
{
	if (node->left)
		s_delete_tree(node->left);
	if (node->right)
		s_delete_tree(node->right);
	delete node;
}

#ifdef DEBUG
void
UT_RBTree::print() const
{
	if (m_pRoot)
		m_pRoot->print();
	else
		printf("empty set\n");
}
#endif

bool ut_lexico_lesser(UT_RBTree::key_t x, UT_RBTree::key_t y)
{
	return (strcmp(static_cast<const char*> (x), static_cast<const char*> (y)) < 0);
}

bool ut_lexico_greater(UT_RBTree::key_t x, UT_RBTree::key_t y)
{
	return (strcmp(static_cast<const char*> (x), static_cast<const char*> (y)) > 0);
}

bool ut_lexico_equal(UT_RBTree::key_t x, UT_RBTree::key_t y)
{
	return (strcmp(static_cast<const char*> (x), static_cast<const char*> (y)) == 0);
}

