/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */ 


#include "ut_types.h"
#include "px_ChangeRecord_Strux.h"
#include "px_ChangeRecord.h"

PX_ChangeRecord_Strux::PX_ChangeRecord_Strux(PXType type,
											 PT_DocPosition position,
											 PT_AttrPropIndex indexAP,
											 PTStruxType struxType)
	: PX_ChangeRecord(type, position, indexAP)
{
	m_struxType = struxType;
	m_preferredSpanAPI = 0;
}

PX_ChangeRecord_Strux::PX_ChangeRecord_Strux(PXType type,
											 PT_DocPosition position,
											 PT_AttrPropIndex indexAP,
											 PTStruxType struxType,
											 PT_AttrPropIndex preferredSpanAPI)
	: PX_ChangeRecord(type, position, indexAP)
{
	m_struxType = struxType;
	m_preferredSpanAPI = preferredSpanAPI;
	UT_ASSERT((preferredSpanAPI==0) || (struxType==PTX_Block));
}

PX_ChangeRecord_Strux::~PX_ChangeRecord_Strux()
{
}

PX_ChangeRecord * PX_ChangeRecord_Strux::reverse(void) const
{
	PX_ChangeRecord_Strux * pcr
		= new PX_ChangeRecord_Strux(getRevType(),m_position,m_indexAP,m_struxType,m_preferredSpanAPI);
	UT_ASSERT(pcr);
	return pcr;
}

PTStruxType PX_ChangeRecord_Strux::getStruxType(void) const
{
	return m_struxType;
}

PT_AttrPropIndex PX_ChangeRecord_Strux::getPreferredSpanFmt(void) const
{
	return m_preferredSpanAPI;
}

