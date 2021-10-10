/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001, 2009-2021 Hubert Figuière
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

#pragma once

#import <Cocoa/Cocoa.h>

#include "xap_Frame.h"

class XAP_CocoaApp;
class EV_CocoaKeyboard;
class EV_CocoaMouse;
class EV_CocoaMenuBar;
class EV_CocoaMenuPopup;

/*****************************************************************
******************************************************************
** This file defines the cocoa-platform-specific class for the
** cross-platform application frame.  This is used to hold all
** cocoa-specific data.  One of these is created for each top-level
** document window.
******************************************************************
*****************************************************************/

class GR_CocoaGraphics;
class FV_View;
class XAP_Drawable;

@protocol XAP_MouseEventDelegate
- (void)mouseDown:(NSEvent *)theEvent from:(id)sender;
- (void)mouseDragged:(NSEvent *)theEvent from:(id)sender;
- (void)mouseUp:(NSEvent *)theEvent from:(id)sender;
@end


// TODO should figure out if need default values
@interface XAP_CocoaNSView : NSView
{
    NSString* m_name;
	NSCursor* _cursor;
	bool _in_draw_rect;
	XAP_Frame* m_pFrame;
	GR_CocoaGraphics* m_pGR;
	XAP_Drawable* m_drawable;
	NSObject<XAP_MouseEventDelegate>	*_eventDelegate;
}
#if DEBUG
@property (readonly) bool in_draw_rect;
#endif
@property XAP_Drawable* drawable;

- (id)initWith:(XAP_Frame*)frame andFrame:(NSRect)windowFrame andName:(NSString*)name;
- (BOOL)acceptsFirstResponder;
- (BOOL)becomeFirstResponder;
- (XAP_Frame*)xapFrame;
- (void)setGraphics:(GR_CocoaGraphics*)gr;
- (void)setEventDelegate:(NSObject<XAP_MouseEventDelegate>*)delegate;
- (NSObject<XAP_MouseEventDelegate>*)eventDelegate;
- (void)drawRect:(NSRect)aRect;
- (BOOL)isFlipped;
- (BOOL)isOpaque;
- (void)hasBeenResized:(NSNotification*)notif;
- (void)setCursor:(NSCursor*)cursor;
@end
