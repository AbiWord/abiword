/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */ 


#ifndef EV_UNIXKEYBOARD_H
#define EV_UNIXKEYBOARD_H

#include <gdk/gdk.h>
#include "ev_Keyboard.h"
#include "ev_EditBits.h"

class FV_View;


class ev_UnixKeyboard : public EV_Keyboard
{
public:
	ev_UnixKeyboard(EV_EditEventMapper * pEEM);

	UT_Bool keyPressEvent(FV_View * pView, GdkEventKey* e);
};

#endif // EV_UNIXKEYBOARD_H

