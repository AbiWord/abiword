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


// changeSpanFmt-related fuctions for class pt_PieceTable

#include "ut_types.h"
#include "ut_misc.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_growbuf.h"
#include "pt_PieceTable.h"
#include "pf_Frag.h"
#include "pf_Frag_Object.h"
#include "pf_Frag_Strux.h"
#include "pf_Frag_Strux_Block.h"
#include "pf_Frag_Strux_Section.h"
#include "pf_Frag_Text.h"
#include "pf_Frag_FmtMark.h"
#include "pf_Fragments.h"
#include "px_ChangeRecord.h"
#include "px_CR_Span.h"
#include "px_CR_SpanChange.h"
#include "px_CR_Strux.h"


#define SETP(p,v)	do { if (p) (*(p)) = (v); } while (0)

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

		// try to coalesce this modified fragment with one of its neighbors.
		// first we try the pft->next -- if that works, we can stop because
		// the unlink will take care of checking pft->next with pft->prev.
		// if it doesn't work, we then try pft->prev.
		
		pf_Frag * pfNext = pft->getNext();
		if (pfNext && pfNext->getType()==pf_Frag::PFT_Text)
		{
			pf_Frag_Text * pftNext = static_cast<pf_Frag_Text *>(pfNext);
			if (   (pftNext->getIndexAP() == indexNewAP)
				&& (m_varset.isContiguous(pft->getBufIndex(),length,pftNext->getBufIndex())))
			{
				// the pft given and the pft->next can be coalesced.
				// let's donate all of our document data to pft->next,
				// set pft to be empty and then let _unlinkFrag() take
				// care of all the other details (like checking to see
				// if pft->next and pft->prev can be coalesced after pft
				// is out of the way....)

				pftNext->adjustOffsetLength(pft->getBufIndex(),length+pftNext->getLength());
				// we could do a: pft->changeLength(0); but it causes an assert and
				// besides _unlinkFrag() doesn't look at it and we're going to delete it.
				_unlinkFrag(pft,ppfNewEnd,pfragOffsetNewEnd);
				delete pft;
				return UT_TRUE;
			}
		}

		pf_Frag * pfPrev = pft->getPrev();
		if (pfPrev && pfPrev->getType()==pf_Frag::PFT_Text)
		{
			pf_Frag_Text * pftPrev = static_cast<pf_Frag_Text *>(pfPrev);
			if (   (pftPrev->getIndexAP() == indexNewAP)
				&& (m_varset.isContiguous(pftPrev->getBufIndex(),pftPrev->getLength(),pft->getBufIndex())))
			{
				// the pft given and the pft->prev can be coalesced.
				// let's donate all of our document data to the pft->prev,
				// set pft to be empty and then let _unlinkFrag() take
				// care of the dirty work.

				pftPrev->changeLength(pftPrev->getLength()+length);
				// we could do a: pft->changeLength(0); but it causes an assert and
				// besides _unlinkFrag() doesn't look at it and we're going to delete it.
				_unlinkFrag(pft,ppfNewEnd,pfragOffsetNewEnd);
				delete pft;
				return UT_TRUE;
			}
		}

		// otherwise, we just overwrite the indexAP on this fragment.
		
		pft->setIndexAP(indexNewAP);
		SETP(ppfNewEnd, pft->getNext());
		SETP(pfragOffsetNewEnd, 0);
		
		return UT_TRUE;
	}

	if (fragOffset == 0)
	{
		// the change is at the beginning of the fragment.
		// we need to split the existing fragment into 2 parts
		// and apply the new formatting to the new first half.
		// before we actually create the new one, we see if we
		// can coalesce the first half (with the new formatting)
		// with the previous fragment.  if not, then we cut
		// the existing fragment into 2 parts.

		UT_uint32 len_1 = length;
		UT_uint32 len_2 = pft->getLength() - len_1;
		PT_BufIndex bi_1 = m_varset.getBufIndex(pft->getBufIndex(),0);
		PT_BufIndex bi_2 = m_varset.getBufIndex(pft->getBufIndex(),len_1);

		pf_Frag * pfPrev = pft->getPrev();
		if (pfPrev && pfPrev->getType()==pf_Frag::PFT_Text)
		{
			pf_Frag_Text * pftPrev = static_cast<pf_Frag_Text *>(pfPrev);
			if (   (pftPrev->getIndexAP() == indexNewAP)
				&& (m_varset.isContiguous(pftPrev->getBufIndex(),pftPrev->getLength(),pft->getBufIndex())))
			{
				// yes we can coalesce.  move the first half into the previous fragment.

				pftPrev->changeLength(pftPrev->getLength()+length);
				pft->adjustOffsetLength(bi_2,len_2);
				SETP(ppfNewEnd, pft);
				SETP(pfragOffsetNewEnd, 0);
		
				return UT_TRUE;
			}
		}

		// otherwise, we need to actually split this one....
		
		pf_Frag_Text * pftNew = new pf_Frag_Text(this,bi_1,len_1,indexNewAP);
		if (!pftNew)
			return UT_FALSE;

		pft->adjustOffsetLength(bi_2,len_2);
		m_fragments.insertFrag(pft->getPrev(),pftNew);

		SETP(ppfNewEnd, pft);
		SETP(pfragOffsetNewEnd, 0);
		
		return UT_TRUE;
	}

	if (fragOffset+length == pft->getLength())
	{
		// the change is at the end of the fragment, we cut
		// the existing fragment into 2 parts and apply the new
		// formatting to the new second half.  before we actually
		// create the new one, we see if we can coalesce the
		// second half (with the new formatting) with the next
		// fragment.

		UT_uint32 len_1 = fragOffset;
		UT_uint32 len_2 = length;
		PT_BufIndex bi_2 = m_varset.getBufIndex(pft->getBufIndex(),len_1);

		pf_Frag * pfNext = pft->getNext();
		if (pfNext && pfNext->getType()==pf_Frag::PFT_Text)
		{
			pf_Frag_Text * pftNext = static_cast<pf_Frag_Text *>(pfNext);
			if (   (pftNext->getIndexAP() == indexNewAP)
				&& (m_varset.isContiguous(bi_2,len_2,pftNext->getBufIndex())))
			{
				// yes we can coalesce.  move the second half into the next fragment.

				pftNext->adjustOffsetLength(bi_2,len_2+pftNext->getLength());
				pft->changeLength(len_1);
				SETP(ppfNewEnd,pftNext);
				SETP(pfragOffsetNewEnd,len_2);
				return UT_TRUE;
			}
		}

		// otherwise, we actually need to split this one....

		pf_Frag_Text * pftNew = new pf_Frag_Text(this,bi_2,len_2,indexNewAP);
		if (!pftNew)
			return UT_FALSE;

		pft->changeLength(len_1);
		m_fragments.insertFrag(pft,pftNew);

		SETP(ppfNewEnd, pftNew->getNext());
		SETP(pfragOffsetNewEnd, 0);
		
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

	SETP(ppfNewEnd, pft_3);
	SETP(pfragOffsetNewEnd, 0);
		
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

	if (length == 0)					// TODO decide if this is an error.
	{
		UT_DEBUGMSG(("_fmtChangeSpanWithNotify: length==0\n"));
		SETP(ppfNewEnd, pft->getNext());
		SETP(pfragOffsetNewEnd, 0);
		return UT_TRUE;
	}

	UT_ASSERT(fragOffset+length <= pft->getLength());
	
	PT_AttrPropIndex indexNewAP;
	PT_AttrPropIndex indexOldAP = pft->getIndexAP();
	UT_Bool bMerged = m_varset.mergeAP(ptc,indexOldAP,attributes,properties,&indexNewAP);
	UT_ASSERT(bMerged);

	if (indexOldAP == indexNewAP)		// the requested change will have no effect on this fragment.
	{
		if (fragOffset+length == pft->getLength())
		{
			SETP(ppfNewEnd, pft->getNext());
			SETP(pfragOffsetNewEnd, 0);
		}
		else
		{
			SETP(ppfNewEnd, pft);
			SETP(pfragOffsetNewEnd, fragOffset+length);
		}
		
		return UT_TRUE;
	}
	
	// we do this before the actual change because various fields that
	// we need may be blown away during the change.  we then notify all
	// listeners of the change.

	PT_BlockOffset blockOffset = _computeBlockOffset(pfs,pft) + fragOffset;

	PX_ChangeRecord_SpanChange * pcr
		= new PX_ChangeRecord_SpanChange(PX_ChangeRecord::PXT_ChangeSpan,
										 dpos, indexOldAP,indexNewAP,
										 m_varset.getBufIndex(pft->getBufIndex(),fragOffset),
										 length,blockOffset);
	UT_ASSERT(pcr);
	UT_Bool bResult = _fmtChangeSpan(pft,fragOffset,length,indexNewAP,ppfNewEnd,pfragOffsetNewEnd);

	// add record to history.  we do not attempt to coalesce these.
	m_history.addChangeRecord(pcr);
	m_pDocument->notifyListeners(pfs,pcr);

	return bResult;
}

UT_Bool pt_PieceTable::changeSpanFmt(PTChangeFmt ptc,
									 PT_DocPosition dpos1,
									 PT_DocPosition dpos2,
									 const XML_Char ** attributes,
									 const XML_Char ** properties)
{
	// apply a span-level formatting change to the given region.
	
	UT_ASSERT(m_pts==PTS_Editing);

	if (dpos1 == dpos2) 		// if length of change is zero, then we have a toggle format.
	{
		UT_Bool bRes = _insertFmtMarkFragWithNotify(ptc,dpos1,attributes,properties);
		// Won't be a persistant change if it's just a toggle
		PX_ChangeRecord *pcr=0;
		m_history.getUndo(&pcr);
		if (pcr)
		{
			UT_DEBUGMSG(("Setting persistance of change to false\n"));
			pcr->setPersistance(UT_FALSE);
			m_history.setSavePosition(m_history.getSavePosition()+1);
		}
		return bRes;
	}
	
	UT_ASSERT(dpos1 < dpos2);
	UT_Bool bHaveAttributes = (attributes && *attributes);
	UT_Bool bHaveProperties = (properties && *properties);
	UT_ASSERT(bHaveAttributes || bHaveProperties); // must have something to do

	pf_Frag * pf_First;
	pf_Frag * pf_End;
	PT_BlockOffset fragOffset_First;
	PT_BlockOffset fragOffset_End;

	UT_Bool bFound = getFragsFromPositions(dpos1,dpos2,&pf_First,&fragOffset_First,&pf_End,&fragOffset_End);
	UT_ASSERT(bFound);

#if 0
	{
		pf_Frag * pf1, * pf2;
		PT_BlockOffset fo1, fo2;

		UT_Bool bFound1 = getFragFromPosition(dpos1,&pf1,&fo1);
		UT_Bool bFound2 = getFragFromPosition(dpos2,&pf2,&fo2);
		UT_ASSERT(bFound1 && bFound2);
		UT_ASSERT((pf1==pf_First) && (fragOffset_First==fo1));
		UT_ASSERT((pf2==pf_End) && (fragOffset_End==fo2));
	}
#endif	

	// see if the amount of text to be changed is completely
	// contained within a single fragment.  if so, we have a
	// simple change.  otherwise, we need to set up a multi-step
	// change -- it may not actually take more than one step,
	// but it is too complicated to tell at this point, so we
	// assume it will and don't worry about it.
	//
	// we are in a simple change if the beginning and end are
	// within the same fragment.

	// NOTE: if we call beginMultiStepGlob() we ***MUST*** call
	// NOTE: endMultiStepGlob() before we return -- otherwise,
	// NOTE: the undo/redo won't be properly bracketed.

	UT_Bool bSimple = (pf_First == pf_End);
	if (!bSimple)
		beginMultiStepGlob();

	pf_Frag_Strux * pfsContainer = NULL;
	pf_Frag * pfNewEnd;
	UT_uint32 fragOffsetNewEnd;

	UT_uint32 length = dpos2 - dpos1;
	while (length > 0)
	{
		UT_ASSERT(dpos1+length==dpos2);

		UT_uint32 lengthInFrag = pf_First->getLength() - fragOffset_First;
		UT_uint32 lengthThisStep = UT_MIN(lengthInFrag, length);
		
		switch (pf_First->getType())
		{
		case pf_Frag::PFT_EndOfDoc:
		default:
			UT_ASSERT(0);
			return UT_FALSE;
			
		case pf_Frag::PFT_Strux:
			{
				// we are only applying span-level changes, so we ignore strux.
				// but we still need to update our loop indices.

				pfNewEnd = pf_First->getNext();
				fragOffsetNewEnd = 0;
				pfsContainer = static_cast<pf_Frag_Strux *> (pf_First);
			}
			break;

		case pf_Frag::PFT_Text:
			{
				if (!pfsContainer)
				{
					UT_Bool bFoundStrux = _getStruxFromPosition(dpos1,&pfsContainer);
					UT_ASSERT(bFoundStrux);
				}

				UT_Bool bResult
					= _fmtChangeSpanWithNotify(ptc,static_cast<pf_Frag_Text *>(pf_First),
											   fragOffset_First,dpos1,lengthThisStep,
											   attributes,properties,
											   pfsContainer,&pfNewEnd,&fragOffsetNewEnd);
				UT_ASSERT(bResult);
			}
			break;

		case pf_Frag::PFT_Object:
			{
				if (!pfsContainer)
				{
					UT_Bool bFoundStrux = _getStruxFromPosition(dpos1,&pfsContainer);
					UT_ASSERT(bFoundStrux);
				}

				UT_Bool bResult
					= _fmtChangeObjectWithNotify(ptc,static_cast<pf_Frag_Object *>(pf_First),
												 fragOffset_First,dpos1,lengthThisStep,
												 attributes,properties,
												 pfsContainer,&pfNewEnd,&fragOffsetNewEnd);
				UT_ASSERT(bResult);
			}
			break;

		case pf_Frag::PFT_FmtMark:
			{
				if (!pfsContainer)
				{
					UT_Bool bFoundStrux = _getStruxFromPosition(dpos1,&pfsContainer);
					UT_ASSERT(bFoundStrux);
				}

				UT_Bool bResult
					= _fmtChangeFmtMarkWithNotify(ptc,static_cast<pf_Frag_FmtMark *>(pf_First),
												  dpos1, attributes,properties,
												  pfsContainer,&pfNewEnd,&fragOffsetNewEnd);
				UT_ASSERT(bResult);
			}
			break;

		}

		dpos1 += lengthThisStep;
		length -= lengthThisStep;
		
		// since _fmtChange{Span,FmtMark,...}WithNotify(), can delete pf_First, mess with the
		// fragment list, and does some aggressive coalescing of
		// fragments, we cannot just do a pf_First->getNext() here.
		// to advance to the next fragment, we use the *NewEnd variables
		// that each of the cases routines gave us.

		pf_First = pfNewEnd;
		if (!pf_First)
			length = 0;
		fragOffset_First = fragOffsetNewEnd;
	}

	if (!bSimple)
		endMultiStepGlob();
		
	return UT_TRUE;
}
