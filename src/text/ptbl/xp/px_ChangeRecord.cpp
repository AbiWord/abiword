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
		return PX_ChangeRecord::PXT_GlobMarker;				// we are our own inverse
		

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


	case PX_ChangeRecord::PXT_InsertObject:
		return PX_ChangeRecord::PXT_DeleteObject;

	case PX_ChangeRecord::PXT_DeleteObject:
		return PX_ChangeRecord::PXT_InsertObject;

	case PX_ChangeRecord::PXT_ChangeObject:
		return PX_ChangeRecord::PXT_ChangeObject;			// we are our own inverse


	case PX_ChangeRecord::PXT_InsertFmtMark:
		return PX_ChangeRecord::PXT_DeleteFmtMark;

	case PX_ChangeRecord::PXT_DeleteFmtMark:
		return PX_ChangeRecord::PXT_InsertFmtMark;

	case PX_ChangeRecord::PXT_ChangeFmtMark:
		return PX_ChangeRecord::PXT_ChangeFmtMark;			// we are our own inverse

	case PX_ChangeRecord::PXT_ChangePoint:
		return PX_ChangeRecord::PXT_ChangePoint;

	default:
		UT_ASSERT(0);
		return PX_ChangeRecord::PXT_GlobMarker;				// bogus
	}
}
