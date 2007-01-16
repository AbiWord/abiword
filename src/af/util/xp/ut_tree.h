/* AbiSource Program Utilities
 *
 * Copyright (C) 2004 Tomas Frydrych <tomasfrydrych@yahoo.co.uk>
 *
 * This program is g_free software; you can redistribute it and/or
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

#ifndef UT_TREE_H
#define UT_TREE_H

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

#include "ut_debugmsg.h"
#include "ut_stack.h"

#if _MSC_VER >= 1310
// MSVC++ 7.1 warns about debug output limitations.
#pragma warning(disable: 4292)
#endif

/*! \brief UT_GenericTree allows access to its data in a tree-like fashion: iterating over
    branches, or siblings, or accessing individual nodes.
	
    UT_GenericTree Class \n
    ==================== \n

    Summary of usage \n
    ---------------- \n
    
    UT_GenericTree allows access to its data in a tree-like fashion: iterating over
    branches, or siblings, or accessing individual nodes. The idea is to convert data in
    format \n
	 
	 \verbatim
        <a><b/><c><d/></c></a>
	 \endverbatim
	 
    to

	 \htmlonly <pre>
	 \endhtmlonly
	 \verbatim
		    a
		   / \
		  b   c
		       \
		        d
	 \endverbatim			
	 \htmlonly </pre>
	 \endhtmlonly

    After creation, the object is intialised from a sequentiall representation of the data
    (such as found in an xml file or AbiWord piecetable). This is done either by repeated
    calls to the insert() function, or by using the buildTree() function.

    If the tree is used to store pointers that need to be deallocated, before
    destroying the tree object, the owner needs to call either purgeData() or freeData()
    depending on the type of pointer the tree stores.

    Once the tree is filled, it can be navigated either using the node functions or the
    Cursor subclass

    NB: the fuctionality of the present implementation is very simple; notably, there is
    no API to insert/delete nodes after the tree's initial construction.

    
    buildTree() method \n
    ------------------ \n
    
    The buildTree() method requires a callback function and a void pointer; the void
    pointer is passed back to the callback function (e.g., if the call back function is a
    static function in a class, it might be passed a this pointer this way).

    The callback function has the following signature:
    
    	UT_TreeNodeRelationship next(T& n, void* param);
    	
	On each call, this function should place the next node into n, and its return value
   	should indicate the relationship of this node to the previous node; TNR_None should be
   	returned when there are no more nodes to be inserted. Before the function is called,
   	the void * param will be set to whatever the caller passed as callerdata to
   	buildTree().

   	If successful, buildTree() returns true. If the return value is false, the user can
   	(and should!) use purgeData()/freeData() method to deallocate any pointers that were
   	passed into tree.


    Node functions \n
    -------------- \n
	
	UT_sint32 getMaxNodeLevel() const;
	    This function returns the highest node level in the tree, that is, the distance
	    between the the outmost leaf and the trunk of the tree. For example, the tree
	    above will have a max node level 2, because the distance between 'a' and 'd' is 2.

	    
	UT_uint32 getNodeCountForLevel(UT_uint32 iLevel)
	    This function returns the number of nodes of particular level; in the example
	    above, there will be 1 node at level 0 (a), 2 nodes at level 1 (b,c), 1 node at
	    level 2 (d).

	    
	T getNthNodeForLevel(UT_uint32 iLevel, UT_uint32 n) const;
	    This function returns the value stored at n-th node of level iLevel. In the tree
	    above node d could be accessed by getNthNodeLevel(2,0). If dpos is the position of
	    a node in the original linear stream, the following is always true
	    if(iLevel1 == iLevel2 && n1 < n2) => dpos1 < dpos2

    
    Cursor class \n
    --------------- \n
	The cursor class makes it possible to iterate over the tree in structured way using
    the relationships between the indivial nodes; they return true on success.

	bool first() -- sets the cursor to the root of the tree
	
	bool prev()
	bool next()  -- these functions iterate over the tree in sequence that corresponds to
	                the original linear sequence from which the tree was created; they are
	                provided mostly for completeness, as the whole point of creating a
					tree is to be able to access the nodes in non-linear fashion.

	bool firstChild()
	bool nextSibling()
	bool prevSibling()
	bool parent()   these functions position the cursor to the node which is in that
	                particular relationship with the current node.

	bool firstSibling() -- positions the cursor to the node which is registered as the
	                first child of the current node's parent.
	         
    bool lastValidPosition() -- resets the cursor to the last last valid node, for example
                    when call to nextChild() results in the cursor being invalid, the
                    caller can use this function to start iterating in the oposit
                    direction, or what ever.

    bool setPosition(UT_uint32 iLevel, iOffset) -- this function can be used to position
                    the cursor to a particular node described by its level and position in
                    that level. This allows to combine the two methods of navigating tree,
                    for example, you could use the node methods in a loop to move across a
                    particular node level and the cursor inside an inner loop to traverse
                    branches of the subtrees starting at the nodes of the given level).

    bool is_valid() -- returns true if the cursor points to a valid node.
*/


template <class T> class ABI_EXPORT UT_GenericTree
{
  // fwd. declarations of subclasses
  private:
	class Node; // internal use only
	
  public:
  	class Cursor;
	friend class Cursor;
	
  	UT_GenericTree(): m_data(NULL){};
	~UT_GenericTree() {clear();}

	bool      buildTree( UT_sint32 (*next)(T& n, void* param), void * callerdata);
	bool      insert(T value, UT_sint32 iRelativeLevel);

	
	void      clear();           // empty and deallocate internal data structures (but not node data !!!)
	void      freeData(void);    // deallocate node data using g_free()
	void      purgeData(void);   // deallocate node data using delete
	
	UT_sint32 getMaxNodeLevel() const {return m_nodeMap.getItemCount()-1;}
	UT_uint32 getNodeCountForLevel(UT_uint32 iLevel) const;
	T         getNthNodeForLevel(UT_uint32 iLevel, UT_uint32 n) const;

	bool      getNodesForLevel(UT_uint32 iLevel, UT_GenericVector<T> & vecNodes) const;

  private:
	// internall representation of a node
	class Node
	{
		friend class Cursor;
		friend class UT_GenericTree<T>;
		
	  public:
		Node(): parent(NULL),firstChild(NULL),nextSibling(NULL),prevSibling(NULL),level(0) {};
		void setContent(T t) {me = t;}
		T    getContent() const {return me;}

		void setLevel(UT_uint32 i) {level = i;}
		UT_uint32 getLevel()const {return level;}

		bool isLeaf() const {return firstChild == NULL;}

	  private:
		T      me;
		Node * parent;
		Node * firstChild;
		Node * nextSibling;
		Node * prevSibling;

		UT_uint32 level;
	};

  public:
	// cursor for structure iteration over data
	class Cursor
	{
	  public:
		Cursor(const UT_GenericTree<T> * owner)
			: m_d(owner), m_saved_node(NULL)
		{
			m_node = m_d->m_data;
			UT_ASSERT_HARMLESS( m_node );
			m_last_valid_node = m_node;
		}
		
		~Cursor() { }
		
		bool first()
		{
			m_node = m_d->m_data;
			return is_valid();
		}

		bool last()
		{
			m_node = NULL;
			UT_sint32 iMaxLevel = m_d->getMaxNodeLevel();
			if(iMaxLevel < 0)
				return false;

			UT_sint32 iLastNode = m_d->getNodeCountForLevel(iMaxLevel) - 1;

			if(iLastNode < 0)
				return false;

			m_node = m_d->_getNthNodeForLevel(iMaxLevel, iLastNode);

			return is_valid();
		}
		
		bool lastValidPos()
		{
			m_node = m_last_valid_node;
			return is_valid();
		}
		
		bool next()
		{
			if(!is_valid())
				return false;

			m_last_valid_node = m_node;
			
			if(_getNode()->firstChild)
			{
				_setNode(_getNode()->firstChild);
				return is_valid();
			}
	
			while(_getNode())
			{
				if(_getNode()->nextSibling)
				{
					_setNode(_getNode()->nextSibling);
					return is_valid();
				}

				// no next sibling, see if our parent has any siblings
				_setNode(_getNode()->parent);
			}

			return is_valid();
		}

		bool prev()
		{
			if(!is_valid())
				return false;
			
			m_last_valid_node = m_node;

			if(_getNode()->firstChild)
			{
				_setNode(_getNode()->firstChild);
				return is_valid();
			}
	
			while(_getNode())
			{
				if(_getNode()->prevSibling)
				{
					_setNode(_getNode()->prevSibling);
					return is_valid();
				}

				// no previous sibling, see if our parent has any siblings
				_setNode(_getNode()->parent);
			}

			return is_valid();
		}

		bool firstChild()
		{
			if(!is_valid())
				return false;
			
			m_last_valid_node = m_node;

			_setNode(m_node->firstChild);
			return is_valid();
		}

		bool firstSibling()
		{
			if(!is_valid())
				return false;
			
			m_last_valid_node = m_node;

			_setNode(m_node->parent->firstChild);
			return is_valid();
		}

		bool nextSibling()
		{
			if(!is_valid())
				return false;
			
			m_last_valid_node = m_node;

			_setNode(m_node->nextSibling);
			return is_valid();
		}

		bool prevSibling()
		{
			if(!is_valid())
				return false;
			
			m_last_valid_node = m_node;

			_setNode(m_node->prevSibling);
			return is_valid();
		}

		bool parent()
		{
			if(!is_valid())
				return false;

			if(getLevel() == 0)
			{
				// we are at the trunk already
				_setNode(NULL);
				return false;
			}
			
			m_last_valid_node = m_node;

			_setNode(_getNode()->parent);
			return is_valid();
		}

		void setPosition(UT_uint32 iLevel, UT_uint32 iNthNode)
		{
			Node * n = m_d->_getNthNodeForLevel(iLevel, iNthNode);
			UT_return_if_fail( n );

			_setNode(n);
		}
		
		UT_uint32 getLevel()const
		{
			return m_node ? m_node->getLevel() : 0;
		}

		const T getContent()
		{
			// we need some dummy return value in case the user calls us when we are not valid
			static T s;
			return is_valid() ? m_node->getContent() : s;
		}

		inline bool is_valid() const
		{
			return (m_node != NULL);
		}

		bool isLeaf() const {return is_valid() ? m_node->isLeaf() : true;}

		void save()
		{
			m_saved_node = m_node;
		}

		void restore()
		{
			UT_return_if_fail( m_saved_node );
			m_node = m_saved_node;
		}
		
	  private:
		
		inline void	 _setNode(Node *node)	
		{
			m_node = node;
		}
		
		inline const Node* _getNode()
		{
			return m_node;
		}
		
		const UT_GenericTree<T>*	m_d;
		const Node*			        m_node;
		const Node*                 m_last_valid_node;
		const Node*                 m_saved_node;
	};

	friend class Cursor;
	
private:

	bool _insertChild(T value);
	bool _insertSibling(T value);
	bool _insertUncle(T value);
	bool _dropLevel();

	bool   _mapNode(Node *);
	Node * _getNthNodeForLevel(UT_uint32 iLevel, UT_uint32 n) const;
	
	UT_GenericTree(const UT_GenericTree<T>&);	// no impl
	void operator=(const UT_GenericTree<T>&);		// no impl

	Node * m_data;

	UT_Stack m_nodeStack;
	UT_GenericVector<UT_GenericVector<Node*>*> m_nodeMap;
};

	/* purge objects by deleting them */
template <class T> void
UT_GenericTree<T>::purgeData(void) 
{
	Cursor hc1(this);
	for ( hc1.first(); hc1.is_valid(); hc1.next() )
	{
		T t = hc1.getContent();
		if (t) {
			delete t;
		}
	}

	clear();
};
	
/* purge objects by freeing them */
template <class T> void
UT_GenericTree<T>::freeData(void) 
{
	Cursor hc1(this);
	for ( hc1.first(); hc1.is_valid(); hc1.next() )
	{
		T t = hc1.getContent();
		if (t) {
			g_free(const_cast<T>(t));
		}
	}

	clear();
}


template <class T> bool
UT_GenericTree<T>::buildTree( UT_sint32 (*next)(T& t, void* param), void * callerdata)
{
	// first of all reset current state
	clear();
	
	T n;
	UT_sint32 r = next(n, callerdata);
	bool b = true;
	
	while (r <= 1 && b)
	{
		b = insert(n,r);
		r = next(n, callerdata);
	}

	return b;
}


/*!
    Insert value in a relative relationship r into the tree
    valid values or r are:
       1  ~ child of the previous node
       0  ~ sibling of the previous node
       -n ~ sibling of n-th level ancestor of the previous node
*/
template <class T> bool
UT_GenericTree<T>::insert(T value, UT_sint32 r)
{
	UT_return_val_if_fail( r <= 1, false );
	
	if(r == 1)
		return _insertChild(value);
	else if(r == 0)
		return _insertSibling(value);
	else
	{
		bool bOK = true;
		for(UT_sint32 i = r; bOK && i < 0; i++)
		{
			bOK = _dropLevel();
		}

		UT_return_val_if_fail( bOK, false );

		return _insertSibling(value);
	}

	UT_ASSERT_HARMLESS( UT_NOT_REACHED );
	return false;
}


template <class T> void
UT_GenericTree<T>::clear()
{
	m_nodeStack.clear();

	UT_uint32 iLevels = m_nodeMap.getItemCount();
	for(UT_uint32 i = 0; i < iLevels; ++i)
	{
		UT_GenericVector<Node*>* v = m_nodeMap.getNthItem(i);
		if(!v)
		{
			UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
			continue;
		}

		UT_uint32 iNodes = v->getItemCount();

		for(UT_uint32 j = 0; j < iNodes; ++j)
		{
			Node * n = v->getNthItem(j);
			delete n;
		}

		delete  v;
	}

	m_nodeMap.clear();
}


template <class T> UT_uint32
UT_GenericTree<T>::getNodeCountForLevel(UT_uint32 iLevel) const
{
	if(m_nodeMap.getItemCount() < iLevel + 1)
		return 0;

	UT_GenericVector<Node*>* v = m_nodeMap.getNthItem(iLevel);

	UT_return_val_if_fail( v, 0 );

	return v->getItemCount();
}

template <class T> T
UT_GenericTree<T>::getNthNodeForLevel(UT_uint32 iLevel, UT_uint32 n) const
{
	static T s; // this is something to return if are asked for something out of bounds
	UT_return_val_if_fail(m_nodeMap.getItemCount() > iLevel, s);

	UT_GenericVector<Node*>* v = m_nodeMap.getNthItem(iLevel);

	UT_return_val_if_fail( v, s );
	UT_return_val_if_fail( v->getItemCount() > n, s );

	Node * node = v->getNthItem(n);
	UT_return_val_if_fail( node, s );

	return node->getContent();
}

template <class T> bool
UT_GenericTree<T>::getNodesForLevel(UT_uint32 iLevel, UT_GenericVector<T> & vecNodes) const
{
	UT_return_val_if_fail(m_nodeMap.getItemCount() > iLevel, false);

	UT_GenericVector<Node*>* v = m_nodeMap.getNthItem(iLevel);

	UT_return_val_if_fail( v, false );

	for(UT_uint32 i = 0; i < v->getItemCount(); ++i)
	{
		vecNodes.addItem(v->getNthItem(i)->getContent());
	}
}


template <class T> UT_GenericTree<T>::Node *
UT_GenericTree<T>::_getNthNodeForLevel(UT_uint32 iLevel, UT_uint32 n) const
{
	UT_return_val_if_fail(m_nodeMap.getItemCount() > iLevel, NULL);

	UT_GenericVector<Node*>* v = m_nodeMap.getNthItem(iLevel);

	UT_return_val_if_fail( v, NULL );

	UT_return_val_if_fail( v->getItemCount() > n, NULL );

	return v->getNthItem(n);
}


template <class T> bool
UT_GenericTree<T>::_mapNode(Node * n)
{
	UT_return_val_if_fail( n, false );

	UT_uint32 iLevel = n->getLevel();

	if(m_nodeMap.getItemCount() <  iLevel + 1)
	{
		// check we did not skip level
		UT_return_val_if_fail( m_nodeMap.getItemCount() == iLevel, false );
		UT_GenericVector<Node*>* v = new UT_GenericVector<Node*>;
		UT_return_val_if_fail( v, false );

		v->addItem(n);
		m_nodeMap.addItem(v);
	}
	else
	{
		UT_GenericVector<Node*>* v = m_nodeMap.getNthItem(iLevel);
		UT_return_val_if_fail( v, false );

#ifdef DEBUG
		// check the node is not in (this could be quite involved for a larged doc, so we
		// only do it in DEBUG mode, and hence we do not hadle it here, merely assert
		UT_sint32 iPos = v->findItem(n);
		UT_ASSERT_HARMLESS( iPos < 0 );
#endif
		
		v->addItem(n);
	}

	return true;
}

template <class T> bool
UT_GenericTree<T>::_insertChild(T value)
{
	Node * n = new Node();
	UT_return_val_if_fail( n, false );

	n->setContent(value);
	
	Node * p = NULL;
	m_nodeStack.viewTop((void**)&p);

	n->parent = p;

	if(p)
	{
		n->setLevel(p->getLevel()+1);
		p->firstChild = n;
	}
	else
	{
		m_data = n;
	}
	

	m_nodeStack.push((void*)n);
	return 	_mapNode(n);
}


template <class T> bool
UT_GenericTree<T>::_insertUncle(T value)
{
	Node * n = new Node();
	UT_return_val_if_fail( n, false );

	n->setContent(value);

	Node * s;
	m_nodeStack.viewTop((void**)&s);
	
	UT_uint32 iLevel = s ? s->getLevel() : 0;

	bool bOK = true;
	
	if(iLevel)
	{
		while(bOK && s && s->getLevel() >= iLevel)
		{
			m_nodeStack.pop((void**)&s);
			bOK = m_nodeStack.viewTop((void**)&s);
		}
	}
	
	UT_return_val_if_fail( bOK, false );
	
	if(s)
	{
		n->setLevel(s->getLevel());
		n->parent = s->parent;
		
		Node * next = s->nextSibling;

		n->prevSibling = s;
		s->nextSibling = n;
		n->nextSibling = next;
	}
	else
	{
		m_data = n;
	}
	
	m_nodeStack.push((void*)n);
	return _mapNode(n);
}

template <class T> bool
UT_GenericTree<T>::_dropLevel()
{
	Node * s;
	m_nodeStack.viewTop((void**)&s);
	
	UT_uint32 iLevel = s ? s->getLevel() : 0;
	bool bOK = true;

	if(iLevel)
	{
		while(bOK && s && s->getLevel() >= iLevel)
		{
			m_nodeStack.pop((void**)&s);
			bOK = m_nodeStack.viewTop((void**)&s);
		}
	}

	return bOK;
}

template <class T> bool
UT_GenericTree<T>::_insertSibling(T value)
{
	Node * n = new Node();
	UT_return_val_if_fail( n, false );

	n->setContent(value);

	Node * s;
	m_nodeStack.viewTop((void**)&s);

	if(s)
	{
		n->setLevel(s->getLevel());
		n->parent = s->parent;
		
		Node * next = s->nextSibling;

		n->prevSibling = s;
		s->nextSibling = n;
		n->nextSibling = next;
	}
	else
	{
		m_data = n;
	}
	
	m_nodeStack.push((void*)n);
	return  _mapNode(n);
}

#endif /* UT_TREE_H */
