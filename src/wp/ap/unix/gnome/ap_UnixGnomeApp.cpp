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

#define ABIWORD_INTERNAL

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
#include "ap_Args.h"

#include "xap_UnixPSGraphics.h"
#if 1
#include <bonobo.h>
#include <liboaf/liboaf.h>
#include "abiwidget.h"
#endif

// quick hack - this is defined in ap_EditMethods.cpp
extern XAP_Dialog_MessageBox::tAnswer s_CouldNotLoadFileMessage(XAP_Frame * pFrame, const char * pNewFile, UT_Error errorCode);

/*****************************************************************/
/* Static functions needed for the bonobo control...             */

static int s_mainBonobo(int argc, char * argv[]);
static bool s_checkControl(const char * szAppName, XAP_Args XArgs);
static BonoboObject*
   bonobo_AbiWidget_factory  (BonoboGenericFactory *factory, void *closure);

// and for popt
static void s_poptInit(AP_Args *Args);

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
	// This is a static function.		   

	// Step 0: If magic cookie is in XArgs, we're a bonobo control; quit.
	XAP_Args XArgs = XAP_Args(argc,argv);

	if (s_checkControl(szAppName, XArgs))
		return 0;

	AP_Args Args = AP_Args(&XArgs, szAppName, &s_poptInit, doWindowlessArgs);

	// Step 1: Initialize GTK and create the APP.
    // HACK: these calls to gtk reside properly in 
    // HACK: XAP_UNIXBASEAPP::initialize(), but need to be here 
    // HACK: to throw the splash screen as soon as possible.
	// hack needed to intialize gtk before ::initialize
	gtk_set_locale();

 	AP_UnixGnomeApp * pMyUnixApp = new AP_UnixGnomeApp(&XArgs, szAppName);
	Args.setApp(getApp());

	// if the initialize fails, we don't have icons, fonts, etc.
	if (!pMyUnixApp->initialize())
	{
		delete pMyUnixApp;
		return -1;	// make this something standard?
	}

	if (!gnome_vfs_init())
	{
	    UT_DEBUGMSG(("DOM: gnome_vfs_init () failed!\n"));
	    return -1;	    
	}

	// do we show the app&splash?
  	bool bShowSplash = Args.getShowSplash();
 	bool bShowApp = Args.getShowApp();

	pMyUnixApp->setDisplayStatus(bShowApp);

	// set the default icon - must be after the call to ::initialize
	// because that's where we call gnome_init()
	UT_String s = pMyUnixApp->getAbiSuiteLibDir(); 
	s += "/icons/abiword_48.png";
	gnome_window_icon_init ();
	gnome_window_icon_set_default_from_file (s.c_str());

	const XAP_Prefs * pPrefs = pMyUnixApp->getPrefs();
	UT_ASSERT(pPrefs);

	bool bSplashPref = true;
	if (pPrefs &&
		pPrefs->getPrefsValueBool (AP_PREF_KEY_ShowSplash, &bSplashPref))
	{
	    bShowSplash = bShowSplash && bSplashPref;
	}

	if (bShowSplash)
		_showSplash(2000);

	// Step 2: Handle all non-window args.

	if (!Args.doWindowlessArgs())
		return false;

	// Step 3: Create windows as appropriate.
	// if some args are botched, it returns false and we should
	// continue out the door.
	if (pMyUnixApp->parseCommandLine(Args.poptcon) && bShowApp)
	{
		// turn over control to gtk
		gtk_main();
	}
	else
	{
	    UT_DEBUGMSG(("DOM: not parsing command line or showing app\n"));
	}

	// Step 4: Destroy the App.  It should take care of deleting all frames.
	pMyUnixApp->shutdown();
	delete pMyUnixApp;
	
	poptFreeContext (Args.poptcon);

	return 0;
}

static void s_poptInit(AP_Args *Args)
{
	int v = -1, i;

	for (i = 0; Args->const_opts[i].longName != NULL; i++)
		if (!strcmp(Args->const_opts[i].longName, "version"))
		{ 
			v = i; break; 
		}

	if (v == -1)
		v = i;

	struct poptOption * opts = (struct poptOption *)
		UT_calloc(v+1, sizeof(struct poptOption));
	for (int j = 0; j < v; j++)
		opts[j] = Args->const_opts[j];

	Args->options = opts;

#ifndef ABI_OPT_WIDGET
	// This is deprecated.  We're supposed to use gnome_program_init!
	gnome_init_with_popt_table("AbiWord", "1.0.0", 
				   Args->XArgs->m_argc, Args->XArgs->m_argv, 
				   Args->options, 0, &Args->poptcon);
#endif
}

//-------------------------------------------------------------------
// Bonobo Control factory stuff
//-------------------------------------------------------------------

/*!
 * Check to see if we've been activated as a control by OAF
 * \return true if we're already done.
 */
static bool s_checkControl(const char * szAppName, XAP_Args XArgs)
{
	bool bControlFactory = false;
  	for (int k = 1; k < XArgs.m_argc; k++)
  		if (*XArgs.m_argv[k] == '-')
  			if (strstr(XArgs.m_argv[k],
					   "AbiSource_AbiWord_ControlFactory") != 0)
 				bControlFactory = true;

	if(bControlFactory)
	{
		AP_UnixGnomeApp * pMyUnixApp = new AP_UnixGnomeApp(&XArgs, szAppName);
	
		/* intialize gnome - need this before initialize method */
		gtk_set_locale();
		gnome_init_with_popt_table ("AbiSource_AbiWord_ControlFactory",  "0.0",
									XArgs.m_argc, XArgs.m_argv, 
									oaf_popt_options, 0, NULL);
		if (!pMyUnixApp->initialize())
		{
			delete pMyUnixApp;
			exit(-1);
		}
		s_mainBonobo(XArgs.m_argc,XArgs.m_argv);

		// destroy the App.  It should take care of deleting all frames.

		pMyUnixApp->shutdown();
		delete pMyUnixApp;
		return true;
	}
	return false;
}

/*! 
 * Generic main function for a Bonobo control.
 */
static int s_mainBonobo(int argc, char * argv[])
{
	BonoboGenericFactory 	*factory;
	CORBA_ORB 		 orb;

	/*
	 * initialize oaf and bonobo
	 */
	orb = oaf_init (argc, argv);
	if (!orb)
		g_error ("initializing orb failed");
	
	if (!bonobo_init (orb, NULL, NULL))
		g_error ("initializing Bonobo failed");

	/* register the factory (using OAF) */
	factory = bonobo_generic_factory_new
		("OAFIID:AbiSource_AbiWord_ControlFactory",
		 bonobo_AbiWidget_factory, NULL);
	if (!factory)
		g_error ("Registration of Bonobo button factory failed");
	
	/*
	 *  make sure we're unreffing upon exit;
	 *  enter the bonobo main loop
	 */
	bonobo_running_context_auto_exit_unref (BONOBO_OBJECT(factory));
	bonobo_main ();
	
	return 0;
}

/*!
 * Get a value from AbiWidget
 */ 
static void s_getProp (BonoboPropertyBag 	*bag,
					   BonoboArg 		*arg,
					   guint 		 arg_id,
					   CORBA_Environment 	*ev,
					   gpointer 		 user_data)
{
	AbiWidget 	*abi;
	
	g_return_if_fail (IS_ABI_WIDGET(user_data));
		
//
// first create a fresh gtkargument.
//
	GtkArg * gtk_arg = (GtkArg *) g_new0 (GtkArg,1);
//
// Now copy the bonobo argument to this so we know what to extract from
// AbiWidget.
//
	bonobo_arg_to_gtk(gtk_arg,arg);
//
// OK get the data from the widget. Only one argument at a time.
//
	abi = ABI_WIDGET(user_data); 
	gtk_object_getv(GTK_OBJECT(abi),1,gtk_arg);
//
// Now copy it back to the bonobo argument.
//
	bonobo_arg_from_gtk (arg, gtk_arg);
//
// Free up allocated memory
//
	if (gtk_arg->type == GTK_TYPE_STRING && GTK_VALUE_STRING (*gtk_arg))
	{
		g_free (GTK_VALUE_STRING (*gtk_arg));
	}
	g_free(gtk_arg);
}

/*!
 * Tell AbiWidget to do something.
 */
static void s_setProp (BonoboPropertyBag 	*bag,
					   const BonoboArg 	*arg,
					   guint 		 arg_id,
					   CORBA_Environment 	*ev,
					   gpointer 		 user_data)
{
	AbiWidget 	*abi;
	
	g_return_if_fail (IS_ABI_WIDGET(user_data));
		
	abi = ABI_WIDGET (user_data); 
//
// Have to translate BonoboArg to GtkArg now. This is really easy.
//
	GtkArg * gtk_arg = (GtkArg *) g_new0 (GtkArg,1);
	bonobo_arg_to_gtk(gtk_arg,arg);
//
// Can only pass one argument at a time.
//
	gtk_object_setv(GTK_OBJECT(abi),1,gtk_arg);
//
// Free up allocated memory
//
	if (gtk_arg->type == GTK_TYPE_STRING && GTK_VALUE_STRING (*gtk_arg))
	{
		g_free (GTK_VALUE_STRING (*gtk_arg));
	}
	g_free(gtk_arg);
}

/*!
 *  Produce a brand new bonobo_AbiWord_control.
 *  This is a callback function, registered in 
 *  	'bonobo_generic_factory_new'.
 */
static BonoboObject*
bonobo_AbiWidget_factory  (BonoboGenericFactory *factory, void *closure)
{
	BonoboControl* 		 control;
	BonoboPropertyBag 	*prop_bag;
	GtkWidget*     		 abi;

	/*
	 * create a new AbiWidget instance
	 */

	AP_UnixApp * pApp = (AP_UnixApp *) XAP_App::getApp();
	abi = abi_widget_new_with_app (pApp);
	gtk_widget_show (abi);

	/* create a BonoboControl from a widget */

	control = bonobo_control_new (abi);

	/* 
	 * create a property bag:
	 * we provide our accessor functions for properties, 	and 
	 * the gtk widget
	 */
	prop_bag = bonobo_property_bag_new (s_getProp, s_setProp, abi);
	bonobo_control_set_properties (control, prop_bag);

	/* put all AbiWidget's arguments in the property bag - way cool!! */
  
	bonobo_property_bag_add_gtk_args (prop_bag,GTK_OBJECT(abi)); 

	/*
	 *  we don't need the property bag anymore here, so unref it
	 */
	bonobo_object_unref (BONOBO_OBJECT(prop_bag));

	return BONOBO_OBJECT (control);
}

