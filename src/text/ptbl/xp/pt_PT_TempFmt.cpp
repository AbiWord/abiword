 
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

// temporary-format-related functions for class pt_PieceTable.

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
#include "px_ChangeRecord_Span.h"
#include "px_ChangeRecord_SpanChange.h"
#include "px_ChangeRecord_Strux.h"
#include "px_ChangeRecord_TempSpanFmt.h"

/****************************************************************/
/****************************************************************/

// The 'temporarySpanFmt' is used to capture format Attribute/Properties
// in anticipation of content which will use it.  This is used, for
// example, when the user hits the BOLD button (without a selection)
// and then begins typing text.  The new text will appear in bold, but
// if the user moves off (cursor motion or mouse) and then moves back
// and then types, the text will not appear in bold.  This is consistent
// with MSWord and MSWordPad behavior.

UT_Bool pt_PieceTable::_setTemporarySpanFmtWithNotify(PTChangeFmt ptc,
													  PT_DocPosition dpos,
													  const XML_Char ** attributes,
													  const XML_Char ** properties)
{
	// create an unreferenced indexAP containing the merger of the
	// given attributes/properties and the current A/P at the given
	// text position.  create a _TempSpanFmt ChangeRecord.
	//
	// the listeners will have to cache this value to let them know
	// how to toggle the various toolbar and menu items state.
	//
	// because we don't link this into the fragment list (or even
	// create a fragment for it), getSpanAttrProp() won't know about
	// it -- this is good because we want the draw code to be ignorant
	// about this.
	//
	// we do add a change record to the history to allow us to undo/redo
	// this.

	// first, see if we already have a temporary span fmt.  if so, and
	// we are at the same document position, we do a join on the A/P.
	// if it is at a different position, we clear it.
	
	PT_DocPosition dposTemp;
	if (_haveTempSpanFmt(&dposTemp,NULL))
		if (dposTemp != dpos)
			clearTemporarySpanFmt();

	// get the fragment at the given document position.
	
	pf_Frag * pf = NULL;
	PT_BlockOffset fragOffset = 0;
	UT_Bool bFound = getFragFromPosition(dpos,&pf,&fragOffset);
	UT_ASSERT(bFound);

	PT_AttrPropIndex indexNewAP;
	PT_AttrPropIndex indexOldAP = 0;

	if (_haveTempSpanFmt(NULL,&indexOldAP))
	{
		// we were given what we need.
	}
	else
	{
		// we want to bases things off the attributes of the the
		// text immediately to our left, if present.
		if ((pf->getType() == pf_Frag::PFT_Text) && (fragOffset > 0))
		{
			indexOldAP = (static_cast<pf_Frag_Text *>(pf))->getIndexAP();
		}
		// TODO make this look-back cross a paragraph boundary...
		else if (pf->getPrev() && (pf->getPrev()->getType() == pf_Frag::PFT_Text))
		{
			indexOldAP = (static_cast<pf_Frag_Text *>(pf->getPrev()))->getIndexAP();
		}
	}

	UT_Bool bMerged = m_varset.mergeAP(ptc,indexOldAP,attributes,properties,&indexNewAP);
	UT_ASSERT(bMerged);

	if (indexOldAP == indexNewAP)		// the requested change will have no effect on this fragment.
		return UT_TRUE;

	PX_ChangeRecord_TempSpanFmt * pcr
		= new PX_ChangeRecord_TempSpanFmt(PX_ChangeRecord::PXT_TempSpanFmt,
										  dpos,indexNewAP,UT_TRUE);
	UT_ASSERT(pcr);

	pf_Frag_Strux * pfs = NULL;
	UT_Bool bFoundStrux = _getStruxFromPosition(dpos,&pfs);
	UT_ASSERT(bFoundStrux);

	m_pDocument->notifyListeners(pfs,pcr);
	m_history.addChangeRecord(pcr);

	return UT_TRUE;
}

void pt_PieceTable::clearTemporarySpanFmt(void)
{
	// we put _TempSpanFmt ChangeRecords into the history
	// so that things like
	//    <bold><italic>X<undo><undo>Y<undo><undo><undo>Z
	// will produce a bold-italic 'X' then a bold 'Y' and then a plain 'Z'.
	// but if the user does
	//    X<bold><italic><cursor-motion>
	// we want to truncate the history back to the 'X'.  we do this because
	// we want an undo to erase the 'X' rather than warp back to the previous
	// insertion point and remove the <italic>.
	//
	// we do not notify the listeners.  this introduces the possiblity
	// of stale toolbars when we have multiple windows/views on the document,
	// but i'm not sure if it will ever happen.
	// TODO verify this once we get multiple windows working.

	while (_haveTempSpanFmt(NULL,NULL))
		m_history.didUndo();
}

UT_Bool pt_PieceTable::_haveTempSpanFmt(PT_DocPosition * pdpos, PT_AttrPropIndex * papi) const
{
	PX_ChangeRecord * pcr;
	UT_Bool bResult = (m_history.getUndo(&pcr) && (pcr->getType() == PX_ChangeRecord::PXT_TempSpanFmt));
	if (bResult && papi)
		*papi = pcr->getIndexAP();
	if (bResult && pdpos)
		*pdpos = pcr->getPosition();
	
	return bResult;
}
