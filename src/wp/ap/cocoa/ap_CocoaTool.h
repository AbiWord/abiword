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

#ifndef AP_COCOATOOLPROVIDER_H
#define AP_COCOATOOLPROVIDER_H

#include "xap_CocoaToolProvider.h"

@interface AP_CocoaTool : NSObject <XAP_CocoaPlugin_Tool>
{
	NSString *	m_identifier;
	NSString *	m_description;
	NSString *	m_icon_name;

	id <XAP_CocoaPlugin_ToolProvider>	m_provider;

	unsigned	m_toolbarID;
}
+ (void)addStandardToolsToProvider:(XAP_CocoaToolProvider *)provider;

- (id)initWithIdentifier:(NSString *)identifier description:(NSString *)description iconName:(NSString *)iconName toolbarID:(unsigned)tlbrid;
- (void)dealloc;

- (NSString *)identifier;
- (NSString *)description;
- (NSString *)iconName; /* note: an internal ID, not an actual file name */

- (void)setProvider:(id <XAP_CocoaPlugin_ToolProvider>)provider;
- (id <XAP_CocoaPlugin_ToolProvider>)provider;

- (id <NSObject, XAP_CocoaPlugin_ToolInstance>)tool;
@end

@interface AP_CocoaToolInstance_StandardButton : NSObject <XAP_CocoaPlugin_ToolInstance>
{
	AP_CocoaTool *	m_tool;

	unsigned		m_toolbarID;

	NSButton *		m_button;
	NSMenuItem *	m_item;

	NSString *		m_defaultImage;
	NSString *		m_defaultAltImage;

	NSString *		m_configImage;
	NSString *		m_configAltImage;
}
- (id)initWithTool:(AP_CocoaTool *)tool toolbarID:(unsigned)tlbrid;
- (void)dealloc;

- (BOOL)validateMenuItem:(NSMenuItem *)menuItem;

- (IBAction)click:(id)sender;

/* XAP_CocoaPlugin_ToolInstance implementation
 */
- (id <NSObject, XAP_CocoaPlugin_Tool>)tool;

- (NSView *)toolbarButton;

- (NSMenuItem *)toolbarMenuItem;

- (NSString *)configWidth;
- (NSString *)configHeight;
- (NSString *)configImage;
- (NSString *)configAltImage;

- (void)setConfigWidth:(NSString *)width;
- (void)setConfigHeight:(NSString *)height;
- (void)setConfigImage:(NSString *)image;
- (void)setConfigAltImage:(NSString *)altImage;
@end

#endif /* ! AP_COCOATOOLPROVIDER_H */
