 
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
#include "px_ChangeRecord_SpanChange.h"

PX_ChangeRecord_SpanChange::PX_ChangeRecord_SpanChange(PXType type,
													   PT_DocPosition position,
													   PT_AttrPropIndex indexOldAP,
													   PT_AttrPropIndex indexNewAP,
													   PTChangeFmt ptc,
													   PT_BufIndex bufIndex,
													   UT_uint32 length)
	: PX_ChangeRecord(type, position, indexNewAP)
{
	m_ptc = ptc;
	m_bufIndex = bufIndex;
	m_length = length;
	m_indexOldAP = indexOldAP;
}

PX_ChangeRecord_SpanChange::~PX_ChangeRecord_SpanChange()
{
}

PX_ChangeRecord * PX_ChangeRecord_SpanChange::reverse(void) const
{
	PTChangeFmt ptcRev = (PTChangeFmt)( ! ((UT_Bool) m_ptc));
	
	PX_ChangeRecord_SpanChange * pcr
		= new PX_ChangeRecord_SpanChange(getRevType(),
										 m_position,
										 m_indexAP,m_indexOldAP,
										 ptcRev,m_bufIndex,m_length);
	UT_ASSERT(pcr);
	return pcr;
}

UT_uint32 PX_ChangeRecord_SpanChange::getLength(void) const
{
	return m_length;
}

PT_BufIndex PX_ChangeRecord_SpanChange::getBufIndex(void) const
{
	return m_bufIndex;
}

PT_AttrPropIndex PX_ChangeRecord_SpanChange::getOldIndexAP(void) const
{
	return m_indexOldAP;
}
