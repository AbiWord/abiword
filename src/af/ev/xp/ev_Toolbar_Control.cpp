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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */


#include "ut_types.h"
#include "ut_vector.h"
#include "ev_Toolbar_Control.h"
#include "ev_Toolbar.h"


/*****************************************************************/

EV_Toolbar_Control::EV_Toolbar_Control(EV_Toolbar * pToolbar):
	// trying to set some reasonalbe defaults here -- controls can contain many items, and
	// there are not many instances of them -- we start with reasonably small number, but
	// will allow it to grow fast until quite a large size (see ut_vector.h for meaning of
	// the constructor parameters).
	m_vecContents(1024,32)
{
	UT_ASSERT(pToolbar);

	m_pToolbar = pToolbar;

	// set defaults ... should be overridden
	m_nPixels = 40;
	m_nLimit = 0;
	m_bSort = false;
}

EV_Toolbar_Control::~EV_Toolbar_Control(void)
{
}

const UT_GenericVector<const char*> * EV_Toolbar_Control::getContents(void) const
{
	return &m_vecContents;
}

const char * EV_Toolbar_Control::getNthItem(UT_uint32 n) const
{
	return m_vecContents.getNthItem(n);
}

UT_uint32 EV_Toolbar_Control::getPixelWidth(void) const
{
	return m_nPixels;
}

UT_uint32 EV_Toolbar_Control::getMaxLength(void) const
{
	return m_nLimit;
}

bool EV_Toolbar_Control::shouldSort(void) const
{
	return m_bSort;
}
