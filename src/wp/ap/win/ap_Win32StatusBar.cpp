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
#include "ap_Win32StatusBar.h"
#include "gr_Win32Graphics.h"
#include "xap_Win32App.h"
#include "ap_Win32Frame.h"

/*****************************************************************/

#define GWL(hwnd)		(AP_Win32StatusBar*)GetWindowLong((hwnd), GWL_USERDATA)
#define SWL(hwnd, f)	(AP_Win32StatusBar*)SetWindowLong((hwnd), GWL_USERDATA,(LONG)(f))

#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

static char s_StatusBarWndClassName[256];

/*****************************************************************/

AP_Win32StatusBar::AP_Win32StatusBar(XAP_Frame * pFrame)
	: AP_StatusBar(pFrame)
{
	m_hwndStatusBar = NULL;
	m_pG = NULL;
}

AP_Win32StatusBar::~AP_Win32StatusBar(void)
{
	DELETEP(m_pG);
}

void AP_Win32StatusBar::setView(AV_View * pView)
{
	DELETEP(m_pG);
	GR_Win32Graphics * pG = new GR_Win32Graphics(GetDC(m_hwndStatusBar), m_hwndStatusBar, pView->getApp());
	m_pG = pG;
	UT_ASSERT(m_pG);

	pG->init3dColors();
	
	GR_Font * pFont = m_pG->getGUIFont();
	m_pG->setFont(pFont);

	AP_StatusBar::setView(pView);
}

/*****************************************************************/

UT_Bool AP_Win32StatusBar::RegisterClass(XAP_Win32App * app)
{
	WNDCLASSEX  wndclass;
	ATOM a;
	
	// register class for the status bar
	sprintf(s_StatusBarWndClassName, "%sStatusBar", app->getApplicationName());

	memset(&wndclass, 0, sizeof(wndclass));
	wndclass.cbSize        = sizeof(wndclass);
	wndclass.style         = CS_OWNDC;
	wndclass.lpfnWndProc   = AP_Win32StatusBar::_StatusBarWndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = app->getInstance();
	wndclass.hIcon         = NULL;
	wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = s_StatusBarWndClassName;
	wndclass.hIconSm       = NULL;

	a = RegisterClassEx(&wndclass);
	UT_ASSERT(a);

	return UT_TRUE;
}

HWND AP_Win32StatusBar::createWindow(HWND hwndFrame,
									 UT_uint32 left, UT_uint32 top,
									 UT_uint32 width)
{
	XAP_Win32App * app = static_cast<XAP_Win32App *>(m_pFrame->getApp());
	m_hwndStatusBar = CreateWindowEx(0, s_StatusBarWndClassName, NULL,
									WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
									left, top, width, s_iFixedHeight,
									hwndFrame, NULL, app->getInstance(), NULL);
	UT_ASSERT(m_hwndStatusBar);
	SWL(m_hwndStatusBar, this);

	RECT rSize;
	GetClientRect(m_hwndStatusBar,&rSize);
	setHeight(rSize.bottom);
	setWidth(rSize.right);

	int cxVScroll = GetSystemMetrics(SM_CXVSCROLL);
	int cyHScroll = GetSystemMetrics(SM_CYHSCROLL);
	m_hwndSizeGrip = CreateWindowEx(0,"ScrollBar",NULL,
									WS_CHILD | WS_VISIBLE | SBS_SIZEGRIP,
									rSize.right-cxVScroll, rSize.bottom-cyHScroll, cxVScroll, cyHScroll,
									m_hwndStatusBar, NULL, app->getInstance(), NULL);
	UT_ASSERT(m_hwndSizeGrip);
	SWL(m_hwndSizeGrip, this);

	return m_hwndStatusBar;
}
	
LRESULT CALLBACK AP_Win32StatusBar::_StatusBarWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	// this is a static member function.

	AP_Win32StatusBar * pStatusBar = GWL(hwnd);

	if (!pStatusBar)
		return DefWindowProc(hwnd, iMsg, wParam, lParam);
		
	switch (iMsg)
	{
	case WM_SIZE:
		{
			int nWidth = LOWORD(lParam);
			int nHeight = HIWORD(lParam);
			pStatusBar->setHeight(nHeight);
			pStatusBar->setWidth(nWidth);
			int cxVScroll = GetSystemMetrics(SM_CXVSCROLL);
			int cyHScroll = GetSystemMetrics(SM_CYHSCROLL);
			MoveWindow(pStatusBar->m_hwndSizeGrip, nWidth-cxVScroll, nHeight-cyHScroll, cxVScroll, cyHScroll, TRUE);

			return 0;
		}
	
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			pStatusBar->draw();
			EndPaint(hwnd,&ps);
			return 0;
		}

	case WM_SYSCOLORCHANGE:
		{
			GR_Win32Graphics * pG = static_cast<GR_Win32Graphics *>(pStatusBar->m_pG);
			pG->init3dColors();
			return 0;
		}
		
	default:
		break;
	}

	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}
