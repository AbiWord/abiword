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

@class XAP_CocoaPlugin;

@class AP_CocoaPlugin_MenuIDRef;

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

	NSMutableDictionary *	m_FontDictionary;

	NSMutableDictionary *	m_MenuIDRefDictionary;

	NSMutableArray *		m_Plugins;
	NSMutableArray *		m_PluginsTools;

	NSMenuItem *			m_PluginsToolsSeparator;

	NSMutableArray *		m_FilesRequestedDuringLaunch;

	BOOL			m_bApplicationLaunching;
	BOOL			m_bAutoLoadPluginsAfterLaunch;

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

- (void)appendPanelItem:(NSMenuItem *)item;
- (void)appendContextItem:(NSMenuItem *)item;
- (void)appendItem:(NSMenuItem *)item toMenu:(XAP_CocoaAppMenu_Id)appMenu;

- (void)clearContextMenu;
- (void)clearMenu:(XAP_CocoaAppMenu_Id)appMenu; // except AbiWord & Windows
- (void)clearAllMenus;                          // except AbiWord & Windows

- (NSString *)familyNameForFont:(NSString *)fontName;

- (void)appendPluginMenuItem:(NSMenuItem *)menuItem;
- (void)removePluginMenuItem:(NSMenuItem *)menuItem;

/* Do we need this? getLastFocussedFrame() should be tracking this now... [TODO!!]
 */
- (void)setCurrentView:(AV_View *)view inFrame:(XAP_Frame *)frame;
- (void)unsetCurrentView:(AV_View *)view inFrame:(XAP_Frame *)frame;

- (AV_View *)currentView;
- (XAP_Frame *)currentFrame;

- (AV_View *)previousView;
- (XAP_Frame *)previousFrame;

- (void)notifyFrameViewChange; // [re/un]setCurrentView call this

/**
 * Load .Abi bundle plugin at path.
 * 
 * \return Returns nil on failure.
 */
- (XAP_CocoaPlugin *)loadPlugin:(NSString *)path;

/**
 * \return Returns list of currently loaded plugins.
 */
- (NSArray *)plugins;

/**
 * Checks to see whether the plugins can deactivate, and, if they can, deactivates them.
 * 
 * \return Returns false if any of the plugins object.
 */
- (BOOL)deactivateAllPlugins;

/**
 * Checks to see whether the plugins can deactivate, and, if they can, deactivates them.
 * 
 * \return Returns false if the plugin objects, unless override is YES.
 */
- (BOOL)deactivatePlugin:(XAP_CocoaPlugin *)plugin overridePlugin:(BOOL)override;

/**
 * This provides a mechanism for associating XAP_CocoaPlugin_MenuItem objects
 * with a given menu ID.
 */
- (void)addRef:(AP_CocoaPlugin_MenuIDRef *)ref forMenuID:(NSNumber *)menuid;

/**
 * This provides a mechanism for finding XAP_CocoaPlugin_MenuItem objects associated
 * with a given menu ID.
 */
- (AP_CocoaPlugin_MenuIDRef *)refForMenuID:(NSNumber *)menuid;

/**
 * This provides a mechanism for removing XAP_CocoaPlugin_MenuItem objects associated
 * with a given menu ID.
 */
- (void)removeRefForMenuID:(NSNumber *)menuid;

@end

extern XAP_CocoaAppController* XAP_AppController_Instance;
