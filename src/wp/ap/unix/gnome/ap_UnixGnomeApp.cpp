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
#include <fcntl.h>
#include <signal.h>

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
#if 1
#include <bonobo.h>
#include <liboaf/liboaf.h>
#include "abiwidget.h"
#endif

// quick hack - this is defined in ap_EditMethods.cpp
extern XAP_Dialog_MessageBox::tAnswer s_CouldNotLoadFileMessage(XAP_Frame * pFrame, const char * pNewFile, UT_Error errorCode);


//
// Static functions needed for the bonobo control...
//
static int mainBonobo(int argc, char * argv[]);
static BonoboObject*
bonobo_AbiWidget_factory  (BonoboGenericFactory *factory, void *closure);

static BonoboControl* 	AbiControl_construct(BonoboControl * control, AbiWidget * abi);
static BonoboControl * AbiWidget_control_new(AbiWidget * abi);

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
	int k=0;
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

	XAP_Args Args = XAP_Args(argc,argv);
//
// Check to see if we've been activated as a control by OAF
//
	bool bControlFactory = false;
  	for (k = 1; k < Args.m_argc; k++)
  		if (*Args.m_argv[k] == '-')
  			if (strstr(Args.m_argv[k],"GNOME_AbiWord_ControlFactory") != 0)
  			{
 				bControlFactory = true;
  				break;
  			}
	if(bControlFactory)
	{
		
		AP_UnixGnomeApp * pMyUnixApp = new AP_UnixGnomeApp(&Args, szAppName);
		pMyUnixApp->setBonoboRunning();
		
		/* intialize gnome - need this before initialize method */
		gtk_set_locale();
		gnome_init_with_popt_table ("GNOME_AbiWord_ControlFactory",  "0.0",
									Args.m_argc, Args.m_argv, oaf_popt_options, 0, NULL);
		if (!pMyUnixApp->initialize())
		{
			delete pMyUnixApp;
			return -1;	// make this something standard?
		}
		mainBonobo(argc,argv);

		// destroy the App.  It should take care of deleting all frames.

		pMyUnixApp->shutdown();
		delete pMyUnixApp;
		return 0;
	}

	//
	// This is a static function.		   
	//

#ifndef ABI_OPT_WIDGET
	gnome_init_with_popt_table("AbiWord", "1.0.0", argc, argv, options, 0, &poptcon);
#endif

	// hack needed to intialize gtk before ::initialize
	gtk_set_locale();

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

//-------------------------------------------------------------------
// Bonobo Control factory stuff
//-------------------------------------------------------------------

/* 
 * get a value from abiwidget
 */ 
static void get_prop (BonoboPropertyBag 	*bag,
	  BonoboArg 		*arg,
	  guint 		 arg_id,
	  CORBA_Environment 	*ev,
	  gpointer 		 user_data)
{
	AbiWidget 	*abi;
	
	g_return_if_fail (IS_ABI_WIDGET(user_data));
		
	/*
	 * get data from our AbiWidget
	 */
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

/*
 * Tell abiwidget to do something.
 */
static void set_prop (BonoboPropertyBag 	*bag,
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


/*
 * Loads a document from a Bonobo_Stream. Code gratitutously stolen 
 * from ggv
 */
static void
load_document_from_stream (BonoboPersistStream *ps,
					 Bonobo_Stream stream,
					 Bonobo_Persist_ContentType type,
					 void *data,
					 CORBA_Environment *ev)
{
	AbiWidget *abiwidget;
	Bonobo_Stream_iobuf *buffer;
	CORBA_long len_read;
    FILE * tmpfile;
	gboolean bMapToScreen = false;

	g_return_if_fail (data != NULL);
	g_return_if_fail (IS_ABI_WIDGET (data));

	abiwidget = (AbiWidget *) data;

	/* copy stream to a tmp file */
//
// Create a temp file name.
//
	char szTempfile[ 2048 ];
	UT_tmpnam(szTempfile);

	tmpfile = fopen(szTempfile, "w");

	do 
	{
		Bonobo_Stream_read (stream, 32768, &buffer, ev);
		if (ev->_major != CORBA_NO_EXCEPTION)
			goto exit_clean;

		len_read = buffer->_length;

		if (buffer->_buffer && len_read)
			if(fwrite(buffer->_buffer, 1, len_read, tmpfile) != len_read) 
			{
				CORBA_free (buffer);
				goto exit_clean;
			}

		CORBA_free (buffer);
	} 
	while (len_read > 0);

	fclose(tmpfile);

//
// Load the file.
//
//
	gtk_object_set(GTK_OBJECT(abiwidget),"AbiWidget::unlink_after_load",(gboolean) TRUE,NULL);
	gtk_object_set(GTK_OBJECT(abiwidget),"AbiWidget::load_file",(gchar *) szTempfile,NULL);
	return;

 exit_clean:
	fclose (tmpfile);
	unlink(szTempfile);
	return;
}

//
// Implements the get_object interface.
//
static Bonobo_Unknown
abiwidget_get_object(BonoboItemContainer *item_container,
							   CORBA_char          *item_name,
							   CORBA_boolean       only_if_exists,
							   CORBA_Environment   *ev,
							   AbiWidget *  abi)
{
	Bonobo_Unknown corba_object;
	BonoboObject *object = NULL;

	g_return_val_if_fail(abi != NULL, CORBA_OBJECT_NIL);
	g_return_val_if_fail(IS_ABI_WIDGET(abi), CORBA_OBJECT_NIL);

	g_message ("abiwiget_get_object: %d - %s",
			   only_if_exists, item_name);

#if 0
	GSList *params, *c;
	params = ggv_split_string (item_name, "!");
	for (c = params; c; c = c->next) {
		gchar *name = c->data;

		if ((!strcmp (name, "control") || !strcmp (name, "embeddable"))
		    && (object != NULL)) {
			g_message ("ggv_postscript_view_get_object: "
					   "can only return one kind of an Object");
			continue;
		}

		if (!strcmp (name, "control"))
		    object = AbiWidget_control_new(AbiWidget * abi);
		else
			g_message ("ggv_postscript_view_get_object: "
					   "unknown parameter `%s'",
					   name);
	}
	g_slist_foreach (params, (GFunc) g_free, NULL);
	g_slist_free (params);
#endif

	object = (BonoboObject *) AbiWidget_control_new(abi);


	if (object == NULL)
		return NULL;

	corba_object = bonobo_object_corba_objref (object);

	return bonobo_object_dup_ref (corba_object, ev);
}

/*
 * Loads a document from a Bonobo_File. Code gratitutously stolen 
 * from ggv
 */
static int
load_document_from_file(BonoboPersistFile *pf, const CORBA_char *filename,
				   CORBA_Environment *ev, void *data)
{
	AbiWidget *abiwidget;
	gboolean bMapToScreen = false;

	g_return_val_if_fail (data != NULL,-1);
	g_return_val_if_fail (IS_ABI_WIDGET (data),-1);

	abiwidget = ABI_WIDGET (data);

//
// Load the file.
//
	gtk_object_set(GTK_OBJECT(abiwidget),"AbiWidget::load_file",(gchar *) filename,NULL);
	return 0;
}

//
// Data content for persist stream
//
static Bonobo_Persist_ContentTypeList *
pstream_get_content_types (BonoboPersistStream *ps, void *closure,
			   CORBA_Environment *ev)
{
	return bonobo_persist_generate_content_types (3,"application/msword","application/rtf","application/x-abiword");
}


//
// Add extra interfaces to load data into the control
//
BonoboObject *
AbiControl_add_interfaces (AbiWidget *abiwidget,
									BonoboObject *to_aggregate)
{
	BonoboPersistFile   *file;
	BonoboPersistStream *stream;
	BonoboItemContainer *item_container;

	g_return_val_if_fail (IS_ABI_WIDGET(abiwidget), NULL);
	g_return_val_if_fail (BONOBO_IS_OBJECT (to_aggregate), NULL);

	/* Interface Bonobo::PersistStream */
	stream = bonobo_persist_stream_new (load_document_from_stream, 
										NULL, NULL, pstream_get_content_types, abiwidget);
	if (!stream) {
		bonobo_object_unref (BONOBO_OBJECT (to_aggregate));
		return NULL;
	}

	bonobo_object_add_interface (BONOBO_OBJECT (to_aggregate),
								 BONOBO_OBJECT (stream));

	/* Interface Bonobo::PersistFile */

	file = bonobo_persist_file_new (load_document_from_file,
									NULL, (void *) abiwidget);
	if (!file) 
	{
		bonobo_object_unref (BONOBO_OBJECT (to_aggregate));
		return NULL;
	}

	bonobo_object_add_interface (BONOBO_OBJECT (to_aggregate),
								 BONOBO_OBJECT (file));

	/* BonoboItemContainer */

	item_container = bonobo_item_container_new ();

	gtk_signal_connect (GTK_OBJECT (item_container),
						"get_object",
						GTK_SIGNAL_FUNC (abiwidget_get_object),
						abiwidget);

	bonobo_object_add_interface (BONOBO_OBJECT (to_aggregate),
								 BONOBO_OBJECT (item_container));

	return to_aggregate;
}

static BonoboControl * AbiWidget_control_new(AbiWidget * abi)
{
    BonoboControl * control;
	g_return_val_if_fail(abi != NULL, NULL);
	g_return_val_if_fail(IS_ABI_WIDGET(abi), NULL);
	/* create a BonoboControl from a widget */
	control = bonobo_control_new (GTK_WIDGET(abi));
	control = AbiControl_construct(control, abi);

	return control;
}


static BonoboControl* 	AbiControl_construct(BonoboControl * control, AbiWidget * abi)
{
	BonoboPropertyBag 	*prop_bag;
	g_return_val_if_fail(abi != NULL, NULL);
	g_return_val_if_fail(control != NULL, NULL);
	g_return_val_if_fail(IS_ABI_WIDGET(abi), NULL);
	/* 
	 * create a property bag:
	 * we provide our accessor functions for properties, 	and 
	 * the gtk widget
	 * */
	prop_bag = bonobo_property_bag_new (get_prop, set_prop, abi);
	bonobo_control_set_properties (control, prop_bag);

	/* put all AbiWidget's arguments in the property bag - way cool!! */
  
	bonobo_property_bag_add_gtk_args (prop_bag,GTK_OBJECT(abi));
//
// persist_stream , persist_file interfaces/methods, item container
// 
	AbiControl_add_interfaces (ABI_WIDGET(abi),
							   BONOBO_OBJECT(control));
	/*
	 *  we don't need the property bag anymore here, so unref it
	 */
	
	bonobo_object_unref (BONOBO_OBJECT(prop_bag));
	return control;
}


 /*
 *  produce a brand new bonobo_AbiWord_control
 *  (this is a callback function, registered in 
 *  	'bonobo_generic_factory_new')
 */
static BonoboObject*
bonobo_AbiWidget_factory  (BonoboGenericFactory *factory, void *closure)
{
	BonoboControl* 		 control;
	GtkWidget*     		 abi;

	/*
	 * create a new AbiWidget instance
	 */

	AP_UnixApp * pApp = (AP_UnixApp *) XAP_App::getApp();
	abi = abi_widget_new_with_app (pApp);
	gtk_widget_show (abi);

	/* create a BonoboControl from a widget */

	control = bonobo_control_new (abi);
	control = AbiControl_construct(control, ABI_WIDGET(abi));

	return BONOBO_OBJECT (control);
}

static int mainBonobo(int argc, char * argv[])
{


    
#if 0
//
// For debuging
//
    // Setup signal handlers, primarily for segfault
    // If we segfaulted before here, we *really* blew it
    
    struct sigaction sa;
    
    sa.sa_handler = signalWrapper;
    
    sigfillset(&sa.sa_mask);  // We don't want to hear about other signals
    sigdelset(&sa.sa_mask, SIGABRT); // But we will call abort(), so we can't ignore that
/* #ifndef AIX - I presume these are always #define not extern... -fjf */
#if defined (SA_NODEFER) && defined (SA_RESETHAND)
    sa.sa_flags = SA_NODEFER | SA_RESETHAND; // Don't handle nested signals
#else
    sa.sa_flags = 0;
#endif
    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGBUS, &sa, NULL);
    sigaction(SIGILL, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGFPE, &sa, NULL);
    sigaction(9, &sa, NULL);
#endif


	BonoboGenericFactory 	*factory;
	CORBA_ORB 		 orb;
	/*
	 * initialize oaf and bonobo
	 */
	orb = oaf_init (argc, argv);
	if (!orb)
		printf ("initializing orb failed \n");
	
	if (!bonobo_init (orb, NULL, NULL))
		printf("initializing Bonobo failed \n");

	/* register the factory (using OAF) */
	factory = bonobo_generic_factory_new
		("OAFIID:GNOME_AbiWord_ControlFactory",
		 bonobo_AbiWidget_factory, NULL);
	if (!factory)
		printf("Registration of Bonobo button factory failed");

	
	/*
	 *  make sure we're unreffing upon exit;
	 *  enter the bonobo main loop
	 */
	bonobo_running_context_auto_exit_unref (BONOBO_OBJECT(factory));
	bonobo_main ();
	return 0;
}






