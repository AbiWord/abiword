
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
													   PTChangeFmt ptc,
													   PT_BufIndex bufIndex,
													   UT_uint32 length)
	: PX_ChangeRecord(type, atomic, position, bLeftSide, indexNewAP)
{
	UT_ASSERT(length > 0);
	
	// m_indexAP in base class is set to indexNewAP
	m_indexOldAP = indexOldAP;
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
										 ptcRev,m_bufIndex,m_length);
	UT_ASSERT(pcr);
	return pcr;
}

PT_AttrPropIndex PX_ChangeRecord_SpanChange::getOldIndexAP(void) const
{
	return m_indexOldAP;
}

UT_uint32 PX_ChangeRecord_SpanChange::getLength(void) const
{
	return m_length;
}

PT_BufIndex PX_ChangeRecord_SpanChange::getBufIndex(void) const
{
	return m_bufIndex;
}
