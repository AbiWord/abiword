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

#include <windows.h>
#include <stdio.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ap_Win32TopRuler.h"
#include "gr_Win32Graphics.h"
#include "xap_Win32App.h"
#include "ap_Win32Frame.h"

/*****************************************************************/

#define GWL(hwnd)		(AP_Win32Frame*)GetWindowLong((hwnd), GWL_USERDATA)
#define SWL(hwnd, f)	(AP_Win32Frame*)SetWindowLong((hwnd), GWL_USERDATA,(LONG)(f))

#define DELETEP(p)		do { if (p) delete p; p = NULL; } while (0)
#define REPLACEP(p,q)	do { if (p) delete p; p = q; } while (0)
#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

static char s_TopRulerWndClassName[256];

/*****************************************************************/

AP_Win32TopRuler::AP_Win32TopRuler(XAP_Frame * pFrame)
	: AP_TopRuler(pFrame)
{
	m_pG = NULL;
	m_hwndTopRuler = NULL;
}

AP_Win32TopRuler::~AP_Win32TopRuler(void)
{
	DELETEP(m_pG);
}

void AP_Win32TopRuler::setView(AV_View * pView)
{
	AP_TopRuler::setView(pView);

	DELETEP(m_pG);
	m_pG = new Win32Graphics(GetDC(m_hwndTopRuler), m_hwndTopRuler);
	UT_ASSERT(m_pG);
}

/*****************************************************************/

void AP_Win32TopRuler::scrollRuler(UT_sint32 xoff)
{
	// platform-dependent scrolling for the ruler
	UT_DEBUGMSG(("Win32TopRuler: received scrollRuler [x %d]\n",xoff));
}

/*****************************************************************/

UT_Bool AP_Win32TopRuler::RegisterClass(AP_Win32App * app)
{
	WNDCLASSEX  wndclass;
	
	// register class for the top ruler
	sprintf(s_TopRulerWndClassName, "%sTopRuler", app->getApplicationName());

	memset(&wndclass, 0, sizeof(wndclass));
	wndclass.cbSize        = sizeof(wndclass);
	wndclass.style         = CS_DBLCLKS;
	wndclass.lpfnWndProc   = AP_Win32TopRuler::_TopRulerWndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = app->getInstance();
	wndclass.hIcon         = NULL;
	wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = s_TopRulerWndClassName;
	wndclass.hIconSm       = NULL;

	if (!RegisterClassEx(&wndclass))
	{
		DWORD err = GetLastError();
		UT_ASSERT(err);
		return UT_FALSE;
	}

	return UT_TRUE;
}

HWND AP_Win32TopRuler::createWindow(HWND hwndContainer,
									UT_uint32 left, UT_uint32 top,
									UT_uint32 width, UT_uint32 height)
{
	AP_Win32App * app = static_cast<AP_Win32App *>(m_pFrame->getApp());
	m_hwndTopRuler = CreateWindowEx(0, s_TopRulerWndClassName, NULL,
									WS_CHILD | WS_VISIBLE,
									left, top, width, height,
									hwndContainer, NULL, app->getInstance(), NULL);
	UT_ASSERT(m_hwndTopRuler);
	SWL(m_hwndTopRuler, this);

	return m_hwndTopRuler;
}

LRESULT CALLBACK AP_Win32TopRuler::_TopRulerWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	UT_DEBUGMSG(("TopRuler: msg 0x%08lx\n",iMsg));
	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}
