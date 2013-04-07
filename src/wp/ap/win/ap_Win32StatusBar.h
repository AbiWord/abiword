/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 *			 (c) 2002 Jordi Mas i Hernàndez jmas@softcatala.org
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

#ifndef AP_WIN32STATUSBAR_H
#define AP_WIN32STATUSBAR_H

// Class for dealing with the status bar at the bottom of
// the frame window.

#include "ut_types.h"
#include "ap_StatusBar.h"

class XAP_Frame;
class XAP_Win32App;

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

class ABI_EXPORT AP_Win32StatusBar : public AP_StatusBar
{
public:
	AP_Win32StatusBar(XAP_Frame * pFrame);
	virtual ~AP_Win32StatusBar(void);

	HWND					createWindow(HWND hwndFrame,
										 UT_uint32 left, UT_uint32 top,
										 UT_uint32 width);
	virtual void			setView(AV_View * pView);

	WNDPROC getOrgWndProc() { return m_pOrgStatusbarWndProc; }
	int getPrevWidth () { return m_iPrevWidth; }
	void setPrevWidth (int n) { m_iPrevWidth = n; }

	UINT                    getDir() const {return m_iDIR;}
	HWND getProgressBar() const {return m_hwndProgressBar;}
	void        showProgressBar(void);
	void        hideProgressBar(void);
protected:
	virtual void			show();
	virtual void			hide();

	HWND					m_hwndStatusBar;
	HWND					m_hwndProgressBar;
	WNDPROC			m_pOrgStatusbarWndProc;
	int						m_iPrevWidth;
	UINT                    m_iDIR;
};

#endif /* AP_WIN32STATUSBAR_H */
