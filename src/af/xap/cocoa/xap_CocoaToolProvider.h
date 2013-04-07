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

#ifndef XAP_COCOATOOLPROVIDER_H
#define XAP_COCOATOOLPROVIDER_H

#import <Cocoa/Cocoa.h>

#include "xap_CocoaPlugin.h"

@interface XAP_CocoaToolProvider : NSObject <XAP_CocoaPlugin_ToolProvider>
{
	NSString *	m_name;

	NSMutableArray *		m_identifiers;
	NSMutableDictionary *	m_tools;
}
+ (XAP_CocoaToolProvider *)AbiWordToolProvider; // don't use this method; use XAP_CocoaAppController's toolProvider:@"AbiWord"

- (id)initWithName:(NSString *)name;
- (void)dealloc;

/**
 * Get the name of the provider.
 *
 * \return The name identifying the provider.
 */
- (NSString *)name;

/**
 * Add a tool to the provider's list of tools. The provider will send the tool a setProvider: message.
 *
 * \param tool An object which implements the XAP_CocoaPlugin_Tool protocol.
 */
- (void)addTool:(id <NSObject, XAP_CocoaPlugin_Tool>)tool;

/**
 * Remove a tool from the provider's list of tools. The provider will send the tool a setProvider:nil message.
 *
 * \param identifier The identifier of the tool which is to be removed.
 */
- (void)removeToolWithIdentifier:(NSString *)identifier;

/**
 * Get the tool with the specified identifier.
 *
 * \param identifier The identifier of the tool which is desired.
 *
 * \return The specified tool, or nil if the identifier is not matched.
 */
- (id <NSObject, XAP_CocoaPlugin_Tool>)toolWithIdentifier:(NSString *)identifier;

/**
 * Get the identifiers of the tools provided.
 *
 * \return The identifiers of the tools provided.
 */
- (NSArray *)toolIdentifiers;

/**
 * See whether the provider provides a specific tool, and get the description (tooltip).
 *
 * \param identifier The internal identifier of the desired tool.
 *
 * \return The description (the tooltip) of the tool if the identifier is recognized, otherwise nil.
 */
- (NSString *)toolDescription:(NSString *)identifier;

@end

#endif /* ! XAP_COCOATOOLPROVIDER_H */
