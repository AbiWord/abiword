/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001,2002 Tomas Frydrych
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
#include "pp_Revision.h"


#define SETP(p,v)	do { if (p) (*(p)) = (v); } while (0)

/****************************************************************/
/****************************************************************/
bool pt_PieceTable::deleteSpan(PT_DocPosition dpos1,
							   PT_DocPosition dpos2,
							   PP_AttrProp *p_AttrProp_Before, 
							   bool bDontGlob)
{
	return deleteSpan(dpos1,
					  dpos2,
					  p_AttrProp_Before, 
					  true,
					  bDontGlob);
}

/****************************************************************/
bool pt_PieceTable::deleteSpanWithTable(PT_DocPosition dpos1,
							   PT_DocPosition dpos2,
							   PP_AttrProp *p_AttrProp_Before, 
							   bool bDeleteTableStruxes)
{
	return deleteSpan(dpos1,
					  dpos2,
					  p_AttrProp_Before, 
					  bDeleteTableStruxes,
					  false);
}

/****************************************************************/
bool pt_PieceTable::deleteSpan(PT_DocPosition dpos1,
							   PT_DocPosition dpos2,
							   PP_AttrProp *p_AttrProp_Before, 
							   bool bDeleteTableStruxes,
							   bool bDontGlob)
{
	if(m_pDocument->isMarkRevisions())
	{
		const XML_Char name[] = "revision";
		const XML_Char * pRevision = NULL;

		// first retrive the starting and ending fragments
		pf_Frag * pf1, * pf2;
		PT_BlockOffset Offset1, Offset2;

		if(!getFragsFromPositions(dpos1,dpos2, &pf1, &Offset1, &pf2, &Offset2))
			return false;

		// now we have to traverse the fragments and change their
		// formatting

		pf_Frag * pTemp;
		pf_Frag * pEnd = pf2->getNext();
		pf_Frag * pNext;


		for(pTemp = pf1; pTemp != pEnd; pTemp = pNext)
		{
			// we cannot ask for the next in the for statement,
			// because we might have deleted that fragment by then
			pNext = pTemp->getNext();

			// get attributes for this fragement
			const PP_AttrProp * pAP;
			pf_Frag::PFType eType = pTemp->getType();
			UT_uint32 iLen;
			PTStruxType eStruxType;

			if(eType == pf_Frag::PFT_Text)
			{
				if(!getAttrProp(((pf_Frag_Text*)pTemp)->getIndexAP(),&pAP))
					return false;
			}
			else if(eType == pf_Frag::PFT_Strux)
			{
				if(!getAttrProp(((pf_Frag_Strux*)pTemp)->getIndexAP(),&pAP))
					return false;

				eStruxType = ((pf_Frag_Strux*)pTemp)->getStruxType();

				switch (eStruxType)
				{
					case PTX_Block:
						iLen = pf_FRAG_STRUX_BLOCK_LENGTH;
						break;

					case PTX_Section:
					case PTX_SectionHdrFtr:
					case PTX_SectionEndnote:
					case PTX_SectionTable:
					case PTX_SectionCell:
					case PTX_EndCell:
					case PTX_EndTable:
						iLen = pf_FRAG_STRUX_SECTION_LENGTH;
						break;

					default:
						UT_ASSERT(UT_NOT_IMPLEMENTED);
						iLen = 1;
						break;
				}

			}
			else if(eType == pf_Frag::PFT_Object)
			{
				if(!getAttrProp(((pf_Frag_Object*)pTemp)->getIndexAP(),&pAP))
					return false;
			}
			else
			{
				// something that does not carry AP
				continue;
			}

			if(!pAP->getAttribute(name, pRevision))
				pRevision = NULL;

			PP_RevisionAttr Revisions(pRevision);

			// now we need to see if revision with this id is already
			// present, and if it is, whether it might not be addition
			UT_uint32 iId = m_pDocument->getRevisionId();
			const PP_Revision * pRev = Revisions.getGreatestLesserOrEqualRevision(iId);

			PT_DocPosition dposEnd = UT_MIN(dpos2,dpos1 + pTemp->getLength());

			if(pRev && iId == pRev->getId())
			{
				// OK, we already have a revision with this id here,
				// which means that the editor made a change earlier
				// (insertion or format change) but now wants this deleted
				//
				// so if the previous revision is an addition, we just
				// remove this fragment as if this was regular delete
				if(   (pRev->getType() == PP_REVISION_ADDITION)
				   || (pRev->getType() == PP_REVISION_ADDITION_AND_FMT ))
				{
					if(!_realDeleteSpan(dpos1, dposEnd, p_AttrProp_Before,bDeleteTableStruxes,
										bDontGlob))
						return false;

					dpos1 = dposEnd;
					continue;
				}
			}

			Revisions.addRevision(iId,PP_REVISION_DELETION,NULL,NULL);
			const XML_Char * ppRevAttrib[3];
			ppRevAttrib[0] = name;
			ppRevAttrib[1] = Revisions.getXMLstring();
			ppRevAttrib[2] = NULL;

			switch (eType)
			{
				case pf_Frag::PFT_Text:
					if(! _realChangeSpanFmt(PTC_AddFmt, dpos1, dposEnd, ppRevAttrib,NULL))
						return false;
					break;

				case pf_Frag::PFT_Strux:
					if(! _realChangeStruxFmt(PTC_AddFmt, dpos1 + iLen, dpos1 + 2*iLen, ppRevAttrib,NULL,eStruxType))
						return false;
					break;
#if 0
				case pf_Frag::PFT_Object:
					if(! _realChangeStruxFmt(PTC_AddFmt, dpos1, dposEnd, ppRevAttrib,NULL))
						return false;
					break;
#endif
				default:;
			}

			dpos1 = dposEnd;
		}

		return true;
	}
	else
		return _realDeleteSpan(dpos1, dpos2, p_AttrProp_Before, bDeleteTableStruxes,
							   bDontGlob);
}



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
										  pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd,
										  bool bAddChangeRec)
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

	if (!bAddChangeRec || _canCoalesceDeleteSpan(pcr))
	{
		if (bAddChangeRec)
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
#if 0
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
#endif
	case PTX_SectionEndnote:
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

	case PTX_SectionTable:
	case PTX_SectionCell:
	case PTX_EndTable:
	case PTX_EndCell:
//
// We've set things up so that deleting table struxes is done very deliberately. Don't
// mess with the end points here
//
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
				//  Check that there is something between the pf_Before and pf_Last, otherwise
                //  This leads to a segfault from continually pushing pf_Before onto the stack
                //  if we delete a whole lot of blank lines. These get popped off then deleted
                //  only to find the same pointer waiting to come off the stack.
				pf_Frag * pScan = pf_Before->getNext();
				while(pScan && pScan != pf_Last && (pScan->getType() != pf_Frag::PFT_Strux))
				{
					pScan = pScan->getNext();
				}
				if(pScan == pf_Last)
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

					if ((pfs_BeforeSection == pfs_LastSection) && (dpos2 > dpos1 +1))
					{
						dpos2 -= pf_Last->getLength();
						pstDelayStruxDelete->push(pf_Before);
						return true;
					}
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
	while (1)
	{
		PT_DocPosition old_dpos1 = dpos1;
		PT_DocPosition old_dpos2 = dpos2;
		UT_uint32 old_iStackSize = pstDelayStruxDelete->getDepth();

		if(!_tweakDeleteSpanOnce(dpos1, dpos2, pstDelayStruxDelete))
			return false;

		if (dpos1 == old_dpos1 && dpos2 == old_dpos2
			&& pstDelayStruxDelete->getDepth() == old_iStackSize)
			return true;
	}
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
		else if(pfTemp->getType() == pf_Frag::PFT_Strux)
		{
			pf_Frag_Strux * pfFragStrux = static_cast<pf_Frag_Strux *>(pfTemp);
			if(pfFragStrux->getStruxType() == PTX_Section)
			{
				pf_Frag_Strux_Section * pfSec = static_cast<pf_Frag_Strux_Section *>(pfFragStrux);
				_deleteHdrFtrsFromSectionStruxIfPresent(pfSec);
			}
			dposTemp += pfTemp->getLength() - fragOffsetTemp;
			pfTemp = pfTemp->getNext();
			fragOffsetTemp = 0;
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

bool pt_PieceTable::_StruxIsNotTable(pf_Frag_Strux * pfs)
{
	PTStruxType its = pfs->getStruxType();
	bool b = ((its != PTX_SectionTable) && (its != PTX_SectionCell)
			  && (its != PTX_EndTable) && (its != PTX_EndCell));
	return b;
}

bool pt_PieceTable::_deleteComplexSpan(PT_DocPosition dpos1,
									   PT_DocPosition dpos2,
									   UT_Stack * stDelayStruxDelete)
{
	pf_Frag * pfNewEnd;
	UT_uint32 fragOffsetNewEnd;
	bool bPrevWasCell = false;
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
//
// OK this code is leave the cell/table sctructures in place unless we 
// defiantely want to delete
// them.
// 
			pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *> (pf_First);
			bool bResult = true;
			if(_StruxIsNotTable(pfs))
			{
				if(bPrevWasCell && pfs->getStruxType()== PTX_Block)
				{
					bPrevWasCell = false;
					pfNewEnd = pfs->getNext();
					fragOffsetNewEnd = 0;
					pfsContainer = pfs;
					dpos1 = dpos1 +  lengthInFrag;
					stDelayStruxDelete->push(pfs);
				}
				else
				{
					bResult = _deleteStruxWithNotify(dpos1,pfs,
												  &pfNewEnd,&fragOffsetNewEnd);
					bPrevWasCell = false;
				}
			}
			else
			{
				if((pfs->getStruxType() == PTX_SectionCell) || (pfs->getStruxType() == PTX_EndTable))
				{
					bPrevWasCell = true;
				}
				else
				{
					bPrevWasCell = false;
				}
				pfNewEnd = pfs->getNext();
				fragOffsetNewEnd = 0;
				dpos1 = dpos1 +  lengthInFrag; 
				stDelayStruxDelete->push(pfs);
			}
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
		// the bookmark and hyperlink objects require special treatment; since
		// they come in pairs, we have to ensure that both are deleted together
		// so we have to find the other part of the pair, delete it, adjust the
		// positional variables and the delete the one we were originally asked
		// to delete
		case pf_Frag::PFT_Object:
		{
			bool bResult, bResult2;
			pf_Frag_Object *pO = static_cast<pf_Frag_Object *>(pf_First);
			switch(pO->getObjectType())
			{
				case PTO_Bookmark:
				{
					bool bFoundStrux2;
					PT_DocPosition posComrade;
					pf_Frag_Strux * pfsContainer2 = NULL;

					po_Bookmark * pB = pO->getBookmark();
					UT_ASSERT(pB);
					pf_Frag * pF;
					if(pB->getBookmarkType() == po_Bookmark::POBOOKMARK_END)
					{
				    	pF = pO->getPrev();
				    	while(pF)
				    	{
							if(pF->getType() == pf_Frag::PFT_Object)
							{
								pf_Frag_Object *pOb = static_cast<pf_Frag_Object*>(pF);
								po_Bookmark * pB1 = pOb->getBookmark();
								if(!strcmp(pB->getName(),pB1->getName()))
								{

									m_pDocument->removeBookmark(pB1->getName());

									posComrade = getFragPosition(pOb);
									bFoundStrux2 = _getStruxFromPosition(posComrade,&pfsContainer2);
									UT_ASSERT(bFoundStrux2);

									bResult2 =
											_deleteObjectWithNotify(posComrade,pOb,0,1,
										  							pfsContainer2,0,0);

									// now adjusting the positional variables
									if(posComrade <= dpos1)
										// delete before the start of the segement we are working on
										dpos1--;
									else
									{
										// we are inside that section
										length--;

									}
									break;
								}
							}
							pF = pF->getPrev();
				    	}
					}
					else
					{
				    	pF = pO->getNext();
				    	while(pF)
				    	{
							if(pF->getType() == pf_Frag::PFT_Object)
							{
								pf_Frag_Object *pOb = static_cast<pf_Frag_Object*>(pF);
								po_Bookmark * pB1 = pOb->getBookmark();
								if(!strcmp(pB->getName(),pB1->getName()))
								{
									m_pDocument->removeBookmark(pB1->getName());

									posComrade = getFragPosition(pOb);
									bool bFoundStrux2 = _getStruxFromPosition(posComrade,&pfsContainer2);
									UT_ASSERT(bFoundStrux2);

									bResult2 =
											_deleteObjectWithNotify(posComrade,pOb,0,1,
										  							pfsContainer2,0,0);
									if(posComrade < dpos1 + length)
										length--;
									break;
								}
							}
							pF = pF->getNext();
				    	}
					}
				bResult
					= _deleteObjectWithNotify(dpos1,pO,fragOffset_First,lengthThisStep,
									  pfsContainer,&pfNewEnd,&fragOffsetNewEnd);

				}
				break;
				// the one singnificant difference compared to the bookmarks is
				// that we have to always delete the start marker first; this is
				// so that in the case of undo the endmarker would be restored
				// first, because the mechanism that marks runs between them
				// as a hyperlink depents on the end-marker being in place before
				// the start marker
				case PTO_Hyperlink:
				{
					bool bFoundStrux2;
					PT_DocPosition posComrade;
					pf_Frag_Strux * pfsContainer2 = NULL;

					pf_Frag * pF;

		    		const PP_AttrProp * pAP = NULL;
				    pO->getPieceTable()->getAttrProp(pO->getIndexAP(),&pAP);
				    UT_ASSERT(pAP);
				    const XML_Char* pszHref = NULL;
				    const XML_Char* pszHname  = NULL;
		            UT_uint32 k = 0;
        		    bool bStart = false;
				    while((pAP)->getNthAttribute(k++,pszHname, pszHref))
				    {
		    			if(!UT_strcmp(pszHname, "xlink:href"))
				    	{
				    		bStart = true;
		    				break;
				    	}
				    }

					if(!bStart)
					{
						// in this case we are looking for the start marker
						// and so we delete it and then move on
				    	pF = pO->getPrev();
				    	while(pF)
				    	{
							if(pF->getType() == pf_Frag::PFT_Object)
							{
								pf_Frag_Object *pOb = static_cast<pf_Frag_Object*>(pF);
								if(pOb->getObjectType() == PTO_Hyperlink)
								{
									posComrade = getFragPosition(pOb);
									bFoundStrux2 = _getStruxFromPosition(posComrade,&pfsContainer2);
									UT_ASSERT(bFoundStrux2);

									bResult2 =
											_deleteObjectWithNotify(posComrade,pOb,0,1,
										  							pfsContainer2,0,0);

									// now adjusting the positional variables
									if(posComrade <= dpos1)
										// delete before the start of the segement we are working on
										dpos1--;
									else
									{
										// we are inside that section
										length--;
									}
									break;
								}
							}
							pF = pF->getPrev();
				    	}
						bResult
							= _deleteObjectWithNotify(dpos1,pO,fragOffset_First,lengthThisStep,
										  pfsContainer,&pfNewEnd,&fragOffsetNewEnd);

					}
					else
					{
						// in this case we are looking for the end marker,
						// so we have to be carefult the get rid of the start
						// marker first
				    	pF = pO->getNext();
				    	while(pF)
				    	{
							if(pF->getType() == pf_Frag::PFT_Object)
							{
								pf_Frag_Object *pOb = static_cast<pf_Frag_Object*>(pF);
								if(pOb->getObjectType() == PTO_Hyperlink)
								{
									posComrade = getFragPosition(pOb);
									bFoundStrux2 = _getStruxFromPosition(posComrade,&pfsContainer2);
									UT_ASSERT(bFoundStrux2);
					                // delete the original start marker
									bResult
										= _deleteObjectWithNotify(dpos1,pO,fragOffset_First,lengthThisStep,
													  pfsContainer,&pfNewEnd,&fragOffsetNewEnd);

									// now adjusting the positional variables
									posComrade--;

									bResult2 =
											_deleteObjectWithNotify(posComrade,pOb,0,1,
									  							pfsContainer2,0,0);

									if(posComrade >= dpos1 && posComrade <= dpos1 + length - 2)
									{
										// the endmarker was inside of the segment we are working
										// so we have to adjust the length
										length--;
									}

									break;
								}
							}
							pF = pF->getNext();
				    	}
					}
				}
				break;

				case PTO_Field:
				{
					// when deleting endnote reference, have to delete the
					// the associated stuff in the endnote section as well
					if(pO->getField()->getFieldType() == fd_Field::FD_Endnote_Ref)
					{
			    		const PP_AttrProp * pAP = NULL;
					    pO->getPieceTable()->getAttrProp(pO->getIndexAP(),&pAP);
					    UT_ASSERT(pAP);
					    const XML_Char* pszEid = NULL;
    	    		    pf_Frag_Strux *pMySection, *pESection;

						pAP->getAttribute("endnote-id", pszEid);

						pf_Frag_Strux * pfsContainer = NULL;
						pf_Frag * pF;

						// in theory, we should find our section, from
						// there get the id of the endnote section, find the endnote
						// section and then delete our block
						// however, to get to the section we have to scroll through
						// the fragments one by one, so we might just as well not
						// bother and test for the endnote-id directly
						// we know that the endnote section is definitely after this
						// fragment, so we will start with the next one

						// on further thought we will bother

					    const XML_Char* pszEnote = NULL;

						pF = pO->getPrev();
						while(pF)
						{
							if(pF->getType() == pf_Frag::PFT_Strux)
							{
								pfsContainer = static_cast<pf_Frag_Strux *>(pF);
								if(pfsContainer->getStruxType() == PTX_Section)
								{
								    pfsContainer->getPieceTable()->getAttrProp(pfsContainer->getIndexAP(),&pAP);
								    UT_ASSERT(pAP);
									bool bFound = 
										pAP->getAttribute("endnote", pszEnote);
									UT_ASSERT(bFound);
					    			break;
					   			}
					   		}
					   		pF = pF->getPrev();
					    }

					    pMySection = static_cast<pf_Frag_Strux*>(pF);

					    // now that we have got our endnote section id
					    // we will find it

				    	pF = pO->getNext();
				    	while(pF)
				    	{
							if(pF->getType() == pf_Frag::PFT_Strux)
							{
								pfsContainer = static_cast<pf_Frag_Strux *>(pF);
								if(pfsContainer->getStruxType() == PTX_SectionEndnote)
								{
								    pfsContainer->getPieceTable()->getAttrProp(pfsContainer->getIndexAP(),&pAP);
								    UT_ASSERT(pAP);
									const XML_Char * Eid = NULL;
									bool bFound = 
										pAP->getAttribute("id", Eid);
									
									if(bFound && !UT_stricmp(Eid, pszEnote))
					    				break;
					   			}
					   		}
					   		pF = pF->getNext();

				    	}

				    	// we should have our endnote section now
				    	UT_ASSERT(pF);

				    	pESection = static_cast<pf_Frag_Strux*>(pF);

				    	// lets get deleting ...
				    	pF = pF->getNext();
				    	UT_ASSERT(pF);

				    	pf_Frag_Strux * pFirstBlock = NULL;

				    	while(pF)
				    	{
				    		// first check we are still in our section
							if(pF->getType() == pf_Frag::PFT_Strux)
							{
								pfsContainer = static_cast<pf_Frag_Strux *>(pF);
								if(pfsContainer->getStruxType() != PTX_Block)
								{
									UT_DEBUGMSG(("non-block strux\n"));
									break;
								}
							}

							PT_AttrPropIndex iAPI = 0;
							bool bHasIndex = true;

							switch(pF->getType())
							{
								case pf_Frag::PFT_Strux:
									{
										pf_Frag_Strux * pFS = static_cast<pf_Frag_Strux *>(pF);
										iAPI = pFS->getIndexAP();
									}
									break;
								case pf_Frag::PFT_Object:
									{
										pf_Frag_Object * pFO = static_cast<pf_Frag_Object *>(pF);
										iAPI = pFO->getIndexAP();
									}
									break;
								case pf_Frag::PFT_Text:
									{
										pf_Frag_Text * pFT = static_cast<pf_Frag_Text *>(pF);
										iAPI = pFT->getIndexAP();
									}
									break;
								case pf_Frag::PFT_FmtMark:
									// formating marks are real pain and we
									// we delete any we find in this section
									// without mercy
									{
										UT_DEBUGMSG(("Deleting FmtMark\n"));
										pf_Frag_FmtMark * pFM = static_cast<pf_Frag_FmtMark *>(pF);
										pF = pF->getPrev();

										pf_Frag_Strux * pfsContainerTemp = NULL;
										bool bFoundStrux = _getStruxFromPosition(pFM->getPos(),&pfsContainerTemp);
										UT_ASSERT(bFoundStrux);
										bool bRes = _deleteFmtMarkWithNotify(pFM->getPos(),pFM,
													pfsContainerTemp,NULL,NULL);
										UT_ASSERT(bRes);

										pF = pF->getNext();
										continue;

									}
									break;
								case pf_Frag::PFT_EndOfDoc:
								    	bHasIndex = false;
									break;
								default:
									UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
									bHasIndex = false;
							}

							if(bHasIndex)
							{
							    pF->getPieceTable()->getAttrProp(iAPI,&pAP);
							    UT_ASSERT(pAP);
							    const XML_Char* pszEid2 = NULL;
    	    			    	bool bResult = false;

								bFound = pAP->getAttribute("endnote-id", pszEid2) && !UT_stricmp(pszEid2, pszEid);

							    // OK, if we have got something that
							    // carries our id we have to delete
							    // it, we do not know what it is the
							    // simplest thing is to call
							    // deleteSpan recursively this works
							    // in all cases but the first block,
							    // where calling deleteSpan deletes
							    // both the block and the section
							    // break; further, when deleting the
							    // first block we have to delete the
							    // content first
							    if(bFound)
							    {
#ifdef DEBUG
							    	UT_DEBUGMSG(("FOUND AND DELETING ... type %d\n", pF->getType()));
									if(pF->getType() == pf_Frag::PFT_Strux)
									{
										pf_Frag_Strux * pFS = static_cast<pf_Frag_Strux*>(pF);
										UT_DEBUGMSG(("strux type %d\n", pFS->getStruxType()));
									}
#endif
							    	PT_DocPosition posStart = pF->getPos();
								    PT_DocPosition posEnd   = pF->getLength() + posStart;

								    // if this is a strux, we
								    // are going to need it, otherwise
								    // we just need to get the frag ahead of us
									pf_Frag_Strux * pFS;
									if(pF->getType() == pf_Frag::PFT_Strux)
										pFS = static_cast<pf_Frag_Strux *> (pF);
									else
										pFS = NULL;

								    // get the run before us
								    pF = pF->getPrev();
#ifdef DEBUG
								    UT_DEBUGMSG(("type prev %d\n", pF ? pF->getType() : -1));
									if(pF->getType() == pf_Frag::PFT_Strux)
									{
										pf_Frag_Strux * pFS = static_cast<pf_Frag_Strux*>(pF);
										UT_DEBUGMSG(("strux type %d\n", pFS->getStruxType()));
									}
#endif
									// if this is a strux and the previous
									// is also a strux, we need to handle this
									// with more care, so we do not delelete both
									// together
								    if(pFS && pF->getType() == pf_Frag::PFT_Strux)
								    {
								    	UT_DEBUGMSG(("Two strux in sequence\n"));
								    	if(pF == pESection)
								    	{
								    		// we trying to delete the first block
								    		// it will not work, we have to delete
								    		// the content first, so we just skip it
								    		// for the moment
								    		UT_DEBUGMSG(("First block in Endnote section\n"));
								    		pFirstBlock = pFS;
								    		pF = pFS;
								    		bResult = true;
								    	}
								    	else
											bResult = _deleteStruxWithNotify(dpos1,pFS,NULL,NULL);
									}
									else
										bResult = deleteSpan(posStart,posEnd,NULL,true);
									UT_ASSERT(bResult);
								}
							}
							pF = pF->getNext();
							UT_DEBUGMSG(("pF 0x%x\n", pF));
							UT_DEBUGMSG(("type %d\n", pF ? pF->getType() : -1));
#ifdef DEBUG
							if(pF && pF->getType() == pf_Frag::PFT_Strux)
							{
								pf_Frag_Strux * pFS = static_cast<pf_Frag_Strux*>(pF);
								UT_DEBUGMSG(("strux type %d\n", pFS->getStruxType()));
							}
#endif
					  	}

					  	// now if we ignored the first block, we should
					  	// be able to delete it. Also, if we delete the
					  	// first block, we need to check whether there
					  	// are any endnotes left, and if not then we have to
					  	// delete the endnote section and reset the endnote
					  	// attribute of the original section.
					  	if(pFirstBlock)
					  	{
					  		bool bSectionEmpty = true;

					  		pF = pFirstBlock->getNext();
					  		UT_ASSERT(pF);
				  			switch(pF->getType())
				  			{
								case pf_Frag::PFT_Strux:
									{
										pf_Frag_Strux * pFS = static_cast<pf_Frag_Strux *>(pF);
										switch(pFS->getStruxType())
										{
											case PTX_Block:
												bSectionEmpty = false;
											default:
												;
										}
									}
									break;
								case pf_Frag::PFT_FmtMark:
								case pf_Frag::PFT_Text:
								case pf_Frag::PFT_Object:
									bSectionEmpty = false;
									break;
								case pf_Frag::PFT_EndOfDoc:
									break;
								default:
									UT_ASSERT(UT_SHOULD_NOT_HAPPEN);

					  		}

							if(bSectionEmpty)
							{
								UT_DEBUGMSG(("Endnote section empty, deleting\n"));
								bResult = _deleteStruxWithNotify(pESection->getPos(),pESection,NULL,NULL);

								// now we need to remove the endnote attribute from the orig section
								// we assume here that each endnote section is used by only one
								// other section in the document; if this should change in the
								// future, we will have to scan the whole doc ...
								const XML_Char*	sec_attributes2[] = {
									"endnote", "",
									NULL, NULL
									};

								PT_DocPosition posSec = pMySection->getPos();
								changeStruxFmt(PTC_RemoveFmt, posSec, posSec, sec_attributes2, NULL, PTX_Section);

							}

					  		UT_DEBUGMSG(("Deleting first block\n"));
							bResult = _deleteStruxWithNotify(pFirstBlock->getPos(),pFirstBlock,NULL,NULL);
							UT_ASSERT(bResult);
						}
				    }
				    // no break, let everything fall through
				}
				default:
					bResult
						= _deleteObjectWithNotify(dpos1,pO,fragOffset_First,lengthThisStep,
									  pfsContainer,&pfNewEnd,&fragOffsetNewEnd);

			}


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

			bool bResult = _deleteStruxWithNotify(dpos1,pfs,
												  &pfNewEnd,&fragOffsetNewEnd, 
												  false);
			UT_ASSERT(bResult);
			// we do not update pfsContainer because we just deleted pfs.
		}
		break;

		case pf_Frag::PFT_Text:
		{
			bool bResult
				= _deleteSpanWithNotify(dpos1,
										static_cast<pf_Frag_Text *>(pf_First),
										fragOffset_First,lengthThisStep,
										pfsContainer,&pfNewEnd,
										&fragOffsetNewEnd, false);
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



bool pt_PieceTable::_realDeleteSpan(PT_DocPosition dpos1,
									PT_DocPosition dpos2,
									PP_AttrProp *p_AttrProp_Before,
									bool bDeleteTableStruxes, 
									bool bDontGlob)
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

	// The code used to only glob for the complex case. But when
	// there's a simple delete, we may still end up adding the
	// formatmark below (i.e., when deleting all the text in a
	// document), and thus creating a two-step undo for a perceived
	// one-step operation. See Bug FIXME
	if(!bDontGlob)
	{
		beginMultiStepGlob();
	}

	bool bIsSimple = _isSimpleDeleteSpan(dpos1, dpos2) && stDelayStruxDelete.getDepth() == 0;
	if (bIsSimple)
	{
		//  If the delete is sure to be within a fragment, we don't
		//  need to worry about much of the bookkeeping of a complex
		//  delete.
		bSuccess = _deleteComplexSpan(dpos1, dpos2, &stDelayStruxDelete);
	}
	else
	{
		//  If the delete spans multiple fragments, we need to
		//  be a bit more careful about deleting the formatting
		//  first, and then the actual spans.
		_changePointWithNotify(old_dpos2);

		UT_uint32 oldDepth = stDelayStruxDelete.getDepth();
		bSuccess = _deleteFormatting(dpos1, dpos2);
		if (bSuccess)
			bSuccess = _deleteComplexSpan(dpos1, dpos2, &stDelayStruxDelete);
		
		bool prevDepthReached = false;
		PT_DocPosition finalPos = dpos1;
		while (bSuccess && stDelayStruxDelete.getDepth() > 0)
		{
			pf_Frag_Strux * pfs;
			if(stDelayStruxDelete.getDepth() <= oldDepth)
			{
				prevDepthReached = true;
			}
			stDelayStruxDelete.pop((void **)&pfs);

 			pf_Frag *pf;
			PT_DocPosition dp;
			if(bDeleteTableStruxes || prevDepthReached )
			{
				PT_DocPosition myPos = pfs->getPos();
				if(!prevDepthReached)
				{
//					_deleteFormatting(myPos - pfs->getLength(), myPos);
//					bSuccess = _deleteStruxWithNotify(myPos - pfs->getLength(), pfs,
//													  &pf, &dp);
					bSuccess = _deleteStruxWithNotify(myPos, pfs, &pf, &dp);
				}
				else
				{
					_deleteFormatting(dpos1 - pfs->getLength(), dpos1);
					bSuccess = _deleteStruxWithNotify(dpos1 - pfs->getLength(), pfs,
													  &pf, &dp);
				}
			}
			else
			{
				bSuccess = true;
				pf = pfs->getNext();
				dp = 0;
				dpos1 = dpos1 + pfs->getLength();
			}
		}

		_changePointWithNotify(finalPos);
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
		// Except if we're realy don't want it. We know we dont if
		// bDontGlob is true.
		if(!bDontGlob)
			_insertFmtMarkFragWithNotify(PTC_AddFmt, dpos1, &AttrProp_Before);

	}

	// End the glob after (maybe) having inserted the FmtMark
	if (!bDontGlob)
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





