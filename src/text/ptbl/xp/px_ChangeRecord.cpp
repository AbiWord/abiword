
#include "ut_types.h"
#include "pt_Types.h"
#include "px_ChangeRecord.h"

PX_ChangeRecord::PX_ChangeRecord(PXType type,
								 UT_Bool bMultiStepStart,
								 UT_Bool bMultiStepEnd,
								 PT_DocPosition position,
								 UT_uint32 vsIndex,
								 UT_Bool bLeftSide,
								 pt_AttrPropIndex indexAP)
{
	m_type = type;
	m_bMultiStepStart = bMultiStepStart;
	m_bMultiStepEnd = bMultiStepEnd;
	m_position = position;
	m_vsIndex = vsIndex;
	m_bLeftSide = bLeftSide;
	m_indexAP = indexAP;
}

PX_ChangeRecord::~PX_ChangeRecord()
{
}
