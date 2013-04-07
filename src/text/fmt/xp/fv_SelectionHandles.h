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

#ifndef FV_SELECTIONHANDLES_H
#define FV_SELECTIONHANDLES_H

#include "pt_Types.h"
#include "fv_Selection.h"

class FV_View;

class ABI_EXPORT FV_SelectionHandles
{
public:
	FV_SelectionHandles (FV_View * pView, FV_Selection selection);
	virtual ~FV_SelectionHandles();

	virtual void hide(void) {}
	virtual void setCursorCoords(UT_sint32 /*x*/, UT_sint32 /*y*/, UT_uint32 /*height*/, bool /*visible*/) {}
	virtual void setSelectionCoords(UT_sint32 /*start_x*/, UT_sint32 /*start_y*/, UT_uint32 /*start_height*/, bool /*start_visible*/,
	                                UT_sint32 /*end_x*/, UT_sint32 /*end_y*/, UT_uint32 /*end_height*/, bool /*end_visible*/) {}

	void setCursor(PT_DocPosition cursor);
	void setSelection(PT_DocPosition start, PT_DocPosition end);

	void updateSelectionStart(UT_sint32 x, UT_sint32 y);
	void updateSelectionEnd(UT_sint32 x, UT_sint32 y);
	void updateCursor(UT_sint32 x, UT_sint32 y);

protected:
	FV_View *             m_pView;
	FV_Selection          m_pSelection;

private:
	bool _getPositionCoords(PT_DocPosition pos, UT_sint32& x, UT_sint32& y, UT_uint32& height);
};

#endif /* FV_SELECTIONHANDLES_H */
