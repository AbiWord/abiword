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


// deleteSpan-related routines for class pt_PieceTable

#include "ut_types.h"
#include "ut_misc.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_growbuf.h"
#include "pt_PieceTable.h"
#include "pf_Frag.h"
#include "pf_Frag_Object.h"
#include "pf_Frag_Strux.h"
#include "pf_Frag_Strux_Block.h"
#include "pf_Frag_Strux_Section.h"
#include "pf_Frag_Text.h"
#include "pf_Fragments.h"
#include "px_ChangeRecord.h"
#include "px_CR_Span.h"
#include "px_CR_SpanChange.h"
#include "px_CR_Strux.h"


#define SETP(p,v)	do { if (p) (*(p)) = (v); } while (0)

/****************************************************************/
/****************************************************************/

UT_Bool pt_PieceTable::_deleteSpan(pf_Frag_Text * pft, UT_uint32 fragOffset,
								   PT_BufIndex bi, UT_uint32 length,
								   pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd)
{
	// perform simple delete of a span of text.
	// we assume that it is completely contained within this fragment.

	UT_ASSERT(fragOffset+length <= pft->getLength());

	SETP(ppfEnd, pft);
	SETP(pfragOffsetEnd, fragOffset);
	
	if (fragOffset == 0)
	{
		// the change is at the beginning of the fragment,

		if (length == pft->getLength())
		{
			// the change exactly matches the fragment, just delete the fragment.
			// as we delete it, see if the fragments around it can be coalesced.

			_unlinkFrag(pft,ppfEnd,pfragOffsetEnd);
			delete pft;
			return UT_TRUE;
		}

		// the change is a proper prefix within the fragment,
		// do a left-truncate on it.

		pft->adjustOffsetLength(m_varset.getBufIndex(bi,length),pft->getLength()-length);
		return UT_TRUE;
	}

	if (fragOffset+length == pft->getLength())
	{
		// the change is a proper suffix within the fragment,
		// do a right-truncate on it.

		pft->changeLength(fragOffset);

		SETP(ppfEnd, pft->getNext());
		SETP(pfragOffsetEnd, 0);
		
		return UT_TRUE;
	}

	// otherwise, the change is in the middle of the fragment.
	// we right-truncate the current fragment at the deletion
	// point and create a new fragment for the tail piece
	// beyond the end of the deletion.

	UT_uint32 startTail = fragOffset + length;
	UT_uint32 lenTail = pft->getLength() - startTail;
	PT_BufIndex biTail = m_varset.getBufIndex(pft->getBufIndex(),startTail);
	pf_Frag_Text * pftTail = new pf_Frag_Text(this,biTail,lenTail,pft->getIndexAP());
	UT_ASSERT(pftTail);
	pft->changeLength(fragOffset);
	m_fragments.insertFrag(pft,pftTail);

	SETP(ppfEnd, pftTail);
	SETP(pfragOffsetEnd, 0);
	
	return UT_TRUE;
}

UT_Bool pt_PieceTable::_deleteSpanWithNotify(PT_DocPosition dpos,
											 pf_Frag_Text * pft, UT_uint32 fragOffset,
											 UT_uint32 length,
											 pf_Frag_Strux * pfs,
											 pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd)
{
	// create a change record for this change and put it in the history.

	UT_ASSERT(pfs);
	
	if (length == 0)					// TODO decide if this is an error.
	{
		UT_DEBUGMSG(("_deleteSpanWithNotify: length==0\n"));
		SETP(ppfEnd, pft->getNext());
		SETP(pfragOffsetEnd, 0);
		return UT_TRUE;
	}

	// we do this before the actual change because various fields that
	// we need are blown away during the delete.  we then notify all
	// listeners of the change.

	PT_BlockOffset blockOffset = _computeBlockOffset(pfs,pft) + fragOffset;
		
	PX_ChangeRecord_Span * pcr
		= new PX_ChangeRecord_Span(PX_ChangeRecord::PXT_DeleteSpan,
								   dpos, pft->getIndexAP(),
								   m_varset.getBufIndex(pft->getBufIndex(),fragOffset),
								   length,blockOffset);
	UT_ASSERT(pcr);

#ifndef PT_NOTIFY_BEFORE_DELETES
	UT_Bool bResult = _deleteSpan(pft,fragOffset,pft->getBufIndex(),length,ppfEnd,pfragOffsetEnd);
#endif	

	if (_canCoalesceDeleteSpan(pcr))
	{
		m_history.coalesceHistory(pcr);
		m_pDocument->notifyListeners(pfs,pcr);
		delete pcr;
	}
	else
	{
		m_history.addChangeRecord(pcr);
		m_pDocument->notifyListeners(pfs,pcr);
	}

#ifdef PT_NOTIFY_BEFORE_DELETES
	UT_Bool bResult = _deleteSpan(pft,fragOffset,pft->getBufIndex(),length,ppfEnd,pfragOffsetEnd);
#endif	

	return bResult;
}

UT_Bool pt_PieceTable::_canCoalesceDeleteSpan(PX_ChangeRecord_Span * pcrSpan) const
{
	// see if this record can be coalesced with the most recent undo record.

	UT_ASSERT(pcrSpan->getType() == PX_ChangeRecord::PXT_DeleteSpan);

	PX_ChangeRecord * pcrUndo;
	if (!m_history.getUndo(&pcrUndo))
		return UT_FALSE;
	if (pcrSpan->getType() != pcrUndo->getType())
		return UT_FALSE;
	if (pcrSpan->getIndexAP() != pcrUndo->getIndexAP())
		return UT_FALSE;

	PX_ChangeRecord_Span * pcrUndoSpan = static_cast<PX_ChangeRecord_Span *>(pcrUndo);
	UT_uint32 lengthUndo = pcrUndoSpan->getLength();
	PT_BufIndex biUndo = pcrUndoSpan->getBufIndex();

	UT_uint32 lengthSpan = pcrSpan->getLength();
	PT_BufIndex biSpan = pcrSpan->getBufIndex();

	if (pcrSpan->getPosition() == pcrUndo->getPosition())
	{
		if (m_varset.getBufIndex(biUndo,lengthUndo) == biSpan)
			return UT_TRUE;				// a forward delete

		return UT_FALSE;
	}
	else if ((pcrSpan->getPosition() + lengthSpan) == pcrUndo->getPosition())
	{
		if (m_varset.getBufIndex(biSpan,lengthSpan) == biUndo)
			return UT_TRUE;				// a backward delete

		return UT_FALSE;
	}
	else
	{
		return UT_FALSE;
	}
}

UT_Bool pt_PieceTable::deleteSpan(PT_DocPosition dpos1,
								  PT_DocPosition dpos2)
{
	// remove (dpos2-dpos1) characters from the document at the given position.
	
	UT_ASSERT(m_pts==PTS_Editing);

	UT_ASSERT(dpos2 > dpos1);
	UT_uint32 length = dpos2 - dpos1;

	if (_haveTempSpanFmt(NULL,NULL))
		clearTemporarySpanFmt();

	pf_Frag * pf_First;
	pf_Frag * pf_End;
	PT_BlockOffset fragOffset_First;
	PT_BlockOffset fragOffset_End;
	
	UT_Bool bFound = getFragsFromPositions(dpos1,dpos2,&pf_First,&fragOffset_First,&pf_End,&fragOffset_End);
	UT_ASSERT(bFound);

	if ((fragOffset_End==0) && pf_End->getPrev() && (pf_End->getPrev()->getType() == pf_Frag::PFT_Text))
	{
		pf_End = pf_End->getPrev();
		fragOffset_End = pf_End->getLength();
	}

	pf_Frag_Strux * pfsContainer = NULL;
	UT_Bool bFoundStrux = _getStruxFromPosition(dpos1,&pfsContainer);
	UT_ASSERT(bFoundStrux);

	switch (pfsContainer->getStruxType())
	{
	default:
		UT_ASSERT(0);
		return UT_FALSE;
		
	case PTX_Section:
		// if the previous container is a section, then pf_First
		// must be the first block in the section.
		UT_ASSERT((pf_First->getPrev() == pfsContainer));
		UT_ASSERT((pf_First->getType() == pf_Frag::PFT_Strux));
		UT_ASSERT(((static_cast<pf_Frag_Strux *>(pf_First))->getStruxType() == PTX_Block));
		// since, we cannot delete the first block in a section, we
		// secretly translate this into a request to delete the section;
		// the block we have will then be slurped into the previous
		// section.
		// TODO consider just fixing up the variables that we have
		// TODO computed so far, rather than making a recursive call.
		return deleteSpan(dpos1-pfsContainer->getLength(),dpos1);

	case PTX_Block:
		// if the previous container is a block, we're ok.
		// the loop below will take care of everything.
		break;
	}
	
	// see if the amount of text to be deleted is completely
	// contained within the fragment found.  if so, we have
	// a simple delete.  otherwise, we need to set up a multi-step
	// delete -- it may not actually take more than one step, but
	// it is too complicated to tell at this point, so we assume
	// it will and don't worry about it.
	//
	// we are in a simple change if the beginning and end are
	// within the same fragment.
	
	// NOTE: if we call beginMultiStepGlob() we ***MUST*** call
	// NOTE: endMultiStepGlob() before we return -- otherwise,
	// NOTE: the undo/redo won't be properly bracketed.

	UT_Bool bSimple = (pf_First == pf_End);
	if (!bSimple)
		beginMultiStepGlob();

	// loop to delete the amount requested, one text fragment at a time.
	// if we encounter any non-text fragments along the way, we delete
	// them too.  that is, we implicitly delete Strux and Objects here.

	pf_Frag * pfNewEnd;
	UT_uint32 fragOffsetNewEnd;

	while (length > 0)
	{
		UT_uint32 lengthInFrag = pf_First->getLength() - fragOffset_First;
		UT_uint32 lengthThisStep = UT_MIN(lengthInFrag, length);
		
		switch (pf_First->getType())
		{
		case pf_Frag::PFT_EndOfDoc:
		default:
			UT_ASSERT(0);
			return UT_FALSE;
			
		case pf_Frag::PFT_Strux:
			{
				pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *> (pf_First);
				
				UT_Bool bResult = _deleteStruxWithNotify(dpos1,pfs,
														 &pfNewEnd,&fragOffsetNewEnd);
				UT_ASSERT(bResult);
				// we do not update pfsContainer because we just deleted pfs.
			}
			break;

		case pf_Frag::PFT_Text:
			{
				UT_Bool bResult
					= _deleteSpanWithNotify(dpos1,static_cast<pf_Frag_Text *>(pf_First),
											fragOffset_First,lengthThisStep,
											pfsContainer,&pfNewEnd,&fragOffsetNewEnd);
				UT_ASSERT(bResult);
			}
			break;

		case pf_Frag::PFT_Object:
			{
				UT_Bool bResult
					= _deleteObjectWithNotify(dpos1,static_cast<pf_Frag_Object *>(pf_First),
											  fragOffset_First,lengthThisStep,
											  pfsContainer,&pfNewEnd,&fragOffsetNewEnd);
				UT_ASSERT(bResult);
			}
			break;
		}

		// we do not change dpos1, since we are deleting stuff, but we
		// do decrement the length-remaining.
		// dpos2 becomes bogus at this point.

		length -= lengthThisStep;
		
		// since _delete{*}WithNotify(), can delete pf_First, mess with the
		// fragment list, and does some aggressive coalescing of
		// fragments, we cannot just do a pf_First->getNext() here.
		// to advance to the next fragment, we use the *NewEnd variables
		// that each of the _delete routines gave us.

		pf_First = pfNewEnd;
		if (!pf_First)
			length = 0;
		fragOffset_First = fragOffsetNewEnd;
	}

	if (!bSimple)
		endMultiStepGlob();
	
	return UT_TRUE;
}
