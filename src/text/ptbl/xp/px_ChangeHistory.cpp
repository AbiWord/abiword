
#include "ut_types.h"
#include "ut_vector.h"
#include "px_ChangeRecord.h"
#include "px_ChangeHistory.h"

px_ChangeHistory::px_ChangeHistory()
{
	m_undoPosition = 0;
}

px_ChangeHistory::~px_ChangeHistory()
{
	UT_VECTOR_PURGEALL(PX_ChangeRecord,m_vecChangeRecords);
}

UT_Bool px_ChangeHistory::addChangeRecord(PX_ChangeRecord * pcr)
{
	return UT_FALSE;
}

UT_Bool px_ChangeHistory::getUndo(PX_ChangeRecord ** ppcr) const
{
	return UT_FALSE;
}

UT_Bool px_ChangeHistory::getRedo(PX_ChangeRecord ** ppcr) const
{
	return UT_FALSE;
}

UT_Bool px_ChangeHistory::didUndo(void)
{
	return UT_FALSE;
}

UT_Bool px_ChangeHistory::didRedo(void)
{
	return UT_FALSE;
}

