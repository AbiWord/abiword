 
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

#include "ut_types.h"
#include "ut_vector.h"
#include "px_ChangeRecord.h"
#include "px_ChangeHistory.h"
#include "px_ChangeRecord_Span.h"


// m_undoPosition is the position of the undo pointer.
// a value of zero means no undo history.
// the first undo item is at ...[m_undoPosition-1]
// the first redo item is at ...[m_undoPosition] if present.

px_ChangeHistory::px_ChangeHistory()
{
	m_undoPosition = 0;
}

px_ChangeHistory::~px_ChangeHistory()
{
	UT_VECTOR_PURGEALL(PX_ChangeRecord,m_vecChangeRecords);
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
}

UT_Bool px_ChangeHistory::addChangeRecord(PX_ChangeRecord * pcr)
{
	// add a change record to the history.
	// blow away any redo, since it is now invalid.

	_invalidateRedo();
	
	UT_Bool bResult = (m_vecChangeRecords.insertItemAt(pcr,m_undoPosition++) == 0);
	UT_ASSERT(bResult);
	return bResult;
}

UT_Bool px_ChangeHistory::getUndo(PX_ChangeRecord ** ppcr) const
{
	if (m_undoPosition == 0)
		return UT_FALSE;

	PX_ChangeRecord * pcr = (PX_ChangeRecord *)m_vecChangeRecords.getNthItem(m_undoPosition-1);
	UT_ASSERT(pcr);
	*ppcr = pcr;
	return UT_TRUE;
}

UT_Bool px_ChangeHistory::getRedo(PX_ChangeRecord ** ppcr) const
{
	if (m_undoPosition >= m_vecChangeRecords.getItemCount())
		return UT_FALSE;
	PX_ChangeRecord * pcr = (PX_ChangeRecord *)m_vecChangeRecords.getNthItem(m_undoPosition);
	if (!pcr)
		return UT_FALSE;
	*ppcr = pcr;
	return UT_TRUE;
}

UT_Bool px_ChangeHistory::didUndo(void)
{
	if (m_undoPosition == 0)
		return UT_FALSE;
	m_undoPosition--;
	return UT_TRUE;
}

UT_Bool px_ChangeHistory::didRedo(void)
{
	if (m_undoPosition >= m_vecChangeRecords.getItemCount())
		return UT_FALSE;
	m_undoPosition++;
	return UT_TRUE;
}

void px_ChangeHistory::coalesceHistory(const PX_ChangeRecord * pcr)
{
	// coalesce this record with the current undo record.

	PX_ChangeRecord * pcrUndo;
	UT_Bool bResult = getUndo(&pcrUndo);
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

