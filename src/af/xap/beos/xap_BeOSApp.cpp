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
#include <stdlib.h>
#include "xap_Args.h"
#include "xap_BeOSApp.h"
#include "xap_FakeClipboard.h"
#include "xap_BeOSFrame.h"
#include "xap_BeOSToolbar_Icons.h"
#include "xap_BeOSToolbar_ControlFactory.h"
#include "ut_debugmsg.h"
#include "ut_string.h"


/*****************************************************************/

XAP_BeOSApp::XAP_BeOSApp(XAP_Args * pArgs, const char * szAppName)
	  : XAP_App(pArgs, szAppName), m_dialogFactory(this), m_controlFactory()
{
	m_pBeOSToolbarIcons = 0;
        _setAbiSuiteLibDir();                
}

XAP_BeOSApp::~XAP_BeOSApp(void)
{
	DELETEP(m_pBeOSToolbarIcons);
}

UT_Bool XAP_BeOSApp::initialize(void)
{
	// let our base class do it's thing.
	
	XAP_App::initialize();

	// load only one copy of the platform-specific icons.
	m_pBeOSToolbarIcons = new AP_BeOSToolbar_Icons();
	
	// do any thing we need here...

	_pClipboard = new AP_FakeClipboard();
	
	return UT_TRUE;
}

void XAP_BeOSApp::reallyExit(void)
{
	//Send an exit message to the application
	m_BApp.PostMessage(B_QUIT_REQUESTED);
}

XAP_DialogFactory * XAP_BeOSApp::getDialogFactory(void)
{
	return &m_dialogFactory;
}

XAP_Toolbar_ControlFactory * XAP_BeOSApp::getControlFactory(void)
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

void XAP_BeOSApp::_setAbiSuiteLibDir(void) {
	char buf[PATH_MAX];
	char buf2[PATH_MAX];

	// see if a command line option [-lib <AbiSuiteLibraryDirectory>] was given

	int kLimit = m_pArgs->m_argc;
	// BeOS puts the program name in argv[0], so [1] is the first argument
	int nFirstArg = 1;      
	int k;

	for (k=nFirstArg; k<kLimit; k++)
		if ((*m_pArgs->m_argv[k] == '-') && (UT_stricmp(m_pArgs->m_argv[k],"-lib")==0) && (k+1 < kLimit))
		{
			strcpy(buf,m_pArgs->m_argv[k+1]);
			int len = strlen(buf);
			if (buf[len-1]=='/')            // trim trailing slash
				buf[len-1] = 0;
			XAP_App::_setAbiSuiteLibDir(buf);
			return;
		}

	// if not, see if ABISUITE_HOME was set in the environment

	const char * sz = getenv("ABISUITE_HOME");
	if (sz && *sz)
	{
		strcpy(buf,sz);
		char * p = buf;
		int len = strlen(p);
		if ( (p[0]=='"') && (p[len-1]=='"') )
		{
			// trim leading and trailing DQUOTES
			p[len-1]=0;
			p++;
			len -= 2;
		}
		if (p[len-1]=='/')                              // trim trailingslash
			p[len-1] = 0;
		XAP_App::_setAbiSuiteLibDir(p);
		return;
	}
	
	//For BeOS we use /boot/apps/AbiSuite (expect it to be installed there)
	strcpy(buf, "/boot/apps/AbiSuite");

#if 0
	// TODO what to do ??  try the current directory...
	UT_DEBUGMSG(("ABISUITE_HOME not set and -lib not given.  Assuming current directory...."));
	getcwd(buf,sizeof(buf));
	int len = strlen(buf);
	if (buf[len-1]=='/')                            // trim trailing slash
		buf[len-1] = 0;
#endif

	XAP_App::_setAbiSuiteLibDir(buf);
	return;
}

/*
 ABI_BApp Specifics 
*/
ABI_BApp::ABI_BApp()
        :BApplication("application/x-ffw-abiword") {

	; // Nothing specific to be done in constructor
} 

