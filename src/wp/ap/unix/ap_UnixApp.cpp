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

#ifdef ABI_OPT_JS
#include <js.h>
#endif /* ABI_OPT_JS */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "ut_debugmsg.h"
#include "ut_misc.h"

#include "xap_Args.h"
#include "ap_UnixFrame.h"
#include "ap_UnixApp.h"
#include "sp_spell.h"
#include "ap_Strings.h"
#include "xap_EditMethods.h"
#include "ap_LoadBindings.h"
#include "xap_Menu_ActionSet.h"
#include "xap_Toolbar_ActionSet.h"

#include "ie_imp.h"
#include "ie_types.h"

/*****************************************************************/

AP_UnixApp::AP_UnixApp(AP_Args * pArgs, const char * szAppName)
	: XAP_UnixApp(pArgs,szAppName)
{
	m_prefs = NULL;
	m_pStringSet = NULL;
}

AP_UnixApp::~AP_UnixApp(void)
{
	SpellCheckCleanup();

	DELETEP(m_prefs);
	DELETEP(m_pStringSet);
}

static UT_Bool s_createDirectoryIfNecessary(const char * szDir)
{
	struct stat statbuf;
	
	if (stat(szDir,&statbuf) == 0)								// if it exists
	{
		if (S_ISDIR(statbuf.st_mode))							// and is a directory
			return UT_TRUE;

		UT_DEBUGMSG(("Pathname [%s] is not a directory.\n",szDir));
		return UT_FALSE;
	}
	
	if (mkdir(szDir,0700) == 0)
		return UT_TRUE;
	

	UT_DEBUGMSG(("Could not create Directory [%s].\n",szDir));
	return UT_FALSE;
}	

UT_Bool AP_UnixApp::initialize(void)
{
	const char * szUserPrivateDirectory = getUserPrivateDirectory();
	UT_Bool bVerified = s_createDirectoryIfNecessary(szUserPrivateDirectory);
	UT_ASSERT(bVerified);
	
	// load preferences, first the builtin set and then any on disk.
	
	m_prefs = new AP_UnixPrefs(this);
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

	if (! XAP_UnixApp::initialize())
		return UT_FALSE;
	
	//////////////////////////////////////////////////////////////////
	// initializes the spell checker.
	//////////////////////////////////////////////////////////////////
	
	{
		const char * szISpellDirectory = NULL;
		const char * szSpellCheckWordList = NULL;
		if ((getPrefsValue(AP_PREF_KEY_UnixISpellDirectory,&szISpellDirectory)) && (szISpellDirectory) && (*szISpellDirectory))
			;
		else
			szISpellDirectory = AP_PREF_DEFAULT_UnixISpellDirectory;
		if ((getPrefsValue(AP_PREF_KEY_SpellCheckWordList,&szSpellCheckWordList)) && (szSpellCheckWordList) && (*szSpellCheckWordList))
			;
		else
			szSpellCheckWordList = AP_PREF_DEFAULT_SpellCheckWordList;
		
		char * szPathname = (char *)calloc(sizeof(char),strlen(szISpellDirectory)+strlen(szSpellCheckWordList)+2);
		UT_ASSERT(szPathname);
		
		sprintf(szPathname,"%s%s%s",
				szISpellDirectory,
				((szISpellDirectory[strlen(szISpellDirectory)-1]=='/') ? "" : "/"),
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
		m_pStringSet = pBuiltinStringSet;

		// see if we should load an alternate set from the disk
		
		const char * szDirectory = NULL;
		const char * szStringSet = NULL;

		if (   (getPrefsValue(AP_PREF_KEY_StringSet,&szStringSet))
			&& (szStringSet)
			&& (*szStringSet)
			&& (UT_stricmp(szStringSet,AP_PREF_DEFAULT_StringSet) != 0))
		{
			if ((getPrefsValue(AP_PREF_KEY_UnixStringSetDirectory,&szDirectory)) && (szDirectory) && (*szDirectory))
				;
			else
				szDirectory = AP_PREF_DEFAULT_UnixStringSetDirectory;

			char * szPathname = (char *)calloc(sizeof(char),strlen(szDirectory)+strlen(szStringSet)+100);
			UT_ASSERT(szPathname);

			sprintf(szPathname,"%s%s%s.strings",
					szDirectory,
					((szDirectory[strlen(szDirectory)-1]=='/') ? "" : "/"),
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

XAP_Frame * AP_UnixApp::newFrame(void)
{
	AP_UnixFrame * pUnixFrame = new AP_UnixFrame(this);

	if (pUnixFrame)
		pUnixFrame->initialize();

	return pUnixFrame;
}

UT_Bool AP_UnixApp::shutdown(void)
{
	if (m_prefs->getAutoSavePrefs())
		m_prefs->savePrefsFile();

	return UT_TRUE;
}

XAP_Prefs * AP_UnixApp::getPrefs(void) const
{
	return m_prefs;
}

UT_Bool AP_UnixApp::getPrefsValue(const XML_Char * szKey, const XML_Char ** pszValue) const
{
	if (!m_prefs)
		return UT_FALSE;

	return m_prefs->getPrefsValue(szKey,pszValue);
}

const XAP_StringSet * AP_UnixApp::getStringSet(void) const
{
	return m_pStringSet;
}

/*****************************************************************/

int AP_UnixApp::main(const char * szAppName, int argc, char ** argv)
{
	// This is a static function.
		   
	UT_DEBUGMSG(("Build ID:\t%s\n", XAP_App::s_szBuild_ID));
	UT_DEBUGMSG(("Version:\t%s\n", XAP_App::s_szBuild_Version));
	UT_DEBUGMSG(("Build Options: \t%s\n", XAP_App::s_szBuild_Options));
	UT_DEBUGMSG(("Build Target: \t%s\n", XAP_App::s_szBuild_Target));
	UT_DEBUGMSG(("Compile Date:\t%s\n", XAP_App::s_szBuild_CompileDate));
	UT_DEBUGMSG(("Compile Time:\t%s\n", XAP_App::s_szBuild_CompileTime));
	
	// initialize our application.

	AP_Args Args = AP_Args(argc,argv);

	AP_UnixApp * pMyUnixApp = new AP_UnixApp(&Args, szAppName);

	// if the initialize fails, we don't have icons, fonts, etc.
	if (!pMyUnixApp->initialize())
	{
		delete pMyUnixApp;
		return -1;	// make this something standard?
	}

	pMyUnixApp->ParseCommandLine();

	// turn over control to gtk
	
	gtk_main();
	
	// destroy the App.  It should take care of deleting all frames.

	pMyUnixApp->shutdown();
	delete pMyUnixApp;
	
	return 0;
}

void AP_UnixApp::ParseCommandLine(void)
{
	// parse the command line
	// <app> [-script <scriptname>]* [-dumpstrings] [<documentname>]*
	
	// TODO when we refactor the App classes, consider moving
	// TODO this to app-specific, cross-platform.

	// TODO replace this with getopt or something similar.
	
	// Unix puts the program name in argv[0], so [1] is the first argument.

	int nFirstArg = 1;
	int k;
	int kWindowsOpened = 0;
	
	for (k=nFirstArg; (k<m_pArgs->m_argc); k++)
	{
		if (*m_pArgs->m_argv[k] == '-')
		{
			if (UT_stricmp(m_pArgs->m_argv[k],"-script") == 0)
			{
				// [-script scriptname]
#ifdef ABI_OPT_JS			
				js_eval_file(getInterp(),m_pArgs->m_argv[k+1]);
#endif /* ABI_OPT_JS */
				k++;
			}
			else if (UT_stricmp(m_pArgs->m_argv[k],"-dumpstrings") == 0)
			{
				// [-dumpstrings]
#ifdef DEBUG
				AP_BuiltinStringSet * pBuiltinStringSet = new AP_BuiltinStringSet(this,AP_PREF_DEFAULT_StringSet);
				pBuiltinStringSet->dumpBuiltinSet("EnUS.strings");
				delete pBuiltinStringSet;
#endif
			}
			else
			{
				UT_DEBUGMSG(("Unknown command line option [%s]\n",m_pArgs->m_argv[k]));
				// TODO don't know if it has a following argument or not -- assume not
			}
		}
		else
		{
			// [filename]
			
			AP_UnixFrame * pFirstUnixFrame = new AP_UnixFrame(this);
			pFirstUnixFrame->initialize();
			if (pFirstUnixFrame->loadDocument(m_pArgs->m_argv[k], IEFT_Unknown))
			{
				kWindowsOpened++;
			}
			else
			{
				// TODO: warn user that we couldn't open that file

#if 1
				// TODO we crash if we just delete this without putting something
				// TODO in it, so let's go ahead and open an untitled document
				// TODO for now.  this would cause us to get 2 untitled documents
				// TODO if the user gave us 2 bogus pathnames....
				kWindowsOpened++;
				pFirstUnixFrame->loadDocument(NULL, IEFT_Unknown);
#else
				delete pFirstUnixFrame;
#endif
			}
		}
	}

	if (kWindowsOpened == 0)
	{
		// no documents specified or were able to be opened, open an untitled one

		AP_UnixFrame * pFirstUnixFrame = new AP_UnixFrame(this);
		pFirstUnixFrame->initialize();
		pFirstUnixFrame->loadDocument(NULL, IEFT_Unknown);
	}

	return;
}
