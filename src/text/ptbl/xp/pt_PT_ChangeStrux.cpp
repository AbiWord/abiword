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
	return _realChangeStruxForLists(sdh, pszParentID);
}

bool pt_PieceTable::changeSectionAttsNoUpdate(pf_Frag_Strux * pfs,
											  const char * atts,
											  const char * attsValue)
{
	return _realChangeSectionAttsNoUpdate(pfs, atts, attsValue);
}

bool pt_PieceTable::changeStruxFmt(PTChangeFmt ptc,
									  PT_DocPosition dpos1,
									  PT_DocPosition dpos2,
									  const XML_Char ** attributes,
									  const XML_Char ** properties,
								   PTStruxType pts)
{
	if(m_pDocument->isMarkRevisions())
	{
		pf_Frag_Strux * pfs_First;
		pf_Frag_Strux * pfs_End;

		if(!_getStruxOfTypeFromPosition(dpos1,pts,&pfs_First))
			return false;

		if(!_getStruxOfTypeFromPosition(dpos2,pts,&pfs_End))
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
					UT_ASSERT(0);
					return false;

				case pf_Frag::PFT_Strux:
					{
						pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf);
						if (pfs->getStruxType() == pts)
						{
							bool bResult;
							// get attributes for this fragement
							const PP_AttrProp * pAP;
							const XML_Char * pRevision = NULL;
							const XML_Char name[] = "revision";

							if(getAttrProp(pfs->getIndexAP(),&pAP))
							{
								pAP->getAttribute(name, pRevision);
							}

							PP_RevisionAttr Revisions(pRevision);
							Revisions.addRevision(m_pDocument->getRevisionId(),PP_REVISION_FMT_CHANGE,attributes,properties);

							const XML_Char * ppRevAttrib[3];
							ppRevAttrib[0] = name;
							ppRevAttrib[1] = Revisions.getXMLstring();
							ppRevAttrib[2] = NULL;


							bResult = _fmtChangeStruxWithNotify(ptc,pfs,ppRevAttrib,NULL);
							UT_ASSERT(bResult);
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
		return _realChangeStruxFmt(ptc, dpos1, dpos2, attributes, properties, pts);
}

bool pt_PieceTable::_fmtChangeStrux(pf_Frag_Strux * pfs,
									   PT_AttrPropIndex indexNewAP)
{
	pfs->setIndexAP(indexNewAP);
	return true;
}

bool pt_PieceTable::_fmtChangeStruxWithNotify(PTChangeFmt ptc,
												 pf_Frag_Strux * pfs,
												 const XML_Char ** attributes,
												 const XML_Char ** properties)
{
	PT_AttrPropIndex indexNewAP;
	PT_AttrPropIndex indexOldAP = pfs->getIndexAP();
	bool bMerged;
	bMerged = m_varset.mergeAP(ptc,indexOldAP,attributes,properties,&indexNewAP,getDocument());
	UT_ASSERT(bMerged);
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
										  indexOldAP,indexNewAP);
	UT_ASSERT(pcr);

	bool bResult;
	bResult = _fmtChangeStrux(pfs,indexNewAP);
	UT_ASSERT(bResult);

	// add record to history.  we do not attempt to coalesce these.
	m_history.addChangeRecord(pcr);
	m_pDocument->notifyListeners(pfs,pcr);

	return true;
}

/*!
 * This Method implements the change strux we need to reparent lists.
 */
bool pt_PieceTable::_realChangeStruxForLists(PL_StruxDocHandle sdh,
									   const char * pszParentID)
{
	pf_Frag_Strux * pfs = (pf_Frag_Strux *) sdh;
	const char * attributes[3] = {PT_PARENTID_ATTRIBUTE_NAME,pszParentID,NULL};

	PT_AttrPropIndex indexNewAP;
	PT_AttrPropIndex indexOldAP = pfs->getIndexAP();
	bool bMerged;
	bMerged = m_varset.mergeAP( PTC_AddFmt ,indexOldAP,attributes,NULL,&indexNewAP,getDocument());
	UT_ASSERT(bMerged);
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
										  indexOldAP,indexNewAP);
	UT_ASSERT(pcr);

	bool bResult;
	bResult = _fmtChangeStrux(pfs,indexNewAP);
	UT_ASSERT(bResult);

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
	UT_ASSERT(bMerged);
	xxx_UT_DEBUGMSG(("Merging atts/props oldindex=%d , newindex =%d \n",indexOldAP,indexNewAP));
	if (indexOldAP == indexNewAP)		// the requested change will have no effect on this fragment.
		return true;

	bool bResult;
	bResult = _fmtChangeStrux(pfs,indexNewAP);
	UT_ASSERT(bResult);

	return true;

}

bool pt_PieceTable::_realChangeStruxFmt(PTChangeFmt ptc,
									  PT_DocPosition dpos1,
									  PT_DocPosition dpos2,
									  const XML_Char ** attributes,
									  const XML_Char ** properties,
									  PTStruxType pts)
{
	UT_ASSERT(m_pts==PTS_Editing);

	// apply a strux-level formatting change to the given region.

	UT_ASSERT(dpos1 <= dpos2);
	bool bHaveAttributes, bHaveProperties;
	bHaveAttributes = (attributes && *attributes);
	bHaveProperties = (properties && *properties);
	UT_ASSERT(bHaveAttributes || bHaveProperties); // must have something to do

	pf_Frag_Strux * pfs_First;
	pf_Frag_Strux * pfs_End;

	// look backwards and find the containing strux of the given
	// type for both end points of the change.

	bool bFoundFirst;
	bFoundFirst = _getStruxOfTypeFromPosition(dpos1,pts,&pfs_First);
	bool bFoundEnd;
	bFoundEnd = _getStruxOfTypeFromPosition(dpos2,pts,&pfs_End);
	UT_ASSERT(bFoundFirst && bFoundEnd);
	while(pfs_End->getPos() < pfs_First->getPos() && (dpos2 >= dpos1))
	{
		dpos2--;
		bFoundEnd = _getStruxOfTypeFromPosition(dpos2,pts,&pfs_End);
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
				UT_ASSERT(0);
				return false;

			case pf_Frag::PFT_Strux:
				{
					pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf);
					if (pfs->getStruxType() == pts)
					{
						bool bResult;
						bResult = _fmtChangeStruxWithNotify(ptc,pfs,attributes,properties);
						UT_ASSERT(bResult);
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
		const XML_Char * szStyle = UT_getAttribute(PT_STYLE_ATTRIBUTE_NAME,attributes);

		PD_Style * pStyle = NULL;
		PTChangeFmt ptcs = PTC_RemoveFmt;
		getDocument()->getStyle(szStyle,&pStyle);
		UT_ASSERT(pStyle);
		UT_Vector vProps;
//
// Get the vector of properties
//
		pStyle->getAllProperties(&vProps,0);
//
// Finally make the const XML_Char * array of properties
//
		const XML_Char ** sProps = NULL;
		UT_uint32 countp = vProps.getItemCount() + 1;
		sProps = (const XML_Char **) UT_calloc(countp, sizeof(XML_Char *));
		countp--;
		UT_uint32 i;
		for(i=0; i<countp; i++)
		{
			sProps[i] = (const XML_Char *) vProps.getNthItem(i);
		}
		sProps[i] = NULL;

		PT_DocPosition dpos = getFragPosition(pfs_First);
		pf_Frag_Strux * pfsContainer = pfs_First;
		pf_Frag * pfNewEnd;
		UT_uint32 fragOffsetNewEnd;

		bool bEndSeen = false;

		while (!bFinished)
		{
			UT_uint32 lengthThisStep = pf->getLength();

			switch (pf->getType())
			{
			case pf_Frag::PFT_EndOfDoc:
				UT_ASSERT(bEndSeen);
				bFinished = true;
				break;

			default:
				UT_ASSERT(0);
				return false;

			case pf_Frag::PFT_Strux:
				{
					pfNewEnd = pf->getNext();
					fragOffsetNewEnd = 0;
					pfsContainer = static_cast<pf_Frag_Strux *> (pf);
					if (!bEndSeen && (pfsContainer->getStruxType() == pts))
					{
						bool bResult;
						bResult = _fmtChangeStruxWithNotify(ptc,pfsContainer,attributes,sProps);
						UT_ASSERT(bResult);
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
												   pfsContainer,&pfNewEnd,&fragOffsetNewEnd);
					UT_ASSERT(bResult);
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
													 pfsContainer,&pfNewEnd,&fragOffsetNewEnd);
					UT_ASSERT(bResult);
					UT_ASSERT(fragOffsetNewEnd == 0);
				}
				break;

			case pf_Frag::PFT_FmtMark:
				{
					bool bResult;
				 	bResult = _fmtChangeFmtMarkWithNotify(ptcs,static_cast<pf_Frag_FmtMark *>(pf),
														  dpos,
														  attributes,sProps,
													  pfsContainer,&pfNewEnd,&fragOffsetNewEnd);
					UT_ASSERT(bResult);
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














