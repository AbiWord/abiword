 
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
		// let's just overwrite the indexAP and return.

		// TODO check for coalesing.
		
		pft->setIndexAP(indexNewAP);

		SETP(ppfNewEnd, pft->getNext());
		SETP(pfragOffsetNewEnd, 0);
		
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

		// TODO check for coalesing.
		
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
		// formatting to the new one.

		UT_uint32 len_1 = fragOffset;
		UT_uint32 len_2 = length;
		PT_BufIndex bi_2 = m_varset.getBufIndex(pft->getBufIndex(),len_1);
		pf_Frag_Text * pftNew = new pf_Frag_Text(this,bi_2,len_2,indexNewAP);
		if (!pftNew)
			return UT_FALSE;

		// TODO check for coalesing.
		
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

	PX_ChangeRecord_SpanChange * pcr
		= new PX_ChangeRecord_SpanChange(PX_ChangeRecord::PXT_ChangeSpan,
										 dpos, indexOldAP,indexNewAP, ptc,
										 m_varset.getBufIndex(pft->getBufIndex(),fragOffset),
										 length);
	UT_ASSERT(pcr);
	UT_Bool bResult = _fmtChangeSpan(pft,fragOffset,length,indexNewAP,ppfNewEnd,pfragOffsetNewEnd);
	m_pDocument->notifyListeners(pfs,pcr);

	// add record to history.  we do not attempt to coalesce these.
	m_history.addChangeRecord(pcr);

	return bResult;
}

UT_Bool pt_PieceTable::changeSpanFmt(PTChangeFmt ptc,
									 PT_DocPosition dpos1,
									 PT_DocPosition dpos2,
									 const XML_Char ** attributes,
									 const XML_Char ** properties)
{
	UT_ASSERT(m_pts==PTS_Editing);

	// apply a span-level formating change to the given region.

	if (dpos1 == dpos2)
	{
		// if length of change is zero, then we have a toggle format.
		return _setTemporarySpanFmtWithNotify(ptc,dpos1,attributes,properties);
	}
	if (_haveTempSpanFmt(NULL,NULL))
		clearTemporarySpanFmt();
	
	UT_ASSERT(dpos1 < dpos2);
	UT_Bool bHaveAttributes = (attributes && *attributes);
	UT_Bool bHaveProperties = (properties && *properties);
	UT_ASSERT(bHaveAttributes || bHaveProperties); // must have something to do

	pf_Frag * pf_First;
	pf_Frag * pf_End;
	PT_BlockOffset fragOffset_First;
	
	UT_Bool bFound1 = getFragFromPosition(dpos1,&pf_First,&fragOffset_First);
	UT_Bool bFound2 = getFragFromPosition(dpos2,&pf_End,NULL);
	UT_ASSERT(bFound1 && bFound2);

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
		}

		dpos1 += lengthThisStep;
		length -= lengthThisStep;
		
		// since _fmtChangeSpanWithNotify(), can delete pf_First, mess with the
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
