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

#include "xap_Strings.h"
#include "xap_Args.h"
#include "xap_UnixApp.h"
#include "xap_FakeClipboard.h"
#include "gr_UnixImage.h"
#include "xap_UnixFrame.h"
#include "xap_UnixToolbar_Icons.h"
#include "xap_Unix_TB_CFactory.h"
#include "xap_Prefs.h"
#include "xap_UnixEncodingManager.h"

#ifndef WITH_PANGO
#include "xap_UnixFontManager.h"
#endif

#include "xap_UnixNullGraphics.h"

/*****************************************************************/

XAP_UnixApp::XAP_UnixApp(XAP_Args * pArgs, const char * szAppName)
	: XAP_App(pArgs, szAppName), m_dialogFactory(this), m_controlFactory()
{
	m_pUnixToolbarIcons = 0;

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
	m_bBonoboRunning = false;
	// hack to ensure UnixNull_Graphics is linked properly
	// this condition should be false always
	if (this == 0)
	{
		UnixNull_Graphics * pUN_G = new UnixNull_Graphics(0,0);
	}
}

XAP_UnixApp::~XAP_UnixApp()
{
	DELETEP(m_pUnixToolbarIcons);

#ifndef WITH_PANGO
	DELETEP(m_fontManager);
#endif	
}

bool XAP_UnixApp::initialize()
{
	// initialize GTK first.
	
	gtk_set_locale();

#ifndef ABI_OPT_WIDGET
	gtk_init(&m_pArgs->m_argc,(char**)&m_pArgs->m_argv);
#endif

	// let our base class do it's thing.
	
	XAP_App::initialize();

	/*******************************/

#ifndef WITH_PANGO	
	// load the font stuff from the font directory

	if (!_loadFonts())
		return false;
#endif
	
	/*******************************/

	// set up new widgets so that they work well with gdkrgb functions
	//gtk_widget_push_visual(gtk_preview_get_visual());
	//gtk_widget_push_colormap(gtk_preview_get_cmap());

	gdk_rgb_init();
	gtk_widget_set_default_colormap(gdk_rgb_get_cmap());
	gtk_widget_set_default_visual(gdk_rgb_get_visual());

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

#ifndef WITH_PANGO
XAP_UnixFontManager * XAP_UnixApp::getFontManager()
{
	return m_fontManager;
}
#endif

void XAP_UnixApp::setGeometry(int x, int y, UT_uint32 width, UT_uint32 height,
							  windowGeometryFlags flags)
{
	// TODO : do some range checking?
	m_geometry.x = x;
	m_geometry.y = y;
	m_geometry.width = width;
	m_geometry.height = height;
	m_geometry.flags = flags;
}

void XAP_UnixApp::getGeometry(int * x, int * y, UT_uint32 * width,
							  UT_uint32 * height, windowGeometryFlags * flags)
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

#ifndef WITH_PANGO
bool XAP_UnixApp::_loadFonts()
{
	// create a font manager for our app to use
	UT_uint32 relativePathsSoFar = 0, relativePathCount = 0;
	UT_uint32 i = 0;
	
	m_fontManager = new XAP_UnixFontManager();
	UT_ASSERT(m_fontManager);

	// find all the fonts in the appropriate places.  the list of directories
	// is given in a preferences variable.

	char * szTemp = NULL;
	const char * szPrefFontPath = NULL;
	getPrefsValue(XAP_PREF_KEY_UnixFontPath,
		      (const XML_Char**)&szPrefFontPath);
	UT_ASSERT((szPrefFontPath) && (*szPrefFontPath));

 	for (i = 0; szPrefFontPath[i]; i++)
  	{
  		// count the number of segments in the path
  		// path looks like: "/font/dir;dir/anotherdir;/more/fonts"
  		if( ((i == 0) || (szPrefFontPath[i-1] == ';')) && (szPrefFontPath[i] != '/'))
  			relativePathCount++;
  	}
  	
  	// cache the suiteDir's name and length instead of calculating
  	// them on every loop
	char * suiteDirCache = (char *)getAbiSuiteLibDir();
  	UT_uint32 suiteLenCache = strlen(suiteDirCache);

 	// make pointer to the font path. ignore the cast to make gcc shut up
	char *szPrefFontPathPtr = (char *)szPrefFontPath;

	for (i = 0; szPrefFontPathPtr[i]; i++)
 		if ( ((i == 0) || (szPrefFontPathPtr[i-1] == ';')) && (szPrefFontPathPtr[i] != '/'))
 		{
		        // if relative path in prefs, prepend library directory.
 			szTemp = (char *)UT_calloc(suiteLenCache + strlen(szPrefFontPathPtr) + 10, sizeof(char));
 			strcpy(szTemp, szPrefFontPathPtr);
 			sprintf(szTemp + i, "%s/%s", suiteDirCache, szPrefFontPathPtr + i);
 			
 			// if relativePathsSoFar > 1 then szPrefFontPathPtr
 			// was really szTemp and was allocated by our call to
 			// UT_calloc so we must free it. also, we are sure not to
			// free the memory pointed to by getPrefsValue()
 			if (relativePathsSoFar && szPrefFontPathPtr)
 				free(szPrefFontPathPtr);
 			
 			szPrefFontPathPtr = szTemp;
 			relativePathsSoFar++;
 		}
 
 	// HUH? These don't match somehow. Abort!!
 	UT_ASSERT(relativePathsSoFar == relativePathCount);
	//UT_DEBUGMSG(("Using FontPath from preferences [%s].\n",szPrefFontPathPtr));
	m_fontManager->setFontPath(szPrefFontPathPtr);
	FREEP(szTemp);
	
	// let it loose

	if (!m_fontManager->scavengeFonts())
		return false;
	
#if 0
#ifdef DEBUG
	XAP_UnixFont ** fonts = m_fontManager->getAllFonts();
	UT_DEBUGMSG(("Found Fonts:\n"));
	for (i = 0; i < m_fontManager->getCount(); i++)
	{
		UT_DEBUGMSG(("\tName [%s] at [%s], metrics [%s]\n",
					 fonts[i]->getName(), fonts[i]->getFontfile(),
					 fonts[i]->getMetricfile()));
	}

	DELETEP(fonts);
#endif
#endif

	return true;
}
#endif //#ifndef WITH_PANGO

void XAP_UnixApp::_setAbiSuiteLibDir()
{
	char buf[PATH_MAX];
//	char buf2[PATH_MAX]; // not used?

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

	// TODO what to do ??  try the current directory...
	
	UT_DEBUGMSG(("ABISUITE_HOME not set and -lib not given.  Assuming current directory....\n"));

	getcwd(buf,sizeof(buf));
	int len = strlen(buf);
	if (buf[len-1]=='/')				// trim trailing slash
		buf[len-1] = 0;
	XAP_App::_setAbiSuiteLibDir(buf);
	return;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void XAP_UnixApp::setTimeOfLastEvent(UT_uint32 eventTime)
{
	m_eventTime = eventTime;
}

