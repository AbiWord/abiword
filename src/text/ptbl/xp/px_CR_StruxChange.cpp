
#include "ut_types.h"
#include "pt_Types.h"
#include "px_ChangeRecord.h"
#include "px_ChangeRecord_StruxChange.h"

PX_ChangeRecord_StruxChange::PX_ChangeRecord_StruxChange(PXType type,
														 UT_Byte atomic,
														 PT_DocPosition position,
														 PT_AttrPropIndex indexOldAP,
														 PT_AttrPropIndex indexNewAP,
														 UT_Bool bTempBefore,
														 UT_Bool bTempAfter,
														 PTChangeFmt ptc)
	: PX_ChangeRecord(type, atomic, position, UT_FALSE, indexOldAP, indexNewAP, bTempBefore, bTempAfter)
{
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
										  m_bTempAfter,m_bTempBefore,
										  ptcRev);
	UT_ASSERT(pcr);
	return pcr;
}
