/* AbiSource Application Framework
 * Copyright (C) 2003-2004 Hubert Figuiere
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

#include "xap_App.h"
#include "xap_Frame.h"

#include "ie_types.h"

#import "xap_CocoaAppController.h"

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


- (BOOL)application:(NSApplication *)theApplication openFile:(NSString *)filename
{
	bool result;
	UT_DEBUGMSG(("Requested to open %s\n", [filename UTF8String]));
	XAP_App * pApp = XAP_App::getApp();
	XAP_Frame * pNewFrame = pApp->newFrame();

	result = pNewFrame->loadDocument([filename UTF8String], IEFT_Unknown);
	/*
		TODO: check what we should really do now
	*/
	pNewFrame->show();
	return result;
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
	UT_DEBUGMSG(("Requested a new file\n"));
	XAP_App * pApp = XAP_App::getApp();
	XAP_Frame * pNewFrame = pApp->newFrame();
	/*UT_Error error = */pNewFrame->loadDocument(NULL, IEFT_Unknown);
	pNewFrame->show();
	return YES;
}

- (NSMenu *)applicationDockMenu:(NSApplication *)sender
{
	return nil;
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


@end
