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


#import <Cocoa/Cocoa.h>


@interface XAP_CocoaAppController : NSObject {
	IBOutlet NSMenu* m_menuBar;
	IBOutlet NSMenuItem* m_aboutMenuItem;
	IBOutlet NSMenuItem* m_prefMenuItem;
	IBOutlet NSMenuItem* m_quitMenuItem;
	
}
+ (XAP_CocoaAppController*)sharedAppController;
- (NSMenu *)getMenuBar;
- (NSMenuItem *)_aboutMenu;
- (NSMenuItem *)_preferenceMenu;
- (NSMenuItem *)_quitMenu;
@end

extern XAP_CocoaAppController* XAP_AppController_Instance;

