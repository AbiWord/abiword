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

#ifndef AP_RULER_H
#define AP_RULER_H

// Common utilities for the left and top rulers.

#include "ut_types.h"
#include "gr_Graphics.h"

class ap_RulerTicks
{
public:
	ap_RulerTicks(DG_Graphics * pG);

	DG_Graphics *	m_pG;
	
	UT_uint32		tickUnit;
	UT_uint32		tickUnitScale;
	UT_uint32		tickLong;
	UT_uint32		tickLabel;
	UT_uint32		tickScale;
};

#endif /* AP_RULER_H */
