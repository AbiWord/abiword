 
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
#include "pt_Types.h"
#include "px_ChangeRecord.h"
#include "px_ChangeRecord_Glob.h"

PX_ChangeRecord_Glob::PX_ChangeRecord_Glob(PXType type,
										   UT_Byte flags)
	: PX_ChangeRecord(type, 0,0)
{
	m_flags = flags;
}

PX_ChangeRecord_Glob::~PX_ChangeRecord_Glob()
{
}

PX_ChangeRecord * PX_ChangeRecord_Glob::reverse(void) const
{
	PX_ChangeRecord_Glob * pcr
		= new PX_ChangeRecord_Glob(getRevType(),getRevFlags());

	UT_ASSERT(pcr);
	return pcr;
}

UT_Byte PX_ChangeRecord_Glob::getFlags(void) const
{
	return m_flags;
}

UT_Byte PX_ChangeRecord_Glob::getRevFlags(void) const
{
	switch (m_flags)
	{
	case PX_ChangeRecord_Glob::PXF_Null:
		return PX_ChangeRecord_Glob::PXF_Null;
		
	case PX_ChangeRecord_Glob::PXF_MultiStepStart:
		return PX_ChangeRecord_Glob::PXF_MultiStepEnd;
		
	case PX_ChangeRecord_Glob::PXF_MultiStepEnd:
		return PX_ChangeRecord_Glob::PXF_MultiStepStart;
		
	case PX_ChangeRecord_Glob::PXF_UserAtomicStart:
		return PX_ChangeRecord_Glob::PXF_UserAtomicEnd;
		
	case PX_ChangeRecord_Glob::PXF_UserAtomicEnd:
		return PX_ChangeRecord_Glob::PXF_UserAtomicStart;
	default:
		UT_ASSERT(0);
		return 0;
	}
}
