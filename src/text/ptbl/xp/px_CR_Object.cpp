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
#include "px_CR_Object.h"
#include "px_ChangeRecord.h"
#include "fd_Field.h"

PX_ChangeRecord_Object::PX_ChangeRecord_Object(PXType type,
											   PT_DocPosition position,
											   PT_AttrPropIndex indexAP,
											   UT_uint32 iXID,
											   PTObjectType objectType,
											   PT_BlockOffset blockOffset,
                                               fd_Field * pField,
					       pf_Frag_Object* pOH)
	: PX_ChangeRecord(type, position, indexAP, iXID)
{
	m_objectType = objectType;
	m_blockOffset = blockOffset;
    m_field = pField;
    m_OH = pOH;
}

PX_ChangeRecord_Object::~PX_ChangeRecord_Object()
{
}

PX_ChangeRecord * PX_ChangeRecord_Object::reverse(void) const
{
	PX_ChangeRecord_Object * pcr
		= new PX_ChangeRecord_Object(getRevType(),m_position,
					     m_indexAP,getXID(),m_objectType,
					     m_blockOffset,m_field,m_OH);
	return pcr;
}

PTObjectType PX_ChangeRecord_Object::getObjectType(void) const
{
	return m_objectType;
}

PT_BlockOffset PX_ChangeRecord_Object::getBlockOffset(void) const
{
	return m_blockOffset;
}
