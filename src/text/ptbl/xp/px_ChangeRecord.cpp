
#include "ut_types.h"
#include "pt_Types.h"
#include "px_ChangeRecord.h"

PX_ChangeRecord::PX_ChangeRecord(PXType type,
					UT_Bool bMultiStepStart,
					UT_Bool bMultiStepEnd,
					PT_DocPosition position,
					UT_uint32 vsIndex)
{
	m_type = type;
	m_bMultiStepStart = bMultiStepStart;
	m_bMultiStepEnd = bMultiStepEnd;
	m_position = position;
	m_vsIndex = vsIndex;
}

PX_ChangeRecord::~PX_ChangeRecord()
{
}