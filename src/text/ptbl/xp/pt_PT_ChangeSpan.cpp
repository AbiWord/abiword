
// changeSpanFmt-related fuctions for class pt_PieceTable

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

UT_Bool pt_PieceTable::_fmtChangeSpan(pf_Frag_Text * pft, UT_uint32 fragOffset, UT_uint32 length,
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
												pf_Frag_Strux * pfs,
												pf_Frag ** ppfNewEnd,
												UT_uint32 * pfragOffsetNewEnd)
{
	// create a change record for this change and put it in the history.

#if 1
	// TODO when we add length to strux (and remove the bLeftSide
	// TODO stuff) we probably won't need this.
	if (length == 0)
	{
		if (ppfNewEnd)
			*ppfNewEnd = pft->getNext();
		if (pfragOffsetNewEnd)
			*pfragOffsetNewEnd = 0;
		return UT_TRUE;
	}
#else	
	UT_ASSERT(length > 0);
#endif

	UT_ASSERT(fragOffset+length <= pft->getLength());
	
	PT_AttrPropIndex indexNewAP;
	PT_AttrPropIndex indexOldAP = pft->getIndexAP();
	UT_Bool bMerged = m_varset.mergeAP(ptc,indexOldAP,attributes,properties,&indexNewAP);
	UT_ASSERT(bMerged);

	if (indexOldAP == indexNewAP)		// the requested change will have no effect on this fragment.
	{
		if (fragOffset+length == pft->getLength())
		{
			if (ppfNewEnd)
				*ppfNewEnd = pft->getNext();
			if (pfragOffsetNewEnd)
				*pfragOffsetNewEnd = 0;
		}
		else
		{
			if (ppfNewEnd)
				*ppfNewEnd = pft;
			if (pfragOffsetNewEnd)
				*pfragOffsetNewEnd = fragOffset+length;
		}
		
		return UT_TRUE;
	}
	
	// we do this before the actual change because various fields that
	// we need may be blown away during the change.  we then notify all
	// listeners of the change.

	UT_Bool bLeftSide = UT_TRUE;		// TODO we are going to delete these.
	PX_ChangeRecord_SpanChange * pcr
		= new PX_ChangeRecord_SpanChange(PX_ChangeRecord::PXT_ChangeSpan,
										 PX_ChangeRecord::PXF_Null,
										 dpos,bLeftSide,indexOldAP,indexNewAP,ptc,
										 m_varset.getBufIndex(pft->getBufIndex(),fragOffset),
										 length);
	UT_ASSERT(pcr);
	m_history.addChangeRecord(pcr);
	UT_Bool bResult = _fmtChangeSpan(pft,fragOffset,length,indexNewAP,ppfNewEnd,pfragOffsetNewEnd);
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
	// change -- it may not actually take more than one step,
	// but it is too complicated to tell at this point, so we
	// assume it will and don't worry about it.
	//
	// we are in a simple change if the beginning and end are
	// within the same fragment.

	UT_Bool bSimple = (f.x_pft == e.x_pft);
	if (!bSimple)
		beginMultiStepGlob();

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
				// we are only applying span-level changes, so we ignore strux.
				// but we still need to update our loop indices.
				
				pf_Frag * pf = f.x_pft;
				pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *> (pf);

				if (!bSimple)
				{
					if (!f.x_pft->getNext())
						bFinished = UT_TRUE;
					else if (   (length==0)
							 && (f.x_pft->getNext()->getType() == pf_Frag::PFT_Text))
						bFinished = UT_TRUE;
				}
				dpos1 += 0;				// TODO when strux have length, change this to 1.
				pfNewEnd = pf->getNext();
				fragOffsetNewEnd = 0;
				f.x_pfs = pfs;			// everything following this is in this strux
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
				
				UT_Bool bResult = _fmtChangeSpanWithNotify(ptc,f.x_pft,f.x_fragOffset,
														   dpos1,lengthThisStep,
														   attributes,properties,
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
		// that each of the cases routines gave us.

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
