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
#include "ut_stack.h"
#include "pt_PieceTable.h"
#include "pf_Frag.h"
#include "pf_Frag_FmtMark.h"
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

bool pt_PieceTable::_deleteSpan(pf_Frag_Text * pft, UT_uint32 fragOffset,
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
			return true;
		}

		// the change is a proper prefix within the fragment,
		// do a left-truncate on it.

		pft->adjustOffsetLength(m_varset.getBufIndex(bi,length),pft->getLength()-length);
		return true;
	}

	if (fragOffset+length == pft->getLength())
	{
		// the change is a proper suffix within the fragment,
		// do a right-truncate on it.

		pft->changeLength(fragOffset);

		SETP(ppfEnd, pft->getNext());
		SETP(pfragOffsetEnd, 0);
		
		return true;
	}

	// otherwise, the change is in the middle of the fragment.
	// we right-truncate the current fragment at the deletion
	// point and create a new fragment for the tail piece
	// beyond the end of the deletion.

	UT_uint32 startTail = fragOffset + length;
	UT_uint32 lenTail = pft->getLength() - startTail;
	PT_BufIndex biTail = m_varset.getBufIndex(pft->getBufIndex(),startTail);
	pf_Frag_Text * pftTail = new pf_Frag_Text(this,biTail,lenTail,pft->getIndexAP(),pft->getField());
	UT_ASSERT(pftTail);
	pft->changeLength(fragOffset);
	m_fragments.insertFrag(pft,pftTail);

	SETP(ppfEnd, pftTail);
	SETP(pfragOffsetEnd, 0);
	
	return true;
}

bool pt_PieceTable::_deleteSpanWithNotify(PT_DocPosition dpos,
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
		return true;
	}

	// we do this before the actual change because various fields that
	// we need are blown away during the delete.  we then notify all
	// listeners of the change.

	PT_BlockOffset blockOffset = _computeBlockOffset(pfs,pft) + fragOffset;
		
	PX_ChangeRecord_Span * pcr
		= new PX_ChangeRecord_Span(PX_ChangeRecord::PXT_DeleteSpan,
								   dpos, pft->getIndexAP(),
								   m_varset.getBufIndex(pft->getBufIndex(),fragOffset),
								   length,blockOffset,pft->getField());
	UT_ASSERT(pcr);

	bool bResult = _deleteSpan(pft,fragOffset,pft->getBufIndex(),length,ppfEnd,pfragOffsetEnd);

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

	return bResult;
}


bool pt_PieceTable::_deleteSpan_norec(PT_DocPosition dpos,
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
		return true;
	}

	// we do this before the actual change because various fields that
	// we need are blown away during the delete.  we then notify all
	// listeners of the change.

	PT_BlockOffset blockOffset = _computeBlockOffset(pfs,pft) + fragOffset;
		
	PX_ChangeRecord_Span * pcr
		= new PX_ChangeRecord_Span(PX_ChangeRecord::PXT_DeleteSpan,
								   dpos, pft->getIndexAP(),
								   m_varset.getBufIndex(pft->getBufIndex(),fragOffset),
								   length,blockOffset,pft->getField());
	UT_ASSERT(pcr);

	bool bResult = _deleteSpan(pft,fragOffset,pft->getBufIndex(),length,ppfEnd,pfragOffsetEnd);

	m_pDocument->notifyListeners(pfs,pcr);
	delete pcr;

	return bResult;
}

bool pt_PieceTable::_canCoalesceDeleteSpan(PX_ChangeRecord_Span * pcrSpan) const
{
	// see if this record can be coalesced with the most recent undo record.

	UT_ASSERT(pcrSpan->getType() == PX_ChangeRecord::PXT_DeleteSpan);

	PX_ChangeRecord * pcrUndo;
	if (!m_history.getUndo(&pcrUndo))
		return false;
	if (pcrSpan->getType() != pcrUndo->getType())
		return false;
	if (pcrSpan->getIndexAP() != pcrUndo->getIndexAP())
		return false;

	PX_ChangeRecord_Span * pcrUndoSpan = static_cast<PX_ChangeRecord_Span *>(pcrUndo);
	UT_uint32 lengthUndo = pcrUndoSpan->getLength();
	PT_BufIndex biUndo = pcrUndoSpan->getBufIndex();

	UT_uint32 lengthSpan = pcrSpan->getLength();
	PT_BufIndex biSpan = pcrSpan->getBufIndex();

	if (pcrSpan->getPosition() == pcrUndo->getPosition())
	{
		if (m_varset.getBufIndex(biUndo,lengthUndo) == biSpan)
			return true;				// a forward delete

		return false;
	}
	else if ((pcrSpan->getPosition() + lengthSpan) == pcrUndo->getPosition())
	{
		if (m_varset.getBufIndex(biSpan,lengthSpan) == biUndo)
			return true;				// a backward delete

		return false;
	}
	else
	{
		return false;
	}
}

bool pt_PieceTable::_isSimpleDeleteSpan(PT_DocPosition dpos1,
										PT_DocPosition dpos2) const
{
	// see if the amount of text to be deleted is completely
	// contained within the fragment found.  if so, we have
	// a simple delete.  otherwise, we need to set up a multi-step
	// delete -- it may not actually take more than one step, but
	// it is too complicated to tell at this point, so we assume
	// it will and don't worry about it.
	//
	// we are in a simple change if the beginning and end are
	// within the same fragment.
	
	pf_Frag * pf_First;
	pf_Frag * pf_End;
	PT_BlockOffset fragOffset_First;
	PT_BlockOffset fragOffset_End;
	
	bool bFound = getFragsFromPositions(dpos1,dpos2,&pf_First,&fragOffset_First,&pf_End,&fragOffset_End);
	UT_ASSERT(bFound);

	if ((fragOffset_End==0) && pf_End->getPrev() && (pf_End->getPrev()->getType() == pf_Frag::PFT_Text))
	{
		pf_End = pf_End->getPrev();
		fragOffset_End = pf_End->getLength();
	}

	return (pf_First == pf_End);
}

bool pt_PieceTable::_tweakDeleteSpanOnce(PT_DocPosition & dpos1, 
										 PT_DocPosition & dpos2,
										 UT_Stack * pstDelayStruxDelete) const
{
	//  Our job is to adjust the end positions of the delete
	//  operating to delete those structural object that the 
	//  user will expect to have deleted, even if the dpositions
	//  aren't quite right to encompass those.

	pf_Frag * pf_First;
	pf_Frag * pf_End;
	pf_Frag * pf_Other;
	PT_BlockOffset fragOffset_First;
	PT_BlockOffset fragOffset_End;
	
	bool bFound = getFragsFromPositions(dpos1,dpos2,&pf_First,&fragOffset_First,&pf_End,&fragOffset_End);
	UT_ASSERT(bFound);

	pf_Frag_Strux * pfsContainer = NULL;
	bool bFoundStrux = _getStruxFromPosition(dpos1,&pfsContainer);
	UT_ASSERT(bFoundStrux);

    _tweakFieldSpan(dpos1,dpos2);

	switch (pfsContainer->getStruxType())
	{
	default:
		UT_ASSERT(0);
		return false;
		
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
		dpos1 -= pfsContainer->getLength();
		return true;

	case PTX_SectionHdrFtr:
		// if the previous container is a Header/Footersection, then pf_First
		// must be the first block in the section.
		UT_ASSERT((pf_First->getPrev() == pfsContainer));
		UT_ASSERT((pf_First->getType() == pf_Frag::PFT_Strux));
		UT_ASSERT(((static_cast<pf_Frag_Strux *>(pf_First))->getStruxType() == PTX_Block));
		// since, we cannot delete the first block in a section, we
		// secretly translate this into a request to delete the section;
		// the block we have will then be slurped into the previous
		// section.
		dpos1 -= pfsContainer->getLength();
		return true;

	case PTX_Block:
		// if the previous container is a block, we're ok.
		// the loop below will take care of everything.
		break;
	}

	if (pf_First->getType() == pf_Frag::PFT_Strux)
	{
		switch(static_cast<pf_Frag_Strux *>(pf_First)->getStruxType())
		{
		default:
			break;

		case PTX_Section:
			UT_ASSERT(fragOffset_First == 0);
			if (dpos2 == dpos1 + pf_First->getLength())
			{
				//  If we are just deleting a section break, then 
				//  we should delete the first block marker in the
				//  next section, combining the blocks before and
				//  after the section break.
				pf_Other = pf_First->getNext();
				UT_ASSERT(pf_Other);
				UT_ASSERT(pf_Other->getType() == pf_Frag::PFT_Strux);
				UT_ASSERT(((static_cast<pf_Frag_Strux *>(pf_Other))->getStruxType() == PTX_Block));
				dpos2 += pf_Other->getLength();
				return true;
			}
		case PTX_SectionHdrFtr:
			UT_ASSERT(fragOffset_First == 0);
			if (dpos2 == dpos1 + pf_First->getLength())
			{
				//  If we are just deleting a section break, then 
				//  we should delete the first block marker in the
				//  next section, combining the blocks before and
				//  after the section break.
				pf_Other = pf_First->getNext();
				UT_ASSERT(pf_Other);
				UT_ASSERT(pf_Other->getType() == pf_Frag::PFT_Strux);
				UT_ASSERT(((static_cast<pf_Frag_Strux *>(pf_Other))->getStruxType() == PTX_Block));
				dpos2 += pf_Other->getLength();
				return true;
			}

			break;
		}
	}
    
	if (fragOffset_First == 0 && fragOffset_End == 0 && pf_First != pf_End)
	{
		pf_Frag * pf_Before = pf_First->getPrev();
		while (pf_Before && pf_Before->getType() == pf_Frag::PFT_FmtMark)
			pf_Before = pf_Before->getPrev();
		pf_Frag * pf_Last = pf_End->getPrev();
		while (pf_Last && pf_Last->getType() == pf_Frag::PFT_FmtMark)
			pf_Last = pf_Last->getPrev();
		
		if (pf_Before && pf_Before->getType() == pf_Frag::PFT_Strux &&
			pf_Last && pf_Last->getType() == pf_Frag::PFT_Strux)
		{
			PTStruxType pt_BeforeType = static_cast<pf_Frag_Strux *>(pf_Before)->getStruxType();
			PTStruxType pt_LastType = static_cast<pf_Frag_Strux *>(pf_Last)->getStruxType();

			if (pt_BeforeType == PTX_Block && pt_LastType == PTX_Block)
			{
				//  If we are the structure of the document is
				//  '[Block] ... [Block]' and we are deleting the
				//  '... [Block]' part, then the user is probably expecting
				//  us to delete '[Block] ... ' instead, so that any text
				//  following the second block marker retains its properties.
				//  The problem is that it might not be safe to delete the
				//  first block marker until the '...' is deleted because 
				//  it might be the first block of the section.  So, we 
				//  want to delete the '...' first, and then get around
				//  to deleting the block later.

				pf_Frag_Strux * pfs_BeforeSection, * pfs_LastSection;
				_getStruxOfTypeFromPosition(dpos1 - 1, 
											PTX_Section, &pfs_BeforeSection);
				_getStruxOfTypeFromPosition(dpos2 - 1, 
											PTX_Section, &pfs_LastSection);

				if (pfs_BeforeSection == pfs_LastSection)
				{
					dpos2 -= pf_Last->getLength();
					pstDelayStruxDelete->push(pf_Before);
					return true;
				}
			}
		}
	}

	return true;
}

bool pt_PieceTable::_tweakDeleteSpan(PT_DocPosition & dpos1, 
									 PT_DocPosition & dpos2,
									 UT_Stack * pstDelayStruxDelete) const
{
	//  We want to keep tweaking the delete span until there is nothing
	//  more to tweak.  We check to see if nothing has changed in the
	//  last tweak, and if so, we are done. 
//	while (1)
//	{
//		PT_DocPosition old_dpos1 = dpos1;
//		PT_DocPosition old_dpos2 = dpos2;
//		UT_uint32 old_iStackSize = pstDelayStruxDelete->getDepth();

	if(!_tweakDeleteSpanOnce(dpos1, dpos2, pstDelayStruxDelete))
		return false;

//		if (dpos1 == old_dpos1 && dpos2 == old_dpos2
//			&& pstDelayStruxDelete->getDepth() == old_iStackSize)
	return true;
//	}
}

bool pt_PieceTable::_deleteFormatting(PT_DocPosition dpos1,
									  PT_DocPosition dpos2)
{
	pf_Frag * pf_First;
	pf_Frag * pf_End;
	PT_BlockOffset fragOffset_First;
	PT_BlockOffset fragOffset_End;
	
	bool bFound = getFragsFromPositions(dpos1,dpos2,&pf_First,&fragOffset_First,&pf_End,&fragOffset_End);
	UT_ASSERT(bFound);

	// before we delete the content, we do a quick scan and delete
	// any FmtMarks first -- this let's us avoid problems with
	// coalescing FmtMarks only to be deleted.

	pf_Frag * pfTemp = pf_First;
	PT_BlockOffset fragOffsetTemp = fragOffset_First;

	PT_DocPosition dposTemp = dpos1;
	while (dposTemp <= dpos2)
	{
		if (pfTemp->getType() == pf_Frag::PFT_EndOfDoc)
			break;
			
		if (pfTemp->getType() == pf_Frag::PFT_FmtMark)
		{
			pf_Frag * pfNewTemp;
			PT_BlockOffset fragOffsetNewTemp;
			pf_Frag_Strux * pfsContainerTemp = NULL;
			bool bFoundStrux = _getStruxFromPosition(dposTemp,&pfsContainerTemp);
			UT_ASSERT(bFoundStrux);
			bool bResult = _deleteFmtMarkWithNotify(dposTemp,static_cast<pf_Frag_FmtMark *>(pfTemp),
													pfsContainerTemp,&pfNewTemp,&fragOffsetNewTemp);
			UT_ASSERT(bResult);

			// FmtMarks have length zero, so we don't need to update dposTemp.
			pfTemp = pfNewTemp;
			fragOffsetTemp = fragOffsetNewTemp;
		}
		else
		{
			dposTemp += pfTemp->getLength() - fragOffsetTemp;
			pfTemp = pfTemp->getNext();
			fragOffsetTemp = 0;
		}
	}

	return true;
}

bool pt_PieceTable::_deleteComplexSpan(PT_DocPosition dpos1,
									   PT_DocPosition dpos2)
{
	pf_Frag * pfNewEnd;
	UT_uint32 fragOffsetNewEnd;

	pf_Frag * pf_First;
	pf_Frag * pf_End;
	PT_BlockOffset fragOffset_First;
	PT_BlockOffset fragOffset_End;
	
	bool bFound = getFragsFromPositions(dpos1,dpos2,&pf_First,&fragOffset_First,&pf_End,&fragOffset_End);
	UT_ASSERT(bFound);

	pf_Frag_Strux * pfsContainer = NULL;
	bool bFoundStrux = _getStruxFromPosition(dpos1,&pfsContainer);
	UT_ASSERT(bFoundStrux);

	// loop to delete the amount requested, one text fragment at a time.
	// if we encounter any non-text fragments along the way, we delete
	// them too.  that is, we implicitly delete Strux and Objects here.

	UT_uint32 length = dpos2 - dpos1;
	while (length > 0)
	{
		UT_uint32 lengthInFrag = pf_First->getLength() - fragOffset_First;
		UT_uint32 lengthThisStep = UT_MIN(lengthInFrag, length);
		
		switch (pf_First->getType())
		{
		case pf_Frag::PFT_EndOfDoc:
		default:
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return false;
			
		case pf_Frag::PFT_Strux:
		{
			pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *> (pf_First);
				
			bool bResult = _deleteStruxWithNotify(dpos1,pfs,
												  &pfNewEnd,&fragOffsetNewEnd);
			UT_ASSERT(bResult);
			// we do not update pfsContainer because we just deleted pfs.
		}
		break;

		case pf_Frag::PFT_Text:
		{
			bool bResult
				= _deleteSpanWithNotify(dpos1,static_cast<pf_Frag_Text *>(pf_First),
										fragOffset_First,lengthThisStep,
										pfsContainer,&pfNewEnd,&fragOffsetNewEnd);
			UT_ASSERT(bResult);
		}
		break;

		case pf_Frag::PFT_Object:
		{
			bool bResult
				= _deleteObjectWithNotify(dpos1,static_cast<pf_Frag_Object *>(pf_First),
										  fragOffset_First,lengthThisStep,
										  pfsContainer,&pfNewEnd,&fragOffsetNewEnd);
			UT_ASSERT(bResult);
		}
		break;

		case pf_Frag::PFT_FmtMark:
			// we already took care of these...
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
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

	return true;
}


bool pt_PieceTable::_deleteComplexSpan_norec(PT_DocPosition dpos1,
											 PT_DocPosition dpos2)
{
	pf_Frag * pfNewEnd;
	UT_uint32 fragOffsetNewEnd;

	pf_Frag * pf_First;
	pf_Frag * pf_End;
	PT_BlockOffset fragOffset_First;
	PT_BlockOffset fragOffset_End;
	
	bool bFound = getFragsFromPositions(dpos1,dpos2,&pf_First,&fragOffset_First,&pf_End,&fragOffset_End);
	UT_ASSERT(bFound);

	pf_Frag_Strux * pfsContainer = NULL;
	bool bFoundStrux = _getStruxFromPosition(dpos1,&pfsContainer);
	UT_ASSERT(bFoundStrux);

	// loop to delete the amount requested, one text fragment at a time.
	// if we encounter any non-text fragments along the way, we delete
	// them too.  that is, we implicitly delete Strux and Objects here.

	UT_uint32 length = dpos2 - dpos1;
	while (length > 0)
	{
		UT_uint32 lengthInFrag = pf_First->getLength() - fragOffset_First;
		UT_uint32 lengthThisStep = UT_MIN(lengthInFrag, length);
		
		switch (pf_First->getType())
		{
		case pf_Frag::PFT_EndOfDoc:
		default:
			UT_ASSERT(0);
			return false;
			
		case pf_Frag::PFT_Strux:
		{
			pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *> (pf_First);
				
			bool bResult = _deleteStrux_norec(dpos1,pfs,
											  &pfNewEnd,&fragOffsetNewEnd);
			UT_ASSERT(bResult);
			// we do not update pfsContainer because we just deleted pfs.
		}
		break;

		case pf_Frag::PFT_Text:
		{
			bool bResult
				= _deleteSpan_norec(dpos1,static_cast<pf_Frag_Text *>(pf_First),
									fragOffset_First,lengthThisStep,
									pfsContainer,&pfNewEnd,&fragOffsetNewEnd);
			UT_ASSERT(bResult);
		}
		break;

		case pf_Frag::PFT_Object:
		{
			bool bResult
				= _deleteObject_norec(dpos1,static_cast<pf_Frag_Object *>(pf_First),
									  fragOffset_First,lengthThisStep,
									  pfsContainer,&pfNewEnd,&fragOffsetNewEnd);
			UT_ASSERT(bResult);
		}
		break;

		case pf_Frag::PFT_FmtMark:
			// we already took care of these...
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
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

	return true;
}

bool pt_PieceTable::deleteSpan(PT_DocPosition dpos1,
							   PT_DocPosition dpos2,
							   PP_AttrProp *p_AttrProp_Before)
{
	// remove (dpos2-dpos1) characters from the document at the given position.

	UT_ASSERT(m_pts==PTS_Editing);
	UT_ASSERT(dpos2 > dpos1);

	bool bSuccess = true;
	UT_Stack stDelayStruxDelete;

	PT_DocPosition old_dpos2 = dpos2;

	//  Before we begin the delete proper, we might want to adjust the ends 
	//  of the delete slightly to account for expected behavior on 
	//  structural boundaries.
	bSuccess = _tweakDeleteSpan(dpos1, dpos2, &stDelayStruxDelete);
	if (!bSuccess)
	{
		return false;
	}

	// Get the attribute properties before the delete.

	PP_AttrProp AttrProp_Before;

	{
		pf_Frag * pf1;
		PT_BlockOffset Offset1;
		getFragFromPosition(dpos1, &pf1, &Offset1);
		if(pf1->getType() == pf_Frag::PFT_Text)
		{
			const PP_AttrProp *p_AttrProp;
			getAttrProp(((pf_Frag_Text *)pf1)->getIndexAP(), &p_AttrProp);

			AttrProp_Before = *p_AttrProp;
			if(p_AttrProp_Before)
				*p_AttrProp_Before = *p_AttrProp;
		}
	}


	bool bIsSimple = _isSimpleDeleteSpan(dpos1, dpos2) && stDelayStruxDelete.getDepth() == 0;
	if (bIsSimple)
	{
		//  If the delete is sure to be within a fragment, we don't 
		//  need to worry about much of the bookkeeping of a complex
		//  delete.
		bSuccess = _deleteComplexSpan(dpos1, dpos2);
	}
	else
	{
		//  If the delete spans multiple fragments, we need to 
		//  be a bit more careful about deleting the formatting 
		//  first, and then the actual spans.  Also, glob all
		//  changes together.
		beginMultiStepGlob();
		_changePointWithNotify(old_dpos2);

		bSuccess = _deleteFormatting(dpos1, dpos2);
		if (bSuccess)
			bSuccess = _deleteComplexSpan(dpos1, dpos2);

		while (bSuccess && stDelayStruxDelete.getDepth() > 0)
		{
			pf_Frag_Strux * pfs;
			stDelayStruxDelete.pop((void **)&pfs);

			_deleteFormatting(dpos1 - pfs->getLength(), dpos1);

 			pf_Frag *pf;
			PT_DocPosition dp;
			bSuccess = _deleteStruxWithNotify(dpos1 - pfs->getLength(), pfs,
											  &pf, &dp);
		}

		_changePointWithNotify(dpos1);
	}

	// Have we deleted all the text in a paragraph.

	pf_Frag * p_frag_before;
	PT_BlockOffset Offset_before;
	getFragFromPosition(dpos1 - 1, &p_frag_before, &Offset_before);

	pf_Frag * p_frag_after;
	PT_BlockOffset Offset_after;
	getFragFromPosition(dpos1, &p_frag_after, &Offset_after);

	if(((p_frag_before->getType() == pf_Frag::PFT_Strux) ||
		(p_frag_before->getType() == pf_Frag::PFT_EndOfDoc)) &&
	   ((p_frag_after->getType() == pf_Frag::PFT_Strux) ||
		(p_frag_after->getType() == pf_Frag::PFT_EndOfDoc)))
	{
		xxx_UT_DEBUGMSG(("pt_PieceTable::deleteSpan Paragraph empty\n"));

		// All text in paragraph is deleted so insert a text format.

		_insertFmtMarkFragWithNotify(PTC_AddFmt, dpos1, &AttrProp_Before);

	}

	// By ending the glob after having inserted the FmtMark, an undo
	// behaves properly, instead of being a two-step operation. See
	// bug 1140.
	if (!bIsSimple)
	{
		endMultiStepGlob();
	}

	return bSuccess;
}

// need a special delete for a field update because otherwise
// the whole field object would be deleted by _tweakDeleteSpan
bool pt_PieceTable::deleteFieldFrag(pf_Frag * pf)
{
	

	UT_ASSERT(m_pts==PTS_Editing);

	bool bSuccess = true;
	UT_Stack stDelayStruxDelete;

	PT_DocPosition dpos1 = getFragPosition(pf);
	UT_ASSERT(dpos1);
	PT_DocPosition dpos2 = dpos1 + pf->getLength();
    
    
	//  If the delete is sure to be within a fragment, we don't 
	//  need to worry about much of the bookkeeping of a complex
	//  delete.
	bSuccess = _deleteComplexSpan_norec(dpos1, dpos2);
	return bSuccess;
}

void pt_PieceTable::_tweakFieldSpan(PT_DocPosition & dpos1, 
                                    PT_DocPosition & dpos2) const
{
	//  Our job is to adjust the end positions of the delete
	//  operating to delete those structural object that the 
	//  user will expect to have deleted, even if the dpositions
	//  aren't quite right to encompass those.

	pf_Frag * pf_First;
	pf_Frag * pf_End;
	pf_Frag * pf_Other;
	PT_BlockOffset fragOffset_First;
	PT_BlockOffset fragOffset_End;
	
	bool bFound = getFragsFromPositions(dpos1,dpos2,&pf_First,&fragOffset_First,&pf_End,&fragOffset_End);
	UT_ASSERT(bFound);

	pf_Frag_Strux * pfsContainer = NULL;
	bool bFoundStrux = _getStruxFromPosition(dpos1,&pfsContainer);
	UT_ASSERT(bFoundStrux);

    // if start in middle of field widen to include object
    if ((pf_First->getType() == pf_Frag::PFT_Text)&&
        (static_cast<pf_Frag_Text *>(pf_First)->getField()))
    {
        pf_Frag_Text * pft = static_cast<pf_Frag_Text *>(pf_First);
        pf_Frag_Text * pft2 = NULL;
        // we can't delete part of a field so widen deletion to 
        // include object at start
        while (pft->getPrev()->getType() == pf_Frag::PFT_Text)
        {
            pft2 = static_cast<pf_Frag_Text *>(pft->getPrev());
            UT_ASSERT(pft->getField() == pft2->getField());
            pft = pft2;            
        }
        UT_ASSERT(pft->getPrev()->getType() == pf_Frag::PFT_Object);
        pf_Frag_Object *pfo = 
            static_cast<pf_Frag_Object *>(pft->getPrev());
        UT_ASSERT(pfo->getObjectType()==PTO_Field);
        UT_ASSERT(pfo->getField()==pft->getField());
        dpos1 = getFragPosition(pfo);
    }
    // if end in middle of field widen to include whole Frag_Text
    if (((pf_End->getType() == pf_Frag::PFT_Text)&&
         (pf_End)->getField())/*||
								((pf_End->getType() == pf_Frag::PFT_Object
								)&&
								(static_cast<pf_Frag_Object *>(pf_End)
								->getObjectType()==PTO_Field))*/)
    {
        fd_Field * pField = pf_End->getField();
        UT_ASSERT(pField);
        pf_Other = pf_End->getNext();
        UT_ASSERT(pf_Other);
        while (pf_Other->getField()==pField)
        {
            pf_Other = pf_Other->getNext();
            UT_ASSERT(pf_Other);
        }
        dpos2 = getFragPosition(pf_Other);
    }
}
