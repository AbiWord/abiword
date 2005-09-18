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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef AP_COCOATOOLPROVIDER_H
#define AP_COCOATOOLPROVIDER_H

#include "xap_CocoaToolProvider.h"

@interface AP_CocoaTool : NSObject <XAP_CocoaTool_Generic>
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

- (void)setProvider:(id <XAP_CocoaPlugin_ToolProvider>)provider;
- (id <XAP_CocoaPlugin_ToolProvider>)provider;

- (NSButton *)tool;
@end

#endif /* ! AP_COCOATOOLPROVIDER_H */
