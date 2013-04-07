/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001 Hubert Figuiere
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

#ifndef XAP_COCOATOOLBARICONS_H
#define XAP_COCOATOOLBARICONS_H

#import <Cocoa/Cocoa.h>

#include "ut_types.h"

#include "xap_Toolbar_Icons.h"

/*****************************************************************/

@class XAP_CocoaToolbarButton;

@protocol XAP_CocoaButtonController
- (void)menuWillActivate:(NSMenu *)menu forButton:(XAP_CocoaToolbarButton *)button;
@end

@interface XAP_CocoaToolbarButton : NSButton
{
	NSMenu *	m_menu;
	id <XAP_CocoaButtonController>	m_controller;
}
- (id)initWithFrame:(NSRect)frameRect;
- (void)setMenu:(NSMenu *)menu withController:(id <XAP_CocoaButtonController>)controller;
- (void)mouseDown:(NSEvent *)theEvent;
- (void)drawRect:(NSRect)aRect;
@end

class XAP_CocoaToolbar_Icons : public XAP_Toolbar_Icons
{
public:
	XAP_CocoaToolbar_Icons(void);
	~XAP_CocoaToolbar_Icons(void);

	NSImage *			getPixmapForIcon(const char * szIconID);  // the ID is an internal AbiWord identifier

	static NSString *	getPNGNameForIcon(const char * szIconID);
	static NSString *	getFilenameForIcon(NSString * szIconName); // e.g., the PNGName from the above method

protected:
};

#endif /* XAP_COCOATOOLBARICONS_H */
