 
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

// insertSpan-related routined for class pt_PieceTable.

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

PT_Differences pt_PieceTable::_isDifferentFmt(pf_Frag * pf, UT_uint32 fragOffset, PT_AttrPropIndex indexAP)
{
	PT_Differences diff = 0;
	
	switch (pf->getType())
	{
	default:
		UT_ASSERT(0);
		return diff;

	case pf_Frag::PFT_EndOfDoc:
	case pf_Frag::PFT_Strux:
		{
			// we are looking at a strux or EOD.  see if there is text
			// just before us and if it is different.  if there is nothing
			// or something other than text before us, we are by-definition
			// changing fmt.
			if (   (pf->getPrev())
				&& (pf->getPrev()->getType()==pf_Frag::PFT_Text))
			{
				pf_Frag_Text * pftPrev = static_cast<pf_Frag_Text *>(pf->getPrev());
				if (pftPrev->getIndexAP() != indexAP)
					diff |= PT_Diff_Left;
			}
			else
			{
				diff |= PT_Diff_Left;
			}
		}
		return diff;
		
	case pf_Frag::PFT_Text:
		{
			pf_Frag_Text * pft = static_cast<pf_Frag_Text *>(pf);
			UT_uint32 fragLen = pft->getLength();
			if (fragOffset == 0)
			{
				// we are on the left edge of a text frag.
				// see if we are different.  if the previous frag
				// is a text frag, see if it is different.
				if (pft->getIndexAP() != indexAP)
					diff |= PT_Diff_Right;
				if (   (pf->getPrev())
					&& (pf->getPrev()->getType()==pf_Frag::PFT_Text))
				{
					pf_Frag_Text * pftPrev = static_cast<pf_Frag_Text *>(pf->getPrev());
					if (pftPrev->getIndexAP() != indexAP)
						diff |= PT_Diff_Left;
				}
			}
			else if (fragOffset == fragLen)
			{
				// we are on the right edge of a text frag.
				// see if we are different.  if the next frag is
				// a text frag, see if it is different.
				if (pft->getIndexAP() != indexAP)
					diff |= PT_Diff_Left;
				if (   (pft->getNext())
					&& (pft->getNext()->getType()==pf_Frag::PFT_Text))
				{
					pf_Frag_Text * pftNext = static_cast<pf_Frag_Text *>(pf->getNext());
					if (pftNext->getIndexAP() != indexAP)
						diff |= PT_Diff_Right;
				}
			}
			else
			{
				// we are in the middle of a text frag.
				// see if we are different.
				if (pft->getIndexAP() != indexAP)
					diff |= PT_Diff_Left | PT_Diff_Right;
			}
		}
		return diff;
	}
}
	
UT_Bool pt_PieceTable::_insertSpan(pf_Frag * pf,
								   PT_BufIndex bi,
								   PT_BlockOffset fragOffset,
								   UT_uint32 length,
								   PT_AttrPropIndex indexAP)
{
	// update the fragment and/or the fragment list.
	// return true if successful.
	
	pf_Frag_Text * pft = NULL;
	
	switch (pf->getType())
	{
	default:
		UT_ASSERT(0);
		return UT_FALSE;

	case pf_Frag::PFT_EndOfDoc:
	case pf_Frag::PFT_Strux:
		// if the position they gave us is the position of a strux
		// we probably need to re-interpret it slightly.  inserting
		// prior to a paragraph should probably be interpreted as
		// appending to the previous paragraph.  likewise, if they
		// gave us the EOD marker, we probably want to try to append
		// previous text fragment.

		if (pf->getPrev() && (pf->getPrev()->getType() == pf_Frag::PFT_Text))
		{
			pf = pf->getPrev();
			pft = static_cast<pf_Frag_Text *>(pf);
			fragOffset = pft->getLength();
			break;
		}

		// otherwise, we will just insert it before us.
		fragOffset = 0;
		break;
		
	case pf_Frag::PFT_Text:
		pft = static_cast<pf_Frag_Text *>(pf);
		break;
	}

	if (pft)
	{
		// we have a text frag containing or adjacent to the position.
		// deal with merging/splitting/etc.

		UT_uint32 fragLen = pft->getLength();
	
		// try to coalesce this character data with an existing fragment.
		// this is very likely to be sucessful during normal data entry.

		if (fragOffset == fragLen)
		{
			// we are to insert it immediately after this fragment.
			// if we are coalescable, just append it to this fragment
			// rather than create a new one.

			if (   (pft->getIndexAP()==indexAP)
				&& m_varset.isContiguous(pft->getBufIndex(),fragLen,bi))
			{
				// new text is contiguous, we just update the length of this fragment.

				pft->changeLength(fragLen+length);

				// TODO see if we are now contiguous with the next one and try
				// TODO to coalesce them (this will happen on after a delete
				// TODO char followed by undo).

				return UT_TRUE;
			}
		}

		if (fragOffset == 0)
		{
			// we are to insert it immediately before this fragment.
			// if we are coalescable, just prepend it to this fragment.
		
			if (   (pft->getIndexAP()==indexAP)
				&& m_varset.isContiguous(bi,length,pft->getBufIndex()))
			{
				// new text is contiguous, we just update the offset and length of
				// of this fragment.

				pft->adjustOffsetLength(bi,length+fragLen);

				// TODO see if we are now contiguous with the next one and try
				// TODO to coalesce them (this will happen on after a delete
				// TODO char followed by undo).
			
				return UT_TRUE;
			}

			// one last attempt to coalesce.  if we are at the beginning of
			// the fragment, and this fragment and the previous fragment have
			// the same properties, and the character data is contiguous with
			// it, let's stick it in the previous fragment.

			pf_Frag * pfPrev = pft->getPrev();
			if (pfPrev && pfPrev->getType()==pf_Frag::PFT_Text)
			{
				pf_Frag_Text * pftPrev = static_cast<pf_Frag_Text *>(pfPrev);
				UT_uint32 prevLength = pftPrev->getLength();
			
				if (   (pftPrev->getIndexAP() == indexAP)
					&& (m_varset.isContiguous(pftPrev->getBufIndex(),prevLength,bi)))
				{
					pftPrev->changeLength(prevLength+length);
					return UT_TRUE;
				}
			}
		}
	}
	
	// new text is not contiguous, we need to insert one or two new text
	// fragment(s) into the list.  first we construct a new text fragment
	// for the data that we inserted.

	pf_Frag_Text * pftNew = new pf_Frag_Text(this,bi,length,indexAP);
	if (!pftNew)
		return UT_FALSE;

	if (fragOffset == 0)
	{
		// if change is at the beginning of the fragment, we insert a
		// single new text fragment before the one we found.

		m_fragments.insertFrag(pf->getPrev(),pftNew);
		return UT_TRUE;
	}

	UT_uint32 fragLen = pf->getLength();
	if (fragLen==fragOffset)
	{
		// if the change is after this fragment, we insert a single
		// new text fragment after the one we found.

		m_fragments.insertFrag(pf,pftNew);
		return UT_TRUE;
	}

	// if the change is in the middle of the fragment, we construct
	// a second new text fragment for the portion after the insert.

	UT_ASSERT(pft);
	
	UT_uint32 lenTail = pft->getLength() - fragOffset;
	PT_BufIndex biTail = m_varset.getBufIndex(pft->getBufIndex(),fragOffset);
	pf_Frag_Text * pftTail = new pf_Frag_Text(this,biTail,lenTail,pft->getIndexAP());
	if (!pftTail)
		return UT_FALSE;
			
	pft->changeLength(fragOffset);
	m_fragments.insertFrag(pft,pftNew);
	m_fragments.insertFrag(pftNew,pftTail);
	
	return UT_TRUE;
}

UT_Bool pt_PieceTable::insertSpan(PT_DocPosition dpos,
								  UT_UCSChar * p,
								  UT_uint32 length)
{
	// insert character data into the document at the given position.

	UT_ASSERT(m_pts==PTS_Editing);

	PT_DocPosition dposTemp;
	if (_haveTempSpanFmt(&dposTemp,NULL))
		if (dposTemp != dpos)
			clearTemporarySpanFmt();

	// append the text data to the end of the current buffer.

	PT_BufIndex bi;
	if (!m_varset.appendBuf(p,length,&bi))
		return UT_FALSE;

	// get the fragment at the given document position.
	
	pf_Frag * pf = NULL;
	PT_BlockOffset fragOffset = 0;
	UT_Bool bFound = getFragFromPosition(dpos,&pf,&fragOffset);
	UT_ASSERT(bFound);

	// we just did a getFragFromPosition() which gives us the
	// the thing *starting* at that position.  if we have a
	// fragment boundary at that position, it's sort of arbitrary
	// whether we treat this insert as a prepend to the one we just found
	// or an append to the previous one (when it's a text frag).
	// in the normal case, we want the Attr/Prop of a character
	// insertion to take the AP of the thing to the immediate
	// left (seems to be what MS-Word and MS-WordPad do).  It's also
	// useful when the user hits the BOLD button (without a)
	// selection) and then starts typing -- ideally you'd like
	// all of the text to have bold not just the first.  therefore,
	// we will see if we are on a text-text boundary and backup
	// (and thus appending) to the previous.

	if ((fragOffset == 0) && pf->getPrev() && (pf->getPrev()->getType() == pf_Frag::PFT_Text))
	{
		pf = pf->getPrev();
		fragOffset = pf->getLength();
	}
	
	PT_AttrPropIndex indexAP = _chooseIndexAP(pf,fragOffset);

	// before we actually do the insert, see if we are introducing a
	// change in the formatting.
	PT_Differences isDifferentFmt = _isDifferentFmt(pf,fragOffset,indexAP);
	
	if (!_insertSpan(pf,bi,fragOffset,length,indexAP))
		return UT_FALSE;

	// create a change record, add it to the history, and notify
	// anyone listening.

	PX_ChangeRecord_Span * pcr
		= new PX_ChangeRecord_Span(PX_ChangeRecord::PXT_InsertSpan,
								   dpos,indexAP,bi,length,isDifferentFmt);
	UT_ASSERT(pcr);

	pf_Frag_Strux * pfs = NULL;
	UT_Bool bFoundStrux = _getStruxFromPosition(pcr->getPosition(),&pfs);
	UT_ASSERT(bFoundStrux);

	m_pDocument->notifyListeners(pfs,pcr);

	if (_canCoalesceInsertSpan(pcr))
	{
		m_history.coalesceHistory(pcr);
		delete pcr;
	}
	else
		m_history.addChangeRecord(pcr);

	return UT_TRUE;
}

UT_Bool pt_PieceTable::_canCoalesceInsertSpan(PX_ChangeRecord_Span * pcrSpan) const
{
	// see if this record can be coalesced with the most recent undo record.

	UT_ASSERT(pcrSpan->getType() == PX_ChangeRecord::PXT_InsertSpan);

	PX_ChangeRecord * pcrUndo;
	if (!m_history.getUndo(&pcrUndo))
		return UT_FALSE;
	if (pcrSpan->getType() != pcrUndo->getType())
		return UT_FALSE;
	if (pcrSpan->getIndexAP() != pcrUndo->getIndexAP())
		return UT_FALSE;

	PX_ChangeRecord_Span * pcrUndoSpan = static_cast<PX_ChangeRecord_Span *>(pcrUndo);
	UT_uint32 lengthUndo = pcrUndoSpan->getLength();

	if ((pcrUndo->getPosition() + lengthUndo) != pcrSpan->getPosition())
		return UT_FALSE;

	PT_BufIndex biUndo = pcrUndoSpan->getBufIndex();
	PT_BufIndex biSpan = pcrSpan->getBufIndex();

	if (m_varset.getBufIndex(biUndo,lengthUndo) != biSpan)
		return UT_FALSE;

	// TODO decide if we need to test isDifferentFmt bit in the two ChangeRecords.
	
	return UT_TRUE;
}

PT_AttrPropIndex pt_PieceTable::_chooseIndexAP(pf_Frag * pf, PT_BlockOffset fragOffset)
{
	// decide what indexAP to give an insertSpan inserting at the given
	// position in the document [pf,fragOffset].
	
	PT_AttrPropIndex indexAP;

	// if we have a TempSpanFmt active, use it.
	
	if (_haveTempSpanFmt(NULL,&indexAP))
		return indexAP;

	// try to get it from the current fragment.
	
	if ((pf->getType() == pf_Frag::PFT_Text) && (fragOffset > 0))
	{
		// if we are inserting at the middle or end of a text fragment,
		// we take the A/P of this text fragment.

		pf_Frag_Text * pft = static_cast<pf_Frag_Text *>(pf);
		return pft->getIndexAP();
	}

	// we are not looking forward at a text fragment or
	// we are at the beginning of a text fragment.
	// look to the previous fragment to see what to do.
	
	pf_Frag * pfPrev = pf->getPrev();
	switch (pfPrev->getType())
	{
	case pf_Frag::PFT_Text:
		{
			// if we immediately follow another text fragment, we
			// take the A/P of that fragment.

			pf_Frag_Text * pftPrev = static_cast<pf_Frag_Text *>(pfPrev);
			return pftPrev->getIndexAP();
		}

	case pf_Frag::PFT_Strux:
		{
			// if we immediately follow a block (paragraph), we try
			// to use the preferredSpanFmt which was set when the
			// paragraph was created.  this allows continuous typing
			// (typing in a style, hitting return, and typing some more)
			// to work as expected.  ***BUT*** it may cause inserts at
			// the beginning of this paragraph to be *stuck* to that
			// style.

			// TODO figure out how to "forget" about the preferredSpanFmt
			// TODO and/or when to start looking at the existing text in
			// TODO the paragraph.

			pf_Frag_Strux * pfsPrev = static_cast<pf_Frag_Strux *>(pfPrev);
			if (pfsPrev->getStruxType() == PTX_Block)
			{
				pf_Frag_Strux_Block * pfsbPrev = static_cast<pf_Frag_Strux_Block *>(pfsPrev);
				return pfsbPrev->getPreferredSpanFmt();
			}

			// if we are looking back at something else, we really don't
			// have a choice.  as a last ditch effort, try to take
			// a look to the right.
				
			if (pf->getType() == pf_Frag::PFT_Text)
			{
				// we take the A/P of this text fragment.

				pf_Frag_Text * pft = static_cast<pf_Frag_Text *>(pf);
				return pft->getIndexAP();
			}

			// we can't find anything, just use the default.
			
			return 0;
		}

	default:
		UT_ASSERT(0);
		return 0;
	}
}
