/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
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
