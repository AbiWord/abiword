
#include "ut_types.h"
#include "pt_Types.h"
#include "px_ChangeRecord.h"
#include "px_ChangeRecord_Span.h"

PX_ChangeRecord_Span::PX_ChangeRecord_Span(PXType type,
										   UT_Byte atomic,
										   PT_DocPosition position,
										   UT_Bool bLeftSide,
										   PT_AttrPropIndex indexOldAP,
										   PT_AttrPropIndex indexNewAP,
										   UT_Bool bTempBefore,
										   UT_Bool bTempAfter,
										   PT_BufIndex bufIndex,
										   UT_uint32 length)
	: PX_ChangeRecord(type, atomic, position, bLeftSide, indexOldAP, indexNewAP, bTempBefore, bTempAfter)
{
	UT_ASSERT(length > 0);
	
	m_bufIndex = bufIndex;
	m_length = length;
}

PX_ChangeRecord_Span::~PX_ChangeRecord_Span()
{
}

PX_ChangeRecord * PX_ChangeRecord_Span::reverse(void) const
{
	PX_ChangeRecord_Span * pcr
		= new PX_ChangeRecord_Span(getRevType(),getRevFlags(),
								   m_position,m_bLeftSide,
								   m_indexAP,m_indexOldAP,
								   m_bTempAfter,m_bTempBefore,
								   m_bufIndex,m_length);
	UT_ASSERT(pcr);
	return pcr;
}

UT_uint32 PX_ChangeRecord_Span::getLength(void) const
{
	return m_length;
}

PT_BufIndex PX_ChangeRecord_Span::getBufIndex(void) const
{
	return m_bufIndex;
}
