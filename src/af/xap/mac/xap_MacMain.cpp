/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 1999 John Brewer DBA Jera Design
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


#include <string.h>

#ifdef ABI_OPT_JS
#include <js.h>
#endif /* ABI_OPT_JS */

#include "xap_Args.h"
#include "xap_MacApp.h"
#include "xap_MacFrame.h"

#include "ap_MacFrame.h"			// TODO move this

/*****************************************************************/

int AP_MacApp::MacMain(const char * szAppName, int argc, char **argv)
{
	// initialize our application.

	AP_Args Args = AP_Args(argc,argv);
	
	AP_MacApp * pMyMacApp = new AP_MacApp(&Args, szAppName);
	pMyMacApp->initialize();

	// create the first window.

	AP_MacFrame * pFirstMacFrame = new AP_MacFrame(pMyMacApp);
	pFirstMacFrame->initialize();
//	hwnd = pFirstMacFrame->getTopLevelWindow();

	// turn over control to windows

	unsigned short mask = 0;
	struct EventRecord theEvent;
	unsigned long delay = 0;
	while (WaitNextEvent(mask, &theEvent, delay, NULL))
	{
		// Note: we do not call TranslateMessage() because
		// Note: the keybinding mechanism is responsible
		// Note: for deciding if/when to do this.

	}

	// destroy the App.  It should take care of deleting all frames.

	delete pMyMacApp;

	return 0;
}

