/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001, 2003, 2009 Hubert Figuiere
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
 
#include <stdio.h>
#include <string.h>

#include "ut_color.h"
#include "ut_assert.h"
#include "ut_string.h"

#include "xap_CocoaApp.h"
#include "xap_CocoaToolbar_Icons.h"

static NSPoint s_ButtonOnPoint[12] = {
	{	 0.0f,	 7.0f	},
	{	 0.0f,	 0.0f	},
	{	 7.0f,	 0.0f	},
	{	19.0f,	 0.0f	},
	{	26.0f,	 0.0f	},
	{	26.0f,	 7.0f	},
	{	26.0f,	19.0f	},
	{	26.0f,	26.0f	},
	{	19.0f,	26.0f	},
	{	 7.0f,	26.0f	},
	{	 0.0f,	26.0f	},
	{	 0.0f,	19.0f	}
};

static NSPoint s_ButtonMenuPoint[3] = {
	{	26.0f,	21.0f	},
	{	20.0f,	21.0f	},
	{	23.0f,	26.0f	}
};

@implementation XAP_CocoaToolbarButton

- (id)initWithFrame:(NSRect)frameRect
{
	if (![super initWithFrame:frameRect]) {
		return nil;
	}
	m_menu = nil;
	m_controller = nil;
	return self;
}

- (void)setMenu:(NSMenu *)menu withController:(id <XAP_CocoaButtonController>)controller
{
	m_menu = menu;
	m_controller = controller;
}

- (void)mouseDown:(NSEvent *)theEvent
{
	if (m_menu && [self isEnabled])
	{
		if (m_controller)
		{
			[m_controller menuWillActivate:m_menu forButton:self];
		}
		[NSMenu popUpContextMenu:m_menu withEvent:theEvent forView:self];
	}
	else
	{
		[super mouseDown:theEvent];
	}
}

- (void)drawRect:(NSRect)aRect
{
	if ([self state] == NSOnState)
	{
		[[NSColor colorWithCalibratedWhite:0.0f alpha:0.25] set];

		NSBezierPath * path = [NSBezierPath bezierPath];

		[path  moveToPoint:s_ButtonOnPoint[ 0]];
		[path curveToPoint:s_ButtonOnPoint[ 2] controlPoint1:s_ButtonOnPoint[ 1] controlPoint2:s_ButtonOnPoint[ 1]];
		[path  lineToPoint:s_ButtonOnPoint[ 3]];
		[path curveToPoint:s_ButtonOnPoint[ 5] controlPoint1:s_ButtonOnPoint[ 4] controlPoint2:s_ButtonOnPoint[ 4]];
		[path  lineToPoint:s_ButtonOnPoint[ 6]];
		[path curveToPoint:s_ButtonOnPoint[ 8] controlPoint1:s_ButtonOnPoint[ 7] controlPoint2:s_ButtonOnPoint[ 7]];
		[path  lineToPoint:s_ButtonOnPoint[ 9]];
		[path curveToPoint:s_ButtonOnPoint[11] controlPoint1:s_ButtonOnPoint[10] controlPoint2:s_ButtonOnPoint[10]];
		[path closePath];
		[path fill];
	}
	[super drawRect:aRect];

	if (m_menu)
	{
		[[NSColor blackColor] set];

		NSBezierPath * path = [NSBezierPath bezierPath];

		[path moveToPoint:s_ButtonMenuPoint[0]];
		[path lineToPoint:s_ButtonMenuPoint[1]];
		[path lineToPoint:s_ButtonMenuPoint[2]];
		[path closePath];
		[path fill];
	}
}

@end

XAP_CocoaToolbar_Icons::XAP_CocoaToolbar_Icons(void)
{
}

XAP_CocoaToolbar_Icons::~XAP_CocoaToolbar_Icons(void)
{
	// TODO do we need to keep some kind of list
	// TODO of the things we have created and
	// TODO handed out, so that we can delete them ??
}

NSString * XAP_CocoaToolbar_Icons::getPNGNameForIcon(const char * szIconID)
{
	const char * szIconName = NULL;

	if (XAP_Toolbar_Icons::_findIconNameForID(szIconID, &szIconName))
	{
		UT_UTF8String name(szIconName);

		if (char * suffix = strstr(szIconName, "_xpm"))
		{
			name.assign(szIconName, suffix - szIconName);
		}
		name += ".png";

		return [NSString stringWithUTF8String:(name.utf8_str())];
	}

	UT_ASSERT_NOT_REACHED();
	return nil;
}

NSString * XAP_CocoaToolbar_Icons::getFilenameForIcon(NSString * iconName)
{
	NSString * filename = iconName;

	if (iconName)
	{
		XAP_CocoaApp * pApp = static_cast<XAP_CocoaApp *>(XAP_App::getApp());

		std::string path;

		if (pApp->findAbiSuiteLibFile(path, [iconName UTF8String], "ToolbarIcons")) // I'd love to do this inside the bundle but Cocoa gets confused if I try
		{
			filename = [NSString stringWithUTF8String:(path.c_str())];
		}
		else if (pApp->findAbiSuiteBundleFile(path, [iconName UTF8String]))
		{
			filename = [NSString stringWithUTF8String:(path.c_str())];
		}
	}
	return filename;
}


/*!
	returns the pixmap for the named icon
	
	\param szIconName the name of the icon
	\return the newly allocated NSImage [autoreleased]
 */
NSImage * XAP_CocoaToolbar_Icons::getPixmapForIcon(const char * szIconID)
{
	UT_ASSERT(szIconID && *szIconID);

	NSImage * pixmap = nil;

	NSString * path = XAP_CocoaToolbar_Icons::getFilenameForIcon(XAP_CocoaToolbar_Icons::getPNGNameForIcon(szIconID));

	if (path)
	{
		pixmap = [[NSImage alloc] initWithContentsOfFile:path];

		if (pixmap)
		{
			[pixmap autorelease];
		}
		else
		{
			UT_ASSERT(pixmap);
			pixmap = [NSImage imageNamed:@"NSApplicationIcon"];
		}
	}
	else
	{
		UT_ASSERT_NOT_REACHED();
	}
	return pixmap;
}

