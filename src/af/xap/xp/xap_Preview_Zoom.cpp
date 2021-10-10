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


#include <stdlib.h>
#include <stdio.h>
#include "ut_assert.h"
#include "ut_types.h"
#include "ut_string.h"
#include "gr_Graphics.h"
#include "gr_Painter.h"
#include "xap_Preview_Zoom.h"

XAP_Preview_Zoom::XAP_Preview_Zoom(GR_Graphics * gc)
	: XAP_Preview(gc)
{
	m_string = NULL;
	m_pFont = NULL;

	// m_gc is set in base class, so set up defaults
	m_zoomPercent = 100;
	setFont(XAP_Preview_Zoom::font_NORMAL);
	setDrawAtPosition(XAP_Preview_Zoom::pos_CENTER);
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
	UT_ASSERT(m_zoomPercent > 0);
	char fontString [16];

	// when searching for fonts, we use these
	GR_Font * found = NULL;

	switch (f)
	{
	case XAP_Preview_Zoom::font_NORMAL:
		snprintf (fontString, 16, "%dpt", (10 * m_zoomPercent / 100));
		found = m_gc->findFont("Times New Roman",
							   "normal", "", "normal",
							   "", fontString,
							   NULL);

		if (found)
		{
			m_gc->setFont(found);
			m_pFont = found;
		}
		else {
			UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
		}
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

bool XAP_Preview_Zoom::setString(const char * string)
{
	UT_ASSERT(m_gc);
	UT_ASSERT(string);
	
	FREEP(m_string);

	bool foo = UT_UCS4_cloneString_char(&m_string, string);
	return foo;
}

bool XAP_Preview_Zoom::setString(UT_UCSChar * string)
{
	UT_ASSERT(m_gc);
	UT_ASSERT(string);
	
	FREEP(m_string);

	bool foo = UT_UCS4_cloneString(&m_string, string);
	return foo;
}

void XAP_Preview_Zoom::drawImmediate(const UT_Rect *clip)
{
	UT_UNUSED(clip);
	UT_ASSERT(m_gc);
	UT_ASSERT(m_string);
	
	GR_Painter painter(m_gc);

	UT_sint32 iWidth = m_gc->tlu (getWindowWidth());
	UT_sint32 iHeight = m_gc->tlu (getWindowHeight());
	UT_Rect pageRect(m_gc->tlu(7), m_gc->tlu(7), iWidth - m_gc->tlu(14), iHeight - m_gc->tlu(14));	
	
	painter.fillRect(GR_Graphics::CLR3D_Background, 0, 0, iWidth, iHeight);
	painter.clearArea(pageRect.left, pageRect.top, pageRect.width, pageRect.height);

	pageRect.left += m_gc->tlu(5);
	pageRect.top += m_gc->tlu(5);
	pageRect.width -= m_gc->tlu(10);
	pageRect.height -= m_gc->tlu(10);

	// set the cliprect to our page size, so we don't draw over our borders
	m_gc->setClipRect(&pageRect);

	painter.drawChars(m_string, 0, UT_UCS4_strlen(m_string), pageRect.left, pageRect.top);

	UT_Rect rr (0, 0, iWidth, iHeight);
	m_gc->setClipRect(&rr);
}
