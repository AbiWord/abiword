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


// changeStrux-related functions for class pt_PieceTable.

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
bool pt_PieceTable::changeStruxForLists(PL_StruxDocHandle sdh,
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
										 const gchar ** attributes,
										 const gchar ** properties)
{
	PT_AttrPropIndex indexNewAP;
	PTStruxType pts = pfs->getStruxType();

	PT_AttrPropIndex indexOldAP = pfs->getIndexAP();
	bool bMerged;
	bMerged = m_varset.mergeAP(ptc,indexOldAP,attributes,properties,&indexNewAP,getDocument());
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
								   const gchar ** attributes,
								   const gchar ** properties,
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
							const gchar ** attrs = attributes;
							const gchar ** props = properties;
							
							if(ptc == PTC_RemoveFmt)
							{
								revPtc = PTC_AddFmt;

								// used to set these to NULL, but that causes difficulties
								// for attributes, because the attribute value gets stored
								// directly in the hash and the hash considers NULL values
								// invalid, so we are not able to retrieve them (and any
								// associated names
								attrs = UT_setPropsToValue(attributes, "-/-");
								props = UT_setPropsToValue(properties, "-/-");

							}
							
							Revisions.addRevision(m_pDocument->getRevisionId(),PP_REVISION_FMT_CHANGE,
												  attrs,props);

							if(attrs != attributes)
								delete[] attrs;
							
							if(props != properties)
								delete[] props;
							
							const gchar * ppRevAttrib[3];
							ppRevAttrib[0] = name;
							ppRevAttrib[1] = Revisions.getXMLstring();
							ppRevAttrib[2] = NULL;
							bResult = _fmtChangeStruxWithNotify(revPtc,pfs,ppRevAttrib,NULL,false);
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

bool pt_PieceTable::changeStruxFormatNoUpdate(PTChangeFmt ptc ,pf_Frag_Strux * pfs, const gchar ** attributes)
{
	PT_AttrPropIndex indexNewAP;
	PT_AttrPropIndex indexOldAP = pfs->getIndexAP();
	bool bMerged;
	bMerged = m_varset.mergeAP(ptc,indexOldAP,attributes,NULL,&indexNewAP,getDocument());
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
											  const gchar ** attributes,
											  const gchar ** properties,
											  bool bRevisionDelete)
{
	PT_AttrPropIndex indexNewAP;
	PTStruxType pts = pfs->getStruxType();

	PT_AttrPropIndex indexOldAP = pfs->getIndexAP();
	bool bMerged;
	bMerged = m_varset.mergeAP(ptc,indexOldAP,attributes,properties,&indexNewAP,getDocument());
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
											  const gchar ** attributes,
											  const gchar ** properties,
											  bool bDoAll,
											  bool bRevisionDelete)
{
	PT_AttrPropIndex indexNewAP;
	PTStruxType pts = pfs->getStruxType();
   
	PT_AttrPropIndex indexOldAP = pfs->getIndexAP();
	bool bMerged;
	bMerged = m_varset.mergeAP(ptc,indexOldAP,attributes,properties,&indexNewAP,getDocument());
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
bool pt_PieceTable::_realChangeStruxForLists(PL_StruxDocHandle sdh,
											 const char * pszParentID,
											 bool bRevisionDelete)
{
	pf_Frag_Strux * pfs = (pf_Frag_Strux *) sdh;
	PTStruxType pts = pfs->getStruxType();

	const char * attributes[3] = {PT_PARENTID_ATTRIBUTE_NAME,pszParentID,NULL};

	PT_AttrPropIndex indexNewAP;
	PT_AttrPropIndex indexOldAP = pfs->getIndexAP();
	bool bMerged;
	bMerged = m_varset.mergeAP( PTC_AddFmt ,indexOldAP,attributes,NULL,&indexNewAP,getDocument());
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
	bool bMerged;
	bMerged = m_varset.mergeAP( PTC_AddFmt ,indexOldAP,attributes,NULL,&indexNewAP,getDocument());
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
										const gchar ** attributes,
										const gchar ** properties,
										PTStruxType pts,
										bool bRevisionDelete)
{
	UT_return_val_if_fail (m_pts==PTS_Editing,false);
	bool bDoAll = (pts == PTX_StruxDummy);
	// apply a strux-level formatting change to the given region.

	UT_return_val_if_fail (dpos1 <= dpos2, false);
	bool bHaveAttributes, bHaveProperties;
	bHaveAttributes = (attributes && *attributes);
	bHaveProperties = (properties && *properties);
	UT_return_val_if_fail (bHaveAttributes || bHaveProperties,false); // must have something to do

	pf_Frag_Strux * pfs_First;
	pf_Frag_Strux * pfs_End;

	// look backwards and find the containing strux of the given
	// type for both end points of the change.

	bool bFoundFirst;
	PTStruxType ptsTemp = pts;
	if(bDoAll)
	{
		ptsTemp = PTX_Block;
	}
	bFoundFirst = _getStruxOfTypeFromPosition(dpos1,ptsTemp,&pfs_First);
	bool bFoundEnd;
	bFoundEnd = _getStruxOfTypeFromPosition(dpos2,ptsTemp,&pfs_End);
	if(!(bFoundFirst && bFoundEnd))
	{
		UT_DEBUGMSG((" could not find bFoundFirst %d or Maybe bFoundEnd %d \n",
					 bFoundFirst,bFoundEnd));
		UT_DEBUGMSG(("Aborting attempted change. \n"));
		return false;
	}
	while(pfs_End && (pfs_End->getPos() < pfs_First->getPos() && (dpos2 >= dpos1)))
	{
		dpos2--;
		bFoundEnd = _getStruxOfTypeFromPosition(dpos2,ptsTemp,&pfs_End);
	}
	if(!pfs_End)
	{
		return false;
	}
	// see if the change is exactly one block.  if so, we have
	// a simple change.  otherwise, we have a multistep change.

	// NOTE: if we call beginMultiStepGlob() we ***MUST*** call
	// NOTE: endMultiStepGlob() before we return -- otherwise,
	// NOTE: the undo/redo won't be properly bracketed.

	bool bApplyStyle = (ptc == PTC_AddStyle);
	bool bSimple = (!bApplyStyle && (pfs_First == pfs_End));
	if (!bSimple)
		beginMultiStepGlob();

	pf_Frag * pf = pfs_First;
	bool bFinished = false;

	if (!bApplyStyle)
	{
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
						bResult = _fmtChangeStruxWithNotify(ptc,pfs,attributes,properties,bDoAll,bRevisionDelete);
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
	}
	else
	{
		// when applying a block-level style, we also need to clear
		// any props at the frag level, which might trigger coalescing,
		// thus this version of the loop is more complex.

//
// OK for styles we expand out all defined properties including BasedOn styles
// Then we use these to eliminate any specfic properties in the current strux
// Then properties in the current strux will resolve to those defined in the
// style (they exist there) to specifc values in strux (if not overridden by
// the style) then finally to default value.
//
// TODO this is not right; first of all, paragraph style should be applied
// 		to the block Strux only and nothing else -- no Spans, Fmt marks, etc.
//		Second, when applying paragraph style, we should clear the existing
//		strux of all its properties inherited from any previous style
//		not just the ones defined explicitely, by this style, because what
//		is not defined is assumed to default, not to be inherited from a style
//		we are trying to get rid off.
//
// NO. We want to remove all character level properties that clash with properties
// defined in th strux level style. -MES
//
		const gchar * szStyle = UT_getAttribute(PT_STYLE_ATTRIBUTE_NAME,attributes);

		PD_Style * pStyle = NULL;
		PTChangeFmt ptcs = PTC_RemoveFmt;
		getDocument()->getStyle(szStyle,&pStyle);
		UT_return_val_if_fail (pStyle,false);
		UT_Vector vProps;
//
// Get the vector of properties
//
		pStyle->getAllProperties(&vProps,0);
//
// Finally make the const gchar * array of properties
//
		const gchar ** sProps = NULL;
		UT_uint32 countp = vProps.getItemCount() + 1;
		sProps = (const gchar **) UT_calloc(countp, sizeof(gchar *));
		countp--;
		UT_uint32 i;
		for(i=0; i<countp; i++)
		{
			sProps[i] = (const gchar *) vProps.getNthItem(i);
		}
		sProps[i] = NULL;

		PT_DocPosition dpos = getFragPosition(pfs_First);
		pf_Frag_Strux * pfsContainer = pfs_First;
		pf_Frag * pfNewEnd = NULL;
		UT_uint32 fragOffsetNewEnd;

		bool bEndSeen = false;

		while (!bFinished)
		{
			UT_uint32 lengthThisStep = pf->getLength();

			switch (pf->getType())
			{
			case pf_Frag::PFT_EndOfDoc:
				UT_ASSERT_HARMLESS(bEndSeen);
				bFinished = true;
				break;

			default:
				UT_ASSERT_HARMLESS(0);
				return false;

			case pf_Frag::PFT_Strux:
				{
					pfNewEnd = pf->getNext();
					fragOffsetNewEnd = 0;
					pfsContainer = static_cast<pf_Frag_Strux *> (pf);
					if (!bEndSeen && (bDoAll || (pfsContainer->getStruxType() == pts)))
					{
						bool bResult;
						bResult = _fmtChangeStruxWithNotify(ptc,pfsContainer,attributes,sProps,bRevisionDelete);
						pfNewEnd = pf->getNext(); // fix 9226 change strux can delete the following object!
						UT_return_val_if_fail (bResult,false);
					}
					if(!bEndSeen && isEndFootnote(static_cast<pf_Frag *>(pfsContainer)))
					{
						_getStruxFromFragSkip(pfNewEnd,&pfsContainer);
					} 
					if (pfsContainer == pfs_End)
						bEndSeen = true;
					else if (bEndSeen)
						bFinished = true;
				}
				break;
			case pf_Frag::PFT_Text:
				{
					bool bResult;

					bResult = _fmtChangeSpanWithNotify(ptcs,static_cast<pf_Frag_Text *>(pf),
												   0,dpos,lengthThisStep,
													   attributes,sProps,
												   pfsContainer,&pfNewEnd,&fragOffsetNewEnd,bRevisionDelete);
					UT_return_val_if_fail (bResult, false);
					if (fragOffsetNewEnd > 0)
					{
						// skip over the rest of this frag since we've already
						// dealt with it.
						dpos += pfNewEnd->getLength() - fragOffsetNewEnd;
						pfNewEnd = pfNewEnd->getNext();
						fragOffsetNewEnd = 0;
					}

				}
				break;

			case pf_Frag::PFT_Object:
				{
					bool bResult;
					bResult = _fmtChangeObjectWithNotify(ptcs,static_cast<pf_Frag_Object *>(pf),
													 0,dpos,lengthThisStep,
														 attributes,sProps,
													 pfsContainer,&pfNewEnd,&fragOffsetNewEnd,bRevisionDelete);
					UT_return_val_if_fail (bResult, false);
					UT_return_val_if_fail (fragOffsetNewEnd == 0,false);
				}
				break;

			case pf_Frag::PFT_FmtMark:
				{
					bool bResult;
				 	bResult = _fmtChangeFmtMarkWithNotify(ptcs,static_cast<pf_Frag_FmtMark *>(pf),
														  dpos,
														  attributes,sProps,
													  pfsContainer,&pfNewEnd,&fragOffsetNewEnd);
					UT_return_val_if_fail (bResult,false);
				}
				break;
			}
			dpos += lengthThisStep;

			// since _fmtChange{Span,FmtMark,...}WithNotify(), can delete pf, mess with the
			// fragment list, and does some aggressive coalescing of
			// fragments, we cannot just do a pf->getNext() here.
			// to advance to the next fragment, we use the *NewEnd variables
			// that each of the cases routines gave us.
			pf = pfNewEnd;

			if (!pf)
				bFinished = true;
		}
		FREEP(sProps);
	}

	if (!bSimple)
		endMultiStepGlob();

	return true;
}

bool pt_PieceTable::changeLastStruxFmtNoUndo(PT_DocPosition dpos, PTStruxType pst,
											 const gchar ** attrs, const gchar ** props,
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

	PP_AttrProp * pNewAP = pOldAP->cloneWithReplacements(attrs,props,false);
	UT_return_val_if_fail(pNewAP, false);
	pNewAP->markReadOnly();

	PT_AttrPropIndex indexAP;
	if (!m_varset.addIfUniqueAP(pNewAP,&indexAP))
		return false;

	pf->setIndexAP(indexAP);

	return true;
	
}

	
bool pt_PieceTable::changeLastStruxFmtNoUndo(PT_DocPosition dpos, PTStruxType pst,
											 const gchar ** attrs, const gchar * props,
											 bool bSkipEmbededSections)
{
	if(props && *props)
	{
		// we parse the xml props string into separate field by simply duplicating it and then
		// replacing ; and : with '0';
	
		// foolproofing
		if(*props == ';')
			props++;
		
		char * pProps = g_strdup(props);

		const gchar ** pPropsArray = UT_splitPropsToArray(pProps);
		UT_return_val_if_fail( pPropsArray, false );
		
		bool bRet = changeLastStruxFmtNoUndo(dpos, pst, attrs, pPropsArray, bSkipEmbededSections);

		delete [] pPropsArray;
		FREEP(pProps);

		return bRet;
	}
	else
	{
		const gchar ** pPropsArray = NULL;
		return changeLastStruxFmtNoUndo(dpos, pst, attrs, pPropsArray, bSkipEmbededSections);
	}
}


