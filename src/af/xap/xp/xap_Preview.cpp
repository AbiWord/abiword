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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include "ut_assert.h"
#include "ut_types.h"

#include "xap_Preview.h"
#include "gr_Graphics.h"

XAP_Preview::XAP_Preview(GR_Graphics * gc) :
	m_gc(gc), m_iWindowHeight(0), m_iWindowWidth(0)
{
	UT_ASSERT(gc);
	
	// maybe later we'll need scroll offsets
}

// protected constructor
XAP_Preview::XAP_Preview() :
m_iWindowHeight(0), m_iWindowWidth(0)
{
	//UT_ASSERT(gc); -> requires setting of gc after creation
}

XAP_Preview::~XAP_Preview()
{
}

void XAP_Preview::queueDraw(const UT_Rect* clip)
{
    m_drawQueue.push(clip ? UT_Option<UT_Rect>(*clip) : UT_Option<UT_Rect>());
    getGraphics()->queueDraw(clip);
}

/************************************************************************/

void XAP_Preview::setWindowSize(UT_sint32 width, UT_sint32 height)
{
	m_iWindowWidth = width;
	m_iWindowHeight = height;
}

