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

#include <Pt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <popt.h>


#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_misc.h"

#include "ut_Script.h"
#include "ut_PerlBindings.h"

#include "xap_Args.h"
#include "ap_Args.h"
#include "ap_Convert.h"
#include "ap_QNXFrame.h"
#include "ap_QNXApp.h"
#include "sp_spell.h"
#include "ap_Strings.h"
#include "xap_EditMethods.h"
#include "ap_LoadBindings.h"
#include "xap_DialogFactory.h"
#include "xap_Dlg_MessageBox.h"
#include "xap_Dialog_Id.h"
#include "xap_Menu_ActionSet.h"
#include "xap_Menu_Layouts.h"
#include "xap_Toolbar_ActionSet.h"
#include "xap_ModuleManager.h"
#include "xap_Module.h"
#include "xav_View.h"


#include "gr_Graphics.h"
#include "gr_QNXGraphics.h"
#include "gr_Image.h"
#include "ut_bytebuf.h"
#include "ut_png.h"
#include "ut_debugmsg.h"
#include "ut_qnxHelper.h"

#include "fv_View.h"
#include "fp_Run.h"

#include "ut_string_class.h"
#include "xap_EncodingManager.h"

#include "ie_impexp_Register.h"

#include "ie_exp.h"
#include "ie_exp_RTF.h"
#include "ie_exp_Text.h"

#include "ie_imp.h"
#include "ie_imp_RTF.h"
#include "ie_imp_Text.h"

#ifdef HAVE_CURL
#include "ap_QNXHashDownloader.h"
#endif

void signalWrapper(int sig_num);

/*****************************************************************/

AP_QNXApp::AP_QNXApp(XAP_Args * pArgs, const char * szAppName)
	: AP_App(pArgs,szAppName)
{
	m_prefs = NULL;
	m_pStringSet = NULL;
	m_pClipboard = NULL;


#ifdef HAVE_CURL
	m_pHashDownloader = (XAP_HashDownloader*)(new AP_QNXHashDownloader());
#endif
}

AP_QNXApp::~AP_QNXApp(void)
{

	DELETEP(m_prefs);
	DELETEP(m_pStringSet);
	DELETEP(m_pClipboard);
	
#ifdef HAVE_CURL
	DELETEP(m_pHashDownloader);
#endif
	IE_ImpExp_UnRegisterXP ();
}

static bool s_createDirectoryIfNecessary(const char * szDir)
{
	struct stat statbuf;
	
	if (stat(szDir,&statbuf) == 0)								// if it exists
	{
		if (S_ISDIR(statbuf.st_mode))							// and is a directory
			return true;

		UT_DEBUGMSG(("Pathname [%s] is not a directory.\n",szDir));
		return false;
	}
	
	if (mkdir(szDir,0700) == 0)
		return true;
	

	UT_DEBUGMSG(("Could not create Directory [%s].\n",szDir));
	return false;
}	

bool AP_QNXApp::initialize(void)
{
	const char * szUserPrivateDirectory = getUserPrivateDirectory();
	bool bVerified;
	bVerified = s_createDirectoryIfNecessary(szUserPrivateDirectory);
	
	// load the preferences.

	m_prefs = new AP_QNXPrefs(this);
	m_prefs->fullInit();
	
	// now that preferences are established, let the xap init
		   
	m_pClipboard = new AP_QNXClipboard(this);
	UT_ASSERT(m_pClipboard);
	m_pClipboard->initialize();
	   
	m_pEMC = AP_GetEditMethods();
	UT_ASSERT(m_pEMC);

	m_pBindingSet = new AP_BindingSet(m_pEMC);
	UT_ASSERT(m_pBindingSet);
	
	m_pMenuActionSet = AP_CreateMenuActionSet();
	UT_ASSERT(m_pMenuActionSet);

	m_pToolbarActionSet = AP_CreateToolbarActionSet();
	UT_ASSERT(m_pToolbarActionSet);

	if (! XAP_QNXApp::initialize())
		return false;
	
	//////////////////////////////////////////////////////////////////
	// Initialize the importers/exporters
	//////////////////////////////////////////////////////////////////
	IE_ImpExp_RegisterXP();

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
			getPrefsValueDirectory(true,AP_PREF_KEY_StringSetDirectory,&szDirectory);
			UT_ASSERT((szDirectory) && (*szDirectory));

			char * szPathname = (char *)UT_calloc(sizeof(char),strlen(szDirectory)+strlen(szStringSet)+100);
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

	// Now we have the strings loaded we can populate the field names correctly
	// CHECK THIS: the following was added by a Linux developer who can't test
	// on QNX, or even compile, so someone with a QNX box (or fridge? ;-)) needs to check it
	int i;
	
	for (i = 0; fp_FieldTypes[i].m_Type != FPFIELDTYPE_END; i++)
	{
			(&fp_FieldTypes[i])->m_Desc = strdup(m_pStringSet->getValueUTF8(fp_FieldTypes[i].m_DescId).c_str());
	    UT_DEBUGMSG(("Setting field type desc for type %d, desc=%s\n", fp_FieldTypes[i].m_Type, fp_FieldTypes[i].m_Desc));
	}

	for (i = 0; fp_FieldFmts[i].m_Tag != NULL; i++)
	{
			(&fp_FieldFmts[i])->m_Desc = strdup(m_pStringSet->getValueUTF8(fp_FieldFmts[i].m_DescId).c_str());
	    UT_DEBUGMSG(("Setting field desc for field %s, desc=%s\n", fp_FieldFmts[i].m_Tag, fp_FieldFmts[i].m_Desc));
	}

	//////////////////////////////////////////////////////////////////

    ///////////////////////////////////////////////////////////////////////
    /// Build a labelset so the plugins can add themselves to something ///
    ///////////////////////////////////////////////////////////////////////

	const char * szMenuLabelSetName = NULL;
	if (getPrefsValue( AP_PREF_KEY_StringSet, (const XML_Char**)&szMenuLabelSetName) && (szMenuLabelSetName) && (*szMenuLabelSetName))
	{
	}
	else
		szMenuLabelSetName = AP_PREF_DEFAULT_StringSet;

	getMenuFactory()->buildMenuLabelSet(szMenuLabelSetName);

	bool bLoadPlugins = true;
	bool bFound = getPrefsValueBool(XAP_PREF_KEY_AutoLoadPlugins,&bLoadPlugins);
	if(bLoadPlugins || !bFound)
	{
		loadAllPlugins();
	}

#ifdef ABI_OPT_PERL
    // hack to keep the perl bindings working on unix
    UT_ScriptLibrary& instance = UT_ScriptLibrary::instance(); 
    instance.registerScript(new UT_PerlScriptSniffer());
#endif

	return true;
}

XAP_Frame * AP_QNXApp::newFrame(void)
{
	AP_QNXFrame * pQNXFrame = new AP_QNXFrame(this);

	if (pQNXFrame)
		pQNXFrame->initialize();

	return pQNXFrame;
}

bool AP_QNXApp::shutdown(void)
{
	if (m_prefs->getAutoSavePrefs()) {
		m_prefs->savePrefsFile();
	}

	return true;
}

void AP_QNXApp::reallyExit(void)
{
	shutdown();
	XAP_QNXApp::reallyExit();
}

bool AP_QNXApp::getPrefsValueDirectory(bool bAppSpecific,
										   const XML_Char * szKey, const XML_Char ** pszValue) const
{
	if (!m_prefs)
		return false;

	const XML_Char * psz = NULL;
	if (!m_prefs->getPrefsValue(szKey,&psz))
		return false;

	if (*psz == '/')
	{
		*pszValue = psz;
		return true;
	}

	const XML_Char * dir = ((bAppSpecific) ? getAbiSuiteAppDir() : getAbiSuiteLibDir());

	static XML_Char buf[1024];
	UT_ASSERT((strlen(dir) + strlen(psz) + 2) < sizeof(buf));
	
	sprintf(buf,"%s/%s",dir,psz);
	*pszValue = buf;
	return true;
}

const char * AP_QNXApp::getAbiSuiteAppDir(void) const
{
	// we return a static string, use it quickly.
	
	static XML_Char buf[1024];
	UT_ASSERT((strlen(getAbiSuiteLibDir()) + strlen(ABIWORD_APP_LIBDIR) + 2) < sizeof(buf));

	sprintf(buf,"%s/%s",getAbiSuiteLibDir(),ABIWORD_APP_LIBDIR);
	return buf;
}

const XAP_StringSet * AP_QNXApp::getStringSet(void) const
{
	return m_pStringSet;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
#define Ph_CLIPBOARD_RTF "RTF"
#define Ph_CLIPBOARD_TEXT "TEXT"

void AP_QNXApp::copyToClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard)
{
	UT_ByteBuf rtfbuf, txtbuf;

	IE_Exp_RTF * pExpRtf = new IE_Exp_RTF(pDocRange->m_pDoc);
	if (pExpRtf)
	{
		pExpRtf->copyToBuffer(pDocRange,&rtfbuf);
		DELETEP(pExpRtf);
	}
	// put raw 8bit text on the clipboard
	IE_Exp_Text * pExpText = new IE_Exp_Text(pDocRange->m_pDoc);
	if (pExpText)
	{
		pExpText->copyToBuffer(pDocRange,&txtbuf);
		DELETEP(pExpText);
	}
	UT_Byte b = 0;
	if(rtfbuf.getLength() > 0){
		rtfbuf.append(&b,1);			// null terminate string
		UT_DEBUGMSG(("CopyToClipboard: copying %d bytes in RTF format.",rtfbuf.getLength()));
		m_pClipboard->addData(Ph_CLIPBOARD_RTF,(UT_Byte *)rtfbuf.getPointer(0),rtfbuf.getLength());
	}
	if(txtbuf.getLength() > 0){
		txtbuf.append(&b,1);			// null terminate string
		UT_DEBUGMSG(("CopyToClipboard: copying %d bytes in TEXTPLAIN format.\n",txtbuf.getLength()));
		m_pClipboard->addData(Ph_CLIPBOARD_TEXT,(UT_Byte *)txtbuf.getPointer(0),txtbuf.getLength());		
	}

	return;
}

void AP_QNXApp::pasteFromClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard, 
									bool bHonorFormatting)
{
	// paste from the system clipboard using the best-for-us format
	// that is present.
	unsigned char * pData = NULL;
	UT_uint32 iLen=0;

	if (bHonorFormatting && m_pClipboard->getClipboardData(Ph_CLIPBOARD_RTF,(void**)&pData,&iLen)) {
		iLen = UT_MIN(iLen,strlen((const char *) pData));
		UT_DEBUGMSG(("PasteFromClipboard: pasting %d bytes in RTF format.\n",iLen));
		IE_Imp_RTF * pImpRTF = new IE_Imp_RTF(pDocRange->m_pDoc);
		pImpRTF->pasteFromBuffer(pDocRange,pData,iLen);
		DELETEP(pImpRTF);
	}
	else if (m_pClipboard->getClipboardData(Ph_CLIPBOARD_TEXT,(void**)&pData,&iLen)) {
		iLen = UT_MIN(iLen,strlen((const char *) pData));
		UT_DEBUGMSG(("PasteFromClipboard: pasting %d bytes in TEXTPLAIN format.\n",iLen));
		IE_Imp_Text * pImpText = new IE_Imp_Text(pDocRange->m_pDoc);
		pImpText->pasteFromBuffer(pDocRange,pData,iLen);
		DELETEP(pImpText);
	}
	else {
		// TODO figure out what to do with an image....
		UT_DEBUGMSG(("PasteFromClipboard: No TEXT or RTF data in clipboard. TODO: Add Image support"));
	}
	FREEP(pData);
	return;
}

bool AP_QNXApp::canPasteFromClipboard(void)
{
	return true; 
}

/*****************************************************************/
/*****************************************************************/

static void * wSplash = NULL;
static GR_Image * pSplashImage = NULL;
static GR_QNXGraphics * pQNXGraphics = NULL;
static bool firstExpose = false;
static UT_uint32 splashTimeoutValue = 0;

static int s_hideSplash(PtWidget_t *w, void *data, PtCallbackInfo_t *info)
{
	if (wSplash) {
		PtDestroyWidget((PtWidget_t *)wSplash);
		wSplash = NULL;
		DELETEP(pQNXGraphics);
		DELETEP(pSplashImage);
	}
	return Pt_CONTINUE;
}

static int s_drawingarea_expose(PtWidget_t *widget, PhTile_t *damage) {

	if (pQNXGraphics && pSplashImage) {
		pQNXGraphics->drawImage(pSplashImage, 0, 0);

		// on the first full paint of the image, start a 2 second timer
		if (!firstExpose) {
			PtArg_t args[1];
			PtSetArg(&args[0], Pt_ARG_TIMER_INITIAL, splashTimeoutValue, 0);
			PtWidget_t *timer = PtCreateWidget(PtTimer, widget, 1, args);
			PtAddCallback(timer, Pt_CB_TIMER_ACTIVATE, s_hideSplash, NULL);
			PtRealizeWidget(timer);
			firstExpose = true;
		}
	}

	return Pt_CONTINUE;
}

// szFile is optional; a NULL pointer will use the default splash screen.
// The delay is how long the splash should stay on screen in milliseconds.
static GR_Image * _showSplash(PtWidget_t *spwin, UT_uint32 delay)
{
	wSplash = spwin;
	pSplashImage = NULL;

	UT_ByteBuf* pBB = NULL;

	// use a default if they haven't specified anything
	const char * szFile = "splash.png";

	// store value for use by the expose event, which attaches the timer
	splashTimeoutValue = delay;
	
	extern unsigned char g_pngSplash[];		// see ap_wp_Splash.cpp
	extern unsigned long g_pngSplash_sizeof;	// see ap_wp_Splash.cpp

	pBB = new UT_ByteBuf();
	if ((pBB->insertFromFile(0, szFile)) || 
        (pBB->ins(0, g_pngSplash, g_pngSplash_sizeof)))
	{
		PtArg_t	args[10];
		int     n = 0;

		// get splash size
		UT_sint32 iSplashWidth;
		UT_sint32 iSplashHeight;
		UT_PNG_getDimensions(pBB, iSplashWidth, iSplashHeight);

		// create a centered window the size of our image
		PtSetArg(&args[n++], Pt_ARG_WIDTH, iSplashWidth, 0);
		PtSetArg(&args[n++], Pt_ARG_HEIGHT, iSplashHeight, 0);
		PtSetArg(&args[n++], Pt_ARG_WINDOW_RENDER_FLAGS, 
				 0, 
				Ph_WM_RENDER_RESIZE | Ph_WM_RENDER_TITLE | Ph_WM_RENDER_MENU);
		PtSetArg(&args[n++], Pt_ARG_WINDOW_MANAGED_FLAGS, 
				0,
				Ph_WM_CLOSE | Ph_WM_RESIZE | Ph_WM_HIDE | Ph_WM_MAX);
		PtSetResources(spwin, n, args);
		UT_QNXCenterWindow(NULL, spwin);

		// create a frame to add depth

		// create a drawing area
		n = 0;
		PtSetArg(&args[n++], Pt_ARG_WIDTH, iSplashWidth, 0);
		PtSetArg(&args[n++], Pt_ARG_HEIGHT, iSplashHeight, 0);
		//PtSetArg(&args[n++], Pt_ARG_USER_DATA, &data, sizeof(this)); 
		PtSetArg(&args[n++], Pt_ARG_RAW_DRAW_F, &s_drawingarea_expose, 1);
		PtWidget_t *da = PtCreateWidget(PtRaw, spwin, n, args);
		PtAddEventHandler(da, Ph_EV_BUT_RELEASE, s_hideSplash, NULL);

		// create image context
		// TODO: find an XAP_App pointer for the following call:
		pQNXGraphics = new GR_QNXGraphics(spwin, da, 0);
		pSplashImage = pQNXGraphics->createNewImage("splash", pBB, pQNXGraphics->tlu(iSplashWidth), pQNXGraphics->tlu(iSplashHeight));

		PtRealizeWidget(spwin);
	}

	DELETEP(pBB);

	return pSplashImage;
}

/*****************************************************************/
AP_QNXApp * gQNXApp = NULL; 
PtWidget_t	*gTimerWidget = NULL;

int AP_QNXApp::main(const char * szAppName, int argc, const char ** argv)
{
	// This is a static function.
		   
	UT_DEBUGMSG(("Build ID:\t%s\n", XAP_App::s_szBuild_ID));
	UT_DEBUGMSG(("Version:\t%s\n", XAP_App::s_szBuild_Version));
	UT_DEBUGMSG(("Build Options: \t%s\n", XAP_App::s_szBuild_Options));
	UT_DEBUGMSG(("Build Target: \t%s\n", XAP_App::s_szBuild_Target));
	UT_DEBUGMSG(("Compile Date:\t%s\n", XAP_App::s_szBuild_CompileDate));
	UT_DEBUGMSG(("Compile Time:\t%s\n", XAP_App::s_szBuild_CompileTime));

	// initialize our application.

	XAP_Args XArgs = XAP_Args(argc,argv);


	//TODO: Do a PtAppInit() here with the main window being the splash screen
	PtWidget_t *spwin;
	spwin = PtAppInit(NULL, NULL /* XArgs.m_argc */, NULL /* XArgs.m_argv */, 0, NULL);

	AP_QNXApp * pMyQNXApp = new AP_QNXApp(&XArgs, szAppName);
	AP_Args Args = AP_Args(&XArgs,szAppName,pMyQNXApp);
	pMyQNXApp->parsePoptOpts();

	// if the initialize fails, we don't have icons, fonts, etc.
	if (!pMyQNXApp->initialize())
	{
		delete pMyQNXApp;
		return -1;	// make this something standard?
	}

	//This is used by all the timer classes, and should probably be in the XAP contructor
	PtArg_t args[2];
	PtSetArg(&args[0], Pt_ARG_REGION_FIELDS, Ph_REGION_EV_SENSE, Ph_REGION_EV_SENSE);
	PtSetArg(&args[1], Pt_ARG_REGION_SENSE, Ph_EV_TIMER, Ph_EV_TIMER);
	PtSetParentWidget(NULL);
	gTimerWidget = PtCreateWidget(PtRegion, NULL, 2, args);
	PtRealizeWidget(gTimerWidget);

	
 // do we show the app&splash?
 bool bShowSplash = Args.getShowSplash();

 const XAP_Prefs * pPrefs = pMyQNXApp->getPrefs();
 UT_ASSERT(pPrefs);
 bool bSplashPref = true;
 if (pPrefs && 
	pPrefs->getPrefsValueBool (AP_PREF_KEY_ShowSplash, &bSplashPref))
  {
  	bShowSplash = bShowSplash && bSplashPref;
  }
	if (bShowSplash) {
		_showSplash(spwin, 2000);
	}
	else {
		PtDestroyWidget(spwin);

  if (!Args.doWindowlessArgs())
    return false;
	
    // Setup signal handlers, primarily for segfault
    // If we segfaulted before here, we *really* blew it

    struct sigaction sa;

    sa.sa_handler = signalWrapper;

    sigfillset(&sa.sa_mask);  // We don't want to hear about other signals
    sigdelset(&sa.sa_mask, SIGABRT); // But we will call abort(), so we can't ignore that

    sa.sa_flags = SA_NODEFER | SA_RESETHAND; // Don't handle nested signals

    sigaction(SIGSEGV, &sa, NULL);
    sigaction(SIGBUS, &sa, NULL);
    sigaction(SIGILL, &sa, NULL);
    sigaction(SIGQUIT, &sa, NULL);
    sigaction(SIGFPE, &sa, NULL);
    // TODO: handle SIGABRT

	}
	// this function takes care of all the command line args.
	// if some args are botched, it returns false and we should
	// continue out the door.
	// We used to check for bShowApp here.  It shouldn't be needed
	// anymore, because doWindowlessArgs was supposed to bail already. -PL
	if (pMyQNXApp->openCmdLineFiles(Args.poptcon))
	{
		PtMainLoop();
	}
	else
	{
		UT_DEBUGMSG(("Not parsing command line or showing app\n"));
	}
	
	// destroy the App.  It should take care of deleting all frames.
	pMyQNXApp->shutdown();
	delete pMyQNXApp;

	return 0;
}

/*** Signal handling functionality ***/
void signalWrapper(int sig_num)
{
	AP_QNXApp *pApp = (AP_QNXApp *) XAP_App::getApp();
	pApp->catchSignals(sig_num);
}

static int s_signal_count = 0;

void AP_QNXApp::catchSignals(int sig_num)
{
        // Reset the signal handler 
  // (not that it matters - this is mostly for race conditions)
        signal(SIGSEGV, signalWrapper);

        s_signal_count = s_signal_count + 1;
        if(s_signal_count > 1)
        {
                UT_DEBUGMSG(("Segfault during filesave - no file saved  \n"));
                fflush(stdout);
                abort();
        }

	UT_DEBUGMSG(("Oh no - we just segfaulted!\n"));

	UT_uint32 i = 0;
	for(;i<m_vecFrames.getItemCount();i++)
	{
		AP_QNXFrame * curFrame = (AP_QNXFrame*) m_vecFrames[i];
		UT_ASSERT(curFrame);
		curFrame->backup();
	}

	fflush(stdout);

	// Abort and dump core
	abort();
}

/* Taken from unix july 1'th.
 * Updated similarly July 20th (fjf)
 */
static int so_only (struct dirent *d)
{
	const char * name = d->d_name;
	
	if ( name )
	{
		int len = strlen (name);
		
		if (len >= 3)
		{
			if(!strcmp(name+(len-3), ".so"))
    return 1;
		}
	}
	return 0;
}

void AP_QNXApp::loadAllPlugins ()
{
  struct direct **namelist;
  int n = 0;

  UT_String pluginList[2];
  UT_String pluginDir;

  // the global plugin directory
  pluginDir = getAbiSuiteAppDir();
  pluginDir += "/plugins/";
  pluginList[0] = pluginDir;

  // the user-local plugin directory
  pluginDir = getUserPrivateDirectory ();
  pluginDir += "/AbiWord/plugins/";
  pluginList[1] = pluginDir;

  for(UT_uint32 i = 0; i < (sizeof(pluginList)/sizeof(pluginList[0])); i++)
  {
      pluginDir = pluginList[i];

      n = scandir((char*)pluginDir.c_str(), &namelist, so_only, alphasort);
      UT_DEBUGMSG(("DOM: found %d plugins in %s\n", n, pluginDir.c_str()));

      if (n > 0)
	  {
		  while(n--) 
		  {
			  UT_String plugin (pluginDir + namelist[n]->d_name);

			  UT_DEBUGMSG(("DOM: loading plugin %s\n", plugin.c_str()));

			  int len = strlen (namelist[n]->d_name);
			  if (len < 4)
			  {
				  UT_DEBUGMSG(("FJF: bad name for a plugin\n"));
				  free(namelist[n]);
				  continue;
			  }
			  if(strcmp (namelist[n]->d_name+(len-3), ".so") != 0)
			  {
				  UT_DEBUGMSG(("FJF: not really a plugin?\n"));
				  free(namelist[n]);
				  continue;
			  }

			  if (XAP_ModuleManager::instance().loadModule (plugin.c_str()))
			  {
				  UT_DEBUGMSG(("DOM: loaded plugin: %s\n", namelist[n]->d_name));
			  }
			  else
			  {
				  UT_DEBUGMSG(("DOM: didn't load plugin: %s\n", namelist[n]->d_name));
			  }
			  free(namelist[n]);
		  }
		  free(namelist);
      }
  }

  /* SPI modules don't register automatically on loading, so
   * now that we've loaded the modules we need to register them:
   */
  XAP_ModuleManager::instance().registerPending ();
}

void AP_QNXApp::errorMsgBadArg(AP_Args *Args,int nextopt)
{
  printf ("Error on option %s: %s.\nRun '%s --help' to see a full list of available command line options.\n",
	  poptBadOption (Args->poptcon, 0),
	  poptStrerror (nextopt),
	  Args->XArgs->m_argv[0]);
}

void AP_QNXApp::errorMsgBadFile(XAP_Frame * pFrame, const char * file, 
				 UT_Error error)
{
}

bool AP_QNXApp::doWindowlessArgs(const AP_Args *Args)
{

return false;
}

XAP_Frame * AP_QNXApp::newFrame(AP_App * app)
{
  AP_QNXFrame * pFrame = new AP_QNXFrame(app);
  if (pFrame)
    pFrame->initialize();
  
  return pFrame;
}


