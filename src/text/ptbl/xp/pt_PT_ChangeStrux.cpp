 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
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
#include "pf_Frag_Strux_Column.h"
#include "pf_Frag_Strux_ColumnSet.h"
#include "pf_Frag_Strux_Section.h"
#include "pf_Frag_Text.h"
#include "pf_Fragments.h"
#include "px_ChangeRecord.h"
#include "px_ChangeRecord_Strux.h"
#include "px_ChangeRecord_StruxChange.h"

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

	PX_ChangeRecord_StruxChange * pcr
		= new PX_ChangeRecord_StruxChange(PX_ChangeRecord::PXT_ChangeStrux,
										  PX_ChangeRecord::PXF_Null,
										  getFragPosition(pfs),
										  indexOldAP,indexNewAP,
										  m_bHaveTemporarySpanFmt,UT_FALSE,
										  ptc);
	UT_ASSERT(pcr);

	if (m_bHaveTemporarySpanFmt)		// TODO decide if it correct to clear this.  if the user
		clearTemporarySpanFmt();		// TODO hits BOLD, RETURN, 'X', shouldn't 'X' be bold...

	m_history.addChangeRecord(pcr);
	UT_Bool bResult = _fmtChangeStrux(pfs,indexNewAP);
	UT_ASSERT(bResult);

	m_pDocument->notifyListeners(pfs,pcr);
	m_bHaveTemporarySpanFmt = UT_FALSE;

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

	// apply a strux-level formating change to the given region.

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

		case pf_Frag::PFT_Text:
			break;
		}

		pf = pf->getNext();
	}

	if (!bSimple)
		endMultiStepGlob();
		
	return UT_TRUE;
}

