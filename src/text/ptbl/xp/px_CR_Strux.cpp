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
#include "ut_debugmsg.h"
#include "px_CR_Strux.h"
#include "px_ChangeRecord.h"

PX_ChangeRecord_Strux::PX_ChangeRecord_Strux(PXType type,
											 PT_DocPosition position,
											 PT_AttrPropIndex indexAP,
											 UT_uint32 iXID,
											 PTStruxType struxType)
	: PX_ChangeRecord(type, position, indexAP, iXID)
{
	xxx_UT_DEBUGMSG(("PX_ChangeRecord_Strux pos = %d \n",position));
	m_struxType = struxType;
}

PX_ChangeRecord_Strux::~PX_ChangeRecord_Strux()
{
}

PX_ChangeRecord * PX_ChangeRecord_Strux::reverse(void) const
{
	PX_ChangeRecord_Strux * pcr
		= new PX_ChangeRecord_Strux(getRevType(),m_position,m_indexAP,getXID(),m_struxType);
	UT_ASSERT_HARMLESS(pcr);
	return pcr;
}

PTStruxType PX_ChangeRecord_Strux::getStruxType(void) const
{
	return m_struxType;
}
