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
#include "ut_vector.h"
#include "ev_Toolbar.h"
#include "xap_Toolbar_ControlFactory.h"

/*****************************************************************/

AP_Toolbar_ControlFactory::AP_Toolbar_ControlFactory(int nrElem, const struct _ctl_table * pCtlTable)
{
	m_nrElementsCtlTable = nrElem;
	m_ctl_table = pCtlTable;
}

AP_Toolbar_ControlFactory::~AP_Toolbar_ControlFactory(void)
{
}

UT_Bool AP_Toolbar_ControlFactory::_find_ControlInTable(AP_Toolbar_Id id, UT_uint32 * pIndex) const
{
	// search the table and return the index of the entry with this id.

	UT_ASSERT(pIndex);

	for (UT_uint32 k=0; (k < m_nrElementsCtlTable); k++)
	{
		if (m_ctl_table[k].m_id == id)
		{
			*pIndex = k;
			return UT_TRUE;
		}
	}

	UT_ASSERT(UT_NOT_IMPLEMENTED);
	return UT_FALSE;
}

/*****************************************************************/

EV_Toolbar_Control * AP_Toolbar_ControlFactory::getControl(EV_Toolbar * pToolbar, AP_Toolbar_Id id)
{
	UT_uint32 index;
	EV_Toolbar_Control * pControl = NULL;
	
	_find_ControlInTable(id,&index);

	// create a fresh Toolbar_Control object and return it -- no strings attached.
	
	pControl = (EV_Toolbar_Control *)((m_ctl_table[index].m_pfnStaticConstructor)(pToolbar,id));
	return pControl;
}

