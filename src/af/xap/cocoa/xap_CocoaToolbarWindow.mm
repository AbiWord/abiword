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

#include "ut_debugmsg.h"
#include "ev_CocoaToolbar.h"


static XAP_CocoaToolbarWindow * pSharedToolbar = nil;

@implementation XAP_CocoaToolbarWindow

+ (XAP_CocoaToolbarWindow *)createFromNib
{
	UT_DEBUGMSG (("Cocoa: @XAP_CocoaToolbarWindow createFromNib\n"));

	XAP_CocoaToolbarWindow * tlbr = [[XAP_CocoaToolbarWindow alloc] initWithWindowNibName:@"xap_CocoaToolbarWindow"];

	return tlbr;
}

+ (XAP_CocoaToolbarWindow *)sharedToolbar
{
	if (pSharedToolbar == NULL) {
		pSharedToolbar = [XAP_CocoaToolbarWindow createFromNib];
		[pSharedToolbar window];
	}
	return pSharedToolbar;
}

/*!
	Standard method override
 */
- (void)windowDidLoad
{
	NSWindow * myWindow = [self window];
	UT_ASSERT (myWindow);	// if does not occur, then this is real bad news.
	NSRect screenFrame = [[myWindow screen] frame];
	NSRect windowFrame = [myWindow frame];
	windowFrame.size.width = screenFrame.size.width;
	windowFrame.origin.x = 0.0f;
	windowFrame.origin.y = 0.0f;		// TODO calc to be stuck to the bottom of the menubar.
	[myWindow setFrame:windowFrame display:YES];
	[myWindow setFrameOrigin:windowFrame.origin];
	[super windowDidLoad];
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

- (NSView *)getTopView
{
	return m_topView;
}

@end
