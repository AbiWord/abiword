
#include "ut_types.h"
#include "pt_Types.h"
#include "px_ChangeRecord.h"
#include "px_ChangeRecord_Span.h"

PX_ChangeRecord_Span::PX_ChangeRecord_Span(PXType type,
										   UT_Bool bMultiStepStart,
										   UT_Bool bMultiStepEnd,
										   PT_DocPosition position,
										   UT_Bool bLeftSide,
										   PT_AttrPropIndex indexAP,
										   PT_BufIndex bufIndex,
										   UT_uint32 length)
	: PX_ChangeRecord(type, bMultiStepStart, bMultiStepEnd, position, bLeftSide, indexAP)
{
	m_bufIndex = bufIndex;
	m_length = length;
}

PX_ChangeRecord_Span::~PX_ChangeRecord_Span()
{
}

UT_uint32 PX_ChangeRecord_Span::getLength(void) const
{
	return m_length;
}
