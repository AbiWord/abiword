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
/*
 Splash Window Related Stuff
*/
#include "ut_Rehydrate.h"
#include <TranslationUtils.h>
#include <DataIO.h>

#define SPLASH_UP_TIME	5				// seconds

extern unsigned char g_pngSplash[];             // see ap_wp_Splash.cpp
extern unsigned long g_pngSplash_sizeof;        // see ap_wp_Splash.cpp
       
class SplashWin:public BWindow {
        public:
                SplashWin(BMessage *data);
                virtual void DispatchMessage(BMessage *msg, BHandler *handler);

        private:
		int ignore;
};

SplashWin::SplashWin(BMessage *data)
          :BWindow(data) {
	BView *view = FindView("splashView");
	if (view) {
		BMemoryIO memio(g_pngSplash, g_pngSplash_sizeof);
		BBitmap *bitmap = BTranslationUtils::GetBitmap(&memio);
		if (bitmap)
			view->SetViewBitmap(bitmap, B_FOLLOW_ALL, 0);
		else
			UT_DEBUGMSG(("Could not interpret splash image...\n"));

		view->Sync();
        }                                     
	Show();
	ignore = SPLASH_UP_TIME;			// keep on screen n seconds
	SetPulseRate(1000000);				// a 1 second pulse
}

void SplashWin::DispatchMessage(BMessage *msg, BHandler *handler) {
	switch (msg->what) {
	case B_PULSE:
		if (ignore-- > 0)
			break;
	case B_KEY_DOWN:
	case B_MOUSE_DOWN:
		BWindow::DispatchMessage(msg, handler);
		this->Close();
		break;
	default:
		BWindow::DispatchMessage(msg, handler);
	}
} 

void _showSplash(XAP_Args * pArgs, const char * /*szAppName*/) {
	// Unix does put the program name in argv[0], 
	// unlike Win32, so [1] is the first argument
        int nFirstArg = 1;
        int k;

        // scan args for splash-related stuff
        for (k=nFirstArg; (k<pArgs->m_argc); k++) {
                if (*pArgs->m_argv[k] == '-') {
                        if (UT_stricmp(pArgs->m_argv[k],"-nosplash") == 0) {
				return;
                        }
                }                                   
	}
	BMessage *msg = new BMessage();
        if (RehydrateWindow("SplashWindow", msg)) {
		//Automatically shows and hides itself
                SplashWin *nwin = new SplashWin(msg);
        }                                        	
}                                             
/*****************************************************************/

AP_BeOSApp::AP_BeOSApp(XAP_Args * pArgs, const char * szAppName)
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
	
	//////////////////////////////////////////////////////////////////
	// initializes the spell checker.
	//////////////////////////////////////////////////////////////////
	
#if 1
	{
		const char * szISpellDirectory = NULL;
		getPrefsValueDirectory(AP_PREF_KEY_SpellDirectory,&szISpellDirectory);
		UT_ASSERT((szISpellDirectory) && (*szISpellDirectory));

		const char * szSpellCheckWordList = NULL;
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
		m_pStringSet = pBuiltinStringSet;

		// see if we should load an alternate set from the disk
		
		const char * szDirectory = NULL;
		const char * szStringSet = NULL;

		if (   (getPrefsValue(AP_PREF_KEY_StringSet,&szStringSet))
			&& (szStringSet)
			&& (*szStringSet)
			&& (UT_stricmp(szStringSet,AP_PREF_DEFAULT_StringSet) != 0))
		{
			getPrefsValueDirectory(AP_PREF_KEY_StringSetDirectory,&szDirectory);
			UT_ASSERT((szDirectory) && (*szDirectory));

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

XAP_Frame * AP_BeOSApp::newFrame(const char *path)
{
	AP_BeOSFrame * pBeOSFrame = new AP_BeOSFrame(this);

	if (pBeOSFrame)
		pBeOSFrame->initialize();

	pBeOSFrame->loadDocument(path, IEFT_Unknown);
	return pBeOSFrame;
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

UT_Bool AP_BeOSApp::getPrefsValueDirectory(const XML_Char * szKey, const XML_Char ** pszValue) const
{
	if (!m_prefs)
		return UT_FALSE;

	const XML_Char * psz = NULL;
	if (!m_prefs->getPrefsValue(szKey,&psz))
		return UT_FALSE;

	if (*psz == '/')
	{
		*pszValue = psz;
		return UT_TRUE;
	}

	static XML_Char buf[1024];
	const char *ld = getAbiSuiteLibDir();
	UT_ASSERT((((ld) ? strlen(ld) : 0) + strlen(psz) + 2) < sizeof(buf));
	
	sprintf(buf,"%s/%s",(ld) ? ld : "NULL" ,psz);
	*pszValue = buf;
	return UT_TRUE;
}

const XAP_StringSet * AP_BeOSApp::getStringSet(void) const
{
	return m_pStringSet;
}

/*****************************************************************/

int AP_BeOSApp::local_main(const char * szAppName, int argc, char ** argv) {
	// This is a static function.
		   
	UT_DEBUGMSG(("Build ID:\t%s\n", XAP_App::s_szBuild_ID));
	UT_DEBUGMSG(("Version:\t%s\n", XAP_App::s_szBuild_Version));
	UT_DEBUGMSG(("Build Options: \t%s\n", XAP_App::s_szBuild_Options));
	UT_DEBUGMSG(("Build Target: \t%s\n", XAP_App::s_szBuild_Target));
	UT_DEBUGMSG(("Compile Date:\t%s\n", XAP_App::s_szBuild_CompileDate));
	UT_DEBUGMSG(("Compile Time:\t%s\n", XAP_App::s_szBuild_CompileTime));
	
	// initialize our application.
	XAP_Args Args = XAP_Args(argc,argv);

	AP_BeOSApp * pMyBeOSApp = new AP_BeOSApp(&Args, szAppName);

	//Show the splash screen perhaps
  	_showSplash(&Args, szAppName);        

	// if the initialize fails, we don't have icons, fonts, etc.
	if (!pMyBeOSApp->initialize())
	{
		delete pMyBeOSApp;
		return -1;	// make this something standard?
	}

	pMyBeOSApp->ParseCommandLine();

	// Turn control over to the runtime (don't return until done)
	pMyBeOSApp->m_BApp.Run();
	
	// destroy the App.  It should take care of deleting all frames.
	pMyBeOSApp->shutdown();
	sleep(1);

	delete pMyBeOSApp;
	return 0;
}

void AP_BeOSApp::ParseCommandLine(void)
{
	// parse the command line
	// <app> [-script <scriptname>]* [-dumpstrings] [-lib <AbiSuiteLibDirectory>] [<documentname>]*
	
	// TODO when we refactor the App classes, consider moving
	// TODO this to app-specific, cross-platform.

	// TODO replace this with getopt or something similar.
	
	// BeOS puts the program name in argv[0], so [1] is the first argument.

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
				k++;
			}
			else if (UT_stricmp(m_pArgs->m_argv[k],"-lib") == 0)
			{
				// [-lib <AbiSuiteLibDirectory>]
				// we've already processed this when we initialized the App class
				k++;
			}
			else if (UT_stricmp(m_pArgs->m_argv[k],"-dumpstrings") == 0)
			{
				// [-dumpstrings]
#ifdef DEBUG
				// dump the string table in english as a template for translators.
				// see abi/docs/AbiSource_Localization.abw for details.
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
			
			AP_BeOSFrame * pFirstBeOSFrame = new AP_BeOSFrame(this);
			pFirstBeOSFrame->initialize();
			if (pFirstBeOSFrame->loadDocument(m_pArgs->m_argv[k], IEFT_Unknown))
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
				pFirstBeOSFrame->loadDocument(NULL, IEFT_Unknown);
#else
				delete pFirstBeOSFrame;
#endif
			}
		}
	}

	if (kWindowsOpened == 0)
	{
		// no documents specified or were able to be opened, open an untitled one

		AP_BeOSFrame * pFirstBeOSFrame = new AP_BeOSFrame(this);
		pFirstBeOSFrame->initialize();
		pFirstBeOSFrame->loadDocument(NULL, IEFT_Unknown);
	}

	return;
}

