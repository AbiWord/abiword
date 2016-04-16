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


// insertStrux-related functions for class pt_PieceTable
#include <list>
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
#include "px_CR_Strux.h"
#include "pp_Revision.h"

/****************************************************************/
/****************************************************************/

/*!
    A helper function which translates a revision attribute associated with fragment of type pts at
    pos dpos into arrays of attributes and properties suitable for passing into formating and other funcitons.

    Revisions -- an instance of an empty PP_RevisionsAttr (i.e., PP_RevisionsAttr(NULL);)
    
    ppRevAttrib -- pointers to arrays of attributes and properties; the actual props and attribs are
    ppRevProps     found inside the Revisions variable, so the returned pointers are only valid
                   within the Revisions scope !!!.

    ppAttrib -- pointers to any attributes/properties that are to be added to this revision, can be NULL
    ppProps
*/
bool pt_PieceTable::_translateRevisionAttribute(PP_RevisionAttr & Revisions, PT_AttrPropIndex indexAP,
												PP_RevisionType eType,
												PP_PropertyVector & ppRevAttrib,
												PP_PropertyVector & ppRevProps,
												const PP_PropertyVector & ppAttrib,
												const PP_PropertyVector & ppProps)
{
	UT_return_val_if_fail(m_pDocument->isMarkRevisions(),false );

	const PP_AttrProp * pRevisedAP = NULL;
	const PP_AttrProp * pAP = NULL;
	getAttrProp(indexAP, &pAP);
	const gchar name[] = "revision";

	if(pAP)
	{
		const gchar * pRev = NULL;
		if(pAP->getAttribute(name, pRev))
		{
			// OK, the previous strux had a revision attribute, which was copied into the new
			// strux. This revision attribute can contain significant properties and attributes
			// which need to be preserved (such as list ids). However, we want the revision
			// attributes from the addition and fmt records to be transfered into the regular
			// attrs and props
				
			Revisions.setRevision(pRev);
			Revisions.pruneForCumulativeResult(m_pDocument);
			pRevisedAP = Revisions.getLastRevision();

			// it is legal of pRevisedAP to be NULL here -- it simply means that the cumulative
			// effect of the revisions attribute was nothing at all (i.e., the highest revision
			// was a deletion)
			if(pRevisedAP)
			{
				PP_RevisionAttr Revisions2(NULL);

				// now add the revision attribute
				Revisions2.addRevision(m_pDocument->getRevisionId(), eType, ppAttrib, ppProps);
				const_cast<PP_AttrProp*>(pRevisedAP)->setAttribute(name, Revisions2.getXMLstring());
			}
			
		}
	}
	
	if(!pRevisedAP)
	{
		// there was either no pAP or no pRev, just add the current revision ...
		// we need to create a rev. instance in Revisions
		Revisions.addRevision(m_pDocument->getRevisionId(), eType, ppAttrib, ppProps);
		pRevisedAP = Revisions.getLastRevision();
		UT_return_val_if_fail( pRevisedAP, false );

		// now set the revision attribute of the revision
		const_cast<PP_AttrProp*>(pRevisedAP)->setAttribute(name, Revisions.getXMLstring());
	}

	ppRevAttrib = pRevisedAP->getAttributes();
	ppRevProps  = pRevisedAP->getProperties();
	
	return true;
}


bool pt_PieceTable::insertStrux(PT_DocPosition dpos,
								PTStruxType pts,
								pf_Frag_Strux ** ppfs_ret)
{
	if(m_pDocument->isMarkRevisions())
	{
		// when the strux is inserted in non-revision mode, it inherits the AP from the previous
		// strux. In revision mode this does not necessarily work because we may need to have a
		// different revision attribute. Consequently, we need to ensure that the AP that gets
		// assigned to the new strux contains all relevant attrs and props from the AP of the
		// previous strux -- we do this by obtaining the index of the AP of the previous strux and
		// running it through _translateRevisionAttribute() which will gives back all attrs and
		// props that need to be passed to _realInsertStrux()
		pf_Frag_Strux * pfsContainer = NULL;
		bool bFoundContainer = _getStruxFromPosition(dpos,&pfsContainer); // the orig. strux
		UT_return_val_if_fail(bFoundContainer, false);
	
		if(isEndFootnote(pfsContainer))
		{
			bFoundContainer = _getStruxFromFragSkip(pfsContainer,&pfsContainer);
			UT_return_val_if_fail(bFoundContainer, false);
		}

		PT_AttrPropIndex indexAP = 0;
		if (pfsContainer->getStruxType() == pts)
		{
			indexAP = pfsContainer->getIndexAP();
		}

		PP_RevisionAttr Revisions(NULL);
		PP_PropertyVector ppRevAttrib;
		PP_PropertyVector ppRevProps;
		_translateRevisionAttribute(Revisions, indexAP, PP_REVISION_ADDITION,
                                    ppRevAttrib, ppRevProps, PP_NOPROPS, PP_NOPROPS);

		//return _realChangeStruxFmt(PTC_AddFmt, dpos, dpos + iLen, ppRevAttrib,ppRevProps,pts);
		return _realInsertStrux(dpos, pts, ppRevAttrib, ppRevProps, ppfs_ret);
	}
	else
	{
		return _realInsertStrux(dpos, pts, PP_NOPROPS, PP_NOPROPS, ppfs_ret);
	}
}

bool pt_PieceTable::insertStrux(PT_DocPosition dpos,
								PTStruxType pts,
								const gchar ** attributes,
								const gchar ** properties,
								pf_Frag_Strux ** ppfs_ret)
{
	if(m_pDocument->isMarkRevisions())
	{
		// This is just like the previous method, except that in addition to calling
		// _translateRevisionAttribute() we also need to set the attrs and props
		// passed to us.
		pf_Frag_Strux * pfsContainer = NULL;
		bool bFoundContainer = _getStruxFromPosition(dpos,&pfsContainer); // the orig. strux
		UT_return_val_if_fail(bFoundContainer, false);

		if(isEndFootnote(pfsContainer))
		{
			bFoundContainer = _getStruxFromFragSkip(pfsContainer,&pfsContainer);
			UT_return_val_if_fail(bFoundContainer, false);
		}

		PT_AttrPropIndex indexAP = 0;
		if (pfsContainer->getStruxType() == pts)
		{
			indexAP = pfsContainer->getIndexAP();
		}

		PP_RevisionAttr Revisions(NULL);
		PP_PropertyVector ppRevAttrs;
		PP_PropertyVector ppRevProps;

		_translateRevisionAttribute(Revisions, indexAP, PP_REVISION_ADDITION,
									ppRevAttrs, ppRevProps, PP_NOPROPS, PP_NOPROPS);

		PP_PropertyVector ppRevAttrib = PP_std_copyProps(attributes);
		ppRevAttrib.insert(ppRevAttrib.end(), ppRevAttrs.begin(), ppRevAttrs.end());

		//return _realChangeStruxFmt(PTC_AddFmt, dpos, dpos + iLen, ppRevAttrib,NULL,pts);
		bool bRet = _realInsertStrux(dpos, pts, ppRevAttrib, PP_std_copyProps(properties), ppfs_ret);
		return bRet;
	}
	else
	{
		return _realInsertStrux(dpos, pts, PP_std_copyProps(attributes), PP_std_copyProps(properties), ppfs_ret);
	}
}


bool pt_PieceTable::_createStrux(PTStruxType pts,
									PT_AttrPropIndex indexAP,
									pf_Frag_Strux ** ppfs)
{
	// create a strux frag for this.
	// return *pfs and true if successful.

	// create an unlinked strux fragment.

	pf_Frag_Strux * pfs = NULL;
	switch (pts)
	{
	case PTX_Section:
		pfs = new pf_Frag_Strux_Section(this,indexAP);
		break;

	case PTX_Block:
		pfs = new pf_Frag_Strux_Block(this,indexAP);
		break;

	case PTX_SectionHdrFtr:
		// should this be a normal section creation instead?
		pfs = new pf_Frag_Strux_SectionHdrFtr(this,indexAP);
		break;

	case PTX_SectionFootnote:
		pfs = new pf_Frag_Strux_SectionFootnote(this, indexAP);
		break;

	case PTX_SectionAnnotation:
		pfs = new pf_Frag_Strux_SectionAnnotation(this, indexAP);
		break;

	case PTX_SectionEndnote:
		pfs = new pf_Frag_Strux_SectionEndnote(this, indexAP);
		break;

	case PTX_SectionFrame:
		pfs = new pf_Frag_Strux_SectionFrame(this, indexAP);
		break;

	case PTX_SectionTable:
		pfs = new pf_Frag_Strux_SectionTable(this, indexAP);
		break;

	case PTX_EndFrame:
		pfs = new pf_Frag_Strux_SectionEndFrame(this, indexAP);
		break;
	case PTX_SectionCell:
		pfs = new pf_Frag_Strux_SectionCell(this, indexAP);
		break;
	case PTX_SectionTOC:
		pfs = new pf_Frag_Strux_SectionTOC(this, indexAP);
		break;
	case PTX_EndTable:
		pfs = new pf_Frag_Strux_SectionEndTable(this, indexAP);
		break;
	case PTX_EndCell:
		pfs = new pf_Frag_Strux_SectionEndCell(this, indexAP);
		break;
	case PTX_EndFootnote:
		pfs = new pf_Frag_Strux_SectionEndFootnote(this, indexAP);
		break;
	case PTX_EndAnnotation:
		pfs = new pf_Frag_Strux_SectionEndAnnotation(this, indexAP);
		break;
	case PTX_EndEndnote:
		pfs = new pf_Frag_Strux_SectionEndEndnote(this, indexAP);
		break;
	case PTX_EndTOC:
		pfs = new pf_Frag_Strux_SectionEndTOC(this, indexAP);
		break;

	default:
		UT_ASSERT_HARMLESS(UT_NOT_IMPLEMENTED);
		break;
	}

	if (!pfs)
	{
		UT_DEBUGMSG(("Could not create structure fragment.\n"));
		// we forget about the AP that we created
		return false;
	}

	*ppfs = pfs;
	return true;
}

/*!
 * If we do an insert strux on a pf_Frag_Strux we actually insert the new strux
 * BEFORE pf. In this case the container is actually in strux before this one.
 * In this case pfsActual returns the rela containing strux.
 */
void pt_PieceTable::_insertStrux(pf_Frag * pf,
								 PT_BlockOffset fragOffset,
								 pf_Frag_Strux * pfsNew)
{
	// insert the new strux frag at (pf,fragOffset)
	//
	// In the case of Frames, the frame must be placed just before the next
	// strux (that's not an emebedded-type strux
	//

	if(pfsNew->getStruxType() == PTX_SectionFrame)
	{
		pf_Frag_Strux * pfsNext = NULL;
		if(pf->getType() != pf_Frag::PFT_Strux)
		{
			_getNextStruxAfterFragSkip(pf, &pfsNext);
			if(pfsNext != NULL)
			{
				pf = static_cast<pf_Frag *>(pfsNext);
			}
			if(isEndFootnote(pf))
			{
				pf = pf->getNext();
			}
			fragOffset = 0;
		}
	}
	switch (pf->getType())
	{
	default:
		UT_ASSERT_HARMLESS(0);
		return;

	case pf_Frag::PFT_Object:
	case pf_Frag::PFT_EndOfDoc:
	case pf_Frag::PFT_Strux:
		{
			// insert pfsNew before pf.
			// TODO this may introduce some oddities due to empty paragraphs.
			// TODO investigate this later.
			UT_return_if_fail (fragOffset == 0);
//
// OK find the real container strux.
//
			m_fragments.insertFrag(pf->getPrev(),pfsNew);
			return;
		}

	case pf_Frag::PFT_FmtMark:
		{
			// insert pfsNew after pf.
            // before this.
			// TODO check this.
			UT_return_if_fail (fragOffset == 0);
			m_fragments.insertFrag(pf,pfsNew);
			return;
		}

	case pf_Frag::PFT_Text:
		{
			// insert pfsNew somewhere inside pf.
			// we have a text fragment which we must deal with.
			// if we are in the middle of it, we split it.
			// if we are at one of the ends of it, we just insert
			// the fragment.

			// TODO if we are at one of the ends of the fragment,
			// TODO should we create a zero-length fragment in one
			// TODO of the paragraphs so that text typed will have
			// TODO the right attributes.

			pf_Frag_Text * pft = static_cast<pf_Frag_Text *> (pf);
			UT_uint32 fragLen = pft->getLength();
			if (fragOffset == fragLen)
			{
				// we are at the right end of the fragment.
				// insert the strux after the text fragment.

				m_fragments.insertFrag(pft,pfsNew);

				// TODO decide if we should create a zero-length
				// TODO fragment in the new paragraph to retain
				// TODO the attr/prop of the pft.
				// TODO         pf_Frag_Text * pftNew = new...
				// TODO         m_fragments.insertFrag(pfsNew,pftNew);
			}
			else if (fragOffset == 0)
			{
				// we are at the left end of the fragment.
				// insert the strux before the text fragment.

				m_fragments.insertFrag(pft->getPrev(),pfsNew);
			}
			else
			{
				// we are in the middle of a text fragment.  split it
				// and insert the new strux in between the pieces.

				UT_uint32 lenTail = pft->getLength() - fragOffset;
				PT_BufIndex biTail = m_varset.getBufIndex(pft->getBufIndex(),fragOffset);
				pf_Frag_Text * pftTail = new pf_Frag_Text(this,biTail,lenTail,pft->getIndexAP(),pft->getField());
				UT_return_if_fail (pftTail);

				pft->changeLength(fragOffset);
				m_fragments.insertFrag(pft,pfsNew);
				m_fragments.insertFrag(pfsNew,pftTail);
			}

			return;
		}
	}
}


bool pt_PieceTable::_realInsertStrux(PT_DocPosition dpos,
									 PTStruxType pts,
									 const PP_PropertyVector & attributes,
									 const PP_PropertyVector & properties,
									 pf_Frag_Strux ** ppfs_ret)
{
	// insert a new structure fragment at the given document position.
	// this function can only be called while editing the document.
	// Also can specify an indexAP to be used for the frag rather
	// than that obtained by default. Very useful for insertting
	// Cells where you can immediately specify the cell position in
	// a table.  this function can only be called while editing the
	// document.

	UT_return_val_if_fail (m_pts==PTS_Editing, false);

	// get the fragment at the doc postion containing the given
	// document position.

	pf_Frag * pf = NULL;
	PT_BlockOffset fragOffset = 0;
	bool bFoundFrag = getFragFromPosition(dpos,&pf,&fragOffset);
	UT_return_val_if_fail (bFoundFrag, false);

	// get the strux containing the given position.

	pf_Frag_Strux * pfsContainer = NULL;
	bool bFoundContainer = _getStruxFromPosition(dpos,&pfsContainer);
	UT_return_val_if_fail (bFoundContainer,false);
	//
	// Can only insert an endTOC into a TOC
	//
	if((pfsContainer->getStruxType() == PTX_SectionTOC) && (pts != PTX_EndTOC))
	{
		bFoundContainer = _getStruxFromPosition(pfsContainer->getPos(),&pfsContainer);
		dpos--;
	}
	if(isEndFootnote(pfsContainer))
	{
		bFoundContainer = _getStruxFromFragSkip(pfsContainer,&pfsContainer);
	}
	// if we are inserting something similar to the previous strux,
	// we will clone the attributes/properties; we assume that the
	// new strux should have the same AP as the one which preceeds us.
	// This is generally true for inserting a paragraph -- it should
	// inherit the style of the one we just broke.

	PT_AttrPropIndex indexAP = 0;
	if (pfsContainer->getStruxType() == pts)
	{
		// TODO paul, add code here to see if this strux has a "followed-by"
		// TODO paul, property (or property in the style) and get the a/p
		// TODO paul, from there rather than just taking the attr/prop
		// TODO paul, of the previous strux.
		indexAP = pfsContainer->getIndexAP();
	}

//
// Look to see if we're in the middle of a hyperlink span now.
//
	pf_Frag * pHype = _findPrevHyperlink(pf);
	if(pHype != NULL && (pts != PTX_SectionFrame) // allow annotations in
	                                              // hyperlinks
	   && (pts != PTX_SectionAnnotation)
	   && (pts != PTX_EndAnnotation)) // frames are always placed
		                                           // at the end of blocks
                                                   // so we don't need this 
	{
//
// We have an open hyperlink! FIXME later we should allow this by terminating
// the hyperlink span just before this strux, then doing the insert strux.
// Instead for now we'll just disallow this insertStrux.
//
// This assert is to remind use to write the code to terminate
// the hyperlink.
//
		pf_Frag * pEndHype = _findNextHyperlink(pf);
		PT_DocPosition posEnd = 0;
		if(pEndHype)
		{
			posEnd = pEndHype->getPos();
		}
		//
		// OK now insert a new end of hyperlink at pf
		//
		insertObject(dpos, PTO_Hyperlink, PP_NOPROPS, PP_NOPROPS);
		dpos++;
		if(posEnd > 0)
		{
			//
			// Now delete the old endhyperlink.
			//
			pf_Frag * pfEnd = NULL;
			UT_uint32 newOff = 0;
			posEnd++; // from the insert
			UT_uint32 offset = 0;
			_deleteObjectWithNotify(posEnd,
									static_cast<pf_Frag_Object*>(pEndHype),
									offset,1,pfsContainer,&pfEnd,&newOff,true);
		}
		bFoundFrag = getFragFromPosition(dpos,&pf,&fragOffset);
		UT_return_val_if_fail (bFoundFrag, false);
	}

//
// If desired, merge in the specified attributes/properties. This
// enables cells to inherit the properties of the block from which
// they were inserted.
//
	if (!attributes.empty() || !properties.empty())
	{
		PT_AttrPropIndex pAPIold = indexAP;
		bool bMerged = m_varset.mergeAP(PTC_AddFmt, pAPIold, attributes, properties, &indexAP, getDocument());
        UT_UNUSED(bMerged);
		UT_ASSERT_HARMLESS(bMerged);
	}

	pf_Frag_Strux * pfsNew = NULL;
	if (!_createStrux(pts,indexAP,&pfsNew))
		return false;

	pfsNew->setXID(getXID());

	// when inserting paragraphs, we try to remember the current
	// span formatting active at the insertion point and add a
	// FmtMark immediately after the block.  this way, if the
	// user keeps typing text, the FmtMark will control it's
	// attr/prop -- if the user warps away and/or edits elsewhere
	// and then comes back to this point (the FmtMark may or may
	// not still be here) new text will either use the FmtMark or
	// look to the right.
	// Do not copy FmtMark if this is the first block of a footnote, 
	// or an endnote

	bool bNeedGlob = false;
	PT_AttrPropIndex apFmtMark = 0;
	if (pfsNew->getStruxType() == PTX_Block && !isFootnote(pfsContainer))
	{
		bNeedGlob = _computeFmtMarkForNewBlock(pfsNew,pf,fragOffset,&apFmtMark);
		if (bNeedGlob)
			beginMultiStepGlob();

		// if we are leaving an empty block (are stealing all it's content) we should
		// put a FmtMark in it to remember the active span fmt at the time.
		// this lets things like hitting two consecutive CR's and then comming
		// back to the first empty paragraph behave as expected.

		// fixme sevior here

		if ((pf->getType()==pf_Frag::PFT_Text) && (fragOffset == 0) &&
			(pf->getPrev()!=NULL) && (pf->getPrev()->getType()==pf_Frag::PFT_Strux))
		{
			pf_Frag_Strux *pfsStrux = static_cast<pf_Frag_Strux *>(pf->getPrev());
			if(pfsStrux->getStruxType() == PTX_Block)
			{
				_insertFmtMarkAfterBlockWithNotify(pfsContainer,dpos,apFmtMark);
			}
		}
	}
	//
	// Look if we're placing an endcell in an empty block. If so, 
	// insert a format mark
	//
	if (pfsNew->getStruxType() == PTX_EndCell)
	{
		if((pf->getPrev()!=NULL) && (pf->getPrev()->getType()==pf_Frag::PFT_Strux))
		{
			pf_Frag_Strux *pfsStrux = static_cast<pf_Frag_Strux *>(pf->getPrev());
			if(pfsStrux->getStruxType() == PTX_Block)
			{
				_insertFmtMarkAfterBlockWithNotify(pfsContainer,dpos,apFmtMark);
			}
		}
	}

	// insert this frag into the fragment list. Update the container strux as needed
	_insertStrux(pf,fragOffset,pfsNew);
	if (ppfs_ret)
		*ppfs_ret = pfsNew;

	// insert frag in the embedded_strux list if needed
	if ((pts == PTX_EndFootnote) || (pts == PTX_EndEndnote) || (pts == PTX_EndAnnotation)) 
	{
		_insertNoteInEmbeddedStruxList(pfsNew);
	}

	// create a change record to describe the change, add
	// it to the history, and let our listeners know about it.
	if(pfsNew->getStruxType() == PTX_SectionFrame)
	{
		// Inserting a sectionFrame screws up dos. It goes just before the next
		// block strux found.
		dpos = pfsNew->getPrev()->getPos() + pfsNew->getPrev()->getLength();
	}
	PX_ChangeRecord_Strux * pcrs
		= new PX_ChangeRecord_Strux(PX_ChangeRecord::PXT_InsertStrux,
									dpos,indexAP,pfsNew->getXID(), pts);
	UT_return_val_if_fail (pcrs,false);

	// add record to history.  we do not attempt to coalesce these.
	m_history.addChangeRecord(pcrs);
	m_pDocument->notifyListeners(pfsContainer,pfsNew,pcrs);

	if (bNeedGlob)
	{
		// dpos might have shifted if a frame was moved in between the two blocks
		dpos = pfsNew->getPos();
		UT_return_val_if_fail (!pfsNew->getNext() || pfsNew->getNext()->getType()!=pf_Frag::PFT_FmtMark, false);
		_insertFmtMarkAfterBlockWithNotify(pfsNew,dpos+pfsNew->getLength(),apFmtMark);
		endMultiStepGlob();
	}

	return true;
}

bool pt_PieceTable::_computeFmtMarkForNewBlock(pf_Frag_Strux * /* pfsNewBlock */,
												  pf_Frag * pfCurrent, PT_BlockOffset fragOffset,
												  PT_AttrPropIndex * pFmtMarkAP)
{
	*pFmtMarkAP = 0;

	// pfsNewBlock will soon be inserted at [pfCurrent,fragOffset].
	// look at the attr/prop and/or style on this block and see if we should
	// create a FmtMark based upon it.  then look to previous blocks for
	// information to create one.

	// TODO paul, if we set a style on this block and it implies a span-level
	// TODO paul, format, create the proper FmtMark and return TRUE here rather
	// TODO paul, than looking backwards.

	// next we look backwards for an active FmtMark or Text span.

	pf_Frag * pfPrev;
	if ((fragOffset!=0) || (pfCurrent->getType()==pf_Frag::PFT_Text))
		pfPrev = pfCurrent;
	else if (pfCurrent->getLength()==0)
		pfPrev = pfCurrent;
	else
		pfPrev = pfCurrent->getPrev();

	for (/*pfPrev*/; (pfPrev); pfPrev=pfPrev->getPrev())
	{
		switch (pfPrev->getType())
		{
		default:
			{
				UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
				return false;
			}

		case pf_Frag::PFT_Text:
			{
				pf_Frag_Text * pfPrevText = static_cast<pf_Frag_Text *>(pfPrev);
				*pFmtMarkAP = pfPrevText->getIndexAP();
				return true;
			}

		case pf_Frag::PFT_Object:
			{
				// this might not be the right thing to do.  Referencing
				// the span-level formatting for a Field is probably OK,
				// but referencing the span-level formatting of an Image
				// is probably bogus.
				pf_Frag_Object * pfPrevObject = static_cast<pf_Frag_Object *>(pfPrev);
				switch (pfPrevObject->getObjectType())
				{
				case PTO_Field:
					*pFmtMarkAP = pfPrevObject->getIndexAP();
					return true;

				default:					// keep looking back
					break;
				}
			}

		case pf_Frag::PFT_Strux:
			{
				return false;
			}

		case pf_Frag::PFT_FmtMark:
			{
				// this one is easy.
				pf_Frag_FmtMark * pfPrevFM = static_cast<pf_Frag_FmtMark *>(pfPrev);
				*pFmtMarkAP = pfPrevFM->getIndexAP();
				return true;
			}

		case pf_Frag::PFT_EndOfDoc:
			{
				break;						// keep looking back
			}
		}
	}

	return false;
}


bool pt_PieceTable::_insertNoteInEmbeddedStruxList(pf_Frag_Strux * pfsNew)
{
	pf_Frag * pfPrev = pfsNew->getPrev();
	pf_Frag_Strux * pfsPrev = NULL;
	while(pfPrev)
	{
		if (pfPrev->getType() == pf_Frag::PFT_Strux) 
		{
			pfsPrev = static_cast <pf_Frag_Strux *> (pfPrev);
			if ((pfsPrev->getStruxType() == PTX_SectionFootnote) ||
				(pfsPrev->getStruxType() == PTX_SectionEndnote) ||
				(pfsPrev->getStruxType() == PTX_SectionAnnotation))
			{
				break;
			}
		}
		pfPrev = pfPrev->getPrev();
	}
	if (pfsPrev)
	{
		embeddedStrux newNote;
		newNote.beginNote = pfsPrev;
		newNote.endNote = pfsNew;
		newNote.type = pfsPrev->getStruxType();
		bool bNoteInserted = false;
		if (!m_embeddedStrux.empty())
		{
			std::list<embeddedStrux>::iterator it;
			for (it = m_embeddedStrux.begin(); it != m_embeddedStrux.end(); ++it)
			{
				if (pfsPrev->getPos() < (*it).beginNote->getPos())
				{
					m_embeddedStrux.insert(it,newNote);
					bNoteInserted = true;
					break;
				}
			}
		}
		if (!bNoteInserted)
		{
			m_embeddedStrux.push_back(newNote);
		}
		return true;
	}
	else
	{
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		return false;
	}
}
