/* AbiSource Program Utilities
 * Copyright (C) 2002 Hubert Figuiere
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
 
#import "xap_CocoaToolbarWindow.h"

#include "ut_vector.h"
#include "ut_debugmsg.h"
#include "ev_CocoaToolbar.h"


static XAP_CocoaToolbarWindow * pSharedToolbar = nil;

@implementation XAP_CocoaToolbarWindow

+ (XAP_CocoaToolbarWindow *)create
{
	UT_DEBUGMSG (("Cocoa: @XAP_CocoaToolbarWindow create\n"));

	NSScreen * mainScreen = [NSScreen mainScreen];
	NSRect screenFrame = [mainScreen visibleFrame];
	NSRect windowFrame;
	windowFrame.size.height = 100.0f;	// TODO calc the bottom
	windowFrame.size.width = screenFrame.size.width;
	windowFrame.origin.x = 0.0f;
	windowFrame.origin.y = screenFrame.size.height - windowFrame.size.height;		
	NSWindow * myWindow = [[NSWindow alloc] initWithContentRect:windowFrame styleMask:NSBorderlessWindowMask 
											backing:NSBackingStoreBuffered defer:YES];
	UT_ASSERT (myWindow);
	[myWindow setHidesOnDeactivate:YES];
	
	XAP_CocoaToolbarWindow * tlbr = [[XAP_CocoaToolbarWindow alloc] initWithWindow:myWindow];

	return tlbr;
}

+ (XAP_CocoaToolbarWindow *)sharedToolbar
{
	if (pSharedToolbar == NULL) {
		/* no toolbar created. create one and show it */
		pSharedToolbar = [XAP_CocoaToolbarWindow create];
		[pSharedToolbar showWindow:pSharedToolbar];
		xxx_UT_DEBUGMSG (("Toolbar is visible ? : %d\n", [myWindow isVisible]));
	}
	return pSharedToolbar;
}


- (id) init
{
	self = [super init];
	if (self) {
		m_toolbarVector = new UT_Vector (5);
	}
	return self;
}

- (void)dealloc
{
	if (m_toolbarVector) {
		delete m_toolbarVector;
	}
	[super dealloc];
}

- (void)removeAllToolbars
{
}


- (BOOL)addToolbar:(EV_CocoaToolbar *)aToolbar
{
}


- (BOOL)removeToolbar:(EV_CocoaToolbar *)aToolbar
{
}

- (void)autoResize
{

}

#if 0
- (NSView *)getTopView
{
	return [[self window] contentView];
}
#endif

@end

