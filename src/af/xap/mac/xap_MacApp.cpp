/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 1999 John Brewer DBA Jera Design
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
/*
	Platform: MacOS Classic & Carbon
	
	$Id$	
*/

#include <string.h>

/* MacOS includes */
#ifndef XP_MAC_TARGET_QUARTZ
# include <QuickDraw.h>
#endif
#include <Folders.h>
#include <AppleEvents.h>
#include <MacWindows.h>
#include <Menus.h>
#include <Devices.h>
#include <Events.h>
#ifdef XP_MAC_TARGET_MACOSX
# include <AEInteraction.h>
#endif

/* end */

#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_MacFiles.h"
#include "xap_Args.h"
#include "xap_MacApp.h"
#include "xap_MacClipboard.h"
#include "xap_MacFrame.h"
#include "xap_MacTlbr_Icons.h"
#include "xap_MacTlbr_ControlFactory.h"
#include "ev_MacMenu.h"


bool XAP_MacApp::m_NotInitialized = true;

/*****************************************************************/

XAP_MacApp::XAP_MacApp(XAP_Args * pArgs, const char * szAppName)
	: XAP_App(pArgs, szAppName), m_dialogFactory(this), m_controlFactory()
{
	if (m_NotInitialized) {
		InitializeMacToolbox ();
	}

	m_finished = false;
	m_pMacToolbarIcons = 0;
}

XAP_MacApp::~XAP_MacApp(void)
{
	DELETEP(m_pMacToolbarIcons);
}

bool XAP_MacApp::initialize(void)
{
	// let our base class do it's thing.
	
	XAP_App::initialize();


	// load only one copy of the platform-specific icons.

//	m_pMacToolbarIcons = new AP_MacToolbar_Icons();
	
	// do anything else we need here...

	// _pClipboard = new AP_MacClipboard();
	
	return true;
}

void XAP_MacApp::reallyExit(void)
{
	ExitToShell();
}

XAP_DialogFactory * XAP_MacApp::getDialogFactory(void)
{
	return &m_dialogFactory;
}

XAP_Toolbar_ControlFactory * XAP_MacApp::getControlFactory(void)
{
	return &m_controlFactory;
}

const char * XAP_MacApp::getUserPrivateDirectory(void) {
        /* return a pointer to a static buffer */
	short vRefNum;
	long dirID;
	FSSpec dirSpec;
	OSErr err;
	Handle fullPath = NULL;
	short fpLen = 0;
	
#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

        static char buf[PATH_MAX];
//        memset(buf,0,sizeof(buf));
		buf [0] = 0;
		
		/* change this later as MacOS don't really have user dir. use prefs dir
		   instead */
		::FindFolder (kOnSystemDisk, kPreferencesFolderType, true, &vRefNum, &dirID);
		err = ::FSMakeFSSpec (vRefNum, dirID, "\p", &dirSpec);
		UT_ASSERT (err == noErr);
		
		err = ::FSpGetFullPath (&dirSpec, &fpLen, &fullPath);
		UT_ASSERT (err == noErr);
		UT_ASSERT (fullPath != NULL);
		
		::HLock (fullPath);
		strncpy (buf, (char *)*fullPath, PATH_MAX);
		buf [fpLen] = 0;
		::HUnlock (fullPath); 
		::DisposeHandle (fullPath);
		fullPath = NULL;
		
		/* TODO: check buffer */
		strcat (buf, ":AbiSuite");
		
        return buf;                                     
}

UT_uint32 XAP_MacApp::_getExeDir(char* /*pDirBuf*/, UT_uint32 /*iBufLen*/)
{
	UT_ASSERT (UT_NOT_IMPLEMENTED); 
	return 0;
}



void XAP_MacApp::InitializeMacToolbox ()
{
	if (m_NotInitialized) {
		// This is REALLY the first thing to do
		// Even before any other Initialization. Note that this this is MacOS dependant
		// and application independent.
	#if XP_MAC_TARGET_CLASSIC
		// Initialize the Toolbox in Classic API

		::InitGraf(&qd.thePort);		// Toolbox Managers
		::InitFonts();
		::InitWindows();
		::InitMenus();
		::TEInit();
		::InitDialogs(nil);
		::MaxApplZone();

		::MoreMasters ();
		::MoreMasters ();
		::MoreMasters ();
		::MoreMasters ();
	#endif

	#if XP_MAC_TARGET_CARBON
		::MoreMasterPointers (4);
	#endif
		m_NotInitialized = false;
	}
}


void XAP_MacApp::DispatchEvent (const EventRecord & theEvent)
{
	WindowPtr	targetWin;
	short		winLocation;
	XAP_MacFrame *frame = NULL;
	
	switch (theEvent.what) {
	case keyDown:
		break;
	case mouseDown:
		winLocation = ::FindWindow(theEvent.where, &targetWin);
#ifdef XP_MAC_TARGET_CARBON
		if (::GetWindowKind(targetWin) == XAP_MACFRAME_WINDOW_KIND) {
			frame = (XAP_MacFrame*)GetWRefCon(targetWin);
#else
		if (((WindowRecord *)targetWin)->windowKind == XAP_MACFRAME_WINDOW_KIND) {
			frame = (XAP_MacFrame*)((WindowRecord *)targetWin)->refCon;
#endif
		}
		switch (winLocation) {
		case inMenuBar:
			HandleMenus (::MenuSelect(theEvent.where));
			break;
		case inSysWindow:
#if defined(XP_MAC_TARGET_CARBON) && XP_MAC_TARGET_CARBON
#else
			::SystemClick (&theEvent, targetWin);
#endif
			break;
		case inContent:
			if (frame != NULL) {
				frame->raise ();
			}
			else {
				::BringToFront (targetWin);
			}
			break;
		case inDrag:
			::DragWindow (targetWin, theEvent.where, 
#if defined(XP_MAC_TARGET_CARBON) && XP_MAC_TARGET_CARBON
			NULL			/* valid only for Carbon 1.0 and forward */
#else
			&qd.screenBits.bounds
#endif
			);	/* handle multiple screens */
			break;
		case inGrow:
			break;
		case inGoAway:
			if (::TrackGoAway (targetWin, theEvent.where)) {
				if (frame != NULL) {
					frame->close ();
				}
			}
			break;
		case inZoomIn:
		case inZoomOut:
			if (::TrackBox (targetWin, theEvent.where, winLocation)) {
				if (frame != NULL) {
					
				}
			}			
			break;
		}
		break;
	case kHighLevelEvent:
		::AEProcessAppleEvent (&theEvent);
		break;
	} 
}


void XAP_MacApp::HandleMenus (long menuSelection)
{
	short id, item;
	
	id = HiWord (menuSelection);
	item = LoWord (menuSelection);
	
	if (id != 0) {
		XAP_MacFrame *theFrame = dynamic_cast<XAP_MacFrame *>(getLastFocussedFrame ());
		UT_ASSERT (theFrame);
		
		EV_MacMenu *menu = theFrame->getMenu();
		UT_ASSERT (menu);
		
		XAP_Menu_Id xapId = menu->findMenuId (id, item);
		menu->onCommand (xapId);
	}	
	HiliteMenu(0);
}



/*
	Run the main event loop.
*/
void XAP_MacApp::run ()
{
	unsigned short mask = everyEvent;
	EventRecord theEvent;
	unsigned long delay = ::GetCaretTime();
	while (!m_finished) {
		while (::WaitNextEvent(mask, &theEvent, delay, NULL))
		{
			DispatchEvent (theEvent);
		}
	}
}
