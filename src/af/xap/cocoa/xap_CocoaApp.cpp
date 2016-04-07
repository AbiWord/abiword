/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001, 2003-2004, 2009 Hubert Figuiere
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
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
#include "ut_path.h"
#include "ut_string.h"
#include "ut_uuid.h"

#include "xap_Strings.h"
#include "xap_Args.h"
#include "xap_CocoaApp.h"
#include "xap_FakeClipboard.h"
#include "xap_CocoaFrame.h"
#include "xap_CocoaFrameImpl.h"
#include "xap_CocoaToolbar_Icons.h"
#include "xap_Cocoa_TB_CFactory.h"
#include "xap_Prefs.h"
#include "xap_CocoaEncodingManager.h"
#include "gr_CocoaCairoGraphics.h"
#include "ev_CocoaMenuBar.h"

/*****************************************************************/

XAP_CocoaApp::XAP_CocoaApp(const char * szAppName)
	: XAP_App(szAppName), 
	m_dialogFactory(this), 
	m_controlFactory(),
	m_pCocoaMenu(NULL),
	m_szMenuLayoutName(NULL),
	m_szMenuLabelSetName(NULL)
{
	m_pCocoaToolbarIcons = 0;

	[NSApplication sharedApplication];
    [NSBundle loadNibNamed:@"MainMenu" owner:NSApp];

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

	// create an instance of UT_UUIDGenerator or appropriate derrived class
	_setUUIDGenerator(new UT_UUIDGenerator());

	// register graphics allocator
	GR_GraphicsFactory * pGF = getGraphicsFactory();
	UT_ASSERT( pGF );

	if(pGF)
	{
		UT_DebugOnly<bool> bSuccess = pGF->registerClass(GR_CocoaCairoGraphics::graphicsAllocator,
										   GR_CocoaCairoGraphics::graphicsDescriptor,
										   GR_CocoaCairoGraphics::s_getClassId());

		// we are in deep trouble if this did not succeed
		UT_ASSERT( bSuccess );
		pGF->registerAsDefault(GR_CocoaCairoGraphics::s_getClassId(), true);
		pGF->registerAsDefault(GR_CocoaCairoGraphics::s_getClassId(), false);
	}

}

XAP_CocoaApp::~XAP_CocoaApp()
{
	DELETEP(m_pCocoaToolbarIcons);
	FREEP(m_szMenuLayoutName);
	FREEP(m_szMenuLabelSetName);
}

/*!
	Returns the GUI string encoding.
 */
const char * XAP_CocoaApp::getDefaultEncoding () const
{
	return "UTF-8";
}

bool XAP_CocoaApp::initialize(const char * szKeyBindingsKey, const char * szKeyBindingsDefaultValue)
{
	// let our base class do it's thing.
	
	XAP_App::initialize(szKeyBindingsKey, szKeyBindingsDefaultValue);

	/*******************************/

	// load the font stuff from the font directory

	if (!_loadFonts())
		return false;
	
	/*******************************/
  
	// load only one copy of the platform-specific icons.
	
	m_pCocoaToolbarIcons = new XAP_CocoaToolbar_Icons();
	
	// do any thing we need here...



	return true;
}

void XAP_CocoaApp::reallyExit()
{
	[NSApp stop:NSApp];
}

void XAP_CocoaApp::notifyFrameCountChange()
{
	XAP_App::notifyFrameCountChange();
}

UT_sint32 XAP_CocoaApp::makeDirectory(const char * szPath, const UT_sint32 mode ) const
{ 
	return mkdir(szPath, mode); 
}

XAP_App::BidiSupportType XAP_CocoaApp::theOSHasBidiSupport() const 
{
	return BIDI_SUPPORT_FULL;
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

const char * XAP_CocoaApp::getUserPrivateDirectory() const
{
	static const char * szAbiDir = "Library/Application Support/AbiSuite";
	
	static char upd_buffer[PATH_MAX];
	static char * upd_cache = 0;
	if (upd_cache) return upd_cache; // points to the static buffer
	
	const char * szHome = getenv("HOME");
	if (!szHome || !*szHome) {
		szHome = ".";
	}
	
	if (strlen(szHome)+strlen(szAbiDir)+2 >= PATH_MAX) 
		return NULL;
	
	strcpy(upd_buffer,szHome);
	strcat(upd_buffer,"/");
	strcat(upd_buffer,szAbiDir);

	upd_cache = upd_buffer;
	return upd_cache;
}

bool XAP_CocoaApp::findAbiSuiteBundleFile(std::string & path, const char * filename, const char * subdir) // checks only bundle
{
	if (!filename)
	{
		return false;
	}

	bool bFound = false;

	// get Bundle resource directory and use that.
	NSString * resDir = [[NSBundle mainBundle] resourcePath];
	if (resDir)
	{
		path  = [resDir UTF8String];
		if (subdir)
		{
			path += "/";
			path += subdir;
		}
		path += "/";
		path += filename;
xxx_UT_DEBUGMSG(("XAP_CocoaApp::findAbiSuiteBundleFile(\"%s\",\"%s\",\"%s\")\n",path.c_str(),filename,subdir));
		bFound = UT_isRegularFile(path.c_str());
	}
	return bFound;
}

bool XAP_CocoaApp::findAbiSuiteLibFile(std::string & path, const char * filename, const char * subdir)
{
	if (!filename)
	{
		return false;
	}
	if (XAP_App::findAbiSuiteLibFile(path, filename, subdir))
	{
		return true;
	}
	return findAbiSuiteBundleFile(path, filename, subdir);
}

bool XAP_CocoaApp::findAbiSuiteAppFile(std::string & path, const char * filename, const char * subdir)
{
	if (!filename)
	{
		return false;
	}
	if (XAP_App::findAbiSuiteLibFile(path, filename, subdir))
	{
		return true;
	}
	return findAbiSuiteBundleFile(path, filename, subdir);
}

bool XAP_CocoaApp::_loadFonts()
{
	return true;
}

void XAP_CocoaApp::_setAbiSuiteLibDir()
{
	XAP_App::_setAbiSuiteLibDir("/Library/Application Support/AbiSuite");
#if 0
	// TODO Change that to use Bundle path instead. Probably by using FJF code.
	char buf[PATH_MAX];
//	char buf2[PATH_MAX]; // not used?

	// see if a command line option [-lib <AbiSuiteLibraryDirectory>] was given

	int kLimit = m_pArgs->m_argc;
	int nFirstArg = 1;	// Cocoa puts the program name in argv[0], so [1] is the first argument
	int k;
	
	for (k = nFirstArg; k < kLimit; ++k)
		if ((*m_pArgs->m_argv[k] == '-') && (g_ascii_strcasecmp(m_pArgs->m_argv[k],"-lib")==0) && (k+1 < kLimit))
		{
			strcpy(buf,m_pArgs->m_argv[k+1]);
			int len = strlen(buf);
			if (buf[len-1]=='/')		// trim trailing slash
				buf[len-1] = 0;
			XAP_App::_setAbiSuiteLibDir(buf);
			return;
		}
	
	// if not, see if ABIWORD_DATADIR was set in the environment

	const char * sz = getenv("ABIWORD_DATADIR");
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
	
	// get Bundle resource directory and use that.
	NSString* resDir = [[NSBundle mainBundle] resourcePath];
	if (resDir != nil) {
		XAP_App::_setAbiSuiteLibDir([resDir UTF8String]);
		return;
	}

	// otherwise, use the hard-coded value
	// this is unlikely to happen
	UT_DEBUGMSG(("Couldn't get resource bundle directory."));
	XAP_App::_setAbiSuiteLibDir(getAbiSuiteHome());
	return;
#endif
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


XAP_Frame * XAP_CocoaApp::_getFrontFrame(void)
{
    XAP_Frame* myFrame = NULL;
    NSArray* array = [NSApp orderedWindows];

	NSEnumerator* e = [array objectEnumerator];
    while(NSWindow *win = [e nextObject])
    {
        id ctrl = [win delegate];
        if ([ctrl isKindOfClass:[XAP_CocoaFrameController class]])
        {
            myFrame = [(XAP_CocoaFrameController*)ctrl frameImpl]->getFrame();
            UT_ASSERT(myFrame);
            return myFrame;
        }
    }
    UT_DEBUGMSG(("Could not find Frame\n"));
    return myFrame;
}

const char*         XAP_CocoaApp::_findNearestFont(const char* pszFontFamily,
												   const char* /*pszFontStyle*/,
												   const char* /*pszFontVariant*/,
												   const char* /*pszFontWeight*/,
												   const char* /*pszFontStretch*/,
												   const char* /*pszFontSize*/,
												   const char * /*pszLang*/)
{
	return pszFontFamily;
}
