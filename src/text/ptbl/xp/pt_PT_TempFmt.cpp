 
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
													  UT_Bool bLeftSide,
													  const XML_Char ** attributes,
													  const XML_Char ** properties)
{
	// create an unreferenced indexAP containing the merger of the
	// given attributes/properties and the current A/P at the given
	// text position.  fire a change record to the listeners of a
	// zero length changeSpan at the position.
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

	pf_Frag_Strux * pfs;
	pf_Frag_Text * pft;
	PT_BlockOffset fragOffset;
	UT_Bool bFound = getTextFragFromPosition(dpos,bLeftSide,&pfs,&pft,&fragOffset);
	UT_ASSERT(bFound);

	PT_AttrPropIndex indexNewAP;
	PT_AttrPropIndex indexOldAP;
	if (m_bHaveTemporarySpanFmt)
		indexOldAP = m_indexAPTemporarySpanFmt;
	else
		indexOldAP = pft->getIndexAP();
	UT_Bool bMerged = m_varset.mergeAP(ptc,indexOldAP,attributes,properties,&indexNewAP);
	UT_ASSERT(bMerged);

	if (indexOldAP == indexNewAP)		// the requested change will have no effect on this fragment.
		return UT_TRUE;

	PX_ChangeRecord_SpanChange * pcr
		= new PX_ChangeRecord_SpanChange(PX_ChangeRecord::PXT_ChangeSpan,
										 PX_ChangeRecord::PXF_Null,
										 dpos,bLeftSide,
										 indexOldAP,indexNewAP,
										 m_bHaveTemporarySpanFmt,UT_TRUE,
										 ptc,
										 m_varset.getBufIndex(pft->getBufIndex(),fragOffset),
										 0);
	UT_ASSERT(pcr);
	m_history.addChangeRecord(pcr);
	_setTemporarySpanFmt(indexNewAP,dpos);
	m_pDocument->notifyListeners(pfs,pcr);
	
	return UT_TRUE;
}

void pt_PieceTable::_setTemporarySpanFmt(PT_AttrPropIndex indexNewAP,
										 PT_DocPosition dpos)
{
	m_bHaveTemporarySpanFmt = UT_TRUE;
	m_indexAPTemporarySpanFmt = indexNewAP;
	m_dposTemporarySpanFmt = dpos;
}

void pt_PieceTable::clearTemporarySpanFmt(void)
{
	// we put zero length SpanChange ChangeRecords in the history
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
	if (!m_bHaveTemporarySpanFmt)
		return;

	PX_ChangeRecord * pcr;
	while (m_bHaveTemporarySpanFmt && m_history.getUndo(&pcr))
	{
		UT_ASSERT(pcr->getType() == PX_ChangeRecord::PXT_ChangeSpan);
		PX_ChangeRecord_SpanChange * pcrs = static_cast<PX_ChangeRecord_SpanChange *>(pcr);

		UT_ASSERT(pcrs->getLength() == 0);
		m_bHaveTemporarySpanFmt = pcrs->getTempBefore();
		m_history.didUndo();
	}

	UT_ASSERT(!m_bHaveTemporarySpanFmt);
}
