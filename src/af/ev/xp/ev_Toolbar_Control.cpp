/* AbiSource Program Utilities
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
#include "ev_Toolbar_Control.h"
#include "ev_Toolbar.h"


/*****************************************************************/

EV_Toolbar_Control::EV_Toolbar_Control(EV_Toolbar * pToolbar)
{
	UT_ASSERT(pToolbar);

	m_pToolbar = pToolbar;

	// set defaults ... should be overridden
	m_nPixels = 40;
	m_nLimit = 0;
	m_bSort = UT_FALSE;
}

EV_Toolbar_Control::~EV_Toolbar_Control(void)
{
}

const UT_Vector * EV_Toolbar_Control::getContents(void) const
{
	return &m_vecContents;
}

XML_Char * EV_Toolbar_Control::getNthItem(UT_uint32 n) const
{
	return (XML_Char *) m_vecContents.getNthItem(n);
}

UT_uint32 EV_Toolbar_Control::getPixelWidth(void) const
{
	return m_nPixels;
}

UT_uint32 EV_Toolbar_Control::getMaxLength(void) const
{
	return m_nLimit;
}

UT_Bool EV_Toolbar_Control::shouldSort(void) const
{
	return m_bSort;
}
