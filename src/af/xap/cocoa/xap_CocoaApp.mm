/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001 Hubert Figuiere
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

#import <Cocoa/Cocoa.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include <sys/stat.h>

#include <glib.h>

#include "ut_debugmsg.h"
#include "ut_string.h"

#include "xap_Strings.h"
#include "xap_Args.h"
#include "xap_CocoaApp.h"
#include "xap_FakeClipboard.h"
#include "gr_CocoaImage.h"
#include "xap_CocoaFrame.h"
#include "xap_CocoaToolbar_Icons.h"
#include "xap_Cocoa_TB_CFactory.h"
#include "xap_Prefs.h"
#include "xap_CocoaEncodingManager.h"

/*****************************************************************/

XAP_CocoaApp::XAP_CocoaApp(XAP_Args * pArgs, const char * szAppName)
	: XAP_App(pArgs, szAppName), 
	m_dialogFactory(this), 
	m_controlFactory()
{
	m_pCocoaToolbarIcons = 0;

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

XAP_CocoaApp::~XAP_CocoaApp()
{
	DELETEP(m_pCocoaToolbarIcons);
}

/*!
	Returns the GUI string encoding.
 */
const char * XAP_CocoaApp::getDefaultEncoding () const
{
	return "UTF-8";
}

bool XAP_CocoaApp::initialize()
{
	if (!g_thread_supported ()) {
		g_thread_init(NULL);
	}

	// let our base class do it's thing.
	
	XAP_App::initialize();

	/*******************************/

	// load the font stuff from the font directory

	if (!_loadFonts())
		return false;
	
	/*******************************/
  
	// load only one copy of the platform-specific icons.
	
	m_pCocoaToolbarIcons = new AP_CocoaToolbar_Icons();
	
	// do any thing we need here...

	return true;
}

void XAP_CocoaApp::reallyExit()
{
	NSApplication * app = [NSApplication sharedApplication];
	[app stop:app];
}

UT_sint32 XAP_CocoaApp::makeDirectory(const char * szPath, const UT_sint32 mode ) const
{ 
	return mkdir(szPath, mode); 
}

XAP_DialogFactory * XAP_CocoaApp::getDialogFactory()
{
	return &m_dialogFactory;
}

XAP_Toolbar_ControlFactory * XAP_CocoaApp::getControlFactory()
{
	return &m_controlFactory;
}

void XAP_CocoaApp::setGeometry(int x, int y, UT_uint32 width, UT_uint32 height,
							  windowGeometryFlags flags)
{
	// TODO : do some range checking?
	m_geometry.x = x;
	m_geometry.y = y;
	m_geometry.width = width;
	m_geometry.height = height;
	m_geometry.flags = flags;
}

void XAP_CocoaApp::getGeometry(int * x, int * y, UT_uint32 * width,
							  UT_uint32 * height, windowGeometryFlags * flags)
{
	UT_ASSERT(x && y && width && height);
	*x = m_geometry.x;
	*y = m_geometry.y;
	*width = m_geometry.width;
	*height = m_geometry.height;
	*flags = m_geometry.flags;
}

const char * XAP_CocoaApp::getUserPrivateDirectory()
{
	static const char * szAbiDir = "Library/Application Support/AbiSuite";
	
#ifndef PATH_MAX
#define PATH_MAX 1024
#endif
	static char upd_buffer[PATH_MAX];

	static char * upd_cache = 0;
	if (upd_cache) return upd_cache; // points to the static buffer
	
	char * szHome = getenv("HOME");
	if (!szHome || !*szHome) szHome = ".";
	
	if (strlen(szHome)+strlen(szAbiDir)+2 >= PATH_MAX) return NULL;
	
	strcpy(upd_buffer,szHome);
	strcat(upd_buffer,"/");
	strcat(upd_buffer,szAbiDir);

	upd_cache = upd_buffer;
	return upd_cache;
}

bool XAP_CocoaApp::_loadFonts()
{
	// create a font manager for our app to use
//	UT_uint32 relativePathsSoFar = 0, relativePathCount = 0;
//	UT_uint32 i = 0;
	
//	m_fontManager = new XAP_CocoaFontManager();
//	UT_ASSERT(m_fontManager);

	// let it loose

//TODO	if (!m_fontManager->scavengeFonts())
//		return false;

	return true;
}

void XAP_CocoaApp::_setAbiSuiteLibDir()
{
	// TODO Change that to use Bundle path instead. Probably by using FJF code.
	char buf[PATH_MAX];
//	char buf2[PATH_MAX]; // not used?

	// see if a command line option [-lib <AbiSuiteLibraryDirectory>] was given

	int kLimit = m_pArgs->m_argc;
	int nFirstArg = 1;	// Cocoa puts the program name in argv[0], so [1] is the first argument
	int k;
	
	for (k = nFirstArg; k < kLimit; ++k)
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

	// otherwise, use the hard-coded value
	XAP_App::_setAbiSuiteLibDir(getAbiSuiteHome());
	return;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

// causes memory corruption!
void XAP_CocoaApp::setTimeOfLastEvent(NSTimeInterval timestamp)
{
	// assert until memory corruption fixed
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	m_eventTime = timestamp;
}

