/* AbiSource Application Framework
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

#ifdef ABI_OPT_JS
#include <js.h>
#endif /* ABI_OPT_JS */

#include <stdio.h>
#include <string.h>

#include "xap_Args.h"
#include "xap_UnixApp.h"
#include "xap_UnixFrame.h"

int AP_UnixApp::main(const char * szAppName, int argc, char ** argv)
{
	/*
		These printfs are not here permanently.
		TODO remove them later
	*/

	printf("Build ID:\t%s\n", AP_App::s_szBuild_ID);
	printf("Version:\t%s\n", AP_App::s_szBuild_Version);
	printf("Build Options: \t%s\n", AP_App::s_szBuild_Options);
	printf("Compile Date:\t%s\n", AP_App::s_szBuild_CompileDate);
	printf("Compile Time:\t%s\n", AP_App::s_szBuild_CompileTime);

	// initialize our application.

	AP_Args Args = AP_Args(argc,argv);
	
	AP_UnixApp * pMyUnixApp = new AP_UnixApp(&Args, szAppName);
	pMyUnixApp->initialize();

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
		pFirstUnixFrame->loadDocument(argv[i]);
	}

	// turn over control to gtk

	gtk_main();

	// destroy the App.  It should take care of deleting all frames.

	delete pMyUnixApp;
	
	return 0;
}
