/* AbiWord
 * Copyright (C) 2006, M.Sevior <msevior@physics.unimelb.edu.au>
 * 1998 AbiSource, Inc.
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
#include "px_CR_DocProp.h"
#include "px_ChangeRecord.h"

PX_ChangeRecord_DocProp::PX_ChangeRecord_DocProp(PXType type,
					       PT_DocPosition position,
					       PT_AttrPropIndex indexAP,
					       UT_uint32 iXID)
	: PX_ChangeRecord(type, position, indexAP, iXID)
{
	xxx_UT_DEBUGMSG(("PX_ChangeRecord_DocProp pos = %d \n",position));
}

PX_ChangeRecord_DocProp::~PX_ChangeRecord_DocProp()
{
}

PX_ChangeRecord * PX_ChangeRecord_DocProp::reverse(void) const
{
	PX_ChangeRecord_DocProp * pcr
		= new PX_ChangeRecord_DocProp(getRevType(),m_position,m_indexAP,getXID());
	UT_ASSERT_HARMLESS(pcr);
	return pcr;
}

