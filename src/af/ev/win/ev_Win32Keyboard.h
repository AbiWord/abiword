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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */
 



#ifndef EV_WIN32KEYBOARD_H
#define EV_WIN32KEYBOARD_H

#include "ev_Keyboard.h"
#include "ev_EditBits.h"
#include "iconv.h"

class AV_View;

class ev_Win32Keyboard : public EV_Keyboard
{
public:
	ev_Win32Keyboard(EV_EditEventMapper * pEEM);
	~ev_Win32Keyboard();
	void remapKeyboard(HKL hKeyboardLayout);

	UT_Bool onKeyDown(AV_View * pView,
					  HWND hWnd, UINT iMsg, WPARAM nVirtKey, LPARAM keyData);
	UT_Bool onChar(AV_View * pView,
				   HWND hWnd, UINT iMsg, WPARAM nVirtKey, LPARAM keyData);
	
protected:
	EV_EditBits			_getModifierState(void);
	void				_translateMessage(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	void				_emitChar(AV_View * pView,
								  HWND hWnd, UINT iMsg, WPARAM nVirtKey, LPARAM keyData,
								  BYTE b, EV_EditModifierState ems);
	HKL					m_hKeyboardLayout;
	iconv_t				m_iconv; /* Selected translation to Unicode */
};

#endif /* EV_WIN32KEYBOARD_H */
