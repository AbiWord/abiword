/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiSource Application Framework
 * Copyright (C) 2004, 2012 Hubert Figuiere
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#include "xap_ModuleManager.h"
#include "ap_Args.h"
#include "ap_QtApp.h"
#include "ap_QtFrame.h"

#include "ie_impexp_Register.h"

// TODO move this in a compat layer

static int s_signal_count = 0;

/*!
  This is a global function to call our signal handler.  It needs to
  be global so that we can pass a function pointer to it to C code
  that handles signals.  
  \todo Could this be a static member function?
  JCA: No, but it can be extern "C" { static void signalWrapper(int) }
  JCA: (well, there is a way to use a static member function and to remain
  JCA: correct, but it's a bit cumbersome.)
*/
void signalWrapper(int sig_num)
{
    AP_QtApp *pApp = static_cast<AP_QtApp *>(XAP_App::getApp());

	/* make sure we have application, in case we have been called after
	 * the application object is gone
	 */
	if (pApp)
		pApp->catchSignals(sig_num);
}

/*!
  This function actually handles signals.  The most commonly recieved
  one is SIGSEGV, the segfault signal.  We want to clean up, save the
  user's files to backup locations (currently <filename>.saved) and then
  call abort, so we still get a core dump that we can debug.
  \param sig_num the integer representing which signal we recieved
*/
void AP_QtApp::catchSignals(int /*sig_num*/)
{
    // Reset the signal handler 
    // (not that it matters - this is mostly for race conditions)
    signal(SIGSEGV, signalWrapper);

    s_signal_count = s_signal_count + 1;
    if(s_signal_count > 1)
    {
		UT_DEBUGMSG(("Crash during filesave - no file saved\n"));
		fflush(stdout);
		abort();
    }
    
    UT_DEBUGMSG(("Oh no - we just crashed!\n"));
//
// fixme: Enable this to help debug the bonobo component. After a crash the
// the program hangs here and you can connect to it from gdb and backtrace to
// the crash point
#ifdef LOGFILE
	fprintf(logfile,"abicrashed \n");
	fclose(logfile);
#endif

#if 0
	while(1)
	{
		UT_usleep(10000);
	}
#endif

#warning TODO implement

#if 0 // TODO implement
    UT_sint32 i = 0;
	IEFileType abiType = IE_Imp::fileTypeForSuffix(".abw");
    for(;i<m_vecFrames.getItemCount();i++)
    {
		AP_QtFrame * curFrame = const_cast<AP_QtFrame*>(static_cast<const AP_QtFrame*>(m_vecFrames[i]));
		UT_continue_if_fail(curFrame);
		if (NULL == curFrame->getFilename())
		  curFrame->backup(".abw.saved",abiType);
		else
		  curFrame->backup(".saved",abiType);
    }
#endif

    fflush(stdout);
 
    // Abort and dump core
    abort();
}

AP_QtApp::AP_QtApp(const char * szAppName)
	: AP_App(szAppName)
	, m_pStringSet(NULL)
{
}

AP_QtApp::~AP_QtApp()
{
	delete m_pStringSet;

    IE_ImpExp_UnRegisterXP ();
}

bool AP_QtApp::initialize(bool has_display)
{
#warning TODO
// refactor stuff
	return true;
}

/*
 * TODO Should we refactor this too?
 * Seems to be same code everywhere
 */
XAP_Frame* AP_QtApp::newFrame(void)
{
    AP_QtFrame * pQtFrame = new AP_QtFrame();

    if (pQtFrame) {
		pQtFrame->initialize();
	}

    return pQtFrame;
}

GR_Graphics *AP_QtApp::newDefaultScreenGraphics() const
{
#warning TODO implement
}

const XAP_StringSet *AP_QtApp::getStringSet(void) const
{
    return m_pStringSet;
}

const char *AP_QtApp::getAbiSuiteAppDir(void) const
{
#warning TODO implement
}

void AP_QtApp::copyToClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard)
{
#warning TODO implement
}

void AP_QtApp::pasteFromClipboard(PD_DocumentRange * pDocRange, bool bUseClipboard, bool bHonorFormatting)
{
#warning TODO implement
}

bool AP_QtApp::canPasteFromClipboard(void)
{
#warning TODO implement
}

void AP_QtApp::cacheCurrentSelection(AV_View *)
{
#warning TODO implement
}

bool AP_QtApp::shutdown(void)
{
#warning TODO prefs
//	if (m_prefs->getAutoSavePrefs())
//		m_prefs->savePrefsFile();
	return true;
}

int AP_QtApp::main(const char * szAppName, int argc, char ** argv)
{

    // initialize our application.
	int exit_status = 0;
	AP_QtApp * pMyQtApp = new AP_QtApp(szAppName);

#if 0
//#ifdef WITH_CHAMPLAIN
    gtk_clutter_init (&argc, &argv);
#endif

	/* this brace is here to ensure that our local variables on the stack
	 * do not outlive the application object by giving them a lower scope
	 */
	{
		XAP_Args XArgs = XAP_Args(argc, argv);
		AP_Args Args = AP_Args(&XArgs, szAppName, pMyQtApp);
#ifdef LOGFILE
		UT_String sLogFile = pMyQtApp->getUserPrivateDirectory();
		sLogFile += "abiLogFile";
		logfile = fopen(sLogFile.c_str(),"a+");
		fprintf(logfile,"About to do gtk_set_locale \n");
		fprintf(logfile,"New logfile \n");
#endif
		// Step 1: Initialize GTK and create the APP.
		// hack needed to intialize gtk before ::initialize
		setlocale(LC_ALL, "");

		bool have_display = true;
#if 0 // GTK to port?
		have_display = gtk_init_check(&argc, &argv);
#ifdef LOGFILE
		fprintf(logfile,"Got display %d \n",have_display);
		fprintf(logfile,"Really display %d \n",have_display);
#endif
		if (have_display > 0) {
			Args.addOptions(gtk_get_option_group(TRUE));
			Args.parseOptions();
		}
		else {
			// no display, but we still need to at least parse our own arguments, damnit, for --to, --to-png, and --print
			Args.addOptions(gtk_get_option_group(FALSE));
			Args.parseOptions();
		}
#endif

		// if the initialize fails, we don't have icons, fonts, etc.
		if (!pMyQtApp->initialize(have_display))
		{
			delete pMyQtApp;
			return -1;	// make this something standard?
		}

		// Setup signal handlers, primarily for segfault
		// If we segfaulted before here, we *really* blew it
		struct sigaction sa;
		sa.sa_handler = signalWrapper;

		sigfillset(&sa.sa_mask);  // We don't want to hear about other signals
		sigdelset(&sa.sa_mask, SIGABRT); // But we will call abort(), so we can't ignore that
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

		// TODO: handle SIGABRT

		// Step 2: Handle all non-window args.

		bool windowlessArgsWereSuccessful = true;
		if (!Args.doWindowlessArgs(windowlessArgsWereSuccessful )) {
			delete pMyQtApp;
			return (windowlessArgsWereSuccessful ? 0 : -1);
		}

		if (have_display) {

			// Step 3: Create windows as appropriate.
			// if some args are botched, it returns false and we should
			// continue out the door.
			// We used to check for bShowApp here.  It shouldn't be needed
			// anymore, because doWindowlessArgs was supposed to bail already. -PL

			if (pMyQtApp->openCmdLineFiles(&Args))
			{
				// turn over control to Qt
				pMyQtApp->exec();
			}
			else
			{
				UT_DEBUGMSG(("DOM: not parsing command line or showing app\n"));
			}
		}
		else {
			fprintf(stderr, "No DISPLAY: this may not be what you want.\n");
			exit_status = 1;
		}
		// unload all loaded plugins (remove some of the memory leaks shown at shutdown :-)
		XAP_ModuleManager::instance().unloadAllPlugins();

		// Step 4: Destroy the App.  It should take care of deleting all frames.
		pMyQtApp->shutdown();
	}
	
	delete pMyQtApp;
	
	return exit_status;
}
