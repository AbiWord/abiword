/* AbiSource Application Framework
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
#include "ut_vector.h"
#include "ap_Win32DialogFactory.h"
class AP_Win32App;
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
	AP_Win32Frame(AP_Win32App * app);
	AP_Win32Frame(AP_Win32Frame * f);
	~AP_Win32Frame(void);

	virtual UT_Bool				initialize(void);
	virtual	AP_Frame *			cloneFrame(void);
	virtual UT_Bool				loadDocument(const char * szFilename);
	virtual UT_Bool				close(void);
	virtual UT_Bool				raise(void);
	virtual UT_Bool				show(void);
	virtual UT_Bool				updateTitle(void);

	HWND						getTopLevelWindow(void) const;
	HWND						getToolbarWindow(void) const;
	EV_Win32Mouse *				getWin32Mouse(void);
	ev_Win32Keyboard *			getWin32Keyboard(void);

	virtual AP_DialogFactory *	getDialogFactory(void);

	static UT_Bool				RegisterClass(AP_Win32App * app);

protected:
	void						_createTopLevelWindow(void);
	UT_Bool						_showDocument(void);
	static void					_scrollFunc(void * pData, UT_sint32 xoff, UT_sint32 yoff);
	static LRESULT CALLBACK		_WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	static LRESULT CALLBACK		_ChildWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);

	// TODO see why ev_Win32Keyboard has lowercase prefix...
	AP_Win32App *				m_pWin32App;
	ev_Win32Keyboard *			m_pWin32Keyboard;
	EV_Win32Mouse *				m_pWin32Mouse;
	EV_Win32Menu *				m_pWin32Menu;
	UT_Vector					m_vecWin32Toolbars;
	UT_uint32					m_iBarHeight;
	
	HWND						m_hwndFrame;
	HWND						m_hwndRebar;
	HWND						m_hwndChild;

	AP_Win32DialogFactory		m_dialogFactory;
};

#endif /* AP_WIN32FRAME_H */
