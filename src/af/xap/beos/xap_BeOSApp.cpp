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

#include <stdio.h>
#include <string.h>
#include "xap_Args.h"
#include "xap_BeOSApp.h"
#include "xap_FakeClipboard.h"
#include "xap_BeOSFrame.h"
#include "xap_BeOSToolbar_Icons.h"
#include "xap_BeOSToolbar_ControlFactory.h"

#include "ap_BeOSFrame.h"				// TODO move this

/*****************************************************************/

XAP_BeOSApp::XAP_BeOSApp(AP_Args * pArgs, const char * szAppName)
	  : XAP_App(pArgs, szAppName), m_dialogFactory(this), m_controlFactory()
{
	printf("BEAPP: Starting Application! \n");
	m_pBeOSToolbarIcons = 0;
}

XAP_BeOSApp::~XAP_BeOSApp(void)
{
	DELETEP(m_pBeOSToolbarIcons);
}

UT_Bool XAP_BeOSApp::initialize(void)
{
	printf("BEAPP: Initialize! \n");
	// let our base class do it's thing.
	
	XAP_App::initialize();

	// load only one copy of the platform-specific icons.
	m_pBeOSToolbarIcons = new AP_BeOSToolbar_Icons();
	
	// do any thing we need here...

	_pClipboard = new AP_FakeClipboard();
	
	return UT_TRUE;
}

/*
XAP_Frame * XAP_BeOSApp::newFrame(void)
{
	printf("BEAPP: New Frame! \n");

	//Create a new on screen frame and initialize it
	AP_BeOSFrame * pBeOSFrame = new AP_BeOSFrame(this);

	if (pBeOSFrame)
		pBeOSFrame->initialize();

	return pBeOSFrame;
}
*/

void XAP_BeOSApp::reallyExit(void)
{
	//Send an exit message to the application
	m_BApp.PostMessage(B_QUIT_REQUESTED);
}

AP_DialogFactory * XAP_BeOSApp::getDialogFactory(void)
{
	return &m_dialogFactory;
}

AP_Toolbar_ControlFactory * XAP_BeOSApp::getControlFactory(void)
{
	return &m_controlFactory;
}

const char * XAP_BeOSApp::getUserPrivateDirectory(void) {
        /* return a pointer to a static buffer */

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

        char * szAbiDir = ".AbiSuite";

        static char buf[PATH_MAX];
        memset(buf,0,sizeof(buf));

        char * szHome = getenv("HOME");
        if (!szHome || !*szHome)
                szHome = "./";

        if (strlen(szHome)+strlen(szAbiDir)+2 >= PATH_MAX)
                return NULL;

        strcpy(buf,szHome);
        if (buf[strlen(buf)-1] != '/')
                strcat(buf,"/");
        strcat(buf,szAbiDir);
        return buf;                                     
}

/*
 ABI_BApp Specifics 
*/
ABI_BApp::ABI_BApp()
        :BApplication("application/x-ffw-abiword") {

	; // Nothing specific to be done in constructor
} 

