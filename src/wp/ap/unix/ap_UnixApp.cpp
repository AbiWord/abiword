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
#include "sp_spell.h"
#include "ap_Strings.h"
#include "xap_EditMethods.h"
#include "ap_LoadBindings.h"
#include "xap_Menu_ActionSet.h"
#include "xap_Toolbar_ActionSet.h"

#include "ie_imp.h"
#include "ie_types.h"

#include "gr_Graphics.h"
#include "gr_UnixGraphics.h"
#include "gr_Image.h"
#include "ut_bytebuf.h"
#include "ut_png.h"
#include "ut_dialogHelper.h"

/*****************************************************************/

AP_UnixApp::AP_UnixApp(XAP_Args * pArgs, const char * szAppName)
	: XAP_UnixApp(pArgs,szAppName)
{
	m_prefs = NULL;
	m_pStringSet = NULL;
}

AP_UnixApp::~AP_UnixApp(void)
{
	SpellCheckCleanup();

	DELETEP(m_prefs);
	DELETEP(m_pStringSet);
}

static UT_Bool s_createDirectoryIfNecessary(const char * szDir)
{
	struct stat statbuf;
	
	if (stat(szDir,&statbuf) == 0)								// if it exists
	{
		if (S_ISDIR(statbuf.st_mode))							// and is a directory
			return UT_TRUE;

		UT_DEBUGMSG(("Pathname [%s] is not a directory.\n",szDir));
		return UT_FALSE;
	}
	
	if (mkdir(szDir,0700) == 0)
		return UT_TRUE;
	

	UT_DEBUGMSG(("Could not create Directory [%s].\n",szDir));
	return UT_FALSE;
}	

UT_Bool AP_UnixApp::initialize(void)
{
	const char * szUserPrivateDirectory = getUserPrivateDirectory();
	UT_Bool bVerified = s_createDirectoryIfNecessary(szUserPrivateDirectory);
	UT_ASSERT(bVerified);
	
	// load preferences, first the builtin set and then any on disk.
	
	m_prefs = new AP_UnixPrefs(this);
	m_prefs->loadBuiltinPrefs();
	m_prefs->loadPrefsFile();

	// TODO overlay command line arguments onto preferences...
		   
	// now that preferences are established, let the xap init
		   
	m_pEMC = AP_GetEditMethods();
	UT_ASSERT(m_pEMC);

	m_pBindingSet = new AP_BindingSet(m_pEMC);
	UT_ASSERT(m_pBindingSet);
	
	m_pMenuActionSet = AP_CreateMenuActionSet();
	UT_ASSERT(m_pMenuActionSet);

	m_pToolbarActionSet = AP_CreateToolbarActionSet();
	UT_ASSERT(m_pToolbarActionSet);

	if (! XAP_UnixApp::initialize())
		return UT_FALSE;
	
	//////////////////////////////////////////////////////////////////
	// initializes the spell checker.
	//////////////////////////////////////////////////////////////////
	
	{
		const char * szISpellDirectory = NULL;
		getPrefsValueDirectory(AP_PREF_KEY_SpellDirectory,&szISpellDirectory);
		UT_ASSERT((szISpellDirectory) && (*szISpellDirectory));

		const char * szSpellCheckWordList = NULL;
		if ((getPrefsValue(AP_PREF_KEY_SpellCheckWordList,&szSpellCheckWordList)) && (szSpellCheckWordList) && (*szSpellCheckWordList))
			;
		else
			szSpellCheckWordList = AP_PREF_DEFAULT_SpellCheckWordList;
		
		char * szPathname = (char *)calloc(sizeof(char),strlen(szISpellDirectory)+strlen(szSpellCheckWordList)+2);
		UT_ASSERT(szPathname);
		
		sprintf(szPathname,"%s%s%s",
				szISpellDirectory,
				((szISpellDirectory[strlen(szISpellDirectory)-1]=='/') ? "" : "/"),
				szSpellCheckWordList);

		UT_DEBUGMSG(("Loading SpellCheckWordList [%s]\n",szPathname));
		SpellCheckInit(szPathname);
		free(szPathname);
		
		// we silently go on if we cannot load it....
	}
	
	//////////////////////////////////////////////////////////////////
	// load the dialog and message box strings
	//////////////////////////////////////////////////////////////////
	
	{
		// assume we will be using the builtin set (either as the main
		// set or as the fallback set).
		
		AP_BuiltinStringSet * pBuiltinStringSet = new AP_BuiltinStringSet(this,AP_PREF_DEFAULT_StringSet);
		UT_ASSERT(pBuiltinStringSet);
		m_pStringSet = pBuiltinStringSet;

		// see if we should load an alternate set from the disk
		
		const char * szDirectory = NULL;
		const char * szStringSet = NULL;

		if (   (getPrefsValue(AP_PREF_KEY_StringSet,&szStringSet))
			&& (szStringSet)
			&& (*szStringSet)
			&& (UT_stricmp(szStringSet,AP_PREF_DEFAULT_StringSet) != 0))
		{
			getPrefsValueDirectory(AP_PREF_KEY_StringSetDirectory,&szDirectory);
			UT_ASSERT((szDirectory) && (*szDirectory));

			char * szPathname = (char *)calloc(sizeof(char),strlen(szDirectory)+strlen(szStringSet)+100);
			UT_ASSERT(szPathname);

			sprintf(szPathname,"%s%s%s.strings",
					szDirectory,
					((szDirectory[strlen(szDirectory)-1]=='/') ? "" : "/"),
					szStringSet);

			AP_DiskStringSet * pDiskStringSet = new AP_DiskStringSet(this);
			UT_ASSERT(pDiskStringSet);

			if (pDiskStringSet->loadStringsFromDisk(szPathname))
			{
				pDiskStringSet->setFallbackStringSet(m_pStringSet);
				m_pStringSet = pDiskStringSet;
				UT_DEBUGMSG(("Using StringSet [%s]\n",szPathname));
			}
			else
			{
				DELETEP(pDiskStringSet);
				UT_DEBUGMSG(("Unable to load StringSet [%s] -- using builtin strings instead.\n",szPathname));
			}
				
			free(szPathname);
		}
	}

	//////////////////////////////////////////////////////////////////

	return UT_TRUE;
}

XAP_Frame * AP_UnixApp::newFrame(void)
{
	AP_UnixFrame * pUnixFrame = new AP_UnixFrame(this);

	if (pUnixFrame)
		pUnixFrame->initialize();

	return pUnixFrame;
}

UT_Bool AP_UnixApp::shutdown(void)
{
	if (m_prefs->getAutoSavePrefs())
		m_prefs->savePrefsFile();

	return UT_TRUE;
}

XAP_Prefs * AP_UnixApp::getPrefs(void) const
{
	return m_prefs;
}

UT_Bool AP_UnixApp::getPrefsValue(const XML_Char * szKey, const XML_Char ** pszValue) const
{
	if (!m_prefs)
		return UT_FALSE;

	return m_prefs->getPrefsValue(szKey,pszValue);
}

UT_Bool AP_UnixApp::getPrefsValueDirectory(const XML_Char * szKey, const XML_Char ** pszValue) const
{
	if (!m_prefs)
		return UT_FALSE;

	const XML_Char * psz = NULL;
	if (!m_prefs->getPrefsValue(szKey,&psz))
		return UT_FALSE;

	if (*psz == '/')
	{
		*pszValue = psz;
		return UT_TRUE;
	}

	static XML_Char buf[1024];
	UT_ASSERT((strlen(getAbiSuiteLibDir()) + strlen(psz) + 2) < sizeof(buf));
	
	sprintf(buf,"%s/%s",getAbiSuiteLibDir(),psz);
	*pszValue = buf;
	return UT_TRUE;
}

const XAP_StringSet * AP_UnixApp::getStringSet(void) const
{
	return m_pStringSet;
}

/*****************************************************************/


static GtkWidget * wSplash = NULL;
static GR_Image * pSplashImage = NULL;
static GR_UnixGraphics * pUnixGraphics = NULL;
static guint timeout_handler = 0;
static UT_Bool firstExpose = FALSE;

static gint s_hideSplash(gpointer /*data*/)
{
	if (wSplash)
	{
		gtk_timeout_remove(timeout_handler);
		gtk_widget_destroy(wSplash);
		wSplash = NULL;
		DELETEP(pUnixGraphics);
		DELETEP(pSplashImage);
	}
	return TRUE;
}

// GTK never seems to let me have this event on a pop-up style window
//static void s_key_event(GtkWidget * /*window*/, GdkEventKey * /*key*/)
//{
//	s_hideSplash(NULL);
//}

static void s_button_event(GtkWidget * /*window*/)
{
	s_hideSplash(NULL);
}

static gint s_drawingarea_expose(GtkWidget * /* widget */,
								 GdkEventExpose * /* pExposeEvent */)
{
	if (pUnixGraphics && pSplashImage)
	{
		pUnixGraphics->drawImage(pSplashImage, 0, 0);

		// on the first full paint of the image, start a 2 second timer
		if (!firstExpose)
		{
			firstExpose = UT_TRUE;
			timeout_handler = gtk_timeout_add(2000, s_hideSplash, NULL);
		}
	}

	return FALSE;
}

static GR_Image * _showSplash(XAP_Args * pArgs, const char * /*szAppName*/)
{
	wSplash = NULL;
	pSplashImage = NULL;

	UT_ByteBuf* pBB = NULL;
	UT_Bool bShowSplash = UT_TRUE;
	const char * szFile = "splash.png";

	// Unix does put the program name in argv[0], unlike Win32, so [1] is the first argument
	int nFirstArg = 1;
	int k;
	
	// scan args for splash-related stuff
	for (k=nFirstArg; (k<pArgs->m_argc); k++)
	{
		if (*pArgs->m_argv[k] == '-')
		{
			if (UT_stricmp(pArgs->m_argv[k],"-nosplash") == 0)
			{
				bShowSplash = UT_FALSE;
				break;
			}
#if DEBUG
			else if (UT_stricmp(pArgs->m_argv[k],"-splash") == 0)
			{
				// [-splash filename]
				szFile = pArgs->m_argv[k+1];
				break;

				// NOTE: this switch is just for debugging artwork, so 
				// it's OK that the filename also gets opened as a document
			}
#endif
		}

		// TODO: platform-specific reasons to not show splash?
		// TODO: for example, if being launched via DDE or OLE??
	}

	if (!bShowSplash)
		goto Done;

	extern unsigned char g_pngSplash[];		// see ap_wp_Splash.cpp
	extern unsigned long g_pngSplash_sizeof;	// see ap_wp_Splash.cpp

	pBB = new UT_ByteBuf();
	if (
		(pBB->insertFromFile(0, szFile))
		|| (pBB->ins(0, g_pngSplash, g_pngSplash_sizeof))
		)
	{
		// get splash size
		UT_sint32 iSplashWidth;
		UT_sint32 iSplashHeight;
		UT_PNG_getDimensions(pBB, iSplashWidth, iSplashHeight);

		// create a centered window the size of our image
		wSplash = gtk_window_new(GTK_WINDOW_POPUP);
		gtk_object_set_data(GTK_OBJECT(wSplash), "wSplash", wSplash);
		gtk_widget_set_usize(wSplash, iSplashWidth, iSplashHeight);
		gtk_window_set_policy(GTK_WINDOW(wSplash), FALSE, FALSE, FALSE);

		// create a frame to add depth
		GtkWidget * frame = gtk_frame_new(NULL);
		gtk_object_set_data(GTK_OBJECT(wSplash), "frame", frame);
		gtk_container_add(GTK_CONTAINER(wSplash), frame);
		gtk_frame_set_shadow_type(GTK_FRAME(frame), GTK_SHADOW_OUT);
		gtk_widget_show(frame);

		// create a drawing area
		GtkWidget * da = gtk_drawing_area_new ();
		gtk_object_set_data(GTK_OBJECT(wSplash), "da", da);
		gtk_widget_set_events(da, GDK_ALL_EVENTS_MASK);
		gtk_widget_set_usize(da, iSplashWidth, iSplashHeight);
		gtk_signal_connect(GTK_OBJECT(da), "expose_event",
						   GTK_SIGNAL_FUNC(s_drawingarea_expose), NULL);
//		gtk_signal_connect(GTK_OBJECT(da), "key_press_event",
//						   GTK_SIGNAL_FUNC(s_key_event), NULL);
		gtk_signal_connect(GTK_OBJECT(da), "button_press_event",
						   GTK_SIGNAL_FUNC(s_button_event), NULL);
		gtk_container_add(GTK_CONTAINER(frame), da);
		gtk_widget_show(da);

		// now bring the window up front & center
		gtk_window_set_position(GTK_WINDOW(wSplash), GTK_WIN_POS_CENTER);

		// create the window so we can attach a GC to it
		gtk_widget_show(wSplash);

		// create image context
		pUnixGraphics = new GR_UnixGraphics(da->window, NULL);
		pSplashImage = pUnixGraphics->createNewImage("splash", pBB, iSplashWidth, iSplashHeight);

		// another for luck (to bring it up forward and paint)
		gtk_widget_show(wSplash);
	}

	DELETEP(pBB);

Done:
	return pSplashImage;
}

/*****************************************************************/

int AP_UnixApp::main(const char * szAppName, int argc, char ** argv)
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

	AP_UnixApp * pMyUnixApp = new AP_UnixApp(&Args, szAppName);

	// if the initialize fails, we don't have icons, fonts, etc.
	if (!pMyUnixApp->initialize())
	{
		delete pMyUnixApp;
		return -1;	// make this something standard?
	}

	pMyUnixApp->ParseCommandLine();

	_showSplash(&Args, szAppName);

	// we just let it time out at 2 seconds, or let the user click
//	s_hideSplash(NULL);

	// turn over control to gtk
	gtk_main();
	
	// destroy the App.  It should take care of deleting all frames.

	pMyUnixApp->shutdown();
	delete pMyUnixApp;
	
	return 0;
}

void AP_UnixApp::ParseCommandLine(void)
{
	// parse the command line
	// <app> [-script <scriptname>]* [-dumpstrings] [-lib <AbiSuiteLibDirectory>] [<documentname>]*
	
	// TODO when we refactor the App classes, consider moving
	// TODO this to app-specific, cross-platform.

	// TODO replace this with getopt or something similar.
	
	// Unix puts the program name in argv[0], so [1] is the first argument.

	int nFirstArg = 1;
	int k;
	int kWindowsOpened = 0;
	
	for (k=nFirstArg; (k<m_pArgs->m_argc); k++)
	{
		if (*m_pArgs->m_argv[k] == '-')
		{
			if (UT_stricmp(m_pArgs->m_argv[k],"-script") == 0)
			{
				// [-script scriptname]
				k++;
			}
			else if (UT_stricmp(m_pArgs->m_argv[k],"-lib") == 0)
			{
				// [-lib <AbiSuiteLibDirectory>]
				// we've already processed this when we initialized the App class
				k++;
			}
			else if (UT_stricmp(m_pArgs->m_argv[k],"-dumpstrings") == 0)
			{
				// [-dumpstrings]
#ifdef DEBUG
				// dump the string table in english as a template for translators.
				// see abi/docs/AbiSource_Localization.abw for details.
				AP_BuiltinStringSet * pBuiltinStringSet = new AP_BuiltinStringSet(this,AP_PREF_DEFAULT_StringSet);
				pBuiltinStringSet->dumpBuiltinSet("EnUS.strings");
				delete pBuiltinStringSet;
#endif
			}
			else
			{
				UT_DEBUGMSG(("Unknown command line option [%s]\n",m_pArgs->m_argv[k]));
				// TODO don't know if it has a following argument or not -- assume not
			}
		}
		else
		{
			// [filename]
			
			AP_UnixFrame * pFirstUnixFrame = new AP_UnixFrame(this);
			pFirstUnixFrame->initialize();
			if (pFirstUnixFrame->loadDocument(m_pArgs->m_argv[k], IEFT_Unknown))
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
	}

	if (kWindowsOpened == 0)
	{
		// no documents specified or were able to be opened, open an untitled one

		AP_UnixFrame * pFirstUnixFrame = new AP_UnixFrame(this);
		pFirstUnixFrame->initialize();
		pFirstUnixFrame->loadDocument(NULL, IEFT_Unknown);
	}

	return;
}
