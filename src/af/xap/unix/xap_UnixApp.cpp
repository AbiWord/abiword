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

/*
 * Port to Maemo Development Platform 
 * Author: INdT - Renato Araujo <renato.filho@indt.org.br>
 */

#include <glib.h>
#include <gtk/gtkwidget.h>
#include <gtk/gtkmain.h>
#include <gdk/gdkrgb.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include <sys/stat.h>

#include "ut_debugmsg.h"
#include "xap_UnixDialogHelper.h"
#include "ut_string.h"
#include "ut_uuid.h"

#include "xap_Strings.h"
#include "xap_Args.h"
#include "xap_UnixApp.h"
#include "xap_FakeClipboard.h"
#include "gr_UnixImage.h"
#include "xap_UnixToolbar_Icons.h"
#include "xap_Unix_TB_CFactory.h"
#include "xap_Prefs.h"
#include "xap_UnixEncodingManager.h"
#include "xap_UnixFontManager.h"

#include "xap_UnixNullGraphics.h"
#include "xap_UnixPSGraphics.h"
#include "gr_UnixGraphics.h"
#include "gr_UnixPangoGraphics.h"

UnixNull_Graphics * abi_unixnullgraphics_instance = 0;

/*****************************************************************/
// #include <sys/time.h> // tmp just to measure the time that XftInit takes

XAP_UnixApp::XAP_UnixApp(XAP_Args * pArgs, const char * szAppName)
	: XAP_App(pArgs, szAppName), m_dialogFactory(this), m_controlFactory()
{
#if FC_MINOR > 2
	FcInit();
#else
	XftInit(NULL);
#endif

	m_pUnixToolbarIcons = 0;

	_setAbiSuiteLibDir();

	memset(&m_geometry, 0, sizeof(m_geometry));

	// create an instance of UT_UUIDGenerator or appropriate derrived class
	_setUUIDGenerator(new UT_UUIDGenerator());

	// register graphics allocator
	GR_GraphicsFactory * pGF = getGraphicsFactory();
	UT_ASSERT( pGF );

	if(pGF)
	{
		bool bSuccess = pGF->registerClass(GR_UnixGraphics::graphicsAllocator,
										   GR_UnixGraphics::graphicsDescriptor,
										   GR_UnixGraphics::s_getClassId());

		// we are in deep trouble if this did not succeed
		UT_ASSERT( bSuccess );
		pGF->registerAsDefault(GR_UnixGraphics::s_getClassId(), true);

		bSuccess = pGF->registerClass(PS_Graphics::graphicsAllocator,
									  PS_Graphics::graphicsDescriptor,
									  PS_Graphics::s_getClassId());

		UT_ASSERT( bSuccess );
		pGF->registerAsDefault(PS_Graphics::s_getClassId(), false);
		

		bSuccess = pGF->registerClass(UnixNull_Graphics::graphicsAllocator,
									  UnixNull_Graphics::graphicsDescriptor,
									  UnixNull_Graphics::s_getClassId());

		
		UT_ASSERT( bSuccess );
#if 1
		bSuccess = pGF->registerClass(GR_UnixPangoGraphics::graphicsAllocator,
									  GR_UnixPangoGraphics::graphicsDescriptor,
									  GR_UnixPangoGraphics::s_getClassId());

		UT_ASSERT( bSuccess );
#endif
	}

	/* We need to link UnixNull_Graphics because the AbiCommand
	 * plugin uses it.
	 */
	if (abi_unixnullgraphics_instance)
	  {
	    delete abi_unixnullgraphics_instance;
	    //abi_unixnullgraphics_instance = new UnixNull_Graphics(0,0);
		XAP_UnixNullGraphicsAllocInfo ai(NULL, NULL);
		abi_unixnullgraphics_instance = (UnixNull_Graphics*) XAP_App::getApp()->newGraphics(GRID_UNIX_NULL, ai);
	  }
	  
}

XAP_UnixApp::~XAP_UnixApp()
{
	DELETEP(m_pUnixToolbarIcons);
	
#if FC_MINOR > 2
	FcFini();
#endif
	
	delete m_fontManager;
}

bool XAP_UnixApp::initialize(const char * szKeyBindingsKey, const char * szKeyBindingsDefaultValue)
{
	if (!g_thread_supported ()) g_thread_init (NULL);

	// let our base class do it's thing.
	
	XAP_App::initialize(szKeyBindingsKey, szKeyBindingsDefaultValue);

	/*******************************/

	// load the font stuff from the font directory
	UT_DEBUGMSG(("Loading Fonts\n"));
	if (!_loadFonts())
		return false;
	UT_DEBUGMSG(("Fonts Loaded \n"));
	
	/*******************************/
	
	// load only one copy of the platform-specific icons.
	
	m_pUnixToolbarIcons = new AP_UnixToolbar_Icons();
	
	// do any thing we need here...

	return true;
}

void XAP_UnixApp::reallyExit()
{
	
	gtk_main_quit();
}

UT_sint32 XAP_UnixApp::makeDirectory(const char * szPath, const UT_sint32 mode ) const
{ 
  return mkdir(szPath, mode); 
}

XAP_DialogFactory * XAP_UnixApp::getDialogFactory()
{
	return &m_dialogFactory;
}

XAP_Toolbar_ControlFactory * XAP_UnixApp::getControlFactory()
{
	return &m_controlFactory;
}

XAP_UnixFontManager * XAP_UnixApp::getFontManager() const
{
	return m_fontManager;
}

void XAP_UnixApp::setWinGeometry(int x, int y, UT_uint32 width, UT_uint32 height,
								 UT_uint32 flags)
{
	// TODO : do some range checking?
	m_geometry.x = x;
	m_geometry.y = y;
	m_geometry.width = width;
	m_geometry.height = height;
	m_geometry.flags = flags;
}

void XAP_UnixApp::getWinGeometry(int * x, int * y, UT_uint32 * width,
								 UT_uint32 * height, UT_uint32 * flags)
{
	UT_ASSERT(x && y && width && height);
	*x = m_geometry.x;
	*y = m_geometry.y;
	*width = m_geometry.width;
	*height = m_geometry.height;
	*flags = m_geometry.flags;
}

const char * XAP_UnixApp::getUserPrivateDirectory()
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

bool XAP_UnixApp::_loadFonts()
{
	// create a font manager for our app to use
	m_fontManager = new XAP_UnixFontManager();
	XAP_UnixFontManager::pFontManager = m_fontManager; // set the static variable pFontManager, so we can access our fontmanager from a static context
	UT_ASSERT(m_fontManager);

	// let it loose
	UT_DEBUGMSG(("Scavange Fonts started \n"));
	if (!m_fontManager->scavengeFonts())
		return false;
	
	UT_DEBUGMSG(("Scavange Fonts finished \n"));
	return true;
}

void XAP_UnixApp::_setAbiSuiteLibDir()
{
	// FIXME: this code sucks hard

	char buf[PATH_MAX];

	// see if a command line option [-lib <AbiSuiteLibraryDirectory>] was given

	int kLimit = m_pArgs->m_argc;
	int nFirstArg = 1;	// Unix puts the program name in argv[0], so [1] is the first argument
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

void XAP_UnixApp::setTimeOfLastEvent(UT_uint32 eventTime)
{
	m_eventTime = eventTime;
}

