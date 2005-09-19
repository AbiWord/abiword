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

#include <stdio.h>

#include "ut_assert.h"

#include "xap_App.h"
#include "xap_Toolbar_LabelSet.h"

#include "ev_Toolbar_Labels.h"

#include "ap_CocoaTool.h"
#include "ap_Prefs_SchemeIds.h"
#include "ap_Toolbar_Id.h"

static void s_addToolToProvider (XAP_CocoaToolProvider * provider, EV_Toolbar_Label * pLabel, const char * szIdentifier, unsigned tlbrid)
{
	NSString * identifier  = [NSString stringWithUTF8String:szIdentifier];
	NSString * description = [NSString stringWithUTF8String:(pLabel->getToolTip())];
	NSString * icon_name   = [NSString stringWithUTF8String:(pLabel->getIconName())];

	AP_CocoaTool * tool = [[AP_CocoaTool alloc] initWithIdentifier:identifier description:description iconName:icon_name toolbarID:tlbrid];

	[provider addTool:tool];

	[tool release];
}

static void s_addToolsToProvider (XAP_CocoaToolProvider * provider)
{
	const char * szToolbarLabelSetKey          = AP_PREF_KEY_StringSet;
	const char * szToolbarLabelSetDefaultValue = AP_PREF_DEFAULT_StringSet;

	const char * szToolbarLabelSetName = NULL;

	XAP_App * pApp = XAP_App::getApp();

	if ((pApp->getPrefsValue(szToolbarLabelSetKey, static_cast<const XML_Char **>(&szToolbarLabelSetName))) && (szToolbarLabelSetName) && (*szToolbarLabelSetName))
		;
	else
		szToolbarLabelSetName = szToolbarLabelSetDefaultValue;

	EV_Toolbar_LabelSet * toolbarLabelSet = AP_CreateToolbarLabelSet(szToolbarLabelSetName);
	UT_ASSERT(toolbarLabelSet);
	if (!toolbarLabelSet)
	{
		return;
	}

#ifdef toolbaritem
#undef toolbaritem
#endif
#define toolbaritem(id) s_addToolToProvider(provider, \
											toolbarLabelSet->getLabel(AP_TOOLBAR_ID_##id), \
											#id, \
											static_cast<unsigned>(AP_TOOLBAR_ID_##id));

#include "ap_Toolbar_Id_List.h"

	DELETEP(toolbarLabelSet);

	// TODO: this isn't all available tools, though; just the buttons...
}

static id <NSObject, XAP_CocoaPlugin_ToolInstance> s_toolInstance (NSString * identifier, NSString * description, NSString * iconName, unsigned toolbarID);

@implementation AP_CocoaTool

+ (void)addStandardToolsToProvider:(XAP_CocoaToolProvider *)provider
{
	s_addToolsToProvider (provider);
}

- (id)initWithIdentifier:(NSString *)identifier description:(NSString *)description iconName:(NSString *)iconName toolbarID:(unsigned)tlbrid
{
	if (self = [super init])
	{
		m_identifier = identifier;
		[m_identifier retain];

		m_description = description;
		[m_description retain];

		m_icon_name = iconName;
		[m_icon_name retain];

		m_toolbarID = tlbrid;

		m_provider = nil;
	}
	return self;
}

- (void)dealloc
{
	[m_identifier  release];
	[m_description release];
	[m_icon_name   release];
	[super dealloc];
}

- (NSString *)identifier
{
	return m_identifier;
}

- (NSString *)description
{
	return m_description;
}

- (void)setProvider:(id <XAP_CocoaPlugin_ToolProvider>)provider
{
	m_provider = provider;
}

- (id <XAP_CocoaPlugin_ToolProvider>)provider
{
	return m_provider;
}

- (id <NSObject, XAP_CocoaPlugin_ToolInstance>)tool
{
	if (!m_provider || !m_description)
	{
		return nil;
	}

	id <NSObject, XAP_CocoaPlugin_ToolInstance> instance = nil;

	switch (m_toolbarID)
	{
		// TODO: special cases

	default:
		{
			AP_CocoaToolInstance_StandardButton * SB = [[AP_CocoaToolInstance_StandardButton alloc] initWithTool:self toolbarID:m_toolbarID];

			instance = SB;

			[SB autorelease];
		}
		break;
	}
	return instance;
}

@end

@implementation AP_CocoaToolInstance_StandardButton

- (id)initWithTool:(AP_CocoaTool *)tool toolbarID:(unsigned)tlbrid
{
	if (self = [super init])
		{
			m_tool = tool;

			m_toolbarID = tlbrid;
		}
	return self;
}

- (void)dealloc
{
	// ...
	[super dealloc];
}

- (IBAction)click:(id)sender
{
	// TODO
}

/* XAP_CocoaPlugin_ToolInstance implementation
 */
- (id <NSObject, XAP_CocoaPlugin_Tool>)tool
{
	return m_tool;
}

- (NSButton *)toolbarButton
{
	// TODO
	return nil;
}

- (NSMenuItem *)toolbarMenuItem
{
	// TODO
	return nil;
}

- (NSString *)configWidth
{
	return @"auto";
}

- (NSString *)configHeight
{
	return @"auto";
}

- (NSString *)configImage
{
	// TODO
	return @"auto";
}

- (NSString *)configAltImage
{
	// TODO
	return @"auto";
}

- (void)setConfigWidth:(NSString *)width
{
	// ...
}

- (void)setConfigHeight:(NSString *)height;
{
	// ...
}

- (void)setConfigImage:(NSString *)image;
{
	// TODO
}

- (void)setConfigAltImage:(NSString *)altImage;
{
	// TODO
}

@end
