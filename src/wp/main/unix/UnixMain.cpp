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

#ifdef ABI_OPT_JS
#include <js.h>
#endif /* ABI_OPT_JS */

#include <string.h>

#include "ap_UnixApp.h"
#include "ap_UnixFrame.h"

int main(int argc, char ** argv)
{
	// initialize our application.

	AP_UnixApp * pMyUnixApp = new AP_UnixApp();
	pMyUnixApp->initialize(&argc,&argv);

	// create the first window.

	AP_UnixFrame * pFirstUnixFrame = new AP_UnixFrame(pMyUnixApp);
	pFirstUnixFrame->initialize(&argc,&argv);
	
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
#ifdef ABI_OPT_JS
			if (0 == strcmp(argv[i], "-script"))
			{
				i++;
				
				js_eval_file(pMyUnixApp->getInterp(), argv[i]);
			}
			else
#endif /* ABI_OPT_JS */
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
