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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ut_debugmsg.h"

#include "ut_string.h"
#include "xap_Args.h"
#include "xap_QNXApp.h"
#include "xap_FakeClipboard.h"
#include "gr_QNXImage.h"
#include "xap_QNXFrame.h"
#include "xap_QNXToolbar_Icons.h"
#include "xap_QNX_TB_CFactory.h"
#include "xap_Prefs.h"

/*****************************************************************/

XAP_QNXApp::XAP_QNXApp(XAP_Args * pArgs, const char * szAppName)
	: XAP_App(pArgs, szAppName), m_dialogFactory(this), m_controlFactory()
{
	m_pQNXToolbarIcons = 0;

	_setAbiSuiteLibDir();

	// set some generic window sizes and positions
	m_geometry.x = 1;
	m_geometry.y = 1;
	m_geometry.width = 600;
	m_geometry.height = 600;

	// by default, which will be applied if the user does NOT
	// specify a --geometry argument, we only want to obey the
	// size (which is set above), not a position.
	m_geometry.flags = GEOMETRY_FLAG_SIZE;
}

XAP_QNXApp::~XAP_QNXApp(void)
{
	DELETEP(m_pQNXToolbarIcons);
}

UT_Bool XAP_QNXApp::initialize(void)
{
	XAP_App::initialize();

	/*******************************/
  
	// load only one copy of the platform-specific icons.
	
	m_pQNXToolbarIcons = new AP_QNXToolbar_Icons();
	
	// do any thing we need here...

	return UT_TRUE;
}

void XAP_QNXApp::reallyExit(void)
{
	//There must be a nicer way to drop out of the event loop
	exit(0);
}

XAP_DialogFactory * XAP_QNXApp::getDialogFactory(void)
{
	return &m_dialogFactory;
}

XAP_Toolbar_ControlFactory * XAP_QNXApp::getControlFactory(void)
{
	return &m_controlFactory;
}

void * XAP_QNXApp::getFontManager(void)
{
	return NULL;
}

void XAP_QNXApp::setGeometry(int x, int y, 
							 unsigned int width, unsigned int height, 
							 windowGeometryFlags flags)
{
	// TODO : do some range checking?
	m_geometry.x = x;
	m_geometry.y = y;
	m_geometry.width = width;
	m_geometry.height = height;
	m_geometry.flags = flags;
}

void XAP_QNXApp::getGeometry(int * x, int * y, unsigned int * width,
							  unsigned int * height, windowGeometryFlags * flags)
{
	UT_ASSERT(x && y && width && height);
	*x = m_geometry.x;
	*y = m_geometry.y;
	*width = m_geometry.width;
	*height = m_geometry.height;
	*flags = m_geometry.flags;
}

const char * XAP_QNXApp::getUserPrivateDirectory(void)
{
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

UT_Bool XAP_QNXApp::_loadFonts(void)
{
	//Not needed in QNX
	return UT_TRUE;
}

void XAP_QNXApp::_setAbiSuiteLibDir(void)
{
	char buf[PATH_MAX];
//	char buf2[PATH_MAX]; // not used?

	// see if a command line option [-lib <AbiSuiteLibraryDirectory>] was given

	int kLimit = m_pArgs->m_argc;
	int nFirstArg = 1;	// QNX puts the program name in argv[0], so [1] is the first argument
	int k;
	
	for (k=nFirstArg; k<kLimit; k++)
		if ((*m_pArgs->m_argv[k] == '-') && (UT_stricmp(m_pArgs->m_argv[k],"-lib")==0) && (k+1 < kLimit))
		{
			strcpy(buf,m_pArgs->m_argv[k+1]);
			int len = strlen(buf);
			if (buf[len-1]=='/')		// trim trailing slash
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
		if (p[len-1]=='/')				// trim trailing slash
			p[len-1] = 0;
		XAP_App::_setAbiSuiteLibDir(p);
		return;
	}

	// TODO what to do ??  try the current directory...
	
	UT_DEBUGMSG(("ABISUITE_HOME not set and -lib not given.  Assuming current directory...."));

	getcwd(buf,sizeof(buf));
	int len = strlen(buf);
	if (buf[len-1]=='/')				// trim trailing slash
		buf[len-1] = 0;
	XAP_App::_setAbiSuiteLibDir(buf);
	return;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
/*
void XAP_QNXApp::setTimeOfLastEvent(int eventTime)
{
	//Not needed for QNX
	//m_eventTime = eventTime;
}
*/
