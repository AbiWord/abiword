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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include "ut_debugmsg.h"
#include "xap_Args.h"
#include "ap_BeOSFrame.h"
#include "ap_BeOSApp.h"
#include "sp_spell.h"
#include "ap_Strings.h"

#include "xap_EditMethods.h"
#include "ap_LoadBindings.h"
#include "xap_Menu_ActionSet.h"
#include "xap_Toolbar_ActionSet.h"       

/*****************************************************************/

AP_BeOSApp::AP_BeOSApp(AP_Args * pArgs, const char * szAppName)
	: XAP_BeOSApp(pArgs,szAppName)
{
	m_prefs = NULL;
	m_pStringSet = NULL;
}

AP_BeOSApp::~AP_BeOSApp(void)
{
//	SpellCheckCleanup();

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

UT_Bool AP_BeOSApp::initialize(void)
{
	const char * szUserPrivateDirectory = getUserPrivateDirectory();
	UT_Bool bVerified = s_createDirectoryIfNecessary(szUserPrivateDirectory);
	UT_ASSERT(bVerified);
	
	// load preferences, first the builtin set and then any on disk.
	
	m_prefs = new AP_BeOSPrefs(this);
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
                                        
		   
	if (! XAP_BeOSApp::initialize())
		return UT_FALSE;
	
#if 0
	//////////////////////////////////////////////////////////////////
	// initializes the spell checker.
	//////////////////////////////////////////////////////////////////
	
	{
		const char * szISpellDirectory = NULL;
		const char * szSpellCheckWordList = NULL;
		if ((getPrefsValue(AP_PREF_KEY_BeOSISpellDirectory,&szISpellDirectory)) && (szISpellDirectory) && (*szISpellDirectory))
			;
		else
			szISpellDirectory = AP_PREF_DEFAULT_BeOSISpellDirectory;
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
#endif
	
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
		pBuiltinStringSet->dumpBuiltinSet("/tmp/EnUS.strings");
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
			if ((getPrefsValue(AP_PREF_KEY_BeOSStringSetDirectory, &szDirectory)) 
			     && (szDirectory) && (*szDirectory))
				;
			else
				szDirectory = AP_PREF_DEFAULT_BeOSStringSetDirectory;

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

XAP_Frame * AP_BeOSApp::newFrame(void)
{
	AP_BeOSFrame * pBeOSFrame = new AP_BeOSFrame(this);

	if (pBeOSFrame)
		pBeOSFrame->initialize();

	return pBeOSFrame;
}

UT_Bool AP_BeOSApp::shutdown(void)
{
	if (m_prefs->getAutoSavePrefs())
		m_prefs->savePrefsFile();

	return UT_TRUE;
}

XAP_Prefs * AP_BeOSApp::getPrefs(void) const
{
	return m_prefs;
}

UT_Bool AP_BeOSApp::getPrefsValue(const XML_Char * szKey, const XML_Char ** pszValue) const
{
	if (!m_prefs)
		return UT_FALSE;

	return m_prefs->getPrefsValue(szKey,pszValue);
}

const XAP_StringSet * AP_BeOSApp::getStringSet(void) const
{
	return m_pStringSet;
}

/*****************************************************************/

int AP_BeOSApp::local_main(const char * szAppName, int argc, char ** argv) {
	// This is a static function.
		   
	// TODO These printfs are not here permanently.  remove them later.

	printf("Build ID:\t%s\n", XAP_App::s_szBuild_ID);
	printf("Version:\t%s\n", XAP_App::s_szBuild_Version);
	printf("Build Options: \t%s\n", XAP_App::s_szBuild_Options);
	printf("Build Target: \t%s\n", XAP_App::s_szBuild_Target);
	printf("Compile Date:\t%s\n", XAP_App::s_szBuild_CompileDate);
	printf("Compile Time:\t%s\n", XAP_App::s_szBuild_CompileTime);

	printf("\n");
	
	// initialize our application.

	AP_Args Args = AP_Args(argc,argv);

	AP_BeOSApp * pMyBeOSApp = new AP_BeOSApp(&Args, szAppName);

	// if the initialize fails, we don't have icons, fonts, etc.
	if (!pMyBeOSApp->initialize())
	{
		delete pMyBeOSApp;
		return -1;	// make this something standard?
	}

	// create the first window.
	AP_BeOSFrame * pFirstBeOSFrame = new AP_BeOSFrame(pMyBeOSApp);
	pFirstBeOSFrame->initialize();
	
	/*
		TODO command-line parsers are a-dime-a-dozen.
		We should find one and put it in a util directory
		somewhere so we can use it.  For now, we
		cruise through and find filenames, and look for
		-script arguments.
	*/

	{
		int i;

		// try to load the document named on the command line,
		// if that fails, create an new, untitled document window.
		
		if (!pFirstBeOSFrame->loadDocument(argv[i]))
		{
			// TODO: warn user that we couldn't open that file
			
			pFirstBeOSFrame->loadDocument(NULL);
		}
	}

        //Turn control over to the runtime (don't return until done)
        printf("Enter into infinite loop here \n");
        pMyBeOSApp->m_BApp.Run();

	// destroy the App.  It should take care of deleting all frames.
	pMyBeOSApp->shutdown();
	delete pMyBeOSApp;
	
	return 0;
}
