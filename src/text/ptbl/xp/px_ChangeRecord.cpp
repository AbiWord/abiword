
#include "ut_types.h"
#include "ut_debugmsg.h"
#include "pt_Types.h"
#include "px_ChangeRecord.h"

PX_ChangeRecord::PX_ChangeRecord(PXType type,
								 UT_Byte atomic,
								 PT_DocPosition position,
								 UT_Bool bLeftSide,
								 PT_AttrPropIndex indexAP)
{
	m_type = type;
	m_atomic = atomic;
	m_position = position;
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

UT_Byte PX_ChangeRecord::getFlags(void) const
{
	return m_atomic;
}

PT_DocPosition PX_ChangeRecord::getPosition(void) const
{
	return m_position;
}

PT_AttrPropIndex PX_ChangeRecord::getIndexAP(void) const
{
	return m_indexAP;
}

PX_ChangeRecord * PX_ChangeRecord::reverse(void) const
{
	PX_ChangeRecord * pcr
		= new PX_ChangeRecord(getRevType(),getRevFlags(),
							  m_position,m_bLeftSide,m_indexAP);
	UT_ASSERT(pcr);
	return pcr;
}

PX_ChangeRecord::PXType PX_ChangeRecord::getRevType(void) const
{
	switch (m_type)
	{
	case PX_ChangeRecord::PXT_UserAtomicGlobMarker:
		return PX_ChangeRecord::PXT_UserAtomicGlobMarker;
		
	case PX_ChangeRecord::PXT_InsertSpan:
		return PX_ChangeRecord::PXT_DeleteSpan;
		
	case PX_ChangeRecord::PXT_DeleteSpan:
		return PX_ChangeRecord::PXT_InsertSpan;
		
	default:
		UT_ASSERT(0);
		return PX_ChangeRecord::PXT_UserAtomicGlobMarker; // bogus
	}
}

UT_Byte PX_ChangeRecord::getRevFlags(void) const
{
	switch (m_atomic)
	{
	case PX_ChangeRecord::PXF_Null:
		return PX_ChangeRecord::PXF_Null;
		
	case PX_ChangeRecord::PXF_MultiStepStart:
		return PX_ChangeRecord::PXF_MultiStepEnd;
		
	case PX_ChangeRecord::PXF_MultiStepEnd:
		return PX_ChangeRecord::PXF_MultiStepStart;
		
	case PX_ChangeRecord::PXF_UserAtomicStart:
		return PX_ChangeRecord::PXF_UserAtomicEnd;
		
	case PX_ChangeRecord::PXF_UserAtomicEnd:
		return PX_ChangeRecord::PXF_UserAtomicStart;
	default:
		UT_ASSERT(0);
		return 0;
	}
}
	

void PX_ChangeRecord::dump(void) const
{
#ifdef UT_DEBUG
	static const char * _a[] = { "insSpan", "delSpan", "insFmt", "delFmt",
								 "insStrux","delStrux","insObj", "delObj" };
	
	UT_DEBUGMSG(("CRec: T[%s] [b %x] [ap %d]\n",
				 (_a[m_type]),m_atomic,m_indexAP));
#endif
}
