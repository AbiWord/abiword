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
#include <crtdbg.h>

#include "ap_Win32Ap.h"
#include "ap_Win32Frame.h"

#ifdef   _DEBUG
#define  SET_CRT_DEBUG_FIELD(a) \
            _CrtSetDbgFlag((a) | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
#define  CLEAR_CRT_DEBUG_FIELD(a) \
            _CrtSetDbgFlag(~(a) & _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
#else
#define  SET_CRT_DEBUG_FIELD(a)   ((void) 0)
#define  CLEAR_CRT_DEBUG_FIELD(a) ((void) 0)
#endif

int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    PSTR szCmdLine, int iCmdShow)
{
	static char szAppName[] = "Abi" ;
	HWND        hwnd ;
	MSG         msg ;

	int argc = 0;
	char ** argv = NULL;

	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_DEBUG );
	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
	_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_DEBUG );

	// initialize our application.

	AP_Win32Ap * pMyWin32Ap = new AP_Win32Ap(hInstance);
	pMyWin32Ap->initialize(&argc,&argv);

	// create the first window.

	AP_Win32Frame * pFirstWin32Frame = new AP_Win32Frame(pMyWin32Ap);
	pFirstWin32Frame->initialize(&argc,&argv);
	hwnd = pFirstWin32Frame->getTopLevelWindow();
	
	// TODO convert szCmdLine into argc, argv format??
	// TODO for now, we just treat it as a filename.

	pFirstWin32Frame->loadDocument(szCmdLine);

	ShowWindow (hwnd, iCmdShow) ;
	UpdateWindow (hwnd) ;

	// turn over control to windows

	while (GetMessage (&msg, NULL, 0, 0))
	{
//		TranslateMessage (&msg) ;
		DispatchMessage (&msg) ;
	}

	// destroy the Ap.  It should take care of deleting all frames.

	delete pMyWin32Ap;

	SET_CRT_DEBUG_FIELD( _CRTDBG_LEAK_CHECK_DF );

	return msg.wParam ;
}
