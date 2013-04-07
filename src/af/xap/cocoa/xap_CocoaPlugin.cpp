/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 2004 Francis James Franklin
 * Copyright (C) 2007 Hubert Figuiere
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

#include "config.h"

#include "ut_string_class.h"
#include "ut_xml.h"

#include "xap_CocoaApp.h"
#include "xap_CocoaAppController.h"
#include "xap_CocoaPlugin.h"

#include "ap_CocoaPlugin.h"

class s_SimpleXML_Listener : public UT_XML::Listener
{
private:
	id <XAP_CocoaPlugin_SimpleXML>	m_callback;

	NSString *	m_error;

	UT_XML		m_parser;

public:
	s_SimpleXML_Listener (id <XAP_CocoaPlugin_SimpleXML> callback) :
		m_callback(callback),
		m_error(0)
		{
			m_parser.setListener(this);
		}

	virtual ~s_SimpleXML_Listener ()
		{
			if (m_error)
			{
				[m_error release];
				m_error = 0;
			}
		}

	NSString * parse (NSString * path)
		{
			if (m_parser.parse ([path UTF8String]) != UT_OK)
			{
				if (!m_error)
				{
					m_error = [NSString stringWithFormat:@"Error while parsing \"%@\"", path];
					[m_error retain];
				}
			}
			return m_error;
		}

	virtual void startElement (const gchar * element_name, const gchar ** atts)
		{
			NSMutableDictionary * dictionary = [NSMutableDictionary dictionaryWithCapacity:4];

			NSString * name = [NSString stringWithUTF8String:element_name];

			const gchar ** attr = atts;

			while (*attr)
			{
				const gchar * key   = *attr++;
				const gchar * value = *attr++;

				if (*key && value)
				{
					[dictionary setObject:[NSString stringWithUTF8String:value] forKey:[NSString stringWithUTF8String:key]];
				}
			}
			if (![m_callback startElement:name attributes:dictionary])
			{
				m_parser.stop();

				m_error = @"Simple XML parser stopped by callback method \"startElement:attributes:\".";
				[m_error retain];
			}
		}

	virtual void endElement (const gchar * element_name)
		{
			NSString * name = [NSString stringWithUTF8String:element_name];

			if (![m_callback endElement:name])
			{
				m_parser.stop();

				m_error = @"Simple XML parser stopped by callback method \"endElement:\".";
				[m_error retain];
			}
		}

	virtual void charData (const gchar * buffer, int length)
		{
			UT_UTF8String data(buffer,length);
			// FIXME: remove the UT_UTF8String from here
			if (![m_callback characterData:[NSString stringWithUTF8String:(data.utf8_str())]])
			{
				m_parser.stop();

				m_error = @"Simple XML parser stopped by callback method \"characterData:\".";
				[m_error retain];
			}
		}
};

@implementation XAP_CocoaPlugin

- (id)init
{
	if (![super init])
	{
		return nil;
	}
	m_delegate = nil;
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

	if (NSBundle * bundle = [NSBundle bundleWithPath:path]) {
		if (![bundle isLoaded]) {
			if ([bundle load]) {
				if (Class bundleClass = [bundle principalClass]) {
					if (id <NSObject, XAP_CocoaPluginDelegate> instance = [[bundleClass alloc] init])
					{
						if ([instance respondsToSelector:@selector(pluginCanRegisterForAbiWord:version:interface:)])
						{
							[self setDelegate:instance];
							unsigned long interface = XAP_COCOAPLUGIN_INTERFACE;
							NSString * version = [NSString stringWithUTF8String:VERSION];
							bLoaded = [instance pluginCanRegisterForAbiWord:self version:version interface:interface];
						}
						if (!bLoaded)
						{
							[instance release];
						}
					}
				}
			}
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

- (NSArray *)toolProviders
{
	XAP_CocoaAppController * pController = (XAP_CocoaAppController *) [NSApp delegate];
	return [pController toolProviders];
}

- (id <NSObject, XAP_CocoaPlugin_ToolProvider>)toolProvider:(NSString *)name
{
	XAP_CocoaAppController * pController = (XAP_CocoaAppController *) [NSApp delegate];
	return [pController toolProvider:name];
}

- (NSString *)findResourcePath:(NSString *)relativePath
{
	XAP_CocoaApp * pApp = static_cast<XAP_CocoaApp *>(XAP_App::getApp());

	NSString * resource_path = 0;

	if (relativePath)
	{
		if ([relativePath length])
		{
			std::string path;

			if (pApp->findAbiSuiteLibFile(path, [relativePath UTF8String]))
			{
				resource_path = [NSString stringWithUTF8String:(path.c_str())];
			}
		}
	}
	return resource_path;
}

- (NSString *)userResourcePath:(NSString *)relativePath
{
	XAP_CocoaApp * pApp = static_cast<XAP_CocoaApp *>(XAP_App::getApp());

	NSString * resource_path = [NSString stringWithUTF8String:(pApp->getUserPrivateDirectory())];

	if (relativePath) {
		if ([relativePath length]) {
			resource_path = [resource_path stringByAppendingPathComponent:relativePath];
		}
	}

	return resource_path;
}

- (NSString *)parseFile:(NSString *)path simpleXML:(id <XAP_CocoaPlugin_SimpleXML>)callback
{
	NSString * error = 0;

	if (path && callback)
	{
		s_SimpleXML_Listener parser(callback);
		error = parser.parse(path);
	}
	else
	{
		error = @"Method \"parseFile:simpleXML:\" requires path and callback!";
	}
	return error;
}

@end
