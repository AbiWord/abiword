/* AbiSource Program Utilities
 * Copyright (C) 2002-2003 Hubert Figuiere
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

#import "xap_CocoaFrameImpl.h"


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
	windowFrame.origin.x = screenFrame.origin.x;
	windowFrame.origin.y = screenFrame.origin.y + (screenFrame.size.height - windowFrame.size.height);		
	NSWindow * myWindow = [[NSWindow alloc] initWithContentRect:windowFrame styleMask:NSBorderlessWindowMask 
											backing:NSBackingStoreBuffered defer:YES];
	UT_ASSERT (myWindow);
	[myWindow setHidesOnDeactivate:YES];
	[myWindow setReleasedWhenClosed:NO];
	[myWindow setExcludedFromWindowsMenu:YES];
	[myWindow setCanHide:YES];
	
	XAP_CocoaToolbarWindow * tlbr = [[XAP_CocoaToolbarWindow alloc] initWithWindow:myWindow];

	return tlbr;
}

+ (XAP_CocoaToolbarWindow *)sharedToolbar
{
	if (pSharedToolbar == nil) {
		/* no toolbar created. create one and show it */
		pSharedToolbar = [XAP_CocoaToolbarWindow create];
		[pSharedToolbar showWindow:pSharedToolbar];
		xxx_UT_DEBUGMSG (("Toolbar is visible ? : %d\n", [myWindow isVisible]));
	}
	return pSharedToolbar;
}


- (id)initWithWindow:(NSWindow *)window
{
	self = [super initWithWindow:window];
	if (self) {
		m_toolbarVector = new UT_Vector (5);
		[[NSNotificationCenter defaultCenter] addObserver:self 
			selector:@selector(showToolbarNotification:)
			name:XAP_CocoaFrameImpl::XAP_FrameNeedToolbar 
			object:nil]; 
		[[NSNotificationCenter defaultCenter] addObserver:self 
			selector:@selector(hideToolbarNotification:)
			name:XAP_CocoaFrameImpl::XAP_FrameReleaseToolbar 
			object:nil]; 
	}
	return self;
}

- (void)dealloc
{
	[[NSNotificationCenter defaultCenter] removeObserver:self];
	if (m_toolbarVector) {
		delete m_toolbarVector;
	}
	[super dealloc];
}

- (void)removeAllToolbars
{
	NSArray* views = [[[self window] contentView] subviews];
	NSEnumerator* iter = [views objectEnumerator];
	NSView* obj;
	while (obj = [iter nextObject]) {
		[obj removeFromSuperview];
	}
}


- (void)autoResize
{

}

- (void)showToolbarNotification:(NSNotification*)notif
{
	UT_DEBUGMSG(("received showToolbarNotification:\n"));
	XAP_CocoaFrameController* frame = [notif object];
	
	if (frame == m_current) {
		UT_DEBUGMSG(("already shown\n"));
		return;
	}
	
	[self removeAllToolbars];
	
	NSArray* toolbars = [frame getToolbars];
	int count = [toolbars count];
	float height = count * EV_CocoaToolbar::getToolbarHeight();
	
	NSRect bounds = [[self window] frame];
	float delta = bounds.size.height - height;
	bounds.size.height = height;
	bounds.origin.y += delta;
	[[self window] setFrame:bounds display:NO];
	XAP_CocoaFrameImpl::setToolbarRect(bounds);
	
	NSEnumerator*	iter = [toolbars objectEnumerator];
	NSView*		superView = [[self window] contentView];
	NSView* 	obj;
	float currentY = height - EV_CocoaToolbar::getToolbarHeight();
	while (obj = [iter nextObject]) {
		[superView addSubview:obj];
		bounds = [obj frame];
		bounds.origin.y = currentY;
		[obj setFrame:bounds];
		currentY -= EV_CocoaToolbar::getToolbarHeight();
	}
	[[self window] orderFront:self];
	m_current = frame;
}


- (void)hideToolbarNotification:(NSNotification*)notif
{
	UT_DEBUGMSG(("received hideToolbarNotification:\n"));
	if (m_current != [notif object]) {
		NSLog(@"attempt to hide toolbar for a different frame.");
	}
	[self removeAllToolbars];
	m_current = nil;
	[[self window] orderOut:self];
}


#if 0
- (NSView *)getTopView
{
	return [[self window] contentView];
}
#endif

@end

