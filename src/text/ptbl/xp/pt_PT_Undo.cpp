 
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

// undo/redo-related functions for class pt_PieceTable.

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
#include "px_ChangeRecord_StruxChange.h"
#include "px_ChangeRecord_Glob.h"
#include "px_ChangeRecord_TempSpanFmt.h"

/****************************************************************/
/****************************************************************/

UT_Bool pt_PieceTable::_doTheDo(const PX_ChangeRecord * pcr)
{
	// actually do the work of the undo or redo.

	switch (pcr->getType())
	{
	case PX_ChangeRecord::PXT_GlobMarker:
		return UT_TRUE;
		
	case PX_ChangeRecord::PXT_InsertSpan:
		{
			const PX_ChangeRecord_Span * pcrSpan = static_cast<const PX_ChangeRecord_Span *>(pcr);
			pf_Frag * pf = NULL;
			PT_BlockOffset fragOffset = 0;
			UT_Bool bFound = getFragFromPosition(pcrSpan->getPosition(),&pf,&fragOffset);
			UT_ASSERT(bFound);

			if (!_insertSpan(pf,pcrSpan->getBufIndex(),fragOffset,
							 pcrSpan->getLength(),pcrSpan->getIndexAP()))
				return UT_FALSE;

			pf_Frag_Strux * pfs = NULL;
			UT_Bool bFoundStrux = _getStruxFromPosition(pcrSpan->getPosition(),&pfs);
			UT_ASSERT(bFoundStrux);
			
			m_pDocument->notifyListeners(pfs,pcr);
		}
		return UT_TRUE;
		
	case PX_ChangeRecord::PXT_DeleteSpan:
		{
			// Our deleteSpan is much simpler than the main routine.
			// We can do this becase the change history is composed
			// of atomic operations, whereas the main routine has to
			// to deal with whatever the user chose to do (and cut
			// it into a series of steps).

			const PX_ChangeRecord_Span * pcrSpan = static_cast<const PX_ChangeRecord_Span *>(pcr);
			pf_Frag * pf = NULL;
			PT_BlockOffset fragOffset = 0;
			UT_Bool bFound = getFragFromPosition(pcrSpan->getPosition(),&pf,&fragOffset);
			UT_ASSERT(bFound);
			UT_ASSERT(pf->getType() == pf_Frag::PFT_Text);

			pf_Frag_Text * pft = static_cast<pf_Frag_Text *> (pf);
			UT_ASSERT(pft->getIndexAP() == pcrSpan->getIndexAP());

			_deleteSpan(pft,fragOffset,pcrSpan->getBufIndex(),pcrSpan->getLength(),NULL,NULL);

			pf_Frag_Strux * pfs = NULL;
			UT_Bool bFoundStrux = _getStruxFromPosition(pcrSpan->getPosition(),&pfs);
			UT_ASSERT(bFoundStrux);

			m_pDocument->notifyListeners(pfs,pcr);
		}
		return UT_TRUE;

	case PX_ChangeRecord::PXT_ChangeSpan:
		{
			// ChangeSpan is it's own inverse.  similarly, we have a much simpler
			// job than the main routine, because we have broken up the user's
			// request into atomic operations.

			const PX_ChangeRecord_SpanChange * pcrs = static_cast<const PX_ChangeRecord_SpanChange *>(pcr);

			pf_Frag * pf = NULL;
			PT_BlockOffset fragOffset = 0;
			UT_Bool bFound = getFragFromPosition(pcrs->getPosition(),&pf,&fragOffset);
			UT_ASSERT(bFound);
			UT_ASSERT(pf->getType() == pf_Frag::PFT_Text);

			pf_Frag_Text * pft = static_cast<pf_Frag_Text *> (pf);

			// we need to loop here, because even though we have a simple (atomic) change,
			// the document may be fragmented slightly differently (or rather, it may not
			// yet be possible to coalesce it (until the end of the loop)).
			
			pf_Frag * pfEnd;
			UT_uint32 fragOffsetEnd;
			UT_uint32 length = pcrs->getLength();
			while (length)
			{
				UT_uint32 lengthInFrag = pft->getLength() - fragOffset;
				UT_uint32 lengthThisStep = UT_MIN(lengthInFrag, length);

				_fmtChangeSpan(pft,fragOffset,lengthThisStep,pcrs->getIndexAP(),&pfEnd,&fragOffsetEnd);

				length -= lengthThisStep;
				if (length == 0)
					break;

				UT_ASSERT(pfEnd->getType() == pf_Frag::PFT_Text);
				pft = static_cast<pf_Frag_Text *> (pfEnd);
				fragOffset = fragOffsetEnd;
			}
			
			pf_Frag_Strux * pfs = NULL;
			UT_Bool bFoundStrux = _getStruxFromPosition(pcrs->getPosition(),&pfs);
			UT_ASSERT(bFoundStrux);

			m_pDocument->notifyListeners(pfs,pcr);
		}
		return UT_TRUE;
			
	case PX_ChangeRecord::PXT_InsertStrux:
		{
			const PX_ChangeRecord_Strux * pcrStrux = static_cast<const PX_ChangeRecord_Strux *>(pcr);
			pf_Frag_Strux * pfsNew = NULL;
			if (!_createStrux(pcrStrux->getStruxType(),pcrStrux->getIndexAP(),&pfsNew))
				return UT_FALSE;

			if (pfsNew->getStruxType() == PTX_Block)
			{
				pf_Frag_Strux_Block * pfsbNew = static_cast<pf_Frag_Strux_Block *>(pfsNew);
				pfsbNew->setPreferredSpanFmt(pcrStrux->getPreferredSpanFmt());
			}
			
			pf_Frag * pf = NULL;
			PT_BlockOffset fragOffset = 0;
			UT_Bool bFoundFrag = getFragFromPosition(pcrStrux->getPosition(),&pf,&fragOffset);
			UT_ASSERT(bFoundFrag);

			// get the strux containing the given position.
	
			pf_Frag_Strux * pfsContainer = NULL;
			UT_Bool bFoundContainer = _getStruxFromPosition(pcrStrux->getPosition(),&pfsContainer);
			UT_ASSERT(bFoundContainer);

			_insertStrux(pf,fragOffset,pfsNew);
			m_pDocument->notifyListeners(pfsContainer,pfsNew,pcr);
		}
		return UT_TRUE;
		
	case PX_ChangeRecord::PXT_DeleteStrux:
		{
			const PX_ChangeRecord_Strux * pcrStrux = static_cast<const PX_ChangeRecord_Strux *>(pcr);
			switch (pcrStrux->getStruxType())
			{
			case PTX_Block:
				{
					pf_Frag * pf = NULL;
					PT_BlockOffset fragOffset = 0;
					UT_Bool bFoundFrag = getFragFromPosition(pcrStrux->getPosition(),&pf,&fragOffset);
					UT_ASSERT(bFoundFrag);
					UT_ASSERT(pf->getType() == pf_Frag::PFT_Strux);

					pf_Frag_Strux * pfs = static_cast<pf_Frag_Strux *> (pf);
					UT_Bool bResult = _unlinkStrux_Block(pfs,NULL,NULL);
					UT_ASSERT(bResult);
					m_pDocument->notifyListeners(pfs,pcr);

					delete pfs;
				}
				break;
				
			default:
				// TODO handle the other types of strux
				UT_ASSERT(0);
				return UT_FALSE;
			}
		}
		return UT_TRUE;

	case PX_ChangeRecord::PXT_ChangeStrux:
		{
			// ChangeStrux is it's own inverse.

			const PX_ChangeRecord_StruxChange * pcrs = static_cast<const PX_ChangeRecord_StruxChange *>(pcr);
			pf_Frag_Strux * pfs;
			UT_Bool bFound = _getStruxFromPosition(pcrs->getPosition(),&pfs);
			UT_ASSERT(bFound);
			UT_Bool bResult = _fmtChangeStrux(pfs,pcrs->getIndexAP());
			UT_ASSERT(bResult);
			m_pDocument->notifyListeners(pfs,pcr);
		}
		return UT_TRUE;

	case PX_ChangeRecord::PXT_TempSpanFmt:
		{
			// TempSpanFmt is it's own inverse.
			// We don't really have anything to do here other than notify the listeners.

			const PX_ChangeRecord_TempSpanFmt * pcrTSF = static_cast<const PX_ChangeRecord_TempSpanFmt *>(pcr);
			pf_Frag_Strux * pfs;
			UT_Bool bFound = _getStruxFromPosition(pcrTSF->getPosition(),&pfs);
			UT_ASSERT(bFound);
			m_pDocument->notifyListeners(pfs,pcr);
		}
		return UT_TRUE;
	
	default:
		UT_ASSERT(0);
		return UT_FALSE;
	}
}

/*****************************************************************/
/*****************************************************************/

#define GETGLOBFLAGS(pcr)			(  (pcr->getType() == PX_ChangeRecord::PXT_GlobMarker)			\
									 ? (static_cast<PX_ChangeRecord_Glob *>((pcr))->getFlags())		\
									 : PX_ChangeRecord_Glob::PXF_Null)
#define GETREVGLOBFLAGS(pcr)		(  (pcr->getType() == PX_ChangeRecord::PXT_GlobMarker)			\
									 ? (static_cast<PX_ChangeRecord_Glob *>((pcr))->getRevFlags())	\
									 : PX_ChangeRecord_Glob::PXF_Null)

/*****************************************************************/
/*****************************************************************/
	
UT_Bool pt_PieceTable::undoCmd(void)
{
	// do a user-atomic undo.
	// return false if we can't.
	
	PX_ChangeRecord * pcr;
	if (!m_history.getUndo(&pcr))
		return UT_FALSE;
	UT_ASSERT(pcr);

	// the first undo record tells us whether it is
	// a simple change or a glob.  there are two kinds
	// of globs: a multi-step change (display atomic)
	// like deleting a selection that spans a paragraph
	// break; and a user-atomic glob like doing a search
	// and replace over the whole document.
	//
	// for a simple change, we just do it and return.
	// for a glob, we loop until we do the
	// corresponding other end.

	UT_Byte flagsFirst = GETGLOBFLAGS(pcr);

	do
	{
		PX_ChangeRecord * pcrRev = pcr->reverse(); // we must delete this.
		UT_ASSERT(pcrRev);
		UT_Byte flagsRev = GETGLOBFLAGS(pcrRev);
		UT_Bool bResult = _doTheDo(pcrRev);
		delete pcrRev;
		
		if (!bResult)
			return UT_FALSE;
		m_history.didUndo();
		if (flagsRev == flagsFirst)		// stop when we have a matching end
			break;

	} while (m_history.getUndo(&pcr));

	return UT_TRUE;
}

UT_Bool pt_PieceTable::redoCmd(void)
{
	// do a user-atomic redo.
	// return false if we can't.
	
	PX_ChangeRecord * pcr;
	if (!m_history.getRedo(&pcr))
		return UT_FALSE;
	UT_ASSERT(pcr);

	// the first undo record tells us whether it is
	// a simple change or a glob.  there are two kinds
	// of globs: a multi-step change (display atomic)
	// like deleting a selection that spans a paragraph
	// break; and a user-atomic glob like doing a search
	// and replace over the whole document.
	//
	// for a simple change, we just do it and return.
	// for a glob, we loop until we do the
	// corresponding other end.

	UT_Byte flagsRevFirst = GETREVGLOBFLAGS(pcr);

	while (m_history.getRedo(&pcr))
	{
		if (!_doTheDo(pcr))
			return UT_FALSE;
		
		m_history.didRedo();
		if (flagsRevFirst == GETGLOBFLAGS(pcr))		// stop when we have a matching end
			break;
	}

	return UT_TRUE;
}
