
#include "ut_types.h"
#include "pt_Types.h"
#include "px_ChangeRecord.h"
#include "px_ChangeRecord_StruxChange.h"

PX_ChangeRecord_StruxChange::PX_ChangeRecord_StruxChange(PXType type,
														 UT_Byte atomic,
														 PT_DocPosition position,
														 PT_AttrPropIndex indexOldAP,
														 PT_AttrPropIndex indexNewAP,
														 PTChangeFmt ptc)
	: PX_ChangeRecord(type, atomic, position, UT_FALSE, indexNewAP)
{
	// m_indexAP in base class is set to indexNewAP
	m_indexOldAP = indexOldAP;
	m_ptc = ptc;
}

PX_ChangeRecord_StruxChange::~PX_ChangeRecord_StruxChange()
{
}

PX_ChangeRecord * PX_ChangeRecord_StruxChange::reverse(void) const
{
	PTChangeFmt ptcRev = (PTChangeFmt)( ! ((UT_Bool) m_ptc));
	
	PX_ChangeRecord_StruxChange * pcr
		= new PX_ChangeRecord_StruxChange(getRevType(),getRevFlags(),
										  m_position,
										  m_indexAP,m_indexOldAP,
										  ptcRev);
	UT_ASSERT(pcr);
	return pcr;
}

PT_AttrPropIndex PX_ChangeRecord_StruxChange::getOldIndexAP(void) const
{
	return m_indexOldAP;
}
