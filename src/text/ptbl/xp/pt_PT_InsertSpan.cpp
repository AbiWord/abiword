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


// insertSpan-related routined for class pt_PieceTable.

#include "ut_types.h"
#include "ut_misc.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_growbuf.h"
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
#include "fd_Field.h"
/****************************************************************/
/****************************************************************/
	
UT_Bool pt_PieceTable::_insertSpan(pf_Frag * pf,
								   PT_BufIndex bi,
								   PT_BlockOffset fragOffset,
								   UT_uint32 length,
								   PT_AttrPropIndex indexAP,
                                   fd_Field * pField)
{
	// update the fragment and/or the fragment list.
	// return true if successful.
	
	pf_Frag_Text * pft = NULL;
	
	switch (pf->getType())
	{
	default:
		UT_ASSERT(0);
		return UT_FALSE;

	case pf_Frag::PFT_EndOfDoc:
	case pf_Frag::PFT_Strux:
	case pf_Frag::PFT_Object:
		// if the position they gave us is the position of a strux
		// we probably need to re-interpret it slightly.  inserting
		// prior to a paragraph should probably be interpreted as
		// appending to the previous paragraph.  likewise, if they
		// gave us the EOD marker or an Object, we probably want to
		// try to append previous text fragment.

		if (pf->getPrev() && (pf->getPrev()->getType() == pf_Frag::PFT_Text))
		{
			pf = pf->getPrev();
			pft = static_cast<pf_Frag_Text *>(pf);
			fragOffset = pft->getLength();
			break;
		}

		// otherwise, we will just insert it before us.
		fragOffset = 0;
		break;
		
	case pf_Frag::PFT_Text:
		pft = static_cast<pf_Frag_Text *>(pf);
		break;

	case pf_Frag::PFT_FmtMark:
		// insert after the FmtMark.  This is an error here.
		// we need to replace the FmtMark with a Text frag with
		// the same API.  This needs to be handled at the higher
		// level (so the glob markers can be set).
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return UT_FALSE;
	}

	if (pft&&pField==NULL)
	{
		// we have a text frag containing or adjacent to the position.
		// deal with merging/splitting/etc.

		UT_uint32 fragLen = pft->getLength();
	
		// try to coalesce this character data with an existing fragment.
		// this is very likely to be sucessful during normal data entry.

		if (fragOffset == fragLen)
		{
			// we are to insert it immediately after this fragment.
			// if we are coalescable, just append it to this fragment
			// rather than create a new one.

			if (   (pft->getIndexAP()==indexAP)
				&& m_varset.isContiguous(pft->getBufIndex(),fragLen,bi))
			{
				// new text is contiguous, we just update the length of this fragment.

				pft->changeLength(fragLen+length);

				// see if this (enlarged) fragment is now contiguous with the
				// one that follows (this can happen after a delete-char followed
				// by undo).  if so, we coalesce them.

				if (pft->getNext() && (pft->getNext()->getType() == pf_Frag::PFT_Text) && (pft->getNext()->getField()==NULL))
				{
					pf_Frag_Text * pftNext = static_cast<pf_Frag_Text *>(pft->getNext());
					if (   (pft->getIndexAP() == pftNext->getIndexAP())
						&& m_varset.isContiguous(pft->getBufIndex(),pft->getLength(),pftNext->getBufIndex()))
					{
						pft->changeLength(pft->getLength()+pftNext->getLength());
						m_fragments.unlinkFrag(pftNext);
						delete pftNext;
					}
				}
				
				return UT_TRUE;
			}
		}

		if (fragOffset == 0)
		{
			// we are to insert it immediately before this fragment.
			// if we are coalescable, just prepend it to this fragment.
		
			if (   (pft->getIndexAP()==indexAP)
				&& m_varset.isContiguous(bi,length,pft->getBufIndex()))
			{
				// new text is contiguous, we just update the offset and length of
				// of this fragment.

				pft->adjustOffsetLength(bi,length+fragLen);

				// see if this (enlarged) fragment is now contiguous with the
				// one that preceeds us (this can happen after a delete-char followed
				// by undo).  if so, we coalesce them.

				if (pft->getPrev() && (pft->getPrev()->getType() == pf_Frag::PFT_Text)&&(pft->getPrev()->getField()==NULL))
				{
					pf_Frag_Text * pftPrev = static_cast<pf_Frag_Text *>(pft->getPrev());
					if (   (pft->getIndexAP() == pftPrev->getIndexAP())
						&& m_varset.isContiguous(pftPrev->getBufIndex(),pftPrev->getLength(),pft->getBufIndex()))
					{
						pftPrev->changeLength(pftPrev->getLength()+pft->getLength());
						m_fragments.unlinkFrag(pft);
						delete pft;
					}
				}
			
				return UT_TRUE;
			}

			// one last attempt to coalesce.  if we are at the beginning of
			// the fragment, and this fragment and the previous fragment have
			// the same properties, and the character data is contiguous with
			// it, let's stick it in the previous fragment.

			pf_Frag * pfPrev = pft->getPrev();
			if (pfPrev && pfPrev->getType()==pf_Frag::PFT_Text && (pfPrev->getField()==NULL))
			{
				pf_Frag_Text * pftPrev = static_cast<pf_Frag_Text *>(pfPrev);
				UT_uint32 prevLength = pftPrev->getLength();
			
				if (   (pftPrev->getIndexAP() == indexAP)
					&& (m_varset.isContiguous(pftPrev->getBufIndex(),prevLength,bi)))
				{
					pftPrev->changeLength(prevLength+length);
					return UT_TRUE;
				}
			}
		}
	}
	
	// new text is not contiguous, we need to insert one or two new text
	// fragment(s) into the list.  first we construct a new text fragment
	// for the data that we inserted.
    
	pf_Frag_Text * pftNew = new pf_Frag_Text(this,bi,length,indexAP,pField);
	if (!pftNew)
		return UT_FALSE;

	if (fragOffset == 0)
	{
		// if change is at the beginning of the fragment, we insert a
		// single new text fragment before the one we found.

		m_fragments.insertFrag(pf->getPrev(),pftNew);
		return UT_TRUE;
	}

	UT_uint32 fragLen = pf->getLength();
	if (fragLen==fragOffset)
	{
		// if the change is after this fragment, we insert a single
		// new text fragment after the one we found.

		m_fragments.insertFrag(pf,pftNew);
		return UT_TRUE;
	}

	// if the change is in the middle of the fragment, we construct
	// a second new text fragment for the portion after the insert.

	UT_ASSERT(pft);
	
	UT_uint32 lenTail = pft->getLength() - fragOffset;
	PT_BufIndex biTail = m_varset.getBufIndex(pft->getBufIndex(),fragOffset);
	pf_Frag_Text * pftTail = new pf_Frag_Text(this,biTail,lenTail,pft->getIndexAP(),pft->getField());
	if (!pftTail)
		return UT_FALSE;
			
	pft->changeLength(fragOffset);
	m_fragments.insertFrag(pft,pftNew);
	m_fragments.insertFrag(pftNew,pftTail);
	
	return UT_TRUE;
}

UT_Bool pt_PieceTable::_lastUndoIsThisFmtMark(PT_DocPosition dpos)
{
	// look backwards thru the undo from this point and see
	// if we have <InsertFmtMark>[<ChangeFmtMark>*]
	
	PX_ChangeRecord * pcr;
	UT_uint32 undoNdx = 0;

	while (1)
	{
		UT_Bool bHaveUndo = m_history.getUndo(&pcr,undoNdx);

		if (!bHaveUndo)
			return UT_FALSE;
		if (!pcr)
			return UT_FALSE;
		if (pcr->getPosition() != dpos)
			return UT_FALSE;
		
		switch (pcr->getType())
		{
		default:
			return UT_FALSE;
		case PX_ChangeRecord::PXT_InsertFmtMark:
			return UT_TRUE;
		case PX_ChangeRecord::PXT_ChangeFmtMark:
			undoNdx++;
			break;
		}
	}

	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	return UT_FALSE;
}

UT_Bool pt_PieceTable::insertSpan(PT_DocPosition dpos,
								  const UT_UCSChar * p,
								  UT_uint32 length, fd_Field * pField)
{
	// insert character data into the document at the given position.

	UT_ASSERT(m_pts==PTS_Editing);

	// get the fragment at the given document position.
	
	pf_Frag * pf = NULL;
	PT_BlockOffset fragOffset = 0;
	UT_Bool bFound = getFragFromPosition(dpos,&pf,&fragOffset);
	UT_ASSERT(bFound);
    

	// append the text data to the end of the current buffer.

	PT_BufIndex bi;
	if (!m_varset.appendBuf(p,length,&bi))
		return UT_FALSE;

	UT_Bool bSuccess = UT_FALSE;
	pf_Frag_Strux * pfs = NULL;
	UT_Bool bFoundStrux = _getStruxFromFrag(pf,&pfs);
	UT_ASSERT(bFoundStrux);

	// we just did a getFragFromPosition() which gives us the
	// the thing *starting* at that position.  if we have a
	// fragment boundary at that position, it's sort of arbitrary
	// whether we treat this insert as a prepend to the one we just found
	// or an append to the previous one (when it's a text frag).
	// in the normal case, we want the Attr/Prop of a character
	// insertion to take the AP of the thing to the immediate
	// left (seems to be what MS-Word and MS-WordPad do).  It's also
	// useful when the user hits the BOLD button (without a)
	// selection) and then starts typing -- ideally you'd like
	// all of the text to have bold not just the first.  therefore,
	// we will see if we are on a text-text boundary and backup
	// (and thus appending) to the previous.

	UT_Bool bNeedGlob = UT_FALSE;
	PT_AttrPropIndex indexAP = 0;
	
	if ( (fragOffset==0) && (pf->getPrev()) )
	{
		UT_Bool bRightOfFmtMark = (pf->getPrev()->getType() == pf_Frag::PFT_FmtMark);
		if (bRightOfFmtMark)
		{
			// if we're just to the right of a _FmtMark, we want to replace
			// it with a _Text frag with the same attr/prop (we
			// only used the _FmtMark to remember a toggle format
			// before we had text for it).

			pf_Frag_FmtMark * pfPrevFmtMark = static_cast<pf_Frag_FmtMark *>(pf->getPrev());
			indexAP = pfPrevFmtMark->getIndexAP();

			if (_lastUndoIsThisFmtMark(dpos))
			{
				// if the last thing in the undo history is the insertion of this
				// _FmtMark, then let's remember the indexAP, do an undo, and then
				// insert the text.  this way the only thing remaining in the undo
				// is the insertion of this text (with no globbing around it).  then
				// a user-undo will undo all of the coalesced text back to this point
				// and leave the insertion point as if the original InsertFmtMark
				// had never happened.
				//
				// we don't allow consecutive FmtMarks, but the undo may be a
				// changeFmtMark and thus just re-change the mark frag rather
				// than actually deleting it.  so we loop here to get back to
				// the original insertFmtMark (this is the case if the user hit
				// BOLD then ITALIC then UNDERLINE then typed a character).

				do { undoCmd(); } while (_lastUndoIsThisFmtMark(dpos));
			}
			else
			{
				// for some reason, something else has happened to the document
				// since this _FmtMark was inserted (perhaps it was one that we
				// inserted when we did a paragraph break and inserted several
				// to remember the current inline formatting).
				//
				// here we have to do it the hard way and use a glob and an
				// explicit deleteFmtMark.  note that this messes up the undo
				// coalescing.  that is, if the user starts typing at this
				// position and then hits UNDO, we will erase all of the typing
				// except for the first character.  the second UNDO, will erase
				// the first character and restores the current FmtMark.  if the
				// user BACKSPACES instead of doing the second UNDO, both the
				// first character and the FmtMark would be gone.
				//
				// TODO decide if we like this...
				// NOTE this causes BUG#431.... :-)

				bNeedGlob = UT_TRUE;
				beginMultiStepGlob();
				_deleteFmtMarkWithNotify(dpos,pfPrevFmtMark,pfs,&pf,&fragOffset);
			}

			// we now need to consider pf invalid, since the fragment list may have
			// been coalesced as the FmtMarks were deleted.  let's recompute them
			// but with a few shortcuts.

			bFound = getFragFromPosition(dpos,&pf,&fragOffset);
			UT_ASSERT(bFound);
			bFoundStrux = _getStruxFromFrag(pf,&pfs);
			UT_ASSERT(bFoundStrux);

			// with the FmtMark now gone, we make a minor adjustment so that we
			// try to append text to the previous rather than prepend to the current.
			// this makes us consistent with other places in the code.

			if ( (fragOffset==0) && (pf->getPrev()) && (pf->getPrev()->getType() == pf_Frag::PFT_Text) && pf->getPrev()->getField()== NULL )
			{
				// append to the end of the previous frag rather than prepend to the current one.
				pf = pf->getPrev();
				fragOffset = pf->getLength();
			}
		}
		else if (pf->getPrev()->getType() == pf_Frag::PFT_Text && pf->getPrev()->getField()==NULL)
		{
			pf_Frag_Text * pfPrevText = static_cast<pf_Frag_Text *>(pf->getPrev());
			indexAP = pfPrevText->getIndexAP();

			// append to the end of the previous frag rather than prepend to the current one.
			pf = pf->getPrev();
			fragOffset = pf->getLength();
		}
		else
		{
		  // is existing fragment a field? If so do nothing 
		  // Or should we display a message to the user?
     
		        if(pf->getField() != NULL)
			{
				return UT_FALSE;
			}

			indexAP = _chooseIndexAP(pf,fragOffset);
		}
	}
	else
	{
  	         // is existing fragment a field? If so do nothing 
	  // Or should we display a message to the user?
     
                if(pf->getField() != NULL)
		{
		       return UT_FALSE;
		}


		indexAP = _chooseIndexAP(pf,fragOffset);
	}
	
	PT_BlockOffset blockOffset = _computeBlockOffset(pfs,pf) + fragOffset;
	PX_ChangeRecord_Span * pcr = NULL;
	
	if (!_insertSpan(pf,bi,fragOffset,length,indexAP,pField))
		goto Finish;

	// note: because of coalescing, pf should be considered invalid at this point.
	
	// create a change record, add it to the history, and notify
	// anyone listening.

	pcr = new PX_ChangeRecord_Span(PX_ChangeRecord::PXT_InsertSpan,
								   dpos,indexAP,bi,length,
                                   blockOffset, pField);
	UT_ASSERT(pcr);

	if (_canCoalesceInsertSpan(pcr))
	{
		m_history.coalesceHistory(pcr);
		m_pDocument->notifyListeners(pfs,pcr);
		delete pcr;
	}
	else
	{
		m_history.addChangeRecord(pcr);
		m_pDocument->notifyListeners(pfs,pcr);
	}

	bSuccess = UT_TRUE;
	
Finish:
	if (bNeedGlob)
		endMultiStepGlob();
	
	return bSuccess;
}


UT_Bool pt_PieceTable::insertSpan_norec(PT_DocPosition dpos,
								  const UT_UCSChar * p,
								  UT_uint32 length, fd_Field * pField)
{
	// insert character data into the document at the given position.
  //No not throw a change record. 
  // We need this to getupdate fields working correctly

	UT_ASSERT(m_pts==PTS_Editing);
	// get the fragment at the given document position.
	
	pf_Frag * pf = NULL;
	PT_BlockOffset fragOffset = 0;
	UT_Bool bFound = getFragFromPosition(dpos,&pf,&fragOffset);
	UT_ASSERT(bFound);
    

	// append the text data to the end of the current buffer.

	PT_BufIndex bi;
	if (!m_varset.appendBuf(p,length,&bi))
		return UT_FALSE;

	UT_Bool bSuccess = UT_FALSE;
	pf_Frag_Strux * pfs = NULL;
	UT_Bool bFoundStrux = _getStruxFromFrag(pf,&pfs);
	UT_ASSERT(bFoundStrux);

	// we just did a getFragFromPosition() which gives us the
	// the thing *starting* at that position.  if we have a
	// fragment boundary at that position, it's sort of arbitrary
	// whether we treat this insert as a prepend to the one we just found
	// or an append to the previous one (when it's a text frag).
	// in the normal case, we want the Attr/Prop of a character
	// insertion to take the AP of the thing to the immediate
	// left (seems to be what MS-Word and MS-WordPad do).  It's also
	// useful when the user hits the BOLD button (without a)
	// selection) and then starts typing -- ideally you'd like
	// all of the text to have bold not just the first.  therefore,
	// we will see if we are on a text-text boundary and backup
	// (and thus appending) to the previous.

	UT_Bool bNeedGlob = UT_FALSE;
	PT_AttrPropIndex indexAP = 0;
	
	if ( (fragOffset==0) && (pf->getPrev()) )
	{
		UT_Bool bRightOfFmtMark = (pf->getPrev()->getType() == pf_Frag::PFT_FmtMark);
		if (bRightOfFmtMark)
		{
			// if we're just to the right of a _FmtMark, we want to replace
			// it with a _Text frag with the same attr/prop (we
			// only used the _FmtMark to remember a toggle format
			// before we had text for it).

			pf_Frag_FmtMark * pfPrevFmtMark = static_cast<pf_Frag_FmtMark *>(pf->getPrev());
			indexAP = pfPrevFmtMark->getIndexAP();

			if (_lastUndoIsThisFmtMark(dpos))
			{
				// if the last thing in the undo history is the insertion of this
				// _FmtMark, then let's remember the indexAP, do an undo, and then
				// insert the text.  this way the only thing remaining in the undo
				// is the insertion of this text (with no globbing around it).  then
				// a user-undo will undo all of the coalesced text back to this point
				// and leave the insertion point as if the original InsertFmtMark
				// had never happened.
				//
				// we don't allow consecutive FmtMarks, but the undo may be a
				// changeFmtMark and thus just re-change the mark frag rather
				// than actually deleting it.  so we loop here to get back to
				// the original insertFmtMark (this is the case if the user hit
				// BOLD then ITALIC then UNDERLINE then typed a character).

				do { undoCmd(); } while (_lastUndoIsThisFmtMark(dpos));
			}
			else
			{
				// for some reason, something else has happened to the document
				// since this _FmtMark was inserted (perhaps it was one that we
				// inserted when we did a paragraph break and inserted several
				// to remember the current inline formatting).
				//
				// here we have to do it the hard way and use a glob and an
				// explicit deleteFmtMark.  note that this messes up the undo
				// coalescing.  that is, if the user starts typing at this
				// position and then hits UNDO, we will erase all of the typing
				// except for the first character.  the second UNDO, will erase
				// the first character and restores the current FmtMark.  if the
				// user BACKSPACES instead of doing the second UNDO, both the
				// first character and the FmtMark would be gone.
				//
				// TODO decide if we like this...
				// NOTE this causes BUG#431.... :-)

				bNeedGlob = UT_TRUE;
				beginMultiStepGlob();
				_deleteFmtMarkWithNotify(dpos,pfPrevFmtMark,pfs,&pf,&fragOffset);
			}

			// we now need to consider pf invalid, since the fragment list may have
			// been coalesced as the FmtMarks were deleted.  let's recompute them
			// but with a few shortcuts.

			bFound = getFragFromPosition(dpos,&pf,&fragOffset);
			UT_ASSERT(bFound);
			bFoundStrux = _getStruxFromFrag(pf,&pfs);
			UT_ASSERT(bFoundStrux);

			// with the FmtMark now gone, we make a minor adjustment so that we
			// try to append text to the previous rather than prepend to the current.
			// this makes us consistent with other places in the code.

			if ( (fragOffset==0) && (pf->getPrev()) && (pf->getPrev()->getType() == pf_Frag::PFT_Text) && pf->getPrev()->getField()== NULL )
			{
				// append to the end of the previous frag rather than prepend to the current one.
				pf = pf->getPrev();
				fragOffset = pf->getLength();
			}
		}
		else if (pf->getPrev()->getType() == pf_Frag::PFT_Text && pf->getPrev()->getField()==NULL)
		{
			pf_Frag_Text * pfPrevText = static_cast<pf_Frag_Text *>(pf->getPrev());
			indexAP = pfPrevText->getIndexAP();

			// append to the end of the previous frag rather than prepend to the current one.
			pf = pf->getPrev();
			fragOffset = pf->getLength();
		}
		else
		{
		  // is existing fragment a field? If so do nothing 
		  // Or should we display a message to the user?
     
		        if(pf->getField() != NULL)
			{
				return UT_FALSE;
			}

			indexAP = _chooseIndexAP(pf,fragOffset);
		}
	}
	else
	{
  	         // is existing fragment a field? If so do nothing 
	  // Or should we display a message to the user?
     
                if(pf->getField() != NULL)
		{
		       return UT_FALSE;
		}


		indexAP = _chooseIndexAP(pf,fragOffset);
	}
	
	PT_BlockOffset blockOffset = _computeBlockOffset(pfs,pf) + fragOffset;
	PX_ChangeRecord_Span * pcr = NULL;
	
	if (!_insertSpan(pf,bi,fragOffset,length,indexAP,pField))
		goto Finish;

	// note: because of coalescing, pf should be considered invalid at this point.
	
	// create a change record, add it to the history, and notify
	// anyone listening.

	pcr = new PX_ChangeRecord_Span(PX_ChangeRecord::PXT_InsertSpan,
								   dpos,indexAP,bi,length,
                                   blockOffset, pField);
	UT_ASSERT(pcr);

	// Need this to update formatting
	m_pDocument->notifyListeners(pfs,pcr);
	// We've finished with this now so delete it

	delete pcr;
	bSuccess = UT_TRUE;
	
Finish:
	if (bNeedGlob)
		endMultiStepGlob();
	return bSuccess;
}

UT_Bool pt_PieceTable::_canCoalesceInsertSpan(PX_ChangeRecord_Span * pcrSpan) const
{
	// see if this record can be coalesced with the most recent undo record.

	UT_ASSERT(pcrSpan->getType() == PX_ChangeRecord::PXT_InsertSpan);

	PX_ChangeRecord * pcrUndo;
	if (!m_history.getUndo(&pcrUndo))
		return UT_FALSE;
	if (pcrSpan->getType() != pcrUndo->getType())
		return UT_FALSE;
	if (pcrSpan->getIndexAP() != pcrUndo->getIndexAP())
		return UT_FALSE;

	PX_ChangeRecord_Span * pcrUndoSpan = static_cast<PX_ChangeRecord_Span *>(pcrUndo);
	UT_uint32 lengthUndo = pcrUndoSpan->getLength();

	if ((pcrUndo->getPosition() + lengthUndo) != pcrSpan->getPosition())
		return UT_FALSE;

	PT_BufIndex biUndo = pcrUndoSpan->getBufIndex();
	PT_BufIndex biSpan = pcrSpan->getBufIndex();

	if (m_varset.getBufIndex(biUndo,lengthUndo) != biSpan)
		return UT_FALSE;

	return UT_TRUE;
}

PT_AttrPropIndex pt_PieceTable::_chooseIndexAP(pf_Frag * pf, PT_BlockOffset fragOffset)
{
	// decide what indexAP to give an insertSpan inserting at the given
	// position in the document [pf,fragOffset].
	// try to get it from the current fragment.

	if (pf->getType() == pf_Frag::PFT_FmtMark)
	{
		pf_Frag_FmtMark * pffm = static_cast<pf_Frag_FmtMark *>(pf);
		return pffm->getIndexAP();
	}
	
	if ((pf->getType() == pf_Frag::PFT_Text) && (fragOffset > 0))
	{
		// if we are inserting at the middle or end of a text fragment,
		// we take the A/P of this text fragment.

		pf_Frag_Text * pft = static_cast<pf_Frag_Text *>(pf);
		return pft->getIndexAP();
	}

	// we are not looking forward at a text fragment or
	// we are at the beginning of a text fragment.
	// look to the previous fragment to see what to do.
	
	pf_Frag * pfPrev = pf->getPrev();
	switch (pfPrev->getType())
	{
	case pf_Frag::PFT_Text:
		{
			// if we immediately follow another text fragment, we
			// take the A/P of that fragment.

			pf_Frag_Text * pftPrev = static_cast<pf_Frag_Text *>(pfPrev);
			return pftPrev->getIndexAP();
		}

	case pf_Frag::PFT_Strux:
		{
			// if we immediately follow a block (paragraph),
			// (and don't have a FmtMark (tested earlier) at
			// the beginning of the block), look to the right.

			if (pf->getType() == pf_Frag::PFT_Text)
			{
				// we take the A/P of this text fragment.

				pf_Frag_Text * pft = static_cast<pf_Frag_Text *>(pf);
				return pft->getIndexAP();
			}

			// we can't find anything, just use the default.
			
			return 0;
		}

	case pf_Frag::PFT_Object:
		{
			// if we immediately follow an object, then we may or may not
			// want to use the A/P of the object.  for an image, it is
			// probably not correct to use it (and we should probably use
			// the text to the right of the image).  for a field, it may
			// be a valid to use the A/P of the object.

			pf_Frag_Object * pfo = static_cast<pf_Frag_Object *>(pfPrev);
			switch (pfo->getObjectType())
			{
			case PTO_Image:
				return _chooseIndexAP(pf->getPrev(),pf->getPrev()->getLength());
				
			case PTO_Field:
				return pfo->getIndexAP();
				
			default:
				UT_ASSERT(0);
				return 0;
			}
		}

	case pf_Frag::PFT_FmtMark:
		{
			// TODO i'm not sure this is possible
			
			pf_Frag_FmtMark * pffm = static_cast<pf_Frag_FmtMark *>(pfPrev);
			return pffm->getIndexAP();
		}
		
	default:
		UT_ASSERT(0);
		return 0;
	}
}
