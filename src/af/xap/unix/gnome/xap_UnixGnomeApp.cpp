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

// I've to clean a bit all these include's...
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include "ut_debugmsg.h"
#include "ut_dialogHelper.h"
#include "ut_string.h"

#include "xap_Args.h"
#include "xap_UnixApp.h"
#include "xap_UnixGnomeApp.h"
#include "xap_FakeClipboard.h"
#include "gr_UnixImage.h"
#include "xap_UnixFrame.h"
#include "xap_UnixToolbar_Icons.h"
#include "xap_Unix_TB_CFactory.h"
#include "xap_Prefs.h"

/*****************************************************************/

XAP_UnixGnomeApp::XAP_UnixGnomeApp(XAP_Args * pArgs, const char * szAppName)
	: XAP_UnixApp(pArgs, szAppName)
{
}

XAP_UnixGnomeApp::~XAP_UnixGnomeApp(void)
{
}

bool XAP_UnixGnomeApp::initialize(void)
{
	// TODO: We need a VERSION define
	gnome_init(m_szAppName, "0.7.13", m_pArgs->m_argc, m_pArgs->m_argv);
	glade_gnome_init ();

	// let the base class of XAP_UnixApp do it's thing.
	
	XAP_App::initialize();

	/*******************************/

	// load the font stuff from the font directory

	if (!_loadFonts())
		return false;
	
	/*******************************/

	// set up new widgets so that they work well with gdkrgb functions
	//gtk_widget_push_visual(gtk_preview_get_visual());
	//gtk_widget_push_colormap(gtk_preview_get_cmap());

	// We don't need this if we use GNOME
	//	gdk_rgb_init();
	//	gtk_widget_set_default_colormap(gdk_rgb_get_cmap());
	//	gtk_widget_set_default_visual(gdk_rgb_get_visual());

	/*******************************/
  
	// load only one copy of the platform-specific icons.
	// I think that we don't need this if we use GNOME, but...
	m_pUnixToolbarIcons = new AP_UnixToolbar_Icons();
	
	// do any thing we need here...

	return true;
}


void XAP_UnixGnomeApp::_setAbiSuiteLibDir(void)
{
	char buf[PATH_MAX];
	//	char buf2[PATH_MAX]; // not used?

	// see if a command line option [-lib <AbiSuiteLibraryDirectory>] was given
	// NOTE: Gnome parses the command line with popt, so we have to change the way to parse the line

	int kLimit = m_pArgs->m_argc;
	int nFirstArg = 1;	// Unix puts the program name in argv[0], so [1] is the first argument
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
	
	UT_DEBUGMSG(("ABISUITE_HOME not set and -lib not given.  Assuming current directory....\n"));

	getcwd(buf,sizeof(buf));
	int len = strlen(buf);
	if (buf[len-1]=='/')				// trim trailing slash
		buf[len-1] = 0;
	XAP_App::_setAbiSuiteLibDir(buf);
	return;
}
