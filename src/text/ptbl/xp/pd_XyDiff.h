/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (c) 2004 Tomas Frydrych <tomasfrydrych@yahoo.co.uk>
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




/* This is a re-implementation of the XyDiff algorithm (designed by Gregory
 * Cobena, Serge Abiteboul and Amelie Marian at INRIA, http://www.inria.fr/).
 *
 * Most of the code has been written from scratch to remove dependency on the xercesc
 * library and generally to make it easier to plug it into AW piecetable (the original
 * implementation starts with parsing an actuall xml document through xercesc), but the
 * original INRIA implementation was consulted extensively, and where the published
 * description of the algorithm varied from the published implementation, the
 * implemenation was followed.
 *
 * The algorithm has been modified at some points:
 * 
 *   1. The LUT optimatisation for matching children and parents by weight was omitted
 *      as performance is less critical to us than readability of the code, at least for
 *      now (i.e., if the user has two really big documents she is trying to merge, it
 *      does not greatly matter if it takes, say, 20 seconds to do).
 *
 *   2. An additional optimatisation step has been added to the end of the matching
 *      process in which we attempt to match unmatched text and data leaves across the
 *      whole document; this is based on the observation that in wordprocessing document
 *      moving text around is a common procedure that deserves extra effort.
 *
 *   3. Some of the recursive function calls have been unrolled into loops; this is mostly
 *      for debugging purposes, as with recursive calls it can be difficult to keep track
 *      of where on the recursive stack we are (I am generally not too fond of recursion as
 *      the stack size can get out of hand and stack overflow bugs can be hard to debug, but
 *      in this case that probably is not a huge problem as our document trees tend to be
 *      broad rather than tall).
 *
 */


#ifndef PD_XYDIFF_H
#define PD_XYDIFF_H

#include "ut_vector.h"
#include "ut_hash.h"
#include "ut_tree.h"
#include "ut_queue.h"

class pf_Frag;
class PD_DocNode;
class pt_PieceTable;
class PD_XyDiffNodeMatch
{
  public:
	PD_DocNode * node1;
	PD_DocNode * node2;
} ;

typedef enum {ELEMENT, TEXT, DATA} PD_DocNodeType;

#define PD_DOCNODE_STATUS_MASK 0x00ff
#define PD_DOCNODE_SUBTREE_MASK    0xff00

typedef enum {UNMODIFIED  = 0x01,
			  DELETED     = 0x02,
			  INSERTED    = 0x03,
			  MODIFIED    = 0x04,
			  MOVE_STRONG = 0x05,
			  MOVE_WEAK   = 0x06,

			  // this value can be or-ed with those above
			  POTENTIAL_REORDER    = 0x0100
} PD_DocNodeStatus;


class PD_DocNode
{
  public:
	PD_DocNode(pf_Frag * pf, UT_uint64 iHash = 0, UT_uint32 iLinearWeight = 0);

#ifdef DEBUG
	PD_DocNode(const char * pName, UT_uint32 iXID, PD_DocNodeType eType,
			   UT_uint64 iHash = 0, UT_uint32 iLinearWeight = 0);
	
	const char * getName() const {return m_pName;}
#endif
	
	void                setXID(UT_uint32 xid) {m_iXID = xid;}
	UT_uint32           getXID()  const {return m_iXID;}

	void                setMyHash(UT_uint64 h) {m_iMyHash = h;}
	UT_uint64           getMyHash() const {return m_iMyHash;}

	void                setOffspringHash(UT_uint64 h) {m_iOffspringHash = h;}
	UT_uint64           getOffspringHash() const {return m_iOffspringHash;}
	
	void                setSubtreeHash(UT_uint64 h) {m_iSubtreeHash = h;}
	UT_uint64           getSubtreeHash() const {return m_iSubtreeHash;}

	void                setWeight(double w) {m_fWeight = w;}
	void                addWeight(double w) {m_fWeight += w;}
	double              getWeight() const {return m_fWeight;}

	void                setNoMatch(bool b) {m_bNoMatch = true;}
	bool                hasNoMatch() const {return m_bNoMatch;}

	void                markMatched(PD_DocNode * p) {m_pTheOther = p;};
	bool                isMatched() const {return m_pTheOther != NULL;}
	PD_DocNode *        getTheOther() const {return m_pTheOther;}

	UT_uint32           getLevel() const {return m_iLevel;}
	void                setLevel(UT_uint32 i) {m_iLevel = i;}
	
	UT_uint32           getOffset() const {return m_iOffset;}
	void                setOffset(UT_uint32 i) {m_iOffset = i;}

	PD_DocNodeType      getNodeType() const {return m_eType;}
	bool                isContentNode() const {return (m_eType == TEXT || m_eType == DATA);}

	PD_DocNodeStatus    getStatus() const {return (PD_DocNodeStatus)((UT_uint32)m_eStatus & PD_DOCNODE_STATUS_MASK);}
	void                setStatus(PD_DocNodeStatus s) {m_eStatus = s;}
	PD_DocNodeStatus    orStatus(PD_DocNodeStatus s)
		                      { m_eStatus = (PD_DocNodeStatus)((UT_uint32)s | (UT_uint32) m_eStatus); return m_eStatus;}
	PD_DocNodeStatus    getSubtreeStatus() const
		                      {return (PD_DocNodeStatus)((UT_uint32)m_eStatus & PD_DOCNODE_SUBTREE_MASK);}

	bool                potentialReorder() const {return getSubtreeStatus() == POTENTIAL_REORDER;}
	
	void                resetPotentialReorder()
	                          {m_eStatus = (PD_DocNodeStatus)((UT_uint32)m_eStatus & PD_DOCNODE_SUBTREE_MASK);}

	pf_Frag *           getFrag() const {return m_pFrag;}
	
  private:
	UT_uint32           m_iXID;
	UT_uint64           m_iMyHash;
	UT_uint64           m_iOffspringHash;
	UT_uint64           m_iSubtreeHash;
	double              m_fWeight;
	bool                m_bNoMatch;

	UT_uint32           m_iLevel;  // coords in the node's original tree
	UT_uint32           m_iOffset;

	PD_DocNode *        m_pTheOther;

	PD_DocNodeType      m_eType;

	PD_DocNodeStatus    m_eStatus;

	pf_Frag *           m_pFrag;

#ifdef DEBUG
	const char *        m_pName; // human redeable node name for debug purposes
#endif
};

class wSequence
{
  public:
	wSequence(UT_sint32 d, double w): data(d), weight(w) { };

	UT_sint32 data ;
	double    weight ;
} ;

class PD_XyDiff
{
  public:
	PD_XyDiff(pt_PieceTable * pPT1, pt_PieceTable * pPT2);
	~PD_XyDiff();

#ifdef DEBUG
	PD_XyDiff();

	void dump();
#endif

	bool apply();
	
  private:
	bool _constructorCommonCode();
	
	bool _buildDocTree(pt_PieceTable * pPT, UT_GenericTree<PD_DocNode*> &tree);
	bool _locateIdNodes();
	bool _calculateHashesAndWeights(UT_GenericTree<PD_DocNode*> & tree, double & fWeight);
	bool _constructPriorityQueue();

	bool _recursiveMatch(PD_DocNode * n1, PD_DocNode * n2);
	
	bool _addNodesToQueue(UT_GenericVector<PD_DocNode*> & v);
	bool _matchNode(PD_DocNode * n);

	bool _matchAllNodes();

	bool _optimizeMatches();
	bool _optimizeNode(PD_DocNode *n);
	bool _optimizeContentLeaves();

	bool _computeAdjustments();

	bool _lCss(UT_GenericVector<wSequence*> & s1, UT_GenericVector<wSequence*> & s2);
	bool _easyCss(UT_GenericVector<wSequence*> & s1, UT_GenericVector<wSequence*> & s2);

	bool _computeWeakMove(PD_DocNode *n);
	
	UT_GenericStringMap<PD_XyDiffNodeMatch *> m_hashNodes;
	
	UT_GenericTree<PD_DocNode*> m_tree1;
	UT_GenericTree<PD_DocNode*> m_tree2;

	UT_GenericQueue<PD_DocNode*> m_queue;

	double m_fWeight1;
	double m_fWeight2;

	UT_uint32 m_iNodeCount2;
};
#endif /* PD_XYDIFF_H */
