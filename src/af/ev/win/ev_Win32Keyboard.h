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


#ifndef EV_WIN32KEYBOARD_H
#define EV_WIN32KEYBOARD_H

#include "ev_Keyboard.h"
#include "ev_EditBits.h"
#include "ut_iconv.h"

class AV_View;

class ev_Win32Keyboard : public EV_Keyboard
{
public:
	ev_Win32Keyboard(EV_EditEventMapper * pEEM);
	virtual ~ev_Win32Keyboard();

	void remapKeyboard(HKL hKeyboardLayout);

	bool onKeyDown(AV_View * pView,
					  HWND hWnd, UINT iMsg, WPARAM nVirtKey, LPARAM keyData);

	bool onIMEChar(AV_View * pView,
					  HWND hWnd, UINT iMsg, WPARAM nVirtKey, LPARAM keyData);

	bool onChar(AV_View * pView,
				   HWND hWnd, UINT iMsg, WPARAM nVirtKey, LPARAM keyData);

protected:
	EV_EditBits			_getModifierState(void);
	void				_translateMessage(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	void				_emitChar(AV_View * pView,
								  HWND hWnd, UINT iMsg, WPARAM nVirtKey, LPARAM keyData,
								  WCHAR b, EV_EditModifierState ems);
	int					_scanCodeToChars(UINT nVirtKey, UINT wScanCode,
										 CONST PBYTE lpKeyState, LPWSTR pwszBuff, int cchBuff);
	HKL					m_hKeyboardLayout;
	UT_iconv_t			m_iconv; /* Selected translation to Unicode */
	bool				m_bIsUnicodeInput;
};

#endif /* EV_WIN32KEYBOARD_H */
