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


// Glob-related routines for class pt_PieceTable.

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
#include "px_ChangeRecord_Glob.h"

/****************************************************************/
/****************************************************************/

void pt_PieceTable::beginMultiStepGlob(void)
{
	// a 'multi-step-glob' is a change record marker used to indicate
	// the start of a set of edits which we created to represent a
	// single insert/delete/change.  the insert/delete/change code
	// must break certain operations into multiple steps -- when they
	// cross multiple fragments, for example.  these records are used
	// to bracket the undo/redo.

	// TODO decide if we want to have the view/formatter suppress
	// TODO screen activity during a multi-step change (and only
	// TODO update the screen after the last step).

	PX_ChangeRecord * pcr = new PX_ChangeRecord_Glob(PX_ChangeRecord::PXT_GlobMarker,
													 PX_ChangeRecord_Glob::PXF_MultiStepStart);
	UT_ASSERT(pcr);

	// add record to history.  we do not attempt to coalesce these.
	m_history.addChangeRecord(pcr);
	m_pDocument->notifyListeners(NULL,pcr);
}

void pt_PieceTable::endMultiStepGlob(void)
{
	PX_ChangeRecord * pcr = new PX_ChangeRecord_Glob(PX_ChangeRecord::PXT_GlobMarker,
													 PX_ChangeRecord_Glob::PXF_MultiStepEnd);
	UT_ASSERT(pcr);

	// add record to history.  we do not attempt to coalesce these.
	m_history.addChangeRecord(pcr);
	m_pDocument->notifyListeners(NULL,pcr);
}

void pt_PieceTable::beginUserAtomicGlob(void)
{
	// a 'user-atomic-glob' is a change record marker used to indicate
	// the start of a set of edits which the user will probably consider
	// an atomic operation.  we leave it upto the view/layout/formatter
	// to decide what that is.  this is used to glob events for the
	// purposes of UNDO/REDO -- this might be word globbing of a contiguous
	// sequence of keystokes or bracket a global search/replace.
	//
	// we do not notify the listeners.
	
	PX_ChangeRecord * pcr = new PX_ChangeRecord_Glob(PX_ChangeRecord::PXT_GlobMarker,
													 PX_ChangeRecord_Glob::PXF_UserAtomicStart);
	UT_ASSERT(pcr);

	// add record to history.  we do not attempt to coalesce these.
	m_history.addChangeRecord(pcr);
	m_pDocument->notifyListeners(NULL,pcr);
}

void pt_PieceTable::endUserAtomicGlob(void)
{
	PX_ChangeRecord * pcr = new PX_ChangeRecord_Glob(PX_ChangeRecord::PXT_GlobMarker,
													 PX_ChangeRecord_Glob::PXF_UserAtomicEnd);
	UT_ASSERT(pcr);

	// add record to history.  we do not attempt to coalesce these.
	m_history.addChangeRecord(pcr);
	m_pDocument->notifyListeners(NULL,pcr);
}
