/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include "xap_CocoaAppController.h"
#include "xap_CocoaPlugin.h"

#include "ap_CocoaPlugin.h"

@implementation XAP_CocoaPlugin

- (id)init
{
	if (self = [super init])
		{
			m_delegate = nil;
		}
	return self;
}

- (void)dealloc
{
	// 
	[super dealloc];
}

- (BOOL)loadBundleWithPath:(NSString *)path
{
	BOOL bLoaded = NO;

	if (NSBundle * bundle = [NSBundle bundleWithPath:path])
		if (![bundle isLoaded])
			if ([bundle load])
				if (Class bundleClass = [bundle principalClass])
					if (id <NSObject, XAP_CocoaPluginDelegate> instance = [[bundleClass alloc] init])
					{
						if ([instance respondsToSelector:@selector(pluginCanRegisterForAbiWord:version:interface:)])
						{
							[self setDelegate:instance];
							unsigned long interface = XAP_COCOAPLUGIN_INTERFACE;
							NSString * version = [NSString stringWithUTF8String:ABI_BUILD_VERSION];
							bLoaded = [instance pluginCanRegisterForAbiWord:self version:version interface:interface];
						}
						if (!bLoaded)
						{
							[instance release];
						}
					}
	return bLoaded;
}

- (void)setDelegate:(id <NSObject, XAP_CocoaPluginDelegate>)delegate
{
	m_delegate = delegate;
}

- (id <NSObject, XAP_CocoaPluginDelegate>)delegate
{
	return m_delegate;
}

- (void)appendMenuItem:(NSMenuItem *)menuItem
{
	XAP_CocoaAppController * pController = (XAP_CocoaAppController *) [NSApp delegate];
	[pController appendPluginMenuItem:menuItem];
}

- (void)removeMenuItem:(NSMenuItem *)menuItem
{
	XAP_CocoaAppController * pController = (XAP_CocoaAppController *) [NSApp delegate];
	[pController removePluginMenuItem:menuItem];
}

- (id <NSObject, XAP_CocoaPlugin_Document>)currentDocument // may return nil;
{
	return [AP_CocoaPlugin_Document currentDocument];
}

- (NSArray *)documents
{
	return [AP_CocoaPlugin_Document documents];
}

- (NSString *)selectMailMergeSource // may return nil
{
	return [AP_CocoaPlugin_Document selectMailMergeSource];
}

/* Returns an NSMutableArray whose objects are NSMutableArray of NSString, the first row holding the
 * field names, the rest being records; returns nil on failure.
 */
- (NSMutableArray *)importMailMergeSource:(NSString *)path
{
	return [AP_CocoaPlugin_Document importMailMergeSource:path];
}

- (id <NSObject, XAP_CocoaPlugin_FramelessDocument>)importDocumentFromFile:(NSString *)path importOptions:(NSDictionary *)options
{
	return [AP_CocoaPlugin_FramelessDocument documentFromFile:path importOptions:options];
}

- (id <NSObject, XAP_CocoaPlugin_MenuItem>)contextMenuItemWithLabel:(NSString *)label
{
	return [AP_CocoaPlugin_ContextMenuItem itemWithLabel:label];
}

@end
