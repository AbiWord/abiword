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
#include "ap_Win32LeftRuler.h"
#include "gr_Win32Graphics.h"
#include "xap_Win32App.h"
#include "ap_Win32Frame.h"

/*****************************************************************/

#define GWL(hwnd)		(AP_Win32LeftRuler*)GetWindowLong((hwnd), GWL_USERDATA)
#define SWL(hwnd, f)	(AP_Win32LeftRuler*)SetWindowLong((hwnd), GWL_USERDATA,(LONG)(f))

#define ENSUREP(p)		do { UT_ASSERT(p); if (!p) goto Cleanup; } while (0)

static char s_LeftRulerWndClassName[256];

/*****************************************************************/

AP_Win32LeftRuler::AP_Win32LeftRuler(XAP_Frame * pFrame)
	: AP_LeftRuler(pFrame)
{
	m_pG = NULL;
	m_hwndLeftRuler = NULL;
}

AP_Win32LeftRuler::~AP_Win32LeftRuler(void)
{
	DELETEP(m_pG);

	if (m_hwndLeftRuler)
	{
		if (IsWindow(m_hwndLeftRuler))
			DestroyWindow(m_hwndLeftRuler);
	}
}

void AP_Win32LeftRuler::setView(AV_View * pView)
{
	AP_LeftRuler::setView(pView);

	DELETEP(m_pG);
	GR_Win32Graphics * pG = new GR_Win32Graphics(GetDC(m_hwndLeftRuler), m_hwndLeftRuler, m_pFrame->getApp());
	m_pG = pG;
	UT_ASSERT(m_pG);

	pG->init3dColors();
}

/*****************************************************************/

bool AP_Win32LeftRuler::RegisterClass(XAP_Win32App * app)
{
	WNDCLASSEX  wndclass;
	ATOM a;
	
	// register class for the top ruler
	sprintf(s_LeftRulerWndClassName, "%sLeftRuler", app->getApplicationName());

	memset(&wndclass, 0, sizeof(wndclass));
	wndclass.cbSize        = sizeof(wndclass);
	wndclass.style         = CS_DBLCLKS | CS_OWNDC;
	wndclass.lpfnWndProc   = AP_Win32LeftRuler::_LeftRulerWndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = app->getInstance();
	wndclass.hIcon         = NULL;
	wndclass.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = GetSysColorBrush(COLOR_BTNFACE);
	wndclass.lpszMenuName  = NULL;
	wndclass.lpszClassName = s_LeftRulerWndClassName;
	wndclass.hIconSm       = NULL;

	a = RegisterClassEx(&wndclass);
	UT_ASSERT(a);

	return true;
}

HWND AP_Win32LeftRuler::createWindow(HWND hwndContainer,
									UT_uint32 left, UT_uint32 top,
									UT_uint32 height)
{
		
	XAP_Win32App * app = static_cast<XAP_Win32App *>(m_pFrame->getApp());
	m_hwndLeftRuler = CreateWindowEx(0, s_LeftRulerWndClassName, NULL,
									 WS_CHILD | WS_VISIBLE,
									 left, top, s_iFixedWidth, height,
									 hwndContainer, NULL, app->getInstance(), NULL);
	UT_ASSERT(m_hwndLeftRuler);
	SWL(m_hwndLeftRuler, this);
	
	
	DELETEP(m_pG);
	GR_Win32Graphics * pG = new GR_Win32Graphics(GetDC(m_hwndLeftRuler), m_hwndLeftRuler, m_pFrame->getApp());
	m_pG = pG;
	pG->init3dColors();
	UT_ASSERT(m_pG);
	
	RECT rSize;
	GetClientRect(m_hwndLeftRuler,&rSize);
	setHeight(rSize.bottom);
	setWidth(rSize.right);

	return m_hwndLeftRuler;
}

static EV_EditModifierState s_GetEMS(WPARAM fwKeys)
{
	EV_EditModifierState ems = 0;

	if (fwKeys & MK_SHIFT)
		ems |= EV_EMS_SHIFT;
	if (fwKeys & MK_CONTROL)
		ems |= EV_EMS_CONTROL;
	if (GetKeyState(VK_MENU) & 0x8000)
		ems |= EV_EMS_ALT;

	return ems;
}
	
LRESULT CALLBACK AP_Win32LeftRuler::_LeftRulerWndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	// this is a static member function.

	AP_Win32LeftRuler * pRuler = GWL(hwnd);

	if (!pRuler)
		return DefWindowProc(hwnd, iMsg, wParam, lParam);

	GR_Win32Graphics * pG = static_cast<GR_Win32Graphics *>(pRuler->m_pG);
		
	switch (iMsg)
	{
	case WM_LBUTTONDOWN:
		SetCapture(hwnd);
		pRuler->mousePress(s_GetEMS(wParam),EV_EMB_BUTTON1,pG->tlu(LOWORD(lParam)), pG->tlu(HIWORD(lParam)));
		{
			pG->handleSetCursorMessage();
		}
		return 0;
		
	case WM_MBUTTONDOWN:
		SetCapture(hwnd);
		pRuler->mousePress(s_GetEMS(wParam),EV_EMB_BUTTON2,pG->tlu(LOWORD(lParam)),pG->tlu(HIWORD(lParam)));
		{
			pG->handleSetCursorMessage();
		}
		return 0;
		
	case WM_RBUTTONDOWN:
		SetCapture(hwnd);
		pRuler->mousePress(s_GetEMS(wParam),EV_EMB_BUTTON3,pG->tlu(LOWORD(lParam)),pG->tlu(HIWORD(lParam)));
		{
			pG->handleSetCursorMessage();
		}
		return 0;
		
	case WM_MOUSEMOVE:
		pRuler->mouseMotion(s_GetEMS(wParam),pG->tlu(signedLoWord(lParam)),pG->tlu(signedHiWord(lParam)));
		return 0;

	case WM_LBUTTONUP:
		pRuler->mouseRelease(s_GetEMS(wParam),EV_EMB_BUTTON1,pG->tlu(signedLoWord(lParam)),pG->tlu(signedHiWord(lParam)));
		ReleaseCapture();
		return 0;

	case WM_MBUTTONUP:
		pRuler->mouseRelease(s_GetEMS(wParam),EV_EMB_BUTTON2,pG->tlu(signedLoWord(lParam)),pG->tlu(signedHiWord(lParam)));
		ReleaseCapture();
		return 0;
		
	case WM_RBUTTONUP:
		pRuler->mouseRelease(s_GetEMS(wParam),EV_EMB_BUTTON3,pG->tlu(signedLoWord(lParam)),pG->tlu(signedHiWord(lParam)));
		ReleaseCapture();
		return 0;

	case WM_LBUTTONDBLCLK:
	case WM_MBUTTONDBLCLK:
	case WM_RBUTTONDBLCLK:
		return 0;

	case WM_SIZE:
		{
			int nWidth = LOWORD(lParam);
			int nHeight = HIWORD(lParam);	
						
			pRuler->setHeight(nHeight);
			pRuler->setWidth(nWidth);
			return 0;
		}
	
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			UT_Rect r(pG->tlu(ps.rcPaint.left),pG->tlu(ps.rcPaint.top),
					  pG->tlu(ps.rcPaint.right-ps.rcPaint.left+1),
					  pG->tlu(ps.rcPaint.bottom-ps.rcPaint.top+1));
			pRuler->draw(&r);
			EndPaint(hwnd,&ps);
			return 0;
		}

	case WM_SYSCOLORCHANGE:
		{
			pG->init3dColors();
			return 0;
		}

	case WM_SETCURSOR:
		{
			pG->handleSetCursorMessage();
		}
		return 0;
	default:
		break;
	}

	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}
