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

@interface XAP_CocoaPluginImpl : NSObject
{
	id <NSObject, XAP_CocoaPluginDelegate>	m_delegate;
}
- (id)init;
- (void)dealloc;

- (void)setDelegate:(id <NSObject, XAP_CocoaPluginDelegate>)delegate;
- (id <NSObject, XAP_CocoaPluginDelegate>)delegate;
@end

@implementation XAP_CocoaPluginImpl

- (id)init
{
	if (self = [super init])
		{
			m_delegate = 0;
		}
	return self;
}

- (void)dealloc
{
	// 
	[super dealloc];
}

- (void)setDelegate:(id <NSObject, XAP_CocoaPluginDelegate>)delegate
{
	m_delegate = delegate;
}

- (id <NSObject, XAP_CocoaPluginDelegate>)delegate
{
	return m_delegate;
}

@end

@implementation XAP_CocoaPlugin

- (id)init
{
	if (self = [super init])
		{
			m_pImpl = [[XAP_CocoaPluginImpl alloc] init];
			if (!m_pImpl)
				{
					[self dealloc];
					self = 0;
				}
		}
	return self;
}

- (void)dealloc
{
	if (m_pImpl)
		{
			[m_pImpl release];
			m_pImpl = 0;
		}
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
						if ([instance respondsToSelector:@selector(pluginCanRegisterForAbiWord:)])
						{
							[self setDelegate:instance];
							bLoaded = [instance pluginCanRegisterForAbiWord:self];
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
	[m_pImpl setDelegate:delegate];
}

- (id <NSObject, XAP_CocoaPluginDelegate>)delegate
{
	return [m_pImpl delegate];
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

@end
