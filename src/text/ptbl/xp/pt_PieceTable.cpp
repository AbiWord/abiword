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


UT_Bool pt_PieceTable::_fmtChangeStruxWithNotify(PTChangeFmt ptc,
												 pf_Frag_Strux * pfs,
												 const XML_Char ** attributes,
												 const XML_Char ** properties,
												 pf_Frag ** ppfNewEnd,
												 UT_uint32 * pfragOffsetNewEnd)
{
	return UT_TRUE;
}


UT_Bool pt_PieceTable::_struxHasContent(pf_Frag_Strux * pfs) const
{
	// return true iff the paragraph has content (text).

	return (pfs->getNext() && (pfs->getNext()->getType() == pf_Frag::PFT_Text));
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

void pt_PieceTable::dump(FILE * fp) const
{
	fprintf(fp,"  PieceTable: State %d\n",(int)m_pts);
	fprintf(fp,"  PieceTable: Fragments:\n");

	m_fragments.dump(fp);
}
