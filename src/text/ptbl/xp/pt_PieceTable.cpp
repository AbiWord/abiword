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


#include "ut_types.h"
#include "ut_misc.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_growbuf.h"
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


// TODO: calculate this from pf_FRAG_STRUX_*_LENGTH instead?
#define pt_BOD_POSITION 2

/*****************************************************************/
/*****************************************************************/

pt_PieceTable::pt_PieceTable(PD_Document * pDocument) 
	: m_hashStyles(11)
{
	m_pts = PTS_Create;
	m_pDocument = pDocument;

	setPieceTableState(PTS_Create);
	loading.m_indexCurrentInlineAP = 0;
}

pt_PieceTable::~pt_PieceTable()
{
	UT_HASH_PURGEDATA(PD_Style *, &m_hashStyles, delete);
}

void pt_PieceTable::setPieceTableState(PTState pts)
{
	UT_ASSERT(pts >= m_pts);

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

bool pt_PieceTable::_struxHasContent(pf_Frag_Strux * pfs) const
{
	// return true iff the paragraph has content (text).

	return (pfs->getNext() && (pfs->getNext()->getType() == pf_Frag::PFT_Text));
}

bool pt_PieceTable::getAttrProp(PT_AttrPropIndex indexAP, const PP_AttrProp ** ppAP) const
{
	UT_ASSERT(ppAP);

	const PP_AttrProp * pAP = m_varset.getAP(indexAP);
	if (!pAP)
		return false;

	*ppAP = pAP;
	return true;
}

bool pt_PieceTable::_getSpanAttrPropHelper(pf_Frag * pf, const PP_AttrProp ** ppAP) const
{
#define ReturnThis(type,pf)													\
	do {	type * _pf_ = static_cast< type *>((pf));						\
			const PP_AttrProp * pAP = m_varset.getAP(_pf_->getIndexAP());	\
			*ppAP = pAP;													\
			return (pAP != NULL);											\
	} while (0)

	switch (pf->getType())
	{
	case pf_Frag::PFT_FmtMark:
		ReturnThis( pf_Frag_FmtMark, pf );

	case pf_Frag::PFT_Text:
		ReturnThis( pf_Frag_Text, pf );
		
	case pf_Frag::PFT_Object:
		ReturnThis( pf_Frag_Object, pf );

	case pf_Frag::PFT_Strux:
	case pf_Frag::PFT_EndOfDoc:
	default:
		*ppAP = NULL;
		return false;
	}
#undef ReturnThis
}

		  
bool pt_PieceTable::getSpanAttrProp(PL_StruxDocHandle sdh, UT_uint32 offset, bool bLeftSide,
									   const PP_AttrProp ** ppAP) const
{
	// return the AP for the text at the given offset from the given strux.
	// offset zero now refers to the first character in the block, so adding
	// fl_BLOCK_STRUX_OFFSET to the offset in the call is no longer necessary.
	
	UT_ASSERT(sdh);
	UT_ASSERT(ppAP);

	pf_Frag * pf = (pf_Frag *)sdh;
	UT_ASSERT(pf->getType() == pf_Frag::PFT_Strux);
	pf_Frag_Strux * pfsBlock = static_cast<pf_Frag_Strux *> (pf);
	UT_ASSERT(pfsBlock->getStruxType() == PTX_Block);

	UT_uint32 cumOffset = 0;
	UT_uint32 cumEndOffset = 0;
	for (pf_Frag * pfTemp=pfsBlock->getNext(); (pfTemp); cumOffset=cumEndOffset, pfTemp=pfTemp->getNext())
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

		UT_ASSERT(offset > cumOffset);
		
		if (offset == cumEndOffset)		// there's a frag boundary exactly where we want. pfTemp is to our left.
		{
			if (!bLeftSide)
				continue;				// return the next one on the next loop iteration
			
			// FmtMarks have length zero, so we advance to put it to our left and then decide what to do
			if (pfTemp->getNext() && (pfTemp->getNext()->getType()==pf_Frag::PFT_FmtMark))
				continue;				// we'll return this one on the next loop iteration

			// otherwise, we want the thing that we are at the end of (ie that is to the left)
			return _getSpanAttrPropHelper(pfTemp,ppAP);
		}

		UT_ASSERT(offset < cumEndOffset);

		// the place we want is inside of a fragment, so just use it.
		return _getSpanAttrPropHelper(pfTemp,ppAP);
	}

	*ppAP = NULL;
	return false;
}

bool pt_PieceTable::getSpanPtr(PL_StruxDocHandle sdh, UT_uint32 offset,
								  const UT_UCSChar ** ppSpan, UT_uint32 * pLength) const
{
	// note: offset zero refers to the strux.  the first character is at
	// note: (0 + pfsBlock->getLength()).

	*ppSpan = NULL;
	*pLength = 0;
	
	pf_Frag * pf = (pf_Frag *)sdh;
	UT_ASSERT(pf->getType() == pf_Frag::PFT_Strux);
	pf_Frag_Strux * pfsBlock = static_cast<pf_Frag_Strux *> (pf);
	UT_ASSERT(pfsBlock->getStruxType() == PTX_Block);

	UT_uint32 cumOffset = pf->getLength();
	for (pf_Frag * pfTemp=pfsBlock->getNext(); (pfTemp); pfTemp=pfTemp->getNext())
	{
		if (offset == cumOffset)
		{
			if (pfTemp->getType() == pf_Frag::PFT_FmtMark)
				continue;
			
			if (pfTemp->getType() != pf_Frag::PFT_Text)
				return false;
			
			pf_Frag_Text * pfText = static_cast<pf_Frag_Text *> (pfTemp);
			*ppSpan = getPointer(pfText->getBufIndex());
			*pLength = pfText->getLength();
			return true;
		}
		if (offset < cumOffset+pfTemp->getLength())
		{
			if (pfTemp->getType() != pf_Frag::PFT_Text)
				return false;

			pf_Frag_Text * pfText = static_cast<pf_Frag_Text *> (pfTemp);
			const UT_UCSChar * p = getPointer(pfText->getBufIndex());
			UT_uint32 delta = offset - cumOffset;
			*ppSpan = p + delta;
			*pLength = pfText->getLength() - delta;
			return true;
		}

		cumOffset += pfTemp->getLength();
	}
	return false;
}

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
bool pt_PieceTable::getBlockBuf(PL_StruxDocHandle sdh, 
                                   UT_GrowBuf * pgb) const
{
    UT_ASSERT(pgb);
	
    pf_Frag * pf = (pf_Frag *)sdh;
    UT_ASSERT(pf->getType() == pf_Frag::PFT_Strux);
    pf_Frag_Strux * pfsBlock = static_cast<pf_Frag_Strux *> (pf);
    UT_ASSERT(pfsBlock->getStruxType() == PTX_Block);

    UT_uint32 bufferOffset = pgb->getLength();
	
    pf_Frag * pfTemp = pfsBlock->getNext();
    while (pfTemp)
    {
        switch (pfTemp->getType())
        {
        default:
            UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
        case pf_Frag::PFT_Strux:
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
            bAppended = pgb->ins(bufferOffset,pSpan,length);
            UT_ASSERT(bAppended);
            
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
            // TODO malloc and what happens when it fails....
				
            UT_UCSChar* pSpaces = new UT_UCSChar[length];
            for (UT_uint32 i=0; i<length; i++)
            {
                pSpaces[i] = UCS_ABI_OBJECT;
            }
            bool bAppended;
            bAppended = pgb->ins(bufferOffset, pSpaces, length);
            delete[] pSpaces;
            UT_ASSERT(bAppended);
		
            bufferOffset += length;
        }
        pfTemp = pfTemp->getNext();
        break;
        }
    }

    UT_ASSERT(bufferOffset == pgb->getLength());
    return true;
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

bool pt_PieceTable::getFragFromPosition(PT_DocPosition docPos,
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

			// a FmtMark has length zero.  we don't want to find it
			// in this loop -- rather we want the thing just past it.
			
			UT_ASSERT(pf->getType() != pf_Frag::PFT_FmtMark);
			
			return true;
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

	return true;
}

bool pt_PieceTable::getFragsFromPositions(PT_DocPosition dPos1, PT_DocPosition dPos2,
											 pf_Frag ** ppf1, PT_BlockOffset * pOffset1,
											 pf_Frag ** ppf2, PT_BlockOffset * pOffset2) const
{
	// compute the (fragment,offset) pairs for each position given.
	
	UT_ASSERT(dPos1 <= dPos2);
	UT_ASSERT(ppf1);
	UT_ASSERT(pOffset1);
	
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
		length = pf->getLength();
	}

	// a FmtMark has length zero.  we don't want to find it here.
	// rather we want the thing to the right of it.
	UT_ASSERT(pf->getType() != pf_Frag::PFT_FmtMark);

	if (ppf2)
		*ppf2 = pf;
	if (pOffset2)
		*pOffset2 = offset+deltaPos;
	return true;
}
	
bool pt_PieceTable::getStruxFromPosition(PL_ListenerId listenerId,
											PT_DocPosition docPos,
											PL_StruxFmtHandle * psfh) const
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
												  PL_StruxFmtHandle * psfh) const
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
						   PL_StruxDocHandle * sdh) const
{

	pf_Frag_Strux * pfs = NULL;
	if (!_getStruxOfTypeFromPosition(docPos,pts,&pfs))
		return false;
	*sdh = ( PL_StruxDocHandle ) pfs;
	return true;
}

bool pt_PieceTable::_getStruxFromPosition(PT_DocPosition docPos,
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
	return true;
}

bool pt_PieceTable::_getStruxOfTypeFromPosition(PT_DocPosition dpos,
												   PTStruxType pts,
												   pf_Frag_Strux ** ppfs) const
{
	// return the strux fragment of the given type containing
	// the given absolute document position.
	UT_ASSERT(ppfs);
	*ppfs = NULL;
	
	pf_Frag_Strux * pfs = NULL;
	if (!_getStruxFromPosition(dpos,&pfs))
		return false;

	if (pfs->getStruxType() == pts || (pts == PTX_Section && pfs->getStruxType() == PTX_SectionHdrFtr))		// is it of the type we want
	{
		*ppfs = pfs;
		return true;
	}

	// if not, we walk backwards thru the list and try to find it.

	for (pf_Frag * pf=pfs; (pf); pf=pf->getPrev())
		if (pf->getType() == pf_Frag::PFT_Strux)
		{
			pf_Frag_Strux * pfsTemp = static_cast<pf_Frag_Strux *>(pf);
			if (pfsTemp->getStruxType() == pts || (pts == PTX_Section && pfsTemp->getStruxType() == PTX_SectionHdrFtr))	// did we find it
			{
				*ppfs = pfsTemp;
				return true;
			}
		}

	// did not find it.
	
	return false;
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

UT_uint32 pt_PieceTable::_computeBlockOffset(pf_Frag_Strux * pfs,pf_Frag * pfTarget) const
{
	// return the block offset of the beginning of pfTarget from the end of pfs.

	UT_uint32 sum;
	pf_Frag * pf;

	for (pf=pfs->getNext(), sum=0; (pf!=pfTarget); sum+=pf->getLength(), pf=pf->getNext())
		;

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
							  dpos, 0);
	UT_ASSERT(pcr);

	m_history.addChangeRecord(pcr);
	m_pDocument->notifyListeners(NULL, pcr);

	return true;
}



