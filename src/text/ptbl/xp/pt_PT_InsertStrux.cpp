 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
*/

// insertStrux-related functions for class pt_PieceTable

#include "ut_types.h"
#include "ut_misc.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_growbuf.h"
#include "pt_PieceTable.h"
#include "pf_Frag.h"
#include "pf_Frag_Strux.h"
#include "pf_Frag_Strux_Block.h"
#include "pf_Frag_Strux_Column.h"
#include "pf_Frag_Strux_ColumnSet.h"
#include "pf_Frag_Strux_Section.h"
#include "pf_Frag_Text.h"
#include "pf_Fragments.h"
#include "px_ChangeRecord.h"
#include "px_ChangeRecord_Span.h"
#include "px_ChangeRecord_SpanChange.h"
#include "px_ChangeRecord_Strux.h"

/****************************************************************/
/****************************************************************/

UT_Bool pt_PieceTable::_createStrux(PTStruxType pts,
									PT_AttrPropIndex indexAP,
									pf_Frag_Strux ** ppfs)
{
	// create a strux frag for this.
	// return *pfs and true if successful.

	// create an unlinked strux fragment.

	// TODO consider collapsing all of the Frag_Strux classes if they don't
	// TODO have any private data.
	
	pf_Frag_Strux * pfs = NULL;
	switch (pts)
	{
	case PTX_Section:
		pfs = new pf_Frag_Strux_Section(this,indexAP);
		break;
		
	case PTX_ColumnSet:
		pfs = new pf_Frag_Strux_ColumnSet(this,indexAP);
		break;
		
	case PTX_Column:
		pfs = new pf_Frag_Strux_Column(this,indexAP);
		break;
		
	case PTX_Block:
		pfs = new pf_Frag_Strux_Block(this,indexAP);
		break;

	default:
		UT_ASSERT(0);
		break;
	}

	if (!pfs)
	{
		UT_DEBUGMSG(("Could not create structure fragment.\n"));
		// we forget about the AP that we created
		return UT_FALSE;
	}

	*ppfs = pfs;
	return UT_TRUE;
}

void pt_PieceTable::_insertStrux(pf_Frag_Strux * pfsPrev,
								 pf_Frag_Text * pft,
								 PT_BlockOffset fragOffset,
								 UT_Bool bLeftSide,
								 pf_Frag_Strux * pfsNew)
{
	// insert the new strux frag at the given offset within the
	// text fragment (or after the previous strux).

	if (!pft)
	{
		// no text fragment ??
		// insert after the given strux
		// this is probably a column or columnset.

		m_fragments.insertFrag(pfsPrev,pfsNew);
		return;
	}

	// we have a text fragment which we must deal with.
	// if we are in the middle of it, we split it.
	// if we are at one end of it and we came from the
	// correct side, we just insert the block.
	// if we are at one end of it and we came from the
	// other side, we split it (creating a zero length
	// fragment) and insert the block between them.  this
	// lets a paragraph break in the middle of a span
	// to preserve the span's properties into the next
	// block.
	
	UT_uint32 fragLen = pft->getLength();
	if (fragOffset == fragLen)
	{
		// we are at the right end of the fragment.
		// insert the strux after the text fragment.

		m_fragments.insertFrag(pft,pfsNew);
		if (bLeftSide)
		{
			// we are on the left side of the doc position and
			// we are at the right end of the fragment.  inserting
			// a paragraph here should cause the next character
			// typed to be in the same style as this fragment.
			// therefore, we 'split' the fragment -- actually, we
			// insert the strux after this fragment and just
			// create a new text fragment with length zero and
			// insert it after our new strux.

			// TODO figure out how to create a zero length text fragment
			// TODO and do:
			// TODO         pf_Frag_Text * pftNew = new...
			// TODO         m_fragments.insertFrag(pfsNew,pftNew);
		}
		return;
	}

	if (fragOffset == 0)
	{
		// we are at the left end of the fragment.
		// insert the strux before the text fragment.

		m_fragments.insertFrag(pft->getPrev(),pfsNew);
		if (!bLeftSide)
		{
			// we are on the right side of the doc position and
			// we are at the left end of the fragment.  like in
			// the previous section, a paragraph break here should
			// cause future text inserted at the end of the
			// previous paragraph to be in the style of this text
			// fragment.  do a similar split, with a zero length
			// fragment.
			//
			// TODO verify that we want this behaviour.  both
			// TODO MS Word and MS WordPad have a bias toward
			// TODO all typing picking up the style of the character
			// TODO immediately prior -- rather than depend upon
			// TODO how you got there, so I cannot use them as an
			// TODO example since they don't appear to support the
			// TODO concept w/o regard to whether it's text that is
			// TODO inserted or a paragraph break.

			// TODO figure out how to create a zero length text
			// TODO fragment and do:
			// TODO         pf_Frag_Text * pftNew = new...
			// TODO         m_fragments.insertFrag(pfsNew->getPrev(),pftNew);
		}
		return;
	}

	// we are in the middle of a text fragment.  split it
	// and insert the new strux in between the pieces.

	UT_uint32 lenTail = pft->getLength() - fragOffset;
	PT_BufIndex biTail = m_varset.getBufIndex(pft->getBufIndex(),fragOffset);
	pf_Frag_Text * pftTail = new pf_Frag_Text(this,biTail,lenTail,pft->getIndexAP());
	UT_ASSERT(pftTail);
			
	pft->changeLength(fragOffset);
	m_fragments.insertFrag(pft,pfsNew);
	m_fragments.insertFrag(pfsNew,pftTail);
	return;
}
		

UT_Bool pt_PieceTable::insertStrux(PT_DocPosition dpos,
								   UT_Bool bLeftSide,
								   PTStruxType pts)
{
	// insert a new structure fragment at the given document position.
	// this function can only be called while editing the document.

	UT_ASSERT(m_pts==PTS_Editing);

	if (m_bHaveTemporarySpanFmt)
		clearTemporarySpanFmt();

	// get the text fragment at the doc postion and the strux fragment
	// immediately prior to (containing) the given document position.
	// this is valid for stuff within the body of the document, but may
	// be suspect for things like columnset and columns.
	
	pf_Frag_Strux * pfsPrev = NULL;
	pf_Frag_Text * pft = NULL;
	PT_BlockOffset fragOffset = 0;
	UT_Bool bFoundIt = getTextFragFromPosition(dpos,bLeftSide,&pfsPrev,&pft,&fragOffset);
	UT_ASSERT(bFoundIt);

	// if we are inserting something similar to the previous strux,
	// we will clone the attributes/properties; we assume that the
	// new strux should have the same AP as the one which preceeds us.
	// This is generally true for inserting a paragraph -- it should
	// inherit the style of the one we just broke.

	// TODO It may turn out that this is not true for other things, like
	// TODO columns, but for now we will assume it is OK.

	PT_AttrPropIndex indexAP = 0;
	if (pts == pfsPrev->getStruxType())
		indexAP = pfsPrev->getIndexAP();
	
	pf_Frag_Strux * pfsNew = NULL;
	if (!_createStrux(pts,indexAP,&pfsNew))
		return UT_FALSE;

	// insert this frag into the fragment list.

	_insertStrux(pfsPrev,pft,fragOffset,bLeftSide,pfsNew);
	
	// create a change record to describe the change, add
	// it to the history, and let our listeners know about it.
	
	PX_ChangeRecord_Strux * pcrs
		= new PX_ChangeRecord_Strux(PX_ChangeRecord::PXT_InsertStrux,PX_ChangeRecord::PXF_Null,
									dpos,bLeftSide,
									m_indexAPTemporarySpanFmt,indexAP,
									m_bHaveTemporarySpanFmt,UT_FALSE,
									pts);
	UT_ASSERT(pcrs);
	m_history.addChangeRecord(pcrs);
	m_pDocument->notifyListeners(pfsPrev,pfsNew,pcrs);
	m_bHaveTemporarySpanFmt = UT_FALSE;
	
	return UT_TRUE;
}
