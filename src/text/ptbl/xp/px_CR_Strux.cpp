
#include "ut_types.h"
#include "px_ChangeRecord_Strux.h"
#include "px_ChangeRecord.h"

PX_ChangeRecord_Strux::PX_ChangeRecord_Strux(PXType type,
					  UT_Bool bMultiStepStart,
					  UT_Bool bMultiStepEnd,
					  PT_DocPosition position,
					  UT_uint32 vsIndex,
					  UT_Bool bLeftSide,
					  pt_AttrPropIndex indexAP,
					  PTStruxType struxType)
	: PX_ChangeRecord(type, bMultiStepStart, bMultiStepEnd, position, vsIndex)
{
	m_bLeftSide = bLeftSide;
	m_indexAP = indexAP;
	m_struxType = struxType;
}

PX_ChangeRecord_Strux::~PX_ChangeRecord_Strux()
{
}
