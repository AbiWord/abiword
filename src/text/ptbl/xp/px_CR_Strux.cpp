
#include "ut_types.h"
#include "px_ChangeRecord_Strux.h"
#include "px_ChangeRecord.h"

PX_ChangeRecord_Strux::PX_ChangeRecord_Strux(PXType type,
											 UT_Bool bMultiStepStart,
											 UT_Bool bMultiStepEnd,
											 PT_DocPosition position,
											 PT_VarSetIndex vsIndex,
											 UT_Bool bLeftSide,
											 PT_AttrPropIndex indexAP,
											 PTStruxType struxType)
	: PX_ChangeRecord(type, bMultiStepStart, bMultiStepEnd, position, vsIndex, bLeftSide, indexAP)
{
	m_struxType = struxType;
}

PX_ChangeRecord_Strux::~PX_ChangeRecord_Strux()
{
}

PTStruxType PX_ChangeRecord_Strux::getStruxType(void) const
{
	return m_struxType;
}

