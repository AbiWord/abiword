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

#ifndef EV_TOOLBAR_CONTROL_H
#define EV_TOOLBAR_CONTROL_H

#include "ut_types.h"
#include "ut_misc.h"
#include "ut_vector.h"

class EV_Toolbar;


// this abstract class encapsulates the contents of a toolbar control.  
// it allows the EV_Toolbar logic to build and manage the underlying widgets.

class EV_Toolbar_Control
{
public:
	EV_Toolbar_Control(EV_Toolbar * pToolbar);
	virtual ~EV_Toolbar_Control(void);

	virtual bool		populate(void) = 0;

	const UT_Vector *	getContents(void) const;
	XML_Char *			getNthItem(UT_uint32 n) const;

	UT_uint32			getMaxLength(void) const;
	UT_uint32			getPixelWidth(void) const;
	bool				shouldSort(void) const;

protected:
	EV_Toolbar *	m_pToolbar;
	UT_uint32		m_nLimit;
	UT_uint32		m_nPixels;
	bool			m_bSort;

	UT_Vector		m_vecContents;
};

#endif /* EV_TOOLBAR_CONTROL_H */
