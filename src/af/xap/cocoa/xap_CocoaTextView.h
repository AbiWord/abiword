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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */
/*
	Implements the text view that supports the NSTextInput protocol
 */

#import <AppKit/AppKit.h>

#import "xap_CocoaFrame.h"

class AV_View;

@interface XAP_CocoaTextView : XAP_CocoaNSView <NSTextInput> {
	NSRange		m_selectedRange;
	BOOL		m_hasMarkedText;
}
- (id)initWith:(XAP_Frame *)frame andFrame:(NSRect)windowFrame;
@end
