/* AbiSource Program Utilities
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



#ifndef EV_UNIXMOUSE_H
#define EV_UNIXMOUSE_H

#include <gdk/gdk.h>

#include "ut_types.h"
#include "ev_Mouse.h"
#include "ev_EditBits.h"

/*****************************************************************/

class EV_UnixMouse : public EV_Mouse
{
public:
	EV_UnixMouse(EV_EditEventMapper * pEEM);

	void mouseClick(AV_View* pView, GdkEventButton* e);
	void mouseUp(AV_View* pView, GdkEventButton* e);
	void mouseMotion(AV_View* pView, GdkEventMotion *event);
	void mouseScroll(AV_View* pView, GdkEventScroll *e);

protected:
};

#endif // EV_UNIXMOUSE_H
