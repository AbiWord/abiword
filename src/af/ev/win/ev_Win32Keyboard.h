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



#ifndef EV_WIN32KEYBOARD_H
#define EV_WIN32KEYBOARD_H

#include "ev_Keyboard.h"
#include "ev_EditBits.h"

// TODO should pView be passed in on each method or
// TODO should we pass it to the ev_keyboard base class on the
// TODO constructor ??

class FV_View;

class ev_Win32Keyboard : public EV_Keyboard
{
public:
	ev_Win32Keyboard(EV_EditEventMapper * pEEM);

	UT_Bool onKeyDown(FV_View * pView,
					  HWND hWnd, UINT iMsg, WPARAM nVirtKey, LPARAM keyData);
	UT_Bool onChar(FV_View * pView,
				   HWND hWnd, UINT iMsg, WPARAM nVirtKey, LPARAM keyData);
	
protected:
	EV_EditBits			_getModifierState(void);
	void				_translateMessage(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
};

#endif /* EV_WIN32KEYBOARD_H */
