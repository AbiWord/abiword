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
#include "px_CR_Glob.h"

PX_ChangeRecord_Glob::PX_ChangeRecord_Glob(PXType type,
										   UT_Byte flags)
	: PX_ChangeRecord(type, 0,0)
{
	m_flags = flags;
}

PX_ChangeRecord_Glob::~PX_ChangeRecord_Glob()
{
}

PX_ChangeRecord * PX_ChangeRecord_Glob::reverse(void) const
{
	PX_ChangeRecord_Glob * pcr
		= new PX_ChangeRecord_Glob(getRevType(),getRevFlags());

	UT_ASSERT(pcr);
	return pcr;
}

UT_Byte PX_ChangeRecord_Glob::getFlags(void) const
{
	return m_flags;
}

UT_Byte PX_ChangeRecord_Glob::getRevFlags(void) const
{
	switch (m_flags)
	{
	case PX_ChangeRecord_Glob::PXF_Null:
		return PX_ChangeRecord_Glob::PXF_Null;
		
	case PX_ChangeRecord_Glob::PXF_MultiStepStart:
		return PX_ChangeRecord_Glob::PXF_MultiStepEnd;
		
	case PX_ChangeRecord_Glob::PXF_MultiStepEnd:
		return PX_ChangeRecord_Glob::PXF_MultiStepStart;
		
	case PX_ChangeRecord_Glob::PXF_UserAtomicStart:
		return PX_ChangeRecord_Glob::PXF_UserAtomicEnd;
		
	case PX_ChangeRecord_Glob::PXF_UserAtomicEnd:
		return PX_ChangeRecord_Glob::PXF_UserAtomicStart;
	default:
		UT_ASSERT(0);
		return 0;
	}
}
