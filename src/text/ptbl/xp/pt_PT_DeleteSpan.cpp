
// deleteSpan-related routines for class pt_PieceTable

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
											 pf_Frag_Strux * pfs,
											 pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd)
{

#if 1
	// TODO when we add length to strux (and remove the bLeftSide
	// TODO stuff) we probably won't need this.
	if (length == 0)
	{
		if (ppfEnd)
			*ppfEnd = pft->getNext();
		if (pfragOffsetEnd)
			*pfragOffsetEnd = 0;
		return UT_TRUE;
	}
#else	
	UT_ASSERT(length > 0);
#endif
	
	// create a change record for this change and put it in the history.
	// we do this before the actual change because various fields that
	// we need are blown away during the delete.  we then notify all
	// listeners of the change.
		
	PX_ChangeRecord_Span * pcr
		= new PX_ChangeRecord_Span(PX_ChangeRecord::PXT_DeleteSpan,
								   PX_ChangeRecord::PXF_Null,
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
	// delete -- it may not actually take more than one step, but
	// it is too complicated to tell at this point, so we assume
	// it will and don't worry about it.
	//
	// we are in a simple change if the beginning and end are
	// within the same fragment.
	
	UT_Bool bSimple = (f.x_pft == e.x_pft);
	if (!bSimple)
		beginMultiStepGlob();

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
						bFinished = UT_TRUE;
					else if (   (length==0)
							 && (f.x_pft->getNext()->getType() == pf_Frag::PFT_Text))
						bFinished = UT_TRUE;
				}
				
				// TODO decide if we need to send f.x_bLeftSide or if it matters.
				UT_Bool bResult = _deleteStruxWithNotify(dpos,f.x_bLeftSide,pfs,
														 &pfNewEnd,&fragOffsetNewEnd);
				UT_ASSERT(bResult);
				// we do not update f.x_pfs because we just deleted pfs.
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
						bFinished = UT_TRUE;
					else if (length==0)
					{
						if (bLeftSide2)
							bFinished = UT_TRUE;
						else if (lengthInFrag > lengthThisStep)
							bFinished = UT_TRUE;
						else if (f.x_pft->getNext()->getType() == pf_Frag::PFT_Text)
							bFinished = UT_TRUE;
					}
				}
				
				UT_Bool bResult = _deleteSpanWithNotify(dpos,UT_FALSE,f.x_pft,f.x_fragOffset,
														lengthThisStep,
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
		
		if (bSimple && (length==0))
			bFinished = UT_TRUE;
	}

	if (!bSimple)
		endMultiStepGlob();
	
	return UT_TRUE;
}
