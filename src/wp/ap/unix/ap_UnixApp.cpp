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
#include "ut_getopt.h"

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

#include "ap_Clipboard.h"

/*****************************************************************/

AP_UnixApp::AP_UnixApp(XAP_Args * pArgs, const char * szAppName)
	: XAP_UnixApp(pArgs,szAppName)
{
	m_prefs = NULL;
	m_pStringSet = NULL;
	m_pClipboard = NULL;
}

AP_UnixApp::~AP_UnixApp(void)
{
	SpellCheckCleanup();

	DELETEP(m_prefs);
	DELETEP(m_pStringSet);
	DELETEP(m_pClipboard);
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
		   
	m_pClipboard = new AP_UnixClipboard();
	UT_ASSERT(m_pClipboard);
	   
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
		getPrefsValueDirectory(UT_FALSE,AP_PREF_KEY_SpellDirectory,&szISpellDirectory);
		UT_ASSERT((szISpellDirectory) && (*szISpellDirectory));

		const char * szSpellCheckWordList = NULL;
		getPrefsValue(AP_PREF_KEY_SpellCheckWordList,&szSpellCheckWordList);
		UT_ASSERT((szSpellCheckWordList) && (*szSpellCheckWordList));
		
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
			getPrefsValueDirectory(UT_TRUE,AP_PREF_KEY_StringSetDirectory,&szDirectory);
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

UT_Bool AP_UnixApp::getPrefsValueDirectory(UT_Bool bAppSpecific,
										   const XML_Char * szKey, const XML_Char ** pszValue) const
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

	const XML_Char * dir = ((bAppSpecific) ? getAbiSuiteAppDir() : getAbiSuiteLibDir());

	static XML_Char buf[1024];
	UT_ASSERT((strlen(dir) + strlen(psz) + 2) < sizeof(buf));
	
	sprintf(buf,"%s/%s",dir,psz);
	*pszValue = buf;
	return UT_TRUE;
}

const char * AP_UnixApp::getAbiSuiteAppDir(void) const
{
	// we return a static string, use it quickly.
	
	static XML_Char buf[1024];
	UT_ASSERT((strlen(getAbiSuiteLibDir()) + strlen(ABIWORD_APP_LIBDIR) + 2) < sizeof(buf));

	sprintf(buf,"%s/%s",getAbiSuiteLibDir(),ABIWORD_APP_LIBDIR);
	return buf;
}

const XAP_StringSet * AP_UnixApp::getStringSet(void) const
{
	return m_pStringSet;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

void AP_UnixApp::copyToClipboard(PD_DocumentRange * pDocRange)
{
	// copy the given subset of the given document to the
	// system clipboard in a variety of formats.

	if (!m_pClipboard->open())
		return;
	
	m_pClipboard->clear();
	
	{
		// put raw 8bit text on the clipboard
		
		IE_Exp_Text * pExpText = new IE_Exp_Text(pDocRange->m_pDoc);
		if (pExpText)
		{
			UT_ByteBuf buf;
			IEStatus status = pExpText->copyToBuffer(pDocRange,&buf);

			// NOTE: MS Docs state that for CF_TEXT and CF_OEMTEXT we must have a zero
			// NOTE: on the end of the buffer -- that's how they determine how much text
			// NOTE: that we have.
			UT_Byte b = 0;
			buf.append(&b,1);
			
			m_pClipboard->addData(AP_CLIPBOARD_TEXTPLAIN_8BIT,(UT_Byte *)buf.getPointer(0),buf.getLength());
			DELETEP(pExpText);
			UT_DEBUGMSG(("CopyToClipboard: copying %d bytes in TEXTPLAIN format.\n",buf.getLength()));
		}

		// also put RTF on the clipboard
		
		IE_Exp_RTF * pExpRtf = new IE_Exp_RTF(pDocRange->m_pDoc);
		if (pExpRtf)
		{
			UT_ByteBuf buf;
			IEStatus status = pExpRtf->copyToBuffer(pDocRange,&buf);
			m_pClipboard->addData(AP_CLIPBOARD_RTF,(UT_Byte *)buf.getPointer(0),buf.getLength());
			DELETEP(pExpRtf);
			UT_DEBUGMSG(("CopyToClipboard: copying %d bytes in RTF format.\n",buf.getLength()));
		}

#if 0
		// also put our format on the clipboard
		
		IE_Exp_AbiWord_1 * pExpAbw = new IE_Exp_AbiWord_1(pDocRange->m_pDoc);
		if (pExpAbw)
		{
			UT_ByteBuf buf;
			IEStatus status = pExpAbw->copyToBuffer(pDocRange,&buf);
			m_pClipboard->addData(AP_CLIPBOARD_ABIWORD_1,(UT_Byte *)buf.getPointer(0),buf.getLength());
			DELETEP(pExpAbw);
			UT_DEBUGMSG(("CopyToClipboard: copying %d bytes in ABIWORD_1 format.\n",buf.getLength()));
		}

		// TODO on NT, do we need to put unicode text on the clipboard ??
		// TODO do we need to put HTML on the clipboard ??
#endif
	}

	m_pClipboard->close();
}

void AP_UnixApp::pasteFromClipboard(PD_DocumentRange * pDocRange)
{
	// paste from the system clipboard using the best-for-us format
	// that is present.
	
	if (!m_pClipboard->open())
		return;
	
	{
		// TODO decide what the proper order is for these.

#if 0
		if (m_pClipboard->hasFormat(AP_CLIPBOARD_ABIWORD_1))
		{
			UT_uint32 iLen = m_pClipboard->getDataLen(AP_CLIPBOARD_ABIWORD_1);
			UT_DEBUGMSG(("PasteFromClipboard: pasting %d bytes in ABIWORD_1 format.\n",iLen));
			unsigned char * pData = new unsigned char[iLen+1];
			memset(pData,0,iLen+1);
			m_pClipboard->getData(AP_CLIPBOARD_ABIWORD_1,pData);
			IE_Imp_AbiWord_1 * pImpAbw = new IE_Imp_AbiWord_1(pDocRange->m_pDoc);
			pImpAbw->pasteFromBuffer(pDocRange,pData,iLen);
			DELETEP(pImpAbw);
			DELETEP(pData);
			goto MyEnd;
		}
#endif

		if (m_pClipboard->hasFormat(AP_CLIPBOARD_RTF))
		{
			UT_uint32 iLen = m_pClipboard->getDataLen(AP_CLIPBOARD_RTF);
			UT_DEBUGMSG(("PasteFromClipboard: pasting %d bytes in RTF format.\n",iLen));
			unsigned char * pData = new unsigned char[iLen+1];
			memset(pData,0,iLen+1);
			m_pClipboard->getData(AP_CLIPBOARD_RTF,pData);
			IE_Imp_RTF * pImpRTF = new IE_Imp_RTF(pDocRange->m_pDoc);
			pImpRTF->pasteFromBuffer(pDocRange,pData,iLen);
			DELETEP(pImpRTF);
			DELETEP(pData);
			goto MyEnd;
		}

		if (m_pClipboard->hasFormat(AP_CLIPBOARD_TEXTPLAIN_8BIT))
		{
			UT_uint32 iLen = m_pClipboard->getDataLen(AP_CLIPBOARD_TEXTPLAIN_8BIT);
			UT_DEBUGMSG(("PasteFromClipboard: pasting %d bytes in TEXTPLAIN format.\n",iLen));
			unsigned char * pData = new unsigned char[iLen+1];
			memset(pData,0,iLen+1);
			m_pClipboard->getData(AP_CLIPBOARD_TEXTPLAIN_8BIT,pData);
			IE_Imp_Text * pImpText = new IE_Imp_Text(pDocRange->m_pDoc);
			// NOTE: MS Docs state that the terminating zero on the string buffer is 
			// NOTE: included in the length for CF_TEXT and CF_OEMTEXT, so we compensate
			// NOTE: for it here.
			if (pData[iLen-1]==0)
				iLen--;
			pImpText->pasteFromBuffer(pDocRange,pData,iLen);
			DELETEP(pImpText);
			DELETEP(pData);
			goto MyEnd;
		}

		// TODO figure out what to do with an image....
		UT_DEBUGMSG(("PasteFromClipboard: TODO support this format..."));
	}

MyEnd:
	m_pClipboard->close();
	return;
}

UT_Bool AP_UnixApp::canPasteFromClipboard(void)
{
	if (!m_pClipboard->open())
		return UT_FALSE;

#if 0
	if (m_pClipboard->hasFormat(AP_CLIPBOARD_ABIWORD_1))
		goto ReturnTrue;
#endif
	if (m_pClipboard->hasFormat(AP_CLIPBOARD_RTF))
		goto ReturnTrue;
	if (m_pClipboard->hasFormat(AP_CLIPBOARD_TEXTPLAIN_8BIT))
		goto ReturnTrue;

	m_pClipboard->close();
	return UT_FALSE;

ReturnTrue:
	m_pClipboard->close();
	return UT_TRUE;
}

/*****************************************************************/
/*****************************************************************/

static GtkWidget * wSplash = NULL;
static GR_Image * pSplashImage = NULL;
static GR_UnixGraphics * pUnixGraphics = NULL;
static guint timeout_handler = 0;
static UT_Bool firstExpose = FALSE;
static UT_uint32 splashTimeoutValue = 0;

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
			timeout_handler = gtk_timeout_add(splashTimeoutValue, s_hideSplash, NULL);
		}
	}

	return FALSE;
}

// szFile is optional; a NULL pointer will use the default splash screen.
// The delay is how long the splash should stay on screen in milliseconds.
static GR_Image * _showSplash(UT_uint32 delay)
{
	wSplash = NULL;
	pSplashImage = NULL;

	UT_ByteBuf* pBB = NULL;

	// use a default if they haven't specified anything
	const char * szFile = "splash.png";

	// store value for use by the expose event, which attaches the timer
	splashTimeoutValue = delay;
	
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

	// Do a quick and dirty find for "-nosplash"
	UT_Bool bShowSplash = UT_TRUE;
	for (int k = 1; k < Args.m_argc; k++)
		if (*Args.m_argv[k] == '-')
			if (UT_stricmp(Args.m_argv[k],"-nosplash") == 0)
			{
				bShowSplash = UT_FALSE;
				break;
			}

	// HACK : these calls to gtk reside properly in XAP_UnixApp::initialize(),
	// HACK : but need to be here to throw the splash screen as
	// HACK : soon as possible.
	gtk_set_locale();
	gtk_init(&Args.m_argc,&Args.m_argv);
	
	if (bShowSplash)
		_showSplash(2000);
			
	AP_UnixApp * pMyUnixApp = new AP_UnixApp(&Args, szAppName);

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

UT_Bool AP_UnixApp::parseCommandLine(void)
{
	// parse the command line
	// <app> [-script <scriptname>]* [-dumpstrings] [<documentname>]*
        
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
			else if (UT_stricmp(m_pArgs->m_argv[k],"-nosplash") == 0)
			{
				// we've alrady processed this before we initialized the App class
			}
			else if (UT_stricmp(m_pArgs->m_argv[k],"-geometry") == 0)
			{
				// [-geometry <X geometry string>]

				// let us at the next argument
				k++;
				
				// TODO : does X have a dummy geometry value reserved for this?
				gint dummy = 1 << ((sizeof(gint) * 8) - 1);
				gint x = dummy;
				gint y = dummy;
				guint width = 0;
				guint height = 0;
			
				XParseGeometry(m_pArgs->m_argv[k], &x, &y, &width, &height);

				// use both by default
				XAP_UnixApp::windowGeometryFlags f = (XAP_UnixApp::windowGeometryFlags)
					(XAP_UnixApp::GEOMETRY_FLAG_SIZE
					 | XAP_UnixApp::GEOMETRY_FLAG_POS);

				// if pos (x and y) weren't provided just use size
				if (x == dummy || y == dummy)
					f = XAP_UnixApp::GEOMETRY_FLAG_SIZE;

				// if size (width and height) weren't provided just use pos
				if (width == 0 || height == 0)
					f = XAP_UnixApp::GEOMETRY_FLAG_POS;
			
				// set the xap-level geometry for future frame use
				setGeometry(x, y, width, height, f);
			}
			else
			{
				UT_DEBUGMSG(("Unknown command line option [%s]\n",m_pArgs->m_argv[k]));
				// TODO don't know if it has a following argument or not -- assume not

				_printUsage();
				return UT_FALSE;
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

	return UT_TRUE;
}

// TODO : MOVE THIS TO XP CODE!  This is a cut & paste job since each
// TODO : platform _can_ have different options, and we didn't sort
// TODO : out how to honor them correclty yet.  There is a copy of
// TODO : this function in other platforms.

void AP_UnixApp::_printUsage(void)
{
	// just print to stdout, not stderr
	printf("\nUsage: %s [option]... [file]...\n\n", m_pArgs->m_argv[0]);

#ifdef DEBUG
	printf("  -dumpstrings      dump strings strings to file\n");
#endif
	printf("  -geometry geom    set initial frame geometry\n");
	printf("  -lib dir          use dir for application components\n");
	printf("  -nosplash         do not show splash screen\n");
#ifdef ABI_OPT_JS
	printf("  -script file      execute file as script\n");
#endif

	printf("\n");
}

