/* AbiSource Program Utilities
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
 

#ifndef UT_Win32OS_H
#define UT_Win32OS_H

#include <windows.h>
#include "ut_types.h"

OSVERSIONINFO& UT_GetWinVersion(void);
bool UT_IsWinVista(void);
bool UT_IsWinNT(void);
bool UT_IsWin2K(void);
bool UT_IsWin95(void);

DLGTEMPLATE * WINAPI UT_LockDlgRes(HINSTANCE hinst, LPCTSTR lpszResName);

char * UT_GetDefaultPrinterName();

HDC  UT_GetDefaultPrinterDC();

ATOM UT_RegisterClassEx(UINT style, WNDPROC wndproc, HINSTANCE hInstance,
						HICON hIcon, HCURSOR hCursor, HBRUSH hbrBackground, HICON hIconSm,
						const char * menu, const char * name, bool bForceANSI = false);

// NB: the default value for bForceANSI is intentionally set to true, otherwise the
// tooltips do not work, see bug 8976
HWND UT_CreateWindowEx(DWORD dwExStyle, LPCTSTR lpClassName, LPCTSTR lpWindowName, DWORD dwStyle,
					   int x, int y, int nWidth, int nHeight,
					   HWND hWndParent, HMENU hMenu, HINSTANCE hInstance, LPVOID lpParam, bool bForceANSI = true);

LRESULT UT_DefWindowProc(HWND hWnd, UINT Msg, WPARAM wParam,LPARAM lParam, bool bForceANSI = false);

BOOL UT_SetWindowText(HWND hWnd, const char * lpString, bool bForceANSI = false);

// If you change bForceANSI, you must also change m_bForceAnsi in ap_Win32App.h
BOOL UT_GetMessage(LPMSG lpMsg,HWND hWnd,UINT wMsgFilterMin,UINT wMsgFilterMax, bool bForceANSI = false);

LRESULT UT_DispatchMessage(const MSG *lpmsg, bool bForceANSI = false);
#endif /* UT_Win32OS_H */
