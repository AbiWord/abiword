
#include "ut_types.h"
#include "ut_vector.h"
#include "px_ChangeRecord.h"
#include "px_ChangeHistory.h"

px_ChangeHistory::px_ChangeHistory()
{
	m_undoPosition = 0;
	m_historyOn = UT_TRUE;
}

px_ChangeHistory::~px_ChangeHistory()
{
	UT_VECTOR_PURGEALL(PX_ChangeRecord,m_vecChangeRecords);
}

void px_ChangeHistory::enableHistory(UT_Bool bOn)
{
	m_historyOn = bOn;
}

UT_Bool px_ChangeHistory::addChangeRecord(PX_ChangeRecord * pcr)
{
	if (!m_historyOn)
		return UT_TRUE;
	
	// add a change record to the history.
	// blow away any redo, since it is now invalid.

	UT_uint32 kLimit = m_vecChangeRecords.getItemCount();
	UT_ASSERT(m_undoPosition <= kLimit);
	UT_uint32 k;

	for (k=kLimit; (k>m_undoPosition+1); k--)
	{
		PX_ChangeRecord * pcrTemp = (PX_ChangeRecord *)m_vecChangeRecords.getNthItem(k-1);
		if (!pcrTemp)
			break;
		delete pcrTemp;
		m_vecChangeRecords.deleteNthItem(k-1);
	}
	
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
	m_undoPosition++;
	return UT_TRUE;
}

