// TODO consider changing the multi-step stuff to use something
// TODO less fragile.  such as sending a change record for start
// TODO and for end and not try to piggy-back the bits onto the
// TODO first and last one that we generate.

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


#define NrElements(a)		(sizeof(a)/sizeof(a[0]))

/*****************************************************************/
/*****************************************************************/

pt_PieceTable::pt_PieceTable(PD_Document * pDocument)
{
	m_pDocument = pDocument;

	setPieceTableState(PTS_Loading);
	loading.m_indexCurrentInlineAP = 0;
}

pt_PieceTable::~pt_PieceTable()
{
}

void pt_PieceTable::setPieceTableState(PTState pts)
{
	UT_ASSERT(pts >= m_pts);
	
	m_pts = pts;
	m_varset.setPieceTableState(pts);
}

UT_Bool pt_PieceTable::_insertSpan(pf_Frag_Text * pft,
								   PT_BufIndex bi,
								   UT_Bool bLeftSide,
								   PT_BlockOffset fragOffset,
								   UT_uint32 length)
{
	// update the fragment and/or the fragment list.
	// return true if successful.

	UT_uint32 fragLen = pft->getLength();
	
	// try to coalesce this character data with an existing fragment.
	// this is very likely to be sucessful during normal data entry.

	if (bLeftSide && (fragOffset == fragLen))
	{
		// if we are on the left side of the doc position, we
		// received a fragment for which the doc position is
		// either in the middle of or at the right end of.
		// if the fragment length is equal to our offset, we are
		// at the right end of it.
		
		if (m_varset.isContiguous(pft->getBufIndex(),fragLen,bi))
		{
			// we are at the right end of it and the actual data is contiguous.
			// new text is contiguous, we just update the length of this fragment.

			pft->changeLength(fragLen+length);

			// TODO see if we are now contiguous with the next one and try
			// TODO to coalesce them (this will happen on after a delete
			// TODO char followed by undo).

			return UT_TRUE;
		}
	}

	if (!bLeftSide && (fragOffset == 0))
	{
		// if we are on the right side of the doc position,
		// we received a fragment for which the doc position is
		// either in the middle of or at the left end of.
		// if the offset in the fragment is zero, we are at the
		// left end of it.
		// [This case happens when the user hits a right-arrow and
		//  then starts typing, for example.]
		
		if (m_varset.isContiguous(bi,length,pft->getBufIndex()))
		{
			// we are at the left end of it and the actual data is contiguous.
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
		// it, let's stick it in the previous fragment.  (Had bLeftSide
		// been set, we would have been called with the previous fragment
		// anyway.)
		// NOTE: we lie to our caller and let them think that we put it
		// NOTE: in the pft given with bLeftSide==false.  since the everything
		// NOTE: matches here, i don't think it matters.
		// [This case happens when the user hits a left-arrow and then starts
		//  typing, for example.]
		
		pf_Frag * pfPrev = pft->getPrev();
		if (pfPrev && pfPrev->getType()==pf_Frag::PFT_Text)
		{
			pf_Frag_Text * pftPrev = static_cast<pf_Frag_Text *>(pfPrev);
			UT_uint32 prevLength = pftPrev->getLength();
			
			if (   (pftPrev->getIndexAP() == pft->getIndexAP())
				&& (m_varset.isContiguous(pftPrev->getBufIndex(),prevLength,bi)))
			{
				pftPrev->changeLength(prevLength+length);
				return UT_TRUE;
			}
		}
	}
	
	// new text is not contiguous, we need to insert one or two new text
	// fragment(s) into the list.  first we construct a new text fragment
	// for the data that we inserted.

	pf_Frag_Text * pftNew = new pf_Frag_Text(this,bi,length,pft->getIndexAP());
	if (!pftNew)
		return UT_FALSE;

	if (fragOffset == 0)
	{
		// if change is at the beginning of the fragment, we insert a
		// single new text fragment before the one we found.

		m_fragments.insertFrag(pft->getPrev(),pftNew);
		return UT_TRUE;
	}

	if (fragLen==fragOffset)
	{
		// if the change is after this fragment, we insert a single
		// new text fragment after the one we found.

		m_fragments.insertFrag(pft,pftNew);
		return UT_TRUE;
	}

	// if the change is in the middle of the fragment, we construct
	// a second new text fragment for the portion after the insert.

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
								  UT_Bool bLeftSide,
								  UT_UCSChar * p,
								  UT_uint32 length)
{
	// insert character data into the document at the given position.

	UT_ASSERT(m_pts==PTS_Editing);

	// append the text data to the end of the current buffer.

	PT_BufIndex bi;
	if (!m_varset.appendBuf(p,length,&bi))
		return UT_FALSE;

	// get the fragment at the given document position.
	
	pf_Frag_Strux * pfs = NULL;
	pf_Frag_Text * pft = NULL;
	PT_BlockOffset fragOffset = 0;
	if (!getTextFragFromPosition(dpos,bLeftSide,&pfs,&pft,&fragOffset))
		return UT_FALSE;

	if (!_insertSpan(pft,bi,bLeftSide,fragOffset,length))
		return UT_FALSE;

	// create a change record, add it to the history, and notify
	// anyone listening.
	
	PX_ChangeRecord_Span * pcr
		= new PX_ChangeRecord_Span(PX_ChangeRecord::PXT_InsertSpan,PX_ChangeRecord::PXF_Null,
								   dpos,bLeftSide,pft->getIndexAP(),bi,length);
	UT_ASSERT(pcr);
	m_history.addChangeRecord(pcr);
	m_pDocument->notifyListeners(pfs,pcr);

	return UT_TRUE;
}

void pt_PieceTable::_unlinkFrag(pf_Frag * pf,
								pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd)
{
	// unlink the given fragment from the fragment list.
	// also, see if the adjacent fragments can be coalesced.
	// in (ppfEnd,pfragOffsetEnd) we return the position
	// immediately past the frag we're deleting.
	// the caller is responsible for deleting pf.

	if (ppfEnd)
		*ppfEnd = pf->getNext();
	if (pfragOffsetEnd)
		*pfragOffsetEnd = 0;
	
	pf_Frag * pp = pf->getPrev();

	m_fragments.unlinkFrag(pf);

	if (   pp
		&& pp->getType()==pf_Frag::PFT_Text
		&& pp->getNext()
		&& pp->getNext()->getType()==pf_Frag::PFT_Text)
	{
		pf_Frag_Text * ppt = static_cast<pf_Frag_Text *> (pp);
		pf_Frag_Text * pnt = static_cast<pf_Frag_Text *> (pp->getNext());
		UT_uint32 prevLength = ppt->getLength();
		if (   ppt->getIndexAP() == pnt->getIndexAP()
			&& m_varset.isContiguous(ppt->getBufIndex(),prevLength,pnt->getBufIndex()))
		{
			if (ppfEnd)
				*ppfEnd = pp;
			if (pfragOffsetEnd)
				*pfragOffsetEnd = prevLength;

			ppt->changeLength(prevLength+pnt->getLength());
			m_fragments.unlinkFrag(pnt);
			delete pnt;
		}
	}
}

UT_Bool pt_PieceTable::_deleteSpan(pf_Frag_Text * pft, UT_uint32 fragOffset,
								   PT_BufIndex bi, UT_uint32 length,
								   pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd)
{
	// perform simple delete of a span of text.
	// we assume that it is completely contained within this fragment.

	UT_ASSERT(fragOffset+length <= pft->getLength());

	if (ppfEnd)
		*ppfEnd = pft;
	if (pfragOffsetEnd)
		*pfragOffsetEnd = fragOffset;
	
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

		if (ppfEnd)
			*ppfEnd = pft->getNext();
		if (pfragOffsetEnd)
			*pfragOffsetEnd = 0;
		
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

	if (ppfEnd)
		*ppfEnd = pftTail;
	if (pfragOffsetEnd)
		*pfragOffsetEnd = 0;
	
	return UT_TRUE;
}

UT_Bool pt_PieceTable::_deleteSpanWithNotify(PT_DocPosition dpos, UT_Bool bLeftSide,
											 pf_Frag_Text * pft, UT_uint32 fragOffset,
											 UT_uint32 length,
											 UT_Byte changeFlags,
											 pf_Frag_Strux * pfs,
											 pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd)
{
	UT_ASSERT(length > 0);
	
	// create a change record for this change and put it in the history.
	// we do this before the actual change because various fields that
	// we need are blown away during the delete.  we then notify all
	// listeners of the change.
		
	PX_ChangeRecord_Span * pcr
		= new PX_ChangeRecord_Span(PX_ChangeRecord::PXT_DeleteSpan,changeFlags,
								   dpos,bLeftSide,pfs->getIndexAP(),
								   m_varset.getBufIndex(pft->getBufIndex(),fragOffset),
								   length);
	UT_ASSERT(pcr);
	m_history.addChangeRecord(pcr);
	UT_Bool bResult = _deleteSpan(pft,fragOffset,pft->getBufIndex(),length,ppfEnd,pfragOffsetEnd);
	m_pDocument->notifyListeners(pfs,pcr);

	return bResult;
}

UT_Bool pt_PieceTable::deleteSpan(PT_DocPosition dpos,
								  UT_Bool bLeftSide1,
								  UT_Bool bLeftSide2,
								  UT_uint32 length)
{
	// TODO consider changing the signature of this function to be
	// TODO 2 PT_DocPositions rather than a position and a length.
	// TODO this might save the caller from having to count the delta.
	
	// remove length characters from the document at the given position.
	// we interpret the bLeftSide1 flag at the (dpos) and bLeftSide2 at
	// (dpos+length).
	
	UT_ASSERT(m_pts==PTS_Editing);

	struct _x
	{
		UT_Bool				x_bLeftSide;
		pf_Frag_Strux *		x_pfs;
		pf_Frag_Text *		x_pft;
		PT_BlockOffset		x_fragOffset;
	};

	struct _x f = { bLeftSide1, NULL, NULL, 0 }; // first
	struct _x e = { bLeftSide2, NULL, NULL, 0 }; // end

	if (   !getTextFragFromPosition(dpos,       f.x_bLeftSide,&f.x_pfs,&f.x_pft,&f.x_fragOffset)
		|| !getTextFragFromPosition(dpos+length,e.x_bLeftSide,&e.x_pfs,&e.x_pft,&e.x_fragOffset))
	{
		// could not find a text fragment containing the given
		// absolute document position ???
		return UT_FALSE;
	}
	
	// see if the amount of text to be deleted is completely
	// contained withing the fragment found.  if so, we have
	// a simple delete.  otherwise, we need to set up a multi-step
	// delete.
	//
	// we are in a simple change if:
	// [1] beginning and end are within the same fragment.
	// [2] we are at the end of the first fragment and the end fragment
	//     immediately follows the first (would have been case [1] but
	//     bLeftSide1 probably switched).
	// [3] we are at the beginning of the end fragment and the end
	//     fragment immediately follows the first (would have been 
	//     case [1] but bLeftSide2 probably switched).
	// [4] we are at the end of the first fragment and the beginning
	//     of end fragment and there's only one fragment between them
	//     (like case [1] but both bLeftSides switched).
	//
	// cases [2,3,4] are likely if you delete the current selection
	// and it is on the edge of a paragraph break.
	
	UT_Bool bSimple = (   (f.x_pft == e.x_pft)								// case [1]
					   || (f.x_fragOffset==f.x_pft->getLength()
						   && f.x_pft->getNext() == e.x_pft)				// case [2]
					   || (e.x_fragOffset==0
						   && f.x_pft->getNext() == e.x_pft)				// case [3]
					   || (f.x_fragOffset==f.x_pft->getLength()
						   && e.x_fragOffset==0
						   && f.x_pft->getNext() == e.x_pft->getPrev()));	// case [4]
	
	PX_ChangeRecord::PXFlags fMultiStepStart = PX_ChangeRecord::PXF_Null;
	PX_ChangeRecord::PXFlags fMultiStepEnd = PX_ChangeRecord::PXF_Null;

	if (!bSimple)
		fMultiStepStart = PX_ChangeRecord::PXF_MultiStepStart;

	// loop to delete the amount requested, one text fragment at a time.
	// if we encounter any non-text fragments along the way, we delete
	// them too.  that is, we implicitly delete paragraphs here.

	pf_Frag * pfNewEnd;
	UT_uint32 fragOffsetNewEnd;

	UT_Bool bFinished = UT_FALSE;
	while (!bFinished)
	{
		// TODO we need to fix pf_Frag_Strux's so that they take up a position
		// TODO (have length 1) so that we can index them using a PT_DocPosition.
		// TODO until then, there are some kinds of selections that we cannot
		// TODO correctly represent and delete (such as at the edges of
		// TODO paragraphs).  for now, we will try as best as we can.  therefore,
		// TODO we need to change getTextFragFromPosition() to be getFragFromPosition()
		// TODO and change the x_pft to be x_pf.  the logic above the loop will always
		// TODO start us at a text fragment, but the correct behavior is that we
		// TODO can start at any type of fragment.  logic at the bottom of this
		// TODO loop has been changed to advance us by one frag.  so for now,
		// TODO we do a little back casting.

		switch (f.x_pft->getType())
		{
		default:
			UT_ASSERT(0);
			return UT_FALSE;
			
		case pf_Frag::PFT_Strux:
			{
				pf_Frag * pf = f.x_pft;
				pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *> (pf);

				// TODO when strux have length, we will need to decrement 'length'.
				// TODO also, we probably won't need this complicated stuff to
				// TODO determine whether to set the end- flag.  we should be
				// TODO able to just set it at the bottom of the loop.
				// set the end- type for our last trip thru the loop.
				if (!bSimple)
				{
					if (!f.x_pft->getNext())
						fMultiStepEnd = PX_ChangeRecord::PXF_MultiStepEnd;
					else if (   (length==0)
							 && (f.x_pft->getNext()->getType() == pf_Frag::PFT_Text))
						fMultiStepEnd = PX_ChangeRecord::PXF_MultiStepEnd;
				}
				
				// TODO decide if we need to send f.x_bLeftSide or if it matters.
				UT_Bool bResult = _deleteStruxWithNotify(dpos,f.x_bLeftSide,pfs,
														 fMultiStepStart | fMultiStepEnd,
														 &pfNewEnd,&fragOffsetNewEnd);
				UT_ASSERT(bResult);
			}
			break;

		case pf_Frag::PFT_Text:
			{
				// figure out how much to consume during this iteration.  This can
				// be zero if we have a strux within the sequence and they gave us
				// bLeftSide1==TRUE, for example.

				UT_uint32 lengthInFrag = f.x_pft->getLength() - f.x_fragOffset;
				UT_uint32 lengthThisStep = UT_MIN(lengthInFrag, length);
				length -= lengthThisStep;

				// TODO when strux have length, we probably won't need this complicated
				// TODO stuff to determine whether to set the end flag.  we should be
				// TODO able to just set it at the bottom of the loop.
				// set the end- type for our last trip thru the loop.
				if (!bSimple)
				{
					if (!f.x_pft->getNext())
						fMultiStepEnd = PX_ChangeRecord::PXF_MultiStepEnd;
					else if (length==0)
					{
						if (bLeftSide2)
							fMultiStepEnd = PX_ChangeRecord::PXF_MultiStepEnd;
						else if (lengthInFrag > lengthThisStep)
							fMultiStepEnd = PX_ChangeRecord::PXF_MultiStepEnd;
						else if (f.x_pft->getNext()->getType() == pf_Frag::PFT_Text)
							fMultiStepEnd = PX_ChangeRecord::PXF_MultiStepEnd;
					}
				}
				
				UT_Bool bResult = _deleteSpanWithNotify(dpos,UT_FALSE,f.x_pft,f.x_fragOffset,
														lengthThisStep,(fMultiStepStart | fMultiStepEnd),
														f.x_pfs,&pfNewEnd,&fragOffsetNewEnd);
				UT_ASSERT(bResult);
			}
			break;
		}

		// since _delete{*}WithNotify(), can delete f.x_pft, mess with the
		// fragment list, and does some aggressive coalescing of
		// fragments, we cannot just do a f.x_pft->getNext() here.
		// to advance to the next fragment, we use the *NewEnd variables
		// that each of the _delete routines gave us.

		// TODO when strux have length and we change f.x_pft to be f.x_pf,
		// TODO we can remove this static cast.
		f.x_pft = static_cast<pf_Frag_Text *> (pfNewEnd);
		f.x_fragOffset = fragOffsetNewEnd;
		
		if (   (bSimple && (length==0))
			|| (!bSimple && (fMultiStepEnd == PX_ChangeRecord::PXF_MultiStepEnd)))
		{
			// TODO when we change strux to have a length, we probably don't
			// TODO need this complexity -- just set bFinished when length==0.
			// TODO we keep looping until we have deleted the requested amount.
			// TODO when length reaches zero, we still may have to loop, to
			// TODO make sure that we get any strux on the trailing edge.

			bFinished = UT_TRUE;
		}

		if (!bSimple)
		{
			// make sure that we only indicate a start- type once.

			fMultiStepStart = PX_ChangeRecord::PXF_Null;

			// TODO when we change strux to have a length, do something
			// TODO like:
			// TODO   if (length==0) // set the end- type for our last trip thru the loop.
			// TODO       fMultiStepEnd = PX_ChangeRecord::PXF_MultiStepEnd;
			// TODO and delete the code to do this in the switch above.
		}
	}

	UT_ASSERT(bSimple || (fMultiStepStart == PX_ChangeRecord::PXF_Null));
	UT_ASSERT(bSimple || (fMultiStepEnd == PX_ChangeRecord::PXF_MultiStepEnd));
		
	return UT_TRUE;
}

UT_Bool pt_PieceTable::_fmtChangeStruxWithNotify(PTChangeFmt ptc,
												 pf_Frag_Strux * pfs,
												 const XML_Char ** attributes,
												 const XML_Char ** properties,
												 UT_Byte changeFlag,
												 pf_Frag ** ppfNewEnd,
												 UT_uint32 * pfragOffsetNewEnd)
{
	return UT_TRUE;
}

UT_Bool pt_PieceTable::_fmtChange(pf_Frag_Text * pft, UT_uint32 fragOffset, UT_uint32 length,
								  PT_AttrPropIndex indexNewAP,
								  pf_Frag ** ppfNewEnd, UT_uint32 * pfragOffsetNewEnd)
{
	UT_ASSERT(length > 0);
	UT_ASSERT(fragOffset+length <= pft->getLength());
	
	// insert a format change within this text fragment.

	// TODO for each place in this function where we apply a change
	// TODO see if the new fragment could be coalesced with something
	// TODO already in the fragment list.
	
	if ((fragOffset == 0) && (length == pft->getLength()))
	{
		// we have an exact match (we are changing the entire fragment).
		// let's just overwrite the indexAP and return.

		pft->setIndexAP(indexNewAP);

		if (ppfNewEnd)
			*ppfNewEnd = pft->getNext();
		if (pfragOffsetNewEnd)
			*pfragOffsetNewEnd = 0;
		
		return UT_TRUE;
	}

	if (fragOffset == 0)
	{
		// the change is at the beginning of the fragment, we cut
		// the existing fragment into 2 parts and apply the new
		// formatting to the new one.

		UT_uint32 len_1 = length;
		UT_uint32 len_2 = pft->getLength() - len_1;
		PT_BufIndex bi_1 = m_varset.getBufIndex(pft->getBufIndex(),0);
		PT_BufIndex bi_2 = m_varset.getBufIndex(pft->getBufIndex(),len_1);
		pf_Frag_Text * pftNew = new pf_Frag_Text(this,bi_1,len_1,indexNewAP);
		if (!pftNew)
			return UT_FALSE;

		pft->adjustOffsetLength(bi_2,len_2);
		m_fragments.insertFrag(pft->getPrev(),pftNew);

		if (ppfNewEnd)
			*ppfNewEnd = pft;
		if (pfragOffsetNewEnd)
			*pfragOffsetNewEnd = 0;
		
		return UT_TRUE;
	}

	if (fragOffset+length == pft->getLength())
	{
		// the change is at the end of the fragment, we cut
		// the existing fragment into 2 parts and apply the new
		// formatting to the new one.

		UT_uint32 len_1 = fragOffset;
		UT_uint32 len_2 = length;
		PT_BufIndex bi_2 = m_varset.getBufIndex(pft->getBufIndex(),len_1);
		pf_Frag_Text * pftNew = new pf_Frag_Text(this,bi_2,len_2,indexNewAP);
		if (!pftNew)
			return UT_FALSE;

		pft->changeLength(len_1);
		m_fragments.insertFrag(pft,pftNew);

		if (ppfNewEnd)
			*ppfNewEnd = pftNew->getNext();
		if (pfragOffsetNewEnd)
			*pfragOffsetNewEnd = 0;
		
		return UT_TRUE;
	}

	// otherwise, change is in the middle of the fragment.  we
	// need to cut the existing fragment into 3 parts and apply
	// the new formatting to the middle one.

	UT_uint32 len_1 = fragOffset;
	UT_uint32 len_2 = length;
	UT_uint32 len_3 = pft->getLength() - (fragOffset+length);
	PT_BufIndex bi_2 = m_varset.getBufIndex(pft->getBufIndex(),fragOffset);
	PT_BufIndex bi_3 = m_varset.getBufIndex(pft->getBufIndex(),fragOffset+length);
	pf_Frag_Text * pft_2 = new pf_Frag_Text(this,bi_2,len_2,indexNewAP);
	UT_ASSERT(pft_2);
	pf_Frag_Text * pft_3 = new pf_Frag_Text(this,bi_3,len_3,pft->getIndexAP());
	UT_ASSERT(pft_3);

	pft->changeLength(len_1);
	m_fragments.insertFrag(pft,pft_2);
	m_fragments.insertFrag(pft_2,pft_3);

	if (ppfNewEnd)
		*ppfNewEnd = pft_3;
	if (pfragOffsetNewEnd)
		*pfragOffsetNewEnd = 0;
		
	return UT_TRUE;
}
	
UT_Bool pt_PieceTable::_fmtChangeSpanWithNotify(PTChangeFmt ptc,
												pf_Frag_Text * pft, UT_uint32 fragOffset,
												PT_DocPosition dpos,
												UT_uint32 length,
												const XML_Char ** attributes,
												const XML_Char ** properties,
												UT_Byte changeFlag,
												pf_Frag_Strux * pfs,
												pf_Frag ** ppfNewEnd,
												UT_uint32 * pfragOffsetNewEnd)
{
	// create a change record for this change and put it in the history.

	UT_ASSERT(length > 0);

	PT_AttrPropIndex indexNewAP;
	PT_AttrPropIndex indexOldAP = pft->getIndexAP();
	UT_Bool bMerged = m_varset.mergeAP(ptc,indexOldAP,attributes,properties,&indexNewAP);
	UT_ASSERT(bMerged);

	if (indexOldAP == indexNewAP)
	{
		// the requested change had no effect on this fragment.
		// try to avoid actually logging the change.  we can only
		// do this if we are in a single-step change or not one
		// of the ends of a multi-step change.

		if (changeFlag == PX_ChangeRecord::PXF_Null)
			return UT_TRUE;
	}

	// we do this before the actual change because various fields that
	// we need may be blown away during the change.  we then notify all
	// listeners of the change.

	UT_Bool bLeftSide = UT_TRUE;		// TODO we are going to delete these.
	PX_ChangeRecord_SpanChange * pcr
		= new PX_ChangeRecord_SpanChange(PX_ChangeRecord::PXT_ChangeSpan,changeFlag,
										 dpos,bLeftSide,pfs->getIndexAP(),indexNewAP,ptc,
										 m_varset.getBufIndex(pft->getBufIndex(),fragOffset),
										 length);
	UT_ASSERT(pcr);
	m_history.addChangeRecord(pcr);
	UT_Bool bResult = _fmtChange(pft,fragOffset,length,indexNewAP,ppfNewEnd,pfragOffsetNewEnd);
	m_pDocument->notifyListeners(pfs,pcr);

	return bResult;
}

UT_Bool pt_PieceTable::changeSpanFmt(PTChangeFmt ptc,
									 PT_DocPosition dpos1,
									 UT_Bool bLeftSide1,
									 PT_DocPosition dpos2,
									 UT_Bool bLeftSide2,
									 const XML_Char ** attributes,
									 const XML_Char ** properties)
{
	UT_ASSERT(m_pts==PTS_Editing);

	// apply a span-level formating change to the given region.

	UT_ASSERT(dpos1 < dpos2);
	UT_Bool bHaveAttributes = (attributes && *attributes);
	UT_Bool bHaveProperties = (properties && *properties);
	UT_ASSERT(bHaveAttributes || bHaveProperties); // must have something to do

	struct _x
	{
		UT_Bool				x_bLeftSide;
		pf_Frag_Strux *		x_pfs;
		pf_Frag_Text *		x_pft;
		PT_BlockOffset		x_fragOffset;
	};

	struct _x f = { bLeftSide1, NULL, NULL, 0 }; // first
	struct _x e = { bLeftSide2, NULL, NULL, 0 }; // end

	if (   !getTextFragFromPosition(dpos1,f.x_bLeftSide,&f.x_pfs,&f.x_pft,&f.x_fragOffset)
		|| !getTextFragFromPosition(dpos2,e.x_bLeftSide,&e.x_pfs,&e.x_pft,&e.x_fragOffset))
	{
		// could not find a text fragment containing the given
		// absolute document position ???
		return UT_FALSE;
	}

	// see if the amount of text to be changed is completely
	// contained within a single fragment.  if so, we have a
	// simple change.  otherwise, we need to set up a multi-step
	// change.
	//
	// we are in a simple change if:
	// [1] beginning and end are within the same fragment.
	// [2] we are at the end of the first fragment and the end fragment
	//     immediately follows the first (would have been case [1] but
	//     bLeftSide1 probably switched).
	// [3] we are at the beginning of the end fragment and the end
	//     fragment immediately follows the first (would have been 
	//     case [1] but bLeftSide2 probably switched).
	// [4] we are at the end of the first fragment and the beginning
	//     of end fragment and there's only one fragment between them
	//     (like case [1] but both bLeftSides switched).
	//
	// cases [2,3,4] are likely if you delete the current selection
	// and it is on the edge of a paragraph break.

	// TODO fix this to not include strux items in the test.
	// TODO that is, we only want this true if we have multiple
	// TODO span (text) level items to change.

	UT_Bool bSimple = (   (f.x_pft == e.x_pft)								// case [1]
					   || (f.x_fragOffset==f.x_pft->getLength()
						   && f.x_pft->getNext() == e.x_pft)				// case [2]
					   || (e.x_fragOffset==0
						   && f.x_pft->getNext() == e.x_pft)				// case [3]
					   || (f.x_fragOffset==f.x_pft->getLength()
						   && e.x_fragOffset==0
						   && f.x_pft->getNext() == e.x_pft->getPrev()));	// case [4]
	
	PX_ChangeRecord::PXFlags fMultiStepStart = PX_ChangeRecord::PXF_Null;
	PX_ChangeRecord::PXFlags fMultiStepEnd = PX_ChangeRecord::PXF_Null;

	if (!bSimple)
		fMultiStepStart = PX_ChangeRecord::PXF_MultiStepStart;

	pf_Frag * pfNewEnd;
	UT_uint32 fragOffsetNewEnd;

	UT_uint32 length = dpos2 - dpos1;
	UT_Bool bFinished = UT_FALSE;
	while (!bFinished)
	{
		UT_ASSERT(dpos1+length==dpos2);
		
		// TODO we need to fix pf_Frag_Strux's so that they take up a position.
		switch (f.x_pft->getType())
		{
		default:
			UT_ASSERT(0);
			return UT_FALSE;
			
		case pf_Frag::PFT_Strux:
			{
				pf_Frag * pf = f.x_pft;
				pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *> (pf);
#if 0
				if (!bSimple)
				{
					if (!f.x_pft->getNext())
						fMultiStepEnd = PX_ChangeRecord::PXF_MultiStepEnd;
					else if (   (length==0)
							 && (f.x_pft->getNext()->getType() == pf_Frag::PFT_Text))
						fMultiStepEnd = PX_ChangeRecord::PXF_MultiStepEnd;
				}
				
				UT_Bool bResult = _fmtChangeStruxWithNotify(ptc,pfs,attributes,properties,
															fMultiStepStart | fMultiStepEnd,
															&pfNewEnd,&fragOffsetNewEnd);
				UT_ASSERT(bResult);
				dpos1 += 0;				// TODO when strux have length, change this to 1.
#else
				pfNewEnd = pf->getNext();
				fragOffsetNewEnd = 0;
#endif
			}
			break;

		case pf_Frag::PFT_Text:
			{
				// figure out how much to consume during this iteration.  This can
				// be zero if we have a strux within the sequence and they gave us
				// bLeftSide1==TRUE, for example.

				UT_uint32 lengthInFrag = f.x_pft->getLength() - f.x_fragOffset;
				UT_uint32 lengthThisStep = UT_MIN(lengthInFrag, length);
				length -= lengthThisStep;

				// TODO when strux have length, we probably won't need this complicated
				// TODO stuff to determine whether to set the end flag.  we should be
				// TODO able to just set it at the bottom of the loop.
				// set the end- type for our last trip thru the loop.
				if (!bSimple)
				{
					if (!f.x_pft->getNext())
						fMultiStepEnd = PX_ChangeRecord::PXF_MultiStepEnd;
					else if (length==0)
					{
						if (bLeftSide2)
							fMultiStepEnd = PX_ChangeRecord::PXF_MultiStepEnd;
						else if (lengthInFrag > lengthThisStep)
							fMultiStepEnd = PX_ChangeRecord::PXF_MultiStepEnd;
						else if (f.x_pft->getNext()->getType() == pf_Frag::PFT_Text)
							fMultiStepEnd = PX_ChangeRecord::PXF_MultiStepEnd;
					}
				}
				
				UT_Bool bResult = _fmtChangeSpanWithNotify(ptc,f.x_pft,f.x_fragOffset,
														   dpos1,lengthThisStep,
														   attributes,properties,
														   (fMultiStepStart | fMultiStepEnd),
														   f.x_pfs,&pfNewEnd,&fragOffsetNewEnd);
				UT_ASSERT(bResult);
				dpos1 += lengthThisStep;
			}
			break;
		}

		// since _fmtChange{*}WithNotify(), can delete f.x_pft, mess with the
		// fragment list, and does some aggressive coalescing of
		// fragments, we cannot just do a f.x_pft->getNext() here.
		// to advance to the next fragment, we use the *NewEnd variables
		// that each of the _delete routines gave us.

		// TODO when strux have length and we change f.x_pft to be f.x_pf,
		// TODO we can remove this static cast.
		f.x_pft = static_cast<pf_Frag_Text *> (pfNewEnd);
		f.x_fragOffset = fragOffsetNewEnd;
		
		if (   (bSimple && (length==0))
			|| (!bSimple && (fMultiStepEnd == PX_ChangeRecord::PXF_MultiStepEnd)))
		{
			// TODO when we change strux to have a length, we probably don't
			// TODO need this complexity -- just set bFinished when length==0.
			// TODO we keep looping until we have deleted the requested amount.
			// TODO when length reaches zero, we still may have to loop, to
			// TODO make sure that we get any strux on the trailing edge.

			bFinished = UT_TRUE;
		}

		if (!bSimple)
		{
			// make sure that we only indicate a start- type once.

			fMultiStepStart = PX_ChangeRecord::PXF_Null;

			// TODO when we change strux to have a length, do something
			// TODO like:
			// TODO   if (length==0) // set the end- type for our last trip thru the loop.
			// TODO       fMultiStepEnd = PX_ChangeRecord::PXF_MultiStepEnd;
			// TODO and delete the code to do this in the switch above.
		}
	}

	UT_ASSERT(bSimple || (fMultiStepStart == PX_ChangeRecord::PXF_Null));
	UT_ASSERT(bSimple || (fMultiStepEnd == PX_ChangeRecord::PXF_MultiStepEnd));
		
	return UT_TRUE;
}

UT_Bool pt_PieceTable::deleteFmt(PT_DocPosition dpos1,
								 PT_DocPosition dpos2,
								 const XML_Char ** attributes,
								 const XML_Char ** properties)
{
	return UT_TRUE;
}

UT_Bool pt_PieceTable::_createStrux(PTStruxType pts,
									PT_AttrPropIndex indexAP,
									pf_Frag_Strux ** ppfs)
{
	// create a strux frag for this.
	// return *pfs and true if successful.

	// create an unlinked strux fragment.
	
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
									dpos,bLeftSide,indexAP,pts);
	UT_ASSERT(pcrs);
	m_history.addChangeRecord(pcrs);
	m_pDocument->notifyListeners(pfsPrev,pfsNew,pcrs);

	return UT_TRUE;
}

UT_Bool pt_PieceTable::_struxHasContent(pf_Frag_Strux * pfs) const
{
	// return true iff the paragraph has content (text).

	return (pfs->getNext() && (pfs->getNext()->getType() == pf_Frag::PFT_Text));
}

UT_Bool pt_PieceTable::_unlinkStrux_Block(pf_Frag_Strux * pfs,
										  pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd)
{
	// unlink this strux from the document.
	// the caller is responsible for deleting pfs.

	if (ppfEnd)
		*ppfEnd = pfs->getNext();
	if (pfragOffsetEnd)
		*pfragOffsetEnd = 0;
	
	// find the previous strux (either a paragraph or something else).

	pf_Frag_Strux * pfsPrev = NULL;
	pf_Frag * pf = pfs->getPrev();
	while (pf && !pfsPrev)
	{
		if (pf->getType() == pf_Frag::PFT_Strux)
			pfsPrev = static_cast<pf_Frag_Strux *> (pf);
		pf = pf->getPrev();
	}
	UT_ASSERT(pfsPrev);			// we have a block that's not in a section ??

	switch (pfsPrev->getStruxType())
	{
	case PTX_Block:
		// if there is a paragraph before us, we can delete this
		// paragraph knowing that our content will be assimilated
		// in to the previous one.

		_unlinkFrag(pfs,ppfEnd,pfragOffsetEnd);
		return UT_TRUE;

	case PTX_Section:
	case PTX_ColumnSet:
	case PTX_Column:
		// we are the first paragraph in this section.  if we have
		// content, we cannot be deleted, since there is no one to
		// inherit our content.

		if (_struxHasContent(pfs))
		{
			// TODO decide if this should assert or just fail...
			UT_DEBUGMSG(("Cannot delete first paragraph with content.\n"));
			UT_ASSERT(0);
			return UT_FALSE;
		}

		// no content in this paragraph.
		
		_unlinkFrag(pfs,ppfEnd,pfragOffsetEnd);
		return UT_TRUE;

	default:
		UT_ASSERT(0);
		return UT_FALSE;
	}
}
			
UT_Bool pt_PieceTable::_deleteStruxWithNotify(PT_DocPosition dpos, UT_Bool bLeftSide,
											  pf_Frag_Strux * pfs,
											  UT_Byte changeFlags,
											  pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd)
{
	switch (pfs->getStruxType())
	{
	case PTX_Section:
	case PTX_ColumnSet:
	case PTX_Column:
		UT_ASSERT(0);					// TODO
		break;
		
	case PTX_Block:
		if (!_unlinkStrux_Block(pfs,ppfEnd,pfragOffsetEnd))
			return UT_FALSE;
		break;
		
	default:
		UT_ASSERT(0);
		return UT_FALSE;
	}

	// create a change record to describe the change, add
	// it to the history, and let our listeners know about it.
	
	PX_ChangeRecord_Strux * pcrs
		= new PX_ChangeRecord_Strux(PX_ChangeRecord::PXT_DeleteStrux,changeFlags,
									dpos,bLeftSide,pfs->getIndexAP(),pfs->getStruxType());
	UT_ASSERT(pcrs);
	m_history.addChangeRecord(pcrs);
	m_pDocument->notifyListeners(pfs,pcrs);

	delete pfs;

	return UT_TRUE;
}

UT_Bool pt_PieceTable::appendStrux(PTStruxType pts, const XML_Char ** attributes)
{
	// create a new structure fragment at the current end of the document.
	// this function can only be called while loading the document.
	UT_ASSERT(m_pts==PTS_Loading);

	// first, store the attributes and properties and get an index to them.
	
	PT_AttrPropIndex indexAP;
	if (!m_varset.storeAP(attributes,&indexAP))
		return UT_FALSE;

	pf_Frag_Strux * pfs = NULL;
	if (!_createStrux(pts,indexAP,&pfs))
		return UT_FALSE;
	
	m_fragments.appendFrag(pfs);
	return UT_TRUE;
}

UT_Bool pt_PieceTable::appendFmt(const XML_Char ** attributes)
{
	// can only be used while loading the document
	UT_ASSERT(m_pts==PTS_Loading);

	// create a new Attribute/Property structure in the table
	// and set the current index to it.  the next span of text
	// (in this block) that comes in will then be set to these
	// attributes/properties.  becase we are loading, we do not
	// create a Fragment or a ChangeRecord.  (Formatting changes
	// are implicit at this point in time.)

	if (!m_varset.storeAP(attributes,&loading.m_indexCurrentInlineAP))
		return UT_FALSE;

	return UT_TRUE;
}

UT_Bool pt_PieceTable::appendFmt(const UT_Vector * pVecAttributes)
{
	// can only be used while loading the document
	UT_ASSERT(m_pts==PTS_Loading);

	if (!m_varset.storeAP(pVecAttributes,&loading.m_indexCurrentInlineAP))
		return UT_FALSE;

	return UT_TRUE;
}

UT_Bool pt_PieceTable::appendSpan(UT_UCSChar * pbuf, UT_uint32 length)
{
	// can only be used while loading the document
	UT_ASSERT(m_pts==PTS_Loading);

	// create a new fragment for this text span.
	// append the text data to the end of the buffer.
	// set the formatting Attributes/Properties to that
	// of the last fmt set in this paragraph.
	// becauase we are loading, we do not create change
	// records or any of the other stuff that an insertSpan
	// would do.

	PT_BufIndex bi;
	if (!m_varset.appendBuf(pbuf,length,&bi))
		return UT_FALSE;

	pf_Frag_Text * pft = new pf_Frag_Text(this,bi,length,loading.m_indexCurrentInlineAP);
	if (!pft)
		return UT_FALSE;

	m_fragments.appendFrag(pft);
	return UT_TRUE;
}

UT_Bool pt_PieceTable::addListener(PL_Listener * pListener,
								   PL_ListenerId listenerId)
{
	// walk document and for each fragment, send a notification
	// to each layout.

	PL_StruxFmtHandle sfh = 0;
	
	for (pf_Frag * pf = m_fragments.getFirst(); (pf); pf=pf->getNext())
	{
		switch (pf->getType())
		{
		case pf_Frag::PFT_Text:
			{
				pf_Frag_Text * pft = static_cast<pf_Frag_Text *> (pf);
				PX_ChangeRecord * pcr = 0;
				UT_Bool bStatus = (   pft->createSpecialChangeRecord(&pcr)
								   && pListener->populate(sfh,pcr));
				if (pcr)
					delete pcr;
				if (!bStatus)
					return UT_FALSE;
			}
			break;
			
		case pf_Frag::PFT_Strux:
			{
				pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *> (pf);
				PL_StruxDocHandle sdh = (PL_StruxDocHandle)pf;
				sfh = 0;
				PX_ChangeRecord * pcr = 0;
				UT_Bool bStatus = (   pfs->createSpecialChangeRecord(&pcr)
								   && pListener->populateStrux(sdh,pcr,&sfh)
								   && pfs->setFmtHandle(listenerId,sfh));
				if (pcr)
					delete pcr;
				if (!bStatus)
					return UT_FALSE;
			}
			break;
			
		default:
			UT_ASSERT(0);
			return UT_FALSE;
		}
	}

	return UT_TRUE;
}

UT_Bool pt_PieceTable::getAttrProp(PT_AttrPropIndex indexAP, const PP_AttrProp ** ppAP) const
{
	UT_ASSERT(ppAP);

	const PP_AttrProp * pAP = m_varset.getAP(indexAP);
	if (!pAP)
		return UT_FALSE;

	*ppAP = pAP;
	return UT_TRUE;
}

UT_Bool pt_PieceTable::getSpanAttrProp(PL_StruxDocHandle sdh, UT_uint32 offset,
									   const PP_AttrProp ** ppAP) const
{
	// return the AP for the text at the given offset from the given strux.
	
	UT_ASSERT(sdh);
	UT_ASSERT(ppAP);

	pf_Frag * pf = (pf_Frag *)sdh;
	UT_ASSERT(pf->getType() == pf_Frag::PFT_Strux);
	pf_Frag_Strux * pfsBlock = static_cast<pf_Frag_Strux *> (pf);
	UT_ASSERT(pfsBlock->getStruxType() == PTX_Block);

	UT_uint32 cumOffset = 0;
	for (pf_Frag * pfTemp=pfsBlock->getNext(); (pfTemp); pfTemp=pfTemp->getNext())
	{
		if (pfTemp->getType() != pf_Frag::PFT_Text)
			continue;

		pf_Frag_Text * pfText = static_cast<pf_Frag_Text *> (pfTemp);
		
		if ((offset >= cumOffset) && (offset < cumOffset+pfText->getLength()))
		{
			const PP_AttrProp * pAP = m_varset.getAP(pfText->getIndexAP());
			if (!pAP)
				return UT_FALSE;

			*ppAP = pAP;
			return UT_TRUE;
		}

		cumOffset += pfText->getLength();
	}
	return UT_FALSE;
}

const UT_UCSChar * pt_PieceTable::getPointer(PT_BufIndex bi) const
{
	// the pointer that we return is NOT a zero-terminated
	// string.  the caller is responsible for knowing how
	// long the data is within the span/fragment.
	
	return m_varset.getPointer(bi);
}

UT_Bool pt_PieceTable::getSpanPtr(PL_StruxDocHandle sdh, UT_uint32 offset,
								  const UT_UCSChar ** ppSpan, UT_uint32 * pLength) const
{
	pf_Frag * pf = (pf_Frag *)sdh;
	UT_ASSERT(pf->getType() == pf_Frag::PFT_Strux);
	pf_Frag_Strux * pfsBlock = static_cast<pf_Frag_Strux *> (pf);
	UT_ASSERT(pfsBlock->getStruxType() == PTX_Block);

	UT_uint32 cumOffset = 0;
	for (pf_Frag * pfTemp=pfsBlock->getNext(); (pfTemp); pfTemp=pfTemp->getNext())
	{
		if (pfTemp->getType() != pf_Frag::PFT_Text)
			continue;

		pf_Frag_Text * pfText = static_cast<pf_Frag_Text *> (pfTemp);
		
		if (offset == cumOffset)
		{
			*ppSpan = getPointer(pfText->getBufIndex());
			*pLength = pfText->getLength();
			return UT_TRUE;
		}
		if (offset < cumOffset+pfText->getLength())
		{
			const UT_UCSChar * p = getPointer(pfText->getBufIndex());
			UT_uint32 delta = offset - cumOffset;
			*ppSpan = p + delta;
			*pLength = pfText->getLength() - delta;
			return UT_TRUE;
		}

		cumOffset += pfText->getLength();
	}
	return UT_FALSE;
}

PT_DocPosition pt_PieceTable::getStruxPosition(PL_StruxDocHandle sdh) const
{
	// return absolute document position of the given handle.

	pf_Frag * pfToFind = (pf_Frag *)sdh;

	return getFragPosition(pfToFind);
}

PT_DocPosition pt_PieceTable::getFragPosition(const pf_Frag * pfToFind) const
{
	PT_DocPosition sum = 0;

	for (pf_Frag * pf = m_fragments.getFirst(); (pf); pf=pf->getNext())
	{
		if (pf == pfToFind)
			return sum;
		if (pf->getType() == pf_Frag::PFT_Text)
		{
			pf_Frag_Text * pfText = static_cast<pf_Frag_Text *>(pf);
			sum += pfText->getLength();
		}
	}
	UT_ASSERT(0);
	return 0;
}

UT_Bool pt_PieceTable::getStruxFromPosition(PL_ListenerId listenerId,
											PT_DocPosition docPos,
											PL_StruxFmtHandle * psfh) const
{
	// return the SFH of the last strux immediately prior to
	// the given absolute document position.

	pf_Frag_Strux * pfs = NULL;
	if (!_getStruxFromPosition(docPos,&pfs))
		return UT_FALSE;
	
	*psfh = pfs->getFmtHandle(listenerId);
	return UT_TRUE;
}

UT_Bool pt_PieceTable::_getStruxFromPosition(PT_DocPosition docPos,
											 pf_Frag_Strux ** ppfs) const
{
	// return the strux fragment immediately prior (containing)
	// the given absolute document position.
	
	PT_DocPosition sum = 0;
	pf_Frag * pfLastStrux = NULL;

	for (pf_Frag * pf = m_fragments.getFirst(); (pf); pf=pf->getNext())
	{
		if (pf->getType() == pf_Frag::PFT_Text)
		{
			pf_Frag_Text * pfText = static_cast<pf_Frag_Text *>(pf);
			sum += pfText->getLength();

			if (sum >= docPos)
				goto FoundIt;
		}
		else if (pf->getType() == pf_Frag::PFT_Strux)
		{
			pfLastStrux = pf;
		}
	}

	// if we fall out of the loop, we didn't have a text node
	// at or around the document position requested.
	// return the last strux in the document.

 FoundIt:

	pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *> (pfLastStrux);
	*ppfs = pfs;
	return UT_TRUE;
}

UT_Bool pt_PieceTable::getTextFragFromPosition(PT_DocPosition docPos,
											   UT_Bool bLeftSide,
											   pf_Frag_Strux ** ppfs,
											   pf_Frag_Text ** ppft,
											   PT_BlockOffset * pOffset) const
{
	UT_ASSERT(ppft);
	UT_ASSERT(pOffset);
	
	// return the text fragment containing the given position.
	// return the strux containing the text fragment we find.
	// return the offset from the start of the fragment.
	// if the position is between 2 fragments, return the
	// one w/r/t bLeftSide.
	
	PT_DocPosition sum = 0;
	pf_Frag_Strux * pfLastStrux = NULL;
	
	for (pf_Frag * pf = m_fragments.getFirst(); (pf); pf=pf->getNext())
	{
		if (pf->getType() == pf_Frag::PFT_Strux)
			pfLastStrux = static_cast<pf_Frag_Strux *>(pf);
		
		if (pf->getType() != pf_Frag::PFT_Text)
			continue;

		pf_Frag_Text * pfText = static_cast<pf_Frag_Text *>(pf);
		if (docPos == sum)				// if on left-edge of this fragment
		{
			*pOffset = 0;
			*ppft = pfText;
			*ppfs = pfLastStrux;
			return UT_TRUE;
		}
		
		UT_uint32 len = pfText->getLength();

		if (docPos < sum+len)			// if position inside this fragment
		{
			*pOffset = (docPos - sum);
			*ppft = pfText;
			*ppfs = pfLastStrux;
			return UT_TRUE;
		}

		sum += len;
		if ((docPos == sum) && bLeftSide)	// if on right-edge of this fragment 
		{									// (aka the left side of this position).
			*pOffset = len;
			*ppft = pfText;
			*ppfs = pfLastStrux;
			return UT_TRUE;
		}
	}

	// if we fall out of the loop, we didn't have a text node
	// at or around the document position requested.
	// TODO this looks like it should be an error, for now we bail
	// TODO and see if it ever goes off.  later we can just return
	// TODO the last node in the list.
	
	UT_ASSERT(0);
	return UT_FALSE;
}

void pt_PieceTable::beginUserAtomicGlob(void)
{
	// a 'user-atomic-glob' is a change record marker used to indicate
	// the start of a set of edits which the user will probably consider
	// an atomic operation.  we leave it upto the view/layout/formatter
	// to decide what that is.  this is used to glob events for the
	// purposes of UNDO/REDO -- this might be word globbing of a contiguous
	// sequence of keystokes or bracket a global search/replace.
	//
	// we do not notify the listeners.
	
	PX_ChangeRecord * pcr = new PX_ChangeRecord(PX_ChangeRecord::PXT_UserAtomicGlobMarker,
												PX_ChangeRecord::PXF_UserAtomicStart,
												0,0,0);
	UT_ASSERT(pcr);
	m_history.addChangeRecord(pcr);
}

void pt_PieceTable::endUserAtomicGlob(void)
{
	PX_ChangeRecord * pcr = new PX_ChangeRecord(PX_ChangeRecord::PXT_UserAtomicGlobMarker,
												PX_ChangeRecord::PXF_UserAtomicEnd,
												0,0,0);
	UT_ASSERT(pcr);
	m_history.addChangeRecord(pcr);
}

UT_Bool pt_PieceTable::_doTheDo(const PX_ChangeRecord * pcr)
{
	// actually do the work of the undo or redo.

	switch (pcr->getType())
	{
	case PX_ChangeRecord::PXT_UserAtomicGlobMarker:
		return UT_TRUE;
		
	case PX_ChangeRecord::PXT_InsertSpan:
		{
			const PX_ChangeRecord_Span * pcrSpan = static_cast<const PX_ChangeRecord_Span *>(pcr);
			pf_Frag_Strux * pfs = NULL;
			pf_Frag_Text * pft = NULL;
			PT_BlockOffset fragOffset = 0;
			if (!getTextFragFromPosition(pcrSpan->getPosition(),pcrSpan->isLeftSide(),&pfs,&pft,&fragOffset))
				return UT_FALSE;
			UT_ASSERT(pft->getIndexAP() == pcrSpan->getIndexAP());
			if (!_insertSpan(pft,pcrSpan->getBufIndex(),pcrSpan->isLeftSide(),fragOffset,pcrSpan->getLength()))
				return UT_FALSE;
			m_pDocument->notifyListeners(pfs,pcr);
		}
		return UT_TRUE;
		
	case PX_ChangeRecord::PXT_DeleteSpan:
		{
			// Our deleteSpan is much simpler than the main routine.
			// We can do this becase the change history is composed
			// of atomic operations, whereas the main routine has to
			// to deal with whatever the user chose to do (and cut
			// it into a series of steps).

			const PX_ChangeRecord_Span * pcrSpan = static_cast<const PX_ChangeRecord_Span *>(pcr);
			pf_Frag_Strux * pfs = NULL;
			pf_Frag_Text * pft = NULL;
			PT_BlockOffset fragOffset = 0;
			if (!getTextFragFromPosition(pcrSpan->getPosition(),UT_FALSE,&pfs,&pft,&fragOffset))
				return UT_FALSE;
			UT_ASSERT(pft->getIndexAP() == pcrSpan->getIndexAP());
			_deleteSpan(pft,fragOffset,pcrSpan->getBufIndex(),pcrSpan->getLength(),NULL,NULL);
			m_pDocument->notifyListeners(pfs,pcr);
		}
		return UT_TRUE;

	case PX_ChangeRecord::PXT_ChangeSpan:
		{
			// ChangeSpan is it's own inverse.  similarly, we have a much simpler
			// job than the main routine, because we have broken up the user's
			// request into atomic operations.

			const PX_ChangeRecord_SpanChange * pcrs = static_cast<const PX_ChangeRecord_SpanChange *>(pcr);
			pf_Frag_Strux * pfs = NULL;
			pf_Frag_Text * pft = NULL;
			PT_BlockOffset fragOffset = 0;
			if (!getTextFragFromPosition(pcrs->getPosition(),UT_FALSE,&pfs,&pft,&fragOffset))
				return UT_FALSE;
			_fmtChange(pft,fragOffset,pcrs->getLength(),pcrs->getIndexAP(),NULL,NULL);
			m_pDocument->notifyListeners(pfs,pcr);
		}
		return UT_TRUE;
			
	case PX_ChangeRecord::PXT_InsertStrux:
		{
			const PX_ChangeRecord_Strux * pcrStrux = static_cast<const PX_ChangeRecord_Strux *>(pcr);
			pf_Frag_Strux * pfsNew = NULL;
			if (!_createStrux(pcrStrux->getStruxType(),pcrStrux->getIndexAP(),&pfsNew))
				return UT_FALSE;
			pf_Frag_Strux * pfsPrev = NULL;
			pf_Frag_Text * pft = NULL;
			PT_BlockOffset fragOffset = 0;
			if (!getTextFragFromPosition(pcrStrux->getPosition(),pcrStrux->isLeftSide(),&pfsPrev,&pft,&fragOffset))
				return UT_FALSE;
			_insertStrux(pfsPrev,pft,fragOffset,pcrStrux->isLeftSide(),pfsNew);
			m_pDocument->notifyListeners(pfsPrev,pfsNew,pcr);
		}
		return UT_TRUE;
		
	case PX_ChangeRecord::PXT_DeleteStrux:
		{
			const PX_ChangeRecord_Strux * pcrStrux = static_cast<const PX_ChangeRecord_Strux *>(pcr);
			switch (pcrStrux->getStruxType())
			{
			case PTX_Block:
				{
					pf_Frag_Strux * pfs = NULL;
					pf_Frag_Text * pft = NULL;
					PT_BlockOffset fragOffset = 0;
					UT_Bool bFoundIt = getTextFragFromPosition(pcrStrux->getPosition(),pcrStrux->isLeftSide(),
															   &pfs,&pft,&fragOffset);
					UT_ASSERT(bFoundIt);
					// TODO because strux aren't directly addressible because
					// TODO they don't take up a doc position, we're not sure
					// TODO if we're looking at the strux to delete or just
					// TODO left of it.
					if (fragOffset != 0)
					{
						pf_Frag * pfNext = pft->getNext();
						UT_ASSERT(pfNext->getType() == pf_Frag::PFT_Strux);
						pfs = static_cast<pf_Frag_Strux *> (pfNext);
						pft = NULL;
						fragOffset = 0;
					}
					UT_Bool bResult = _unlinkStrux_Block(pfs,NULL,NULL);
					UT_ASSERT(bResult);
					m_pDocument->notifyListeners(pfs,pcr);
					delete pfs;
				}
				break;
				
			default:
				UT_ASSERT(0);
				return UT_FALSE;
			}
		}
		return UT_TRUE;
		
	default:
		UT_ASSERT(0);
		return UT_FALSE;
	}
}

UT_Bool pt_PieceTable::undoCmd(void)
{
	// do a user-atomic undo.
	// return false if we can't.
	
	PX_ChangeRecord * pcr;
	if (!m_history.getUndo(&pcr))
		return UT_FALSE;
	UT_ASSERT(pcr);

	// the flags on the first undo record tells us whether it is
	// a simple change, a multi-step change (display atomic) or
	// a user-atomic (via globbing).
	// for a simple change, we just do it and return.
	// for a multi-step or user-atomic we loop until we do the
	// corresponding other end.
	
	UT_Byte flagsFirst = pcr->getFlags();
	while (m_history.getUndo(&pcr))
	{
		PX_ChangeRecord * pcrRev = pcr->reverse(); // we must delete this.
		UT_ASSERT(pcrRev);
		UT_Byte flagsRev = pcrRev->getFlags();
		UT_Bool bResult = _doTheDo(pcrRev);
		delete pcrRev;
		if (!bResult)
			return UT_FALSE;
		m_history.didUndo();
		if (flagsRev == flagsFirst)		// stop when we have a matching end
			break;
	}

	return UT_TRUE;
}

UT_Bool pt_PieceTable::redoCmd(void)
{
	// do a user-atomic redo.
	// return false if we can't.
	
	PX_ChangeRecord * pcr;
	if (!m_history.getRedo(&pcr))
		return UT_FALSE;
	UT_ASSERT(pcr);

	// the flags on the first redo record tells us whether it is
	// a simple change, a multi-step change (display atomic) or
	// a user-atomic (via globbing).
	// for a simple change, we just do it and return.
	// for a multi-step or user-atomic we loop until we do the
	// corresponding other end.
	
	UT_Byte flagsRevFirst = pcr->getRevFlags();
	while (m_history.getRedo(&pcr))
	{
		if (!_doTheDo(pcr))
			return UT_FALSE;
		m_history.didRedo();
		if (flagsRevFirst == pcr->getFlags())		// stop when we have a matching end
			break;
	}

	return UT_TRUE;
}

void pt_PieceTable::dump(FILE * fp) const
{
	fprintf(fp,"  PieceTable: State %d\n",(int)m_pts);
	fprintf(fp,"  PieceTable: Fragments:\n");

	m_fragments.dump(fp);
}
