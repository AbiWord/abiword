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
#include "xap_ModuleManager.h"
#include "xap_Module.h"
#include "ev_EditMethod.h"
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
#include "xap_UnixDialogHelper.h"
#include "ut_debugmsg.h"
#include "ut_PerlBindings.h"

#include "fv_View.h"
#include "libgnomevfs/gnome-vfs.h"

#include <libgnomeui/gnome-window-icon.h>

#include "xap_Prefs.h"
#include "ap_Prefs_SchemeIds.h"

#include "xap_UnixPSGraphics.h"

// quick hack - this is defined in ap_EditMethods.cpp
extern XAP_Dialog_MessageBox::tAnswer s_CouldNotLoadFileMessage(XAP_Frame * pFrame, const char * pNewFile, UT_Error errorCode);

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
#ifdef ABI_OPT_PERL
 	char *script = NULL;
#endif
	char *geometry = NULL;
	char *to = NULL;
	int topng = 0;
	char *printto = NULL;
	int verbose = 1;
	int show = 0;
	char * plugin = NULL;
	int nosplash = 0;

	char *file = NULL;

#ifdef DEBUG
 	gboolean dumpstrings = FALSE;
#endif
	poptContext poptcon;
	static const struct poptOption options[] =
	{{"geometry", 'g', POPT_ARG_STRING, &geometry, 0, "set initial frame geometry", "GEOMETRY"},
	 {"nosplash", 'n', POPT_ARG_NONE,   &nosplash, 0, "do not show splash screen", NULL},

#ifdef ABI_OPT_PERL
	 {"script", 's', POPT_ARG_STRING, &script, 0, "Execute FILE as script", "FILE"},
#endif
#ifdef DEBUG
	 {"dumpstrings", 'd', POPT_ARG_NONE, &dumpstrings, 0, "Dump strings to file", NULL},
#endif
	 {"to", 't', POPT_ARG_STRING, &to, 0, "The target format of the file (abw, zabw, rtf, txt, utf8, html, latex)", "FORMAT"},
	 {"to-png", '\0', POPT_ARG_NONE, &topng, 0, "Convert the incoming file to a PNG image", ""},
	 {"verbose", 'v', POPT_ARG_INT, &verbose, 0, "The verbosity level (0, 1, 2)", "LEVEL"},
	 {"print", 'p', POPT_ARG_STRING, &printto, 0, "print this file to a file or printer", "FILE or |lpr"},
	 {"show", '\0', POPT_ARG_NONE, &show, 0, "If you really want to start the GUI (even if you use the --to option)", ""},
	 {"plugin", '\0', POPT_ARG_NONE, &plugin, 0, "If you want to execute a plugin instead of the main application ", ""},
	 {NULL, '\0', 0, NULL, 0, NULL, NULL} /* end the list */
	};

	//
	// This is a static function.		   
	//

#ifndef ABI_OPT_WIDGET
	gnome_init_with_popt_table("AbiWord", "1.0.0", argc, argv, options, 0, &poptcon);
#endif

	// hack needed to intialize gtk before ::initialize
	gtk_set_locale();

	XAP_Args Args = XAP_Args(argc,argv);
 	AP_UnixGnomeApp * pMyUnixApp = new AP_UnixGnomeApp(&Args, szAppName);

	// if the initialize fails, we don't have icons, fonts, etc.
	if (!pMyUnixApp->initialize())
	{
		delete pMyUnixApp;
		return -1;	// make this something standard?
	}

	if (! gnome_vfs_init ())
	{
	    UT_DEBUGMSG(("DOM: gnome_vfs_init () failed!\n"));
	    return -1;	    
	}

	// do we show the app&splash?
  	bool bShowSplash = true;
 	bool bShowApp = true;

	if ( to || topng || printto )
	  {
	    bShowApp = false;
	    bShowSplash = false;
	  }

	if ( show )
	  {
	    bShowApp = true;
	    bShowSplash = true;	   
	  }
 
	if ( nosplash || plugin )
	  {
	    bShowSplash = false;
	  }

	pMyUnixApp->setDisplayStatus(bShowApp);

	// set the default icon - must be after the call to ::initialize
	// because that's where we call gnome_init()
	UT_String s = pMyUnixApp->getAbiSuiteLibDir();
	s += "/icons/abiword_48.png";
	gnome_window_icon_init ();
	gnome_window_icon_set_default_from_file (s.c_str());

	const XAP_Prefs * pPrefs = pMyUnixApp->getPrefs();

	bool bSplashPref = true;
	if (pPrefs->getPrefsValueBool (AP_PREF_KEY_ShowSplash, &bSplashPref))
	{
	    bShowSplash = bShowSplash && bSplashPref;
	}

	if (bShowSplash)
		_showSplash(2000);

	{
#ifdef DEBUG
	  if (dumpstrings)
	    {
	      // dump the string table in english as a template for translators.
	      // see abi/docs/AbiSource_Localization.abw for details.
	      AP_BuiltinStringSet * pBuiltinStringSet = new AP_BuiltinStringSet(pMyUnixApp,(XML_Char*)AP_PREF_DEFAULT_StringSet);
	      pBuiltinStringSet->dumpBuiltinSet("en-US.strings");
	      delete pBuiltinStringSet;
	    }
#endif

#ifdef ABI_OPT_PERL
	if (script)
	{
		UT_PerlBindings& pb(UT_PerlBindings::getInstance());
		if (!pb.evalFile(script))
			printf("%s\n", pb.errmsg().c_str());
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
		pMyUnixApp->setGeometry(x, y, width, height, f);
	}
	
	if (to) {
		AP_Convert * conv = new AP_Convert(getApp());
		conv->setVerbose(verbose);

		while ((file = poptGetArg (poptcon)) != NULL) {
			conv->convertTo(file, to);
		}

		delete conv;

		if (!show)
		  return false;
	}
	
	if (topng) {
	  AP_Convert * conv = new AP_Convert( getApp() ) ;
	  conv->setVerbose(verbose);
	  while ((file = poptGetArg (poptcon)) != NULL) {
	    conv->convertToPNG(file);
	  }
	  delete conv;
	  
	  if (!show)
	    return false;
	}

	if (printto) {
	  if ((file = poptGetArg (poptcon)) != NULL)
	    {
	      UT_DEBUGMSG(("DOM: Printing file %s\n", file));

	      AP_Convert * conv = new AP_Convert(pMyUnixApp);
	      conv->setVerbose(verbose);
	      
	      PS_Graphics * pG = new PS_Graphics ((printto[0] == '|' ? printto+1 : printto), file, 
						  pMyUnixApp->getApplicationName(), pMyUnixApp->getFontManager(),
						  (printto[0] != '|'), pMyUnixApp);
	      
	      conv->print (file, pG);
	      
	      delete pG;
	      delete conv;
	    }
	  else
	    {
	      // couldn't load document
	      printf("Error: no file to print!\n");
	    }

	  if (!show)
	    return false;
	}
	
	if(plugin)
	{
//
// Start a plugin rather than the main abiword application.
//
	    const char * szName = NULL;
		XAP_Module * pModule = NULL;
		plugin = poptGetArg(poptcon);
		bool bFound = false;
		if(plugin != NULL)
		{
			const UT_Vector * pVec = XAP_ModuleManager::instance().enumModules ();
			for (UT_uint32 i = 0; (i < pVec->size()) && !bFound; i++)
			{
				pModule = (XAP_Module *)pVec->getNthItem (i);
				szName = pModule->getModuleInfo()->name;
				if(UT_strcmp(szName,plugin) == 0)
				{
					bFound = true;
				}
			}
		}
		if(!bFound)
		{
			printf("Plugin %s not found or loaded \n",plugin);
			return false;
		}
//
// You must put the name of the ev_EditMethod in the usage field
// of the plugin registered information.
//
		const char * evExecute = pModule->getModuleInfo()->usage;
		EV_EditMethodContainer* pEMC = pMyUnixApp->getEditMethodContainer();
		const EV_EditMethod * pInvoke = pEMC->findEditMethodByName(evExecute);
		if(!pInvoke)
		{
			printf("Plugin %s invoke method %s not found \n",plugin,evExecute);
			return false;
		}
//
// Execute the plugin, then quit
//
		ev_EditMethod_invoke(pInvoke, "Called From UnixGnomeApp");
		return false;
	}
	}

	// this function takes care of all the command line args.
	// if some args are botched, it returns false and we should
	// continue out the door.
	if (pMyUnixApp->parseCommandLine(poptcon) && bShowApp)
	{
		// turn over control to gtk
		gtk_main();
	}
	else
	  {
	    UT_DEBUGMSG(("DOM: not parsing command line or showing app\n"));
	  }

	// destroy the App.  It should take care of deleting all frames.
	pMyUnixApp->shutdown();
	delete pMyUnixApp;
	
	poptFreeContext (poptcon);

	return 0;
}

bool AP_UnixGnomeApp::parseCommandLine(poptContext poptcon)
{
	int kWindowsOpened = 0;
	char *file = NULL;

	// parse the command line
	// <app> [--script <scriptname>]* [--dumpstrings] [--to <format>] [--geometry <format>] [<documentname>]*
	
	// TODO when we refactor the App classes, consider moving
	// TODO this to app-specific, cross-platform.
	
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

			s_CouldNotLoadFileMessage (pFirstUnixFrame, file, error);

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





