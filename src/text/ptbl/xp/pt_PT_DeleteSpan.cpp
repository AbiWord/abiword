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
							   UT_uint32 &iRealDeleteCount,
							   bool bDontGlob)
{
	return deleteSpan(dpos1,
					  dpos2,
					  p_AttrProp_Before,
					  iRealDeleteCount,
					  true,
					  bDontGlob);
}

/****************************************************************/
bool pt_PieceTable::deleteSpanWithTable(PT_DocPosition dpos1,
										PT_DocPosition dpos2,
										PP_AttrProp *p_AttrProp_Before,
										UT_uint32 &iRealDeleteCount,
										bool bDeleteTableStruxes)
{
	return deleteSpan(dpos1,
					  dpos2,
					  p_AttrProp_Before,
					  iRealDeleteCount,
					  bDeleteTableStruxes,
					  false);
}

/****************************************************************/
bool pt_PieceTable::deleteSpan(PT_DocPosition dpos1,
							   PT_DocPosition dpos2,
							   PP_AttrProp *p_AttrProp_Before,
							   UT_uint32 &iRealDeleteCount,
							   bool bDeleteTableStruxes,
							   bool bDontGlob)
{
	if(m_pDocument->isMarkRevisions())
	{
		
		bool bHdrFtr = false;
		// if the user selected the whole document for deletion, we will not delete the
		// first block (we need always a visible block in any document); we make an
		// exception to this in VDND, because in that case the original block is
		// guaranteed to come back
		// 
		// NB: it is possible that the user might delete all contents in several separate
		// steps; there is no easy way to protect against that
		bool bWholeDoc = false;
		if(!m_pDocument->isVDNDinProgress())
		{
			pf_Frag * pLast = getFragments().getLast();
			bWholeDoc = (dpos1 <= 2 && pLast->getPos() == dpos2);
		}
		
		iRealDeleteCount = 0;

		const gchar name[] = "revision";
		const gchar * pRevision = NULL;

		// we cannot retrieve the start and end fragments here and
		// then work between them in a loop using getNext() because
		// processing might result in merging of fargments. so we have
		// to use the doc position to keep track of where we are and
		// retrieve the fragments afresh in each step of the loop
		// Tomas, Oct 28, 2003

		bool bRet = false;
		while(dpos1 < dpos2)
		{
			// first retrive the starting and ending fragments
			pf_Frag * pf1, * pf2;
			PT_BlockOffset Offset1, Offset2;
			bool bTableStrux = false;
			bool bHasEndStrux = false;
			UT_sint32 iTableDepth = 0;

			if(!getFragsFromPositions(dpos1,dpos2, &pf1, &Offset1, &pf2, &Offset2))
				return bRet;
			else
				bRet = true;

			// get attributes for this fragement
			const PP_AttrProp * pAP2;
			pf_Frag::PFType eType = pf1->getType();
			UT_uint32 iLen = 1;
			PTStruxType eStruxType = PTX_StruxDummy;

			if(eType == pf_Frag::PFT_Text)
			{
				if(!getAttrProp(static_cast<pf_Frag_Text*>(pf1)->getIndexAP(),&pAP2))
					return false;
			}
			else if(eType == pf_Frag::PFT_Strux)
			{
				if(!getAttrProp(static_cast<pf_Frag_Strux*>(pf1)->getIndexAP(),&pAP2))
					return false;

				eStruxType = static_cast<pf_Frag_Strux*>(pf1)->getStruxType();

				switch (eStruxType)
				{
					case PTX_Block:
						iLen = pf_FRAG_STRUX_BLOCK_LENGTH;
						if(bWholeDoc && dpos1 == 2)
						{
							dpos1 += iLen;
							continue;
						}
						
						break;
						
					case PTX_SectionTable:
						iTableDepth = 1;
						// fall through
					case PTX_SectionCell:
						bTableStrux = true;
						bHasEndStrux = true;
						iLen = pf_FRAG_STRUX_SECTION_LENGTH;
						break;
						
					case PTX_EndCell:
					case PTX_EndTable:
						bTableStrux = true;
						iLen = pf_FRAG_STRUX_SECTION_LENGTH;
						break;
						
					case PTX_SectionEndnote:
					case PTX_SectionFootnote:
					case PTX_SectionAnnotation:
					case PTX_SectionFrame:
					case PTX_SectionTOC:
						bHasEndStrux = true;
						// fall through ...
					case PTX_SectionHdrFtr:
						bHdrFtr = true;
						// fall through
					case PTX_Section:
				    case PTX_EndFootnote:
				    case PTX_EndEndnote:
				    case PTX_EndAnnotation:
				    case PTX_EndFrame:
					case PTX_EndTOC:
						iLen = pf_FRAG_STRUX_SECTION_LENGTH;
						break;

					default:
						UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
						iLen = 1;
						break;
				}

			}
			else if(eType == pf_Frag::PFT_Object)
			{
				if(!getAttrProp(static_cast<pf_Frag_Object*>(pf1)->getIndexAP(),&pAP2))
					return false;
			}
			else if(eType == pf_Frag:: PFT_FmtMark)
			{
				iLen = 0;
				if(!getAttrProp(static_cast<pf_Frag_Object*>(pf1)->getIndexAP(),&pAP2))
					return false;
				// something that does not carry AP
			}
			else if (eType == pf_Frag:: PFT_EndOfDoc)
			{
				break;
			}
			else
			{
				UT_ASSERT(0); // Dunno what this could be
				break;
			}

			if(bTableStrux && !bDeleteTableStruxes)
			{
				// skip over this frag
				dpos1 += iLen;
				continue;
			}
			
			if(!pAP2->getAttribute(name, pRevision))
				pRevision = NULL;

			PP_RevisionAttr Revisions(pRevision);

			// now we need to see if revision with this id is already
			// present, and if it is, whether it might not be addition
			UT_uint32 iId = m_pDocument->getRevisionId();
			const PP_Revision * pS;
			const PP_Revision * pRev = Revisions.getGreatestLesserOrEqualRevision(iId, &pS);

			PT_DocPosition dposEnd = UT_MIN(dpos2,dpos1 + pf1->getLength());

			if(pRev && iId == pRev->getId())
			{
				// OK, we already have a revision with this id here,
				// which means that the user made a change earlier
				// (insertion or format change) but now wants this deleted
				//
				// so if the previous revision is an addition, we just
				// remove this fragment as if this was regular delete
				if(   (pRev->getType() == PP_REVISION_ADDITION)
				   || (pRev->getType() == PP_REVISION_ADDITION_AND_FMT ))
				{
					// if this fragment is one of the struxes that is paired with an end-strux, we
					// need to delete both struxes and everything in between
					if(bHasEndStrux || bHdrFtr)
					{
						PT_DocPosition posEnd = dposEnd;
						for(pf_Frag * pf = pf1->getNext(); pf != NULL; pf = pf->getNext())
						{
							posEnd += pf->getLength();
							
							if(pf_Frag::PFT_Strux != pf->getType())
								continue;

							pf_Frag_Strux * pfs = (pf_Frag_Strux*) pf;
							PTStruxType eStrux2Type = pfs->getStruxType();

							if(eStrux2Type == PTX_SectionTable)
								iTableDepth++;
							else if (eStrux2Type == PTX_EndTable)
								iTableDepth--;
								
							
							switch(eStruxType )
							{
								case PTX_SectionEndnote:
									if(eStrux2Type != PTX_EndEndnote)
										continue;
									break;
								case PTX_SectionTable:
									if(iTableDepth > 0 || eStrux2Type != PTX_EndTable)
										continue;
									break;
								case PTX_SectionCell:
									if(iTableDepth > 0 || eStrux2Type != PTX_EndCell)
										continue;
									break;
								case PTX_SectionFootnote:
									if(eStrux2Type != PTX_EndFootnote)
										continue;
									break;
								case PTX_SectionAnnotation:
									if(eStrux2Type != PTX_EndAnnotation)
										continue;
									break;
								case PTX_SectionFrame:
									if(eStrux2Type != PTX_EndFrame)
										continue;
									break;
								case PTX_SectionTOC:
									if(eStrux2Type != PTX_EndTOC)
										continue;
									break;
								case PTX_SectionHdrFtr:
									if(eStrux2Type != PTX_SectionHdrFtr)
										continue;
									break;
							    default:
									break;
							}

							// if we got this far, we found what we are looking for and we have the
							// correct end position
							break;
						}
						
						dposEnd = posEnd;
					}

					if(bHdrFtr)
					{
						bHdrFtr = false; // only do this once
						pf_Frag_Strux_SectionHdrFtr * pfHdr = static_cast<pf_Frag_Strux_SectionHdrFtr *>(pf1);

						const PP_AttrProp * pAP = NULL;

						if(!getAttrProp(pfHdr->getIndexAP(),&pAP) || !pAP)
							return false;

						const gchar * pszHdrId;
						if(!pAP->getAttribute("id", pszHdrId) || !pszHdrId)
							return false;

						const gchar * pszHdrType;
						if(!pAP->getAttribute("type", pszHdrType) || !pszHdrType)
							return false;

						// needs to be in this order because of undo
						_realDeleteHdrFtrStrux(static_cast<pf_Frag_Strux*>(pf1));
						_fixHdrFtrReferences(pszHdrType, pszHdrId);
					}
					else
					{
						if(!_realDeleteSpan(dpos1, dposEnd, p_AttrProp_Before,bDeleteTableStruxes,
											bDontGlob))
							return false;
					}
					
					UT_uint32 iDeleteThisStep = dposEnd - dpos1;
					
					iRealDeleteCount += iDeleteThisStep;

					// because we removed stuff, the position dpos1 remains the same and dpos2 needs
					// to be adjusted
					if(dpos2 > iDeleteThisStep)
						dpos2 -= iDeleteThisStep;
					else
						dpos2 = 0;
					
					continue;
				}
			}

			Revisions.addRevision(iId,PP_REVISION_DELETION,NULL,NULL);
			const gchar * ppRevAttrib[3];
			ppRevAttrib[0] = name;
			ppRevAttrib[1] = Revisions.getXMLstring();
			ppRevAttrib[2] = NULL;

			switch (eType)
			{
				case pf_Frag::PFT_Object:
				case pf_Frag::PFT_Text:
					if(! _realChangeSpanFmt(PTC_AddFmt, dpos1, dposEnd, ppRevAttrib,NULL,true))
						return false;
					break;

				case pf_Frag::PFT_Strux:
					// _realChangeStruxFmt() changes the strux
					// *containing* the given position, hence we pass
					// it the position immediately after the strux; we
					// only want the one strux changed, so we pass
					// identical position in both parameters
					if(! _realChangeStruxFmt(PTC_AddFmt, dpos1 + iLen, dpos1 + iLen /*2*iLen*/, ppRevAttrib,NULL,
											 eStruxType,true))
						return false;

					if(bHdrFtr)
					{
						// even though this is just a notional removal, we still have to
						// fix the references
						bHdrFtr = false; // only do this once
						pf_Frag_Strux_SectionHdrFtr * pfHdr = static_cast<pf_Frag_Strux_SectionHdrFtr *>(pf1);

						const PP_AttrProp * pAP = NULL;

						if(!getAttrProp(pfHdr->getIndexAP(),&pAP) || !pAP)
							return false;

						const gchar * pszHdrId;
						if(!pAP->getAttribute("id", pszHdrId) || !pszHdrId)
							return false;

						const gchar * pszHdrType;
						if(!pAP->getAttribute("type", pszHdrType) || !pszHdrType)
							return false;

						_fixHdrFtrReferences(pszHdrType, pszHdrId, true);
                        // empty the strux listener chache since the pointers are now
                        // invalid                                              
                        pfHdr->clearAllFmtHandles();                            
					}
					
					break;

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

/*!
    scan piecetable and remove any references to the hdr/ftr located at pos dpos
    bNotional indicates that the header has been marked deleted in revison mode, but not
    physically removed from the document
*/
bool pt_PieceTable::_fixHdrFtrReferences(const gchar * pszHdrType, const gchar * pszHdrId,
										 bool bNotional /* = false */)
{
	UT_return_val_if_fail( pszHdrType && pszHdrId, false );
	
	bool bRet = true;
	const PP_AttrProp * pAP = NULL;

	// look for any doc sections that referrence this header type and id
	const pf_Frag * pFrag = m_fragments.getFirst();
	while(pFrag)
	{
		if(pFrag->getType() == pf_Frag::PFT_Strux &&
		   static_cast<const pf_Frag_Strux*>(pFrag)->getStruxType()==PTX_Section)
		{
			if(!getAttrProp(pFrag->getIndexAP(),&pAP) || !pAP)
				continue;

			// check for normal attribute
			const gchar * pszMyHdrId2 = NULL;
			if(pAP->getAttribute(pszHdrType, pszMyHdrId2) && pszMyHdrId2)
			{
				if(0 == strcmp(pszMyHdrId2, pszHdrId))
				{
					const gchar* pAttrs [3];
					pAttrs[0] = pszHdrType;
					pAttrs[1] = pszMyHdrId2;
					pAttrs[2] = NULL;

					bRet &= _fmtChangeStruxWithNotify(PTC_RemoveFmt, (pf_Frag_Strux*)pFrag,
													  pAttrs, NULL, false);
				}
			}

			// now check for revision attribute ...
			const gchar * pszRevision;
			if(pAP->getAttribute("revision", pszRevision) && pszRevision)
			{
				bool bFound = false;
				PP_RevisionAttr Revisions(pszRevision);

				for(UT_uint32 i = 0; i < Revisions.getRevisionsCount(); ++i)
				{
					const PP_Revision * pRev2 = Revisions.getNthRevision(i);
					UT_return_val_if_fail( pRev2, false );

					const gchar * pszMyHdrId = NULL;
					if(pRev2->getAttribute(pszHdrType, pszMyHdrId) && pszMyHdrId)
					{
						if(0 != strcmp(pszHdrId, pszMyHdrId))
							continue;

						if(!bNotional)
						{
							// NB: this is safe, since we own the PP_RevisionAttr object
							// of local scope which in turn owns this revisions
							const_cast<PP_Revision*>(pRev2)->setAttribute(pszHdrType, "");
						}
						else
						{
							UT_uint32 iId = m_pDocument->getRevisionId();
							UT_uint32 iMinId;
							const PP_Revision * pRev = Revisions.getRevisionWithId(iId, iMinId);
							if(pRev)
							{
								// NB: this is safe, since we own the PP_RevisionAttr object
								// of local scope which in turn owns this revisions
								const_cast<PP_Revision*>(pRev)->setAttribute(pszHdrType, "");
							}
							else
							{
								// we have a section that references this header in
								// previous revision and has no changes in the current
								// revision, so we need to add a new revisions in which
								// the header is not referenced
								const gchar * pAttrs [3] = {pszHdrType, pszHdrId, NULL};
								Revisions.addRevision(iId, PP_REVISION_FMT_CHANGE, pAttrs, NULL);
							}
						}

						Revisions.forceDirty();
						bFound = true;
					}
				}
				
				if(bFound)
				{
					const gchar* pAttrs [3];
					pAttrs[0] = "revision";
					pAttrs[1] = Revisions.getXMLstring();
					pAttrs[2] = NULL;

					bRet &= _fmtChangeStruxWithNotify(PTC_SetFmt, (pf_Frag_Strux*)pFrag,
													  pAttrs, NULL, false);
				}
			}
								
								
		}
							
		pFrag = pFrag->getNext();
	}


	return bRet;
}

bool pt_PieceTable::_deleteSpan(pf_Frag_Text * pft, UT_uint32 fragOffset,
								PT_BufIndex bi, UT_uint32 length,
								pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd)
{
	// perform simple delete of a span of text.
	// we assume that it is completely contained within this fragment.

	UT_return_val_if_fail (fragOffset+length <= pft->getLength(),false);

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
	UT_return_val_if_fail (pftTail, false);
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

	UT_return_val_if_fail (pfs, false);

	if (length == 0)					// TODO decide if this is an error.
	{
		xxx_UT_DEBUGMSG(("_deleteSpanWithNotify: length==0\n"));
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
	UT_return_val_if_fail (pcr, false);
	pcr->setDocument(m_pDocument);
	bool bResult = _deleteSpan(pft,fragOffset,pft->getBufIndex(),length,ppfEnd,pfragOffsetEnd);

	bool canCoalesce = _canCoalesceDeleteSpan(pcr);
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

	return bResult;
}

bool pt_PieceTable::_canCoalesceDeleteSpan(PX_ChangeRecord_Span * pcrSpan) const
{
	// see if this record can be coalesced with the most recent undo record.

	UT_return_val_if_fail (pcrSpan->getType() == PX_ChangeRecord::PXT_DeleteSpan, false);

	PX_ChangeRecord * pcrUndo;
	if (!m_history.getUndo(&pcrUndo,true))
		return false;
	if (pcrSpan->getType() != pcrUndo->getType())
		return false;
	if (pcrSpan->getIndexAP() != pcrUndo->getIndexAP())
		return false;
	if((pcrUndo->isFromThisDoc() != pcrSpan->isFromThisDoc()))
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
	UT_return_val_if_fail (bFound, false);

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
	if(m_bDoNotTweakPosition)
		return true;
	
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
	UT_return_val_if_fail (bFound, false);

	pf_Frag_Strux * pfsContainer = NULL;
	bool bFoundStrux = _getStruxFromPosition(dpos1,&pfsContainer);
	UT_return_val_if_fail (bFoundStrux,false);

    _tweakFieldSpan(dpos1,dpos2);

	switch (pfsContainer->getStruxType())
	{
	default:
		UT_ASSERT_HARMLESS(0);
		return false;

	case PTX_Section:
#if 0
		// if the previous container is a section, then pf_First
		// must be the first block in the section.
		UT_return_val_if_fail ((pf_First->getPrev() == pfsContainer),false);
		UT_return_val_if_fail ((pf_First->getType() == pf_Frag::PFT_Strux),false);
		UT_return_val_if_fail (((static_cast<pf_Frag_Strux *>(pf_First))->getStruxType() == PTX_Block),false);
		// since, we cannot delete the first block in a section, we
		// secretly translate this into a request to delete the section;
		// the block we have will then be slurped into the previous
		// section.
		dpos1 -= pfsContainer->getLength();
		return true;
#endif
	case PTX_SectionHdrFtr:
		// if the previous container is a Header/Footersection, then pf_First
		// must be the first block or the first Table in the section.
		UT_return_val_if_fail ((pf_First->getPrev() == pfsContainer),false);
		UT_return_val_if_fail ((pf_First->getType() == pf_Frag::PFT_Strux),false);
		UT_return_val_if_fail ((((static_cast<pf_Frag_Strux *>(pf_First))->getStruxType() == PTX_Block) || (static_cast<pf_Frag_Strux *>(pf_First))->getStruxType() == PTX_SectionTable),false);

		//
		// This allows us to delete the first Table in a section
		//
		if(static_cast<pf_Frag_Strux *>(pf_First)->getStruxType() == PTX_SectionTable)
		{
		     return true;
		}
		// since, we cannot delete the first block in a section, we
		// secretly translate this into a request to delete the section;
		// the block we have will then be slurped into the previous
		// section.
		dpos1 -= pfsContainer->getLength();
		
		return true;

	case PTX_SectionTable:
	case PTX_SectionCell:
	case PTX_SectionFrame:
	case PTX_EndTable:
	case PTX_EndCell:
	case PTX_EndFrame:
	case PTX_SectionTOC:
	case PTX_EndTOC:
//
// We've set things up so that deleting table struxes is done very deliberately.//  Don't mess with the end points here
//
		return true;
	case PTX_SectionFootnote:
	case PTX_SectionAnnotation:
	case PTX_SectionEndnote:
	{
//
// Get the actual block strux container for the endnote. 
//
 		xxx_UT_DEBUGMSG(("_deleteSpan 1: orig pfsContainer %x type %d \n",pfsContainer,pfsContainer->getStruxType()));
 		_getStruxFromFragSkip(pfsContainer,&pfsContainer);
 		xxx_UT_DEBUGMSG(("_deleteSpan 2: After skip  pfsContainer %x type %d \n",pfsContainer,pfsContainer->getStruxType()));
 		break;
	}
 	case PTX_EndFootnote:	
 	case PTX_EndEndnote:	
 	case PTX_EndAnnotation:	
 	{
//
// Get the actual block strux container for the endnote. 
//
 		xxx_UT_DEBUGMSG(("_deleteSpan 1: orig pfsContainer %x type %d \n",pfsContainer,pfsContainer->getStruxType()));
 		_getStruxFromFragSkip(pfsContainer,&pfsContainer);
 		xxx_UT_DEBUGMSG(("_deleteSpan 2: After skip  pfsContainer %x type %d \n",pfsContainer,pfsContainer->getStruxType()));
 		break;
	}
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
			UT_return_val_if_fail (fragOffset_First == 0,false);
			if (dpos2 == dpos1 + pf_First->getLength())
			{
				//  If we are just deleting a section break, then
				//  we should delete the first block marker in the
				//  next section, combining the blocks before and
				//  after the section break.
				pf_Other = pf_First->getNext();
				UT_return_val_if_fail (pf_Other, false);
				UT_return_val_if_fail (pf_Other->getType() == pf_Frag::PFT_Strux,false);
				UT_return_val_if_fail (((static_cast<pf_Frag_Strux *>(pf_Other))->getStruxType() == PTX_Block),false);
				dpos2 += pf_Other->getLength();
				return true;
			}
		case PTX_SectionHdrFtr:
			UT_return_val_if_fail (fragOffset_First == 0, false);
			if (dpos2 == dpos1 + pf_First->getLength())
			{
				//  If we are just deleting a section break, then
				//  we should delete the first block marker in the
				//  next section, combining the blocks before and
				//  after the section break.
				pf_Other = pf_First->getNext();
				UT_return_val_if_fail (pf_Other,false);
				UT_return_val_if_fail (pf_Other->getType() == pf_Frag::PFT_Strux,false);
				UT_return_val_if_fail (((static_cast<pf_Frag_Strux *>(pf_Other))->getStruxType() == PTX_Block),false);
				dpos2 += pf_Other->getLength();
				return true;
			}

			break;
		}
	}

	if(pf_End->getType() ==  pf_Frag::PFT_Strux)
	{
	    if(static_cast<pf_Frag_Strux *>(pf_End)->getStruxType() == PTX_EndTOC)
		{
			dpos2++;
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
	if(m_bDoNotTweakPosition)
		return true;
	
	//
// First we want to handle hyperlinks. If we delete all the text within
// a hyperlink or annotation, then we should also delete the start 
// and end point of the
// hyperlink or annotation.
//
	pf_Frag * pf_First;
	pf_Frag * pf_End;
	PT_BlockOffset fragOffset_First;
	PT_BlockOffset fragOffset_End;

	bool bFound = getFragsFromPositions(dpos1,dpos2,&pf_First,&fragOffset_First,&pf_End,&fragOffset_End);
	UT_return_val_if_fail (bFound,false);
	while(pf_First && (pf_First->getLength() == 0))
	{
		pf_First = pf_First->getNext();
	}
	if(pf_First)
	{
		while(pf_End && (pf_End->getLength() == 0))
		{
			pf_End = pf_End->getPrev();
		}
		bool bDoit = false;
		if(pf_End && ((pf_End->getPos() + pf_End->getLength() - pf_First->getPos())  == (dpos2 - dpos1 +1)))
		{
			if((pf_First->getType() == pf_Frag::PFT_Text) && (pf_First->getLength() == 2))
			{
				bDoit = false;
			}
			else if((pf_First->getType() == pf_Frag::PFT_Text) && (pf_End->getType() == pf_Frag::PFT_Text) && (pf_First != pf_End))
			{
				bDoit = false;
			}
			else
			{
				bDoit = true;
			}
		}
		if(pf_End && ((pf_End->getPos() + pf_End->getLength() - pf_First->getPos())  == (dpos2 - dpos1)))
		{
			bDoit = true;
		}
		if(bDoit)
		{
//
// OK these frags are entirely contained by dpos1  and dpos2
// OK now look to see if there is a hyperlink or annotation just before and after these
//
			if(pf_End->getType() != pf_Frag::PFT_Object)
			{
				pf_End = pf_End->getNext();
			}
			while(pf_End && (pf_End->getLength() == 0))
			{
				pf_End = pf_End->getNext();
			}
			if(pf_First->getType() != pf_Frag::PFT_Object)
			{
				pf_First = pf_First->getPrev();
			}
			while(pf_First && (pf_First->getLength() == 0))
			{
				pf_First = pf_First->getPrev();
			}
			if(pf_First && (pf_First->getType() == pf_Frag::PFT_Object))
			{
				pf_Frag_Object *pFO = static_cast<pf_Frag_Object *>(pf_First);
				bool bFoundBook = false;
				bool bFoundHype = false;
				bool bFoundAnn = false;
				if(pFO->getObjectType() == PTO_Bookmark)
				{
					bFoundBook = true;
				}
				if(pFO->getObjectType() == PTO_Hyperlink)
				{
					bFoundHype = true;
				}
				if(pFO->getObjectType() == PTO_Annotation)
				{
					bFoundAnn = true;
				}
				if(pf_End && (pf_End->getType() == pf_Frag::PFT_Object) && (pf_End != pf_First))
				{
					pFO = static_cast<pf_Frag_Object *>(pf_End);
					if(pFO->getObjectType() == PTO_Bookmark && bFoundBook)
					{
//
// Found a bookmark which will have all contents deleted so delete it too
//
						dpos1--;
						dpos2++;
					}
					else if(pFO->getObjectType() == PTO_Hyperlink && bFoundHype)
					{
//
// Found a Hyperlink which will have all contents deleted so delte it too
//
						dpos1--;
						dpos2++;
					}
					else if(pFO->getObjectType() == PTO_Annotation && bFoundAnn)
					{
//
// Found a Annotation which will have all contents deleted so delte it too
//
						dpos1--;
						dpos2++;
					}
				}
			}
		}
	}
//
// Can't handle a delete span start from an endTOC. sum1 has arranged corner
// cases where this is possible. HAndle this corner case by starting at the 
// next strux
//
	if(!pf_First)
	{
	    return false;
	}
	if(pf_First->getType() == pf_Frag::PFT_Strux)
	{
		pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf_First);
		if(pfs->getStruxType() == PTX_EndTOC)
		{
			pf_Frag * pf = pf_First->getNext();
			while(pf && pf->getLength() == 0)
			{
				pf = pf->getNext();
			}
			if(pf && pf->getType() ==  pf_Frag::PFT_Strux)
			{
				pfs = static_cast<pf_Frag_Strux *>(pf);
				if(pfs->getStruxType() == PTX_Block)
				{
					dpos1++;
				}
			}
		}
	}
	//  We want to keep tweaking the delete span until there is nothing
	//  more to tweak.  We check to see if nothing has changed in the
	//  last tweak, and if so, we are done.
	while (1)
	{
		PT_DocPosition old_dpos1 = dpos1;
		PT_DocPosition old_dpos2 = dpos2;
		UT_sint32 old_iStackSize = pstDelayStruxDelete->getDepth();

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
	UT_return_val_if_fail (bFound,false);

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
			if(isEndFootnote(pfsContainerTemp))
			{
				xxx_UT_DEBUGMSG(("_deleteSpan 5: orig pfsContainer %x type %d \n",pfsContainerTemp,pfsContainerTemp->getStruxType()));
				_getStruxFromFragSkip(pfsContainerTemp,&pfsContainerTemp);
				xxx_UT_DEBUGMSG(("_deleteSpan 6: After skip  pfsContainer %x type %d \n",pfsContainerTemp,pfsContainerTemp->getStruxType()));
			}
			UT_return_val_if_fail (bFoundStrux,false);
			bool bResult = _deleteFmtMarkWithNotify(dposTemp,static_cast<pf_Frag_FmtMark *>(pfTemp),
										 pfsContainerTemp,&pfNewTemp,&fragOffsetNewTemp);
			UT_return_val_if_fail (bResult,false);

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

/*!
 * Returns true if pfs is not a strux connected with a table or frame
 */
bool pt_PieceTable::_StruxIsNotTable(pf_Frag_Strux * pfs)
{
	PTStruxType its = pfs->getStruxType();
	bool b = ((its != PTX_SectionTable) && (its != PTX_SectionCell)
			  && (its != PTX_EndTable) && (its != PTX_EndCell)
			  && (its != PTX_SectionFrame) && (its != PTX_EndFrame));
	return b;
}

/*
    Because complex span can involve deletion of a bookmark the comrade of which is outside of the
    deletion range, this function can change dpos1 and dpos2 to indicate which document positions
    after the deletion correspond to the original values passed to the function -- the caller needs
    to take this into account
 */
bool pt_PieceTable::_deleteComplexSpan(PT_DocPosition & origPos1,
									   PT_DocPosition & origPos2,
									   UT_Stack * stDelayStruxDelete)
{
	pf_Frag * pfNewEnd;
	UT_uint32 fragOffsetNewEnd;
	bool bPrevWasCell = false;
	bool bPrevWasEndTable = false;
	bool bPrevWasFrame = false;
	pf_Frag * pf_First;
	pf_Frag * pf_End;
	PT_BlockOffset fragOffset_First;
	PT_BlockOffset fragOffset_End;

	PT_DocPosition dpos1 = origPos1;
	PT_DocPosition dpos2 = origPos2;

	bool bFound = getFragsFromPositions(dpos1,dpos2,&pf_First,&fragOffset_First,&pf_End,&fragOffset_End);
	UT_return_val_if_fail (bFound, false);
	UT_DEBUGMSG(("deleteComplex span dpos1 %d dpos2 %d pf_First %p pf_First pos %d \n",dpos1,dpos2,pf_First,pf_First->getPos()));
	pf_Frag_Strux * pfsContainer = NULL;
	bool bFoundStrux = _getStruxFromPosition(dpos1,&pfsContainer);
	UT_return_val_if_fail (bFoundStrux, false);
	if(isEndFootnote(pfsContainer))
	{
		xxx_UT_DEBUGMSG(("_deleteSpan 3: orig pfsContainer %x type %d \n",pfsContainer,pfsContainer->getStruxType()));
		_getStruxFromFragSkip(pfsContainer,&pfsContainer);
		xxx_UT_DEBUGMSG(("_deleteSpan 4: After skip  pfsContainer %x type %d \n",pfsContainer,pfsContainer->getStruxType()));
	}
	// loop to delete the amount requested, one text fragment at a time.
	// if we encounter any non-text fragments along the way, we delete
	// them too.  that is, we implicitly delete Strux and Objects here.

	UT_uint32 length = dpos2 - dpos1;
	UT_uint32 iTable = 0;
	UT_sint32 iFootnoteCount = 0;
	if(pfsContainer->getStruxType() == PTX_SectionFrame)
	{
		bPrevWasFrame = true;
	        if(pf_First->getType() == pf_Frag::PFT_Strux)
		{
		     pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf_First);
		     if(pfs->getStruxType() == PTX_SectionTable)
		     {
		          bPrevWasFrame = false;
		     }
		}
	}
	if(pfsContainer->getStruxType() == PTX_SectionCell)
	{
		bPrevWasCell = true;
	}
	if(pfsContainer->getStruxType() == PTX_EndTable)
	{
		bPrevWasEndTable = true;
	}
	bool bPrevWasFootnote = false;
	UT_sint32 iLoopCount = -1;
	while (length >=0)
	{
	        if(length == 0 && iFootnoteCount <= 0)
		  break;
		iLoopCount++;
		UT_uint32 lengthInFrag = pf_First->getLength() - fragOffset_First;
		UT_uint32 lengthThisStep = UT_MIN(lengthInFrag, length);
		
		switch (pf_First->getType())
		{
		case pf_Frag::PFT_EndOfDoc:
		default:
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
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
			if(bPrevWasCell && (iLoopCount == 0))
			{
//
// Dont' delete this Block strux if the previous strux is a cell and this
// is the start of the delete phase.
//
				pfNewEnd = pfs->getNext();
				fragOffsetNewEnd = 0;
				pfsContainer = pfs;
				dpos1 = dpos1 +  lengthInFrag;
				break;
			}
			if(bPrevWasFrame && (iLoopCount == 0))
			{
//
// Dont' delete this Block strux if the previous strux is a cell and this
// is the start of the delete phase.
//
				pfNewEnd = pfs->getNext();
				fragOffsetNewEnd = 0;
				pfsContainer = pfs;
				dpos1 = dpos1 +  lengthInFrag;
				break;
			}
			if(_StruxIsNotTable(pfs))
			{
				if((bPrevWasCell || bPrevWasFootnote || bPrevWasEndTable|| bPrevWasFrame) 
				   && pfs->getStruxType()== PTX_Block)
				{
					bPrevWasCell = false;
					bPrevWasFootnote = false;
					bPrevWasEndTable = false;
					bPrevWasFrame = false;
					pfNewEnd = pfs->getNext();
					fragOffsetNewEnd = 0;
					pfsContainer = pfs;
					dpos1 = dpos1 +  lengthInFrag;
					stDelayStruxDelete->push(pfs);
				}
				else
				{
//
// Now put in code to handle deleting footnote struxtures. We have to reverse
// the order of deleting footnote and Endfootnotes.
// 
					if(!isFootnote(pfs) && !isEndFootnote(pfs))
					{ 
						UT_DEBUGMSG(("Delete Block strux dpos1 %d \n",dpos1));
						bResult = _deleteStruxWithNotify(dpos1,pfs,
										 &pfNewEnd,&fragOffsetNewEnd);
						
						if(!bResult) // can't delete this block strux
						             // but can delete the rest of the content
						{
						  UT_DEBUGMSG(("dpos1 = %d \n",dpos1));
						  pfNewEnd = pfs->getNext();
						  dpos1 += pfs->getLength();
						  pfsContainer = pfs;
						  fragOffsetNewEnd = 0;
						}
						// UT_return_val_if_fail(bResult,false);
						bPrevWasCell = false;
						bPrevWasFrame = false;
						bPrevWasEndTable = false;
						break;
					}
					else
					{
						if(isFootnote(pfs))
						{
							pfNewEnd = pfs->getNext();
							fragOffsetNewEnd = 0;
							pfsContainer = pfs;
							dpos1 = dpos1 +  lengthInFrag;
							UT_DEBUGMSG(("Push footnote strux \n"));
							stDelayStruxDelete->push(pfs);
							iFootnoteCount++;
							bPrevWasFootnote = true;
						}
						else
						{
							UT_DEBUGMSG(("Push endfootnote strux \n"));
							stDelayStruxDelete->push(pfs);
						}
					}
				}
			}
			else
			{
				if(pfs->getStruxType() == PTX_SectionCell )
				{
					bPrevWasCell = true;
				}
				else
				{
					bPrevWasCell = false;
				}
				if(pfs->getStruxType() == PTX_SectionFrame )
				{
					bPrevWasFrame = true;
				}
				else
				{
					bPrevWasFrame = false;
				}
				if(pfs->getStruxType() == PTX_EndTable)
				{
					bPrevWasEndTable = true;
				}
				else
				{
					bPrevWasEndTable = false;
				}
				if(pfs->getStruxType() == PTX_SectionTable)
				{
					iTable++;
				}
				if((pfs->getStruxType() != PTX_EndTable) || (iTable != 1))
				{
					pfNewEnd = pfs->getNext();
					fragOffsetNewEnd = 0;
					dpos1 = dpos1 + lengthInFrag;
				}
				stDelayStruxDelete->push(pfs);
			}
//
// Look to see if we've reached the end of the table, if so delete it all now
//
			pf_Frag *pff;
			PT_DocPosition dpp;
			if(pfs->getStruxType() == PTX_EndTable)
			{
				iTable--;
				if(iTable==0)
				{
//
// Deleting the table so don't delay deleting the following strux.
//
					bPrevWasEndTable = false;
					UT_DEBUGMSG(("Doing Table delete immediately \n"));
//					iTable = 1;
					UT_sint32 myTable =1;
//
// First delete the EndTable Strux
//
					stDelayStruxDelete->pop(reinterpret_cast<void **>(&pfs));
					if(m_fragments.areFragsDirty())
					{
						m_fragments.cleanFrags();
					}
					PT_DocPosition myPos2 = pfs->getPos();
					_deleteFormatting(myPos2 - pfs->getLength(), myPos2);
					UT_DEBUGMSG(("DELeteing EndTable Strux, pos= %d \n",pfs->getPos()));
					bResult = _deleteStruxWithNotify(myPos2, pfs,
													  &pfNewEnd,
													  &fragOffsetNewEnd);
					while(bResult && myTable > 0)
					{
						stDelayStruxDelete->pop(reinterpret_cast<void **>(&pfs));
						if(pfs->getStruxType() == PTX_SectionTable)
						{
							myTable--;
						}
						else if(pfs->getStruxType() == PTX_EndTable)
						{
							myTable++;
						}
						PT_DocPosition myPos = pfs->getPos();
						_deleteFormatting(myPos - pfs->getLength(), myPos);
						bResult = _deleteStruxWithNotify(myPos, pfs, &pff, &dpp);
//
// Each strux is one in length, we've added one while delaying the delete
// so subtract it now.
//
						dpos1 -= 1;
					}
//
// Now we have to update pfsContainer from dpos1
//
					bFoundStrux = _getStruxFromPosition(dpos1,&pfsContainer);
				}
				UT_DEBUGMSG(("Finished doing table delete -1 \n"));
				break;
			}
//
// Look to see if we've reached the end of a footnote section.
//
			if (isEndFootnote(pfs) && (iFootnoteCount > 0))
			{
//
// First delete the EndFootnote Strux
//
				UT_DEBUGMSG(("Doing Footnote delete immediately \n"));
				stDelayStruxDelete->pop(reinterpret_cast<void **>(&pfs));
				if(m_fragments.areFragsDirty())
				{
					m_fragments.cleanFrags();
				}
				PT_DocPosition myPos2 = pfs->getPos();
				_deleteFormatting(myPos2 - pfs->getLength(), myPos2);
				bResult = _deleteStruxWithNotify(myPos2, pfs,
												  &pfNewEnd,
												  &fragOffsetNewEnd);
//
// Now delete the Footnote strux. Doing things in this order works for
// the layout classes for both the delete (needs the endFootnote strux 
// deleted first) and for
// undo where we want the Footnote Strux inserted first.
//
				while(bResult && iFootnoteCount > 0)
				{
					stDelayStruxDelete->pop(reinterpret_cast<void **>(&pfs));
					if(isFootnote(pfs))
					{
						UT_DEBUGMSG(("Found and deleted footnote strux \n"));
						iFootnoteCount--;
					}
					else
					{
						UT_DEBUGMSG(("Found and deleted Block strux in footnote \n"));
					}
					PT_DocPosition myPos = pfs->getPos();
					if(m_fragments.areFragsDirty())
					{
						m_fragments.cleanFrags();
					}
					_deleteFormatting(myPos - pfs->getLength(), myPos);
					bResult = _deleteStruxWithNotify(myPos, pfs, &pff, &dpp);
//
// Each strux is one in length, we've added one while delaying the delete
// so subtract it now.
//
					dpos1 -= 1;
				}
//
// Now we have to update pfsContainer from dpos1
//
				bFoundStrux = _getStruxFromPosition(dpos1,&pfsContainer);
				break;
			}
			else if(isEndFootnote(pfs))
			{
//
// Attempting to delete an EndFootnote end strux without a matching begin.
// terminate the loop now.
//
				return false;

			}
//
// Look to see if we've reached the end of a Frame section.
//
			if (pfs->getStruxType() == PTX_EndFrame)
			{
//
// First delete the EndFrame Strux
//
				UT_DEBUGMSG(("Doing Frame delete now \n"));
				stDelayStruxDelete->pop(reinterpret_cast<void **>(&pfs));
				if(m_fragments.areFragsDirty())
				{
					m_fragments.cleanFrags();
				}
				PT_DocPosition myPos2 = pfs->getPos();
				_deleteFormatting(myPos2 - pfs->getLength(), myPos2);
				bool isFrame =  pfs->getStruxType() == PTX_SectionFrame;
				bResult = _deleteStruxWithNotify(myPos2, pfs,
												  &pfNewEnd,
												  &fragOffsetNewEnd);
//
// Each strux is one in length, we've added one while delaying the delete
// so subtract it now.
//
				dpos1 -= 1;
//
// Now delete the Frame strux. Doing things in this order works for
// the layout classes for both the delete (needs the endFrame strux 
// deleted first) and for
// undo where we want the Frame Strux inserted first.
//
				while(bResult && !isFrame)
				{
					stDelayStruxDelete->pop(reinterpret_cast<void **>(&pfs));
					if(m_fragments.areFragsDirty())
					{
						m_fragments.cleanFrags();
					}
					PT_DocPosition myPos = 0;
					if(pfs)
					{
						myPos = pfs->getPos();
						isFrame =  pfs->getStruxType() == PTX_SectionFrame;
						_deleteFormatting(myPos - pfs->getLength(), myPos);
						bResult = _deleteStruxWithNotify(myPos, pfs, &pff, &dpp);
					}
//
// Each strux is one in length, we've added one while delaying the delete
// so subtract it now.
//
					dpos1 -= 1;
				}
//
// Now we have to update pfsContainer from dpos1
//
				bFoundStrux = _getStruxFromPosition(dpos1,&pfsContainer);
				break;
			}
			
			UT_return_val_if_fail (bResult,false);
			// we do not update pfsContainer because we just deleted pfs.
		}
		break;

		case pf_Frag::PFT_Text:
		{
			if(isEndFootnote(pfsContainer))
			{
				_getStruxFromFragSkip(static_cast<pf_Frag *>(pfsContainer),&pfsContainer);
			}
			bool bResult = _deleteSpanWithNotify(dpos1,static_cast<pf_Frag_Text *>(pf_First),
									  fragOffset_First,lengthThisStep,
									  pfsContainer,&pfNewEnd,&fragOffsetNewEnd);
			UT_return_val_if_fail (bResult, false);
		}
		break;
		// the bookmark, hyperlink and annotation objects require specxial treatment; since
		// they come in pairs, we have to ensure that both are deleted together
		// so we have to find the other part of the pair, delete it, adjust the
		// positional variables and the delete the one we were originally asked
		// to delete
		case pf_Frag::PFT_Object:
		{
			if(isEndFootnote(pfsContainer))
			{
				_getStruxFromFragSkip(static_cast<pf_Frag *>(pfsContainer),&pfsContainer);
			}
			bool bResult = false;
			bool bResult2;
			pf_Frag_Object *pO = static_cast<pf_Frag_Object *>(pf_First);
			switch(pO->getObjectType())
			{
				case PTO_Bookmark:
				{
					bool bFoundStrux3;
					PT_DocPosition posComrade;
					pf_Frag_Strux * pfsContainer2 = NULL;

					po_Bookmark * pB = pO->getBookmark();
					UT_return_val_if_fail (pB, false);
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
								if(pB1 && !strcmp(pB->getName(),pB1->getName()))
								{
									m_pDocument->removeBookmark(pB1->getName());

									posComrade = getFragPosition(pOb);
									
									if(posComrade < origPos1)
									{
										origPos1--;
									}
									
									bFoundStrux3 = _getStruxFromFragSkip(pOb,&pfsContainer2);
									UT_return_val_if_fail (bFoundStrux3, false);

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
								if(pB1 && !strcmp(pB->getName(),pB1->getName()))
								{
									m_pDocument->removeBookmark(pB1->getName());

									posComrade = getFragPosition(pOb);
									bool bFoundStrux2 = _getStruxFromFragSkip(pOb,&pfsContainer2);
									UT_return_val_if_fail (bFoundStrux2, false);

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
				     UT_return_val_if_fail (pAP, false);
				     const gchar* pszHref = NULL;
				     const gchar* pszHname  = NULL;
				     UT_uint32 k = 0;
				     bool bStart = false;
				     while((pAP)->getNthAttribute(k++,pszHname, pszHref))
				     {
				          if(!strcmp(pszHname, "xlink:href"))
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
							bFoundStrux2 = _getStruxFromFragSkip(pOb,&pfsContainer2);
							UT_return_val_if_fail (bFoundStrux2, false);


							xxx_UT_DEBUGMSG(("Deleting End Hyperlink 1 %p \n",pOb));
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

					  xxx_UT_DEBUGMSG(("Deleting Start Hyperlink 1 %p \n",pO));
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
							  bFoundStrux2 = _getStruxFromFragSkip(pOb,&pfsContainer2);
							  UT_return_val_if_fail (bFoundStrux2, false);
							  // delete the original start marker

							  xxx_UT_DEBUGMSG(("Deleting Start Hyperlink 2 %p \n",pO));
							  bResult
							    = _deleteObjectWithNotify(dpos1,pO,fragOffset_First,lengthThisStep,
										      pfsContainer,&pfNewEnd,&fragOffsetNewEnd);

							  // now adjusting the positional variables
							  posComrade--;
							  xxx_UT_DEBUGMSG(("Deleting End Hyperlink 2 %p \n",pOb));
							  // One last twist make sure the next frag from the previous
							  // delete isn't the same as this this other we need to get the next frag from thi
							  // second delete
							  if(pfNewEnd != static_cast<pf_Frag *>(pOb))
							  {
							        bResult2 =
								  _deleteObjectWithNotify(posComrade,pOb,0,1,
											pfsContainer2,0,0);
							  }
							  else
							  {
							        bResult2 =
								  _deleteObjectWithNotify(posComrade,pOb,0,1,
											pfsContainer2,&pfNewEnd,&fragOffsetNewEnd);
							  }

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
				xxx_UT_DEBUGMSG(("Finished Deleting Hyperlink %p \n",pO));
				break;

				//
				// When deleting either the start or end of annotation we have to deleted
				// the content of the annotation as well.
				// We have to always delete the start marker first; this is
				// so that in the case of undo the endmarker would be restored
				// first, because the mechanism that marks runs between them
				// as a hyperlink depents on the end-marker being in place before
				// the start marker
				case PTO_Annotation:
				{
				    UT_ASSERT(pO->getObjectType() == PTO_Annotation);
				    bool bFoundStrux2;
				    PT_DocPosition posComrade;
				    pf_Frag_Strux * pfsContainer2 = NULL;

				    pf_Frag * pF;

				    const PP_AttrProp * pAP = NULL;
				    pO->getPieceTable()->getAttrProp(pO->getIndexAP(),&pAP);
				    UT_return_val_if_fail (pAP, false);
				    const gchar* pszHref = NULL;
				    const gchar* pszHname  = NULL;
				    UT_uint32 k = 0;
				    bool bStart = false;
				    while((pAP)->getNthAttribute(k++,pszHname, pszHref))
				    {
				      if((strcmp(pszHname, "annotation") == 0) ||(strcmp(pszHname, "Annotation") == 0) )
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
						       if(pOb->getObjectType() == PTO_Annotation)
						       {
							   posComrade = getFragPosition(pOb);
							   bFoundStrux2 = _getStruxFromFragSkip(pOb,&pfsContainer2);
							   UT_return_val_if_fail (bFoundStrux2, false);

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
					     UT_ASSERT(pO->getObjectType() == PTO_Annotation);
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
						     if(pOb->getObjectType() == PTO_Annotation)
						     {
						         posComrade = getFragPosition(pOb);
							 bFoundStrux2 = _getStruxFromFragSkip(pOb,&pfsContainer2);
							 UT_return_val_if_fail (bFoundStrux2, false);
					                // delete the original start marker
							 bResult
							   = _deleteObjectWithNotify(dpos1,pO,fragOffset_First,lengthThisStep,
										     pfsContainer,&pfNewEnd,&fragOffsetNewEnd);

							 // now adjusting the positional variables
							 posComrade--;


							 // One last twist make sure the next frag from the previous
							 // delete isn't the same as this this other we need to get the next frag from thi
							 // second delete
							 if(pfNewEnd != static_cast<pf_Frag *>(pOb))
							 {
							      bResult2 =
								_deleteObjectWithNotify(posComrade,pOb,0,1,
											pfsContainer2,0,0);
							 }
							 else
							 {
							      bResult2 =
								_deleteObjectWithNotify(posComrade,pOb,0,1,
											pfsContainer2,&pfNewEnd,&fragOffsetNewEnd);
							 }

							 // FIXME!! Need to work out how to delete the content of the annotation
							 // at this point

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
				}
				default:
					bResult
						= _deleteObjectWithNotify(dpos1,pO,fragOffset_First,lengthThisStep,
									  pfsContainer,&pfNewEnd,&fragOffsetNewEnd);

			}


			UT_return_val_if_fail (bResult, false);
		}
		break;

		case pf_Frag::PFT_FmtMark:
			// we already took care of these...
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
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
	UT_return_val_if_fail (bFound, false);

	pf_Frag_Strux * pfsContainer = NULL;
	bool bFoundStrux = _getStruxFromPosition(dpos1,&pfsContainer);
	UT_return_val_if_fail (bFoundStrux, false);

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
			UT_ASSERT_HARMLESS(0);
			return false;

		case pf_Frag::PFT_Strux:
		{
			pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *> (pf_First);

			bool bResult = _deleteStruxWithNotify(dpos1,pfs,
									   &pfNewEnd,&fragOffsetNewEnd,
									   false);
			UT_return_val_if_fail (bResult, false);
			// we do not update pfsContainer because we just deleted pfs.
		}
		break;

		case pf_Frag::PFT_Text:
		{
			bool bResult = _deleteSpanWithNotify(dpos1,
									  static_cast<pf_Frag_Text *>(pf_First),
									  fragOffset_First,lengthThisStep,
									  pfsContainer,&pfNewEnd,
									  &fragOffsetNewEnd, false);
			UT_return_val_if_fail (bResult, false);
		}
		break;

		case pf_Frag::PFT_Object:
		{
			bool bResult = _deleteObjectWithNotify(dpos1,static_cast<pf_Frag_Object *>(pf_First),
									fragOffset_First,lengthThisStep,
									pfsContainer,&pfNewEnd,&fragOffsetNewEnd,
									false);
			UT_return_val_if_fail (bResult, false);
		}
		break;

		case pf_Frag::PFT_FmtMark:
			// we already took care of these...
			UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
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

	UT_return_val_if_fail (m_pts==PTS_Editing, false);
	UT_return_val_if_fail (dpos2 > dpos1, false);

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
			getAttrProp(static_cast<pf_Frag_Text *>(pf1)->getIndexAP(), &p_AttrProp);
		  
			AttrProp_Before = *p_AttrProp;
			if(p_AttrProp_Before)
				*p_AttrProp_Before = *p_AttrProp;

			// we do not want to inherit revision attribute
			AttrProp_Before.setAttribute("revision", "");
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

		UT_sint32 oldDepth = stDelayStruxDelete.getDepth();
		bSuccess = _deleteFormatting(dpos1, dpos2);
		if (bSuccess)
		{
			bSuccess = _deleteComplexSpan(dpos1, dpos2, &stDelayStruxDelete);
		}

		bool prevDepthReached = false;
		PT_DocPosition finalPos = dpos1;
		while (bSuccess && stDelayStruxDelete.getDepth() > 0)
		{
			pf_Frag_Strux * pfs;
			if(stDelayStruxDelete.getDepth() <= oldDepth)
			{
				prevDepthReached = true;
			}
			stDelayStruxDelete.pop(reinterpret_cast<void **>(&pfs));
			if(m_fragments.areFragsDirty())
			{
				m_fragments.cleanFrags();
			}

 			pf_Frag *pf;
			PT_DocPosition dp;
			if(bDeleteTableStruxes || prevDepthReached )
			{
				if(!prevDepthReached)
				{				
					_deleteFormatting(dpos1 - pfs->getLength(), dpos1);
//
// FIXME this code should be removed if undo/redo on table manipulations
//       works fine.
#if 0
//					_deleteFormatting(myPos - pfs->getLength(), myPos);
//					bSuccess = _deleteStruxWithNotify(myPos - pfs->getLength(), pfs,
//													  &pf, &dp);
#endif

					PT_DocPosition myPos = pfs->getPos();
					bSuccess = _deleteStruxWithNotify(myPos, pfs, &pf, &dp);
				}
				else if(pfs->getPos() >= dpos1)
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

		// All text in paragraph is deleted so insert a text format
		// Unless we've deleted all text in a footnote type structure.
		// or if bDontGlob is true.
		// If we insert an FmtMark is an empty footnote it
		// will appear in the enclosing block and
		// screw up the run list.
		//
		bool bDoit = !bDontGlob;
		if(bDoit && (p_frag_after->getType() == pf_Frag::PFT_Strux))
		{
		     pf_Frag_Strux * pfsa = static_cast<pf_Frag_Strux *>(p_frag_after);
		     if(isEndFootnote(pfsa))
		     {
			 bDoit = false;
		     }
		}
		pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(p_frag_before);
		if(bDoit && ((pfs->getStruxType() == PTX_Block) || (p_frag_before->getType() == pf_Frag::PFT_EndOfDoc) ))
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


	UT_return_val_if_fail (m_pts==PTS_Editing,false);

	bool bSuccess = true;
	UT_Stack stDelayStruxDelete;

	PT_DocPosition dpos1 = getFragPosition(pf);
	UT_return_val_if_fail (dpos1,false);
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
	if(m_bDoNotTweakPosition)
		return;
	
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
	UT_return_if_fail (bFound);

	pf_Frag_Strux * pfsContainer = NULL;
	bool bFoundStrux = _getStruxFromPosition(dpos1,&pfsContainer);
	UT_return_if_fail (bFoundStrux);

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
            UT_ASSERT_HARMLESS(pft->getField() == pft2->getField());
            pft = pft2;
        }
        UT_return_if_fail (pft->getPrev()->getType() == pf_Frag::PFT_Object);
        pf_Frag_Object *pfo =
            static_cast<pf_Frag_Object *>(pft->getPrev());
        UT_return_if_fail (pfo->getObjectType()==PTO_Field);
        UT_return_if_fail (pfo->getField()==pft->getField());
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
        UT_return_if_fail (pField);
        pf_Other = pf_End->getNext();
        UT_return_if_fail (pf_Other);
        while (pf_Other->getField()==pField)
        {
            pf_Other = pf_Other->getNext();
            UT_return_if_fail (pf_Other);
        }
        dpos2 = getFragPosition(pf_Other);
    }
}
