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

#include <stdlib.h>
#include <stdio.h>

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_types.h"
#include "ut_string.h"

#include "xap_Preview.h"
#include "ap_Preview_Paragraph.h"

AP_Preview_Paragraph::AP_Preview_Paragraph(GR_Graphics * gc)
	: XAP_Preview(gc)
{

}

AP_Preview_Paragraph::~AP_Preview_Paragraph()
{
}

void AP_Preview_Paragraph::draw(void)
{
	UT_ASSERT(m_gc);

	UT_RGBColor white(255,255,255);
	UT_RGBColor black(0,0,0);

	// clear area
	m_gc->fillRect(white, 0, 0, getWindowWidth(), getWindowHeight());

	// draw a black one pixel border
	m_gc->setColor(black);
	m_gc->drawLine(0, 0, getWindowWidth(), 0);
	m_gc->drawLine(getWindowWidth() - 1, 0, getWindowWidth() - 1, getWindowHeight());
	m_gc->drawLine(getWindowWidth() - 1, getWindowHeight() - 1, 0, getWindowHeight() - 1);
	m_gc->drawLine(0, getWindowHeight() - 1, 0, 0);
}
