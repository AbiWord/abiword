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

#ifndef XAP_COCOAAPPCONTROLLER_H
#define XAP_COCOAAPPCONTROLLER_H

#import <Cocoa/Cocoa.h>

class AV_View;
class XAP_Frame;

@class EV_CocoaMenuDelegate;

@class XAP_CocoaPlugin;

@interface XAP_CocoaApplication : NSApplication
{
	// 
}
- (void)terminate:(id)sender;
- (void)orderFrontStandardAboutPanel:(id)sender;
- (void)orderFrontPreferencesPanel:(id)sender;
- (void)openContextHelp:(id)sender;
- (void)sendEvent:(NSEvent *)anEvent;
@end

enum XAP_CocoaAppMenu_Id
{
	XAP_CocoaAppMenu_AbiWord = 0,
	XAP_CocoaAppMenu_File,
	XAP_CocoaAppMenu_Edit,
	XAP_CocoaAppMenu_View,
	XAP_CocoaAppMenu_Insert,
	XAP_CocoaAppMenu_Format,
	XAP_CocoaAppMenu_Tools,
	XAP_CocoaAppMenu_Table,
	XAP_CocoaAppMenu_Window,
	XAP_CocoaAppMenu_Help,
	XAP_CocoaAppMenu_count__
};

@interface XAP_CocoaAppController : NSObject
{
	IBOutlet NSMenu *		oMenu_AbiWord;

	IBOutlet NSMenuItem *	oMenuItem_AboutAbiWord;
	IBOutlet NSMenuItem *	oMenuItem_Preferences;

	IBOutlet NSMenu *		oMenu_File;
	IBOutlet NSMenu *		oMenu_Edit;
	IBOutlet NSMenu *		oMenu_View;
	IBOutlet NSMenu *		oMenu_Insert;
	IBOutlet NSMenu *		oMenu_Format;
	IBOutlet NSMenu *		oMenu_Tools;
	IBOutlet NSMenu *		oMenu_Table;
	IBOutlet NSMenu *		oMenu_Window;
	IBOutlet NSMenu *		oMenu_Help;

	IBOutlet NSMenuItem *	oMenuItem_File;
	IBOutlet NSMenuItem *	oMenuItem_Edit;
	IBOutlet NSMenuItem *	oMenuItem_View;
	IBOutlet NSMenuItem *	oMenuItem_Insert;
	IBOutlet NSMenuItem *	oMenuItem_Format;
	IBOutlet NSMenuItem *	oMenuItem_Tools;
	IBOutlet NSMenuItem *	oMenuItem_Table;
	IBOutlet NSMenuItem *	oMenuItem_Window;
	IBOutlet NSMenuItem *	oMenuItem_Help;

	IBOutlet NSMenuItem *	oMenuItem_AbiWordHelp;

	NSMenu *				m_PanelMenu;
	NSMenu *				m_ContextMenu;

	NSMenu *				m_AppMenu[XAP_CocoaAppMenu_count__];
	NSMenuItem *			m_AppItem[XAP_CocoaAppMenu_count__];

	NSMutableArray *		m_Plugins;
	NSMutableArray *		m_PluginsTools;

	BOOL			m_bFileOpenedDuringLaunch;
	BOOL			m_bApplicationLaunching;
	BOOL			m_bAutoLoadPluginsAfterLaunch;

	AV_View *		m_pViewCurrent;
	XAP_Frame *		m_pFrameCurrent;

	AV_View *		m_pViewPrevious;
	XAP_Frame *		m_pFramePrevious;
}
+ (XAP_CocoaAppController*)sharedAppController;

- (id)init;
- (void)dealloc;

- (void)setAutoLoadPluginsAfterLaunch:(BOOL)autoLoadPluginsAfterLaunch;

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

/* - (NSMenu *)applicationDockMenu:(NSApplication *)sender; */

/* For building the Application Menu
 */
- (void)setAboutTitle:(NSString *)title;
- (void)setPrefsTitle:(NSString *)title;
- (void)setCHelpTitle:(NSString *)title; // context-help

- (void)setTitle:(NSString *)title forMenu:(XAP_CocoaAppMenu_Id)appMenu;

- (const char *)keyEquivalentForMenuID:(int /* XAP_Menu_Id */)menuid modifierMask:(unsigned int *)mask;

- (NSMenu *)panelMenu;
- (NSMenu *)contextMenu;

- (void)appendPanelItem:(NSMenuItem *)item;
- (void)appendContextItem:(NSMenuItem *)item;
- (void)appendItem:(NSMenuItem *)item toMenu:(XAP_CocoaAppMenu_Id)appMenu;

- (void)clearContextMenu;
- (void)clearMenu:(XAP_CocoaAppMenu_Id)appMenu; // except AbiWord & Windows
- (void)clearAllMenus;                          // except AbiWord & Windows

- (void)appendPluginMenuItem:(NSMenuItem *)menuItem;
- (void)removePluginMenuItem:(NSMenuItem *)menuItem;

/* Do we need this? getLastFocussedFrame() should be tracking this now... [TODO!!]
 */
- (void)setCurrentView:(AV_View *)view inFrame:(XAP_Frame *)frame;
- (void)resetCurrentView:(AV_View *)view inFrame:(XAP_Frame *)frame;
- (void)unsetCurrentView:(AV_View *)view inFrame:(XAP_Frame *)frame;

- (AV_View *)currentView;
- (XAP_Frame *)currentFrame;

- (AV_View *)previousView;
- (XAP_Frame *)previousFrame;

/* load .Abi bundle plugin at path, returns nil on failure
 */
- (XAP_CocoaPlugin *)loadPlugin:(NSString *)path;

/* list of currently loaded plugins
 */
- (NSArray *)plugins;

/* checks to see whether the plugins can deactivate, and, if they can, deactivates them;
 * returns false if any of the plugins object
 */
- (BOOL)deactivateAllPlugins;

/* checks to see whether the plugins can deactivate, and, if they can, deactivates them;
 * returns false if the plugin objects, unless override is YES.
 */
- (BOOL)deactivatePlugin:(XAP_CocoaPlugin *)plugin overridePlugin:(BOOL)override;
@end

#endif /* ! XAP_COCOAAPPCONTROLLER_H */
