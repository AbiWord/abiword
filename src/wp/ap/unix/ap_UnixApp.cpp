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
#include <string.h>

#include "xap_Args.h"
#include "ap_UnixFrame.h"
#include "ap_UnixApp.h"

#define NrElements(a)		(sizeof(a) / sizeof(a[0]))
#define FREEP(p)	do { if (p) free(p); (p)=NULL; } while (0)
#define DELETEP(p)	do { if (p) delete(p); (p)=NULL; } while (0)

/*****************************************************************/

AP_UnixApp::AP_UnixApp(AP_Args * pArgs, const char * szAppName)
	: XAP_UnixApp(pArgs,szAppName)
{
	m_prefs = NULL;
}

AP_UnixApp::~AP_UnixApp(void)
{
	DELETEP(m_prefs);
}

UT_Bool AP_UnixApp::initialize(void)
{
	// load preferences, first the builtin set and then any on disk.
	
	m_prefs = new AP_UnixPrefs(this);
	m_prefs->loadBuiltinPrefs();
	m_prefs->loadPrefsFile();

	// TODO overlay command line arguments onto preferences...
		   
	// now that preferences are established, let the xap init
		   
	return XAP_UnixApp::initialize();
}

UT_Bool AP_UnixApp::shutdown(void)
{
	if (m_prefs->getAutoSave())
		m_prefs->savePrefsFile();

	return UT_TRUE;
}

UT_Bool AP_UnixApp::getPrefsValue(const XML_Char * szKey, const XML_Char ** pszValue) const
{
	if (!m_prefs)
		return UT_FALSE;

	return m_prefs->getPrefsValue(szKey,pszValue);
}

/*****************************************************************/

int AP_UnixApp::main(const char * szAppName, int argc, char ** argv)
{
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

	AP_UnixApp * pMyUnixApp = new AP_UnixApp(&Args, szAppName);

	// if the initialize fails, we don't have icons, fonts, etc.
	if (!pMyUnixApp->initialize())
	{
		delete pMyUnixApp;
		return -1;	// make this something standard?
	}

	// create the first window.
	AP_UnixFrame * pFirstUnixFrame = new AP_UnixFrame(pMyUnixApp);
	pFirstUnixFrame->initialize();
	
	/*
		TODO command-line parsers are a-dime-a-dozen.
		We should find one and put it in a util directory
		somewhere so we can use it.  For now, we
		cruise through and find filenames, and look for
		-script arguments.
	*/

	{
		int i;

		for (i=1; i<argc; i++)
		{
			if (0 == strcmp(argv[i], "-script"))
			{
				i++;
				
#ifdef ABI_OPT_JS
				js_eval_file(pMyUnixApp->getInterp(), argv[i]);
#endif /* ABI_OPT_JS */
			}
			else
			{
				break;
			}
		}

		// try to load the document named on the command line,
		// if that fails, create an new, untitled document window.
		
		if (!pFirstUnixFrame->loadDocument(argv[i]))
		{
			// TODO: warn user that we couldn't open that file
			
			pFirstUnixFrame->loadDocument(NULL);
		}
	}

	// turn over control to gtk

	gtk_main();

	// destroy the App.  It should take care of deleting all frames.

	pMyUnixApp->shutdown();
	delete pMyUnixApp;
	
	return 0;
}
