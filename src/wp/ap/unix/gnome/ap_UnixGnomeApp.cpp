/* AbiWord
 * Copyright (C) 1998-2000 AbiSource, Inc.
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

// HACK to no collide with perl DEBUG
#ifdef DEBUG
#define ABI_DEBUG
#undef DEBUG
#endif

#ifdef ABI_OPT_PERL
#include <EXTERN.h>
#include <perl.h>
#endif

#ifdef DEBUG
#define PERL_DEBUG
#undef DEBUG
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_misc.h"

#include "gr_Image.h"
#include "xap_Args.h"
#include "ap_Convert.h"
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

#ifdef HAVE_GNOMEVFS
#include "gnome-vfs.h"
#endif

#include <libgnomeui/gnome-window-icon.h>

#include "xap_Prefs.h"
#include "ap_Prefs_SchemeIds.h"

/*****************************************************************/

AP_UnixGnomeApp::AP_UnixGnomeApp(XAP_Args * pArgs, const char * szAppName)
  : AP_UnixApp(pArgs,szAppName)
{
}

AP_UnixGnomeApp::~AP_UnixGnomeApp()
{
}

/*****************************************************************/

int AP_UnixGnomeApp::main(const char * szAppName, int argc, char ** argv)
{
	int k = 0;

	static const struct poptOption options[] =
	{{"geometry", 'g', POPT_ARG_STRING, NULL, 0, "set initial frame geometry", "GEOMETRY"},
	 {"nosplash", 'n', POPT_ARG_NONE,   NULL, 0, "do not show splash screen", NULL},
	 {"lib",      'l', POPT_ARG_STRING, NULL, 0, "use DIR for application components", "DIR"},
#ifdef ABI_OPT_PERL
	 {"script", 's', POPT_ARG_STRING, NULL, 0, "Execute FILE as script", "FILE"},
#endif
#ifdef DEBUG
	 {"dumpstrings", 'd', POPT_ARG_NONE, NULL, 0, "Dump strings to file", NULL},
#endif
	 {"to", 't', POPT_ARG_STRING, NULL, 0, "The target format of the file (abw, zabw, rtf, txt, utf8, html, latex)", "FORMAT"},
	 {"verbose", 'v', POPT_ARG_INT, NULL, 0, "The verbosity level (0, 1, 2)", "LEVEL"},
	 {"show", '\0', POPT_ARG_NONE, NULL, 0, "If you really want to start the GUI (even if you use the --to option)", ""},
	 {NULL, '\0', 0, NULL, 0, NULL, NULL} /* end the list */
	};
	gnomelib_register_popt_table (options, "Abiword Options");

	// This is a static function.
		   
	UT_DEBUGMSG(("Build ID:\t%s\n", XAP_App::s_szBuild_ID));
	UT_DEBUGMSG(("Version:\t%s\n", XAP_App::s_szBuild_Version));
	UT_DEBUGMSG(("Build Options: \t%s\n", XAP_App::s_szBuild_Options));
	UT_DEBUGMSG(("Build Target: \t%s\n", XAP_App::s_szBuild_Target));
	UT_DEBUGMSG(("Compile Date:\t%s\n", XAP_App::s_szBuild_CompileDate));
	UT_DEBUGMSG(("Compile Time:\t%s\n", XAP_App::s_szBuild_CompileTime));

	// initialize our application.

	XAP_Args Args = XAP_Args(argc,argv);
  
 	AP_UnixGnomeApp * pMyUnixApp = new AP_UnixGnomeApp(&Args, szAppName);
 
 	// Do a quick and dirty find for "--to"
  	bool bShowSplash = true;
 	bool bShowApp = true;
  	for (k = 1; k < Args.m_argc; k++)
  		if (*Args.m_argv[k] == '-')
  			if ((UT_stricmp(Args.m_argv[k],"--to") == 0) ||
 				(UT_stricmp(Args.m_argv[k],"-t") == 0))
  			{
 				bShowApp = false;
  				bShowSplash = false;
  				break;
  			}
 
 	// Do a quick and dirty find for "--show"
  	for (k = 1; k < Args.m_argc; k++)
  		if (*Args.m_argv[k] == '-')
  			if (UT_stricmp(Args.m_argv[k],"--show") == 0)
  			{
 				bShowApp = true;
  				bShowSplash = true;
  				break;
  			}
 
 	// Do a quick and dirty find for "--nosplash"
 	for (k = 1; k < Args.m_argc; k++)
 		if (*Args.m_argv[k] == '-')
 			if ((UT_stricmp(Args.m_argv[k],"--nosplash") == 0) ||
				(UT_stricmp(Args.m_argv[k], "-n") == 0))
 			{
 				bShowSplash = false;
 				break;
 			}

	// HACK : these calls to gtk reside properly in XAP_UnixApp::initialize(),
	// HACK : but need to be here to throw the splash screen as
	// HACK : soon as possible.
	gtk_set_locale();
	//gnome_init(pMyUnixApp->m_szAppName, "0.9.0", Args.m_argc, Args.m_argv);
	gtk_init(&Args.m_argc,&Args.m_argv);

#ifdef HAVE_GNOMEVFS
	if (! gnome_vfs_init ())
	  {
	    UT_DEBUGMSG(("DOM: gnome_vfs_init () failed!\n"));
	    return -1;	    
	  }
#endif

	// set the default icon
	UT_String s = pMyUnixApp->getAbiSuiteLibDir();
	s += "/icons/abiword_48.png";
	gnome_window_icon_init ();
	gnome_window_icon_set_default_from_file (s.c_str());

	// if the initialize fails, we don't have icons, fonts, etc.
	if (!pMyUnixApp->initialize())
	{
		delete pMyUnixApp;
		return -1;	// make this something standard?
	}

	const XAP_Prefs * pPrefs = pMyUnixApp->getPrefs();

	bool bSplashPref = true;
	if (pPrefs->getPrefsValueBool (AP_PREF_KEY_ShowSplash, &bSplashPref))
	{
	    bShowSplash = bShowSplash && bSplashPref;
	}

	if (bShowSplash)
		_showSplash(2000);

	// this function takes care of all the command line args.
	// if some args are botched, it returns false and we should
	// continue out the door.
	if (pMyUnixApp->parseCommandLine() && bShowApp)
	{
		// turn over control to gtk
		gtk_main();
	}
	
	// destroy the App.  It should take care of deleting all frames.
	pMyUnixApp->shutdown();
	delete pMyUnixApp;
	
	return 0;
}

bool AP_UnixGnomeApp::parseCommandLine()
{
	// parse the command line
	// <app> [--script <scriptname>]* [--dumpstrings] [--to <format>] [--geometry <format>] [<documentname>]*
	
	// TODO when we refactor the App classes, consider moving
	// TODO this to app-specific, cross-platform.
	
	int kWindowsOpened = 0;
#ifdef ABI_OPT_PERL
 	char *script = NULL;
#endif
	char *geometry = NULL;
	char *file = NULL;
	char *to = NULL;
	int verbose = 1;
	int show = 0;
#ifdef DEBUG
 	gboolean dumpstrings = FALSE;
#endif
	poptContext poptcon;
 	static const struct poptOption options[] =
 	{{"geometry", 'g', POPT_ARG_STRING, &geometry, 0, "HACK", "HACK"},
	 {"nosplash", 'n', POPT_ARG_NONE, NULL, 0, "HACK", "HACK"},
	 {"lib",      'l', POPT_ARG_STRING, NULL, 0, "HACK", "HACK"},
#ifdef ABI_OPT_PERL
	 {"script", 's', POPT_ARG_STRING, &script, 0,
	  "HACK", "HACK"},
#endif
#ifdef DEBUG
	 {"dumpstrings", 'd', POPT_ARG_NONE, &dumpstrings, 0,
	  "HACK", NULL},
#endif
	 {"to", 't', POPT_ARG_STRING, &to, 0, "HACK", "HACK"},
	 {"verbose", 'v', POPT_ARG_INT, &verbose, 0, "HACK", "HACK"},
	 {"show", '\0', POPT_ARG_NONE, &show, 0, "HACK", NULL},
	 {NULL, '\0', 0, NULL, 0, NULL, NULL} /* end the list */
	};
	
 	gnomelib_register_popt_table (options, "Hack");
	poptcon = gnomelib_parse_args (m_pArgs->m_argc, m_pArgs->m_argv, 0);

#ifdef DEBUG
	if (dumpstrings)
	{
		// dump the string table in english as a template for translators.
		// see abi/docs/AbiSource_Localization.abw for details.
		AP_BuiltinStringSet * pBuiltinStringSet = new AP_BuiltinStringSet(this,(XML_Char*)AP_PREF_DEFAULT_StringSet);
		pBuiltinStringSet->dumpBuiltinSet("en-US.strings");
		delete pBuiltinStringSet;
	}
#endif

#ifdef ABI_OPT_PERL
	if (script)
		perlEvalFile(script);
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
	
	if (to) {
		AP_Convert * conv = new AP_Convert(getApp());
		conv->setVerbose(verbose);

		while ((file = poptGetArg (poptcon)) != NULL) {
			conv->convertTo(file, to);
		}

		delete conv;

		if (!show)
			return true;
	}

	while ((file = poptGetArg (poptcon)) != NULL) {
		AP_UnixFrame * pFirstUnixFrame = new AP_UnixFrame(this);
		pFirstUnixFrame->initialize();

		UT_Error error = pFirstUnixFrame->loadDocument(file, IEFT_Unknown, true);
		if (!error)
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

	return true;
}


