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
