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


#include <stdlib.h>
#include <stdio.h>

#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_types.h"
#include "ut_string.h"

#include "xap_Preview_Zoom.h"

XAP_Preview_Zoom::XAP_Preview_Zoom(GR_Graphics * gc)
	: XAP_Preview(gc)
{
	m_string = NULL;

	// m_gc is set in base class, so set up defaults
	setFont(XAP_Preview_Zoom::font_NORMAL);
	setDrawAtPosition(XAP_Preview_Zoom::pos_CENTER);
	setZoomPercent(100);
}

XAP_Preview_Zoom::~XAP_Preview_Zoom()
{
	FREEP(m_string);
}

void XAP_Preview_Zoom::setDrawAtPosition(XAP_Preview_Zoom::tPos pos)
{
	UT_ASSERT(m_gc);
	
	m_pos = pos;
}

void XAP_Preview_Zoom::setFont(XAP_Preview_Zoom::tFont f)
{
	UT_ASSERT(m_gc);

	// when searching for fonts, we use these
	GR_Font * found = NULL;
	char fontString[10];

	UT_uint32 pointSize = 0;

	switch (f)
	{
	case XAP_Preview_Zoom::font_NORMAL:

		// 10 points for "NORMAL"
		pointSize = (UT_uint32) (10.0 * (float) m_zoomPercent / 100.0);
 		sprintf(fontString, "%dpt", pointSize);

		found = m_gc->findFont("Times New Roman", "normal", "", "normal", "", fontString);
		UT_ASSERT(found);

		m_gc->setFont(found);

		break;
	default:
		UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	}

	// store it for next time
	m_previewFont = f;

}

void XAP_Preview_Zoom::setZoomPercent(UT_uint32 percent)
{
	UT_ASSERT(m_gc);
	UT_ASSERT(percent > 0 || percent < 1000);

	m_zoomPercent = percent;
	
	// re-calculate the GC's font using the new zoom size
	setFont(m_previewFont);
}


UT_Bool XAP_Preview_Zoom::setString(const char * string)
{
	UT_ASSERT(m_gc);
	UT_ASSERT(string);
	
	FREEP(m_string);
	UT_Bool foo = UT_UCS_cloneString_char(&m_string, string);
	return foo;
}

UT_Bool XAP_Preview_Zoom::setString(UT_UCSChar * string)
{
	UT_ASSERT(m_gc);
	UT_ASSERT(string);
	
	FREEP(m_string);
	UT_Bool foo = UT_UCS_cloneString(&m_string, string);
	return foo;
}

void XAP_Preview_Zoom::draw(void)
{
	UT_ASSERT(m_gc);
	UT_ASSERT(m_string);
	
	// TODO : replace 5,5 with real coordinates
	m_gc->clearArea(0, 0, getWindowWidth(), getWindowHeight());
	m_gc->drawChars(m_string, 0, UT_UCS_strlen(m_string), 5, 5);
}
