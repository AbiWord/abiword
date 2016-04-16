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


// insertSpan-related routined for class pt_PieceTable.
#include "ut_string_class.h"
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
#include "pp_Revision.h"
#include "pd_Document.h"

/****************************************************************/
/****************************************************************/


bool pt_PieceTable::insertSpan(PT_DocPosition dpos,
							   const UT_UCSChar * p,
							   UT_uint32 length, fd_Field * pField,
							   bool bAddChangeRec)
{
	if(bAddChangeRec && m_pDocument->isMarkRevisions())
	{
		PP_RevisionAttr Revisions(NULL);
		PP_PropertyVector ppRevAttrib;
		PP_PropertyVector ppRevProps;

		pf_Frag * pf = NULL;
		PT_BlockOffset fragOffset = 0;
		bool bFound = getFragFromPosition(dpos,&pf,&fragOffset);
		UT_return_val_if_fail( bFound, false );

		if(pf->getType() == pf_Frag::PFT_EndOfDoc)
			pf = pf->getPrev();

		UT_return_val_if_fail( pf, false );

		PT_AttrPropIndex indexAP = pf->getIndexAP();

		_translateRevisionAttribute(Revisions, indexAP, PP_REVISION_ADDITION, ppRevAttrib, ppRevProps, PP_NOPROPS, PP_NOPROPS);

		//return _realChangeSpanFmt(PTC_AddFmt, dpos, dpos + length, ppRevAttrib, ppRevProps);
		return _realInsertSpan(dpos, p, length, ppRevAttrib, ppRevProps, pField, bAddChangeRec);
	}
	else if(bAddChangeRec)
	{
		// When the revision marking is not on, we need to make sure
		// that the text does not get inserted with a leftover
		// revision attribute (e.g., if we are inserting it next to
		// revisioned text
		const char* name = "revision";
		PP_PropertyVector ppRevAttrib = {
			name, "",
		};

		const gchar * pRevision = NULL;

		// first retrive the fmt we have (_realChangeSpanFmt()) is
		// quite involved, so we want to avoid calling it, if we can)
		pf_Frag * pf1;
		PT_BlockOffset Offset1;

		if(!getFragFromPosition(dpos, &pf1, &Offset1))
			return false;

		const PP_AttrProp * pAP;
		if(_getSpanAttrPropHelper(pf1, &pAP))
		{
			const gchar * szStyleNameVal = NULL;
			pAP->getAttribute(PT_STYLE_ATTRIBUTE_NAME,szStyleNameVal);
			if(!pAP->getAttribute(name, pRevision))
			{
				return _realInsertSpan(dpos, p, length, PP_NOPROPS, PP_NOPROPS, pField, bAddChangeRec);
			}
			if(szStyleNameVal != NULL)
			{
				ppRevAttrib.push_back(PT_STYLE_ATTRIBUTE_NAME);
				ppRevAttrib.push_back(szStyleNameVal);
			}
			//if(!_realChangeSpanFmt(PTC_RemoveFmt, dpos, dpos+length, ppRevAttrib,NULL))
			//	return false;
			return _realInsertSpan(dpos, p, length, ppRevAttrib, PP_NOPROPS, pField, bAddChangeRec);
		}
		else
		{
			// no AP, this is probably OK
			UT_DEBUGMSG(("pt_PieceTable::insertSpan: no AP\n"));
			return _realInsertSpan(dpos, p, length, PP_NOPROPS, PP_NOPROPS, pField, bAddChangeRec);
		}
	}
	else
	{
		return _realInsertSpan(dpos, p, length, PP_NOPROPS, PP_NOPROPS, pField, bAddChangeRec);
	}
}


bool pt_PieceTable::_insertSpan(pf_Frag * pf,
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
		UT_ASSERT_HARMLESS(0);
		return false;

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
		UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
		return false;
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

				return true;
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

				return true;
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
					return true;
				}
			}
		}
	}

	// new text is not contiguous, we need to insert one or two new text
	// fragment(s) into the list.  first we construct a new text fragment
	// for the data that we inserted.

	pf_Frag_Text * pftNew = new pf_Frag_Text(this,bi,length,indexAP,pField);
	if (!pftNew)
		return false;

	if (fragOffset == 0)
	{
		// if change is at the beginning of the fragment, we insert a
		// single new text fragment before the one we found.

		m_fragments.insertFrag(pf->getPrev(),pftNew);
		return true;
	}

	UT_uint32 fragLen = pf->getLength();
	if (fragLen==fragOffset)
	{
		// if the change is after this fragment, we insert a single
		// new text fragment after the one we found.

		m_fragments.insertFrag(pf,pftNew);
		return true;
	}

	// if the change is in the middle of the fragment, we construct
	// a second new text fragment for the portion after the insert.

	UT_return_val_if_fail (pft,false);

	UT_uint32 lenTail = pft->getLength() - fragOffset;
	PT_BufIndex biTail = m_varset.getBufIndex(pft->getBufIndex(),fragOffset);
	pf_Frag_Text * pftTail = new pf_Frag_Text(this,biTail,lenTail,pft->getIndexAP(),pft->getField());
	if (!pftTail)
		return false;

	pft->changeLength(fragOffset);
	m_fragments.insertFrag(pft,pftNew);
	m_fragments.insertFrag(pftNew,pftTail);

	return true;
}

bool pt_PieceTable::_lastUndoIsThisFmtMark(PT_DocPosition dpos)
{
	// look backwards thru the undo from this point and see
	// if we have <InsertFmtMark>[<ChangeFmtMark>*]

	PX_ChangeRecord * pcr;
	UT_uint32 undoNdx = 0;

	while (1)
	{
		bool bHaveUndo = m_history.getNthUndo(&pcr,undoNdx);

		if (!bHaveUndo)
			return false;
		if (!pcr)
			return false;
		if (pcr->getPosition() != dpos)
			return false;

		switch (pcr->getType())
		{
		default:
			return false;
		case PX_ChangeRecord::PXT_InsertFmtMark:
			return true;
		case PX_ChangeRecord::PXT_ChangeFmtMark:
			undoNdx++;
			break;
		}
	}

	UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	return false;
}

bool pt_PieceTable::_realInsertSpan(PT_DocPosition dpos,
									const UT_UCSChar * p,
									UT_uint32 length,
									const PP_PropertyVector & attributes,
									const PP_PropertyVector & properties,
									fd_Field * pField,
									bool bAddChangeRec)
{
	// insert character data into the document at the given position.

	UT_return_val_if_fail (m_pts==PTS_Editing, false);

	// get the fragment at the given document position.

	pf_Frag * pf = NULL;
	PT_BlockOffset fragOffset = 0;
	bool bFound = getFragFromPosition(dpos,&pf,&fragOffset);
	UT_return_val_if_fail (bFound,false);


	// append the text data to the end of the current buffer.

	PT_BufIndex bi;
	if (!m_varset.appendBuf(p,length,&bi))
		return false;

	pf_Frag_Strux * pfs = NULL;
	bool bFoundStrux = _getStruxFromFrag(pf,&pfs);
	UT_return_val_if_fail (bFoundStrux,false);
	if(isEndFootnote((pf_Frag *)pfs))
	{
		bFoundStrux = _getStruxFromFragSkip((pf_Frag *) pfs,&pfs);
	}
	UT_return_val_if_fail (pfs,false);
	if(pfs->getStruxType() == PTX_EndFrame)
	{
		bFoundStrux = _getStruxFromFragSkip((pf_Frag *) pfs,&pfs);
	}
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

	bool bNeedGlob = false;
	PT_AttrPropIndex indexAP = 0;

	if ( (fragOffset==0) && (pf->getPrev()) )
	{
		bool bRightOfFmtMark = (pf->getPrev()->getType() == pf_Frag::PFT_FmtMark);
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

				bNeedGlob = true;
				beginMultiStepGlob();
				_deleteFmtMarkWithNotify(dpos,pfPrevFmtMark,pfs,&pf,&fragOffset);
			}

			// we now need to consider pf invalid, since the fragment list may have
			// been coalesced as the FmtMarks were deleted.  let's recompute them
			// but with a few shortcuts.

			bFound = getFragFromPosition(dpos,&pf,&fragOffset);
			UT_return_val_if_fail (bFound, false);

			bFoundStrux = _getStruxFromFrag(pf,&pfs);
			UT_return_val_if_fail (bFoundStrux,false);
			if(isEndFootnote((pf_Frag *)pfs))
			{
				bFoundStrux = _getStruxFromFragSkip((pf_Frag *)pfs,&pfs);
			}
			UT_return_val_if_fail (bFoundStrux, false);
			xxx_UT_DEBUGMSG(("Got FragStrux at Pos %d \n",pfs->getPos()));

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
			indexAP = _chooseIndexAP(pf,fragOffset);
			// PLAM: This is the list of field attrs that should not inherit
			// PLAM: to the span following a field.
			PP_PropertyVector pFieldAttrs = {
				"type", "",
				"param", "",
				"name", "",
				"endnote-id", ""
			};

			const PP_AttrProp * pAP = NULL;

			if (!getAttrProp(indexAP, &pAP))
				return false;

			if (pAP->areAnyOfTheseNamesPresent(pFieldAttrs, PP_NOPROPS))
			{
				// We do not want to inherit a char style from a field.
				pFieldAttrs.push_back("style");
				pFieldAttrs.push_back("");
				PP_AttrProp * pAPNew = pAP->cloneWithElimination(pFieldAttrs, PP_NOPROPS);
				if (!pAPNew)
					return false;
				pAPNew->markReadOnly();

				if (!m_varset.addIfUniqueAP(pAPNew, &indexAP))
					return false;
			}
		}
	}
	else
	{
		// is existing fragment a field? If so do nothing
		// Or should we display a message to the user?

		if(pf->getField() != NULL)
		{
		       return false;
		}

		indexAP = _chooseIndexAP(pf,fragOffset);
	}
	PT_BlockOffset blockOffset = _computeBlockOffset(pfs,pf) + fragOffset;
	PX_ChangeRecord_Span * pcr = NULL;

	if(!attributes.empty() || !properties.empty())
	{
		// we need to add the attrs and props passed to us ...
		PT_AttrPropIndex indexNewAP;
		bool bMerged;
		bMerged = m_varset.mergeAP(PTC_AddFmt, indexAP, attributes, properties, &indexNewAP, getDocument());
		UT_ASSERT_HARMLESS( bMerged );

		if(bMerged)
			indexAP = indexNewAP;
	}
	
	if (!_insertSpan(pf,bi,fragOffset,length,indexAP,pField))
	{
		if (bNeedGlob)
			endMultiStepGlob();
		return false;
	}

	// note: because of coalescing, pf should be considered invalid at this point.
	// create a change record, add it to the history, and notify
	// anyone listening.

	pcr = new PX_ChangeRecord_Span(PX_ChangeRecord::PXT_InsertSpan,
								   dpos,indexAP,bi,length,
								   blockOffset, pField);
	UT_return_val_if_fail (pcr, false);
	
	pcr->setDocument(m_pDocument);
	bool canCoalesce = _canCoalesceInsertSpan(pcr);
	if (!bAddChangeRec || (canCoalesce && !m_pDocument->isCoalescingMasked()))
	{
		if (canCoalesce)
			m_history.coalesceHistory(pcr);
		
		m_pDocument->notifyListeners(pfs,pcr);
		delete pcr;
	}
	else
	{
		m_history.addChangeRecord(pcr);
		m_pDocument->notifyListeners(pfs,pcr);
	}

	if (bNeedGlob)
		endMultiStepGlob();	
	return true;
}

bool pt_PieceTable::_canCoalesceInsertSpan(PX_ChangeRecord_Span * pcrSpan) const
{
	// see if this record can be coalesced with the most recent undo record.

	UT_return_val_if_fail (pcrSpan->getType() == PX_ChangeRecord::PXT_InsertSpan, false);

	PX_ChangeRecord * pcrUndo;
	if (!m_history.getUndo(&pcrUndo,true))
		return false;
	if (pcrSpan->getType() != pcrUndo->getType())
		return false;
	if (pcrSpan->getIndexAP() != pcrUndo->getIndexAP())
		return false;

	PX_ChangeRecord_Span * pcrUndoSpan = static_cast<PX_ChangeRecord_Span *>(pcrUndo);
	if((pcrUndoSpan->isFromThisDoc() != pcrSpan->isFromThisDoc()))
	   return false;

	UT_uint32 lengthUndo = pcrUndoSpan->getLength();

	if ((pcrUndo->getPosition() + lengthUndo) != pcrSpan->getPosition())
		return false;

	PT_BufIndex biUndo = pcrUndoSpan->getBufIndex();
	PT_BufIndex biSpan = pcrSpan->getBufIndex();

	if (m_varset.getBufIndex(biUndo,lengthUndo) != biSpan)
		return false;

	// "Coalescing not allowed across a save." - PL
	// So, if we're clean, make us dirty.
	if (!m_history.isDirty())
		return false;

	return true;
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
			case PTO_Math:
			case PTO_Embed:
				return pfo->getIndexAP();

			// TODO: determine what we want to do about these guys
			case PTO_Bookmark:
			case PTO_Hyperlink:
			case PTO_RDFAnchor:
				return 0;

			default:
				UT_ASSERT_HARMLESS(0);
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
		UT_ASSERT_HARMLESS(0);
		return 0;
	}
}
