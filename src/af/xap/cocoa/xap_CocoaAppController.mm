/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 2003-2004 Hubert Figuiere
 * Copyright (C) 2004 Francis James Franklin
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


#include "ut_debugmsg.h"

#include "ev_EditMethod.h"
#include "ev_CocoaMenuBar.h"

#include "xap_CocoaApp.h"
#include "xap_CocoaToolPalette.h"
#include "xap_App.h"
#include "xap_Frame.h"

#include "ie_types.h"

#import "xap_CocoaAppController.h"

@implementation XAP_CocoaApplication

- (id)init
{
	if (self = [super init])
		{
			m_MenuDelegate = 0;
		}
	return self;
}

- (void)dealloc
{
	if (m_MenuDelegate)
		{
			[m_MenuDelegate release];
			m_MenuDelegate = 0;
		}
	[super dealloc];
}

- (void)sendEvent:(NSEvent *)anEvent
{
	if (m_MenuDelegate)
		if ([anEvent type] == NSKeyDown)
			if ([anEvent modifierFlags] & NSCommandKeyMask)
				{
					id  target;
					SEL action;

					if ([m_MenuDelegate menuHasKeyEquivalent:[self mainMenu] forEvent:anEvent target:&target action:&action])
						{
							[self sendAction:action to:target from:self];
							return;
						}
				}
	[super sendEvent:anEvent];
}

- (void)setMenuDelegate:(EV_CocoaMenuDelegate *)menuDelegate
{
	if (m_MenuDelegate)
		{
			[m_MenuDelegate release];
		}

	m_MenuDelegate = menuDelegate;

	if (m_MenuDelegate)
		{
			[m_MenuDelegate retain];
		}
}

@end

XAP_CocoaAppController* XAP_AppController_Instance = nil;

@implementation XAP_CocoaAppController

- (id) init 
{
	if (XAP_AppController_Instance) {
		NSLog (@"Attempt to allocate more that one XAP_CocoaAppController");
		return nil;
	}
	self = [super init];
	if (self) {
		XAP_AppController_Instance = self;
		[[NSApplication sharedApplication] setDelegate:self];
		m_bFileOpenedDuringLaunch = NO;
		m_bApplicationLaunching = YES;
	}
	return self;
}

+ (XAP_CocoaAppController*)sharedAppController
{
	if (!XAP_AppController_Instance) {
		[[XAP_CocoaAppController alloc] init];
	}
	return XAP_AppController_Instance;
}

- (BOOL)application:(NSApplication *)sender delegateHandlesKey:(NSString *)key
{
	return [key isEqualToString:@"orderedDocuments"];
}

- (void)applicationWillFinishLaunching:(NSNotification *)aNotification
{
	if (const char * home = getenv("HOME"))
		{
			NSString * desktop = [[NSString stringWithUTF8String:home] stringByAppendingPathComponent:@"Desktop"];

			[[NSFileManager defaultManager] changeCurrentDirectoryPath:desktop];
		}
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	UT_DEBUGMSG(("[XAP_CocoaAppController -applicationDidFinishLaunching:]\n"));
	m_bApplicationLaunching = NO;

	if (m_bFileOpenedDuringLaunch == NO)
	{
		UT_DEBUGMSG(("No file opened during launch, so opening untitled document:\n"));
		[self applicationOpenUntitledFile:NSApp];
	}
	if (EV_CocoaMenuBar * application_menu = EV_CocoaMenuBar::instance())
	{
		[NSApp setMainMenu:(application_menu->getMenuBar())];
	}
	[XAP_CocoaToolPalette instance:self];
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender
{
	UT_DEBUGMSG(("- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender\n"));
	UT_UCS4String ucs4_empty;
	bool bQuit = ev_EditMethod_invoke("querySaveAndExit", ucs4_empty);
	return (bQuit ? NSTerminateNow : NSTerminateCancel);
}

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
	if ([XAP_CocoaToolPalette instantiated])
		{
			[[XAP_CocoaToolPalette instance:self] close];
		}
}

- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename
{
	UT_DEBUGMSG(("Requested to open %s\n", [filename UTF8String]));
	XAP_App * pApp = XAP_App::getApp();
	XAP_Frame * pNewFrame = pApp->newFrame();

	bool result = (UT_OK == pNewFrame->loadDocument([filename UTF8String], IEFT_Unknown));
	if (result)
	{
		/*
		 * TODO: check what we should really do now
		 */
	}
	if (result)
	{
		pNewFrame->show();

		if (m_bApplicationLaunching == YES)
			m_bFileOpenedDuringLaunch = YES;
	}
	return (result ? YES : NO);
}

- (BOOL)application:(NSApplication *)theApplication openTempFile:(NSString *)filename
{
	/*
		TODO: really open temp file. ie, delete it when done
	 */
	UT_DEBUGMSG(("Requested to open temp file %s\n", [filename UTF8String]));
	return [self application:theApplication openFile:filename];
}

- (BOOL)application:(NSApplication *)theApplication printFile:(NSString *)filename
{
	/*
		TODO: really print the file.
	 */
	UT_DEBUGMSG(("Requested to print %s\n", [filename UTF8String]));
	return [self application:theApplication openFile:filename];
}

- (BOOL)applicationOpenUntitledFile:(NSApplication *)theApplication
{
	UT_DEBUGMSG(("Requested to open untitled file...\n"));

	EV_EditMethodContainer * pEMC = XAP_App::getApp()->getEditMethodContainer();
	if (!pEMC)
		return NO;

	EV_EditMethod * pEM = pEMC->findEditMethodByName("fileNew");
	if (!pEM)
		return NO;

	bool result = pEM->Fn(0,0);
	if (result)
	{
		if (m_bApplicationLaunching == YES)
			m_bFileOpenedDuringLaunch = YES;
	}
	return (result ? YES : NO);
}

- (BOOL)applicationOpenFile:(NSApplication *)theApplication
{
	EV_EditMethodContainer * pEMC = XAP_App::getApp()->getEditMethodContainer();
	if (!pEMC)
		return NO;

	EV_EditMethod * pEM = pEMC->findEditMethodByName("fileOpen");
	if (!pEM)
		return NO;

	return (pEM->Fn(0,0) ? YES : NO);
}

- (id)dockFileNew:(id)sender
{
	[self applicationOpenUntitledFile:NSApp];
	return self;
}

- (id)dockFileOpen:(id)sender
{
	[self applicationOpenFile:NSApp];
	return self;
}

- (NSMenu *)applicationDockMenu:(NSApplication *)sender
{
	XAP_CocoaApp * pCocoaApp = static_cast<XAP_CocoaApp *>(XAP_App::getApp());
	return (NSMenu *) pCocoaApp->getDockNSMenu ();
}


- (NSMenu *)getMenuBar
{
	return m_menuBar;
}

- (NSMenuItem *)_aboutMenu
{
	return m_aboutMenuItem;
}

- (NSMenuItem *)_quitMenu
{
	return m_quitMenuItem;
}

- (NSMenuItem *)_preferenceMenu
{
	return m_prefMenuItem;
}

- (void)setCurrentView:(AV_View *)view inFrame:(XAP_Frame *)frame
{
	if (frame)
		{
			XAP_App * pApp = XAP_App::getApp();

			pApp->clearLastFocussedFrame();
			pApp->rememberFocussedFrame(static_cast<void *>(frame));
		}

	m_pViewPrevious  = m_pViewCurrent;
	m_pFramePrevious = m_pFrameCurrent;

	m_pViewCurrent  = view;
	m_pFrameCurrent = frame;

	if ([XAP_CocoaToolPalette instantiated])
		{
			[[XAP_CocoaToolPalette instance:self] setCurrentView:view inFrame:frame];
		}
}

- (void)unsetCurrentView:(AV_View *)view inFrame:(XAP_Frame *)frame
{
	XAP_App * pApp = XAP_App::getApp();

	if (pApp->getLastFocussedFrame() == frame)
		{
			pApp->clearLastFocussedFrame();
		}
	if ((m_pViewCurrent == view) && (m_pFrameCurrent == frame))
		{
			m_pViewCurrent = 0;
			m_pFrameCurrent = 0;
		}
	if ((m_pViewPrevious == view) && (m_pFramePrevious == frame))
		{
			m_pViewPrevious = 0;
			m_pFramePrevious = 0;
		}
}

- (AV_View *)currentView
{
	return m_pViewCurrent;
}

- (XAP_Frame *)currentFrame
{
	return m_pFrameCurrent;
}

- (AV_View *)previousView
{
	return m_pViewPrevious;
}

- (XAP_Frame *)previousFrame
{
	return m_pFramePrevious;
}

@end
