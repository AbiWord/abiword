
#include "ut_types.h"
#include "pt_Types.h"
#include "px_ChangeRecord_Span.h"
#include "px_ChangeRecord.h"

PX_ChangeRecord_Span::PX_ChangeRecord_Span(PXType type,
										   UT_Bool bMultiStepStart,
										   UT_Bool bMultiStepEnd,
										   PT_DocPosition position,
										   PT_VarSetIndex vsIndex,
										   UT_Bool bLeftSide,
										   PT_AttrPropIndex indexAP,
										   pt_BufPosition offset,
										   UT_uint32 length)
	: PX_ChangeRecord(type, bMultiStepStart, bMultiStepEnd, position, vsIndex, bLeftSide, indexAP)
{
	m_offset = offset;
	m_length = length;
}

PX_ChangeRecord_Span::~PX_ChangeRecord_Span()
{
}

UT_uint32 PX_ChangeRecord_Span::getLength(void) const
{
	return m_length;
}