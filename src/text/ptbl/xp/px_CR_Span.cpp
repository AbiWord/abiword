 
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
										   PT_DocPosition position,
										   PT_AttrPropIndex indexNewAP,
										   PT_BufIndex bufIndex,
										   UT_uint32 length,
										   PT_Differences bDifferentFmt)
	: PX_ChangeRecord(type, position, indexNewAP)
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
		= new PX_ChangeRecord_Span(getRevType(),m_position,m_indexAP,
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

void PX_ChangeRecord_Span::coalesce(const PX_ChangeRecord_Span * pcr)
{
	// append the effect of the given pcr onto the end of us.

	// some quick sanity checks.  our caller is supposed to have already verified this
	
	UT_ASSERT(getType() == pcr->getType());
	UT_ASSERT(getIndexAP() == pcr->getIndexAP());

	m_length += pcr->getLength();

	if (pcr->getPosition() < getPosition())			// if we have a prepend (like a backspace)
	{
		m_position = pcr->getPosition();
		m_bufIndex = pcr->getBufIndex();
	}
	
	return;
}

