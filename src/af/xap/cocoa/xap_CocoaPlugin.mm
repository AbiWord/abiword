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

#import "xap_CocoaPlugin.h"

@interface XAP_CocoaPluginImpl : NSObject
{
	id<XAP_CocoaPluginDelegate>	m_delegate;

	NSString *	m_path;
	NSString *	m_configurationFile;
}
- (id)initWithPath:(NSString *)path;
- (void)dealloc;
- (void)setDelegate:(id<XAP_CocoaPluginDelegate>)delegate;
- (id<XAP_CocoaPluginDelegate>)delegate;
- (NSString *)path;
- (NSString *)configurationFile;
- (void)configure:(NSString *)configurationFile;
@end

@implementation XAP_CocoaPluginImpl

- (id)initWithPath:(NSString *)path
{
	if (self = [super init])
		{
			m_delegate = 0;

			m_path = path;
			[m_path retain];

			m_configurationFile = 0;
		}
	return self;
}

- (void)dealloc
{
	if (m_path)
		{
			[m_path release];
			m_path = 0;
		}
	if (m_configurationFile)
		{
			[m_configurationFile release];
			m_configurationFile = 0;
		}
	[super dealloc];
}

- (void)setDelegate:(id<XAP_CocoaPluginDelegate>)delegate
{
	m_delegate = delegate;
}

- (id<XAP_CocoaPluginDelegate>)delegate
{
	return m_delegate;
}

- (NSString *)path
{
	return m_path;
}

- (NSString *)configurationFile
{
	return m_configurationFile;
}

- (void)configure:(NSString *)configurationFile
{
	if (m_configurationFile)
		{
			[m_configurationFile release];
			m_configurationFile = 0;
		}
	if (configurationFile)
		{
			m_configurationFile = configurationFile;
			[m_configurationFile retain];
		}
	if (m_delegate)
		{
			[m_delegate pluginHasConfigurationFile:m_configurationFile];
		}
}

@end

@implementation XAP_CocoaPlugin

- (id)initWithPath:(NSString *)path
{
	if (self = [super init])
		{
			m_pImpl = [[XAP_CocoaPluginImpl alloc] initWithPath:path];
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

- (void)setDelegate:(id)delegate
{
	[m_pImpl setDelegate:((id<XAP_CocoaPluginDelegate>) delegate)];
}

- (id)delegate
{
	return [m_pImpl delegate];
}

- (NSString *)path
{
	return [m_pImpl path];
}

- (NSString *)configurationFile
{
	return (m_pImpl ? [m_pImpl configurationFile] : nil);
}

- (void)configure:(NSString *)configurationFile
{
	if (m_pImpl)
		[m_pImpl configure:configurationFile];
}

@end
