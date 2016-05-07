/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; ident-tabs-mode: t -*- */

/* AbiSource Application Framework
 * Copyright (C) 2003-2016 Hubert Figuiere
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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#import <string>

#include "ut_debugmsg.h"

#include "ev_EditMethod.h"
#include "ev_CocoaMenuBar.h"

#include "xap_CocoaApp.h"
#include "xap_CocoaAppController.h"
#include "xap_CocoaModule.h"
#include "xap_CocoaToolPalette.h"
#include "xap_CocoaToolProvider.h"
#include "xap_App.h"
#include "xap_Frame.h"

#include "ie_types.h"

#include "ap_Menu_Id.h"

struct EV_CocoaKeyEquiv
{
	XAP_Menu_Id		menuid;
	const char *			equiv;
	unsigned int	modifier;
};

static struct EV_CocoaKeyEquiv KeyEquiv[] = {
	{	AP_MENU_ID_FILE_NEW,				"n",	NSCommandKeyMask									}, // Cmd-N
	{	AP_MENU_ID_FILE_NEW_USING_TEMPLATE,	"N",	NSCommandKeyMask									}, // Cmd-Shift-N
	{	AP_MENU_ID_FILE_OPEN,				"o",	NSCommandKeyMask									}, // Cmd-O
	{	AP_MENU_ID_FILE_SAVE,				"s",	NSCommandKeyMask									}, // Cmd-S
	{	AP_MENU_ID_FILE_SAVEAS,				"S",	NSCommandKeyMask									}, // Cmd-Shift-S
	{	AP_MENU_ID_FILE_CLOSE,				"w",	NSCommandKeyMask									}, // Cmd-W
	{	AP_MENU_ID_FILE_PAGESETUP,			"P",	NSCommandKeyMask									}, // Cmd-Shift-P
	{	AP_MENU_ID_FILE_PRINT,				"p",	NSCommandKeyMask									}, // Cmd-P
	{	AP_MENU_ID_EDIT_UNDO,				"z",	NSCommandKeyMask									}, // Cmd-Z
	{	AP_MENU_ID_EDIT_REDO,				"Z",	NSCommandKeyMask									}, // Cmd-Shift-Z
	{	AP_MENU_ID_EDIT_CUT,				"x",	NSCommandKeyMask									}, // Cmd-X
	{	AP_MENU_ID_EDIT_COPY,				"c",	NSCommandKeyMask									}, // Cmd-C
	{	AP_MENU_ID_EDIT_PASTE,				"v",	NSCommandKeyMask									}, // Cmd-V
	{	AP_MENU_ID_EDIT_PASTE_SPECIAL,		"V",	NSCommandKeyMask|NSAlternateKeyMask					}, // Cmd-Shift-Alt-V
	{	AP_MENU_ID_EDIT_SELECTALL,			"a",	NSCommandKeyMask									}, // Cmd-A
	{	AP_MENU_ID_EDIT_FIND,				"f",	NSCommandKeyMask									}, // Cmd-F
	{	AP_MENU_ID_EDIT_GOTO,				"j",	NSCommandKeyMask									}, // Cmd-J
	{	AP_MENU_ID_VIEW_RULER,				"r",	NSCommandKeyMask									}, // Cmd-R
	{	AP_MENU_ID_FMT_FONT,				"t",	NSCommandKeyMask									}, // Cmd-T
	{	AP_MENU_ID_FMT_BOLD,				"b",	NSCommandKeyMask									}, // Cmd-B
	{	AP_MENU_ID_FMT_ITALIC,				"i",	NSCommandKeyMask									}, // Cmd-I
	{	AP_MENU_ID_FMT_UNDERLINE,			"u",	NSCommandKeyMask									}, // Cmd-U
	{	AP_MENU_ID_TOOLS_SPELL,				":",	NSCommandKeyMask									}, // Cmd-:
	{	AP_MENU_ID_ALIGN_LEFT,				"{",	NSCommandKeyMask									}, // Cmd-{
	{	AP_MENU_ID_ALIGN_CENTER,			"|",	NSCommandKeyMask									}, // Cmd-|
	{	AP_MENU_ID_ALIGN_RIGHT,				"}",	NSCommandKeyMask									}, // Cmd-}
	{	0,									0,		0													}
};

@implementation XAP_CocoaApplication

- (void)terminate:(id)sender
{
	UT_UNUSED(sender);
	UT_UCS4String ucs4_empty;
	ev_EditMethod_invoke("querySaveAndExit", ucs4_empty);
}

- (void)orderFrontStandardAboutPanel:(id)sender
{
	UT_UNUSED(sender);
	UT_UCS4String ucs4_empty;
	ev_EditMethod_invoke("dlgAbout", ucs4_empty);
}

- (void)orderFrontPreferencesPanel:(id)sender
{
	UT_UNUSED(sender);
	UT_UCS4String ucs4_empty;
	ev_EditMethod_invoke("dlgOptions", ucs4_empty);
}

- (void)showHelp:(id)sender
{
	UT_UNUSED(sender);
	UT_UCS4String ucs4_empty; // Can we use this to override help-contents location? e.g., to bundle help files? [TODO]
	ev_EditMethod_invoke("helpContents", ucs4_empty); // [TODO: this needs to be redireced to firstResponder]	
}

- (void)openContextHelp:(id)sender
{
	UT_UNUSED(sender);
	UT_UCS4String ucs4_empty; // Can we use this to override help-contents location? e.g., to bundle help files? [TODO]
	ev_EditMethod_invoke("helpContents", ucs4_empty); // [TODO: this needs to be redireced to firstResponder]
}

- (void)sendEvent:(NSEvent *)anEvent
{
	NSWindow * keyWindow = [self keyWindow];

	XAP_CocoaAppController * pController = (XAP_CocoaAppController *) [self delegate];

	bool bFrameIsActive = pController ? ([pController currentFrame] ? true : false) : false;

	bool bEventHandled = false;

	if ([anEvent type] == NSKeyDown)
	{
		unsigned int modifierFlags = [anEvent modifierFlags];

		if (modifierFlags & NSCommandKeyMask)
		{
			if ((bFrameIsActive || !keyWindow) && !(modifierFlags & (NSShiftKeyMask|NSControlKeyMask)))
			{
				NSString * str = [anEvent charactersIgnoringModifiers];
				if ([str length] == 1)
				{
					unichar uc;
					[str getCharacters:&uc];

					if ((uc & 0x7f) == uc) 
					{
						switch (static_cast<char>(uc))
						{
						case ',': // Cmd-, (preferences)
							if (!(modifierFlags & NSAlternateKeyMask))
							{
								UT_DEBUGMSG(("[XAP_CocoaApplication -sendEvent: (Cmd-,)]\n"));
								[self orderFrontPreferencesPanel:self];
								bEventHandled = true;
							}
							break;

						case '?': // Cmd-? (help)
							if (!(modifierFlags & NSAlternateKeyMask))
							{
								UT_DEBUGMSG(("[XAP_CocoaApplication -sendEvent: (Cmd-?)]\n"));
								[self openContextHelp:self];
								bEventHandled = true;
							}
							break;

						case 'h': // Alt-Cmd-H (hide others)
							if (modifierFlags & NSAlternateKeyMask)
							{
								UT_DEBUGMSG(("[XAP_CocoaApplication -sendEvent: (Alt-Cmd-H)]\n"));
								[self hideOtherApplications:self];
								bEventHandled = true;
							}
							else  // Cmd-H (hide application)
							{
								UT_DEBUGMSG(("[XAP_CocoaApplication -sendEvent: (Cmd-H)]\n"));
								[self hide:self];
								bEventHandled = true;
							}
							break;

						case 'm': // Cmd-M (minimize current frame/window)
							if (bFrameIsActive && keyWindow && !(modifierFlags & NSAlternateKeyMask))
							{
								UT_DEBUGMSG(("[XAP_CocoaApplication -sendEvent: (Cmd-M)]\n"));
								[keyWindow miniaturize:self];
								bEventHandled = true;
							}
							break;

						case 'n': // Cmd-N (open untitled)
							if (pController && !(modifierFlags & NSAlternateKeyMask))
							{
								UT_DEBUGMSG(("[XAP_CocoaApplication -sendEvent: (Cmd-N)]\n"));
								[pController applicationOpenUntitledFile:self];
								bEventHandled = true;
							}
							break;

						case 'o': // Cmd-O (open file)
							if (pController && !(modifierFlags & NSAlternateKeyMask))
							{
								UT_DEBUGMSG(("[XAP_CocoaApplication -sendEvent: (Cmd-O)]\n"));
								[pController applicationOpenFile:self];
								bEventHandled = true;
							}
							break;

						case 'q': // Cmd-Q (quit)
							if (!(modifierFlags & NSAlternateKeyMask))
							{
								UT_DEBUGMSG(("[XAP_CocoaApplication -sendEvent: (Cmd-Q)]\n"));
								[self terminate:self];
								bEventHandled = true;
							}
							break;

						default:
							break;
						}
					}
				}
			}

			if (!bEventHandled && false /* m_MenuDelegate */ && bFrameIsActive)
			{
				id  target;
				SEL action;

				if (false /* [m_MenuDelegate menuHasKeyEquivalent:[self mainMenu] forEvent:anEvent target:&target action:&action] */)
				{
					[self sendAction:action to:target from:self];
					bEventHandled = true;
				}
			}
			else if (!bEventHandled && keyWindow && !bFrameIsActive)
			{
				NSString * str = [anEvent charactersIgnoringModifiers];
				if ([str length] == 1)
				{
					unichar uc;
					[str getCharacters:&uc];

					if ((uc & 0x7f) == uc)
						switch (static_cast<char>(uc))
						{
						case 'a': // In other applications this would normally be handled by the Edit menu
						{
							if (NSResponder * firstResponder = [keyWindow firstResponder])
							{
								[firstResponder selectAll:self];
								bEventHandled = true;
							}
							break;
						}
						case 'c': // In other applications this would normally be handled by the Edit menu
						{
							if (NSResponder * firstResponder = [keyWindow firstResponder]) 
							{
								if ([firstResponder respondsToSelector:@selector(copy:)])
								{
									NSText * textResponder = (NSText *) firstResponder;
									[textResponder copy:self];
									bEventHandled = true;
								}
							}
							break;
						}
						case 'v': // In other applications this would normally be handled by the Edit menu
						{
							if (NSResponder * firstResponder = [keyWindow firstResponder]) 
							{
								if ([firstResponder respondsToSelector:@selector(paste:)])
								{
									NSText * textResponder = (NSText *) firstResponder;
									[textResponder paste:self];
									bEventHandled = true;
								}
							}
							break;
						}
						case 'x': // In other applications this would normally be handled by the Edit menu
						{
							if (NSResponder * firstResponder = [keyWindow firstResponder]) 
							{
								if ([firstResponder respondsToSelector:@selector(cut:)])
								{
									NSText * textResponder = (NSText *) firstResponder;
									[textResponder cut:self];
									bEventHandled = true;
								}
							}
							break;
						}
						default:
							break;
						}
				}
			}
		}
	}
	if (!bEventHandled)
	{
		[super sendEvent:anEvent];
	}
}

@end

static XAP_CocoaAppController * XAP_AppController_Instance = nil;

@implementation XAP_CocoaAppController

+ (XAP_CocoaAppController*)sharedAppController // do we really need/want this?? the app controller is instantiated within the application's nib
{
	if (!XAP_AppController_Instance)
	{
		UT_DEBUGMSG(("sharedAppController does not exist.\n"));
		[[XAP_CocoaAppController alloc] init];
	}
	return XAP_AppController_Instance;
}

- (id)init 
{
	UT_DEBUGMSG(("create the XAP_CocoaAppController init\n"));
	if (XAP_AppController_Instance)
	{
		NSLog (@"Attempt to allocate more that one XAP_CocoaAppController");
		return nil;
	}

	if(![super init]) {
		return nil;
	}
	XAP_AppController_Instance = self;

	m_FilesRequestedDuringLaunch = [[NSMutableArray alloc] initWithCapacity:8];

	m_bApplicationLaunching   = YES;

	m_bAutoLoadPluginsAfterLaunch = NO;

	m_PanelMenu   = [[NSMenu alloc] initWithTitle:@"Panels"];
	m_ContextMenu = [[NSMenu alloc] initWithTitle:@"Context Menu"];

	m_MenuIDRefDictionary = [[NSMutableDictionary alloc] initWithCapacity:16];

	m_Plugins      = [[NSMutableArray alloc] initWithCapacity:16];
	m_PluginsTools = [[NSMutableArray alloc] initWithCapacity:16];

	m_PluginsToolsSeparator = [NSMenuItem separatorItem];
	[m_PluginsToolsSeparator retain];

	m_ToolProviders = [[NSMutableArray alloc] initWithCapacity:4];
	return self;
}

- (void)dealloc
{
	[m_FilesRequestedDuringLaunch release];
	[m_PanelMenu release];
	[m_ContextMenu release];
	[m_MenuIDRefDictionary release];
	[m_Plugins release];
	[m_PluginsTools release];
	[m_PluginsToolsSeparator release];
	[m_ToolProviders release];
	[super dealloc];
}

- (void)setAutoLoadPluginsAfterLaunch:(BOOL)autoLoadPluginsAfterLaunch
{
	m_bAutoLoadPluginsAfterLaunch = autoLoadPluginsAfterLaunch;
}

- (BOOL)application:(NSApplication *)sender delegateHandlesKey:(NSString *)key
{
	UT_UNUSED(sender);
	return [key isEqualToString:@"orderedDocuments"];
}

- (void)applicationWillFinishLaunching:(NSNotification *)aNotification
{
	UT_UNUSED(aNotification);
	if (const char * home = getenv("HOME"))
	{
		NSString * desktop = [[NSString stringWithUTF8String:home] stringByAppendingPathComponent:@"Desktop"];
		
		[[NSFileManager defaultManager] changeCurrentDirectoryPath:desktop];
	}
	if (NSMenu * menu = [NSApp windowsMenu]) {
		if (NSMenuItem * item = [[NSMenuItem alloc] initWithTitle:@"Panels" action:nil keyEquivalent:@""])
		{
			[menu addItem:item];
			[item setSubmenu:m_PanelMenu];
			[item release];
		}
	}
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	UT_UNUSED(aNotification);
	UT_DEBUGMSG(("[XAP_CocoaAppController -applicationDidFinishLaunching:]\n"));
	m_bApplicationLaunching = NO;

	UT_DEBUGMSG(("[...FinishLaunching] Constructing standard toolset:\n"));
	[m_ToolProviders addObject:[XAP_CocoaToolProvider AbiWordToolProvider]];

	XAP_CocoaApp * pApp = static_cast<XAP_CocoaApp *>(XAP_App::getApp());

	if (EV_CocoaMenuBar * pCocoaMenuBar = pApp->getCocoaMenuBar())
	{
		m_AppItem[XAP_CocoaAppMenu_File  ] = oMenuItem_File;
		m_AppItem[XAP_CocoaAppMenu_Edit  ] = oMenuItem_Edit;
		m_AppItem[XAP_CocoaAppMenu_View  ] = oMenuItem_View;
		m_AppItem[XAP_CocoaAppMenu_Insert] = oMenuItem_Insert;
		m_AppItem[XAP_CocoaAppMenu_Format] = oMenuItem_Format;
		m_AppItem[XAP_CocoaAppMenu_Tools ] = oMenuItem_Tools;
		m_AppItem[XAP_CocoaAppMenu_Table ] = oMenuItem_Table;
		m_AppItem[XAP_CocoaAppMenu_Window] = oMenuItem_Window;
		m_AppItem[XAP_CocoaAppMenu_Help  ] = oMenuItem_Help;

		m_AppMenu[XAP_CocoaAppMenu_File  ] = oMenu_File;
		m_AppMenu[XAP_CocoaAppMenu_Edit  ] = oMenu_Edit;
		m_AppMenu[XAP_CocoaAppMenu_View  ] = oMenu_View;
		m_AppMenu[XAP_CocoaAppMenu_Insert] = oMenu_Insert;
		m_AppMenu[XAP_CocoaAppMenu_Format] = oMenu_Format;
		m_AppMenu[XAP_CocoaAppMenu_Tools ] = oMenu_Tools;
		m_AppMenu[XAP_CocoaAppMenu_Table ] = oMenu_Table;
		m_AppMenu[XAP_CocoaAppMenu_Window] = oMenu_Window;
		m_AppMenu[XAP_CocoaAppMenu_Help  ] = oMenu_Help;

		UT_DEBUGMSG(("[...FinishLaunching] Building application menu:\n"));
		pCocoaMenuBar->buildAppMenu();
	}
	[XAP_CocoaToolPalette instance:self];

	if (m_bAutoLoadPluginsAfterLaunch)
	{
		UT_DEBUGMSG(("[...FinishLaunching] Auto-Loading plug-ins:\n"));
		XAP_CocoaModule::loadAllPlugins();
	}

	BOOL bFileOpenedDuringLaunch = NO;

	unsigned count = [m_FilesRequestedDuringLaunch count];

	for (unsigned i = 0; i < count; i++) // filter out plugins from list and open those first
	{
		NSString * filename = (NSString *) [m_FilesRequestedDuringLaunch objectAtIndex:i];

		std::string path([filename UTF8String]);

		if (XAP_CocoaModule::hasPluginExtension(path))
		{
			[self application:NSApp openFile:filename];
		}
	}
	for (unsigned i = 0; i < count; i++) // open the rest
	{
		NSString * filename = (NSString *) [m_FilesRequestedDuringLaunch objectAtIndex:i];

		std::string path([filename UTF8String]);

		if (!XAP_CocoaModule::hasPluginExtension(path))
			if ([self application:NSApp openFile:filename])
			{
				bFileOpenedDuringLaunch = YES;
			}
	}
	if (bFileOpenedDuringLaunch == NO)
	{
		UT_DEBUGMSG(("[...FinishLaunching] No file opened during launch, so opening untitled document:\n"));
		[self applicationOpenUntitledFile:NSApp];
	}
}

- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender // probably unused now that NSApp's terminate is overridden
{
	UT_UNUSED(sender);
	UT_DEBUGMSG(("- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication *)sender\n"));
	UT_UCS4String ucs4_empty;
	bool bQuit = ev_EditMethod_invoke("querySaveAndExit", ucs4_empty);
	return (bQuit ? NSTerminateNow : NSTerminateCancel);
}

- (void)applicationWillTerminate:(NSNotification *)aNotification // probably unused now that NSApp's terminate is overridden
{
	UT_UNUSED(aNotification);
	if ([XAP_CocoaToolPalette instantiated])
	{
		[[XAP_CocoaToolPalette instance:self] close];
	}
}

- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename
{
	UT_UNUSED(theApplication);
	if (m_bApplicationLaunching == YES)
	{
		[m_FilesRequestedDuringLaunch addObject:filename];
		return YES;
	}

	UT_DEBUGMSG(("Requested to open %s\n", [filename UTF8String]));

	std::string path([filename UTF8String]);

	if (XAP_CocoaModule::hasPluginExtension(path))
	{
		return (XAP_CocoaModule::loadPlugin(path) ? YES : NO);
	}

	XAP_App * pApp = XAP_App::getApp();
	XAP_Frame * pNewFrame = pApp->newFrame();

	bool result = (UT_OK == pNewFrame->loadDocument([filename UTF8String], IEFT_Unknown));
	/*
	 * TODO: check what we should really do now
	 */
	if (result)
	{
		pNewFrame->show();
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
	UT_UNUSED(theApplication);
	if (m_bApplicationLaunching == YES)
	{
		return YES;
	}

	UT_DEBUGMSG(("Requested to open untitled file...\n"));

	EV_EditMethodContainer * pEMC = XAP_App::getApp()->getEditMethodContainer();
	if (!pEMC) {
		return NO;
	}

	EV_EditMethod * pEM = pEMC->findEditMethodByName("fileNew");
	if (!pEM) {
		return NO;
	}

	return (pEM->Fn(0,0) ? YES : NO);
}

- (BOOL)applicationOpenFile:(NSApplication *)theApplication
{
	UT_UNUSED(theApplication);
	EV_EditMethodContainer * pEMC = XAP_App::getApp()->getEditMethodContainer();
	if (!pEMC) {
		return NO;
	}

	EV_EditMethod * pEM = pEMC->findEditMethodByName("fileOpen");
	if (!pEM) {
		return NO;
	}

	return (pEM->Fn(0,0) ? YES : NO);
}

- (id)dockFileNew:(id)sender
{
	UT_UNUSED(sender);
	[self applicationOpenUntitledFile:NSApp];
	return self;
}

- (id)dockFileOpen:(id)sender
{
	UT_UNUSED(sender);
	[self applicationOpenFile:NSApp];
	return self;
}
/*
  - (NSMenu *)applicationDockMenu:(NSApplication *)sender
  {
  XAP_CocoaApp * pCocoaApp = static_cast<XAP_CocoaApp *>(XAP_App::getApp());
  return (NSMenu *) pCocoaApp->getDockNSMenu ();
  }
*/
/* For building the Application Menu
 */
- (void)setAboutTitle:(NSString *)title
{
	[oMenuItem_AboutAbiWord setTitle:title];
}

- (void)setPrefsTitle:(NSString *)title
{
	[oMenuItem_Preferences setTitle:title];
}

- (void)setCHelpTitle:(NSString *)title // context-help
{
	[oMenuItem_AbiWordHelp setTitle:title];
}

- (void)setTitle:(NSString *)title forMenu:(XAP_CocoaAppMenu_Id)appMenu
{
	switch (appMenu)
	{
	case XAP_CocoaAppMenu_File:
	case XAP_CocoaAppMenu_Edit:
	case XAP_CocoaAppMenu_View:
	case XAP_CocoaAppMenu_Insert:
	case XAP_CocoaAppMenu_Format:
	case XAP_CocoaAppMenu_Tools:
	case XAP_CocoaAppMenu_Table:
	case XAP_CocoaAppMenu_Window: // ??
	case XAP_CocoaAppMenu_Help:
		[m_AppItem[appMenu] setTitle:title];
		break;

	case XAP_CocoaAppMenu_AbiWord: // disallow

	case XAP_CocoaAppMenu_count__:
	default:
		// shouldn't really happen, but...
		break;
	}
}

- (const char *)keyEquivalentForMenuID:(int /* XAP_Menu_Id */)menuid modifierMask:(unsigned int *)mask
{
	const char * equiv = 0;

	struct EV_CocoaKeyEquiv * pKE = KeyEquiv;
	while (pKE->equiv)
	{
		if (menuid == (int) pKE->menuid)
		{
			equiv = pKE->equiv;
			if ( mask)
				*mask = pKE->modifier;
			break;
		}
		++pKE;
	}
	return equiv;
}

- (NSMenu *)panelMenu
{
	return m_PanelMenu;
}

- (NSMenu *)contextMenu
{
	return m_ContextMenu;
}

- (void)appendPanelItem:(NSMenuItem *)item
{
	[m_PanelMenu addItem:item];
}

- (void)appendContextItem:(NSMenuItem *)item
{
	[m_ContextMenu addItem:item];
}

- (void)appendItem:(NSMenuItem *)item toMenu:(XAP_CocoaAppMenu_Id)appMenu
{
	switch (appMenu)
	{
	case XAP_CocoaAppMenu_File:
	case XAP_CocoaAppMenu_Edit:
	case XAP_CocoaAppMenu_View:
	case XAP_CocoaAppMenu_Insert:
	case XAP_CocoaAppMenu_Format:
	case XAP_CocoaAppMenu_Tools:
	case XAP_CocoaAppMenu_Table:
	case XAP_CocoaAppMenu_Help:
	{
		// UT_DEBUGMSG(("appendItem: toMenu:\"%s\"\n", [[m_AppItem[appMenu] title] UTF8String]));
		[m_AppMenu[appMenu] addItem:item];
	}
	break;

	case XAP_CocoaAppMenu_AbiWord:
	case XAP_CocoaAppMenu_Window:
		// these menus treated separately

	case XAP_CocoaAppMenu_count__:
	default:
		// shouldn't really happen, but...
		break;
	}
}

- (void)clearContextMenu
{
	while (int count = [m_ContextMenu numberOfItems])
	{
		[m_ContextMenu removeItemAtIndex:(count - 1)];
	}
}

- (void)clearMenu:(XAP_CocoaAppMenu_Id)appMenu
{
	switch (appMenu)
	{
	case XAP_CocoaAppMenu_File:
	case XAP_CocoaAppMenu_Edit:
	case XAP_CocoaAppMenu_View:
	case XAP_CocoaAppMenu_Insert:
	case XAP_CocoaAppMenu_Format:
	case XAP_CocoaAppMenu_Tools:
	case XAP_CocoaAppMenu_Table:
		while (int count = [m_AppMenu[appMenu] numberOfItems])
		{
			[m_AppMenu[appMenu] removeItemAtIndex:(count - 1)];
		}
		break;

	case XAP_CocoaAppMenu_Help:
		while (int count = [m_AppMenu[appMenu] numberOfItems])
		{
			if (count == 1) // the first Help item (context-help) is treated separately
				break;
			[m_AppMenu[appMenu] removeItemAtIndex:(count - 1)];
		}
		break;

	case XAP_CocoaAppMenu_AbiWord:
	case XAP_CocoaAppMenu_Window:
		// these menus treated separately

	case XAP_CocoaAppMenu_count__:
	default:
		// shouldn't really happen, but...
		break;
	}
}

- (void)clearAllMenus
{
	// !AbiWord
	[self clearMenu:XAP_CocoaAppMenu_File  ];
	[self clearMenu:XAP_CocoaAppMenu_Edit  ];
	[self clearMenu:XAP_CocoaAppMenu_View  ];
	[self clearMenu:XAP_CocoaAppMenu_Insert];
	[self clearMenu:XAP_CocoaAppMenu_Format];
	[self clearMenu:XAP_CocoaAppMenu_Tools ];
	[self clearMenu:XAP_CocoaAppMenu_Table ];
	// !Windows
	[self clearMenu:XAP_CocoaAppMenu_Help  ];
}


- (void)reappendPluginMenuItems
{
	if (unsigned count = [m_PluginsTools count])
	{
		[m_AppMenu[XAP_CocoaAppMenu_Tools] addItem:m_PluginsToolsSeparator];

		for (unsigned i = 0; i < count; i++)
		{
			NSMenuItem * menuItem = (NSMenuItem *) [m_PluginsTools objectAtIndex:i];

			[m_AppMenu[XAP_CocoaAppMenu_Tools] addItem:menuItem];
		}
	}
}

- (void)appendPluginMenuItem:(NSMenuItem *)menuItem
{
	if (![m_PluginsTools containsObject:menuItem])
	{
		if ([m_PluginsTools count] == 0)
		{
			[m_AppMenu[XAP_CocoaAppMenu_Tools] addItem:m_PluginsToolsSeparator];
		}
		[m_AppMenu[XAP_CocoaAppMenu_Tools] addItem:menuItem];

		[m_PluginsTools addObject:menuItem];
	}
}

- (void)removePluginMenuItem:(NSMenuItem *)menuItem
{
	if ([m_PluginsTools containsObject:menuItem])
	{
		[m_AppMenu[XAP_CocoaAppMenu_Tools] removeItem:menuItem];

		if ([m_PluginsTools count] == 1)
		{
			[m_AppMenu[XAP_CocoaAppMenu_Tools] removeItem:m_PluginsToolsSeparator];
		}
		[m_PluginsTools removeObject:menuItem];
	}
}

/* Do we need this? getLastFocussedFrame() should be tracking this now... [TODO!!]
 */
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

	[self notifyFrameViewChange];
}

- (void)resetCurrentView:(AV_View *)view inFrame:(XAP_Frame *)frame
{
	// UT_DEBUGMSG(("XAP_CocoaAppController - (void)resetCurrentView:(AV_View *)view inFrame:(XAP_Frame *)frame\n"));
	if (m_pFrameCurrent == frame)
	{
		m_pViewCurrent = view;
		[self notifyFrameViewChange];
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
		m_pViewCurrent = NULL;
		m_pFrameCurrent = NULL;
	}
	if ((m_pViewPrevious == view) && (m_pFramePrevious == frame))
	{
		m_pViewPrevious = NULL;
		m_pFramePrevious = NULL;
	}
	[self notifyFrameViewChange];
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

- (void)notifyFrameViewChange
{
	if ([XAP_CocoaToolPalette instantiated])
	{
		[[XAP_CocoaToolPalette instance:self] setCurrentView:m_pViewCurrent inFrame:m_pFrameCurrent];
	}

	unsigned count = [m_Plugins count];

	for (unsigned i = 0; i < count; i++)
	{
		XAP_CocoaPlugin * plugin = (XAP_CocoaPlugin *) [m_Plugins objectAtIndex:i];
		[[plugin delegate] pluginCurrentDocumentHasChanged];
	}
}

/* load .Abi bundle plugin at path, returns nil on failure
 */
- (XAP_CocoaPlugin *)loadPlugin:(NSString *)path
{
	if (!path) {
		return nil;
	}

	XAP_CocoaPlugin * cocoa_plugin = [[XAP_CocoaPlugin alloc] init];
	if (!cocoa_plugin) {
		return nil;
	}

	if ([cocoa_plugin loadBundleWithPath:path])
	{
		[m_Plugins addObject:cocoa_plugin];
	}
	else
	{
		[cocoa_plugin release];
		cocoa_plugin = nil;
	}
	return cocoa_plugin;
}

/* list of currently loaded plugins
 */
- (NSArray *)plugins
{
	return m_Plugins;
}

/* checks to see whether the plugins can deactivate, and, if they can, deactivates them;
 * returns false if any of the plugins object
 */
- (BOOL)deactivateAllPlugins
{
	unsigned count = [m_Plugins count];

	BOOL bCanDeactivate = YES;

	for (unsigned i = 0; i < count; i++)
	{
		XAP_CocoaPlugin * plugin = (XAP_CocoaPlugin *) [m_Plugins objectAtIndex:i];

		bCanDeactivate = [[plugin delegate] pluginCanDeactivate];
		if (!bCanDeactivate)
			break;
	}
	if (!bCanDeactivate) {
		return NO;
	}

	for (unsigned i = 0; i < count; i++)
	{
		XAP_CocoaPlugin * plugin = (XAP_CocoaPlugin *) [m_Plugins objectAtIndex:i];

		[[plugin delegate] pluginDeactivate];
	}
	return YES;
}

/* checks to see whether the plugins can deactivate, and, if they can, deactivates them;
 * returns false if the plugin objects, unless override is YES.
 */
- (BOOL)deactivatePlugin:(XAP_CocoaPlugin *)plugin overridePlugin:(BOOL)override
{
	if (!override)
	{
		if (![[plugin delegate] pluginCanDeactivate]) {
			return NO;
		}
	}
	[[plugin delegate] pluginDeactivate];

	return YES;
}

/* This provides a mechanism for associating XAP_CocoaPlugin_MenuItem objects
 * with a given menu ID.
 */
- (void)addRef:(AP_CocoaPlugin_MenuIDRef *)ref forMenuID:(NSNumber *)menuid
{
	[m_MenuIDRefDictionary setObject:ref forKey:menuid];
}

/* This provides a mechanism for finding XAP_CocoaPlugin_MenuItem objects associated
 * with a given menu ID.
 */
- (AP_CocoaPlugin_MenuIDRef *)refForMenuID:(NSNumber *)menuid
{
	return (AP_CocoaPlugin_MenuIDRef *) [m_MenuIDRefDictionary objectForKey:menuid];
}

/* This provides a mechanism for removing XAP_CocoaPlugin_MenuItem objects associated
 * with a given menu ID.
 */
- (void)removeRefForMenuID:(NSNumber *)menuid
{
	[m_MenuIDRefDictionary removeObjectForKey:menuid];
}

/* Get a list of all the tool providers.
 * Each tool provider is of type id <NSObject, XAP_CocoaPlugin_ToolProvider>.
 */
- (NSArray *)toolProviders
{
	return m_ToolProviders;
}

/* Find a tool provider by name.
 * (TODO: If plug-ins are registering tool providers, we need to implement a notification
 *        system to update toolbar systems.)
 */
- (id <NSObject, XAP_CocoaPlugin_ToolProvider>)toolProvider:(NSString *)name
{
	id <NSObject, XAP_CocoaPlugin_ToolProvider> matched_provider = 0;

	int count = [m_ToolProviders count];
	int i;

	if (!name)
		return 0;

	if ([name length] == 0)
		return 0;

	for (i = 0; i < count; i++)
		{
			id <NSObject, XAP_CocoaPlugin_ToolProvider> provider = (id <NSObject, XAP_CocoaPlugin_ToolProvider>) [m_ToolProviders objectAtIndex:i];

			if ([name isEqualToString:[provider name]])
				{
					matched_provider = provider;
					break;
				}
		}
	return matched_provider;
}

@end
