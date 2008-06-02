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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */


#ifndef EV_X11KEYBOARD_H
#define EV_X11KEYBOARD_H

#include "ev_UnixKeyboard.h"


class EV_X11Keyboard : public EV_UnixKeyboard
{
	friend EV_UnixKeyboard * EV_UnixKeyboard::create(EV_EditEventMapper * pEEM);

public:
	virtual ~EV_X11Keyboard(void);

	bool keyPressEvent(AV_View * pView, GdkEventKey* e);
	virtual GdkModifierType getAltModifierMask(void);

protected:
	// only instantiable via parent's factory method create().
	EV_X11Keyboard(EV_EditEventMapper * pEEM);

private:
	int m_altMask;
};

#endif // EV_X11KEYBOARD_H

