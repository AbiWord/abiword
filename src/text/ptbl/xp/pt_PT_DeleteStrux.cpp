 
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

UT_Bool pt_PieceTable::_unlinkStrux_Block(pf_Frag_Strux * pfs,
										  pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd)
{
	// unlink this strux from the document.
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

	switch (pfsPrev->getStruxType())
	{
	case PTX_Block:
		// if there is a paragraph before us, we can delete this
		// paragraph knowing that our content will be assimilated
		// in to the previous one.

		_unlinkFrag(pfs,ppfEnd,pfragOffsetEnd);
		return UT_TRUE;

	case PTX_Section:
	case PTX_ColumnSet:
	case PTX_Column:
		// we are the first paragraph in this section.  if we have
		// content, we cannot be deleted, since there is no one to
		// inherit our content.

		if (_struxHasContent(pfs))
		{
			// TODO decide if this should assert or just fail...
			UT_DEBUGMSG(("Cannot delete first paragraph with content.\n"));
			UT_ASSERT(0);
			return UT_FALSE;
		}

		// no content in this paragraph.
		
		_unlinkFrag(pfs,ppfEnd,pfragOffsetEnd);
		return UT_TRUE;

	default:
		UT_ASSERT(0);
		return UT_FALSE;
	}
}
			
UT_Bool pt_PieceTable::_deleteStruxWithNotify(PT_DocPosition dpos,
											  pf_Frag_Strux * pfs,
											  pf_Frag ** ppfEnd, UT_uint32 * pfragOffsetEnd)
{
	switch (pfs->getStruxType())
	{
	case PTX_Section:
	case PTX_ColumnSet:
	case PTX_Column:
		UT_ASSERT(0);					// TODO
		break;
		
	case PTX_Block:
		if (!_unlinkStrux_Block(pfs,ppfEnd,pfragOffsetEnd))
			return UT_FALSE;
		break;
		
	default:
		UT_ASSERT(0);
		return UT_FALSE;
	}

	// create a change record to describe the change, add
	// it to the history, and let our listeners know about it.
	
	PX_ChangeRecord_Strux * pcrs
		= new PX_ChangeRecord_Strux(PX_ChangeRecord::PXT_DeleteStrux,
									dpos,
									m_indexAPTemporarySpanFmt,pfs->getIndexAP(),
									m_bHaveTemporarySpanFmt,UT_FALSE,
									pfs->getStruxType());
	UT_ASSERT(pcrs);
	m_history.addChangeRecord(pcrs);
	m_pDocument->notifyListeners(pfs,pcrs);
	m_bHaveTemporarySpanFmt = UT_FALSE;

	delete pfs;

	return UT_TRUE;
}
