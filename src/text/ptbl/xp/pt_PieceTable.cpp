
#include "ut_types.h"
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
		= new PX_ChangeRecord_Span(PX_ChangeRecord::PXT_InsertSpan,0,
								   dpos,bLeftSide,pft->getIndexAP(),bi,length);
	UT_ASSERT(pcr);
	m_history.addChangeRecord(pcr);
	m_pDocument->notifyListeners(pfs,pcr);

	return UT_TRUE;
}

void pt_PieceTable::_deleteTextFrag(pf_Frag_Text * pft)
{
	// delete the given fragment from the fragment list.
	// also, see if the adjacent fragments can be coalesced.

	pf_Frag * pp = pft->getPrev();

	m_fragments.unlinkFrag(pft);
	delete pft;

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
			ppt->changeLength(prevLength+pnt->getLength());
			m_fragments.unlinkFrag(pnt);
			delete pnt;
		}
	}
}

UT_Bool pt_PieceTable::_deleteSpan(pf_Frag_Text * pft, UT_uint32 fragOffset,
								   PT_BufIndex bi, UT_uint32 length)
{
	// perform simple delete of a span of text.
	// we assume that it is completely contained within this fragment.

	UT_ASSERT(fragOffset+length <= pft->getLength());

	if (fragOffset == 0)
	{
		// the change is at the beginning of the fragment,

		if (length == pft->getLength())
		{
			// the change exactly matches the fragment, just delete the fragment.
			// as we delete it, see if the fragments around it can be coalesced.

			_deleteTextFrag(pft);
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
	
	return UT_TRUE;
}

UT_Bool pt_PieceTable::deleteSpan(PT_DocPosition dpos,
								  UT_uint32 length)
{
	// remove length characters from the document at the given position.

	UT_ASSERT(m_pts==PTS_Editing);

	struct _x
	{
		UT_Bool				x_bLeftSide;
		pf_Frag_Strux *		x_pfs;
		pf_Frag_Text *		x_pft;
		PT_BlockOffset		x_fragOffset;
		PT_BufIndex			x_bi;
		pf_Frag *			x_pfPrev;
		UT_Bool				x_prevIsText;
		UT_uint32			x_prevLength;
	};

	struct _x f = { UT_FALSE, NULL, NULL, 0, 0, NULL, UT_FALSE, 0 };

	if (!getTextFragFromPosition(dpos,f.x_bLeftSide,&f.x_pfs,&f.x_pft,&f.x_fragOffset))
	{
		// could not find a text fragment containing the given
		// absolute document position ???
		return UT_FALSE;
	}
	f.x_bi = m_varset.getBufIndex(f.x_pft->getBufIndex(),f.x_fragOffset);
	f.x_pfPrev = f.x_pft->getPrev();
	f.x_prevIsText = (f.x_pfPrev->getType()==pf_Frag::PFT_Text);
	if (f.x_prevIsText)
		f.x_prevLength = (static_cast<pf_Frag_Text *>(f.x_pfPrev))->getLength();
	
	// see if the amount of text to be deleted is completely
	// contained withing the fragment found.  if so, we have
	// a simple delete.  otherwise, we need to set up a multi-step
	// delete.

	PX_ChangeRecord::PXFlags fMultiStepStart = PX_ChangeRecord::PXF_Null;
	PX_ChangeRecord::PXFlags fMultiStepEnd = PX_ChangeRecord::PXF_Null;
	UT_Bool bInMultiStep = UT_FALSE;

	if (f.x_fragOffset+length > f.x_pft->getLength())
	{
		fMultiStepStart = PX_ChangeRecord::PXF_MultiStepStart;
		bInMultiStep = UT_TRUE;
	}

	// loop to delete the amount requested, one text fragment at a time.
	// if we encounter any non-text fragments along the way, we delete
	// them too.  that is, we implicitly delete paragraphs here.
	
	while (length)
	{
		UT_ASSERT(f.x_pft->getType()==pf_Frag::PFT_Text);

		// figure out how much to consume during this iteration.
		// set the multi-step end flag, if this will be the last iteration.
		
		UT_uint32 lengthThisStep = f.x_pft->getLength() - f.x_fragOffset;
		if (length <= lengthThisStep)
		{
			lengthThisStep = length;
			if (bInMultiStep)
				fMultiStepEnd = PX_ChangeRecord::PXF_MultiStepEnd;
		}
		
		// create a change record for this change and put it in the history.
		// we do this before the actual change because various fields that
		// we need are blown away during the delete.  we then notify all
		// listeners of the change.
		
		PX_ChangeRecord_Span * pcr
			= new PX_ChangeRecord_Span(PX_ChangeRecord::PXT_DeleteSpan,
									   fMultiStepStart | fMultiStepEnd,
									   dpos,UT_FALSE,
									   f.x_pft->getIndexAP(),
									   f.x_bi,lengthThisStep);
		UT_ASSERT(pcr);
		m_history.addChangeRecord(pcr);
		_deleteSpan(f.x_pft,f.x_fragOffset,f.x_bi,lengthThisStep);
		m_pDocument->notifyListeners(f.x_pfs,pcr);

		// we decrement length by the amount that we processed
		// in this iteration of the loop.  if there is more to
		// do, then we need to jump thru some hoops....
		
		length -= lengthThisStep;
		if (length)
		{
			fMultiStepStart = PX_ChangeRecord::PXF_Null;

			// since _deleteSpan(), can delete f.x_pft, messes with the
			// fragment list, and does some aggressive coalescing of
			// fragments, we cannot just do a f.x_pft->getNext() here.
			// instead, we look at the previous fragment and look
			// forward from it and try to figure out what happened to
			// the list.

			if (f.x_pfPrev->getNext() == f.x_pft)
			{
				// the current fragment was not unlinked.  we must have
				// just truncated it.  use this fragment as the 'prev'
				// in the next iteration and advance pft.
				UT_ASSERT(f.x_fragOffset > 0);

				f.x_pfPrev = f.x_pft;
				f.x_prevIsText = UT_TRUE;
				f.x_prevLength = f.x_pft->getLength();
				// we fall thru for the advance.
			}
			else
			{
				// our pft was deleted (and unlinked from the fragment list).
				// if in deleting our pft, the prev and next we coalesced,
				// we want to backup -- making our prev the current pft
				// (with an offset) for the next iteration.
				// if our pft was deleted, but there was no coalescing around
				// us, we keep 'prev' the same for the next iteration and
				// advance pft.
				
				if (f.x_prevIsText)
				{
					pf_Frag_Text * pftPrev = static_cast<pf_Frag_Text *>(f.x_pfPrev);
					if (f.x_prevLength != pftPrev->getLength())
					{
						// our pft was deleted and our previous was coalesced
						// with our next, so we backup and start in prev with
						// an offset.
						
						UT_ASSERT(f.x_fragOffset==0); // could not have coalesced prev if we were in middle of ours

						f.x_pft = pftPrev;
						f.x_fragOffset = f.x_prevLength;
						f.x_bi = m_varset.getBufIndex(f.x_pft->getBufIndex(),f.x_fragOffset);
						f.x_pfPrev = f.x_pft->getPrev();
						f.x_prevIsText = (f.x_pfPrev->getType()==pf_Frag::PFT_Text);
						if (f.x_prevIsText)
							f.x_prevLength = (static_cast<pf_Frag_Text *>(f.x_pfPrev))->getLength();
						goto NextIteration;				// a 'continue'
					}
				}

				// our pft was deleted, but no coalescing was done.
				// keep our prev the same and advance pft.
				// we fall thru for the advance.
			}
			
			pf_Frag * pfNext = f.x_pfPrev->getNext();
			UT_ASSERT(pfNext);			// delete beyond the end of the document
			switch (pfNext->getType())
			{
			case pf_Frag::PFT_Text:
				f.x_pft = static_cast<pf_Frag_Text *>(pfNext);
				f.x_fragOffset = 0;
				f.x_bi = m_varset.getBufIndex(f.x_pft->getBufIndex(),f.x_fragOffset);
				// we leave all of the prev-related vars unchanged.
				goto NextIteration;

			default:
				UT_ASSERT(0);			// TODO
			}
		}
	NextIteration:
		;
	} // end while (length)

	return UT_TRUE;
}

#if 0
UT_Bool pt_PieceTable::insertFmt(PT_DocPosition dpos1,
								 PT_DocPosition dpos2,
								 const XML_Char ** attributes,
								 const XML_Char ** properties)
{
	return UT_TRUE;
}

UT_Bool pt_PieceTable::deleteFmt(PT_DocPosition dpos1,
								 PT_DocPosition dpos2,
								 const XML_Char ** attributes,
								 const XML_Char ** properties)
{
	return UT_TRUE;
}

UT_Bool pt_PieceTable::insertStrux(PT_DocPosition dpos,
								   PTStruxType pts,
								   const XML_Char ** attributes,
								   const XML_Char ** properties)
{
	return UT_TRUE;
}

UT_Bool pt_PieceTable::deleteStrux(PT_DocPosition dpos)
{
	return UT_TRUE;
}
#endif

UT_Bool pt_PieceTable::appendStrux(PTStruxType pts, const XML_Char ** attributes)
{
	// can only be used while loading the document
	UT_ASSERT(m_pts==PTS_Loading);

	// create a new structure fragment at the current end of the document.
	
	PT_AttrPropIndex indexAP;
	if (!m_varset.storeAP(attributes,&indexAP))
		return UT_FALSE;

	pf_Frag_Strux * pfs = 0;
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
	}
	if (!pfs)
	{
		UT_DEBUGMSG(("Could not create structure fragment.\n"));
		// we forget about the AP that we created
		return UT_FALSE;
	}

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
			_deleteSpan(pft,fragOffset,pcrSpan->getBufIndex(),pcrSpan->getLength());
			m_pDocument->notifyListeners(pfs,pcr);
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
