 
/*
** The contents of this file are subject to the AbiSource Public
** License Version 1.0 (the "License"); you may not use this file
** except in compliance with the License. You may obtain a copy
** of the License at http://www.abisource.com/LICENSE/ 
** 
** Software distributed under the License is distributed on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, either express or
** implied. See the License for the specific language governing
** rights and limitations under the License. 
** 
** The Original Code is AbiWord.
** 
** The Initial Developer of the Original Code is AbiSource, Inc.
** Portions created by AbiSource, Inc. are Copyright (C) 1998 AbiSource, Inc. 
** All Rights Reserved. 
** 
** Contributor(s):
**  
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
