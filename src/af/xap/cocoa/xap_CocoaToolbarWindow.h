/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Program Utilities
 * Copyright (C) 2002 Hubert Figuiere
 * Copyright (C) 2004 Francis James Franklin
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

#ifndef XAP_COCOATOOLBARWINDOW_H
#define XAP_COCOATOOLBARWINDOW_H

/* XAP_CocoaToolbarWindow */

#import <Cocoa/Cocoa.h>

@class XAP_CocoaFrameController;

@interface XAP_CocoaToolbarWindow_Controller : NSWindowController
{
	XAP_CocoaFrameController *	m_current;
	NSMutableArray *			m_windows;
	NSString *					m_SummaryID;
	NSRect						m_bounds;

	BOOL	m_lock;
}
+ (XAP_CocoaToolbarWindow_Controller *)sharedToolbar;

- (id)initWithWindow:(NSWindow *)window;
- (void)dealloc;

- (void)removeAllToolbars;
- (void)redisplayToolbars:(XAP_CocoaFrameController *)frame;
- (void)autoResize;

/* lock and unlock redraw for batch toolbar changes, at initialization
 */
- (void)lock;
- (void)unlock;

- (void)_showAllToolbars:(XAP_CocoaFrameController *)frame;

- (float)height;

- (void)showToolbarNotification:(NSNotification *)notif;
- (void)hideToolbarNotification:(NSNotification *)notif;
@end

#endif /* ! XAP_COCOATOOLBARWINDOW_H */
