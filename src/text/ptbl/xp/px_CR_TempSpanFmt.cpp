 
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
#include "px_ChangeRecord_TempSpanFmt.h"

PX_ChangeRecord_TempSpanFmt::PX_ChangeRecord_TempSpanFmt(PXType type,
														 PT_DocPosition position,
														 PT_AttrPropIndex indexNewAP,
														 UT_Bool bEnableTempSpanFmt)
	: PX_ChangeRecord(type, position, indexNewAP)
{
	m_bEnableTempSpanFmt = bEnableTempSpanFmt;
}

PX_ChangeRecord_TempSpanFmt::~PX_ChangeRecord_TempSpanFmt()
{
}

PX_ChangeRecord * PX_ChangeRecord_TempSpanFmt::reverse(void) const
{
	PX_ChangeRecord_TempSpanFmt * pcr
		= new PX_ChangeRecord_TempSpanFmt(getRevType(),m_position,m_indexAP,!m_bEnableTempSpanFmt);

	UT_ASSERT(pcr);
	return pcr;
}

UT_Bool PX_ChangeRecord_TempSpanFmt::getEnabled(void) const
{
	return m_bEnableTempSpanFmt;
}
