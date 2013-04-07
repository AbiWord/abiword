/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 2005 Francis James Franklin
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

#include "ut_assert.h"

#include "xap_CocoaToolProvider.h"
#include "ap_CocoaTool.h"

@implementation XAP_CocoaToolProvider

+ (XAP_CocoaToolProvider *)AbiWordToolProvider
{
	XAP_CocoaToolProvider * provider = [[XAP_CocoaToolProvider alloc] initWithName:@"AbiWord"];

	[AP_CocoaTool addStandardToolsToProvider:provider];

	[provider autorelease];

	return provider;
}

- (id)initWithName:(NSString *)name
{
	if (![super init])
	{
		return nil;
	}
	UT_ASSERT(name);

	m_name = name;
	[m_name retain];

	m_identifiers = [[NSMutableArray      alloc] initWithCapacity:16];
	m_tools       = [[NSMutableDictionary alloc] initWithCapacity:16];
	return self;
}

- (void)dealloc
{
	[m_name release];
	[m_identifiers release];
	[m_tools release];
	[super dealloc];
}

/**
 * Get the name of the provider.
 * 
 * \return The name identifying the provider.
 */
- (NSString *)name
{
	return m_name;
}

/**
 * Add a tool to the provider's list of tools. The provider will send the tool a setProvider: message.
 * 
 * \param tool An object which implements the XAP_CocoaPlugin_Tool protocol.
 */
- (void)addTool:(id <NSObject, XAP_CocoaPlugin_Tool>)tool
{
	NSString * identifier = [tool identifier];

	if ([m_identifiers containsObject:identifier] == NO)
	{
		[m_identifiers addObject:identifier];

		[m_tools setObject:tool forKey:identifier];

		[tool setProvider:self];
	}
}

/**
 * Remove a tool from the provider's list of tools. The provider will send the tool a setProvider:nil message.
 * 
 * \param identifier The identifier of the tool which is to be removed.
 */
- (void)removeToolWithIdentifier:(NSString *)identifier
{
	if ([m_identifiers containsObject:identifier])
	{
		id <NSObject, XAP_CocoaPlugin_Tool> tool = (id <NSObject, XAP_CocoaPlugin_Tool>) [m_tools objectForKey:identifier];

		[m_identifiers removeObject:identifier];

		[m_tools removeObjectForKey:identifier];

		if (tool)
		{
			[tool setProvider:nil];
		}
	}
}

/**
 * Get the tool with the specified identifier.
 * 
 * \param identifier The identifier of the tool which is desired.
 * 
 * \return The specified tool, or nil if the identifier is not matched.
 */
- (id <NSObject, XAP_CocoaPlugin_Tool>)toolWithIdentifier:(NSString *)identifier
{
	id <NSObject, XAP_CocoaPlugin_Tool> tool = (id <NSObject, XAP_CocoaPlugin_Tool>) [m_tools objectForKey:identifier];

	return tool;
}

/**
 * Get the identifiers of the tools provided.
 * 
 * \return The identifiers of the tools provided.
 */
- (NSArray *)toolIdentifiers
{
	return m_identifiers;
}

/**
 * See whether the provider provides a specific tool, and get the description (tooltip).
 * 
 * \param identifier The internal identifier of the desired tool.
 * 
 * \return The description (the tooltip) of the tool if the identifier is recognized, otherwise nil.
 */
- (NSString *)toolDescription:(NSString *)identifier
{
	id <NSObject, XAP_CocoaPlugin_Tool> tool = (id <NSObject, XAP_CocoaPlugin_Tool>) [m_tools objectForKey:identifier];

	return tool ? [tool description] : 0;
}

@end
