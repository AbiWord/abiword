 
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
								 PT_DocPosition position,
								 PT_AttrPropIndex indexNewAP)
{
	m_type = type;
	m_position = position;
	m_indexAP = indexNewAP;
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

PX_ChangeRecord * PX_ChangeRecord::reverse(void) const
{
	PX_ChangeRecord * pcr
		= new PX_ChangeRecord(getRevType(),
							  m_position,
							  m_indexAP);
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

	case PX_ChangeRecord::PXT_TempSpanFmt:
		return PX_ChangeRecord::PXT_TempSpanFmt;			// we are our own inverse
		
	default:
		UT_ASSERT(0);
		return PX_ChangeRecord::PXT_GlobMarker;				// bogus
	}
}
