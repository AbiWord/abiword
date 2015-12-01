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

#ifndef XAP_COCOAWINDOW_H
#define XAP_COCOAWINDOW_H

#import <Cocoa/Cocoa.h>

/* pre-emptive dismissal; ut_types.h is needed by just about everything,
 * so even if it's commented out in-file that's still a lot of work for
 * the preprocessor to do...
 */
#ifndef UT_TYPES_H
#include "ut_types.h"
#endif

class XAP_CocoaWindow;

@interface XAP_CocoaWindowDelegate
	: NSWindowController<NSWindowDelegate>
{
	XAP_CocoaWindow * m_window;
}
- (id)initWithWindow:(NSWindow *)window withXAPWindow:(XAP_CocoaWindow *)xap;
- (void)windowDidResize:(NSNotification *)aNotification;
@end

class XAP_CocoaWindow
{
public:
	enum WindowStyle
	{
		ws_Normal = 0,
		ws_Raw, // i.e., no decorations; use by toolbar window
		ws_Frame,
		ws_Panel
	};
	enum WindowError
	{
		we_NoController,
		we_NoWindow
	};

protected:
	XAP_CocoaWindow (WindowStyle ws, const NSRect & frameRect);
	XAP_CocoaWindow (); // special case for document/frame
	XAP_CocoaWindow (float height); // special case for toolbar
private:
	void _init (WindowStyle ws);
public:
	virtual ~XAP_CocoaWindow ();

	/* callback notification of main-window resize
	 */
	virtual void			_windowResized ();

protected:
	void					_show (); // this should be called only from the child's constructor, I think

	void					_moveto (const NSPoint & position);

	void					_resize (const NSSize & size);
	void					_resize (float height); // special case for toolbar

protected:
	unsigned int			m_styleMask;

	NSBackingStoreType		m_backingType;
	XAP_CocoaWindowDelegate *	m_controller;
	NSWindow *				m_window;
	bool					m_isToolbar;
	NSRect					m_frame;
};

#endif /* ! XAP_COCOAWINDOW_H */
