 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
*/

#include "ut_types.h"
#include "ut_debugmsg.h"
#include "pt_Types.h"
#include "px_ChangeRecord.h"

PX_ChangeRecord::PX_ChangeRecord(PXType type,
								 UT_Byte atomic,
								 PT_DocPosition position,
								 PT_AttrPropIndex indexOldAP,
								 PT_AttrPropIndex indexNewAP,
								 UT_Bool bTempBefore,
								 UT_Bool bTempAfter)
{
	m_type = type;
	m_atomic = atomic;
	m_position = position;
	m_indexOldAP = indexOldAP;
	m_indexAP = indexNewAP;
	m_bTempBefore = bTempBefore;
	m_bTempAfter = bTempAfter;
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

PT_AttrPropIndex PX_ChangeRecord::getOldIndexAP(void) const
{
	return m_indexOldAP;
}

UT_Bool PX_ChangeRecord::getTempBefore(void) const
{
	return m_bTempBefore;
}

UT_Bool PX_ChangeRecord::getTempAfter(void) const
{
	return m_bTempAfter;
}

PX_ChangeRecord * PX_ChangeRecord::reverse(void) const
{
	PX_ChangeRecord * pcr
		= new PX_ChangeRecord(getRevType(),getRevFlags(),
							  m_position,
							  m_indexAP,m_indexOldAP,
							  m_bTempAfter,m_bTempBefore);
	UT_ASSERT(pcr);
	return pcr;
}

PX_ChangeRecord::PXType PX_ChangeRecord::getRevType(void) const
{
	switch (m_type)
	{
	case PX_ChangeRecord::PXT_GlobMarker:
		return PX_ChangeRecord::PXT_GlobMarker;
		
	case PX_ChangeRecord::PXT_InsertSpan:
		return PX_ChangeRecord::PXT_DeleteSpan;
		
	case PX_ChangeRecord::PXT_DeleteSpan:
		return PX_ChangeRecord::PXT_InsertSpan;

	case PX_ChangeRecord::PXT_ChangeSpan:
		return PX_ChangeRecord::PXT_ChangeSpan;				// we are our own inverse
		
	case PX_ChangeRecord::PXT_InsertStrux:
		return PX_ChangeRecord::PXT_DeleteStrux;

	case PX_ChangeRecord::PXT_DeleteStrux:
		return PX_ChangeRecord::PXT_InsertStrux;

	case PX_ChangeRecord::PXT_ChangeStrux:
		return PX_ChangeRecord::PXT_ChangeStrux;			// we are our own inverse
		
	default:
		UT_ASSERT(0);
		return PX_ChangeRecord::PXT_GlobMarker;				// bogus
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
