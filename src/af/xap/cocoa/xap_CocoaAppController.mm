/* AbiSource Application Framework
 * Copyright (C) 2003 Hubert Figuiere
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
