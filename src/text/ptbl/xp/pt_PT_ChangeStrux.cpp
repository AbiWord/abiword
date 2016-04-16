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


// changeStrux-related functions for class pt_PieceTable.

#include <vector>
#include <string>
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
#include "px_CR_Strux.h"
#include "px_CR_StruxChange.h"
#include "pd_Style.h"
#include "pp_Revision.h"

/****************************************************************/
/****************************************************************/
bool pt_PieceTable::changeStruxForLists(pf_Frag_Strux* sdh,
										const char * pszParentID)
{
	return _realChangeStruxForLists(sdh, pszParentID, false);
}

bool pt_PieceTable::changeSectionAttsNoUpdate(pf_Frag_Strux * pfs,
											  const char * atts,
											  const char * attsValue)
{
	return _realChangeSectionAttsNoUpdate(pfs, atts, attsValue);
}


/*!
 * This method sends out change records to the layouts but it does put
 * revision marks on them nor are they saved in the undo stack.
 * It's used by the strux resizer for the hdrftr and maybe later for the frame.
 */
bool pt_PieceTable::changeStruxFmtNoUndo(PTChangeFmt ptc,
										 pf_Frag_Strux * pfs,
										 const PP_PropertyVector & attributes,
										 const PP_PropertyVector & properties)
{
	PT_AttrPropIndex indexNewAP;
	PTStruxType pts = pfs->getStruxType();

	PT_AttrPropIndex indexOldAP = pfs->getIndexAP();
	UT_DebugOnly<bool> bMerged;
	bMerged = m_varset.mergeAP(ptc, indexOldAP, attributes, properties, &indexNewAP, getDocument());
	UT_ASSERT_HARMLESS(bMerged);
	xxx_UT_DEBUGMSG(("Merging atts/props oldindex=%d , newindex =%d \n",indexOldAP,indexNewAP));
	if (indexOldAP == indexNewAP)		// the requested change will have no effect on this fragment.
		return true;

	// convert this fragStrux into a doc position.  we add the length
	// of the strux (in doc position coords) so that when undo looks
	// it up by position it will be to the right of the beginning of
	// the fragment and will find us -- rather than finding the end of
	// the previous fragment.

	PT_DocPosition dpos = getFragPosition(pfs) + pfs->getLength();

	PX_ChangeRecord_StruxChange * pcr
		= new PX_ChangeRecord_StruxChange(PX_ChangeRecord::PXT_ChangeStrux,
										  dpos,
										  indexOldAP,indexNewAP,pts,false);
	UT_return_val_if_fail (pcr,false);

	bool bResult;
	bResult = _fmtChangeStrux(pfs,indexNewAP);
	UT_return_val_if_fail (bResult,false);
	m_pDocument->notifyListeners(pfs,pcr);

	return true;
}

bool pt_PieceTable::changeStruxFmt(PTChangeFmt ptc,
								   PT_DocPosition dpos1,
								   PT_DocPosition dpos2,
								   const PP_PropertyVector & attributes,
								   const PP_PropertyVector & properties,
								   PTStruxType pts)
{
	bool bDoAll = (pts == PTX_StruxDummy);
	if(m_pDocument->isMarkRevisions())
	{
		pf_Frag_Strux * pfs_First;
		pf_Frag_Strux * pfs_End;
		PTStruxType ptsTemp = pts;
		if(pts==PTX_StruxDummy)
		{
			ptsTemp = PTX_Block;
		}
		if(!_getStruxOfTypeFromPosition(dpos1,ptsTemp,&pfs_First))
			return false;

		if(!_getStruxOfTypeFromPosition(dpos2,ptsTemp,&pfs_End))
			return false;

		// see if the change is exactly one block.  if so, we have
		// a simple change.  otherwise, we have a multistep change.
		bool bSimple = (pfs_First == pfs_End);

		if (!bSimple)
			beginMultiStepGlob();

		pf_Frag * pf = pfs_First;
		bool bFinished = false;

		// simple loop for normal strux change
		while (!bFinished)
		{
			switch (pf->getType())
			{
				case pf_Frag::PFT_EndOfDoc:
				default:
					UT_ASSERT_HARMLESS(0);
					return false;

				case pf_Frag::PFT_Strux:
					{
						pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf);
						if (bDoAll || (pfs->getStruxType() == pts))
						{
							bool bResult;
							// get attributes for this fragement
							const PP_AttrProp * pAP;
							const gchar * pRevision = NULL;
							const gchar name[] = "revision";

							if(getAttrProp(pfs->getIndexAP(),&pAP))
							{
								pAP->getAttribute(name, pRevision);
							}

							// if the request is for removal of fmt, in the revision mode, we still
							// have to add these props (the removal is indicated by their emptiness)
							// as we cannot rely on callers to set these correctly, we have to emtpy
							// them ourselves
							PTChangeFmt revPtc = ptc;
							PP_RevisionAttr Revisions(pRevision);
							PP_PropertyVector attrs;
							PP_PropertyVector props;

							if(ptc == PTC_RemoveFmt)
							{
								revPtc = PTC_AddFmt;

								// used to set these to NULL, but that
								// causes difficulties for attributes,
								// because the attribute value gets
								// stored directly in the hash and the
								// hash considers NULL values invalid,
								// so we are not able to retrieve them
								// (and any associated names
								attrs = PP_std_setPropsToValue(attributes, "-/-");
								props = PP_std_setPropsToValue(properties, "-/-");

							} else {
								attrs = attributes;
								props = properties;
							}

							Revisions.addRevision(m_pDocument->getRevisionId(), PP_REVISION_FMT_CHANGE, attrs, props);

							const PP_PropertyVector ppRevAttrib = {
								name, Revisions.getXMLstring()
							};
							bResult = _fmtChangeStruxWithNotify(revPtc, pfs, ppRevAttrib, PP_NOPROPS,false);
							UT_return_val_if_fail (bResult,false);
						}
						if (pfs == pfs_End)
							bFinished = true;
					}
					break;

				case pf_Frag::PFT_Object:
				case pf_Frag::PFT_Text:
				case pf_Frag::PFT_FmtMark:
					break;
			}

			pf = pf->getNext();
		}

		if (!bSimple)
			endMultiStepGlob();
		return true;
	}
	else
		return _realChangeStruxFmt(ptc, dpos1, dpos2, attributes, properties, pts, false);
}

bool pt_PieceTable::changeStruxFormatNoUpdate(PTChangeFmt ptc ,pf_Frag_Strux * pfs, const PP_PropertyVector & attributes)
{
	PT_AttrPropIndex indexNewAP;
	PT_AttrPropIndex indexOldAP = pfs->getIndexAP();
	UT_DebugOnly<bool> bMerged;
	bMerged = m_varset.mergeAP(ptc, indexOldAP, attributes, PP_NOPROPS, &indexNewAP, getDocument());
	UT_ASSERT_HARMLESS(bMerged);
	xxx_UT_DEBUGMSG(("Merging atts/props oldindex=%d , newindex =%d \n",indexOldAP,indexNewAP));
	if (indexOldAP == indexNewAP)		// the requested change will have no effect on this fragment.
		return true;

	bool bResult;
	bResult = _fmtChangeStrux(pfs,indexNewAP);
	UT_return_val_if_fail (bResult,false);

	return true;

}

bool pt_PieceTable::_fmtChangeStrux(pf_Frag_Strux * pfs,
									   PT_AttrPropIndex indexNewAP)
{
	pfs->setIndexAP(indexNewAP);
	return true;
}

bool pt_PieceTable::_fmtChangeStruxWithNotify(PTChangeFmt ptc,
											  pf_Frag_Strux * pfs,
											  const PP_PropertyVector & attributes,
											  const PP_PropertyVector & properties,
											  bool bRevisionDelete)
{
	PT_AttrPropIndex indexNewAP;
	PTStruxType pts = pfs->getStruxType();

	PT_AttrPropIndex indexOldAP = pfs->getIndexAP();
	UT_DebugOnly<bool> bMerged;
	bMerged = m_varset.mergeAP(ptc, indexOldAP, attributes, properties, &indexNewAP, getDocument());
	UT_ASSERT_HARMLESS(bMerged);
	xxx_UT_DEBUGMSG(("Merging atts/props oldindex=%d , newindex =%d \n",indexOldAP,indexNewAP));
	if (indexOldAP == indexNewAP)		// the requested change will have no effect on this fragment.
		return true;

	// convert this fragStrux into a doc position.  we add the length
	// of the strux (in doc position coords) so that when undo looks
	// it up by position it will be to the right of the beginning of
	// the fragment and will find us -- rather than finding the end of
	// the previous fragment.

	PT_DocPosition dpos = getFragPosition(pfs) + pfs->getLength();

	PX_ChangeRecord_StruxChange * pcr
		= new PX_ChangeRecord_StruxChange(PX_ChangeRecord::PXT_ChangeStrux,
										  dpos,
										  indexOldAP,indexNewAP,pts,bRevisionDelete);
	UT_return_val_if_fail (pcr,false);

	bool bResult;
	bResult = _fmtChangeStrux(pfs,indexNewAP);
	UT_return_val_if_fail (bResult,false);

	// add record to history.  we do not attempt to coalesce these.
	m_history.addChangeRecord(pcr);
	m_pDocument->notifyListeners(pfs,pcr);

	return true;
}

/*!
 * Don't broadcast changes to the endstruxs because will cause them to destroy
 * the properties of the layout since they linked to the layout.
 */
bool pt_PieceTable::_fmtChangeStruxWithNotify(PTChangeFmt ptc,
											  pf_Frag_Strux * pfs,
											  const PP_PropertyVector & attributes,
											  const PP_PropertyVector & properties,
											  bool bDoAll,
											  bool bRevisionDelete)
{
	PT_AttrPropIndex indexNewAP;
	PTStruxType pts = pfs->getStruxType();
   
	PT_AttrPropIndex indexOldAP = pfs->getIndexAP();
	UT_DebugOnly<bool> bMerged;
	bMerged = m_varset.mergeAP(ptc, indexOldAP, attributes, properties, &indexNewAP, getDocument());
	UT_ASSERT_HARMLESS(bMerged);
	xxx_UT_DEBUGMSG(("Merging atts/props oldindex=%d , newindex =%d \n",indexOldAP,indexNewAP));
	if (indexOldAP == indexNewAP)		// the requested change will have no effect on this fragment.
		return true;

	// convert this fragStrux into a doc position.  we add the length
	// of the strux (in doc position coords) so that when undo looks
	// it up by position it will be to the right of the beginning of
	// the fragment and will find us -- rather than finding the end of
	// the previous fragment.

	PT_DocPosition dpos = getFragPosition(pfs) + pfs->getLength();

	PX_ChangeRecord_StruxChange * pcr
		= new PX_ChangeRecord_StruxChange(PX_ChangeRecord::PXT_ChangeStrux,
										  dpos,
										  indexOldAP,indexNewAP,pts,bRevisionDelete);
	UT_return_val_if_fail (pcr,false);

	bool bResult;
	bResult = _fmtChangeStrux(pfs,indexNewAP);
	UT_return_val_if_fail (bResult,false);

	// add record to history.  we do not attempt to coalesce these.
	m_history.addChangeRecord(pcr);
	if(bDoAll || ( (pts != PTX_EndTable) && (pts != PTX_EndCell)
	   && (pts != PTX_EndFrame) && (pts != PTX_EndTOC) 
	   && (pts != PTX_EndFootnote) && (pts != PTX_EndEndnote)))
	{ 
		m_pDocument->notifyListeners(pfs,pcr);
	}
	return true;
}


/*!
 * This Method implements the change strux we need to reparent lists.
 */
bool pt_PieceTable::_realChangeStruxForLists(pf_Frag_Strux* sdh,
											 const char * pszParentID,
											 bool bRevisionDelete)
{
	pf_Frag_Strux * pfs = (pf_Frag_Strux *) sdh;
	PTStruxType pts = pfs->getStruxType();

	const char * attributes[3] = {PT_PARENTID_ATTRIBUTE_NAME,pszParentID,NULL};

	PT_AttrPropIndex indexNewAP;
	PT_AttrPropIndex indexOldAP = pfs->getIndexAP();
	UT_DebugOnly<bool> bMerged;
	bMerged = m_varset.mergeAP(PTC_AddFmt, indexOldAP, PP_std_copyProps(attributes), PP_NOPROPS, &indexNewAP, getDocument());
	UT_ASSERT_HARMLESS(bMerged);
	xxx_UT_DEBUGMSG(("Merging atts/props oldindex=%d , newindex =%d \n",indexOldAP,indexNewAP));
	if (indexOldAP == indexNewAP)		// the requested change will have no effect on this fragment.
		return true;

	// convert this fragStrux into a doc position.  we add the length
	// of the strux (in doc position coords) so that when undo looks
	// it up by position it will be to the right of the beginning of
	// the fragment and will find us -- rather than finding the end of
	// the previous fragment.

	PT_DocPosition dpos = getFragPosition(pfs) + pfs->getLength();

	PX_ChangeRecord_StruxChange * pcr
		= new PX_ChangeRecord_StruxChange(PX_ChangeRecord::PXT_ChangeStrux,
										  dpos,
										  indexOldAP,indexNewAP,pts,bRevisionDelete);
	UT_return_val_if_fail (pcr,false);

	bool bResult;
	bResult = _fmtChangeStrux(pfs,indexNewAP);
	UT_return_val_if_fail (bResult, false);

	// add record to history.  So we can undo it later.

	m_history.addChangeRecord(pcr);

	return true;

}


/*!
 * This Method implements the change strux we need to reparent lists.
 */
bool pt_PieceTable::_realChangeSectionAttsNoUpdate(pf_Frag_Strux * pfs,
											  const char * atts,
											  const char * attsValue)
{
	const char * attributes[3] = {atts,attsValue,NULL};

	PT_AttrPropIndex indexNewAP;
	PT_AttrPropIndex indexOldAP = pfs->getIndexAP();
	UT_DebugOnly<bool> bMerged;
	bMerged = m_varset.mergeAP(PTC_AddFmt, indexOldAP, PP_std_copyProps(attributes), PP_NOPROPS, &indexNewAP, getDocument());
	UT_ASSERT_HARMLESS(bMerged);
	xxx_UT_DEBUGMSG(("Merging atts/props oldindex=%d , newindex =%d \n",indexOldAP,indexNewAP));
	if (indexOldAP == indexNewAP)		// the requested change will have no effect on this fragment.
		return true;

	bool bResult;
	bResult = _fmtChangeStrux(pfs,indexNewAP);
	UT_return_val_if_fail (bResult,false);

	return true;

}

bool pt_PieceTable::_realChangeStruxFmt(PTChangeFmt ptc,
										PT_DocPosition dpos1,
										PT_DocPosition dpos2,
										const PP_PropertyVector & attributes,
										const PP_PropertyVector & properties,
										PTStruxType pts,
										bool bRevisionDelete)
{
	UT_return_val_if_fail (m_pts==PTS_Editing,false);
	bool bDoAll = (pts == PTX_StruxDummy);
	// apply a strux-level formatting change to the given region.

	UT_return_val_if_fail (dpos1 <= dpos2, false);
	bool bHaveAttributes, bHaveProperties;
	bHaveAttributes = (!attributes.empty());
	bHaveProperties = (!properties.empty());
	UT_return_val_if_fail (bHaveAttributes || bHaveProperties,false); // must have something to do

	pf_Frag_Strux * pfs_First;
	pf_Frag_Strux * pfs_End;

	// look backwards and find the containing strux of the given
	// type for both end points of the change.

	PTStruxType ptsTemp = pts;
	if(bDoAll)
	{
		ptsTemp = PTX_Block;
	}
	bool bApplyStyle = (ptc == PTC_AddStyle);

	// determine the first and last strux on which changes are applied
	// if the selection anchor and the caret position are both in the main text,
	// changes are not applied to embedded structures (footnotes ...)
	//
	// if the selection spans both the main text and an embedded structure, the
	// extreme strux are determined within the main text in order to capture the
	// outside block structure and the change is applied both to the main text 
	// and the embedded structures
	// NB: changes will be applied to all footnotes within a block (not only 
	// those within the selection) 
	
	bool bSkipFootnote = false;
	if ((pts != PTX_SectionTOC) && (pts != PTX_SectionFootnote) && 
		(pts != PTX_SectionEndnote) && (pts != PTX_SectionAnnotation))
	{
		bSkipFootnote = _checkSkipFootnote(dpos1,dpos2); 
	}
	bool bStopOnEndFootnote = (ptsTemp != PTX_Block);
	bool bFoundFirst;
	bool bFoundEnd;
	if (bSkipFootnote || bStopOnEndFootnote)
	{
		bFoundFirst = _getStruxOfTypeFromPosition(dpos1,ptsTemp,&pfs_First);
		bFoundEnd = _getStruxOfTypeFromPosition(dpos2,ptsTemp,&pfs_End);
	}
	else
	{
		pf_Frag * pfPos1 = NULL;
		pf_Frag * pfPos2 = NULL;
		bool bNoteFirst = isInsideFootnote(dpos1,&pfPos1);
		bool bNoteEnd = isInsideFootnote(dpos2,&pfPos2);

		if (bNoteFirst && bNoteEnd && (pfPos1 == pfPos2))
		{
			bStopOnEndFootnote = true;
			bFoundFirst = _getStruxOfTypeFromPosition(dpos1,ptsTemp,&pfs_First);
			bFoundEnd = _getStruxOfTypeFromPosition(dpos2,ptsTemp,&pfs_End);
		}
		else 
		{
			PT_BlockOffset offset;
			bool bFound;
			if (!bNoteFirst)
			{
				bFound = getFragFromPosition(dpos1,&pfPos1,&offset);
				UT_return_val_if_fail(bFound,false);
			}
			bFoundFirst = _getStruxFromFragSkip(pfPos1,&pfs_First);
			
			if (!bNoteEnd || !bApplyStyle)
			{
				// When not changing style, the algorithm exits right after
				// the end structure is found; the end block should be thus 
				// be inside the embedded structure if requested
				bFound = getFragFromPosition(dpos2,&pfPos2,&offset);
				UT_return_val_if_fail(bFound,false);
			}
			bFoundEnd = _getStruxFromFragSkip(pfPos2,&pfs_End);
		}
	}

	UT_return_val_if_fail(bFoundEnd && bFoundFirst,false);
	UT_return_val_if_fail(pfs_End->getPos() >= pfs_First->getPos(),false);

	// see if the change is exactly one block.  if so, we have
	// a simple change.  otherwise, we have a multistep change.

	// NOTE: if we call beginMultiStepGlob() we ***MUST*** call
	// NOTE: endMultiStepGlob() before we return -- otherwise,
	// NOTE: the undo/redo won't be properly bracketed.

	bool bSimple = (!bApplyStyle && (pfs_First == pfs_End));
	if (!bSimple)
		beginMultiStepGlob();

	pf_Frag * pf = pfs_First;
	bool bFinished = false;

	if (!bApplyStyle)
	{
		// simple loop for normal strux change
		while (!bFinished && pf)
		{
			switch (pf->getType())
			{
			case pf_Frag::PFT_EndOfDoc:
			default:
				UT_ASSERT_HARMLESS(0);
				return false;

			case pf_Frag::PFT_Strux:
			{
				if (bSkipFootnote && isFootnote(pf))
				{
					while(pf && !isEndFootnote(pf))
					{
						pf = pf->getNext();
					}
				}
				else
				{
					pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf);
					if (bDoAll || (pfs->getStruxType() == pts))
					{
						bool bResult;
						bResult = _fmtChangeStruxWithNotify(ptc,pfs,attributes,properties,bDoAll,bRevisionDelete);
						UT_return_val_if_fail (bResult,false);
					}
					if (pfs == pfs_End)
						bFinished = true;
				}
				break;
			}
			case pf_Frag::PFT_Object:
			case pf_Frag::PFT_Text:
			case pf_Frag::PFT_FmtMark:
				break;
			}

			pf = pf->getNext();
		}
	}
	else
	{
		// OK for styles we expand out all defined properties including BasedOn styles	                 }
		// Then we use these to eliminate any specfic properties in the current strux	 
		// Then properties in the current strux will resolve to those defined in the	 
		// style (they exist there) to specifc values in strux (if not overridden by	 
		// the style) then finally to default value.

		PTChangeFmt ptcs = PTC_RemoveFmt;
		pf_Frag_Strux * pfsContainer = pfs_First;
		pf_Frag_Strux * pfsMainBlock = NULL;
		pf_Frag * pfNewEnd = NULL;
		UT_uint32 fragOffsetNewEnd;
		PP_PropertyVector sProps;
		const gchar * sOldStyleBlock = NULL;
		const gchar * sStyleMainBlock = NULL;
		std::vector <std::string> vPropNames;
		std::string szStyle = PP_getAttribute(PT_STYLE_ATTRIBUTE_NAME,attributes);
		PD_Style * pStyle = NULL;
		getDocument()->getStyle(szStyle.c_str(), &pStyle);
		UT_return_val_if_fail (pStyle,false);
		UT_Vector vProps;
		pStyle->getAllProperties(&vProps,0);
		UT_uint32 countp = vProps.getItemCount();
		for (UT_uint32 i = 0; i < countp; i++)
		{
			sProps.push_back((const gchar *)vProps.getNthItem(i));
		}

		// Changing block style should not affect character styles
		PP_PropertyVector attrSpan = attributes;
		for(auto iter = attrSpan.begin(); iter != attrSpan.end();
			iter += 2) {
			if (*iter == PT_STYLE_ATTRIBUTE_NAME) {
				attrSpan.erase(iter, iter + 2);
				break;
			}
		}
		bool bEndSeen = false;
		bool bEndSeenMainBlock = false;

		while (!bFinished && pf)
		{
			switch (pf->getType())
			{
			case pf_Frag::PFT_EndOfDoc:
				UT_ASSERT_HARMLESS(bEndSeen);
				bFinished = true;
				break;

			case pf_Frag::PFT_Strux:
			{
				if (isFootnote(pf))
				{
					if (bSkipFootnote)
					{
						while(pf && !isEndFootnote(pf))
						{
							pf = pf->getNext();
						}
						if (!pf)
						{
							UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
							continue;
						}
					}
					else
					{
						sStyleMainBlock = sOldStyleBlock;
						pfsMainBlock = pfsContainer;
						bEndSeenMainBlock = bEndSeen;
						bEndSeen = false;
					}
				}
				else
				{
					pfsContainer = static_cast<pf_Frag_Strux *> (pf);
					if (!bEndSeen && (bDoAll || (pfsContainer->getStruxType() == pts)))
					{
						const PP_AttrProp * pAP = m_varset.getAP(pf->getIndexAP());
						pAP->getAttribute(PT_STYLE_ATTRIBUTE_NAME,sOldStyleBlock);
						bool bResult;
						bResult = _fmtChangeStruxWithNotify(ptc,pfsContainer,attributes,sProps,bRevisionDelete);
						UT_return_val_if_fail (bResult,false);
					}

					bool bEndFootnote = isEndFootnote(pf);
					if(bEndFootnote)
					{
						sOldStyleBlock = sStyleMainBlock;
						pfsContainer = pfsMainBlock;
						bEndSeen = (bEndSeen || bEndSeenMainBlock);
					}

					if (pfsContainer == pfs_End)
					{
						bEndSeen = true;
					}
					else if (bEndSeen && (!bEndFootnote || bStopOnEndFootnote))
					{
						bFinished = true;
					}
				}

				pfNewEnd = pf->getNext();
			}
			break;

			case pf_Frag::PFT_Text:
			{
				bool bResult = _fmtChangeSpanWithNotify(ptcs,static_cast<pf_Frag_Text *>(pf),
												   0,pf->getPos(),pf->getLength(), attrSpan, sProps,
												   pfsContainer,&pfNewEnd,&fragOffsetNewEnd,bRevisionDelete);
				UT_return_val_if_fail (bResult, false);
				if ((fragOffsetNewEnd > 0) && pfNewEnd->getNext())
				{
					pfNewEnd = pfNewEnd->getNext();
				}
			}
			break;

			case pf_Frag::PFT_Object:
			{
				bool bResult = _fmtChangeObjectWithNotify(ptcs, static_cast<pf_Frag_Object *>(pf),
													 0, pf->getPos(), pf->getLength(), attrSpan, sProps,
													 pfsContainer, &pfNewEnd, &fragOffsetNewEnd, bRevisionDelete);
				UT_return_val_if_fail (bResult, false);
				if ((fragOffsetNewEnd > 0) && pfNewEnd->getNext())
				{
					pfNewEnd = pfNewEnd->getNext();
				}
			}
			break;

			case pf_Frag::PFT_FmtMark:
			{
				bool bResult = _fmtChangeFmtMarkWithNotify(ptcs,static_cast<pf_Frag_FmtMark *>(pf),
													  pf->getPos(), attrSpan, sProps,
													  pfsContainer,&pfNewEnd,&fragOffsetNewEnd);
				UT_return_val_if_fail (bResult,false);
			}
			break;
			default:
				UT_ASSERT_HARMLESS(0);
				return false;
			}

			pf = pfNewEnd;
		}
	}

	if (!bSimple)
		endMultiStepGlob();

	return true;
}

bool pt_PieceTable::changeLastStruxFmtNoUndo(PT_DocPosition dpos, PTStruxType pst,
											 const PP_PropertyVector & attrs, const PP_PropertyVector & props,
											 bool bSkipEmbededSections)
{
	UT_return_val_if_fail (NULL != m_fragments.getFirst(), false);

	pf_Frag * pf = m_fragments.findFirstFragBeforePos(dpos);

	UT_return_val_if_fail ( pf, false );

	pf = _findLastStruxOfType(pf, pst, bSkipEmbededSections);
	
	UT_return_val_if_fail( pf, false );
	
	PT_AttrPropIndex currentAP = pf->getIndexAP();

	const PP_AttrProp * pOldAP;
	if(!getAttrProp(currentAP,&pOldAP))
		return false;

	PP_AttrProp * pNewAP = pOldAP->cloneWithReplacements(attrs, props, false);
	UT_return_val_if_fail(pNewAP, false);
	pNewAP->markReadOnly();

	PT_AttrPropIndex indexAP;
	if (!m_varset.addIfUniqueAP(pNewAP,&indexAP))
		return false;

	pf->setIndexAP(indexAP);

	return true;
	
}

	
bool pt_PieceTable::changeLastStruxFmtNoUndo(PT_DocPosition dpos, PTStruxType pst,
											 const PP_PropertyVector & attrs, const std::string & props,
											 bool bSkipEmbededSections)
{
	// XXX appendListStruxFmtNoUndo() has a lot of code in common.... refactor.
	if(!props.empty())
	{
		// we parse the xml props string into separate field by simply duplicating it and then
		// replacing ; and : with '0';

		// foolproofing
		const char *pProps = props.c_str();
		if(*pProps == ';')
			pProps++;

		char * pProps2 = g_strdup(pProps);

		const gchar ** pPropsArray = UT_splitPropsToArray(pProps2);
		UT_return_val_if_fail( pPropsArray, false );

		bool bRet = changeLastStruxFmtNoUndo(dpos, pst, attrs, PP_std_copyProps(pPropsArray), bSkipEmbededSections);

		delete [] pPropsArray;
		FREEP(pProps2);

		return bRet;
	}
	else
	{
		return changeLastStruxFmtNoUndo(dpos, pst, attrs, PP_NOPROPS, bSkipEmbededSections);
	}
}
