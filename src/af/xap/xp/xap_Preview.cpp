/* AbiSource Application Framework
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

#include "ut_assert.h"
#include "ut_types.h"

#include "xap_Preview.h"

XAP_Preview::XAP_Preview(GR_Graphics * gc)
{
	UT_ASSERT(gc);
	
	// maybe later we'll need scroll offsets
	
	m_iWindowHeight = 0;
	m_iWindowWidth = 0;

	m_gc = gc;
}

XAP_Preview::~XAP_Preview()
{
}

/************************************************************************/

void XAP_Preview::setWindowSize(UT_sint32 width, UT_sint32 height)
{
	m_iWindowWidth = width;
	m_iWindowHeight = height;
}
