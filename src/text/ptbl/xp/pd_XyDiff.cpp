/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2004 Tomas Frydrych <tomasfrydrych@yahoo.co.uk>
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

#include <math.h>
#include "pd_XyDiff.h"
#include "pf_Frag.h"
#include "pf_Frag_Object.h"
#include "pf_Frag_Strux.h"
#include "pf_Frag_Text.h"
#include "pt_PieceTable.h"
#include "ut_string_class.h"
#include "ut_misc.h"

#ifdef DEBUG
#include <stdio.h>
#include "xap_App.h"
#endif

static void hashToStr(UT_uint64 iH, UT_String &s)
{
	UT_uint32 * pH1 = (UT_uint32 *) &iH;
	UT_uint32 * pH2 = pH1 + 1;
		
	UT_String_sprintf(s, "%d%d", *pH1, *pH2);
}

/*!
    iXID           the unique xid attribute of this node, or 0 if absent
    iHash          hash of text/data for leafs, has of the node name for the rest
    iLinearWeight  length of data for content elements (text/data), 0 for the rest
 */
PD_DocNode::PD_DocNode(pf_Frag * pf, UT_uint64 iHash, UT_uint32 iLinearWeight):
	m_iMyHash(iHash),
	m_iOffspringHash(0),
	m_bNoMatch(false),
	m_iLevel(0),
	m_iOffset(0),
	m_pTheOther(NULL),
	m_eStatus(UNMODIFIED),
	m_pFrag(pf)
{
	// the weight of non-leafs should be 1, but it seems simpler for the API to expect 0
	// and add 1 here
	if(!iLinearWeight)
		m_fWeight = 1.0;
	else
		m_fWeight = log((double)iLinearWeight) + 1.0;

	UT_return_if_fail( pf );
	m_iXID = pf->getXID();

	switch(pf->getType())
	{
		case pf_Frag::PFT_Text:
			m_eType = TEXT;
			break;
			
		case pf_Frag::PFT_Strux:
			m_eType = ELEMENT;
			break;

		case pf_Frag::PFT_Object:
		{
			pf_Frag_Object * pfo = (pf_Frag_Object*) pf;
			switch(pfo->getObjectType())
			{
				case PTO_Image:
					m_eType = DATA;
					break;
						
				default:
					m_eType = ELEMENT;
			}
		}
			
		default:
			UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
			m_eType = ELEMENT;
	}
	
};

#ifdef DEBUG
void mytreefunct()
{
	PD_XyDiff xdiff;
}

PD_DocNode::PD_DocNode(const char * pName, UT_uint32 iXID, PD_DocNodeType eType,
					   UT_uint64 iHash, UT_uint32 iLinearWeight):
	m_pName(pName),
	m_iXID(iXID),
	m_iMyHash(iHash),
	m_iOffspringHash(0),
	m_bNoMatch(false),
	m_iLevel(0),
	m_iOffset(0),
	m_pTheOther(NULL),
	m_eType(eType),
	m_eStatus(UNMODIFIED),
	m_pFrag(NULL)
{
	if(!iLinearWeight)
		m_fWeight = 1.0;
	else
		m_fWeight = log((double)iLinearWeight) + 1.0;
}

PD_XyDiff::PD_XyDiff():
	m_fWeight1(0.0), m_fWeight2(0.0), m_iNodeCount2(0)
{
	// some code here to build the simulated trees ...
	m_tree1.insert(new PD_DocNode("section xid=1", 1, ELEMENT, UT_hash64("section"), 0), 1);
	m_tree1.insert(new PD_DocNode("p xid=2", 2, ELEMENT, UT_hash64("p"), 0), 1);
	m_tree1.insert(new PD_DocNode("c ABCDE", 0, TEXT, UT_hash64("ABCDE"), 5), 1);
	m_tree1.insert(new PD_DocNode("p", 0, ELEMENT, UT_hash64("p"), 0), -1);
	m_tree1.insert(new PD_DocNode("c XYZ", 0, TEXT, UT_hash64("XYZ"), 3), 1);
	m_tree1.insert(new PD_DocNode("section xid=3", 3, ELEMENT, UT_hash64("section"), 0), -2);
	m_tree1.insert(new PD_DocNode("p", 0, ELEMENT, UT_hash64("p"), 0), 1);
	m_tree1.insert(new PD_DocNode("c abcd", 0, TEXT, UT_hash64("abcd"), 4), 1);
	m_tree1.insert(new PD_DocNode("section", 0, ELEMENT, UT_hash64("section"), 0), -2);
	m_tree1.insert(new PD_DocNode("table", 0, ELEMENT, UT_hash64("table"), 0), 1);
	m_tree1.insert(new PD_DocNode("cell", 0, ELEMENT, UT_hash64("cell"), 0), 1);
	m_tree1.insert(new PD_DocNode("c abcd", 0, TEXT, UT_hash64("abcd"), 5), 1);
	m_tree1.insert(new PD_DocNode("p", 0, ELEMENT, UT_hash64("p"), 0), -2);
	m_tree1.insert(new PD_DocNode("c QRST", 0, TEXT, UT_hash64("QRST"), 4), 1);


	m_tree2.insert(new PD_DocNode("section", 0, ELEMENT, UT_hash64("section"), 0), 1);
	m_tree2.insert(new PD_DocNode("table", 0, ELEMENT, UT_hash64("table"), 0), 1);
	m_tree2.insert(new PD_DocNode("cell", 0, ELEMENT, UT_hash64("cell"), 0), 1);
	m_tree2.insert(new PD_DocNode("c abcde", 0, TEXT, UT_hash64("abcde"), 5), 1);
	m_tree2.insert(new PD_DocNode("p xid=2", 2, ELEMENT, UT_hash64("p"), 0), -2);
	m_tree2.insert(new PD_DocNode("c ABCDEF", 0, TEXT, UT_hash64("ABCDEF"), 6), 1);
	m_tree2.insert(new PD_DocNode("section xid=1", 1, ELEMENT, UT_hash64("section"), 0), -2);
	m_tree2.insert(new PD_DocNode("p", 0, ELEMENT, UT_hash64("p"), 0), 1);
	m_tree2.insert(new PD_DocNode("c XYZ", 0, TEXT, UT_hash64("XYZ"), 3), 1);
	m_tree2.insert(new PD_DocNode("p", 0, ELEMENT, UT_hash64("p"), 0), -1);
	m_tree2.insert(new PD_DocNode("c QRST", 0, TEXT, UT_hash64("QRST"), 4), 1);
	
	UT_return_if_fail(_constructorCommonCode());

	dump();
}

void PD_XyDiff::dump()
{
	UT_GenericTree<PD_DocNode*>::Cursor c1(&m_tree1);
	UT_GenericTree<PD_DocNode*>::Cursor c2(&m_tree2);

	UT_String fname; // = XAP_App::getApp()->getAbiSuiteLibDir();
	fname += "xydiff.log";
	
	FILE * f = fopen(fname.c_str(),"wt");
	

	UT_DEBUGMSG(("\n----------------- START OF TREE DUMP -------------------\n\nDumping tree 1:\n"));
	fprintf(f,"\n----------------- START OF TREE DUMP -------------------\n\nDumping tree 1:\n");
	
	for(c1.first();c1.is_valid();c1.next())
	{
		PD_DocNode * n = c1.getContent();
		PD_DocNode * o = n->getTheOther();

		if(o)
		{
			UT_String s1, s2;
			hashToStr(n->getMyHash(), s1);
			hashToStr(o->getMyHash(), s2);
			
			UT_DEBUGMSG(("\tn1[%s](%d,%d: 0x%s) -> o2[%s](%d,%d: 0x%s) %04x\n",
						 n->getName(), n->getLevel(), n->getOffset(), s1.c_str(),
						 o->getName(), o->getLevel(), o->getOffset(), s2.c_str(), n->getStatus()));

			fprintf(f, "\tn1[%s](%d,%d: 0x%s) -> o2[%s](%d,%d: 0x%s) %04x\n",
					n->getName(), n->getLevel(), n->getOffset(), s1.c_str(),
					o->getName(), o->getLevel(), o->getOffset(), s2.c_str(), n->getStatus());
		}
		else
		{
			UT_String s1;
			hashToStr(n->getMyHash(), s1);

			UT_DEBUGMSG(("\tn1[%s](%d,%d: 0x%s) -> no match %04x\n",
						 n->getName(), n->getLevel(), n->getOffset(), s1.c_str(), n->getStatus()));

			fprintf(f, "\tn1[%s](%d,%d: 0x%s) -> no match %04x\n",
					n->getName(), n->getLevel(), n->getOffset(), s1.c_str(), n->getStatus());
		}
	}

	UT_DEBUGMSG(("\nDumping tree 2:\n"));
	fprintf(f, "\nDumping tree 2:\n");

	for(c2.first();c2.is_valid();c2.next())
	{
		PD_DocNode * n = c2.getContent();
		PD_DocNode * o = n->getTheOther();

		if(o)
		{
			UT_DEBUGMSG(("\tn2[%s](%d,%d) -> o2[%s](%d,%d) %04x\n",
						 n->getName(), n->getLevel(), n->getOffset(),
						 o->getName(), o->getLevel(), o->getOffset(), n->getStatus()));

			fprintf(f, "\tn2[%s](%d,%d) -> o2[%s](%d,%d) %04x\n",
					n->getName(), n->getLevel(), n->getOffset(),
					o->getName(), o->getLevel(), o->getOffset(), n->getStatus());
		}
		else
		{
			UT_DEBUGMSG(("\tn2[%s](%d,%d) -> no match %04x\n",
						 n->getName(), n->getLevel(), n->getOffset(), n->getStatus()));

			fprintf(f, "\tn2[%s](%d,%d) -> no match %04x\n",
					n->getName(), n->getLevel(), n->getOffset(), n->getStatus());
		}
	}

	UT_DEBUGMSG(("\n------------ END OF TREE DUMP ---------------------\n"));
	fprintf(f, "\n------------ END OF TREE DUMP ---------------------\n");

	fclose(f);
}

#endif

PD_XyDiff::PD_XyDiff(pt_PieceTable * pPT1, pt_PieceTable * pPT2):
	m_fWeight1(0.0), m_fWeight2(0.0), m_iNodeCount2(0)
{
	UT_return_if_fail(_buildDocTree(pPT1, m_tree1));
	UT_return_if_fail(_buildDocTree(pPT2, m_tree2));

	UT_return_if_fail(_constructorCommonCode());
}

bool PD_XyDiff::_constructorCommonCode()
{
	UT_return_val_if_fail(_locateIdNodes(), false);
	UT_return_val_if_fail(_calculateHashesAndWeights(m_tree1, m_fWeight1), false);
	UT_return_val_if_fail(_calculateHashesAndWeights(m_tree2, m_fWeight2), false);
	UT_return_val_if_fail(_constructPriorityQueue(), false);
	UT_return_val_if_fail(_matchAllNodes(), false);
	UT_return_val_if_fail(_optimizeMatches(), false);
	UT_return_val_if_fail(_optimizeContentLeaves(), false);
	UT_return_val_if_fail(_computeAdjustments(), false);
	
	return true;
}


PD_XyDiff::~PD_XyDiff()
{
	m_hashNodes.purgeData();
	m_tree1.purgeData();
	m_tree2.purgeData();
}

static int s_compare_weight(const void * n1, const void * n2)
{
	UT_return_val_if_fail( n1 && n2, 0 );
	
	const PD_DocNode *  N1 = (const PD_DocNode *) n1;
	const PD_DocNode *  N2 = (const PD_DocNode *) n2;

	// we want to heavier first ...
	if(N1->getWeight() > N2->getWeight())
		return -1;
	else if (N2->getWeight() > N1->getWeight())
		return 1;

	// this is the case of equal weights; the algorithm specifies that if the weights are
	// equal, the subtree which is bigger is prefered; in the original xydiff
	// implementation this is interpreted as the node which is nearer to the trunk
	if(N1->getLevel() < N2->getLevel())
		return -1;
	else if (N1->getLevel() > N2->getLevel())
		return 1;

	//Hm, nodes with same weight, and same level -- take the one that is physically
	//earlier in the doc
	if(N1->getOffset() < N2->getOffset())
		return -1;
	else if (N1->getOffset() > N2->getOffset())
		return 1;

	// OK, I give up, there is no telling these two apart ...
	return 0;
}

/*!
    Add the nodes passed to the function into the processing gueue, in the order from the
    heavies to the lightest
*/
bool PD_XyDiff::_addNodesToQueue(UT_GenericVector<PD_DocNode*> & v)
{
	// now we need to sort these by weight
	v.qsort(s_compare_weight);

	for(UT_uint32 i = 0; i < v.getItemCount(); ++i)
	{
		UT_return_val_if_fail(m_queue.push(v.getNthItem(i)), false);
	}

	return true;
}

/*!
    This function does some extra optimatisation that is not found in the original xydiff
    algorithm

    Basically, we go over all unmatched leaves and see if any of those that are either
    text or data can be matched to any of those in the other tree

    this fuction returns true if further optimatization was done, false if no new matches
    were found

*/
bool PD_XyDiff::_optimizeContentLeaves()
{
	UT_GenericTree<PD_DocNode*>::Cursor c1(&m_tree1);
	UT_GenericTree<PD_DocNode*>::Cursor c2(&m_tree2);

	UT_GenericStringMap<PD_DocNode*> hashText;
	UT_GenericStringMap<PD_DocNode*> hashData;

	UT_GenericVector<PD_DocNode *>   vecMatches;

	UT_String s;
	
	for(c1.first(); c1.is_valid(); c1.next())
	{
		if(!c1.isLeaf())
			continue;
		
		PD_DocNode * n = c1.getContent();
		if(!n->isMatched())
		{
			if(n->getNodeType() == TEXT)
			{
				hashToStr(n->getMyHash(),s);
				hashText.insert(s, n);
			}
			else if(n->getNodeType() == DATA)
			{
				hashToStr(n->getMyHash(),s);
				hashData.insert(s, n);
			}
		}
	}
	
	for(c2.first(); c2.is_valid(); c2.next())
	{
		if(!c2.isLeaf())
			continue;
		
		PD_DocNode * m = c2.getContent();
		PD_DocNode * n = NULL;
		PD_DocNode * x;
		
		if(!m->isMatched())
		{
			if(m->getNodeType() == TEXT)
			{
				hashToStr(m->getMyHash(),s);
				n = hashText.pick(s);
				hashText.remove(s, x);
			}
			else if(m->getNodeType() == DATA)
			{
				hashToStr(m->getMyHash(),s);
				n = hashData.pick(s);
				hashData.remove(s, x);
			}

			if(n)
			{
				n->markMatched(m);
				m->markMatched(n);
				vecMatches.addItem(n);
			}
		}
	}

	// see if we can propagate our matches up stream
	for(UT_uint32 i = 0; i < vecMatches.getItemCount(); ++i)
	{
		PD_DocNode * n = vecMatches.getNthItem(i);
		UT_return_val_if_fail(n, false);
		
		PD_DocNode * m = n->getTheOther();
		UT_return_val_if_fail(m, false);

		c1.setPosition(n->getLevel(),n->getOffset());
		c2.setPosition(m->getLevel(),m->getOffset());
		c1.parent();
		c2.parent();
		
		for(;c1.is_valid() && c2.is_valid(); c1.parent(), c2.parent())
		{
			n = c1.getContent();
			m = c2.getContent();

			if(n->isMatched() || m->isMatched())
				break;

			if(n->getMyHash() != m->getMyHash())
				break;
			
			n->markMatched(m);
			m->markMatched(n);
		}
		
	}
	
	return true;
}

/*!
    This function computes the status of various nodes (inserted, deleted, modified, moved ...)
*/
bool PD_XyDiff::_computeAdjustments()
{
	UT_GenericTree<PD_DocNode*>::Cursor c1(&m_tree1);
	UT_GenericTree<PD_DocNode*>::Cursor c2(&m_tree2);

	UT_GenericStringMap<PD_DocNode*> hashText;
	UT_GenericStringMap<PD_DocNode*> hashData;

	UT_GenericVector<PD_DocNode *>   vecMatches;

	UT_String s;

	// iterate the source tree and mark all unmatched nodes as deleted ...
	for(c1.first(); c1.is_valid(); c1.next())
	{
		PD_DocNode * n = c1.getContent();
		if(n->isMatched())
		{
			// if a matched node has a single child and that child is an unmatched content node and
			// the matching node too has a single unmatched child of the same type, we will mark the
			// two children as MODIFIED
			PD_DocNode * m = n->getTheOther();

			UT_GenericTree<PD_DocNode*>::Cursor c3(&m_tree1);
			UT_GenericTree<PD_DocNode*>::Cursor c4(&m_tree2);
			c3.setPosition(n->getLevel(), n->getOffset());
			c4.setPosition(m->getLevel(), m->getOffset());

			c3.firstChild();
			c4.firstChild();
			if(c3.is_valid() && c4.is_valid())
			{
				PD_DocNode *ch1 = c3.getContent();
				PD_DocNode *ch2 = c4.getContent();

				c3.nextSibling();
				c4.nextSibling();
				if(!c3.is_valid() && !c4.is_valid() &&                               // only children
				   !ch1->isMatched() && !ch2->isMatched() &&                         // unmatched
				   ch1->isContentNode() && ch1->getNodeType() == ch2->getNodeType()) // of (content) type
				{
					if( ch1->getMyHash() != ch2->getMyHash())
					{
						ch1->setStatus(MODIFIED);
						ch2->setStatus(MODIFIED);
					}
					else
					{
						// this should not happen -- these two nodes should have been
						// matched by now
						UT_ASSERT_HARMLESS( UT_SHOULD_NOT_HAPPEN );
						ch1->setStatus(UNMODIFIED);
						ch2->setStatus(UNMODIFIED);
					}
					
					ch1->markMatched(ch2);
					ch2->markMatched(ch1);
				}
			}
		}
		else
		{
			n->setStatus(DELETED);
		}
	}

	// iterate the result tree and mark all unmatched nodes as inserted, and for all
	// matched nodes, determine if they have moved or not
	for(c2.first(); c2.is_valid(); c2.next())
	{
		PD_DocNode * m = c2.getContent();
		if(!m->isMatched())
		{
			m->setStatus(INSERTED);
		}
		else
		{
			PD_DocNode * n = m->getTheOther();
			if(m->getNodeType() == TEXT && n->getMyHash() != m->getMyHash())
			{
				if(n->getMyHash() != m->getMyHash())
				{
					n->setStatus(MODIFIED);
					m->setStatus(MODIFIED);
				}
			}
			else
			{
				// check whether the parents of the two nodes match, if not we have a move
				UT_GenericTree<PD_DocNode*>::Cursor c3(&m_tree1);
				UT_GenericTree<PD_DocNode*>::Cursor c4(&m_tree2);
				c3.setPosition(n->getLevel(), n->getOffset());
				c4.setPosition(m->getLevel(), m->getOffset());

				c3.parent();
				c4.parent();

				if((!c3.is_valid() && c4.is_valid()) || (c3.is_valid() && !c4.is_valid()))
				{
					// the nodes are on different levels ...
					// (in the original xydiff sources this is called STRONG MOVE)
					n->setStatus(MOVE_STRONG);
					m->setStatus(MOVE_STRONG);
				}

				if(c3.is_valid() && c4.is_valid())
				{
					PD_DocNode * n2 = c3.getContent();
					if(!n2->isMatched())
					{
						n->setStatus(MOVE_STRONG);
						m->setStatus(MOVE_STRONG);
					}
					else
					{
						PD_DocNode * m2 = c4.getContent();
						if(n2->getTheOther() != m2)
						{
							n->setStatus(MOVE_STRONG);
							m->setStatus(MOVE_STRONG);
						}
						else
						{
							// This is the case when the two parents match; there is still
							// the posibility that the two nodes come in different place
							// in the parent's subtree. We can use the offspring hash to
							// short circut some no-move cases; trying to
							// determine the rest on node by node basis is too
							// computationally intensive -- we will handle it in a separate
							// pass over the tree, but we will mark these nodes here as
							// potential candidates.
							if(n2->getOffspringHash() != m2->getOffspringHash())
							{
								// n->setStatus(POTENTIAL_INNER_MOVE);
								// m->setStatus(POTENTIAL_INNER_MOVE);
								n2->orStatus(POTENTIAL_REORDER);
								m2->orStatus(POTENTIAL_REORDER);
							}
						}
					}
				}
			}
		}
	}

	// now we need to determine moves within nodes ...
	// if there is a chance of inner moves within a node, then we need to use a common
	// sequence algorithm to work out what is the best way of reordering them
	for(c1.first(); c1.is_valid(); c1.next())
	{
		PD_DocNode * n = c1.getContent();
		if(n->potentialReorder())
		{
			_computeWeakMove(n);
		}
	}
	
	return true;
}


/*!
    This function tries to further refine matching achieved by the _matchAllNodes() call
*/
bool PD_XyDiff::_optimizeMatches()
{
	UT_GenericTree<PD_DocNode*>::Cursor c1(&m_tree1);
	c1.first();
	
	if(c1.is_valid())
		return _optimizeNode(c1.getContent());

	return false;
}

/*!
    We use two separate optimatisations on the node

    1. If the node is matched, we match any of its unmatched children to the children of
       node's match.

    2. If this node is not matched, we try to match it to the parent of its heaviest
       matched child.
*/
bool PD_XyDiff::_optimizeNode(PD_DocNode * n1)
{
	UT_return_val_if_fail( n1, false );
	UT_GenericTree<PD_DocNode*>::Cursor c1(&m_tree1);
	UT_GenericTree<PD_DocNode*>::Cursor c2(&m_tree2);

	if(n1->isMatched())
	{
		// we create a map of hashes of this nodes children and try to match them to any
		// unassigned nodes in the other tree
		UT_GenericStringMap<UT_GenericVector<PD_XyDiffNodeMatch *>*> hashNodes;
	
		UT_String s;

		// work over the first tree, and insert any id nodes into the hash
		c1.setPosition(n1->getLevel(),n1->getOffset());
		c1.firstChild();
	
		if(!c1.is_valid())
			return true;
	
		for(; c1.is_valid(); c1.nextSibling())
		{
			UT_return_val_if_fail( c1.getContent(), false);
		
			if(c1.getContent()->isMatched() || c1.getContent()->hasNoMatch())
			{
				// we have already processed this node
				continue;
			}
		
			PD_XyDiffNodeMatch * n = new PD_XyDiffNodeMatch;
			UT_return_val_if_fail( n, false );

			n->node1 = c1.getContent();
			n->node2 = NULL;
			hashToStr(n->node1->getMyHash(),s);

			UT_GenericVector<PD_XyDiffNodeMatch *> * pV;
			// NB: the hash contains() method is broken, we need to get the contents by a
			// separate call to pick()
			if(hashNodes.contains(s, NULL))
			{
				pV = hashNodes.pick(s);
				UT_return_val_if_fail( pV, false );

				pV->addItem(n);
			}
			else
			{
				pV = new UT_GenericVector<PD_XyDiffNodeMatch *>;
				UT_return_val_if_fail( pV, false );
				pV->addItem(n);
				hashNodes.insert(s, pV);
			}
		
		}

		if(hashNodes.size())
		{
			// now sort the vectors in the hash by their weight
			UT_GenericStringMap<UT_GenericVector<PD_XyDiffNodeMatch *>*>::UT_Cursor c3(&hashNodes);
			UT_GenericVector<PD_XyDiffNodeMatch *> * pV;
			for(pV = c3.first(); c3.is_valid(); c3.next())
			{
				pV->qsort(s_compare_weight);
			}
	
			PD_DocNode * n2 = n1->getTheOther();
			UT_return_val_if_fail( n2, false );
		
			// now work over the second tree and look for suitable nodes ...
			c2.setPosition(n2->getLevel(),n2->getOffset());
			c2.firstChild();
			for(; c2.is_valid(); c2.nextSibling())
			{
				UT_return_val_if_fail( c2.getContent(), false );

				if(c2.getContent()->isMatched() || c2.getContent()->hasNoMatch())
				{
					// we have already processed this node
					continue;
				}

				hashToStr(c2.getContent()->getMyHash(), s);

				UT_GenericVector<PD_XyDiffNodeMatch *> * pV = hashNodes.pick(s);

				if(pV && pV->getItemCount())
				{
					PD_XyDiffNodeMatch * n = pV->getNthItem(0);
					if(!n)
					{
						// this node has no match
						// this is probably not necessary
						// c2.getContent()->setNoMatch(true);
					}
					else
					{
						n->node2 = c2.getContent();
						n->node1->markMatched(n->node2);
						n->node2->markMatched(n->node1);

						// now remove this node from the vector
						delete n;
						pV->deleteNthItem(0);
					}
				}
				else
				{
					// this node has no match
					// I am not sure whether we should mark it as having no match, probably
					// should insert it into the queue
					// c2.getContent()->setNoMatch(true);
				}
			}

			// clean up
			for( pV = c3.first(); c3.is_valid(); c3.next())
			{
				for(UT_uint32 j = 0; j < pV->getItemCount(); ++j)
				{
					PD_XyDiffNodeMatch * n = pV->getNthItem(j);
					delete n;
				}
		
				delete pV;
				// we do not call hashNodes.remove() here ...
			}
		}
	}
	else
	{
		// this node does not have any matches; we check its children to see if any of
		// them have matches, and if so we match this node to the parent of the haviest
		// matched child
		c1.setPosition(n1->getLevel(),n1->getOffset());
		double fMaxWeight = 0.0;
		PD_DocNode * hc = NULL;
		
		for(c1.firstChild(); c1.is_valid(); c1.nextSibling())
		{
			PD_DocNode * m1 = c1.getContent();
			UT_return_val_if_fail( m1, false );

			if(m1->isMatched() && m1->getWeight() > fMaxWeight)
			{
				fMaxWeight = m1->getWeight();
				hc = m1;
			}
		}
		
		if(hc)
		{
			PD_DocNode * m2 = hc->getTheOther();
			UT_return_val_if_fail( m2, false );

			c2.setPosition(m2->getLevel(), m2->getOffset());
			if(c2.parent())
			{
				PD_DocNode * n2 = c2.getContent();
				UT_return_val_if_fail( n2, false );
				n1->markMatched(n2);
				n2->markMatched(n1);

				// not 100% sure about this, but it seems sensible to do children matches
				// based on the new match
				UT_return_val_if_fail(_optimizeNode(n1), false);
			}
			
		}
	}
	
	// now we call ourselves recursively on our children
	c1.setPosition(n1->getLevel(),n1->getOffset());

	for(c1.firstChild(); c1.is_valid(); c1.nextSibling())
	{
		UT_return_val_if_fail(_optimizeNode(c1.getContent()), false);
	}

	return true;
}

/*!
    Process the entire queue of tree nodes
 */
bool PD_XyDiff::_matchAllNodes()
{
	// process the entire quueue
	PD_DocNode * n = NULL;
	while(m_queue.pop(n))
	{
		UT_return_val_if_fail(_matchNode(n), false);
	}

	return true;
}


/*!
    Try to match given node (from tree2) to a node in tree1
*/
bool PD_XyDiff::_matchNode(PD_DocNode * n2)
{
	UT_return_val_if_fail( n2, false );
	UT_return_val_if_fail( m_fWeight2 != 0.0, false );

	// a list of candiates from the original tree
	UT_GenericVector<PD_DocNode*> vCandidates;
	UT_GenericTree<PD_DocNode*>::Cursor c2(&m_tree2);
	
	// any xid-nodes are to be ingored
	if(n2->isMatched())
		return true;
	
	if(n2->hasNoMatch())
		goto nomatch;
	
	{ // force limited scope for sake of goto

		// the matching is based on ancestry to a specific level; if this node does not have enough
		// ancestors, then we can move on early

		// work out the depth to which we need to match ancestry
		double fMaxDepth = 1.0 + 5.0 * log((double)m_iNodeCount2) * n2->getWeight()/m_fWeight2;

		UT_uint32 iMaxDepth = (UT_uint32)fMaxDepth;

		if(fMaxDepth - (double)iMaxDepth > 0.0)
			iMaxDepth++;

		if(n2->getLevel() < iMaxDepth)
		{
			// not enough ancestors ...
			return true;
		}

		// now we need to construct a list of candiates from the original tree
		// construct a cursor for tree1 so we can traverse over it
		UT_GenericTree<PD_DocNode*>::Cursor c1(&m_tree1);

		for(c1.first(); c1.is_valid(); c1.next())
		{
			// any id-matched nodes are out of considereation as are already matched nodes
			if(c1.getContent()->isMatched() || c1.getContent()->hasNoMatch())
				continue;

			// we match the hashes, but if the node is not deep enough in the tree, we will
			// exclude it
			if(c1.getContent()->getMyHash() == n2->getMyHash() && c1.getContent()->getLevel() >= iMaxDepth)
				vCandidates.addItem(c1.getContent());
		}

		if(vCandidates.getItemCount() == 0)
			goto nomatch;

		// sort the candiate list by weight
		vCandidates.qsort(s_compare_weight);

		// construct tree cursor and position it to our node n2
		c2.setPosition(n2->getLevel(),n2->getOffset());
		UT_return_val_if_fail( c2.is_valid(),false );
	
		UT_GenericTree<PD_DocNode*>::Cursor c3(&m_tree1);
	
		PD_DocNode * m2 = c2.getContent();

		// m2 should be guaranteed, but assert when in doubt
		UT_return_val_if_fail(m2,false);

		UT_uint32 iDepth = 0;
		bool bSuccess = true;

		PD_DocNode * n1 = NULL;

		// now we work through our nodes ancestors and see if any of the candidates has
		// ancestors that match them
		while(iDepth < iMaxDepth && bSuccess)
		{
			bSuccess = false; // be pesimistic
		
			// position the cursor on the previously processed node
			c2.setPosition(m2->getLevel(),m2->getOffset());
			UT_return_val_if_fail( c2.is_valid(),false );
		
			if(!c2.parent())
			{
				// the  node has no parent for this level, we cannot match
				goto nomatch;
			}

			m2 = c2.getContent();
			if(!m2 || m2->hasNoMatch() || !m2->isMatched())
			{
				// the node's parent has no match in the document (it has xid which is not
				// matched)
				goto nomatch;
			}

			// OK, check the parents of our candidates
			for(UT_sint32 i = 0; i < (UT_sint32)vCandidates.getItemCount(); ++i)
			{
				n1 = vCandidates.getNthItem(i);
				PD_DocNode * m1 = n1;
				UT_return_val_if_fail( n1, false );

				// does this node have adequate level to match ?
				if(n1->getLevel() < iMaxDepth - iDepth)
				{
					// this node will not match in any further iterations of the outer loop,
					// so we will remove it from the candiate list
					vCandidates.deleteNthItem(i);
					i--; // to compensate for the loop increment
					continue;
				}

				c3.setPosition(n1->getLevel(),n1->getOffset());
				UT_return_val_if_fail(c3.is_valid(), false );
				UT_return_val_if_fail(c3.parent(), false );

				m1 = c3.getContent();
				UT_return_val_if_fail( m1,false );

				if(!m1->isMatched())
				{
					// this node will not match in any further iterations of the outer loop,
					// so we will remove it from the candiate list
					vCandidates.deleteNthItem(i);
					i--; // to compensate for the loop increment
					continue;
				}

				bSuccess = true;
			}

			iDepth++;
		}

		if(bSuccess && iDepth != 0)
		{
			// our node matches
			UT_return_val_if_fail( _recursiveMatch(n1,n2), false);
			return true;
		}
	}
	
 nomatch:
	// we did not find match for this node, so we add its children to the queue
	vCandidates.clear();
	
	for(c2.firstChild(); c2.is_valid(); c2.nextSibling())
	{
		vCandidates.addItem(c2.getContent());
	}

	_addNodesToQueue(vCandidates);
	return true;
}

/*!
    This function tries to recursively match children nodes of the two nodes passed to it
    (n1 and n2 have to be matching); for any two children that match, we attempt to match
    their children, and so on.
 */
bool PD_XyDiff::_recursiveMatch(PD_DocNode * n1, PD_DocNode * n2)
{
	UT_return_val_if_fail( n1 && n2, false );
	
	n2->markMatched(n1);
	n1->markMatched(n2);

	// now we walk over the children of the two nodes and if any of the nodes at identical
	// positions have same hashes, we match them

	UT_GenericTree<PD_DocNode*>::Cursor c1(&m_tree1);
	c1.setPosition(n1->getLevel(),n1->getOffset());

	UT_GenericTree<PD_DocNode*>::Cursor c2(&m_tree2);
	c2.setPosition(n2->getLevel(),n2->getOffset());

	for(c1.firstChild(), c2.firstChild(); c1.is_valid() && c2.is_valid(); c1.nextSibling(), c2.nextSibling())
	{
		PD_DocNode * m1 = c1.getContent();
		PD_DocNode * m2 = c2.getContent();
		
		UT_return_val_if_fail( m1 && m2, false );
		
		if(!m1->isMatched()  && !m2->isMatched()  &&
		   !m1->hasNoMatch() && !m2->hasNoMatch() &&
		    m1->getMyHash() == m2->getMyHash())
		{
			_recursiveMatch(m1, m2);
		}
	}
		
	return true;
}



/*!
    initialiase the processing queue with the level 0 nodes
 */
bool PD_XyDiff::_constructPriorityQueue()
{
	// we construct initial queue from the 0-level nodes of tree2
	UT_GenericVector<PD_DocNode*> vNodes;
	UT_return_val_if_fail( m_tree2.getNodesForLevel(0, vNodes), false);
	m_queue.clear();

	return _addNodesToQueue(vNodes);
}

/*!
    This function iterates over the given tree and calculates hashes and weights for its
    nodes; it also set the coords of the nodes in the tree. fWeight is a variable in which
    the cumulative weight of the entire tree is returned
 
*/
bool PD_XyDiff::_calculateHashesAndWeights(UT_GenericTree<PD_DocNode *> & tree, double &fWeight)
{
	UT_sint32 iMaxLevel = tree.getMaxNodeLevel();
	UT_return_val_if_fail( iMaxLevel >= 0, false );
	fWeight = 0.0;
	
	for(UT_sint32 i = iMaxLevel; i >= 0; --i)
	{
		UT_uint32 iNodeCount = tree.getNodeCountForLevel(i);
		for(UT_uint32 j = 0; j < iNodeCount; j++)
		{
			PD_DocNode * n = tree.getNthNodeForLevel(i,j);
			UT_return_val_if_fail( n, false );

			// we set the coords of this node; we do it here out of convenience since we
			// are already running the two loops over the entire tree
			// (the coords will be needed later when working with the priority queue)
			n->setLevel(i);
			n->setOffset(j);
			
			// calculate hash and weight for this node

			// the stand-alone hash and weight are set at creation
			// n->setMyHash(???);
			// n->setWeight(1.0);
			
			UT_GenericTree<PD_DocNode*>::Cursor c(&tree);
			c.setPosition(i,j);

			UT_String sHash;
			hashToStr(n->getMyHash(),sHash);

			UT_String sOffHash;
			
			// now iterate our subtree and process hashes and weights
			for(c.firstChild(); c.is_valid(); c.nextSibling())
			{
				// we have calculate these in the previous iteration of the outside loop,
				// just need to factor them into the parent
				PD_DocNode * m = c.getContent();
				UT_String sH;
				hashToStr(m->getMyHash(), sH);

				sOffHash += sH;
				
				n->addWeight(m->getWeight());
			}

			if(sOffHash.size())
				n->setOffspringHash(UT_hash64(sOffHash.c_str()));
			else
				n->setOffspringHash(0);
			
			sHash += sOffHash;
			n->setSubtreeHash(UT_hash64(sHash.c_str()));
			
			fWeight += n->getWeight();
		}
	}
	
	return true;
}

/*!
    Work over the two trees, matching nodes that share xid
*/
bool PD_XyDiff::_locateIdNodes()
{
	// work over the first tree, and insert any id nodes into the hash
	UT_String s;

	UT_GenericTree<PD_DocNode*>::Cursor c1(&m_tree1);
	for(c1.first(); c1.is_valid(); c1.next())
	{
		UT_return_val_if_fail( c1.getContent(), false);
		UT_uint32 iXID = c1.getContent()->getXID();
		if(iXID)
		{
			PD_XyDiffNodeMatch * n = new PD_XyDiffNodeMatch;
			UT_return_val_if_fail( n, false );

			n->node1 = c1.getContent();
			n->node2 = NULL;

			UT_String_sprintf(s, "%d", iXID);

			m_hashNodes.insert(s, n);
		}
	}

	UT_GenericTree<PD_DocNode*>::Cursor c2(&m_tree2);
	for(c2.first(); c2.is_valid(); c2.next())
	{
		m_iNodeCount2++;
		
		UT_return_val_if_fail( c2.getContent(), false );
		UT_uint32 iXID = c2.getContent()->getXID();
		if(iXID)
		{
			UT_String_sprintf(s, "%d", iXID);
			PD_XyDiffNodeMatch * n = m_hashNodes.pick(s);

			if(!n)
			{
				// this node has no match
				c2.getContent()->setNoMatch(true);
			}
			else
			{
				n->node2 = c2.getContent();
				n->node1->markMatched(n->node2);
				n->node2->markMatched(n->node1);
			}
		}
	}

	// now we traverse over the hash, eliminating any nodes from the first tree that
	// do no have a match
	UT_GenericStringMap<PD_XyDiffNodeMatch *>::UT_Cursor c3(&m_hashNodes);
	for(PD_XyDiffNodeMatch * n = c3.first(); c3.is_valid(); c3.next())
	{
		UT_return_val_if_fail( n, false );
		if(!n->node2)
		{
			n->node1->setNoMatch(true);
			UT_String_sprintf(s, "%d", n->node1->getXID());
			PD_XyDiffNodeMatch * n2;
			m_hashNodes.remove(s, n2);
			delete n;
		}
	}
	
	return true;
}

bool PD_XyDiff::_buildDocTree(pt_PieceTable * pPT, UT_GenericTree<PD_DocNode*> &tree)
{
	UT_return_val_if_fail( pPT, false );

	pf_Fragments & frags = pPT->getFragments();
	UT_Stack stackFrag;
	UT_sint32 iRelationship = 0;
	
	for(UT_uint32 i = 0; i < frags.getNumberOfFrags(); ++i)
	{
		pf_Frag * pf = frags.getNthFrag(i);
		UT_return_val_if_fail( pf, false );
		
		pf_Frag::PFType eType = pf->getType();

		if(eType == pf_Frag::PFT_FmtMark || eType == pf_Frag::PFT_EndOfDoc)
			continue;
		
		UT_uint64 iHash = 0;
		UT_uint32 iLinearWeight = 0;
		// TODO -- calculate hash and weight here ... 

		PD_DocNode * n = new PD_DocNode(pf, iHash, iLinearWeight);
		UT_return_val_if_fail( n, false );

		if(eType == pf_Frag::PFT_Strux)
		{
			pf_Frag_Strux * pfs = (pf_Frag_Strux*) pf;
			switch(pfs->getStruxType())
			{
				case PTX_Section:
				{
					pf_Frag * p;
					bool bRet = stackFrag.viewTop((void**)&p);
					iRelationship = 0; // assume sibling to start with

					while(bRet)
					{
						if(p->getType() == pf_Frag::PFT_Strux)
						{
							pf_Frag_Strux * s = (pf_Frag_Strux *) p;
							if(s->getStruxType() == PTX_Section)
							{
								bRet = stackFrag.pop((void**)&p);
								break;
							}
							else if(s->getStruxType() == PTX_SectionTable ||
									s->getStruxType() == PTX_SectionCell)
							{
							}
							
							
						}

						bRet = stackFrag.pop((void**)&p);
					}
					
				}
					
				case PTX_Block:
				{
					pf_Frag * p;
					bool bRet = stackFrag.viewTop((void**)&p);
					UT_ASSERT_HARMLESS( bRet );

					if(bRet)
					{
						if(p->getType() != pf_Frag::PFT_Strux || pPT->isEndFootnote(p))
							iRelationship = -1; // sibling to the previous block
						else
						{
							pf_Frag_Strux * s = (pf_Frag_Strux *) p;
							if(s->getStruxType() != PTX_Block)
								iRelationship = 0; // sibling
							else
								iRelationship = 1; // child
						}
					}

					// now pop what we can from the stack
					while (bRet)
					{
						if( p->getType() == pf_Frag::PFT_Strux)
						{
							pf_Frag_Strux * s = (pf_Frag_Strux *) p;
							
							if(s->getStruxType() == PTX_Section ||
							   s->getStruxType() == PTX_SectionHdrFtr)
							{
								break;
							}
							else if(s->getStruxType() == PTX_Block)
							{
								stackFrag.pop((void**)&p);
								break;
							}
							
						}

						bRet = stackFrag.pop((void**)&p);
					}

					// there should always be a section before a block ...
					UT_ASSERT_HARMLESS( bRet );
					stackFrag.push((void*)pf);
				}
				break;
				
				case PTX_SectionHdrFtr:
				case PTX_SectionEndnote:
				case PTX_SectionTable:
				case PTX_SectionCell:
				case PTX_SectionFootnote:
				case PTX_SectionMarginnote:
				case PTX_SectionFrame:
				case PTX_SectionTOC:
					stackFrag.push((void*)pf);
					break;
					
				case PTX_EndCell:
				case PTX_EndTable:
				case PTX_EndFootnote:
				case PTX_EndMarginnote:
				case PTX_EndEndnote:
				case PTX_EndFrame:
				case PTX_EndTOC:
				{
					pf_Frag * p;
					bool bFoundMatch = false;
					
					while (stackFrag.pop(((void**)&p)))
					{
						if(pfs->isMatchingType(p))
						{
							bFoundMatch = true;
							break;
						}
					}

					UT_ASSERT_HARMLESS( bFoundMatch );
				}
					
				case PTX_StruxDummy:
					break;
			}
		}
		else
		{
			// text or object
			pf_Frag * p;
			bool bRet = stackFrag.viewTop((void**)&p);
			UT_ASSERT_HARMLESS( bRet );

			if(bRet)
			{
				if(p->getType() == pf_Frag::PFT_Strux)
					iRelationship = 1; // child
				else
					iRelationship = 0; // sibling
			}
			stackFrag.push((void*)pf);
		}
		
		tree.insert(n, iRelationship);
	}
	
	return true;
}

bool PD_XyDiff::apply()
{
	UT_ASSERT_HARMLESS( 0 );
	UT_GenericTree<PD_DocNode*>::Cursor c1(&m_tree1);
	UT_GenericTree<PD_DocNode*>::Cursor c2(&m_tree2);

	UT_return_val_if_fail( c1.first() && c2.first(), false );

	pt_PieceTable * pPt1 = c1.getContent()->getFrag()->getPieceTable();
	pt_PieceTable * pPt2 = c2.getContent()->getFrag()->getPieceTable();

	UT_return_val_if_fail( pPt1 && pPt2, false );

	// make sure the doc coords are correct ...
	// (we only care about the original doc)
	pPt1->getFragments().cleanFrags();
	
	// we will do this in several passes

	// PASS 1: remove all nodes marked DELETED from tree1
	//         remove all nodes marked MOVED from tree1
	// we do it in reverse order, so as to process children before their parents
	// it also means that we do not need to worry about the doc coords getting messed up
	for(c1.last(); c1.is_valid(); c1.prev())
	{
		PD_DocNode * n = c1.getContent();

		if(n->getStatus() != DELETED &&
		   n->getStatus() != MOVE_STRONG &&
		   n->getStatus() != MOVE_WEAK)
			continue;
		
		pf_Frag *pf = n->getFrag();
		UT_return_val_if_fail( pf, false );
		
		PT_DocPosition dpos = pf->getPos();
		UT_uint32 iLen  = pf->getLength();
		UT_uint32 iDelCount;
		pPt1->deleteSpan(dpos, dpos + iLen, NULL, iDelCount);
	}

	// fix up doc coords again 
	pPt1->getFragments().cleanFrags();
	
	// insert anything inserted from the second doc
	// insert anything that has moved
	for(c2.last(); c2.is_valid(); c2.prev())
	{
		PD_DocNode * n = c2.getContent();

		if(n->getStatus() != INSERTED &&
		   n->getStatus() != MOVE_STRONG &&
		   n->getStatus() != MOVE_WEAK)
			continue;
		
		pf_Frag *pf = n->getFrag();
		UT_return_val_if_fail( pf, false );

		// Now work out the dpos in doc1 where this node is to be inserted
		// We will look back in doc2 for the first frag that has not moved and insert this
		// node just after it
		PT_DocPosition dpos = 0;

		// save the cursor position so we can restore it later
		c2.save();
		PD_DocNode * m = NULL;
		
		while(c2.prev())
		{
			m = c2.getContent();

			// see if the node is unmoved
			if(m->getStatus() == UNMODIFIED || m->getStatus() == MODIFIED)
				break;
		}

		if(c2.is_valid())
		{
			UT_return_val_if_fail( m, false );
			
			// m is the previous umoved node, we will insert our node after its match
			PD_DocNode * u = m->getTheOther();
			UT_return_val_if_fail( u, false );

			pf_Frag * pfu = u->getFrag();
			UT_return_val_if_fail( pfu, false );

			dpos = pfu->getPos() + pfu->getLength();
		}

		c2.restore();
		
		PT_AttrPropIndex api = pf->getIndexAP();
		const PP_AttrProp * pAP2;
		UT_return_val_if_fail(pPt2->getAttrProp(api, &pAP2), false);

		const gchar ** pAttrs2 = pAP2->getAttributes();
		const gchar ** pProps2 = pAP2->getProperties();

		switch(pf->getType())
		{
			case pf_Frag::PFT_Text:
			{
				pf_Frag_Text * pft = (pf_Frag_Text*)pf;
				PT_BufIndex bi = pft->getBufIndex();
				const UT_UCS4Char * p = pPt2->getPointer(bi);
				pPt1->insertSpan(dpos, p, pft->getLength());
				pPt1->changeSpanFmt(PTC_SetFmt, dpos, dpos + pft->getLength(), pAttrs2, pProps2);
			}
			break;
			
			case pf_Frag::PFT_Object: 
			{
				pf_Frag_Object * pfo = (pf_Frag_Object*) pf;
				pPt1->insertObject(dpos, pfo->getObjectType(),pAttrs2,pProps2);
			}
			break;

			case pf_Frag::PFT_Strux: 
			{
				pf_Frag_Strux * pfs = (pf_Frag_Strux*) pf;
				pPt1->insertStrux(dpos, pfs->getStruxType(),pAttrs2,pProps2);
			}
			break;
			
			default:;
		}
	}
	
	return true;
}


/*!
    Comparison function used by _lCss()
*/
static inline char min3(double a, double b, double c)
{
	if(a<=b)
	{
		if(a<=c)
			return 'A';
		
		if(c<=a)
			return 'C';
	}
	
	if(b<=c)
		return 'B';
	
	return 'C';
}

/*!
    This function tries to locate the least expensive transformation through which s1 can
    be transformed into s2.

    This code is pretty much as I found it in xydiff.
*/
bool PD_XyDiff::_lCss(UT_GenericVector<wSequence*> & s1, UT_GenericVector<wSequence*> & s2)
{
	// init
	
	UT_uint32 l1 = s1.size() ;
	UT_uint32 l2 = s2.size() ;
	
	double** fCost    = new double*[l2];
	char**   iOrigin = new char* [l2];
	
	UT_uint32 i, j;
	for(j = 0; j < l2; j++)
	{
		fCost[j]  = new double[l1];
		iOrigin[j]= new char [l1];
	}
	
	// Transforming empty string into empty string has no fCost
	fCost[0][0]=0.0 ;
	iOrigin[0][0]='Z' ;

	// Transforming parts of S1 into empty string is just deleting

	for(i = 1; i < l1; i++)
	{
		fCost[0][i] = fCost[0][i-1] + s1[i]->weight;
		iOrigin[0][i] = 'A';
	}
	
	// Transforming empty string into parts of S2 is just inserting
	for(j = 1; j < l2; j++)
	{
		fCost[j][0] = fCost[j-1][0] + s2[j]->weight;
		iOrigin[j][0] = 'B';
	}

	// compute cost for paths which transform S1 into S2
	
	for(j = 1; j < l2; j++)
	{
		for(i = 1; i < l1; i++)
		{
			UT_DEBUGMSG(("Computing cost(i=%3d,j=%3d)\n", i, j));

			// delete item i in S1 and use tranformation of  S1[1..i-1] into S2[1..j]
			double fDeleteCost = s1[i]->weight + fCost[j][i-1];
			UT_DEBUGMSG(("    delete cost is %f\n", fDeleteCost ));
			// use tranformation of  S1[1..i] into S2[1..j-1] and insert S2[j]
			double fInsertCost = fCost[j-1][i] + s2[j]->weight;
			UT_DEBUGMSG(("    insert cost is %f\n", fInsertCost ));
			
			if (s1[i]->data != s2[j]->data)
			{
				if (fDeleteCost < fInsertCost)
				{
					fCost[j][i] = fDeleteCost ;
					iOrigin[j][i] = 'A' ;
				}
				else
				{
					fCost[j][i] = fInsertCost ;
					iOrigin[j][i] = 'B' ;
				}
			}
			else
			{
				double fKeep = fCost[j-1][i-1] ;
				UT_DEBUGMSG(("    keep cost is %f\n", fKeep ));
				iOrigin[j][i] = min3( fDeleteCost, fInsertCost, fKeep );
				
				if(iOrigin[j][i] == 'A')
					fCost[j][i] = fDeleteCost;
				else if (iOrigin[j][i] == 'B')
					fCost[j][i] = fInsertCost;
				else if (iOrigin[j][i] == 'C')
					fCost[j][i] = fKeep;
			}
				
			UT_DEBUGMSG(("cost(i=%3d,j=%3d)=%f, via %c\n", i, j, fCost[j][i], iOrigin[j][i]));
		}
	}
	
	// trace best path
	i = l1-1 ;
	j = l2-1 ;
	UT_uint32 balance = 0 ; // Delete/Insert balance
	
	while((i > 0)||(j > 0))
	{
		UT_return_val_if_fail((i < 0)||(j < 0), false);
		
		UT_DEBUGMSG(("at (i=%3d,j=%3d) fCost=%f iOrigin=%c\n", i, j, fCost[j][i], iOrigin[j][i]));
		switch(iOrigin[j][i])
		{
			case 'A':
				// Delete
				s1[i]->data = 0;
				i-- ;
				balance++ ;
				break;
			case 'B':
				// Insert
				s2[j]->data = 0;
				j-- ;
				balance-- ;
				break;
			case 'C':
				// Keep
				UT_return_val_if_fail(s1[i]->data == s2[j]->data, false);
				i-- ;
				j-- ;
				break;
		}
	}
	
	if (balance != 0)
	{
		UT_DEBUGMSG(("LongestCommonSubSequence Warning: Insert/Delete balance is not null!\n"));
	}
	
	for(j = 0; j < l2; j++)
	{
		delete [] fCost[j];
		delete [] iOrigin[j];
	}
	
	delete [] fCost;
	delete [] iOrigin;

	return true;
}

/*!
    This function tries to locate transformation through which s1 can be transformed into
    s2. In contrast to _lCss() it does not care haw expensive the tranformation is (we
    use it when the subtrees are too large for the full-blown _lCss() search).

    This code is pretty much as I found it in xydiff.
*/
bool PD_XyDiff::_easyCss(UT_GenericVector<wSequence*> & s1, UT_GenericVector<wSequence*> & s2)
{

	UT_uint32 i = 1 ;
	UT_uint32 j = 1 ;
	
	while((j < s2.size()) && (i < s1.size()))
	{
		if (s1[i]->data == 0)
		{
			UT_DEBUGMSG(("s1[%d] already removed\n", i));
			i++;
		}
		else if (s2[j]->data == 0)
		{
			UT_DEBUGMSG(("s2[%d] already removed... but how is that possible????\n", j));
			j++;
		}
		else if (s1[i]->data == s2[j]->data)
		{
			UT_DEBUGMSG(("Ok s1[%d] == s2[%d]\n", i, j));
			i++;
			j++;
		}
		else
		{
			UT_DEBUGMSG(("throwing away s2[%d]\n", j));
			UT_uint32 x = s2[j]->data;

			UT_return_val_if_fail( x >= s1.size(), false );

			s1[x]->data = 0;
			s2[j]->data = 0;
			
			j++ ;
		}
	}

	return true;
}

/*!
    Compute weak moves (moves within parent node) for the given node

    NB: the equivalent function in the original impl. works recursively over childred, we
    do not. We have already marked any subrees that need to be reordered and call
    _computeWeakMove() for the marked subtrees from a loop that iterates over the entire
    tree. Because the status of the children does not affect the statuc of the parent, we
    can do this (if the status of the children affected the status of the parent, we would
    just need to iterate over the tree from bottom up).
*/
bool PD_XyDiff::_computeWeakMove(PD_DocNode *n)
{
	UT_return_val_if_fail( n, false );

	n->resetPotentialReorder();
	
	UT_GenericTree<PD_DocNode*>::Cursor c1(&m_tree1);
	c1.setPosition(n->getLevel(), n->getOffset());

	UT_return_val_if_fail(!c1.is_valid(), false);
	c1.firstChild();

	if(!c1.is_valid())
		return true;

#if 0
	// The original impl. does here full recursive application to children.
	// We not do this -- see note above
	while(c1.is_valid())
	{
		PD_DocNode* ch1 = c1.getContent();
	   _computeWeakMove(ch1);

		c1.nextSibling();
	}
#endif
	
	// now apply this to the node itself
	if(!n->isMatched())
		return true;

	c1.setPosition(n->getLevel(), n->getOffset());
	UT_return_val_if_fail(!c1.is_valid(), false);

	c1.firstChild();
	UT_return_val_if_fail(c1.is_valid(), false);
	
	UT_GenericVector<wSequence*> s0; // old child value
	s0.addItem(new wSequence(-1, 999000.0));
	UT_sint32 iIndx = 1;

	UT_GenericVector<PD_DocNode*> s2Matches; // need this to be able to match items in s2
											 // to items in s0

	s2Matches.addItem(NULL);
	
	do
	{
		PD_DocNode * ch1 = c1.getContent();
		if(ch1->getStatus() == UNMODIFIED)
		{
			s0.addItem(new wSequence(iIndx++,ch1->getWeight()));
		}
		else
		{
			s0.addItem(new wSequence(0, 998000.0));
		}
		
		s2Matches.addItem(ch1->getTheOther());
		
		c1.nextSibling();
	}
	while(c1.is_valid());

	
	UT_GenericVector<wSequence*> s1; // original sequence
	
	s1.addItem(new wSequence(-1, 997000.0));

	UT_uint32 i;
	for(i = 1; i < s0.size(); i++)
		if (s0[i]->data)
			s1.addItem( s0[i] );


	PD_DocNode * m = n->getTheOther();
	
	UT_GenericTree<PD_DocNode*>::Cursor c2(&m_tree2);
	c2.setPosition(m->getLevel(), m->getOffset());
	UT_return_val_if_fail(!c2.is_valid(), false);

	c2.firstChild();
	UT_return_val_if_fail(!c2.is_valid(), false);

	UT_GenericVector<wSequence*> s2; // final sequence
	s2.addItem(new wSequence(-1, 996000.0));

	do
	{
		PD_DocNode * ch2 = c2.getContent();
		if(ch2->getStatus() == UNMODIFIED)
		{
			UT_sint32 iOrigIndx = s2Matches.findItem(ch2);
			UT_return_val_if_fail( iOrigIndx >= 0, false );
			
			s2.addItem(new wSequence(s0[iOrigIndx]->data, ch2->getWeight()));
		}

		c2.nextSibling();
	}
	while(c2.is_valid());
		
	if (s1.size() < 200) // was 100 in orginal xydiff impl.
	{
		UT_DEBUGMSG(("using lcss algorithm...\n"));
		_lCss(s1, s2);
	}
	else
	{
		UT_DEBUGMSG(("sequence too long, using quick subsequence finder.\n"));
		_easyCss(s1, s2) ;
	}


	c1.setPosition(n->getLevel(), n->getOffset());
	UT_return_val_if_fail(!c1.is_valid(), false);

	c1.firstChild();
	UT_return_val_if_fail( c1.is_valid(), false );

	iIndx = 1;
	
	while(c1.is_valid())
	{
		PD_DocNode * ch1 = c1.getContent();
		if(ch1->getStatus() == UNMODIFIED)
		{
			if(s1[iIndx]->data == 0)
			{
				ch1->setStatus(MOVE_WEAK);
				PD_DocNode * ch2 = ch1->getTheOther();
				UT_return_val_if_fail( ch2, false );

				ch2->setStatus(MOVE_WEAK);
			}

			iIndx++;
		}
	}

	UT_VECTOR_PURGEALL(wSequence*, s0);
	UT_VECTOR_PURGEALL(wSequence*, s1);
	UT_VECTOR_PURGEALL(wSequence*, s2);
	UT_VECTOR_PURGEALL(PD_DocNode*, s2Matches);

	return true;
}

