/* AbiWord - base class for selection handles
 * Copyright (c) 2012 One laptop per child
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
 *
 * Author: Carlos Garnacho <carlos@lanedo.com>
 */

#include "fv_View.h"
#include "fv_SelectionHandles.h"
#include "fp_Page.h"

FV_SelectionHandles::FV_SelectionHandles (FV_View * pView, FV_Selection pSelection)
    : m_pView (pView),
      m_pSelection (pSelection)
{
}

bool FV_SelectionHandles::_getPositionCoords(PT_DocPosition pos, UT_sint32& x, UT_sint32& y, UT_uint32& height)
{
	UT_sint32 x1, y1, x2, y2;
	UT_uint32 h;
	bool bPos, visible = true;

	m_pView->_findPositionCoords (pos,false, x1, y1,
				   x2, y2, h,
				   bPos, NULL, NULL);

        if (x1 < 0 || y1 < 0 ||
            x1 > m_pView->getWindowWidth() ||
            y1 > m_pView->getWindowHeight() - (UT_sint32) h)
          visible = false;

	x = m_pView->getGraphics()->tdu(x1);
	y = m_pView->getGraphics()->tdu(y1);
	height = m_pView->getGraphics()->tdu(h);

        return visible;
}

void FV_SelectionHandles::setCursor (PT_DocPosition cursor)
{
	UT_sint32 x, y;
	UT_uint32 height;
        bool visible;

	visible = _getPositionCoords (cursor, x, y, height);
	setCursorCoords(x, y, height, visible);
}

void FV_SelectionHandles::setSelection (PT_DocPosition start, PT_DocPosition end)
{
	UT_sint32 start_x, start_y, end_x, end_y;
	UT_uint32 start_height, end_height;
        bool start_visible, end_visible;

	start_visible = _getPositionCoords (start, start_x, start_y, start_height);
	end_visible = _getPositionCoords (end, end_x, end_y, end_height);
	setSelectionCoords(start_x, start_y, start_height, start_visible,
			   end_x, end_y, end_height, end_visible);
}

void FV_SelectionHandles::updateSelectionStart(UT_sint32 x, UT_sint32 y)
{
	PT_DocPosition pos, right;
	fp_Page *page;
        bool bBOL, bEOL, isTOC;
        UT_sint32 xClick, yClick;

	x = m_pView->getGraphics()->tlu(x);
	y = m_pView->getGraphics()->tlu(y);
	page = m_pView->_getPageForXY(x, y, xClick, yClick);
        page->mapXYToPosition(xClick, yClick, pos, bBOL, bEOL,isTOC, true, NULL);
        right = m_pView->getSelectionRightAnchor();

        pos = UT_MIN (pos, right - 1);

        m_pView->selectRange(pos, right);
}

void FV_SelectionHandles::updateSelectionEnd(UT_sint32 x, UT_sint32 y)
{
	PT_DocPosition pos, left;
	fp_Page *page;
        bool bBOL, bEOL, isTOC;
        UT_sint32 xClick, yClick;

	x = m_pView->getGraphics()->tlu(x);
	y = m_pView->getGraphics()->tlu(y);
	page = m_pView->_getPageForXY(x, y, xClick, yClick);
        page->mapXYToPosition(xClick, yClick, pos, bBOL, bEOL,isTOC, true, NULL);
        left = m_pView->getSelectionLeftAnchor();

        pos = UT_MAX (pos, left + 1);

        m_pView->selectRange(left, pos);
	m_pView->_fixInsertionPointCoords();
	m_pView->ensureInsertionPointOnScreen();
}

void FV_SelectionHandles::updateCursor(UT_sint32 x, UT_sint32 y)
{
	x = m_pView->getGraphics()->tlu(x);
	y = m_pView->getGraphics()->tlu(y);
	m_pView->warpInsPtToXY(x, y, false);
}

FV_SelectionHandles::~FV_SelectionHandles() {
}
