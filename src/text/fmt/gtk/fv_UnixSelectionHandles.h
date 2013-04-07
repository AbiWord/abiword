/* AbiWord - unix impl for selection handles
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

#ifndef FV_UNIXSELECTIONHANDLES_H
#define FV_UNIXSELECTIONHANDLES_H

#include "fv_SelectionHandles.h"
#include "gtktexthandleprivate.h"

class ABI_EXPORT FV_UnixSelectionHandles : public FV_SelectionHandles
{
	friend class fv_View;

public:
	FV_UnixSelectionHandles (FV_View * pView, FV_Selection selection);
	virtual ~FV_UnixSelectionHandles();

	virtual void hide(void);
	virtual void setCursorCoords (UT_sint32 x, UT_sint32 y, UT_uint32 height, bool visible);
	virtual void setSelectionCoords (UT_sint32 start_x, UT_sint32 start_y, UT_uint32 start_height, bool start_visible,
					 UT_sint32 end_x, UT_sint32 end_y, UT_uint32 end_height, bool end_visible);

private:
        FvTextHandle *m_text_handle;
};

#endif /* FV_UNIXSELECTIONHANDLES_H */
