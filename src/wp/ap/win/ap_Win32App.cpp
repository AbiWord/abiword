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

#include "ie_types.h"

/*****************************************************************/

AP_Win32App::AP_Win32App(HINSTANCE hInstance, AP_Args * pArgs, const char * szAppName)
	: XAP_Win32App(hInstance, pArgs,szAppName)
{
	m_prefs = NULL;
	m_pStringSet = NULL;
}

AP_Win32App::~AP_Win32App(void)
{
	SpellCheckCleanup();

	DELETEP(m_prefs);
	DELETEP(m_pStringSet);
}

static UT_Bool s_createDirectoryIfNecessary(const char * szDir)
{
	struct _stat statbuf;
	
	if (_stat(szDir,&statbuf) == 0)								// if it exists
	{
		if ( (statbuf.st_mode & _S_IFDIR) == _S_IFDIR )			// and is a directory
			return UT_TRUE;

		UT_DEBUGMSG(("Pathname [%s] is not a directory.\n",szDir));
		return UT_FALSE;
	}

	if (CreateDirectory(szDir,NULL))
		return UT_TRUE;

	UT_DEBUGMSG(("Could not create Directory [%s].\n",szDir));
	return UT_FALSE;
}	

UT_Bool AP_Win32App::initialize(void)
{
	const char * szUserPrivateDirectory = getUserPrivateDirectory();
	UT_Bool bVerified = s_createDirectoryIfNecessary(szUserPrivateDirectory);
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
		return UT_FALSE;

	// let various window types register themselves

	if (!AP_Win32Frame::RegisterClass(this))
	{
		UT_DEBUGMSG(("couldn't register class\n"));
		return UT_FALSE;
	}

	//////////////////////////////////////////////////////////////////
	// initializes the spell checker.
	//////////////////////////////////////////////////////////////////
	
	{
#if 0 // TODO turn this part on when we get installation stuff ready
		const char * szISpellDirectory = NULL;
		if ((getPrefsValue(AP_PREF_KEY_WinISpellDirectory,&szISpellDirectory)) && (szISpellDirectory) && (*szISpellDirectory))
			;
		else
			szISpellDirectory = AP_PREF_DEFAULT_WinISpellDirectory;
#else
		char szISpellDirectory[512];
		_getExeDir(szISpellDirectory, 512);
#endif

		const char * szSpellCheckWordList = NULL;
		if ((getPrefsValue(AP_PREF_KEY_SpellCheckWordList,&szSpellCheckWordList)) && (szSpellCheckWordList) && (*szSpellCheckWordList))
			;
		else
			szSpellCheckWordList = AP_PREF_DEFAULT_SpellCheckWordList;
		
		char * szPathname = (char *)calloc(sizeof(char),strlen(szISpellDirectory)+strlen(szSpellCheckWordList)+2);
		UT_ASSERT(szPathname);
		
		sprintf(szPathname,"%s%s%s",
				szISpellDirectory,
				((szISpellDirectory[strlen(szISpellDirectory)-1]=='\\') ? "" : "\\"),
				szSpellCheckWordList);

		UT_DEBUGMSG(("Loading SpellCheckWordList [%s]\n",szPathname));
		SpellCheckInit(szPathname);
		free(szPathname);
		
		// we silently go on if we cannot load it....
	}
	
	//////////////////////////////////////////////////////////////////
	// load the dialog and message box strings
	//////////////////////////////////////////////////////////////////
	
	{
		// assume we will be using the builtin set (either as the main
		// set or as the fallback set).
		
		AP_BuiltinStringSet * pBuiltinStringSet = new AP_BuiltinStringSet(this,AP_PREF_DEFAULT_StringSet);
		UT_ASSERT(pBuiltinStringSet);
#if 0
#ifdef DEBUG
		// TODO Change this to be someplace more friendly.
		pBuiltinStringSet->dumpBuiltinSet(".\\EnUS.strings");
#endif
#endif
		m_pStringSet = pBuiltinStringSet;

		// see if we should load an alternate set from the disk
		
		const char * szDirectory = NULL;
		const char * szStringSet = NULL;

		if (   (getPrefsValue(AP_PREF_KEY_StringSet,&szStringSet))
			&& (szStringSet)
			&& (*szStringSet)
			&& (UT_stricmp(szStringSet,AP_PREF_DEFAULT_StringSet) != 0))
		{
			if ((getPrefsValue(AP_PREF_KEY_WinStringSetDirectory,&szDirectory)) && (szDirectory) && (*szDirectory))
				;
			else
				szDirectory = AP_PREF_DEFAULT_WinStringSetDirectory;

			char * szPathname = (char *)calloc(sizeof(char),strlen(szDirectory)+strlen(szStringSet)+100);
			UT_ASSERT(szPathname);

			sprintf(szPathname,"%s%s%s.strings",
					szDirectory,
					((szDirectory[strlen(szDirectory)-1]=='\\') ? "" : "\\"),
					szStringSet);

			AP_DiskStringSet * pDiskStringSet = new AP_DiskStringSet(this);
			UT_ASSERT(pDiskStringSet);

			if (pDiskStringSet->loadStringsFromDisk(szPathname))
			{
				pDiskStringSet->setFallbackStringSet(m_pStringSet);
				m_pStringSet = pDiskStringSet;
				UT_DEBUGMSG(("Using StringSet [%s]\n",szPathname));
			}
			else
			{
				DELETEP(pDiskStringSet);
				UT_DEBUGMSG(("Unable to load StringSet [%s] -- using builtin strings instead.\n",szPathname));
			}
				
			free(szPathname);
		}
	}

	//////////////////////////////////////////////////////////////////

	return UT_TRUE;
}

XAP_Frame * AP_Win32App::newFrame(void)
{
	AP_Win32Frame * pWin32Frame = new AP_Win32Frame(this);

	if (pWin32Frame)
		pWin32Frame->initialize();

	return pWin32Frame;
}

UT_Bool AP_Win32App::shutdown(void)
{
	if (m_prefs->getAutoSavePrefs())
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

		if (!pFirstWin32Frame->loadDocument(q, IEFT_Unknown))
		{
			// TODO: warn user that we couldn't open that file
			
			// fallback, open empty document so we don't crash
			pFirstWin32Frame->loadDocument(NULL, IEFT_Unknown);
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

