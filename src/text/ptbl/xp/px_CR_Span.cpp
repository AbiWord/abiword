 
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
#include "px_ChangeRecord_Span.h"

PX_ChangeRecord_Span::PX_ChangeRecord_Span(PXType type,
										   UT_Byte atomic,
										   PT_DocPosition position,
										   PT_AttrPropIndex indexOldAP,
										   PT_AttrPropIndex indexNewAP,
										   UT_Bool bTempBefore,
										   UT_Bool bTempAfter,
										   PT_BufIndex bufIndex,
										   UT_uint32 length,
										   PT_Differences bDifferentFmt)
	: PX_ChangeRecord(type, atomic, position, indexOldAP, indexNewAP, bTempBefore, bTempAfter)
{
	UT_ASSERT(length > 0);
	
	m_bufIndex = bufIndex;
	m_length = length;
	m_DifferentFmt = bDifferentFmt;
}

PX_ChangeRecord_Span::~PX_ChangeRecord_Span()
{
}

PX_ChangeRecord * PX_ChangeRecord_Span::reverse(void) const
{
	PX_ChangeRecord_Span * pcr
		= new PX_ChangeRecord_Span(getRevType(),getRevFlags(),
								   m_position,
								   m_indexAP,m_indexOldAP,
								   m_bTempAfter,m_bTempBefore,
								   m_bufIndex,m_length,m_DifferentFmt);
	UT_ASSERT(pcr);
	return pcr;
}

UT_uint32 PX_ChangeRecord_Span::getLength(void) const
{
	return m_length;
}

PT_BufIndex PX_ChangeRecord_Span::getBufIndex(void) const
{
	return m_bufIndex;
}

PT_Differences PX_ChangeRecord_Span::isDifferentFmt(void) const
{
	return m_DifferentFmt;
}
