
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
