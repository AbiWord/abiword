
#include "ut_types.h"
#include "ut_debugmsg.h"
#include "pt_Types.h"
#include "px_ChangeRecord.h"

PX_ChangeRecord::PX_ChangeRecord(PXType type,
								 UT_Bool bMultiStepStart,
								 UT_Bool bMultiStepEnd,
								 PT_DocPosition position,
								 PT_VarSetIndex vsIndex,
								 UT_Bool bLeftSide,
								 PT_AttrPropIndex indexAP)
{
	m_type = type;
	m_bMultiStepStart = bMultiStepStart;
	m_bMultiStepEnd = bMultiStepEnd;
	m_position = position;
	m_vsIndex = vsIndex;
	m_bLeftSide = bLeftSide;
	m_indexAP = indexAP;
}

PX_ChangeRecord::~PX_ChangeRecord()
{
}

PX_ChangeRecord::PXType PX_ChangeRecord::getType(void) const
{
	return m_type;
}

PT_DocPosition PX_ChangeRecord::getPosition(void) const
{
	return m_position;
}

PT_AttrPropIndex PX_ChangeRecord::getIndexAP(void) const
{
	return m_indexAP;
}

PT_VarSetIndex PX_ChangeRecord::getVarSetIndex(void) const
{
	return m_vsIndex;
}


void PX_ChangeRecord::dump(void) const
{
#ifdef UT_DEBUG
	static const char * _a[] = { "insSpan", "delSpan", "insFmt", "delFmt",
								 "insStrux","delStrux","insObj", "delObj" };
	
	UT_DEBUGMSG(("CRec: T[%s] [b %d,%d] [i %d,%d]\n",
				 (_a[m_type]),m_bMultiStepStart,m_bMultiStepEnd,m_vsIndex,m_indexAP));
#endif
}
