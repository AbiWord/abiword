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
#include <string.h>
#include "ut_assert.h"
#include "ut_string.h"
#include "ut_debugmsg.h"

#include "xap_Dialog_Id.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"

#include "ap_Dialog_Columns.h"

AP_Dialog_Columns::AP_Dialog_Columns(XAP_DialogFactory * pDlgFactory, XAP_Dialog_Id id)
	: XAP_Dialog_NonPersistent(pDlgFactory,id)
{

	m_answer = a_OK;
	m_pColumnsPreview = NULL;
	m_bLineBetween = false;
}

AP_Dialog_Columns::~AP_Dialog_Columns(void)
{
	DELETEP(m_pColumnsPreview);
}

AP_Dialog_Columns::tAnswer AP_Dialog_Columns::getAnswer(void) const
{
	return m_answer;
}

void AP_Dialog_Columns::_createPreviewFromGC(GR_Graphics * gc,
										   UT_uint32 width,
										   UT_uint32 height)
{
	UT_ASSERT(gc);

	m_pColumnsPreview = new AP_Columns_preview(gc);
	UT_ASSERT(m_pColumnsPreview);
	
	m_pColumnsPreview->setWindowSize(width, height);
	m_pColumnsPreview->set(m_iColumns, m_bLineBetween);

}

void AP_Dialog_Columns::setColumns(UT_uint32 iColumns)
{
	m_iColumns = iColumns;

	if(m_pColumnsPreview)
		m_pColumnsPreview->set(m_iColumns, m_bLineBetween);

	enableLineBetweenControl(m_iColumns != 1);
}

void AP_Dialog_Columns::setLineBetween(bool bState)
{
	m_bLineBetween = bState;

	if(m_pColumnsPreview)
		m_pColumnsPreview->set(m_iColumns, m_bLineBetween);

}

void AP_Dialog_Columns::_drawColumnButton(GR_Graphics *gc, UT_Rect rect, UT_uint32 iColumns)
{
	gc->clearArea(rect.left, rect.top, rect.width, rect.height);

	rect.left += 2;
	rect.width -= 4;
	rect.top += 2;
	rect.height -= 4;

	m_previewDrawer.draw(gc, rect, iColumns, false);
}

	
//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

AP_Columns_preview::AP_Columns_preview(GR_Graphics * gc)
	: XAP_Preview(gc)
{

}

AP_Columns_preview::~AP_Columns_preview()
{
}

const UT_sint32 iLinesToDraw = 16;

void AP_Columns_preview::draw(void)
{
	UT_sint32 iWidth = getWindowWidth();
	UT_sint32 iHeight = getWindowHeight();

	UT_Rect pageRect(5, 5, iWidth - 10, iHeight - 10);

	m_gc->fillRect(GR_Graphics::CLR3D_Background, 0, 0, iWidth, iHeight);
	m_gc->clearArea(pageRect.left, pageRect.top, pageRect.width, 
					pageRect.height);

	m_gc->setLineWidth(1);
	m_gc->drawLine(pageRect.left, pageRect.top, 
				   pageRect.left + pageRect.width, pageRect.top);
	m_gc->drawLine(pageRect.left, pageRect.top, 
				   pageRect.left, pageRect.top + pageRect.height);

	m_gc->setLineWidth(3);
	m_gc->drawLine(pageRect.left + pageRect.width, pageRect.top + 1, 
				   pageRect.left + pageRect.width, 
				   pageRect.top + pageRect.height);
	m_gc->drawLine(pageRect.left + 1, pageRect.top + pageRect.height, 
				   pageRect.left + pageRect.width, 
				   pageRect.top + pageRect.height);


	pageRect.top += 5;
	pageRect.height -= 5;
	m_previewDrawer.draw(m_gc, pageRect, m_iColumns, m_bLineBetween);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void AP_Columns_preview_drawer::draw(GR_Graphics *gc, UT_Rect &rect, UT_sint32 iColumns, bool bLineBetween)
{

	UT_sint32 iHalfColumnGap = rect.width / 20;

	UT_sint32 y_start = rect.top + iHalfColumnGap;
	UT_sint32 y_end = rect.top + rect.height - iHalfColumnGap;

	UT_sint32 y_step = (y_end - y_start) / iLinesToDraw;
	y_step = 2;
	
	gc->setLineWidth(1);
	UT_RGBColor Line_color(0, 0, 0);
	gc->setColor(Line_color);

	rect.left += iHalfColumnGap;
	rect.width -= 2 * iHalfColumnGap;

	for(UT_sint32 y = y_start; y < y_end; y += y_step)
	{
		for (UT_sint32 i = 1; i <= iColumns; i++)
		{
			UT_sint32 xLeft, xRight;

			// a little bit of math to avoid/replace a (nasty) switch statement
			xLeft = rect.left + iHalfColumnGap + ((i-1) * rect.width / iColumns);
			xRight = rect.left - iHalfColumnGap + (i * rect.width / iColumns);
			gc->drawLine(xLeft, y, xRight, y);
		}
	}

	if(bLineBetween)
	{
		// a bit of math to avoid/replace a (nasty) switch statement
		for (UT_sint32 j = 2; j <= iColumns; j++)
		{
			UT_sint32 x;

			x = rect.left + (j-1) * rect.width / iColumns;
			gc->drawLine(x, y_start, x, y_end);
		}
	}
}
