/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; indent-tabs-mode: t -*- */
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
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
#include "pd_Style.h"
#include "pp_Revision.h"

#define SETP(p,v)	do { if (p) (*(p)) = (v); } while (0)

/****************************************************************/
/****************************************************************/

bool pt_PieceTable::changeSpanFmt(PTChangeFmt ptc,
									 PT_DocPosition dpos1,
									 PT_DocPosition dpos2,
									 const PP_PropertyVector & attributes,
								  const PP_PropertyVector & properties)
{
	// if dpos1 == dpos2 we are inserting a fmt mark; this must be chanelled throught
	// the non-revision branch ...
	if(m_pDocument->isMarkRevisions() && dpos1 != dpos2)
	{
		const gchar name[] = "revision";
		const gchar * pRevision = NULL;

		// we cannot retrieve the start and end fragments here and
		// then work between them in a loop using getNext() because
		// processing might result in merging of fargments. so we have
		// to use the doc position to keep track of where we are and
		// retrieve the fragments afresh in each step of the loop
		// Tomas, Dec 29, 2004

		bool bRet = false;
		while(dpos1 < dpos2)
		{
			// first retrive the starting and ending fragments
			pf_Frag * pf1, * pf2;
			PT_BlockOffset Offset1, Offset2;

			if(!getFragsFromPositions(dpos1,dpos2, &pf1, &Offset1, &pf2, &Offset2) ||
			   pf1->getType() == pf_Frag::PFT_EndOfDoc)
				return bRet;
			else
				bRet = true;
			
			// get attributes for this fragement
			const PP_AttrProp * pAP;
			pRevision = NULL;
			
			if(_getSpanAttrPropHelper(pf1, &pAP))
			{
				pAP->getAttribute(name, pRevision);
			}

			PP_RevisionAttr Revisions(pRevision);


			// if the request is for removal of fmt, in the revision mode, we still
			// have to add these props (the removal is indicated by their emptiness)
			// as we cannot rely on callers to set these correctly, we have to emtpy
			// them ourselves
			PP_PropertyVector attrs;
			PP_PropertyVector props;

			if(ptc == PTC_RemoveFmt)
			{
				attrs = PP_std_setPropsToNothing(attributes);
				props = PP_std_setPropsToNothing(properties);
			} else {
				attrs = attributes;
				props = properties;
			}

			Revisions.addRevision(m_pDocument->getRevisionId(),
								  PP_REVISION_FMT_CHANGE, attrs, props);

			const PP_PropertyVector ppRevAttrib = {
				name, Revisions.getXMLstring()
			};

			PT_DocPosition dposEnd = UT_MIN(dpos2,dpos1 + pf1->getLength());

			if(!_realChangeSpanFmt(PTC_AddFmt, dpos1, dposEnd, ppRevAttrib, PP_NOPROPS, false))
				return false;

			dpos1 = dposEnd;
		}

		return true;
	}
	else
	{
		return _realChangeSpanFmt(ptc, dpos1, dpos2, attributes, properties, false);
	}
}

bool pt_PieceTable::_fmtChangeSpan(pf_Frag_Text * pft, UT_uint32 fragOffset, UT_uint32 length,
									  PT_AttrPropIndex indexNewAP,
									  pf_Frag ** ppfNewEnd, UT_uint32 * pfragOffsetNewEnd)
{
	UT_return_val_if_fail (length > 0,false);
	UT_return_val_if_fail (fragOffset+length <= pft->getLength(), false);

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
				return true;
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
				return true;
			}
		}

		// otherwise, we just overwrite the indexAP on this fragment.

		pft->setIndexAP(indexNewAP);
		SETP(ppfNewEnd, pft->getNext());
		SETP(pfragOffsetNewEnd, 0);

		return true;
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

				return true;
			}
		}

		// otherwise, we need to actually split this one....

		pf_Frag_Text * pftNew = new pf_Frag_Text(this,bi_1,len_1,indexNewAP,pft->getField());
		if (!pftNew)
			return false;

		pft->adjustOffsetLength(bi_2,len_2);
		m_fragments.insertFrag(pft->getPrev(),pftNew);

		SETP(ppfNewEnd, pft);
		SETP(pfragOffsetNewEnd, 0);

		return true;
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
				return true;
			}
		}

		// otherwise, we actually need to split this one....

		pf_Frag_Text * pftNew = new pf_Frag_Text(this,bi_2,len_2,indexNewAP,pft->getField());
		if (!pftNew)
			return false;

		pft->changeLength(len_1);
		m_fragments.insertFrag(pft,pftNew);

		SETP(ppfNewEnd, pftNew->getNext());
		SETP(pfragOffsetNewEnd, 0);

		return true;
	}

	// otherwise, change is in the middle of the fragment.  we
	// need to cut the existing fragment into 3 parts and apply
	// the new formatting to the middle one.

	UT_uint32 len_1 = fragOffset;
	UT_uint32 len_2 = length;
	UT_uint32 len_3 = pft->getLength() - (fragOffset+length);
	PT_BufIndex bi_2 = m_varset.getBufIndex(pft->getBufIndex(),fragOffset);
	PT_BufIndex bi_3 = m_varset.getBufIndex(pft->getBufIndex(),fragOffset+length);
	pf_Frag_Text * pft_2 = new pf_Frag_Text(this,bi_2,len_2,indexNewAP,pft->getField());
	UT_return_val_if_fail (pft_2, false);
	pf_Frag_Text * pft_3 = new pf_Frag_Text(this,bi_3,len_3,pft->getIndexAP(),pft->getField());
	UT_return_val_if_fail (pft_3, false);

	pft->changeLength(len_1);
	m_fragments.insertFrag(pft,pft_2);
	m_fragments.insertFrag(pft_2,pft_3);

	SETP(ppfNewEnd, pft_3);
	SETP(pfragOffsetNewEnd, 0);

	return true;
}

bool pt_PieceTable::_fmtChangeSpanWithNotify(PTChangeFmt ptc,
											 pf_Frag_Text * pft, UT_uint32 fragOffset,
											 PT_DocPosition dpos,
											 UT_uint32 length,
											 const PP_PropertyVector & attributes,
											 const PP_PropertyVector & properties,
											 pf_Frag_Strux * pfs,
											 pf_Frag ** ppfNewEnd,
											 UT_uint32 * pfragOffsetNewEnd,
											 bool bRevisionDelete)
{
	// create a change record for this change and put it in the history.

	if (length == 0)					// TODO decide if this is an error.
	{
		UT_DEBUGMSG(("_fmtChangeSpanWithNotify: length==0\n"));
		SETP(ppfNewEnd, pft->getNext());
		SETP(pfragOffsetNewEnd, 0);
		return true;
	}

	UT_return_val_if_fail (fragOffset+length <= pft->getLength(), false);

	PT_AttrPropIndex indexNewAP;
	PT_AttrPropIndex indexOldAP = pft->getIndexAP();
	UT_DebugOnly<bool> bMerged;
	if(attributes.empty() && properties.empty())
	{
	    //
	    // Clear out all attributes/properties and set to the first index
	    //
	    bMerged = true;
	    indexNewAP = 0;
	}
	else
	  bMerged = m_varset.mergeAP(ptc,indexOldAP,attributes,properties,&indexNewAP,getDocument());

	UT_ASSERT_HARMLESS(bMerged);

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

		return true;
	}

	// we do this before the actual change because various fields that
	// we need may be blown away during the change.  we then notify all
	// listeners of the change.

	PT_BlockOffset blockOffset = _computeBlockOffset(pfs,pft) + fragOffset;

	PX_ChangeRecord_SpanChange * pcr
		= new PX_ChangeRecord_SpanChange(PX_ChangeRecord::PXT_ChangeSpan,
										 dpos, indexOldAP,indexNewAP,
										 m_varset.getBufIndex(pft->getBufIndex(),fragOffset),
										 length,blockOffset,bRevisionDelete);
	UT_return_val_if_fail (pcr,false);
	bool bResult = _fmtChangeSpan(pft,fragOffset,length,indexNewAP,ppfNewEnd,pfragOffsetNewEnd);

	// add record to history.  we do not attempt to coalesce these.
	m_history.addChangeRecord(pcr);
	m_pDocument->notifyListeners(pfs,pcr);

	return bResult;
}

bool pt_PieceTable::_realChangeSpanFmt(PTChangeFmt ptc,
									   PT_DocPosition dpos1,
									   PT_DocPosition dpos2,
									   const PP_PropertyVector & attributes,
									   const PP_PropertyVector & properties,
									   bool bRevisionDelete)
{
	// apply a span-level formatting change to the given region.

	UT_return_val_if_fail (m_pts==PTS_Editing,false);
    _tweakFieldSpan(dpos1,dpos2);
//
// Deal with case of exactly selecting the endOfFootnote
//
	pf_Frag * pfEndDum = m_fragments.findFirstFragBeforePos(dpos2);
	if(isEndFootnote(pfEndDum))
	{
		if(dpos2 > dpos1)
		{
			dpos2--;
		}
	}
//
// Deal with addStyle
//
	bool bApplyStyle = (PTC_AddStyle == ptc);
	PP_PropertyVector lProps = properties;
	if(bApplyStyle)
	{
//
// OK for styles we expand out all defined properties including BasedOn styles
// Then we use these to eliminate any specfic properties in the current strux
// Then properties in the current strux will resolve to those defined in the
// style (they exist there) to specifc values in strux (if not overridden by
// the style) then finally to default value.
//
		const std::string & szStyle = PP_getAttribute(PT_STYLE_ATTRIBUTE_NAME,
													  attributes);
		PD_Style * pStyle = NULL;
		UT_return_val_if_fail (!szStyle.empty(),false);
		getDocument()->getStyle(szStyle.c_str(),&pStyle);
		UT_return_val_if_fail (pStyle,false);
		UT_Vector vProps;
//
// Get the vector of properties
//
		pStyle->getAllProperties(&vProps,0);
		PP_PropertyVector sProps;
//
// Finally make the PropertyVector
//
		UT_uint32 countp = vProps.getItemCount();
		UT_uint32 i;
		for(i=0; i<countp; i++)
		{
			sProps.push_back((const gchar *)vProps.getNthItem(i));
		}
		lProps = sProps;
	}
	if (dpos1 == dpos2) 		// if length of change is zero, then we have a toggle format.
	{
		UT_uint32 startUndoPos = m_history.getUndoPos();
		bool bRes = _insertFmtMarkFragWithNotify(ptc,dpos1,attributes,lProps);
		UT_uint32 endUndoPos = m_history.getUndoPos();
		// Won't be a persistant change if it's just a toggle
		PX_ChangeRecord *pcr=0;
		m_history.getUndo(&pcr,true);
		if (pcr && (startUndoPos != endUndoPos) )
		{
			UT_DEBUGMSG(("Setting persistance of change to false\n"));
			pcr->setPersistance(false);
			m_history.setSavePosition(m_history.getSavePosition()+1);
		}
		return bRes;
	}

	UT_return_val_if_fail (dpos1 < dpos2,false);

	pf_Frag * pf_First;
	pf_Frag * pf_End;
	PT_BlockOffset fragOffset_First;
	PT_BlockOffset fragOffset_End;

	bool bFound;
	bFound = getFragsFromPositions(dpos1,dpos2,&pf_First,&fragOffset_First,&pf_End,&fragOffset_End);
	UT_return_val_if_fail (bFound, false);
	bool bSkipFootnote = _checkSkipFootnote(dpos1,dpos2,pf_End);

#if 0
	{
		pf_Frag * pf1, * pf2;
		PT_BlockOffset fo1, fo2;

		bool bFound1 = getFragFromPosition(dpos1,&pf1,&fo1);
		bool bFound2 = getFragFromPosition(dpos2,&pf2,&fo2);
		UT_return_val_if_fail (bFound1 && bFound2, false);
		UT_return_val_if_fail ((pf1==pf_First) && (fragOffset_First==fo1), false);
		UT_return_val_if_fail ((pf2==pf_End) && (fragOffset_End==fo2), false);
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

	bool bSimple = (pf_First == pf_End);
	if (!bSimple)
		beginMultiStepGlob();
    // UT_DEBUGMSG(("ODTCT: realChangeSpanFmt() bSimple:%d\n", bSimple ));

	pf_Frag_Strux * pfsContainer = NULL;
	pf_Frag * pfNewEnd;
	UT_uint32 fragOffsetNewEnd;

	UT_uint32 length = dpos2 - dpos1;
	while (length != 0)
	{
		// FIXME: Special check to support a FmtMark at the end of the
		// document. This is necessary because FmtMarks don't have a
		// length...  See bug 452.
		if (0 == length
			&& (!pf_First || pf_Frag::PFT_FmtMark != pf_First->getType()))
			break;

		UT_return_val_if_fail (dpos1+length==dpos2, false);

		UT_uint32 lengthInFrag = pf_First->getLength() - fragOffset_First;
		UT_uint32 lengthThisStep = UT_MIN(lengthInFrag, length);

		switch (pf_First->getType())
		{
		case pf_Frag::PFT_EndOfDoc:
		default:
			UT_DEBUGMSG(("fragment type: %d\n",pf_First->getType()));
			UT_ASSERT_HARMLESS(0);
			return false;

		case pf_Frag::PFT_Strux:
			{
				// we are only applying span-level changes, so we ignore strux.
				// but we still need to update our loop indices.
				if (bSkipFootnote  && isFootnote(pf_First))
				{
					UT_uint32 extraLength = 0;
					pfNewEnd = pf_First;
					while(pfNewEnd && !isEndFootnote(pfNewEnd))
					{
						pfNewEnd = pfNewEnd->getNext();
						extraLength += pfNewEnd->getLength();
					}
					if(lengthThisStep + extraLength <= length)
					{
						lengthThisStep += extraLength;
					}
					else
					{
						UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
						lengthThisStep = length;
					}
					pfNewEnd = pfNewEnd->getNext();
					fragOffsetNewEnd = 0;
				}
				else
				{
					pfNewEnd = pf_First->getNext();
					pfsContainer = static_cast<pf_Frag_Strux *> (pf_First);
					fragOffsetNewEnd = 0;
					bool bFoundStrux = false;
					if(isEndFootnote(pfsContainer))
					{
						bFoundStrux = _getStruxFromFragSkip(pfsContainer,&pfsContainer);
						UT_return_val_if_fail (bFoundStrux, false);
					}
				}
			}
			break;

		case pf_Frag::PFT_Text:
			{
				if (!pfsContainer)
				{
					bool bFoundStrux;
					bFoundStrux = _getStruxFromPosition(dpos1,&pfsContainer);
					UT_return_val_if_fail (bFoundStrux,false);
					if(isEndFootnote(pfsContainer))
					{
						bFoundStrux = _getStruxFromFragSkip(pfsContainer,&pfsContainer);
						UT_return_val_if_fail (bFoundStrux,false);
					}
				}

                // UT_DEBUGMSG(("ODTCT: realChangeSpanFmt() text...A\n" ));
				bool bResult;
				bResult	= _fmtChangeSpanWithNotify(ptc,static_cast<pf_Frag_Text *>(pf_First),
											   fragOffset_First,dpos1,lengthThisStep,
											   attributes,lProps,
											   pfsContainer,&pfNewEnd,&fragOffsetNewEnd,bRevisionDelete);
                // UT_DEBUGMSG(("ODTCT: realChangeSpanFmt() text...B\n" ));
				UT_return_val_if_fail (bResult,false);
			}
			break;

		case pf_Frag::PFT_Object:
			{
				if (!pfsContainer)
				{
					bool bFoundStrux;
					bFoundStrux = _getStruxFromPosition(dpos1,&pfsContainer);
					UT_return_val_if_fail (bFoundStrux,false);
					if(isEndFootnote(pfsContainer))
					{
						bFoundStrux = _getStruxFromFragSkip(pfsContainer,&pfsContainer);
						UT_return_val_if_fail (bFoundStrux,false);
					}
				}

				bool bResult;
				bResult	= _fmtChangeObjectWithNotify(ptc,static_cast<pf_Frag_Object *>(pf_First),
												 fragOffset_First,dpos1,lengthThisStep,
												 attributes,lProps,
												 pfsContainer,&pfNewEnd,&fragOffsetNewEnd,false);
				UT_return_val_if_fail (bResult,false);
			}
			break;

		case pf_Frag::PFT_FmtMark:
			{
				if (!pfsContainer)
				{
					bool bFoundStrux;
					bFoundStrux = _getStruxFromPosition(dpos1,&pfsContainer);
					UT_return_val_if_fail (bFoundStrux,false);
					if(isEndFootnote(pfsContainer))
					{
						bFoundStrux = _getStruxFromFragSkip(pfsContainer,&pfsContainer);
						UT_return_val_if_fail (bFoundStrux,false);
					}

				}

				bool bResult;
				bResult = _fmtChangeFmtMarkWithNotify(ptc,static_cast<pf_Frag_FmtMark *>(pf_First),
												  dpos1, attributes,lProps,
												  pfsContainer,&pfNewEnd,&fragOffsetNewEnd);
				UT_return_val_if_fail (bResult,false);
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

	return true;
}
