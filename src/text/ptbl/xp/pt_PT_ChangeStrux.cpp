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
#include "pf_Frag_Strux.h"
#include "pf_Frag_Strux_Block.h"
#include "pf_Frag_Strux_Section.h"
#include "pf_Frag_Text.h"
#include "pf_Fragments.h"
#include "px_ChangeRecord.h"
#include "px_CR_Strux.h"
#include "px_CR_StruxChange.h"

/****************************************************************/
/****************************************************************/

UT_Bool pt_PieceTable::_fmtChangeStrux(pf_Frag_Strux * pfs,
									   PT_AttrPropIndex indexNewAP)
{
	pfs->setIndexAP(indexNewAP);
	return UT_TRUE;
}

UT_Bool pt_PieceTable::_fmtChangeStruxWithNotify(PTChangeFmt ptc,
												 pf_Frag_Strux * pfs,
												 const XML_Char ** attributes,
												 const XML_Char ** properties)
{
	PT_AttrPropIndex indexNewAP;
	PT_AttrPropIndex indexOldAP = pfs->getIndexAP();
	UT_Bool bMerged = m_varset.mergeAP(ptc,indexOldAP,attributes,properties,&indexNewAP);
	UT_ASSERT(bMerged);

	if (indexOldAP == indexNewAP)		// the requested change will have no effect on this fragment.
		return UT_TRUE;

	// convert this fragStrux into a doc position.  we add the length
	// of the strux (in doc position coords) so that when undo looks
	// it up by position it will be to the right of the beginning of
	// the fragment and will find us -- rather than finding the end of
	// the previous fragment.
	
	PT_DocPosition dpos = getFragPosition(pfs) + pfs->getLength();
	
	PX_ChangeRecord_StruxChange * pcr
		= new PX_ChangeRecord_StruxChange(PX_ChangeRecord::PXT_ChangeStrux,
										  dpos,
										  indexOldAP,indexNewAP,
										  ptc);
	UT_ASSERT(pcr);

	if (_haveTempSpanFmt(NULL,NULL))
		clearTemporarySpanFmt();

	UT_Bool bResult = _fmtChangeStrux(pfs,indexNewAP);
	UT_ASSERT(bResult);

	// add record to history.  we do not attempt to coalesce these.
	m_history.addChangeRecord(pcr);
	m_pDocument->notifyListeners(pfs,pcr);

	return UT_TRUE;
}

UT_Bool pt_PieceTable::changeStruxFmt(PTChangeFmt ptc,
									  PT_DocPosition dpos1,
									  PT_DocPosition dpos2,
									  const XML_Char ** attributes,
									  const XML_Char ** properties,
									  PTStruxType pts)
{
	UT_ASSERT(m_pts==PTS_Editing);

	// apply a strux-level formatting change to the given region.

	UT_ASSERT(dpos1 <= dpos2);
	UT_Bool bHaveAttributes = (attributes && *attributes);
	UT_Bool bHaveProperties = (properties && *properties);
	UT_ASSERT(bHaveAttributes || bHaveProperties); // must have something to do

	pf_Frag_Strux * pfs_First;
	pf_Frag_Strux * pfs_End;

	// look backwards and find the containing strux of the given
	// type for both end points of the change.
	
	UT_Bool bFoundFirst = _getStruxOfTypeFromPosition(dpos1,pts,&pfs_First);
	UT_Bool bFoundEnd = _getStruxOfTypeFromPosition(dpos2,pts,&pfs_End);
	UT_ASSERT(bFoundFirst && bFoundEnd);
	
	// see if the change is exactly one block.  if so, we have
	// a simple change.  otherwise, we have a multistep change.

	// NOTE: if we call beginMultiStepGlob() we ***MUST*** call
	// NOTE: endMultiStepGlob() before we return -- otherwise,
	// NOTE: the undo/redo won't be properly bracketed.

	UT_Bool bSimple = (pfs_First == pfs_End);
	if (!bSimple)
		beginMultiStepGlob();

	pf_Frag * pf = pfs_First;
	UT_Bool bFinished = UT_FALSE;
	while (!bFinished)
	{
		switch (pf->getType())
		{
		case pf_Frag::PFT_EndOfDoc:
		default:
			UT_ASSERT(0);
			return UT_FALSE;
			
		case pf_Frag::PFT_Strux:
			{
				pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *>(pf);
				if (pfs->getStruxType() == pts)
				{
					UT_Bool bResult = _fmtChangeStruxWithNotify(ptc,pfs,attributes,properties);
					UT_ASSERT(bResult);
				}
				if (pfs == pfs_End)
					bFinished = UT_TRUE;
			}
			break;

		case pf_Frag::PFT_Object:
		case pf_Frag::PFT_Text:
			break;
		}

		pf = pf->getNext();
	}

	if (!bSimple)
		endMultiStepGlob();
		
	return UT_TRUE;
}

