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


#include "ut_types.h"
#include "ut_vector.h"
#include "px_ChangeRecord.h"
#include "px_ChangeHistory.h"
#include "px_CR_Span.h"


// m_undoPosition is the position of the undo pointer.
// a value of zero means no undo history.
// the first undo item is at ...[m_undoPosition-1]
// the first redo item is at ...[m_undoPosition] if present.

px_ChangeHistory::px_ChangeHistory()
{
	m_undoPosition = 0;
	m_savePosition = 0;
}

px_ChangeHistory::~px_ChangeHistory()
{
	UT_VECTOR_PURGEALL(PX_ChangeRecord *,m_vecChangeRecords);
}

void px_ChangeHistory::_invalidateRedo(void)
{
	UT_uint32 kLimit = m_vecChangeRecords.getItemCount();
	UT_ASSERT(m_undoPosition <= kLimit);
	UT_uint32 k;

	// walk backwards (most recent to oldest) from the end of the
	// history and delete any redo records.
	// we do the math a little odd here because they are unsigned.
	
	for (k=kLimit; (k > m_undoPosition); k--)
	{
		PX_ChangeRecord * pcrTemp = (PX_ChangeRecord *)m_vecChangeRecords.getNthItem(k-1);
		if (!pcrTemp)
			break;
		delete pcrTemp;
		m_vecChangeRecords.deleteNthItem(k-1);
	}

	if (m_savePosition > (UT_sint32) m_undoPosition)
		m_savePosition = -1;
}

        
bool px_ChangeHistory::addChangeRecord(PX_ChangeRecord * pcr)
{
	// add a change record to the history.
	// blow away any redo, since it is now invalid.

	_invalidateRedo();
	
	bool bResult = (m_vecChangeRecords.insertItemAt(pcr,m_undoPosition++) == 0);
	UT_ASSERT(bResult);
	return bResult;
}

bool px_ChangeHistory::canDo(bool bUndo) const
{
	PX_ChangeRecord * pcr;

	return (bUndo ? getUndo(&pcr) : getRedo(&pcr));
}

UT_sint32 px_ChangeHistory::getSavePosition(void) const
{
	return m_savePosition;
}

UT_uint32 px_ChangeHistory::getUndoPos(void)
{
        return m_undoPosition;
}

void px_ChangeHistory::setSavePosition(UT_sint32 savePosition)
{
	m_savePosition = savePosition;
}


bool px_ChangeHistory::getUndo(PX_ChangeRecord ** ppcr) const
{
	if (m_undoPosition == 0)
		return false;

	PX_ChangeRecord * pcr = (PX_ChangeRecord *)m_vecChangeRecords.getNthItem(m_undoPosition-1);
	UT_ASSERT(pcr);
	*ppcr = pcr;
	return true;
}

bool px_ChangeHistory::getUndo(PX_ChangeRecord ** ppcr, UT_uint32 undoNdx) const
{
	if (m_undoPosition <= undoNdx)
		return false;

	PX_ChangeRecord * pcr = (PX_ChangeRecord *)m_vecChangeRecords.getNthItem(m_undoPosition-undoNdx-1);
	UT_ASSERT(pcr);
	*ppcr = pcr;
	return true;
}

bool px_ChangeHistory::getRedo(PX_ChangeRecord ** ppcr) const
{
	if (m_undoPosition >= m_vecChangeRecords.getItemCount())
		return false;
	PX_ChangeRecord * pcr = (PX_ChangeRecord *)m_vecChangeRecords.getNthItem(m_undoPosition);
	if (!pcr)
		return false;
	*ppcr = pcr;
	return true;
}

bool px_ChangeHistory::didUndo(void)
{
	if (m_undoPosition == 0)
		return false;
	m_undoPosition--;
	PX_ChangeRecord * pcr = (PX_ChangeRecord *)m_vecChangeRecords.getNthItem(m_undoPosition);
	if (pcr && !pcr->getPersistance())
		m_savePosition--;
	return true;
}

bool px_ChangeHistory::didRedo(void)
{
	if (m_undoPosition >= m_vecChangeRecords.getItemCount())
		return false;
	PX_ChangeRecord * pcr = (PX_ChangeRecord *)m_vecChangeRecords.getNthItem(m_undoPosition);
	m_undoPosition++;
	if (pcr && !pcr->getPersistance())
		m_savePosition++;
	return true;
}

void px_ChangeHistory::coalesceHistory(const PX_ChangeRecord * pcr)
{
	// coalesce this record with the current undo record.

	PX_ChangeRecord * pcrUndo;
	bool bResult;
	bResult = getUndo(&pcrUndo);
	UT_ASSERT(bResult);
	UT_ASSERT(pcr->getType() == pcrUndo->getType());

	switch (pcr->getType())
	{
	default:
		UT_ASSERT(0);
		return;
		
	case PX_ChangeRecord::PXT_InsertSpan:
	case PX_ChangeRecord::PXT_DeleteSpan:
		{
			const PX_ChangeRecord_Span * pcrSpan = static_cast<const PX_ChangeRecord_Span *>(pcr);
			PX_ChangeRecord_Span * pcrSpanUndo = static_cast<PX_ChangeRecord_Span *>(pcrUndo);

			_invalidateRedo();
			pcrSpanUndo->coalesce(pcrSpan);
		}
		return;
	}
}

void px_ChangeHistory::setClean(void)
{
	m_savePosition = (UT_sint32) m_undoPosition;
}

bool px_ChangeHistory::isDirty(void) const
{
	return (m_savePosition != (UT_sint32) m_undoPosition);
}


