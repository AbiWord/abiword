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

/*****************************************************************
** Only one of these is created by the application.
*****************************************************************/

#include <windows.h>
#include <commctrl.h>   // includes the common control header
#include <crtdbg.h>

#ifdef ABI_OPT_JS
#include <js.h>
#endif /* ABI_OPT_JS */

#include <stdio.h>
#include <string.h>

#include "xap_Args.h"
#include "ap_Win32Frame.h"
#include "ap_Win32App.h"

#define NrElements(a)		(sizeof(a) / sizeof(a[0]))
#define FREEP(p)	do { if (p) free(p); (p)=NULL; } while (0)
#define DELETEP(p)	do { if (p) delete(p); (p)=NULL; } while (0)

/*****************************************************************/

AP_Win32App::AP_Win32App(HINSTANCE hInstance, AP_Args * pArgs, const char * szAppName)
	: XAP_Win32App(hInstance, pArgs,szAppName)
{
	m_prefs = NULL;
}

AP_Win32App::~AP_Win32App(void)
{
	DELETEP(m_prefs);
}

UT_Bool AP_Win32App::initialize(void)
{
	// load preferences, first the builtin set and then any on disk.
	
	m_prefs = new AP_Win32Prefs(this);
	m_prefs->loadBuiltinPrefs();
	m_prefs->loadPrefsFile();

	// TODO overlay command line arguments onto preferences...
		   
	// now that preferences are established, let the xap init
		   
	return XAP_Win32App::initialize();
}

UT_Bool AP_Win32App::shutdown(void)
{
	if (m_prefs->getAutoSave())
		m_prefs->savePrefsFile();

	return UT_TRUE;
}

XAP_Prefs * AP_Win32App::getPrefs(void) const
{
	return m_prefs;
}

UT_Bool AP_Win32App::getPrefsValue(const XML_Char * szKey, const XML_Char ** pszValue) const
{
	if (!m_prefs)
		return UT_FALSE;

	return m_prefs->getPrefsValue(szKey,pszValue);
}

/*****************************************************************/

#ifdef   _DEBUG
#define  SET_CRT_DEBUG_FIELD(a) \
            _CrtSetDbgFlag((a) | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
#define  CLEAR_CRT_DEBUG_FIELD(a) \
            _CrtSetDbgFlag(~(a) & _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
#else
#define  SET_CRT_DEBUG_FIELD(a)   ((void) 0)
#define  CLEAR_CRT_DEBUG_FIELD(a) ((void) 0)
#endif

/*****************************************************************/

int AP_Win32App::WinMain(const char * szAppName, HINSTANCE hInstance, 
						 HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow)
{
	// this is a static function and doesn't have a 'this' pointer.
	HWND hwnd;
	MSG msg;

	// TODO the following two variables are a temporary hack.
	// TODO we need to convert szCmdLine into argc/argv format.
	int argc = 0;
	char ** argv = NULL;

	_CrtSetReportMode( _CRT_WARN, _CRTDBG_MODE_DEBUG );
	_CrtSetReportMode( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
	_CrtSetReportMode( _CRT_ASSERT, _CRTDBG_MODE_DEBUG );

	// Ensure that common control DLL is loaded

	INITCOMMONCONTROLSEX icex;
	icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC = ICC_COOL_CLASSES | ICC_BAR_CLASSES;	// load the rebar and toolbar
	InitCommonControlsEx(&icex);

	// initialize our application.

	AP_Args Args = AP_Args(argc,argv);
	
	AP_Win32App * pMyWin32App = new AP_Win32App(hInstance, &Args, szAppName);
	pMyWin32App->initialize();

	// create the first window.

	AP_Win32Frame * pFirstWin32Frame = new AP_Win32Frame(pMyWin32App);
	pFirstWin32Frame->initialize();
	hwnd = pFirstWin32Frame->getTopLevelWindow();
	
	{
		// Yuck!  strtok is a horrible way to do this
		char*	p = strdup(szCmdLine);
		char*	q = strtok(p, " ");

		/*
		  Yuck!  This command-line parsing loop is horrible.
		  It's a barely-functional hack which allows us to pass
		  a script using -script on the command line.  Replace
		  this ugliness.
		*/
		while (q)
		{
			if (0 == strcmp(q, "-script"))
			{
				q = strtok(NULL, " ");
#ifdef ABI_OPT_JS			
				js_eval_file(pMyWin32App->getInterp(), q);
#endif /* ABI_OPT_JS */
				
				q = strtok(NULL, " ");
			}
			else
			{
				break;
			}
		}
		
		if (!pFirstWin32Frame->loadDocument(q))
		{
			// TODO: warn user that q didn't work?
			
			// fallback, open empty document so we don't crash
			pFirstWin32Frame->loadDocument(NULL);
		}

		free(p);
	}
	

	ShowWindow(hwnd, iCmdShow);
	UpdateWindow(hwnd);

	// turn over control to windows

	while (GetMessage(&msg, NULL, 0, 0))
	{
		// Note: we do not call TranslateMessage() because
		// Note: the keybinding mechanism is responsible
		// Note: for deciding if/when to do this.

		DispatchMessage(&msg);
	}

	// destroy the App.  It should take care of deleting all frames.

	pMyWin32App->shutdown();
	delete pMyWin32App;

	SET_CRT_DEBUG_FIELD( _CRTDBG_LEAK_CHECK_DF );

	return msg.wParam;
}

