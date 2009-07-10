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
#include "px_CR_ObjectChange.h"

PX_ChangeRecord_ObjectChange::PX_ChangeRecord_ObjectChange(PXType type,
														   PT_DocPosition position,
														   PT_AttrPropIndex indexOldAP,
														   PT_AttrPropIndex indexNewAP,
														   PTObjectType pto,
														   PT_BlockOffset blockOffset,
														   bool bRevisionDelete)
	: PX_ChangeRecord(type, position, indexNewAP, 0),
	  m_bRevisionDelete(bRevisionDelete)
{
	m_indexOldAP = indexOldAP;
	m_objectType = pto;
	m_blockOffset = blockOffset;
}

PX_ChangeRecord_ObjectChange::~PX_ChangeRecord_ObjectChange()
{
}

PX_ChangeRecord * PX_ChangeRecord_ObjectChange::reverse(void) const
{
	PX_ChangeRecord_ObjectChange * pcr
		= new PX_ChangeRecord_ObjectChange(getRevType(),
										   m_position,
										   m_indexAP,m_indexOldAP,
										   m_objectType,
										   m_blockOffset,
										   m_bRevisionDelete);
	UT_ASSERT_HARMLESS(pcr);
	return pcr;
}

PT_AttrPropIndex PX_ChangeRecord_ObjectChange::getOldIndexAP(void) const
{
	return m_indexOldAP;
}

PTObjectType PX_ChangeRecord_ObjectChange::getObjectType(void) const
{
	return m_objectType;
}

PT_BlockOffset PX_ChangeRecord_ObjectChange::getBlockOffset(void) const
{
	return m_blockOffset;
}
