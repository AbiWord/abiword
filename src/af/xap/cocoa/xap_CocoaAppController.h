/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 2003 Hubert Figuiere
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


#import <Cocoa/Cocoa.h>

class AV_View;
class XAP_Frame;

@class EV_CocoaMenuDelegate;

@interface XAP_CocoaApplication : NSApplication
{
	EV_CocoaMenuDelegate *	m_MenuDelegate;
}
- (id)init;
- (void)dealloc;
- (void)sendEvent:(NSEvent *)anEvent;
- (void)setMenuDelegate:(EV_CocoaMenuDelegate *)menuDelegate;
@end

@interface XAP_CocoaAppController : NSObject {
	IBOutlet NSMenu* m_menuBar;
	IBOutlet NSMenuItem* m_aboutMenuItem;
	IBOutlet NSMenuItem* m_prefMenuItem;
	IBOutlet NSMenuItem* m_quitMenuItem;

	BOOL m_bFileOpenedDuringLaunch;
	BOOL m_bApplicationLaunching;

	AV_View *		m_pViewCurrent;
	XAP_Frame *		m_pFrameCurrent;

	AV_View *		m_pViewPrevious;
	XAP_Frame *		m_pFramePrevious;
}
+ (XAP_CocoaAppController*)sharedAppController;

- (id)init;

- (BOOL)application:(NSApplication *)sender delegateHandlesKey:(NSString *)key;

- (void)applicationWillFinishLaunching:(NSNotification *)aNotification;
- (void)applicationDidFinishLaunching:(NSNotification *)aNotification;

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender;
- (void)applicationWillTerminate:(NSNotification *)aNotification;

- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename;
- (BOOL)application:(NSApplication *)theApplication openTempFile:(NSString *)filename;
- (BOOL)application:(NSApplication *)theApplication printFile:(NSString *)filename;
- (BOOL)applicationOpenUntitledFile:(NSApplication *)theApplication;
- (BOOL)applicationOpenFile:(NSApplication *)theApplication;

- (id)dockFileNew:(id)sender;
- (id)dockFileOpen:(id)sender;

- (NSMenu *)applicationDockMenu:(NSApplication *)sender;

- (NSMenu *)getMenuBar;
- (NSMenuItem *)_aboutMenu;
- (NSMenuItem *)_preferenceMenu;
- (NSMenuItem *)_quitMenu;

- (void)setCurrentView:(AV_View *)view inFrame:(XAP_Frame *)frame;
- (void)unsetCurrentView:(AV_View *)view inFrame:(XAP_Frame *)frame;

- (AV_View *)currentView;
- (XAP_Frame *)currentFrame;

- (AV_View *)previousView;
- (XAP_Frame *)previousFrame;
@end

extern XAP_CocoaAppController* XAP_AppController_Instance;
