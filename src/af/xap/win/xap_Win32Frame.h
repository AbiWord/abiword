/* AbiWord
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


#ifndef AP_WIN32FRAME_H
#define AP_WIN32FRAME_H

#include <windows.h>
#include "ap_Frame.h"
class AP_Win32Ap;
class ev_Win32Keyboard;
class EV_Win32Mouse;
class EV_Win32Menu;

/*****************************************************************
******************************************************************
** This file defines the Win32-platform-specific class for the
** cross-platform application frame.  This is used to hold all
** Win32-specific data.  One of these is created for each top-level
** document window.
******************************************************************
*****************************************************************/

class AP_Win32Frame : public AP_Frame
{
public:
	AP_Win32Frame(AP_Win32Ap * ap);
	~AP_Win32Frame(void);

	virtual UT_Bool				initialize(int * pArgc, char *** pArgv);
	virtual UT_Bool				loadDocument(const char * szFilename);

	HWND						getTopLevelWindow(void) const;
	EV_Win32Mouse *				getWin32Mouse(void);
	ev_Win32Keyboard *			getWin32Keyboard(void);

	static UT_Bool				RegisterClass(AP_Win32Ap * ap);

protected:
	void						_createTopLevelWindow(void);
	static void					_scrollFunc(void * pData, UT_sint32 xoff, UT_sint32 yoff);
	static LRESULT CALLBACK		_WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

	// TODO see why ev_Win32Keyboard has lowercase prefix...
	AP_Win32Ap *				m_pWin32Ap;
	ev_Win32Keyboard *			m_pWin32Keyboard;
	EV_Win32Mouse *				m_pWin32Mouse;
	EV_Win32Menu *				m_pWin32Menu;
	
	HWND						m_hwnd;
};

#endif /* AP_WIN32FRAME_H */
