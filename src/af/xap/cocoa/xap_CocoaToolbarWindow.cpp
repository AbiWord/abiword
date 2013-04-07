/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Program Utilities
 * Copyright (C) 2002-2004 Hubert Figuiere
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
 
#include "ut_debugmsg.h"

#include "ev_CocoaToolbar.h"

#include "xap_CocoaFrameImpl.h"
#include "xap_CocoaToolbarWindow.h"

static XAP_CocoaToolbarWindow_Controller * s_pSharedToolbar = nil;

@interface XAP_CocoaToolbarWindow : NSPanel
{
	// 
}
- (id)initWithContentRect:(NSRect)frame;
- (BOOL)canBecomeKeyWindow;
@end

@implementation XAP_CocoaToolbarWindow

- (id)initWithContentRect:(NSRect)windowFrame
{
	if (![super initWithContentRect:windowFrame styleMask:NSBorderlessWindowMask backing:NSBackingStoreBuffered defer:YES])	{
		return nil;
	}
	[self setBecomesKeyOnlyIfNeeded:YES];
	[self setHidesOnDeactivate:YES];
	[self setReleasedWhenClosed:YES]; // ??
	[self setExcludedFromWindowsMenu:YES];
	[self setCanHide:YES];
	return self;
}

- (BOOL)canBecomeKeyWindow
{
	return YES;
}

@end

@implementation XAP_CocoaToolbarWindow_Controller

+ (XAP_CocoaToolbarWindow_Controller *)sharedToolbar
{
	if (!s_pSharedToolbar)
	{
		/* no toolbar created. create one and show it:
		 */
		s_pSharedToolbar = [[XAP_CocoaToolbarWindow_Controller alloc] initWithWindow:nil];

		// TODO!! [s_pSharedToolbar showWindow:s_pSharedToolbar];
		xxx_UT_DEBUGMSG (("Toolbar is visible ? : %d\n", [myWindow isVisible]));
	}
	return s_pSharedToolbar;
}


- (id)initWithWindow:(NSWindow *)window
{
	if (![super initWithWindow:window])
	{
		return nil;
	}
	m_windows = [[NSMutableArray alloc] initWithCapacity:4];
	if (!m_windows)
	{
		[self release];
		return nil;
	}
	m_SummaryID = @"";
	[m_SummaryID retain];

	m_bounds.size.height = 0;
	NSNotificationCenter * NC = [NSNotificationCenter defaultCenter];
	[NC addObserver:self selector:@selector(showToolbarNotification:) name:(XAP_CocoaFrameImpl::XAP_FrameNeedToolbar)    object:nil];
	[NC addObserver:self selector:@selector(hideToolbarNotification:) name:(XAP_CocoaFrameImpl::XAP_FrameReleaseToolbar) object:nil];
	return self;
}

- (void)dealloc
{
	if (m_windows)
	{
		[self removeAllToolbars];

		[m_windows release];
		m_windows = 0;
	}
	[m_SummaryID release];

	[[NSNotificationCenter defaultCenter] removeObserver:self];
	[super dealloc];
}

- (void)removeAllToolbars
{
	[m_SummaryID release];
	m_SummaryID = @"";
	[m_SummaryID retain];

	m_bounds.size.height = 0;

	unsigned count = [m_windows count];

	for (unsigned i = 0; i < count; i++)
	{
		NSWindowController * window = (NSWindowController *) [m_windows objectAtIndex:i];

		[window close];
		[window release];
	}
	[m_windows removeAllObjects];
}

- (void)autoResize
{
	// ??
}

- (void)redisplayToolbars:(XAP_CocoaFrameController *)frame
{
	if (!m_lock)
	{
		[self _showAllToolbars:frame];
	}
}

- (void)_showAllToolbars:(XAP_CocoaFrameController *)wframe
{
	UT_ASSERT(wframe);
	if (!wframe)
	{
		[self removeAllToolbars];
		return;
	}

	bool bNewConfiguration = true;

	NSString * SummaryID = [wframe getToolbarSummaryID];
	if ([m_SummaryID isEqualToString:SummaryID])
	{
		XAP_CocoaFrameImpl::setToolbarRect(m_bounds);
		bNewConfiguration = false;
	}
	if (bNewConfiguration)
	{
		[self removeAllToolbars];

		[m_SummaryID release];
		m_SummaryID = [SummaryID retain];
	}

	NSArray * toolbars = [wframe getToolbars];

	unsigned count = [toolbars count];
	if (!count)
		return;

	NSRect visibleFrame = [[NSScreen mainScreen] visibleFrame];

	float offset_x = 0;
	float offset_y = 0;

	for (unsigned i = 0; i < count; i++)
	{
		NSView * view = [toolbars objectAtIndex:i];

		if (!bNewConfiguration)
		{
			/* A different frame is being used but with the same configuration of toolbars, so
			 * don't create new windows, just replace the views...
			 */
			NSWindowController * controller = (NSWindowController *) [m_windows objectAtIndex:i];
			NSWindow * window = [controller window];
			NSView * contentView = [window contentView];
			NSArray * subviews = [contentView subviews];
			while ([subviews count])
			{
				NSView * subview = (NSView *) [subviews objectAtIndex:0];
				[subview removeFromSuperview];
			}
			[contentView addSubview:view];
			[window orderFront:self];
			continue;
		}

		NSRect frame = [view frame];

		bool bNewToolbarRow = true;

		if (i > 0)
			if (offset_x + frame.size.width < visibleFrame.origin.x + visibleFrame.size.width)
				bNewToolbarRow = false;

		if (bNewToolbarRow)
		{
			offset_x  = visibleFrame.origin.x;
			offset_y += frame.size.height;
		}
		frame.origin.x = offset_x;
		frame.origin.y = visibleFrame.origin.y + visibleFrame.size.height - offset_y;

		XAP_CocoaToolbarWindow * window = [[XAP_CocoaToolbarWindow alloc] initWithContentRect:frame];
		if (window)
		{
			NSWindowController * controller = [[NSWindowController alloc] initWithWindow:window];
			[m_windows addObject:controller];
			[controller showWindow:self];

			[window orderFront:self];
			[window release]; // ??
		}
		offset_x += frame.size.width;

		frame.origin.x = 0;
		frame.origin.y = 0;

		[[window contentView] addSubview:view];
	}
	if (bNewConfiguration)
	{
		visibleFrame.origin.y = visibleFrame.origin.y + visibleFrame.size.height - offset_y;
		visibleFrame.size.height = offset_y;

		m_bounds = visibleFrame;

		XAP_CocoaFrameImpl::setToolbarRect(m_bounds);
	}
}

- (float)height
{
	return m_bounds.size.height;
}

- (void)lock
{
	UT_ASSERT(m_lock == NO);
	m_lock = YES;
}

- (void)unlock
{
	UT_ASSERT(m_lock);
	m_lock = NO;
}

- (void)showToolbarNotification:(NSNotification*)notif
{
	UT_DEBUGMSG(("received showToolbarNotification:\n"));
	XAP_CocoaFrameController * frame = [notif object];
	
	if (frame == m_current)
	{
		UT_DEBUGMSG(("already shown\n"));
		return;
	}
	m_current = frame;

	[self _showAllToolbars:frame];
#if 0
	unsigned count = [m_windows count];
	if (!count)
		return;

	for (unsigned i = 0; i < count; i++)
	{
		NSWindowController * controller = (NSWindowController *) [m_windows objectAtIndex:i];
		[[controller window] orderFront:self];
	}
#endif
}

- (void)hideToolbarNotification:(NSNotification*)notif
{
	UT_DEBUGMSG(("received hideToolbarNotification:\n"));
	if (m_current != [notif object]) {
		NSLog(@"attempt to hide toolbar for a different frame.");
		return;
	}
	m_current = nil;

	unsigned count = [m_windows count];
	if (!count)
		return;

	for (unsigned i = 0; i < count; i++)
	{
		NSWindowController * controller = (NSWindowController *) [m_windows objectAtIndex:i];
		[[controller window] orderOut:self];
	}
}

@end
