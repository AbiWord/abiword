
#include "ut_types.h"
#include "pt_Types.h"
#include "px_ChangeRecord_Span.h"
#include "px_ChangeRecord.h"

PX_ChangeRecord_Span::PX_ChangeRecord_Span(PXType type,
										   UT_Bool bMultiStepStart,
										   UT_Bool bMultiStepEnd,
										   PT_DocPosition position,
										   UT_uint32 vsIndex,
										   UT_Bool bLeftSide,
										   pt_AttrPropIndex indexAP,
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
