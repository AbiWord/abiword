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

#ifndef AP_WIN32STATUSBAR_H
#define AP_WIN32STATUSBAR_H

// Class for dealing with the status bar at the bottom of
// the frame window.

#include "ut_types.h"
#include "ap_StatusBar.h"
#include "gr_Win32Graphics.h"
class XAP_Frame;

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class AP_Win32StatusBar : public AP_StatusBar
{
public:
	AP_Win32StatusBar(XAP_Frame * pFrame);
	virtual ~AP_Win32StatusBar(void);

	HWND					createWindow(HWND hwndContainer,
										 UT_uint32 left, UT_uint32 top,
										 UT_uint32 width);
	virtual void			setView(AV_View * pView);

	static UT_Bool			RegisterClass(XAP_Win32App * app);
	static LRESULT CALLBACK	_StatusBarWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam);
	
protected:
	HWND					m_hwndStatusBar;
};

#endif /* AP_WIN32STATUSBAR_H */
