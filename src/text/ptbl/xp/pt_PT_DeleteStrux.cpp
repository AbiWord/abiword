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


// deleteStrux-related functions for class pt_PieceTable.

#include "ut_types.h"
#include "ut_misc.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_growbuf.h"
#include "pt_PieceTable.h"
#include "pf_Frag.h"
#include "pf_Frag_Strux.h"
#include "pf_Frag_Strux_Block.h"
#include "pf_Frag_Strux_Section.h"
#include "pf_Frag_Text.h"
#include "pf_Fragments.h"
#include "px_ChangeRecord.h"
#include "px_CR_Span.h"
#include "px_CR_SpanChange.h"
#include "px_CR_Strux.h"

/****************************************************************/
/****************************************************************/
bool pt_PieceTable::_unlinkStrux(pf_Frag_Strux * pfs,
									pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd)
{
	switch (pfs->getStruxType())	
	{
	case PTX_Section:
	case PTX_SectionHdrFtr:
	case PTX_SectionEndnote:
	case PTX_SectionTable:
	case PTX_SectionCell:
	case PTX_EndCell:
	case PTX_EndTable:
		return _unlinkStrux_Section(pfs,ppfEnd,pfragOffsetEnd);
		
	case PTX_Block:
		return _unlinkStrux_Block(pfs,ppfEnd,pfragOffsetEnd);
		
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	}
}

bool pt_PieceTable::_unlinkStrux_Block(pf_Frag_Strux * pfs,
										  pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd)
{
	UT_ASSERT(pfs->getStruxType()==PTX_Block);
	
	// unlink this Block strux from the document.
	// the caller is responsible for deleting pfs.

	if (ppfEnd)
		*ppfEnd = pfs->getNext();
	if (pfragOffsetEnd)
		*pfragOffsetEnd = 0;
	
	// find the previous strux (either a paragraph or something else).

	pf_Frag_Strux * pfsPrev = NULL;
	pf_Frag * pf = pfs->getPrev();
	while (pf && !pfsPrev)
	{
		if (pf->getType() == pf_Frag::PFT_Strux)
			pfsPrev = static_cast<pf_Frag_Strux *> (pf);
		pf = pf->getPrev();
	}
	UT_ASSERT(pfsPrev);			// we have a block that's not in a section ??
	//
	// Code to prevent a crash. But this should not happen and if it does not everything will
    // be deleted - Sevior.
	//
	if(pfsPrev == NULL)
	{
		_unlinkFrag(pfs,ppfEnd,pfragOffsetEnd);
		return false;
	}

	switch (pfsPrev->getStruxType())
	{
	
	case PTX_Block:
		// if there is a paragraph before us, we can delete this
		// paragraph knowing that our content will be assimilated
		// in to the previous one.

		_unlinkFrag(pfs,ppfEnd,pfragOffsetEnd);
		return true;
	
	case PTX_SectionEndnote:
	case PTX_Section:
		// we are the first paragraph in this section.  if we have
		// content, we cannot be deleted, since there is no one to
		// inherit our content.

		if (_struxHasContent(pfs))
		{
			// TODO decide if this should assert or just fail...
			UT_DEBUGMSG(("Cannot delete first paragraph with content.\n"));
			UT_ASSERT(0);
			return false;
		}


	case PTX_SectionHdrFtr:
		// we are the first paragraph in this section.  if we have
		// content, we cannot be deleted, since there is no one to
		// inherit our content.

		if (_struxHasContent(pfs))
		{
			// TODO decide if this should assert or just fail...
			UT_DEBUGMSG(("Cannot delete first paragraph with content.\n"));
			UT_ASSERT(0);
			return false;
		}

		// no content in this paragraph.
		
		_unlinkFrag(pfs,ppfEnd,pfragOffsetEnd);
		return true;


	case PTX_SectionTable:
	case PTX_SectionCell:
	case PTX_EndCell:
	case PTX_EndTable:
//
// deleting tables and cells is a mutlti-step process and we can make no assumptions
// along the way.
//		
		_unlinkFrag(pfs,ppfEnd,pfragOffsetEnd);
		return true;


	default:
		UT_ASSERT(0);
		return false;
	}
}

bool pt_PieceTable::_unlinkStrux_Section(pf_Frag_Strux * pfs,
											pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd)
{
	UT_ASSERT(pfs->getStruxType()==PTX_Section
			  || pfs->getStruxType()==PTX_SectionHdrFtr
			  || pfs->getStruxType()==PTX_SectionEndnote
			  || pfs->getStruxType()==PTX_SectionTable
			  || pfs->getStruxType()==PTX_SectionCell
			  || pfs->getStruxType()==PTX_EndCell
			  || pfs->getStruxType()==PTX_EndTable );
	
	// unlink this Section strux from the document.
	// the caller is responsible for deleting pfs.

	if (ppfEnd)
		*ppfEnd = pfs->getNext();
	if (pfragOffsetEnd)
		*pfragOffsetEnd = 0;
	
	// find the previous strux (either a paragraph or something else).

	pf_Frag_Strux * pfsPrev = NULL;
	pf_Frag * pf = pfs->getPrev();
	while (pf && !pfsPrev)
	{
		if (pf->getType() == pf_Frag::PFT_Strux)
			pfsPrev = static_cast<pf_Frag_Strux *> (pf);
		pf = pf->getPrev();
	}

	if (!pfsPrev)
	{
		// first section in the document cannot be deleted.
		// TODO decide if this should assesrt or just file...
		UT_DEBUGMSG(("Cannot delete first section in document.\n"));
		UT_ASSERT(0);
		return false;
	}
	
	switch (pfsPrev->getStruxType())
	{
	case PTX_Block:
		// if there is a paragraph before us, we can delete this
		// section knowing that our paragraphs will be assimilated
		// in to the previous section (that is, the container of
		// this block).

		_unlinkFrag(pfs,ppfEnd,pfragOffsetEnd);
		return true;
	case PTX_SectionTable:
        //
        // deleting tables is a multi-step process that can't make assumptions
        // on a single step
		_unlinkFrag(pfs,ppfEnd,pfragOffsetEnd);
		return true;

	case PTX_SectionCell:
        //
        // deleting tables is a multi-step process that can't make assumptions
        // on a single step
		_unlinkFrag(pfs,ppfEnd,pfragOffsetEnd);
		return true;

	case PTX_EndCell:
        //
        // deleting tables is a multi-step process that can't make assumptions
        // on a single step
		_unlinkFrag(pfs,ppfEnd,pfragOffsetEnd);
		return true;

	case PTX_EndTable:
        //
        // deleting tables is a multi-step process that can't make assumptions
        // on a single step
		_unlinkFrag(pfs,ppfEnd,pfragOffsetEnd);
		return true;

	case PTX_Section:
		// there are no blocks (paragraphs) between this section
		// and the previous section.  this is not possible.
		// TODO decide if this should assert or just fail...
		UT_DEBUGMSG(("No blocks between sections ??\n"));
		UT_ASSERT(0);
		return false;


	case PTX_SectionHdrFtr:
		// there are no blocks (paragraphs) between this section
		// and the previous section.  this is not possible.
		// TODO decide if this should assert or just fail...
        //
        // Actually this is OK if it's a hdrFtr that has not been 
		// "realized" yet. Like an even hdrftr that has been defined
        // but no even pages exist yet.
		UT_DEBUGMSG(("No blocks between sections ??\n"));
//		_unlinkFrag(pfs,ppfEnd,pfragOffsetEnd);
		UT_ASSERT(0);
		return false;

	default:
		UT_ASSERT(0);
		return false;
	}
}
			
bool pt_PieceTable::_deleteStruxWithNotify(PT_DocPosition dpos,
										   pf_Frag_Strux * pfs,
										   pf_Frag ** ppfEnd, 
										   UT_uint32 * pfragOffsetEnd,
										   bool bWithRec)
{
	PX_ChangeRecord_Strux * pcrs
		= new PX_ChangeRecord_Strux(PX_ChangeRecord::PXT_DeleteStrux,
									dpos, pfs->getIndexAP(), pfs->getStruxType());
	UT_ASSERT(pcrs);

	if (!_unlinkStrux(pfs,ppfEnd,pfragOffsetEnd))
		return false;
	
	// add record to history.  we do not attempt to coalesce these.
	if (bWithRec)
		m_history.addChangeRecord(pcrs);
	m_pDocument->notifyListeners(pfs,pcrs);

	delete pfs;

	return true;
}

/*!
 * This method scans the piecetAble from the section Frag_strux given looking 
 * for any Header/Footers that belong to the strux. If it finds them, they
 * are deleted with notifications.
\params pf_Frag_Strux_Section pfStruxSec the Section strux that might have headers
 *                                        or footers belonging to it.
 * These must be deleted with notification otherwise they won't be recreated on
 * an undo
 */
bool pt_PieceTable::_deleteHdrFtrsFromSectionStruxIfPresent(pf_Frag_Strux_Section * pfStruxSec)
{
	//
	// Get the index to the Attributes/properties of the section strux to see if 
	// if there is a header defined for this strux.
	//
	// FIXME: Handle all the new header/footer types.
	PT_AttrPropIndex indexAP = pfStruxSec->getIndexAP();
	const PP_AttrProp * pAP = NULL;
	getAttrProp(indexAP, &pAP);
	UT_Vector vecHdrFtr;
	UT_String HeaderV,HeaderEvenV,HeaderLastV,HeaderFirstV;
	UT_String FooterV,FooterEvenV,FooterLastV,FooterFirstV;
	vecHdrFtr.clear();
	const XML_Char * szHeaderV = NULL;
	bool bres = pAP->getAttribute("header",szHeaderV);
	if(szHeaderV && *szHeaderV && (UT_strcmp(szHeaderV,"0") != 0))
	{
		HeaderV = szHeaderV;
		vecHdrFtr.addItem((void *) HeaderV.c_str());
	} 
	szHeaderV =  NULL;
	bres = pAP->getAttribute("header-even",szHeaderV);
	if(szHeaderV && *szHeaderV && (UT_strcmp(szHeaderV,"0") != 0))
	{
		HeaderEvenV = szHeaderV;
		vecHdrFtr.addItem((void *) HeaderEvenV.c_str());
	} 
	szHeaderV =  NULL;
	bres = pAP->getAttribute("header-last",szHeaderV);
	if(szHeaderV && *szHeaderV && (UT_strcmp(szHeaderV,"0") != 0))
	{
		HeaderLastV = szHeaderV;
		vecHdrFtr.addItem((void *) HeaderLastV.c_str());
	} 
	szHeaderV =  NULL;
	bres = pAP->getAttribute("header-first",szHeaderV);
	if(szHeaderV && *szHeaderV && (UT_strcmp(szHeaderV,"0") != 0))
	{
		HeaderFirstV = szHeaderV;
		vecHdrFtr.addItem((void *) HeaderFirstV.c_str());
	} 
	szHeaderV =  NULL;
	bres = pAP->getAttribute("footer",szHeaderV);
	if(szHeaderV && *szHeaderV && (UT_strcmp(szHeaderV,"0") != 0))
	{
		FooterV = szHeaderV;
		vecHdrFtr.addItem((void *) FooterV.c_str());
	} 
	szHeaderV =  NULL;
	bres = pAP->getAttribute("footer-even",szHeaderV);
	if(szHeaderV && *szHeaderV && (UT_strcmp(szHeaderV,"0") != 0))
	{
		FooterEvenV = szHeaderV;
		vecHdrFtr.addItem((void *) FooterEvenV.c_str());
	} 
	szHeaderV =  NULL;
	bres = pAP->getAttribute("footer-last",szHeaderV);
	if(szHeaderV && *szHeaderV && (UT_strcmp(szHeaderV,"0") != 0))
	{
		FooterLastV = szHeaderV;
		vecHdrFtr.addItem((void *) FooterLastV.c_str());
	} 
	szHeaderV =  NULL;
	bres = pAP->getAttribute("footer-first",szHeaderV);
	if(szHeaderV && *szHeaderV && (UT_strcmp(szHeaderV,"0") != 0))
	{
		FooterFirstV = szHeaderV;
		vecHdrFtr.addItem((void *) FooterFirstV.c_str());
	} 
	UT_sint32 countHdrFtr = (UT_sint32) vecHdrFtr.getItemCount();
	UT_DEBUGMSG(("SEVIOR: Deleting HdrFtrs from Document, num Header/Footers %d\n",countHdrFtr));
	if(0 == countHdrFtr)
	{
		return true;
	}
//
// This section has a header or footer attribute. Scan the piecetable to see 
// if there is a header strux somewhere with an ID that matches our section.
//
	pf_Frag * curFrag = NULL;
	getFragments().cleanFrags(); // clean up to be safe...
//
// Do this loop for all and headers and footers.
//
	UT_sint32 i = 0;
	for(i=0; i< countHdrFtr; i++)
	{
		curFrag = static_cast<pf_Frag *>(pfStruxSec);
		bool bFoundIt = false;
		pf_Frag_Strux * curStrux = NULL;
		while(curFrag != getFragments().getLast() && !bFoundIt)
		{
			if(curFrag->getType() == pf_Frag::PFT_Strux)
			{
				curStrux = static_cast<pf_Frag_Strux *>(curFrag);
				if(curStrux->getStruxType() == PTX_SectionHdrFtr)
				{
//
// OK we've got a candidate
//
					PT_AttrPropIndex indexAPHdr = curStrux->getIndexAP();
					const PP_AttrProp * pAPHdr = NULL;
					getAttrProp(indexAPHdr, &pAPHdr);
					const XML_Char * szID = NULL;
					bres = pAPHdr->getAttribute("id",szID);
					UT_DEBUGMSG(("SEVIOR: Found candidate id = %s \n",szID));
					if(bres && (szID != NULL))
					{
					//
					// Look for a match.
					//
						szHeaderV = (const char *) vecHdrFtr.getNthItem(i);
						if(szHeaderV != NULL && strcmp(szHeaderV,szID) == 0)
						{
							bFoundIt = true;
						}
					}
				}
			}
			curFrag = curFrag->getNext();
		}
		if(bFoundIt)
		{
		//
		// This Header belongs to our section. It must be deleted.
		//
			_deleteHdrFtrStruxWithNotify(curStrux);
//
// Clean up after deleting the text in the Header.
//
			getFragments().cleanFrags();
		}
	}
	return true;
}

/*!
 * This method deletes the Header/Footer from the pieceTable in the order that
 * will allow an undo to recreate it.
 */
void pt_PieceTable::_deleteHdrFtrStruxWithNotify( pf_Frag_Strux * pfFragStruxHdrFtr)
{
//
// First we need the document position of the header/footer strux.
// 
	UT_DEBUGMSG(("SEVIOR: Deleting hdrftr \n"));
	const pf_Frag * pfFrag = NULL;
	pfFrag = static_cast<pf_Frag *>(pfFragStruxHdrFtr);
	PT_DocPosition HdrFtrPos = getFragPosition(pfFrag);
	UT_Vector vecFragStrux;
	UT_DEBUGMSG(("SEVIOR: Deleting hdrftr Strux Pos = %d \n",HdrFtrPos));
//
// Now find the first Non-strux frag within this hdrftr
//
	bool bStop = false;
	while((pfFrag->getType() == pf_Frag::PFT_Strux) && (pfFrag != getFragments().getLast()) && !bStop)
	{
		const pf_Frag_Strux * pfs = static_cast<const pf_Frag_Strux *>(pfFrag);
		if(pfs != pfFragStruxHdrFtr && pfs->getStruxType() != PTX_Block)
		{
			bStop = true;
		}
		else
		{
			vecFragStrux.addItem((void *) pfFrag);
			pfFrag = pfFrag->getNext();
		}
	}
	PT_DocPosition TextStartPos = getFragPosition(pfFrag);
	UT_DEBUGMSG(("SEVIOR: Deleting hdrftr Text Start Pos = %d \n",TextStartPos));
//
// Now find the end of the text in the header/footer
//
	bool foundEnd = false;
	if(!bStop)
	{
		while(!foundEnd)
		{
			foundEnd = pfFrag == getFragments().getLast();
			if(!foundEnd && pfFrag->getType() == pf_Frag::PFT_Strux)
			{
				const pf_Frag_Strux * pfFragStrux = static_cast<const pf_Frag_Strux *>(pfFrag);
				foundEnd = pfFragStrux->getStruxType() != PTX_Block;
			}
			if(!foundEnd)
			{
				pfFrag = pfFrag->getNext();
			}
		}
		PT_DocPosition TextEndPos = getFragPosition(pfFrag);
		UT_DEBUGMSG(("SEVIOR: Deleting hdrftr Text End Pos = %d \n",TextEndPos));
//
// OK delete the text
//
		if(TextEndPos > TextStartPos)
		{
			deleteSpan(TextStartPos,TextEndPos,NULL,true);
		}
	}
//
// Now delete the struxes at the start.
//
// I assume there are blocks then sections. First delete all the blocks, then the
// section strux.
//
	UT_uint32 count = vecFragStrux.getItemCount();
	UT_ASSERT(count > 1);
	UT_uint32 i=0;
	bool bres = false;
	for(i=1; i<count; i++)
	{
		pf_Frag_Strux * pfs = (pf_Frag_Strux *) vecFragStrux.getNthItem(i);
		bres = _deleteStruxWithNotify(pfs->getPos(),pfs,NULL,NULL);
		UT_ASSERT(bres);
	}
	bres = _deleteStruxWithNotify(pfFragStruxHdrFtr->getPos(),pfFragStruxHdrFtr,NULL,NULL);
	UT_ASSERT(bres);
//	deleteSpan(HdrFtrPos,TextStartPos,NULL,true);
}




