/* AbiWord
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

/*****************************************************************
** Only one of these is created by the application.
*****************************************************************/

#ifdef ABI_OPT_JS
#include <js.h>
#endif /* ABI_OPT_JS */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "ut_debugmsg.h"
#include "ut_misc.h"

#include "xap_Args.h"
#include "ap_UnixFrame.h"
#include "ap_UnixApp.h"
#include "ap_UnixGnomeApp.h"
#include "sp_spell.h"
#include "ap_Strings.h"
#include "xap_EditMethods.h"
#include "ap_LoadBindings.h"
#include "xap_Menu_ActionSet.h"
#include "xap_Toolbar_ActionSet.h"
#include "xav_View.h"

#include "ie_imp.h"
#include "ie_types.h"
#include "ie_exp_Text.h"
#include "ie_exp_RTF.h"
#include "ie_exp_AbiWord_1.h"
#include "ie_exp_HTML.h"
#include "ie_imp_Text.h"
#include "ie_imp_RTF.h"

#include "gr_Graphics.h"
#include "gr_UnixGraphics.h"
#include "gr_Image.h"
#include "ut_bytebuf.h"
#include "ut_png.h"
#include "ut_dialogHelper.h"
#include "ut_debugmsg.h"

#include "fv_View.h"

/*****************************************************************/

AP_UnixGnomeApp::AP_UnixGnomeApp(XAP_Args * pArgs, const char * szAppName)
  : AP_UnixApp(pArgs,szAppName)
{
}

AP_UnixGnomeApp::~AP_UnixGnomeApp(void)
{
}

/*****************************************************************/

UT_Bool	AP_UnixGnomeApp::initialize(void)
{
	static const struct poptOption options[] =
	{{"geometry", 'g', POPT_ARG_STRING, NULL, 0, "set initial frame geometry", "GEOMETRY"},
	 {"nosplash", 'n', POPT_ARG_NONE,   NULL, 0, "do not show splash screen", NULL},
	 {"lib",      'l', POPT_ARG_STRING, NULL, 0, "use DIR for application components", "DIR"},
#ifdef ABI_OPT_JS
	 {"script", 's', POPT_ARG_STRING, NULL, 0, "Execute FILE as script", "FILE"},
#endif
#ifdef DEBUG
	 {"dumpstrings", 'd', POPT_ARG_NONE, NULL, 0, "Dump strings to file", NULL},
#endif
	 {NULL, '\0', 0, NULL, 0, NULL, NULL} /* end the list */
	};
	gnomelib_register_popt_table (options, "Abiword Options");
	
	return AP_UnixApp::initialize();
}

int AP_UnixGnomeApp::main(const char * szAppName, int argc, char ** argv)
{
	// This is a static function.
		   
	UT_DEBUGMSG(("Build ID:\t%s\n", XAP_App::s_szBuild_ID));
	UT_DEBUGMSG(("Version:\t%s\n", XAP_App::s_szBuild_Version));
	UT_DEBUGMSG(("Build Options: \t%s\n", XAP_App::s_szBuild_Options));
	UT_DEBUGMSG(("Build Target: \t%s\n", XAP_App::s_szBuild_Target));
	UT_DEBUGMSG(("Compile Date:\t%s\n", XAP_App::s_szBuild_CompileDate));
	UT_DEBUGMSG(("Compile Time:\t%s\n", XAP_App::s_szBuild_CompileTime));

	// initialize our application.

	XAP_Args Args = XAP_Args(argc,argv);

	// Do a quick and dirty find for "-nosplash"
 	UT_Bool bShowSplash = UT_TRUE;
 	for (int k = 1; k < Args.m_argc; k++)
 		if (*Args.m_argv[k] == '-')
 			if (UT_stricmp(Args.m_argv[k],"--nosplash") == 0)
 			{
 				bShowSplash = UT_FALSE;
 				break;
 			}

	// HACK : these calls to gtk reside properly in XAP_UnixApp::initialize(),
	// HACK : but need to be here to throw the splash screen as
	// HACK : soon as possible.
	// TODO : I've to change that to gnome_init call.
	gtk_set_locale();
	//	gnome_init(m_szAppName, "0.0", Args.m_argc, Args.m_argv);
	gtk_init(&Args.m_argc,&Args.m_argv);
	
	if (bShowSplash)
		_showSplash(2000);
			
	AP_UnixGnomeApp * pMyUnixApp = new AP_UnixGnomeApp(&Args, szAppName);

	// if the initialize fails, we don't have icons, fonts, etc.
	if (!pMyUnixApp->initialize())
	{
		delete pMyUnixApp;
		return -1;	// make this something standard?
	}

	// this function takes care of all the command line args.
	// if some args are botched, it returns false and we should
	// continue out the door.
	if (pMyUnixApp->parseCommandLine())
	{
		// turn over control to gtk
		gtk_main();
	}
	
	// destroy the App.  It should take care of deleting all frames.
	pMyUnixApp->shutdown();
	delete pMyUnixApp;
	
	return 0;
}

UT_Bool AP_UnixGnomeApp::parseCommandLine(void)
{
	// parse the command line
	// <app> [-script <scriptname>]* [-dumpstrings] [<documentname>]*
	
	// TODO when we refactor the App classes, consider moving
	// TODO this to app-specific, cross-platform.
	
	int kWindowsOpened = 0;
 	char *script = NULL, *geometry = NULL;
	char *file = NULL;
 	gboolean dumpstrings = FALSE;
	poptContext poptcon;
 	static const struct poptOption options[] =
 	{{"geometry", 'g', POPT_ARG_STRING, &geometry, 0, "HACK", "HACK"},
	 {"nosplash", 'n', POPT_ARG_NONE, NULL, 0, "HACK", "HACK"},
	 {"lib",      'l', POPT_ARG_STRING, NULL, 0, "HACK", "HACK"},
#ifdef ABI_OPT_JS
	 {"script", 's', POPT_ARG_STRING, &script, 0,
	  N_("Execute the specified script"), N_("SCRIPT")},
#endif
#ifdef DEBUG
	 {"dumpstrings", 'd', POPT_ARG_NONE, &dumpstrings, 0,
	  N_("Dump strings for translation purposes"), NULL},
#endif
	 {NULL, '\0', 0, NULL, 0, NULL, NULL} /* end the list */
	};
	
 	gnomelib_register_popt_table (options, "Hack");
	poptcon = gnomelib_parse_args (m_pArgs->m_argc, m_pArgs->m_argv, 0);

#ifdef DEBUG
	if (dumpstrings)
	{
		// dump the string table in english as a template for translators.
		// see abi/docs/AbiSource_Localization.abw for details.
		AP_BuiltinStringSet * pBuiltinStringSet = new AP_BuiltinStringSet(this,AP_PREF_DEFAULT_StringSet);
		pBuiltinStringSet->dumpBuiltinSet("EnUS.strings");
		delete pBuiltinStringSet;
	}
#endif

#ifdef ABI_OPT_JS
	if (script)
	{
		// By now, do nothing (we don't have script support right now)
	}
#endif

	if (geometry)
	{
		// [--geometry <X geometry string>]

		// TODO : does X have a dummy geometry value reserved for this?
		gint dummy = 1 << ((sizeof(gint) * 8) - 1);
		gint x = dummy;
		gint y = dummy;
		guint width = 0;
		guint height = 0;
		
		XParseGeometry(geometry, &x, &y, &width, &height);
		
		// use both by default
		XAP_UNIXBASEAPP::windowGeometryFlags f = (XAP_UNIXBASEAPP::windowGeometryFlags)
			(XAP_UNIXBASEAPP::GEOMETRY_FLAG_SIZE
			 | XAP_UNIXBASEAPP::GEOMETRY_FLAG_POS);
		
		// if pos (x and y) weren't provided just use size
		if (x == dummy || y == dummy)
			f = XAP_UNIXBASEAPP::GEOMETRY_FLAG_SIZE;
		
		// if size (width and height) weren't provided just use pos
		if (width == 0 || height == 0)
			f = XAP_UNIXBASEAPP::GEOMETRY_FLAG_POS;
		
		// set the xap-level geometry for future frame use
		setGeometry(x, y, width, height, f);
	}
	
	while ((file = poptGetArg (poptcon)) != NULL) {
		AP_UnixFrame * pFirstUnixFrame = new AP_UnixFrame(this);
		pFirstUnixFrame->initialize();
		if (E2B(pFirstUnixFrame->loadDocument(file, IEFT_Unknown)))
		{
			kWindowsOpened++;
		}
		else
		{
			// TODO: warn user that we couldn't open that file
			
#if 1
			// TODO we crash if we just delete this without putting something
			// TODO in it, so let's go ahead and open an untitled document
			// TODO for now.  this would cause us to get 2 untitled documents
			// TODO if the user gave us 2 bogus pathnames....
			kWindowsOpened++;
			pFirstUnixFrame->loadDocument(NULL, IEFT_Unknown);
#else
			delete pFirstUnixFrame;
#endif
		}
	}

	if (kWindowsOpened == 0)
	  {
		// no documents specified or were able to be opened, open an untitled one
		
		AP_UnixFrame * pFirstUnixFrame = new AP_UnixFrame(this);
		pFirstUnixFrame->initialize();
		pFirstUnixFrame->loadDocument(NULL, IEFT_Unknown);
	  }

	return UT_TRUE;
}

