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

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include <stdlib.h>
#include <stdio.h>
#include <strings.h>

#include "ut_debugmsg.h"
#include "ut_dialogHelper.h"

#include "xap_Args.h"
#include "xap_UnixApp.h"
#include "xap_FakeClipboard.h"
#include "gr_UnixImage.h"
#include "xap_UnixFrame.h"
#include "xap_UnixToolbar_Icons.h"
#include "xap_Unix_TB_CFactory.h"

#define DELETEP(p)	do { if (p) delete p; } while (0)

//////////////////////////////////////////////////////////////////
// TODO either move all the AbiWord declarations to wp/ap/unix/ap_UnixApp
// TODO or rename the variables as AbiSuite or something...
//////////////////////////////////////////////////////////////////

// The first defines the environment variable we honor,
// the second defines the default path we use (this should
// change) if that variable is not set.
#define ABIWORD_FONTPATH_VARIABLE 	"ABIWORD_FONTPATH"
#define ABIWORD_FONTPATH_DEFAULT	"./src/wp/lib/unix/fonts;./wp/lib/unix/fonts;../../lib/unix/fonts;/usr/local/AbiWord/lib/unix/fonts"

/*****************************************************************/

XAP_UnixApp::XAP_UnixApp(AP_Args * pArgs, const char * szAppName)
	: XAP_App(pArgs, szAppName), m_dialogFactory(this), m_controlFactory()
{
	m_pUnixToolbarIcons = 0;
}

XAP_UnixApp::~XAP_UnixApp(void)
{
	DELETEP(m_pUnixToolbarIcons);
	DELETEP(_pClipboard);
}

UT_Bool XAP_UnixApp::initialize(void)
{
	// initialize GTK first.
	
	gtk_set_locale();
	gtk_init(&m_pArgs->m_argc,&m_pArgs->m_argv);

	// let our base class do it's thing.
	
	XAP_App::initialize();

	// create a font manager for our app to use

	m_fontManager = new XAP_UnixFontManager();

	// find all the fonts in the appropriate places

	// TODO this should be read from the environment/prefs
	char * fontpath = getenv(ABIWORD_FONTPATH_VARIABLE);
	if (fontpath)
	{
		UT_DEBUGMSG(("Using font path of %s.\n", fontpath));
		m_fontManager->setFontPath(fontpath);
	}
	else
	{
		char message[1024];
		g_snprintf(message, 1024, "$%s not set, using default font path of\n"
				   "[%s].",
				   ABIWORD_FONTPATH_VARIABLE, ABIWORD_FONTPATH_DEFAULT);
		messageBoxOK(message);

		m_fontManager->setFontPath(ABIWORD_FONTPATH_DEFAULT);
	}

	// let it loose
	if (!m_fontManager->scavengeFonts())
		return UT_FALSE;

	
#ifdef DEBUG	
	XAP_UnixFont ** fonts = m_fontManager->getAllFonts();
	UT_DEBUGMSG(("Found Fonts:"));
	for (UT_uint32 i = 0; i < m_fontManager->getCount(); i++)
	{
		UT_DEBUGMSG(("\tName [%s] at [%s], metrics [%s]",
					 fonts[i]->getName(), fonts[i]->getFontfile(),
					 fonts[i]->getMetricfile()));
	}

	DELETEP(fonts);
#endif

	/*******************************/

	// set up new widgets so that they work well with gdkrgb functions
//    gtk_widget_push_visual(gtk_preview_get_visual());
//    gtk_widget_push_colormap(gtk_preview_get_cmap());

	gdk_rgb_init();
	gtk_widget_set_default_colormap(gdk_rgb_get_cmap());
	gtk_widget_set_default_visual(gdk_rgb_get_visual());

	/*******************************/
  
	// load only one copy of the platform-specific icons.
	
	m_pUnixToolbarIcons = new AP_UnixToolbar_Icons();
	
	// do any thing we need here...

	_pClipboard = new AP_FakeClipboard();
	
	return UT_TRUE;
}

void XAP_UnixApp::reallyExit(void)
{
	gtk_main_quit();
}

AP_DialogFactory * XAP_UnixApp::getDialogFactory(void)
{
	return &m_dialogFactory;
}

AP_Toolbar_ControlFactory * XAP_UnixApp::getControlFactory(void)
{
	return &m_controlFactory;
}

XAP_UnixFontManager * XAP_UnixApp::getFontManager(void)
{
	return m_fontManager;
}

const char * XAP_UnixApp::getUserPrivateDirectory(void)
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

