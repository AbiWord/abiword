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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */


#include "ut_types.h"
#include "pt_Types.h"
#include "px_ChangeRecord.h"
#include "px_CR_StruxChange.h"

PX_ChangeRecord_StruxChange::PX_ChangeRecord_StruxChange(PXType type,
														 PT_DocPosition position,
														 PT_AttrPropIndex indexOldAP,
														 PT_AttrPropIndex indexNewAP,
														 PTStruxType pts,
														 bool bRevisionDelete)
	: PX_ChangeRecord(type, position, indexNewAP, 0),
	  m_bRevisionDelete(bRevisionDelete)
{
	m_indexOldAP = indexOldAP;
	m_pts = pts;
}

PX_ChangeRecord_StruxChange::~PX_ChangeRecord_StruxChange()
{
}

PX_ChangeRecord * PX_ChangeRecord_StruxChange::reverse(void) const
{
	PX_ChangeRecord_StruxChange * pcr
		= new PX_ChangeRecord_StruxChange(getRevType(),
										  m_position,
										  m_indexAP,m_indexOldAP,m_pts,m_bRevisionDelete);
	UT_ASSERT_HARMLESS(pcr);
	return pcr;
}

PT_AttrPropIndex PX_ChangeRecord_StruxChange::getOldIndexAP(void) const
{
	return m_indexOldAP;
}
