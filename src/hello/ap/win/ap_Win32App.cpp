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
#include <sys/types.h>
#include <sys/stat.h>

#ifdef ABI_OPT_JS
#include <js.h>
#endif /* ABI_OPT_JS */

#include <stdio.h>
#include <string.h>

#include "ut_debugmsg.h"
#include "xap_Args.h"
#include "ap_Win32Frame.h"
#include "ap_Win32App.h"
#include "sp_spell.h"
#include "ap_Strings.h"
#include "xap_EditMethods.h"
#include "ap_LoadBindings.h"
#include "xap_Menu_ActionSet.h"
#include "xap_Toolbar_ActionSet.h"

/*****************************************************************/

AP_Win32App::AP_Win32App(HINSTANCE hInstance, XAP_Args * pArgs, const char * szAppName)
	: XAP_Win32App(hInstance, pArgs,szAppName)
{
	m_pStringSet = NULL;
}

AP_Win32App::~AP_Win32App(void)
{
	DELETEP(m_pStringSet);
}

static bool s_createDirectoryIfNecessary(const char * szDir)
{
	struct _stat statbuf;
	
	if (_stat(szDir,&statbuf) == 0)								// if it exists
	{
		if ( (statbuf.st_mode & _S_IFDIR) == _S_IFDIR )			// and is a directory
			return true;

		UT_DEBUGMSG(("Pathname [%s] is not a directory.\n",szDir));
		return false;
	}

	if (CreateDirectory(szDir,NULL))
		return true;

	UT_DEBUGMSG(("Could not create Directory [%s].\n",szDir));
	return false;
}	

bool AP_Win32App::initialize(void)
{
	const char * szUserPrivateDirectory = getUserPrivateDirectory();
	bool bVerified = s_createDirectoryIfNecessary(szUserPrivateDirectory);
	UT_ASSERT(bVerified);

	// load preferences, first the builtin set and then any on disk.
	
	m_prefs = new AP_Win32Prefs(this);
	m_prefs->loadBuiltinPrefs();
	m_prefs->loadPrefsFile();

	// TODO overlay command line arguments onto preferences...
		   
	// now that preferences are established, let the xap init
		   
	m_pEMC = AP_GetEditMethods();
	UT_ASSERT(m_pEMC);

	m_pBindingSet = new AP_BindingSet(m_pEMC);
	UT_ASSERT(m_pBindingSet);
	
	m_pMenuActionSet = AP_CreateMenuActionSet();
	UT_ASSERT(m_pMenuActionSet);

	m_pToolbarActionSet = AP_CreateToolbarActionSet();
	UT_ASSERT(m_pToolbarActionSet);

	if (! XAP_Win32App::initialize())
		return false;

	// let various window types register themselves

	if (!AP_Win32Frame::RegisterClass(this))
	{
		UT_DEBUGMSG(("couldn't register class\n"));
		return false;
	}

	// TODO  - load in strings

	return true;
}

XAP_Frame * AP_Win32App::newFrame(void)
{
	AP_Win32Frame * pWin32Frame = new AP_Win32Frame(this);

	if (pWin32Frame)
		pWin32Frame->initialize();

	return pWin32Frame;
}

bool AP_Win32App::shutdown(void)
{
	if (m_prefs->getAutoSavePrefs())
		m_prefs->savePrefsFile();

	return true;
}

const XAP_StringSet * AP_Win32App::getStringSet(void) const
{
	return m_pStringSet;
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

	XAP_Args Args = XAP_Args(argc,argv);
	
	AP_Win32App * pMyWin32App = new AP_Win32App(hInstance, &Args, szAppName);
	pMyWin32App->initialize();

	// create the first window.

	AP_Win32Frame * pFirstWin32Frame = new AP_Win32Frame(pMyWin32App);
	pFirstWin32Frame->initialize();
	hwnd = pFirstWin32Frame->getTopLevelWindow();
	
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

