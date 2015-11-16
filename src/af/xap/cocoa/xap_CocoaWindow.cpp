/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Program Utilities
 * Copyright (C) 2002 Francis James Franklin <fjf@alinameridon.com>
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
 
#include "ut_assert.h"
#include "ut_debugmsg.h"

#include "xap_CocoaWindow.h"

static float s_ToolbarHeight = 0.0;

@interface XAP_CocoaWindowDelegate : NSWindowController
{
	XAP_CocoaWindow * m_window;
}
- (id)initWithWindow:(NSWindow *)window withXAPWindow:(XAP_CocoaWindow *)xap;
- (void)windowDidResize:(NSNotification *)aNotification;
@end

@implementation XAP_CocoaWindowDelegate

- (id)initWithWindow:(NSWindow *)window withXAPWindow:(XAP_CocoaWindow *)xap
{
	if(![super initWithWindow:window]) {
		return nil;
	}
	m_window = xap;
	return self;
}

- (void)windowDidResize:(NSNotification *)aNotification
{
	UT_UNUSED(aNotification);
	if (m_window) 
		m_window->_windowResized ();
}

@end


XAP_CocoaWindow::XAP_CocoaWindow (WindowStyle ws, const NSRect & frameRect) :
	m_styleMask(NSBorderlessWindowMask),
	m_backingType(NSBackingStoreBuffered),
	m_controller(0),
	m_window(0),
	m_isToolbar(false)
{
	NSRect screenFrame = [[NSScreen mainScreen] visibleFrame];

	float diff_width  = screenFrame.size.width  - frameRect.size.width;
	float diff_height = screenFrame.size.height - frameRect.size.height;

	m_frame = NSMakeRect (screenFrame.origin.x + diff_width  / 2, screenFrame.origin.y + diff_height / 2,
							frameRect.size.width, frameRect.size.height);
	_init (ws);
}

XAP_CocoaWindow::XAP_CocoaWindow () :
	m_styleMask(NSTitledWindowMask|NSClosableWindowMask|NSMiniaturizableWindowMask|NSResizableWindowMask),
	m_backingType(NSBackingStoreBuffered),
	m_controller(0),
	m_window(0),
	m_isToolbar(false)
{
	NSRect frame = [[NSScreen mainScreen] visibleFrame];

	float width  = frame.size.width;
	float height = frame.size.height - s_ToolbarHeight;

	float x = width / 16.0;
	float y = height / 16.0;

	width  -= width  / 8.0;
	height -= height / 8.0;

	m_frame = NSMakeRect(x, y + height, width, height);

	_init (ws_Frame);
}

XAP_CocoaWindow::XAP_CocoaWindow (float height) :
	m_styleMask(NSBorderlessWindowMask),
	m_backingType(NSBackingStoreBuffered),
	m_controller(0),
	m_window(0),
	m_isToolbar(true)
{
	m_frame = [[NSScreen mainScreen] visibleFrame];

	m_frame.origin.y = m_frame.size.height - height;
	m_frame.size.height = height;

	_init (ws_Raw);

	s_ToolbarHeight = height;
}

void XAP_CocoaWindow::_init (WindowStyle ws)
{
	switch (ws)
	{
	case ws_Raw:
		m_styleMask = NSBorderlessWindowMask;
		break;

	case ws_Normal:
	default: // ??
		m_styleMask = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask;
		break;
	}
	m_window = [[NSWindow alloc] initWithContentRect:m_frame styleMask:m_styleMask backing:m_backingType defer:YES];
	UT_ASSERT(m_window);
	
	m_controller = [[XAP_CocoaWindowDelegate alloc] initWithWindow:m_window withXAPWindow:this];
	[m_window setDelegate:m_controller];
}

XAP_CocoaWindow::~XAP_CocoaWindow ()
{
	if (m_controller) {
		[m_controller close];
	}
	[m_controller release];
	[m_window release];
	

	if (m_isToolbar) 
		s_ToolbarHeight = 0.0;
}

void XAP_CocoaWindow::_show ()
{
	UT_ASSERT(m_controller);
	if (m_controller == 0) 
		return;

	[m_controller showWindow:m_controller]; // what object should we really be passing?

	[m_window setFrame:m_frame display:YES];
}

/* assume that window position is top-left, not bottom-left */
void XAP_CocoaWindow::_moveto (const NSPoint & position)
{
	[m_window setFrameTopLeftPoint:position];
}

void XAP_CocoaWindow::_resize (const NSSize & size)
{
	[m_window setContentSize:size];
}

void XAP_CocoaWindow::_resize (float height) // special case for toolbar
{
	if (!m_isToolbar) 
		return;

	UT_ASSERT(m_window);
	if (m_window == 0) 
		return;

	m_frame = [[NSScreen mainScreen] visibleFrame];

	m_frame.origin.y = m_frame.size.height - height;
	m_frame.size.height = height;

	[m_window setContentSize:m_frame.size];

	[m_window setFrame:m_frame display:YES];

	s_ToolbarHeight = height;
}

/* callback notification of main-window resize
 */
void XAP_CocoaWindow::_windowResized ()
{
	// 
}
