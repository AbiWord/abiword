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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include <stdio.h>
#include <windows.h>
#include "ut_types.h"
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_Win32OS.h"
#include "ap_Win32LeftRuler.h"
#include "gr_Win32Graphics.h"
#include "xap_Win32App.h"
#include "ap_Win32Frame.h"
#include "ut_Win32LocaleString.h"

/*****************************************************************/

#define GWL(hwnd)		(AP_Win32LeftRuler*)GetWindowLongPtrW((hwnd), GWLP_USERDATA)
#define SWL(hwnd, f)	(AP_Win32LeftRuler*)SetWindowLongPtrW((hwnd), GWLP_USERDATA,(LONG_PTR)(f))

#define ENSUREP(p)		do { UT_ASSERT_HARMLESS(p); if (!p) goto Cleanup; } while (0)

static wchar_t s_LeftRulerWndClassName[256];

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
	GR_Win32AllocInfo ai(GetDC(m_hwndLeftRuler), m_hwndLeftRuler);
	GR_Win32Graphics * pG = (GR_Win32Graphics *)XAP_App::getApp()->newGraphics(ai);
	
	m_pG = pG;
	UT_return_if_fail (m_pG);

	pG->init3dColors();
}

/*****************************************************************/

bool AP_Win32LeftRuler::registerClass(XAP_Win32App * app)
{
	ATOM a;
	UT_Win32LocaleString str;
 	
	str.fromASCII (app->getApplicationName());
	// register class for the left ruler
	swprintf(s_LeftRulerWndClassName, L"%sLeftRuler",  str.c_str());

	a = UT_RegisterClassEx(CS_DBLCLKS | CS_OWNDC, AP_Win32LeftRuler::_LeftRulerWndProc, app->getInstance(),
						   NULL, LoadCursor(NULL, IDC_ARROW), GetSysColorBrush(COLOR_BTNFACE), NULL,
						   NULL, s_LeftRulerWndClassName);
	
	UT_ASSERT_HARMLESS(a);

	return true;
}

HWND AP_Win32LeftRuler::createWindow(HWND hwndContainer,
									UT_uint32 left, UT_uint32 top,
									UT_uint32 height)
{
		
	XAP_Win32App * app = static_cast<XAP_Win32App *>(XAP_App::getApp());
	m_hwndLeftRuler = UT_CreateWindowEx(0, s_LeftRulerWndClassName, NULL,
									 WS_CHILD | WS_VISIBLE,
									 left, top, s_iFixedWidth, height,
									 hwndContainer, NULL, app->getInstance(), NULL);
	UT_return_val_if_fail (m_hwndLeftRuler,0);
	SWL(m_hwndLeftRuler, this);
	
	
	DELETEP(m_pG);
	GR_Win32AllocInfo ai(GetDC(m_hwndLeftRuler), m_hwndLeftRuler);
	GR_Win32Graphics * pG = (GR_Win32Graphics *)XAP_App::getApp()->newGraphics(ai);

	m_pG = pG;
	UT_return_val_if_fail (pG, 0);
	pG->init3dColors();
	
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
		return UT_DefWindowProc(hwnd, iMsg, wParam, lParam);

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

	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);
			UT_return_val_if_fail(hdc && pG, 0);
			UT_Rect r(ps.rcPaint.left,ps.rcPaint.top,
					  ps.rcPaint.right-ps.rcPaint.left+1,
					  ps.rcPaint.bottom-ps.rcPaint.top+1);
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

	return UT_DefWindowProc(hwnd, iMsg, wParam, lParam);
}
