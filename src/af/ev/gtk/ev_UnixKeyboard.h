/* AbiSource Program Utilities
 * Copyright (C) 1998-2000 AbiSource, Inc.
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


#ifndef EV_UNIXKEYBOARD_H
#define EV_UNIXKEYBOARD_H

#include <gdk/gdk.h>
#include "ev_Keyboard.h"
#include "ev_EditBits.h"

class AV_View;


class ev_UnixKeyboard : public EV_Keyboard
{
public:
	ev_UnixKeyboard(EV_EditEventMapper * pEEM);
	virtual ~ev_UnixKeyboard(void);

	bool keyPressEvent(AV_View * pView, GdkEventKey* e);
	bool charDataEvent (AV_View * pView, EV_EditBits state, const char * txt, size_t len);
};

#endif // EV_UNIXKEYBOARD_H

