
#include "ut_types.h"
#include "pt_Types.h"
#include "px_ChangeRecord.h"
#include "px_ChangeRecord_SpanChange.h"

PX_ChangeRecord_SpanChange::PX_ChangeRecord_SpanChange(PXType type,
													   UT_Byte atomic,
													   PT_DocPosition position,
													   UT_Bool bLeftSide,
													   PT_AttrPropIndex indexOldAP,
													   PT_AttrPropIndex indexNewAP,
													   UT_Bool bTempBefore,
													   UT_Bool bTempAfter,
													   PTChangeFmt ptc,
													   PT_BufIndex bufIndex,
													   UT_uint32 length)
	: PX_ChangeRecord(type, atomic, position, bLeftSide, indexOldAP, indexNewAP, bTempBefore, bTempAfter)
{
	m_ptc = ptc;
	m_bufIndex = bufIndex;
	m_length = length;
}

PX_ChangeRecord_SpanChange::~PX_ChangeRecord_SpanChange()
{
}

PX_ChangeRecord * PX_ChangeRecord_SpanChange::reverse(void) const
{
	PTChangeFmt ptcRev = (PTChangeFmt)( ! ((UT_Bool) m_ptc));
	
	PX_ChangeRecord_SpanChange * pcr
		= new PX_ChangeRecord_SpanChange(getRevType(),getRevFlags(),
										 m_position,m_bLeftSide,
										 m_indexAP,m_indexOldAP,
										 m_bTempAfter,m_bTempBefore,
										 ptcRev,m_bufIndex,m_length);
	UT_ASSERT(pcr);
	return pcr;
}

UT_uint32 PX_ChangeRecord_SpanChange::getLength(void) const
{
	return m_length;
}

PT_BufIndex PX_ChangeRecord_SpanChange::getBufIndex(void) const
{
	return m_bufIndex;
}
