
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
		= new PX_ChangeRecord_Span(PX_ChangeRecord::PXT_InsertSpan,UT_FALSE,UT_FALSE,
								   dpos,bLeftSide,pft->getIndexAP(),bi,length);
	if (!pcr)
		return UT_FALSE;

	// now we notify all listeners who have subscribed.

	m_history.addChangeRecord(pcr);
	m_pDocument->notifyListeners(pfs,pcr);

	return UT_TRUE;
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
			
			m_fragments.unlinkFrag(pft);
			delete pft;
			return UT_TRUE;
		}

		// the change is a proper prefix within the fragment,
		// do a left-truncate on it.

		pft->adjustOffsetLength(bi,pft->getLength()-length);
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
	if (!pftTail)
		return UT_FALSE;
			
	pft->changeLength(fragOffset);
	m_fragments.insertFrag(pft,pftTail);
	
	return UT_TRUE;

		
}

UT_Bool pt_PieceTable::deleteSpan(PT_DocPosition dpos,
								  UT_uint32 length)
{
	// remove length characters from the document at the given position.

	UT_ASSERT(m_pts==PTS_Editing);


	// TODO we can do this a bit smarter....
	
	UT_Bool bLeftSide_First = UT_FALSE;
	UT_Bool bLeftSide_Last = UT_TRUE;
	pf_Frag_Strux * pfs_First = NULL;
	pf_Frag_Strux * pfs_Last = NULL;
	pf_Frag_Text * pft_First = NULL;
	pf_Frag_Text * pft_Last = NULL;
	PT_BlockOffset fragOffset_First = 0;
	PT_BlockOffset fragOffset_Last = 0;
	if (!getTextFragFromPosition(dpos,bLeftSide_First,&pfs_First,&pft_First,&fragOffset_First))
		return UT_FALSE;
	if (!getTextFragFromPosition(dpos+length,bLeftSide_Last,&pfs_Last,&pft_Last,&fragOffset_Last))
		return UT_FALSE;

	if (pft_First != pft_Last)	// TODO for now we force it all to be in the same block.
		return UT_FALSE;

	PT_BufIndex biToDelete = m_varset.getBufIndex(pft_First->getBufIndex(),fragOffset_First);
	
	PX_ChangeRecord_Span * pcr = new PX_ChangeRecord_Span(PX_ChangeRecord::PXT_DeleteSpan,
														  UT_FALSE,UT_FALSE,dpos,UT_FALSE,
														  pft_First->getIndexAP(),
														  biToDelete,length);
	if (!pcr)
		return UT_FALSE;

	if (!_deleteSpan(pft_First,fragOffset_First,biToDelete,length))
		return UT_FALSE;

	// now we notify all listeners who have subscribed.

	m_history.addChangeRecord(pcr);
	m_pDocument->notifyListeners(pfs_First,pcr);

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
	*psfh = pfs->getFmtHandle(listenerId);
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

void pt_PieceTable::dump(FILE * fp) const
{
	fprintf(fp,"  PieceTable: State %d\n",(int)m_pts);
	fprintf(fp,"  PieceTable: Fragments:\n");

	m_fragments.dump(fp);
}
