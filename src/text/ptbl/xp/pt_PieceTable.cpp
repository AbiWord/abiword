 
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

// TODO: calculate this from pf_FRAG_STRUX_*_LENGTH instead?
#define pt_BOD_POSITION 2

/*****************************************************************/
/*****************************************************************/

pt_PieceTable::pt_PieceTable(PD_Document * pDocument)
{
	m_pts = PTS_Loading;
	m_pDocument = pDocument;

	setPieceTableState(PTS_Loading);
	loading.m_indexCurrentInlineAP = 0;

	m_bHaveTemporarySpanFmt = UT_FALSE;
	m_indexAPTemporarySpanFmt = 0;
	m_dposTemporarySpanFmt = 0;
}

pt_PieceTable::~pt_PieceTable()
{
}

void pt_PieceTable::setPieceTableState(PTState pts)
{
	UT_ASSERT(pts >= m_pts);

	if ((m_pts==PTS_Loading) && (pts==PTS_Editing))
	{
		// transition from loading to editing.
		// tack on an EOD fragment to the fragment list.
		// this allows us to safely go to the end of the document.
		pf_Frag * pfEOD = new pf_Frag(this,pf_Frag::PFT_EndOfDoc,0);
		m_fragments.appendFrag(pfEOD);
	}

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

UT_Bool pt_PieceTable::_struxHasContent(pf_Frag_Strux * pfs) const
{
	// return true iff the paragraph has content (text).

	return (pfs->getNext() && (pfs->getNext()->getType() == pf_Frag::PFT_Text));
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
	// note: offset zero refers to the strux.  the first character is at
	// note: (0 + pf->getLength()).
	
	UT_ASSERT(sdh);
	UT_ASSERT(ppAP);

	pf_Frag * pf = (pf_Frag *)sdh;
	UT_ASSERT(pf->getType() == pf_Frag::PFT_Strux);
	pf_Frag_Strux * pfsBlock = static_cast<pf_Frag_Strux *> (pf);
	UT_ASSERT(pfsBlock->getStruxType() == PTX_Block);

	UT_uint32 cumOffset = pfsBlock->getLength();
	for (pf_Frag * pfTemp=pfsBlock->getNext(); (pfTemp); pfTemp=pfTemp->getNext())
	{
		if ((offset >= cumOffset) && (offset < cumOffset+pfTemp->getLength()))
		{
			// requested offset is within this fragment.

			// if this is inside something other than a text fragment, we puke.
			
			if (pfTemp->getType() != pf_Frag::PFT_Text)
				return UT_FALSE;

			pf_Frag_Text * pfText = static_cast<pf_Frag_Text *> (pfTemp);
			const PP_AttrProp * pAP = m_varset.getAP(pfText->getIndexAP());
			if (!pAP)
				return UT_FALSE;

			*ppAP = pAP;
			return UT_TRUE;
		}

		cumOffset += pfTemp->getLength();
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
	// note: offset zero refers to the strux.  the first character is at
	// note: (0 + pf->getLength()).

	pf_Frag * pf = (pf_Frag *)sdh;
	UT_ASSERT(pf->getType() == pf_Frag::PFT_Strux);
	pf_Frag_Strux * pfsBlock = static_cast<pf_Frag_Strux *> (pf);
	UT_ASSERT(pfsBlock->getStruxType() == PTX_Block);

	UT_uint32 cumOffset = pf->getLength();
	for (pf_Frag * pfTemp=pfsBlock->getNext(); (pfTemp); pfTemp=pfTemp->getNext())
	{
		if (offset == cumOffset)
		{
			if (pfTemp->getType() != pf_Frag::PFT_Text)
				return UT_FALSE;
			
			pf_Frag_Text * pfText = static_cast<pf_Frag_Text *> (pfTemp);
			*ppSpan = getPointer(pfText->getBufIndex());
			*pLength = pfText->getLength();
			return UT_TRUE;
		}
		if (offset < cumOffset+pfTemp->getLength())
		{
			if (pfTemp->getType() != pf_Frag::PFT_Text)
				return UT_FALSE;

			pf_Frag_Text * pfText = static_cast<pf_Frag_Text *> (pfTemp);
			const UT_UCSChar * p = getPointer(pfText->getBufIndex());
			UT_uint32 delta = offset - cumOffset;
			*ppSpan = p + delta;
			*pLength = pfText->getLength() - delta;
			return UT_TRUE;
		}

		cumOffset += pfTemp->getLength();
	}
	return UT_FALSE;
}

UT_Bool pt_PieceTable::getBlockBuf(PL_StruxDocHandle sdh, UT_GrowBuf * pgb) const
{
	UT_ASSERT(UT_TODO);
	return UT_TRUE;
}

UT_Bool pt_PieceTable::getBounds(UT_Bool bEnd, PT_DocPosition & docPos) const
{
	// be optimistic
	UT_Bool res = UT_TRUE;

	if (!bEnd)
	{
		docPos = pt_BOD_POSITION;
	}
	else
	{
		// NOTE: this gets called for every cursor motion
		// TODO: be more efficient & cache the doc length
		PT_DocPosition sum = 0;
		pf_Frag * pfLast = NULL;

		for (pf_Frag * pf = m_fragments.getFirst(); (pf); pf=pf->getNext())
		{
			sum += pf->getLength();
			pfLast = pf;
		}

		UT_ASSERT(pfLast && (pfLast->getType() == pf_Frag::PFT_EndOfDoc));
		docPos = sum;
	}

	return res;
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

		sum += pf->getLength();
	}

	UT_ASSERT(0);
	return 0;
}

UT_Bool pt_PieceTable::getFragFromPosition(PT_DocPosition docPos,
										   pf_Frag ** ppf,
										   PT_BlockOffset * pFragOffset) const
{
	// return the frag at the given doc position.

	PT_DocPosition sum = 0;
	pf_Frag * pfLast = NULL;
	
	for (pf_Frag * pf = m_fragments.getFirst(); (pf); pf=pf->getNext())
	{
		if ((docPos >= sum) && (docPos < sum+pf->getLength()))
		{
			*ppf = pf;
			if (pFragOffset)
				*pFragOffset = docPos - sum;
			return UT_TRUE;
		}

		sum += pf->getLength();
		pfLast = pf;
	}

	// if we fall out of the loop, we didn't have a node
	// at or around the document position requested.
	// since we now have an EOD fragment, we should not
	// ever see this -- unless the caller sends a bogus
	// doc position.

	UT_ASSERT(pfLast);
	UT_ASSERT(pfLast->getType() == pf_Frag::PFT_EndOfDoc);

	// TODO if (docPos > sum) we should probably complain...
	
	*ppf = pfLast;
	if (pFragOffset)
		*pFragOffset = docPos - sum;

	return UT_TRUE;
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

UT_Bool pt_PieceTable::getStruxOfTypeFromPosition(PL_ListenerId listenerId,
												  PT_DocPosition docPos,
												  PTStruxType pts,
												  PL_StruxFmtHandle * psfh) const
{
	// return the SFH of the last strux of the given type
	// immediately prior to the given absolute document position.

	pf_Frag_Strux * pfs = NULL;
	if (!_getStruxOfTypeFromPosition(docPos,pts,&pfs))
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
		if (pf->getType() == pf_Frag::PFT_Strux)
			pfLastStrux = pf;

		sum += pf->getLength();

		if (sum >= docPos)
			goto FoundIt;
	}

	// if we fall out of the loop, we didn't have a text node
	// at or around the document position requested.
	// return the last strux in the document.

 FoundIt:

	pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *> (pfLastStrux);
	*ppfs = pfs;
	return UT_TRUE;
}

UT_Bool pt_PieceTable::_getStruxOfTypeFromPosition(PT_DocPosition dpos,
												   PTStruxType pts,
												   pf_Frag_Strux ** ppfs) const
{
	// return the strux fragment of the given type containing
	// the given absolute document position.

	UT_ASSERT(ppfs);
	*ppfs = NULL;
	
	pf_Frag_Strux * pfs = NULL;
	if (!_getStruxFromPosition(dpos,&pfs))
		return UT_FALSE;

	if (pfs->getStruxType() == pts)		// is it of the type we want
	{
		*ppfs = pfs;
		return UT_TRUE;
	}

	// if not, we walk backwards thru the list and try to find it.

	for (pf_Frag * pf=pfs; (pf); pf=pf->getPrev())
		if (pf->getType() == pf_Frag::PFT_Strux)
		{
			pf_Frag_Strux * pfsTemp = static_cast<pf_Frag_Strux *>(pf);
			if (pfsTemp->getStruxType() == pts)	// did we find it
			{
				*ppfs = pfs;
				return UT_TRUE;
			}
		}

	// did not find it.
	
	return UT_FALSE;
}

void pt_PieceTable::dump(FILE * fp) const
{
	fprintf(fp,"  PieceTable: State %d\n",(int)m_pts);
	fprintf(fp,"  PieceTable: Fragments:\n");

	m_fragments.dump(fp);
}
