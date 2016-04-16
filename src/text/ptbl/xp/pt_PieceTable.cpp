/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

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

#include <list>
#include "ut_types.h"
#include "ut_misc.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_growbuf.h"
#include "ut_std_map.h"
#include "pt_PieceTable.h"
#include "pf_Frag.h"
#include "pf_Frag_FmtMark.h"
#include "pf_Frag_Strux.h"
#include "pf_Frag_Strux_Block.h"
#include "pf_Frag_Strux_Section.h"
#include "pf_Frag_Text.h"
#include "pf_Frag_Object.h"
#include "pf_Fragments.h"
#include "px_ChangeRecord.h"
#include "px_CR_Span.h"
#include "px_CR_SpanChange.h"
#include "px_CR_Strux.h"
#include "pd_Style.h"
#include "px_CR_Glob.h"

// TODO: calculate this from pf_FRAG_STRUX_*_LENGTH instead?
#define pt_BOD_POSITION 2

/*****************************************************************/
/*****************************************************************/

pt_PieceTable::pt_PieceTable(PD_Document * pDocument)
  : m_pts(PTS_Create), 
	m_history(this),
	m_pDocument(pDocument),
    m_atomicGlobCount(0),
	m_bDoingTheDo(false),
	m_bDoNotTweakPosition(false),
	m_iXID(0),
	m_iCurCRNumber(0)
{

	setPieceTableState(PTS_Create);
	loading.m_indexCurrentInlineAP = 0;
}

pt_PieceTable::~pt_PieceTable()
{
	m_fragments.purgeFrags();
	UT_map_delete_all_second(m_hashStyles);
}

void pt_PieceTable::setPieceTableState(PTState pts)
{
	UT_return_if_fail (pts >= m_pts);

	if ((m_pts==PTS_Create) && (pts==PTS_Loading))
	{
		// transition from create to loading.
		// populate the builtin styles
		_loadBuiltinStyles();
	}

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

/*!
 * Use this for deleting unneeded strux during doc import. Particularly useful for importing
 * RTF.
 */
bool pt_PieceTable::deleteStruxNoUpdate(pf_Frag_Strux* sdh)
{
	const pf_Frag_Strux * pfs = sdh;
	UT_DEBUGMSG(("SEVIOR: deleting strux no update %p \n",sdh));
	pf_Frag * pf = pfs->getNext();
	if(pf != NULL && pf->getType() == pf_Frag::PFT_FmtMark)
	{
		getFragments().unlinkFrag(pf);
		delete pf;
	}
	getFragments().unlinkFrag(const_cast<pf_Frag_Strux*>(pfs));
	delete pfs;
	return true;
}


/*!
 * Use this for deleting unused sections of the document during import.
 * In Particular use this to remove unused headers/footers.
 */
bool pt_PieceTable::deleteFragNoUpdate(pf_Frag * pf)
{
	UT_DEBUGMSG(("SEVIOR: deleting frag no update %p \n",pf));
	getFragments().unlinkFrag(pf);
	delete pf;
	return true;
}

/*!
 * Itterate through the document to calculate the document size
 * Don't call this in production code. This is used only for recovery and 
 * testing purposes
 */
UT_sint32 pt_PieceTable::calcDocsize(void)
{
	UT_sint32 size = 0;
	pf_Frag * pf = getFragments().getFirst();
	while(pf && (pf->getType() !=  pf_Frag::PFT_EndOfDoc))
	{
		size += static_cast<UT_sint32>(pf->getLength());
		pf = pf->getNext();
	}
	UT_ASSERT(pf->getType() ==  pf_Frag::PFT_EndOfDoc);
	return size;
}

bool pt_PieceTable::createAndSendDocPropCR( const gchar ** pAtts, const gchar ** pProps)
{
	PT_AttrPropIndex indexAP = 0;
	PP_AttrProp * pAP = new PP_AttrProp();
	pAP->setAttributes(pAtts);
	pAP->setProperties(pProps);
	bool b = m_varset.addIfUniqueAP(pAP,&indexAP);
	PX_ChangeRecord * pcr= new PX_ChangeRecord(PX_ChangeRecord::PXT_ChangeDocProp,0,indexAP,0);
	const pf_Frag_Strux * pfStart = static_cast<pf_Frag_Strux *>(getFragments().getFirst());
	m_pDocument->notifyListeners(pfStart, pcr);
	delete pcr;
	return b;
}

bool pt_PieceTable::createAndSendCR(PT_DocPosition iPos, UT_sint32 iType,bool bSave,UT_Byte iGlob)
{
	PX_ChangeRecord::PXType cType = static_cast< PX_ChangeRecord::PXType>(iType);
  switch(cType)
    {
    case PX_ChangeRecord::PXT_InsertSpan:
    case PX_ChangeRecord::PXT_DeleteSpan:
    case PX_ChangeRecord::PXT_ChangeSpan:
    case PX_ChangeRecord::PXT_InsertStrux:
    case PX_ChangeRecord::PXT_DeleteStrux:
    case PX_ChangeRecord::PXT_ChangeStrux:
    case PX_ChangeRecord::PXT_InsertObject:
    case PX_ChangeRecord::PXT_ChangeObject:
    case PX_ChangeRecord::PXT_InsertFmtMark:
    case PX_ChangeRecord::PXT_DeleteFmtMark:
    case PX_ChangeRecord::PXT_ChangeFmtMark:
		{
			UT_DEBUGMSG(("CR already implemented \n"));
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
			return false;
		}
    case PX_ChangeRecord::PXT_GlobMarker:
	{
		PX_ChangeRecord * pcr= static_cast<PX_ChangeRecord *>(new PX_ChangeRecord_Glob(cType,iGlob));
		if(bSave)
		{
				m_history.addChangeRecord(pcr);
		}
		m_pDocument->notifyListeners(NULL, pcr);
		if(!bSave)
			delete pcr;
		return true;

	}
    case PX_ChangeRecord::PXT_ChangePoint:
    case PX_ChangeRecord::PXT_ListUpdate:
    case PX_ChangeRecord::PXT_StopList:
    case PX_ChangeRecord::PXT_UpdateField:
    case PX_ChangeRecord::PXT_RemoveList:
    case PX_ChangeRecord::PXT_UpdateLayout:
    {
		PX_ChangeRecord * pcr= new PX_ChangeRecord(cType,iPos, 0,0);
		if(bSave)
			{
				m_history.addChangeRecord(pcr);
			}
		m_pDocument->notifyListeners(NULL, pcr);
		if(!bSave)
			delete pcr;
		return true;
     }
    default:
		return false;
    }
}


/*!
 * Delete the single strux given in sdh and create and record a change record.
 */
bool pt_PieceTable::deleteStruxWithNotify(pf_Frag_Strux* sdh)
{
	pf_Frag_Strux * pfs = sdh;
	PT_DocPosition dpos = pfs->getPos();
	pf_Frag * pfEnd = NULL;
	UT_uint32 pfragOffsetEnd = 0;
	bool b = _deleteStruxWithNotify(dpos,pfs,&pfEnd,&pfragOffsetEnd,true);
	return b;
}

/*!
 * Delete The first FmtMark found at the position given.
 */
bool pt_PieceTable::deleteFmtMark(PT_DocPosition dpos)
{
	pf_Frag * pf = NULL;
	PT_BlockOffset pOffset= 0;
	getFragFromPosition(dpos,&pf,&pOffset);
	pf_Frag_FmtMark * pfm = NULL;
	if(pf->getType() == pf_Frag::PFT_FmtMark)
	{
		pfm = static_cast<pf_Frag_FmtMark *>(pf);
	}
	if(pf->getPrev() && pf->getPrev()->getType() == pf_Frag::PFT_FmtMark)
	{
		pfm = static_cast<pf_Frag_FmtMark *>(pf->getPrev());
	}
	if(pf->getNext() && pf->getNext()->getType() == pf_Frag::PFT_FmtMark)
	{
		pfm = static_cast<pf_Frag_FmtMark *>(pf->getNext());
	}
	if(pfm == NULL)
	{
		return false;
	}
	pf_Frag_Strux * pfs = NULL;
	if (!_getStruxFromFragSkip(pfm,&pfs))
		return false;
	pf_Frag * pfEnd = NULL;
	UT_uint32 fragOff = 0;
	bool b = _deleteFmtMarkWithNotify(dpos,pfm,pfs,&pfEnd,&fragOff);
	return b;
}
/*!
 * This method inserts a strux of type pts immediately before the sdh given.
 * Attributes of the strux can be optionally passed. This method does not throw
 * a change record and should only be used under exceptional circumstances to 
 * repair the piecetable during loading. It was necessary to import RTF tables.
 */
bool pt_PieceTable::insertStruxNoUpdateBefore(pf_Frag_Strux* sdh, PTStruxType pts,const gchar ** attributes )
{
	const pf_Frag_Strux * pfs = sdh;
	UT_DEBUGMSG(("SEVIOR: Inserting strux of type %d no update %p \n",pts,sdh));
//
// Create an indexAP
//
	PT_AttrPropIndex indexAP = pfs->getIndexAP();
	if(attributes)
	{
		PT_AttrPropIndex pAPIold = indexAP;
		bool bMerged = m_varset.mergeAP(PTC_AddFmt,pAPIold,PP_std_copyProps(attributes), PP_NOPROPS, &indexAP,getDocument());
		UT_UNUSED(bMerged);
		UT_ASSERT_HARMLESS(bMerged);
	}
//
// create a strux
//
	pf_Frag_Strux * pNewStrux = NULL;
	_createStrux(pts,indexAP,&pNewStrux);
//
// Insert it.
//
	pf_Frag * pfPrev = pfs->getPrev();
	UT_return_val_if_fail (pfPrev,false);

	m_fragments.insertFrag(pfPrev,pNewStrux);
	// insert frag in the embedded_strux list if needed
	if ((pts == PTX_EndFootnote) || (pts == PTX_EndEndnote) || (pts == PTX_EndAnnotation)) 
	{
		_insertNoteInEmbeddedStruxList(pNewStrux);
	}

#if 0
	m_pDocument->miniDump(sdh,8);
#endif
	return true;
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
	xxx_UT_DEBUGMSG(("Unlink frag %x of type %d \n",pf,pf->getType()));
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
	UT_ASSERT(pp && (pp->getNext() != pf));
}

bool pt_PieceTable::_struxHasContent(pf_Frag_Strux * pfs) const
{
	// return true iff the paragraph has content (text).

	return (pfs->getNext() && (pfs->getNext()->getType() == pf_Frag::PFT_Text));
}

bool  pt_PieceTable::_struxIsEmpty(pf_Frag_Strux * pfs) const
{
	if(pfs->getNext() == NULL)
	{
		return true;
	}
	pf_Frag * pf = pfs->getNext();
	if(pf->getType() == pf_Frag::PFT_EndOfDoc)
	{
		return true;
	}
	if(pf->getType() != pf_Frag::PFT_Strux)
	{
		return false;
	}
	pf_Frag_Strux * pfsNext = static_cast<pf_Frag_Strux *>(pfs->getNext());
	if(isFootnote(pfsNext))
	{
		return false;
	}
	return true;
}

bool pt_PieceTable::getAttrProp(PT_AttrPropIndex indexAP, const PP_AttrProp ** ppAP) const
{
	UT_return_val_if_fail (ppAP,false);

	const PP_AttrProp * pAP = m_varset.getAP(indexAP);
	if (!pAP)
		return false;

	*ppAP = pAP;
	return true;
}

bool pt_PieceTable::_getSpanAttrPropHelper(pf_Frag * pf, const PP_AttrProp ** ppAP) const
{
	switch (pf->getType())
	{
	case pf_Frag::PFT_FmtMark:
	case pf_Frag::PFT_Text:
	case pf_Frag::PFT_Object:
		{	
			const PP_AttrProp * pAP = m_varset.getAP(pf->getIndexAP());
			if (pAP)
			{
				*ppAP = pAP;
				return true;
			}
			else
			{
				UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
				return false;
			}
		}

	case pf_Frag::PFT_Strux:
	case pf_Frag::PFT_EndOfDoc:
	default:
		{
			*ppAP = NULL;
		}
	}
	return false;
}


bool pt_PieceTable::getSpanAttrProp(pf_Frag_Strux* sdh, UT_uint32 offset, bool bLeftSide,
									   const PP_AttrProp ** ppAP) const
{
	// return the AP for the text at the given offset from the given strux.
	// offset zero now refers to the first character in the block, so adding
	// fl_BLOCK_STRUX_OFFSET to the offset in the call is no longer necessary.

	UT_return_val_if_fail (sdh,false);
	UT_return_val_if_fail (ppAP,false);

	const pf_Frag * pf = sdh;
	UT_return_val_if_fail (pf->getType() == pf_Frag::PFT_Strux,false);
	const pf_Frag_Strux * pfsBlock = sdh;
	
	// This assert is incorrect; blocks that are inserted inside a TOC use sdh of the TOC section
	// UT_return_val_if_fail (pfsBlock->getStruxType() == PTX_Block,false);
	UT_return_val_if_fail (pfsBlock->getStruxType() == PTX_Block || pfsBlock->getStruxType() == PTX_SectionTOC,false);
	
	UT_uint32 cumOffset = 0;
	UT_uint32 cumEndOffset = 0;
	pf_Frag * pfTemp = NULL;
	for (pfTemp=pfsBlock->getNext(); (pfTemp); cumOffset=cumEndOffset, pfTemp=pfTemp->getNext())
	{
		cumEndOffset = cumOffset+pfTemp->getLength();

		if (offset > cumEndOffset)		// the place we want is way past the end of pfTemp,
			continue;					// so keep searching.

		if (offset == cumOffset)		// there's a frag boundary exactly where we want. pfTemp is to our right.
		{
			// FmtMarks have length zero, so we have to see what side of the position the caller wants.
			if ((pfTemp->getType()==pf_Frag::PFT_FmtMark) && (!bLeftSide))
				continue;				// go round again and get thing to the right

			return _getSpanAttrPropHelper(pfTemp,ppAP);
		}

		UT_return_val_if_fail (offset > cumOffset,false);

		if (offset == cumEndOffset)		// there's a frag boundary exactly where we want. pfTemp is to our left.
		{
			// FmtMarks have length zero, so we advance to put it to our left and then decide what to do
			if (!bLeftSide || (pfTemp->getNext() && (pfTemp->getNext()->getType()==pf_Frag::PFT_FmtMark)))
				continue;				// return the next one on the next iteration
			// If we are just after a footnote or an endnote, we move to the right fragment
			if (isEndFootnote(pfTemp) && pfTemp->getNext())
			{
				pfTemp = pfTemp->getNext();
			}
			return _getSpanAttrPropHelper(pfTemp,ppAP);
		}

		UT_return_val_if_fail (offset < cumEndOffset,false);

		// the place we want is inside of a fragment, so just use it.
		return _getSpanAttrPropHelper(pfTemp,ppAP);
	}

	*ppAP = NULL;
	return false;
}

#if 0
// I will leave the code here for now to aid in debugging any problems
// with the new iterator (should there be any, that is) Tomas, Nov 15, 2003
bool pt_PieceTable::getSpanPtr(pf_Frag_Strux* sdh, UT_uint32 offset,
								  const UT_UCSChar ** ppSpan, UT_uint32 * pLength) const
{
	// note: offset zero refers to the strux.  the first character is at
	// note: (0 + pfsBlock->getLength()).

	*ppSpan = NULL;
	*pLength = 0;

	const pf_Frag * pf = sdh;
	UT_return_val_if_fail (pf->getType() == pf_Frag::PFT_Strux,false);
	const pf_Frag_Strux * pfsBlock = sdh;
	UT_return_val_if_fail (pfsBlock->getStruxType() == PTX_Block,false);
	xxx_UT_DEBUGMSG(("getSpanPtr: Requested offset %d \n",offset));
	
	UT_uint32 cumOffset = pf->getLength();
	for (pf_Frag * pfTemp=pfsBlock->getNext(); (pfTemp); pfTemp=pfTemp->getNext())
	{
		xxx_UT_DEBUGMSG(("getSpanPtr: offset %d cumOffset %d \n",offset,cumOffset));
		if (offset == cumOffset)
		{
			if (pfTemp->getType() == pf_Frag::PFT_FmtMark)
				continue;
			if(isFootnote(pfTemp) || isEndFootnote(pfTemp))
			{
				cumOffset += pfTemp->getLength();
				continue;
			}
			if (pfTemp->getType() != pf_Frag::PFT_Text)
			{
				xxx_UT_DEBUGMSG(("getSpanPtr: Error 1 offset %d cumOffset %d \n",offset,cumOffset));
//				UT_ASSERT_HARMLESS(0);
				return false;
			}

			pf_Frag_Text * pfText = static_cast<pf_Frag_Text *> (pfTemp);
			*ppSpan = getPointer(pfText->getBufIndex());
			*pLength = pfText->getLength();
			return true;
		}
		if (offset < cumOffset+pfTemp->getLength())
		{
			if(isFootnote(pfTemp) || isEndFootnote(pfTemp))
			{
				cumOffset += pfTemp->getLength();
				continue;
			}
			if (pfTemp->getType() != pf_Frag::PFT_Text)
			{
				xxx_UT_DEBUGMSG(("getSpanPtr: Error 2 offset %d cumOffset %d \n",offset,cumOffset));
				return false;
			}
			pf_Frag_Text * pfText = static_cast<pf_Frag_Text *> (pfTemp);
			const UT_UCSChar * p = getPointer(pfText->getBufIndex());
			UT_uint32 delta = offset - cumOffset;
			*ppSpan = p + delta;
			*pLength = pfText->getLength() - delta;
			return true;
		}

		cumOffset += pfTemp->getLength();
	}
	xxx_UT_DEBUGMSG(("getSpanPtr: Error 3 offset %d cumOffset %d \n",offset,cumOffset));
	return false;
}
#endif

PD_Document * pt_PieceTable::getDocument(void)
{
        return m_pDocument;
}

/*!
  Copy paragraph (block) into buffer
  \param sdh Paragraph to copy
  \retval pgb Buffer where text should be copied to
  \return Always returns true

  Copy the contents (unicode character data) of the paragraph (block)
  into the growbuf given.  We append the content onto the growbuf.
*/
bool pt_PieceTable::getBlockBuf(pf_Frag_Strux* sdh,
                                   UT_GrowBuf * pgb) const
{
    UT_return_val_if_fail (pgb,false);

    const pf_Frag * pf = sdh;
    UT_return_val_if_fail(pf->getType() == pf_Frag::PFT_Strux, false);
    const pf_Frag_Strux * pfsBlock = sdh;
    UT_return_val_if_fail(pfsBlock->getStruxType() == PTX_Block, false);

    UT_uint32 bufferOffset = pgb->getLength();

    pf_Frag * pfTemp = pfsBlock->getNext();
	UT_sint32 countFoots = 0;
    while (pfTemp)
    {
        switch (pfTemp->getType())
        {
        default:
            UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
        case pf_Frag::PFT_Strux:
//
// Have to deal with embedded section struxes like Footnotes. We do this by
// filling the block buffer with the content contained until we find an
// end of embedded container. By placing zero's and content as expected in
// the block buffer we match the situation in fl_BlockLayout.
//
		{
			UT_GrowBufElement valz = 0;
            bool bAppended;
			//
			// Deal with TOC's. Since TOC's are always placed right before
			// the end of the paragraph we terminate the fill here.
			//
			pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pfTemp);
			if(pfs->getStruxType() == PTX_SectionTOC)
			{
				pfTemp = NULL;
				break;
			}
			if(isFootnote(pfTemp))
			{
				countFoots++;
				bAppended = pgb->ins(bufferOffset,&valz,1);
				UT_return_val_if_fail (bAppended,false);
				bufferOffset++;
				pfTemp = pfTemp->getNext();
				break;
			}
			if(isEndFootnote(pfTemp))
			{
				countFoots--;
				if(countFoots < 0)
				{
					pfTemp = NULL;
					break;
				}
				bAppended = pgb->ins(bufferOffset,&valz,1);
				UT_return_val_if_fail (bAppended,false);
				bufferOffset++;
				pfTemp = pfTemp->getNext();
				break;
			}
			if(countFoots>0)
			{
				bAppended = pgb->ins(bufferOffset,&valz,1);
				UT_return_val_if_fail (bAppended,false);
				bufferOffset++;
				pfTemp = pfTemp->getNext();
				break;
			}
			pfTemp = NULL;
			break;
		}
        case pf_Frag::PFT_EndOfDoc:
            pfTemp = NULL;
            break;

        case pf_Frag::PFT_FmtMark:
            pfTemp = pfTemp->getNext();
            break;

        case pf_Frag::PFT_Text:
        {
            pf_Frag_Text * pft = static_cast<pf_Frag_Text *>(pfTemp);
            const UT_UCSChar * pSpan = getPointer(pft->getBufIndex());
            UT_uint32 length = pft->getLength();

            bool bAppended;
            bAppended = pgb->ins(bufferOffset,reinterpret_cast<const UT_GrowBufElement*>(pSpan),length);
            UT_return_val_if_fail (bAppended,false);

            bufferOffset += length;
        }
        pfTemp = pfTemp->getNext();
        break;

        case pf_Frag::PFT_Object:
        {
            /*
              TODO investigate this....  Now *here* is a seriously
              questionable fragment of code.  :-) We can't let
              getBlockBuf halt on a block when it finds an inline
              object.  However, we can't very well sensibly store an
              inline object in a UNICODE character.  So, we dump
              USC_BLOCK in its place, to preserve the integrity of the
              buffer.  Obviously, those codes aren't useful, but at
              least the app doesn't crash, and the rest of the text in
              the block is safely stored in the buffer in the proper
              location.

              The UCS_ABI_OBJECT used to be defined as a space, but
              that caused selection code to fail for fields since the
              code would look for the beginning of a word, ignoring
              spaces. Now the UCS_ABI_OBJECT is instead defined as an
              alpha character. Doesn't really matter since it'll never
              be used for anything but limit checking anyway. See bug
              #223 for details.
			*/

            UT_uint32 length = pfTemp->getLength();

            // TODO investigate appending the SPACES directly to
            // TODO the pgb.  **or** investigate the cost of this
            // TODO g_try_malloc and what happens when it fails....

            UT_UCSChar* pSpaces = new UT_UCSChar[length];
            for (UT_uint32 i=0; i<length; i++)
            {
                pSpaces[i] = UCS_ABI_OBJECT;
            }
            bool bAppended;
            bAppended = pgb->ins(bufferOffset, reinterpret_cast<UT_GrowBufElement*>(pSpaces), length);
            delete[] pSpaces;
            UT_return_val_if_fail (bAppended,false);

            bufferOffset += length;
        }
        pfTemp = pfTemp->getNext();
        break;
        }
    }

    UT_return_val_if_fail (bufferOffset == pgb->getLength(),false);
    return true;
}

PT_DocPosition pt_PieceTable::getPosEnd()
{
    PT_DocPosition ret = 0;
    getBounds( true, ret );
    return ret;
}


bool pt_PieceTable::getBounds(bool bEnd, PT_DocPosition & docPos) const
{
	// be optimistic
	bool res = true;

	if (!bEnd)
	{
		docPos = pt_BOD_POSITION;
	}
	else
	{
		docPos = m_fragments.getLast()->getPos()+m_fragments.getLast()->getLength();
	}
	return res;
}

PT_DocPosition pt_PieceTable::getStruxPosition(pf_Frag_Strux* sdh) const
{
	// return absolute document position of the given handle.

	const pf_Frag * pfToFind = sdh;

	return getFragPosition(pfToFind);
}

void pt_PieceTable::deleteHdrFtrStrux(pf_Frag_Strux * pfs)
{
	UT_return_if_fail( pfs );
	
	if(m_pDocument->isMarkRevisions())
	{
		const pf_Frag * pfFrag = pfs;
		PT_DocPosition dpos1 = getFragPosition(pfFrag);

		pfFrag = pfFrag->getNext();

		while(pfFrag)
		{
			if((pfFrag->getType() == pf_Frag::PFT_EndOfDoc) ||
			   (pfFrag->getType() == pf_Frag::PFT_Strux &&
				static_cast<const pf_Frag_Strux*>(pfFrag)->getStruxType() == PTX_SectionHdrFtr))
			{
				break;
			}

			pfFrag = pfFrag->getNext();
		}

		// there should always be at least EndOfDoc fragment !!!
		UT_return_if_fail( pfFrag );
		
		PT_DocPosition dpos2 = getFragPosition(pfFrag);

		UT_uint32 iRealDelete = 0;
		deleteSpan(dpos1, dpos2, NULL, iRealDelete, true /*delete table struxes*/, false /* don't glob */);
	}
	else
	{
		const PP_AttrProp * pAP = NULL;
		UT_return_if_fail(pfs->getStruxType()==PTX_SectionHdrFtr);
		pf_Frag_Strux_SectionHdrFtr * pfHdr = static_cast<pf_Frag_Strux_SectionHdrFtr *>(pfs);

		if(!getAttrProp(pfHdr->getIndexAP(),&pAP) || !pAP)
			return;

		const gchar * pszHdrId;
		if(!pAP->getAttribute("id", pszHdrId) || !pszHdrId)
			return;

		const gchar * pszHdrType;
		if(!pAP->getAttribute("type", pszHdrType) || !pszHdrType)
			return;

		_realDeleteHdrFtrStrux(pfs);
		_fixHdrFtrReferences(pszHdrType, pszHdrId);
	}
	
}

void pt_PieceTable::_realDeleteHdrFtrStrux(pf_Frag_Strux * pfs)
{
	_deleteHdrFtrStruxWithNotify(pfs);
}

PT_DocPosition pt_PieceTable::getFragPosition(const pf_Frag * pfToFind) const
{
	return  pfToFind->getPos();
}


bool pt_PieceTable::getFragFromPosition(PT_DocPosition docPos,
										   pf_Frag ** ppf,
										   PT_BlockOffset * pFragOffset) const
{
	// return the frag at the given doc position.
//
// Sevior do a binary search here now
//
	pf_Frag * pfLast = m_fragments.findFirstFragBeforePos(docPos);
	if(pfLast)
	{
		xxx_UT_DEBUGMSG(("_findFrag: docPos %d pfLast->getPos %d pfLast->getLength %d \n",docPos,pfLast->getPos(),pfLast->getLength()));
		while(pfLast->getNext() && docPos >= (pfLast->getPos() + pfLast->getLength()))
		{
			xxx_UT_DEBUGMSG(("_findFrag: docPos %d pfLast->getPos %d pfLast->getLength %d \n",docPos,pfLast->getPos(),pfLast->getLength()));
			pfLast = pfLast->getNext();
		}
		xxx_UT_DEBUGMSG(("_findFrag: docPos %d pfLast->getPos %d pfLast->getLength %d offset %d Frag Type %d \n",docPos,pfLast->getPos(),pfLast->getLength(),docPos - pfLast->getPos(),pfLast->getType()));
		if (pFragOffset)
			*pFragOffset = docPos - pfLast->getPos();
		*ppf = pfLast;       
		return true;
	}

	UT_return_val_if_fail (pfLast,false);
	UT_return_val_if_fail (pfLast->getType() == pf_Frag::PFT_EndOfDoc,false);

	return true;
}
	//  PT_DocPosition sum = 0;
//  	pfLast = NULL;
//  	pf_Frag * pf = NULL;
//  	for (pf = m_fragments.getFirst(); (pf); pf=pf->getNext())
//  	{
//  		if ((docPos >= sum) && (docPos < sum+pf->getLength()))
//  		{
//  			*ppf = pf;
//  			if (pFragOffset)
//  				*pFragOffset = docPos - sum;

//  			// a FmtMark has length zero.  we don't want to find it
//  			// in this loop -- rather we want the thing just past it.

//  			UT_ASSERT(pf->getType() != pf_Frag::PFT_FmtMark);

//  			return true;
//  		}

//  		sum += pf->getLength();
//  		pfLast = pf;
//  	}

//  	// if we fall out of the loop, we didn't have a node
//  	// at or around the document position requested.
//  	// since we now have an EOD fragment, we should not
//  	// ever see this -- unless the caller sends a bogus
//  	// doc position.

//  	UT_ASSERT(pfLast);
//  	UT_ASSERT(pfLast->getType() == pf_Frag::PFT_EndOfDoc);

//  	// TODO if (docPos > sum) we should probably complain...

//  	*ppf = pfLast;
//  	if (pFragOffset)
//  		*pFragOffset = docPos - sum;


//  	return true;
//  }

bool pt_PieceTable::getFragsFromPositions(PT_DocPosition dPos1, PT_DocPosition dPos2,
											 pf_Frag ** ppf1, PT_BlockOffset * pOffset1,
											 pf_Frag ** ppf2, PT_BlockOffset * pOffset2) const
{
	// compute the (fragment,offset) pairs for each position given.

	UT_return_val_if_fail (dPos1 <= dPos2,false);
	UT_return_val_if_fail (ppf1,false);
	UT_return_val_if_fail (pOffset1,false);

	// the first set has to be done the hard way.

	if (!getFragFromPosition(dPos1,ppf1,pOffset1))
		return false;

	// now get the second set relative to the first.

	PT_DocPosition deltaPos = dPos2 - dPos1;
	PT_BlockOffset offset = *pOffset1;
	pf_Frag * pf = *ppf1;
	UT_uint32 length = pf->getLength();
	while (offset+deltaPos >= length)
	{
		deltaPos -= (length - offset);
		offset = 0;
		if (pf->getType() == pf_Frag::PFT_EndOfDoc)
			break;						// TODO if we haven't quite reached dPos2, we should probably complain...
		pf = pf->getNext();
		if(pf == NULL)
		{
			return false;
		}
		length = pf->getLength();
	}

	// a FmtMark has length zero.  we don't want to find it here.
	// rather we want the thing to the right of it.
	UT_return_val_if_fail (pf->getType() != pf_Frag::PFT_FmtMark,false);

	if (ppf2)
		*ppf2 = pf;
	if (pOffset2)
		*pOffset2 = offset+deltaPos;
	return true;
}

bool pt_PieceTable::getStruxFromPosition(PL_ListenerId listenerId,
											PT_DocPosition docPos,
											fl_ContainerLayout* * psfh) const
{
	// return the SFH of the last strux immediately prior to
	// the given absolute document position.

	pf_Frag_Strux * pfs = NULL;
	if (!_getStruxFromPosition(docPos,&pfs))
		return false;

	*psfh = pfs->getFmtHandle(listenerId);
	return true;
}

bool pt_PieceTable::getStruxOfTypeFromPosition(PL_ListenerId listenerId,
												  PT_DocPosition docPos,
												  PTStruxType pts,
												  fl_ContainerLayout* * psfh) const
{
	// return the SFH of the last strux of the given type
	// immediately prior to the given absolute document position.

	pf_Frag_Strux * pfs = NULL;
	if (!_getStruxOfTypeFromPosition(docPos,pts,&pfs))
		return false;

	*psfh = pfs->getFmtHandle(listenerId);
	return true;
}
///
/// return the SDH of the last strux of the given type
/// immediately prior to the given absolute document position.
///
bool pt_PieceTable::getStruxOfTypeFromPosition( PT_DocPosition docPos,
						   PTStruxType pts,
						   pf_Frag_Strux* * sdh) const
{
	return _getStruxOfTypeFromPosition(docPos,pts,sdh);
}

bool pt_PieceTable::isEndFootnote(pf_Frag * pf) const
{
	if(pf && (pf->getType() == pf_Frag::PFT_Strux))
	{
		pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf);
		if((pfs->getStruxType() == PTX_EndFootnote) || (pfs->getStruxType() == PTX_EndEndnote) || (pfs->getStruxType() == PTX_EndTOC) || (pfs->getStruxType() == PTX_EndAnnotation))
		{
			return true;
		}
	}
	return false;
}


bool pt_PieceTable::isFootnote(pf_Frag * pf) const
{
	if(pf && (pf->getType() == pf_Frag::PFT_Strux))
	{
		pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf);
		if((pfs->getStruxType() == PTX_SectionFootnote) || (pfs->getStruxType() == PTX_SectionEndnote) || (pfs->getStruxType() == PTX_SectionTOC) || (pfs->getStruxType() == PTX_SectionAnnotation))
		{
			return true;
		}
	}
	return false;
}


bool pt_PieceTable::isInsideFootnote(PT_DocPosition dpos, pf_Frag ** pfBegin) const
{
	// return true if pos is inside a footnote, an endnote or an annotation.
	// the address of the footnote (endnote or annotation) pfs_strux is 
	// returned in pfBegin if pfBegin is not NULL
	if(m_embeddedStrux.empty())
	{
		return false;
	}

	std::list<embeddedStrux>::const_iterator it;
	it = m_embeddedStrux.begin();
	for (it = m_embeddedStrux.begin(); it != m_embeddedStrux.end(); ++it)
	{
		if ((*it).endNote->getPos() > dpos)
		{
			if ((*it).beginNote->getPos() < dpos)
			{
				if (pfBegin)
				{
					*pfBegin = (*it).beginNote;
				}
				return true;
			}
			break;
		}
	}
	return false;
}


bool  pt_PieceTable::hasEmbedStruxOfTypeInRange(PT_DocPosition posStart, PT_DocPosition posEnd, 
												PTStruxType iType) const
{
	if(m_embeddedStrux.empty())
	{
		return false;
	}

	std::list<embeddedStrux>::const_iterator it;
	it = m_embeddedStrux.begin();
	for (it = m_embeddedStrux.begin(); it != m_embeddedStrux.end(); ++it)
	{
		if ((*it).type != iType)
		{
			continue;
		}
		if ((*it).beginNote->getPos() > posStart)
		{
			// if endNote->getPos() > posEnd, there are no notes inside the position range as
			// m_embeddedStrux is ordered by position.
			return ((*it).endNote->getPos() < posEnd);
		}
	}
	return false;	
}


bool pt_PieceTable::_getStruxFromPosition(PT_DocPosition docPos,
											 pf_Frag_Strux ** ppfs,
                                              bool bSkipFootnotes) const
{
	// return the strux fragment immediately prior (containing)
	// the given absolute document position.  If bSkip set, skip past
    // Footnote struxes.
	xxx_UT_DEBUGMSG(("_getStruxFromPosition bSkipFootnotes %d initial pos %d \n",bSkipFootnotes,docPos));
	UT_sint32 countEndFootnotes = 0;
	pf_Frag * pfFirst = m_fragments.findFirstFragBeforePos( docPos);
	xxx_UT_DEBUGMSG(("Frag found %x pos %d \n",pfFirst,pfFirst->getPos()));
	if(isEndFootnote(pfFirst))
		countEndFootnotes++;
	xxx_UT_DEBUGMSG(("Frag found, count endfootnotes %d  \n",countEndFootnotes));

	while(pfFirst && pfFirst->getPrev() && pfFirst->getPos() >= docPos)
	{
		pfFirst = pfFirst->getPrev();
		if (isFootnote(pfFirst))
			countEndFootnotes--;
		else if(isEndFootnote(pfFirst))
			countEndFootnotes++;

		xxx_UT_DEBUGMSG(("countEndNotes 1 %d \n",countEndFootnotes));
	}
	while(pfFirst && pfFirst->getPrev() && 
		  (pfFirst->getType() != pf_Frag::PFT_Strux || 
		   (bSkipFootnotes && ((countEndFootnotes > 0) || isFootnote(pfFirst) || isEndFootnote(pfFirst)))))
	{
		pfFirst = pfFirst->getPrev();
		xxx_UT_DEBUGMSG(("Frag found %x pos %d \n",pfFirst,pfFirst->getPos()));
		if(pfFirst == NULL)
		{
			break;
		}
		if(isFootnote(pfFirst))
			countEndFootnotes--;
		else if(isEndFootnote(pfFirst))
			countEndFootnotes++;
		xxx_UT_DEBUGMSG(("countEndNotes 2 %d \n",countEndFootnotes));
	}
	xxx_UT_DEBUGMSG(("countEndNotes final %d \n",countEndFootnotes));
  	pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *> (pfFirst);
  	*ppfs = pfs;
	return pfs != NULL;
}


pf_Frag_Strux* pt_PieceTable::getBlockFromPosition(PT_DocPosition pos) const
{
	pf_Frag_Strux* pfs = _getBlockFromPosition(pos);
    return pfs;
}


/**
 * Get the PTX_Block that contains the given document position. Note that
 * pos might itself point right at a PTX_Block in which case that is the block
 * that is returned. This might return null if there is no containing block
 */
pf_Frag_Strux* pt_PieceTable::_getBlockFromPosition(PT_DocPosition pos) const
{
    pf_Frag* pf;
    PT_BlockOffset offset;
    pf_Frag_Strux* ret = 0;
    
    if(!getFragFromPosition( pos, &pf, &offset ))
    {
        return ret;
    }

    // if the fragment right at pos is a block, return it.
    if( pf_Frag_Strux* pfs = tryDownCastStrux( pf, PTX_Block ))
    {
        return pfs;
    }
    // otherwise search backwards for the block.
    if(!_getStruxOfTypeFromPosition( pos, PTX_Block, &ret ))
    {
        return 0;
    }
    return ret;
    
}

bool pt_PieceTable::_getStruxOfTypeFromPosition(PT_DocPosition dpos,
												   PTStruxType pts,
												   pf_Frag_Strux ** ppfs) const
{
	// return the strux fragment of the given type containing
	// the given absolute document position.
	UT_return_val_if_fail (ppfs, false);
	xxx_UT_DEBUGMSG(("_getStruxOfTypeFromPosition: looking for type %d \n",pts));
	*ppfs = NULL;

	pf_Frag_Strux * pfs = NULL;
	bool wantFootNoteType = (pts == PTX_EndFootnote || pts == PTX_SectionFootnote || pts == PTX_EndEndnote || pts == PTX_SectionEndnote || pts == PTX_SectionAnnotation || pts == PTX_EndAnnotation || pts == PTX_SectionTOC || pts == PTX_EndTOC);

	if (!_getStruxFromPosition(dpos,&pfs, !wantFootNoteType))
		return false;

	PTStruxType pfsType = pfs->getStruxType();
	xxx_UT_DEBUGMSG(("Got strux type %d \n",pfs->getStruxType()));
	if (pfsType == pts || (pts == PTX_Section && pfsType == PTX_SectionHdrFtr)
		|| (pts == PTX_SectionFootnote && pfsType == PTX_SectionFootnote) 
		|| (pts == PTX_SectionAnnotation && pfsType == PTX_SectionAnnotation) 
		|| (pts == PTX_SectionEndnote && pfsType == PTX_SectionEndnote) 
		|| (pts == PTX_SectionTable && pfsType == PTX_SectionTable) 
		|| (pts == PTX_SectionCell && pfsType == PTX_SectionCell) 
		|| (pts == PTX_EndTable && pfsType == PTX_EndTable) 
		|| (pts == PTX_EndCell && pfsType == PTX_EndCell) 
		|| (pts == PTX_SectionTOC && pfsType == PTX_SectionTOC)  )		// is it of the type we want
	{
		*ppfs = pfs;
		return true;
	}

	// if not, we walk backwards thru the list and try to find it.
	UT_sint32 numEndTable = 0;
	for (pf_Frag * pf=pfs; (pf); pf=pf->getPrev())
		if (pf->getType() == pf_Frag::PFT_Strux)
		{
			pf_Frag_Strux * pfsTemp = NULL;
			if(!wantFootNoteType && isEndFootnote(pf))
			{
				_getStruxFromFragSkip(pf,&pfsTemp);
			}
			else
			{
				pfsTemp = static_cast<pf_Frag_Strux *>(pf);
			}
			UT_return_val_if_fail (pfsTemp, false);
			if(pfsTemp->getStruxType() == PTX_EndTable)
			{
				numEndTable++;
			}
			else if(pfsTemp->getStruxType() == PTX_SectionTable)
			{
				numEndTable--;
			}
			if (pfsTemp->getStruxType() == pts || (pts == PTX_Section && pfsTemp->getStruxType() == PTX_SectionHdrFtr) || (pts == PTX_SectionFootnote && pfsTemp->getStruxType() == PTX_SectionFootnote) || (pts == PTX_EndFootnote && pfsTemp->getStruxType() == PTX_EndFootnote) || (pts == PTX_SectionEndnote && pfsTemp->getStruxType() == PTX_SectionEndnote) || (pts == PTX_EndEndnote && pfsTemp->getStruxType() == PTX_EndEndnote) || (pts == PTX_SectionTOC && pfsTemp->getStruxType() == PTX_SectionTOC) || (pts == PTX_EndTOC && pfsTemp->getStruxType() == PTX_EndTOC))	// did we find it
			{
				if(((numEndTable < 0) && (pfsTemp->getStruxType()==PTX_SectionTable)) || (numEndTable == 0 && (pfsTemp->getStruxType()!=PTX_SectionTable)))
				{
					*ppfs = pfsTemp;
					return true;
				}
				else if(pfsTemp->getStruxType() != PTX_SectionTable &&
						pfsTemp->getStruxType() != PTX_SectionCell &&
						pfsTemp->getStruxType() != PTX_EndTable &&
						pfsTemp->getStruxType() != PTX_EndCell)
				{
					*ppfs = pfsTemp;
					return true;
				}
			}
		}

	// did not find it.

	return false;
}

/*!
 * Scan backwards from the given frag until a start hyperlink is found.
 * This method is used to determine if a frag is inside a hyperlink span.
 * Returns NULL if:
 * (a) It encounters a strux first.
 * (b) It encounters an end hyperlink first
 * (c) It encounters the begin of document
 *
 * FIXME!! Should this code work for annotations too?
 */
pf_Frag *    pt_PieceTable::_findPrevHyperlink(pf_Frag * pfStart)
{
	pf_Frag * pf = pfStart;
	pf_Frag_Object *pOb = NULL;
	UT_sint32 iCountFootnotes = 0;
	while(pf)
	{
		if(pf->getType() == pf_Frag::PFT_Strux)
		{
			if(isEndFootnote(pf))
			{
				iCountFootnotes++;
			}
			else if(isFootnote(pf))
			{
				iCountFootnotes--;
			}
			else if(iCountFootnotes == 0)
			{
				return NULL;
			}
		}
		if(pf->getType() == pf_Frag::PFT_Object)
		{
			pOb = static_cast<pf_Frag_Object*>(pf);
			if(pOb->getObjectType() == PTO_Hyperlink)
			{
				const PP_AttrProp * pAP = NULL;
				pOb->getPieceTable()->getAttrProp(pOb->getIndexAP(),&pAP);
				UT_return_val_if_fail (pAP, NULL);
				const gchar* pszHref = NULL;
				const gchar* pszHname  = NULL;
				UT_uint32 k = 0;
				while((pAP)->getNthAttribute(k++,pszHname, pszHref))
				{
					if(!strcmp(pszHname, "xlink:href"))
				    {
						return pf;
					}
				}
				return NULL;
			}

		}
		pf = pf->getPrev();
	}
	return NULL;
}


/*!
 * Scan backwards fromthe given frag until an end hyperlink is found.
 * This method is used to determine if a frag is inside a hyperlink span.
 * Returns NULL if:
 * (a) It encounters a strux first.
 * (b) It encounters a start hyperlink first
 * (c) It encounters the end of document
 */
pf_Frag *    pt_PieceTable::_findNextHyperlink(pf_Frag * pfStart)
{
	pf_Frag * pf = pfStart;
	pf_Frag_Object *pOb = NULL;
	UT_sint32 iCountFootnotes = 0;
	while(pf && pf != m_fragments.getLast())
	{
		if(pf->getType() == pf_Frag::PFT_Strux)
		{
			if(isFootnote(pf))
			{
				iCountFootnotes++;
			}
			else if(isEndFootnote(pf))
			{
				iCountFootnotes--;
			}
			else if(iCountFootnotes == 0)
			{
				return NULL;
			}
		}
		if(pf->getType() == pf_Frag::PFT_Object)
		{
			pOb = static_cast<pf_Frag_Object*>(pf);
			if(pOb->getObjectType() == PTO_Hyperlink)
			{
				const PP_AttrProp * pAP = NULL;
				pOb->getPieceTable()->getAttrProp(pOb->getIndexAP(),&pAP);
				UT_return_val_if_fail (pAP, NULL);
				const gchar* pszHref = NULL;
				const gchar* pszHname  = NULL;
				UT_uint32 k = 0;
				while((pAP)->getNthAttribute(k++,pszHname, pszHref))
				{
					if(!strcmp(pszHname, "xlink:href"))
				    {
						return NULL;
					}
				}
				//
				// No start marker => Must be end marker - GOT IT!
				//
				return pf;
			}
		}
		pf = pf->getNext();
	}
	return NULL;
}

bool pt_PieceTable::_getStruxFromFrag(pf_Frag * pfStart, pf_Frag_Strux ** ppfs) const
{
	// return the strux frag immediately prior to (containing)
	// the given fragment.

	*ppfs = NULL;

	pf_Frag * pf;
	for (pf=pfStart->getPrev(); (pf && (pf->getType() != pf_Frag::PFT_Strux)); pf=pf->getPrev())
		;
	if (!pf)
		return false;

	*ppfs = static_cast<pf_Frag_Strux *>(pf);
	return true;
}


bool pt_PieceTable::_getNextStruxAfterFragSkip(pf_Frag *pfStart, pf_Frag_Strux ** ppfs)
{

	*ppfs = NULL;

	pf_Frag * pf;
	UT_sint32 countFoots = 0;
	if(isFootnote(pfStart))
	{
		countFoots++;
	}
	pf = pfStart->getNext();
	if(pf && isFootnote(pf))
	{
		countFoots++;
	}
	xxx_UT_DEBUGMSG(("_getStruxFromFragStrux: 1 countFoots %d \n",countFoots));
	while(pf && (pf->getType() != pf_Frag::PFT_EndOfDoc) && ((pf->getType() != pf_Frag::PFT_Strux) || (countFoots > 0) 
				 || isFootnote(pf) || isEndFootnote(pf)))
	{
		pf=pf->getNext();
		if(isFootnote(pf))
		{
			countFoots++;
		}
		else if(isEndFootnote(pf))
		{
			countFoots--;
		}
		xxx_UT_DEBUGMSG(("_getStruxFromFragStrux: 2 countFoots %d \n",countFoots));
	}
		;
	if (!pf)
		return false;

	*ppfs = static_cast<pf_Frag_Strux *>(pf);
	return true;
}


bool pt_PieceTable::_getStruxFromFragSkip(pf_Frag * pfStart, pf_Frag_Strux ** ppfs) const
{
	// return the strux frag immediately prior to (containing)
	// the given fragment while skipping endFootnote/footnote stuff.

	*ppfs = NULL;

	pf_Frag * pf;
	UT_sint32 countFoots = 0;
	if(isEndFootnote(pfStart))
	{
		countFoots++;
	}
	pf = pfStart->getPrev();
	if(isEndFootnote(pf))
	{
		countFoots++;
	}
	if(isFootnote(pf))
	{
		countFoots--;
	}
	xxx_UT_DEBUGMSG(("_getStruxFromFragStrux: 1 countFoots %d \n",countFoots));
	while(pf && ((pf->getType() != pf_Frag::PFT_Strux) || (countFoots > 0) 
				 || isFootnote(pf) || isEndFootnote(pf)))
	{
		pf=pf->getPrev();
		if(pf && isFootnote(pf))
		{
			countFoots--;
		}
		else if(pf && isEndFootnote(pf))
		{
			countFoots++;
		}
		xxx_UT_DEBUGMSG(("_getStruxFromFragStrux: 2 countFoots %d \n",countFoots));
	}
		;
	if (!pf)
		return false;

	*ppfs = static_cast<pf_Frag_Strux *>(pf);
	return true;
}

UT_uint32 pt_PieceTable::_computeBlockOffset(pf_Frag_Strux * pfs,pf_Frag * pfTarget) const
{
	// return the block offset of the beginning of pfTarget from the end of pfs.

	UT_uint32 sum;
	pf_Frag * pf;

	for (pf=pfs->getNext(), sum=0; (pf && (pf!=pfTarget)); sum+=pf->getLength(), pf=pf->getNext())
		;
	if(pf == NULL)
	{
		return 0;
	}

	return sum;
}


void pt_PieceTable::clearIfAtFmtMark(PT_DocPosition dpos)
{
	while (_lastUndoIsThisFmtMark(dpos))
	{
		UT_DEBUGMSG(("clearIfAtFmtMark doing something...\n"));
		undoCmd();
	}
}

bool pt_PieceTable::_changePointWithNotify(PT_DocPosition dpos)
{
	PX_ChangeRecord * pcr
		= new PX_ChangeRecord(PX_ChangeRecord::PXT_ChangePoint,
							  dpos, 0,0);
	UT_return_val_if_fail (pcr,false);

	m_history.addChangeRecord(pcr);
	m_pDocument->notifyListeners(NULL, pcr);

	return true;
}

/*!
    This function crawls the entire PT and assignes new xid to any fragment that should
    have one and does not. It is primarily to be used by exporters (accessed throught
    PD_Document wrapper)
*/
void pt_PieceTable::fixMissingXIDs()
{
	for (pf_Frag * pf = m_fragments.getFirst(); (pf); pf=pf->getNext())
	{
		if(!pf->getXID() && pf->usesXID())
			pf->setXID(getXID());
	}
}

UT_uint32 pt_PieceTable::getXID()
{
	++m_iXID;
	return m_iXID;
}


/* Return true if neither dpos1 nor dpos2 is within a note and the whole document is not selected*/
bool pt_PieceTable::_checkSkipFootnote(PT_DocPosition dpos1, PT_DocPosition dpos2, pf_Frag * pf_End) const
{
	if(m_embeddedStrux.empty())
	{
		return true;
	}

	if (!pf_End)
	{
		PT_BlockOffset offset;
		getFragFromPosition(dpos2,&pf_End,&offset);
	}
	if ((dpos1 == 1) && ((pf_End->getType() == pf_Frag::PFT_EndOfDoc) ||
						 ((pf_End->getType() == pf_Frag::PFT_Strux) && 
						  (static_cast<pf_Frag_Strux*>(pf_End)->getStruxType() == PTX_SectionHdrFtr))))
	{
		return false;
	}

	bool bSkipNote = true;
	std::list<embeddedStrux>::const_reverse_iterator it;
	for (it = m_embeddedStrux.rbegin(); it != m_embeddedStrux.rend(); ++it)
	{
		if ((*it).beginNote->getPos() < dpos2)
		{
			if ((*it).endNote->getPos() > dpos2)
			{
				bSkipNote = false;
			}
			break;
		}
	}
	if (bSkipNote)
	{
		if (it != m_embeddedStrux.rbegin())
		{
			it--;
		}
		for (; it != m_embeddedStrux.rend(); ++it)
		{
			if ((*it).beginNote->getPos() < dpos1)
			{
				if ((*it).endNote->getPos() > dpos1)
				{
					bSkipNote = false;
				}
				break;
			}
		}
	}
	return bSkipNote;
}
