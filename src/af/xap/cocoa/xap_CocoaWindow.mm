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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */
 
#include "ut_assert.h"
#include "ut_debugmsg.h"
#include "ut_exception.h"

#include "xap_CocoaWindow.h"

static UT_uint32 s_ToolbarHeight = 0;

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
	[super initWithWindow:window];
	m_window = xap;
	return self;
}

- (void)windowDidResize:(NSNotification *)aNotification
{
	if (m_window) m_window->_windowResized ();
}

@end

XAP_CocoaWindow::WindowException::WindowException (WindowError we) : m_we(we)
{
	// 
}

/* throws WindowException, and possibly others, who knows...
 */
XAP_CocoaWindow::XAP_CocoaWindow (WindowStyle ws, UT_sint32 x, UT_sint32 y, UT_uint32 width, UT_uint32 height) :
	m_styleMask(NSBorderlessWindowMask),
	m_backingType(NSBackingStoreBuffered),
	m_controller(0),
	m_window(0),
	m_view(0),
	m_isToolbar(false)
{
	NSRect frame = [[NSScreen mainScreen] frame];

	UT_sint32 diff_width  = static_cast<UT_sint32>(frame.size.width)  - width;
	UT_sint32 diff_height = static_cast<UT_sint32>(frame.size.height) - height;

	m_frame.origin.x = static_cast<float>(static_cast<UT_sint32>(frame.origin.x) + diff_width  / 2);
	m_frame.origin.y = static_cast<float>(static_cast<UT_sint32>(frame.origin.y) + diff_height / 2);

	m_frame.size.width  = static_cast<float>(width);
	m_frame.size.height = static_cast<float>(height);

	_init (ws);
}

/* throws WindowException, and possibly others, who knows...
 */
XAP_CocoaWindow::XAP_CocoaWindow () :
	m_styleMask(NSTitledWindowMask|NSClosableWindowMask|NSMiniaturizableWindowMask|NSResizableWindowMask),
	m_backingType(NSBackingStoreBuffered),
	m_controller(0),
	m_window(0),
	m_view(0),
	m_isToolbar(false)
{
	NSRect frame = [[NSScreen mainScreen] frame];

	UT_uint32 width  = static_cast<UT_uint32>(frame.size.width);
	UT_uint32 height = static_cast<UT_uint32>(frame.size.height) - s_ToolbarHeight;

	UT_sint32 x = width  >> 4;
	UT_sint32 y = height >> 4;

	width  -= width  >> 3;
	height -= height >> 3;

	m_frame.origin.x = static_cast<float>(x);
	m_frame.origin.y = static_cast<float>(y + height);

	m_frame.size.width  = static_cast<float>(width);
	m_frame.size.height = static_cast<float>(height);

	_init (ws_Frame);
}

/* throws WindowException, and possibly others, who knows...
 * (special case for toolbar)
 */
XAP_CocoaWindow::XAP_CocoaWindow (UT_uint32 height) :
	m_styleMask(NSBorderlessWindowMask),
	m_backingType(NSBackingStoreBuffered),
	m_controller(0),
	m_window(0),
	m_view(0),
	m_isToolbar(true)
{
	m_frame = [[NSScreen mainScreen] visibleFrame];

	m_frame.origin.y = static_cast<float>((static_cast<UT_uint32>(m_frame.size.height) - 0) - height);
	m_frame.size.height = static_cast<float>(height);

	_init (ws_Raw);

	s_ToolbarHeight = height;
}

/* throws WindowException, and possibly others, who knows...
 */
void XAP_CocoaWindow::_init (WindowStyle ws)
{
	XAP_CocoaWindowDelegate * delegate = [XAP_CocoaWindowDelegate alloc];
	if (delegate == 0)
		{
			UT_THROW (WindowError(we_NoController));
			return;
		}
	m_window = (NSWindow *) [NSWindow alloc];
	if (m_window == 0)
		{
			// [m_controller dealloc];
			// m_controller = 0;
			UT_THROW (WindowError(we_NoWindow));
			return;
		}

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

	[m_window initWithContentRect:m_frame styleMask:m_styleMask backing:m_backingType defer:YES];

	m_controller = (NSWindowController *) [delegate initWithWindow:m_window withXAPWindow:this];

	[m_window setDelegate:m_controller];

	m_view = [m_window contentView];
}

XAP_CocoaWindow::~XAP_CocoaWindow ()
{
	if (m_controller) [m_controller close];
	//	if (m_controller) [m_controller dealloc];

	//	if (m_window) [m_window dealloc];

	m_controller = 0;
	m_window = 0;
	m_view = 0;

	if (m_isToolbar) s_ToolbarHeight = 0;
}

void XAP_CocoaWindow::_show ()
{
	UT_ASSERT(m_controller);
	if (m_controller == 0) return;

	[m_controller showWindow:m_controller]; // what object should we really be passing?

	[m_window setFrame:m_frame display:YES];
}

void XAP_CocoaWindow::_moveto (UT_sint32 x, UT_sint32 y)
{
	// TODO
}

void XAP_CocoaWindow::_resize (UT_uint32 width, UT_uint32 height)
{
	// TODO
}

void XAP_CocoaWindow::_resize (UT_uint32 height) // special case for toolbar
{
	if (!m_isToolbar) return;

	UT_ASSERT(m_window);
	if (m_window == 0) return;

	m_frame = [[NSScreen mainScreen] visibleFrame];

	m_frame.origin.y = static_cast<float>((static_cast<UT_uint32>(m_frame.size.height) - 0) - height);
	m_frame.size.height = static_cast<float>(height);

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
