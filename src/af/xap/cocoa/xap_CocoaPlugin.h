/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 2004-2005 Francis James Franklin
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

#ifndef XAP_COCOAPLUGIN_H
#define XAP_COCOAPLUGIN_H

#import <Cocoa/Cocoa.h>

@class XAP_CocoaPlugin;
@class XAP_CocoaPluginImpl;

/* Please bear in mind that documents can be closed, so don't keep references to documents
 * lying around for future use.
 */
@protocol XAP_CocoaPlugin_Document
- (BOOL)documentStillExists;

- (NSString *)title;

/* Mail Merge interface
 */
- (NSString *)documentMailMergeSource; // may return nil
- (void)setDocumentMailMergeSource:(NSString *)path;

- (void)insertDocumentMailMergeField:(NSString *)field_name;

- (NSArray *)documentMailMergeFields;
- (void)setDocumentMailMergeFields:(NSArray *)field_array;
- (void)unsetDocumentMailMergeFields;

/* value_dictionary maps (NSString *) keys [field names] to (NSString *) values.
 */
- (void)setDocumentMailMergeValues:(NSDictionary *)value_dictionary;
@end

@protocol XAP_CocoaPluginDelegate

/* This will be called immediately after initialization;
 * return NO if for some reason the plugin won't work with the current version of AbiWord.
 * 
 * If the bundle's main class implements *only* this method, it must immediately set the
 * real XAP_CocoaPluginDelegate object as AbiWord's delegate.
 */
- (BOOL)pluginCanRegisterForAbiWord:(XAP_CocoaPlugin *)AbiWord;

- (BOOL)pluginIsActive;
- (void)pluginActivate;   /* Add menuitems etc. */
- (void)pluginDeactivate; /* Remove them...     */

/* Plugin should attempt to save all files, and return NO *only* if the user has answered
 * "Cancel" to an alert about an unsaved document.
 */
- (BOOL)pluginCanDeactivate;

/* AbiWord will call this if the focus changes to a different document, or different window
 * or panel, or if the current document is closed.
 */
- (void)pluginCurrentDocumentHasChanged;

- (NSString *)pluginName;
- (NSString *)pluginAuthor;
- (NSString *)pluginVersion;
- (NSString *)pluginDescription;
- (NSString *)pluginUsage;

@end

@interface XAP_CocoaPlugin : NSObject
{
	id <NSObject, XAP_CocoaPluginDelegate>	m_delegate;
}
- (id)init;
- (void)dealloc;

/* .Abi plugins are bundles; loadBundle attempts to load the bundle specified to initWithPath,
 * returns NO if the bundle is already loaded (to avoid duplication) or if it can't be loaded.
 * The plugin's principal class is initialized with init and is set as the delegate, and must
 * conform therefore to the XAP_CocoaPluginDelegate protocol (or set a new delegate when
 * pluginCanRegisterForAbiWord is called).
 */
- (BOOL)loadBundleWithPath:(NSString *)path;

- (void)setDelegate:(id <NSObject, XAP_CocoaPluginDelegate>)delegate;
- (id <NSObject, XAP_CocoaPluginDelegate>)delegate;

- (void)appendMenuItem:(NSMenuItem *)menuItem;
- (void)removeMenuItem:(NSMenuItem *)menuItem;

/* Please bear in mind that documents can be closed, so don't keep references to documents
 * lying around for future use.
 */
- (id <NSObject, XAP_CocoaPlugin_Document>)currentDocument; // may return nil;
- (NSArray *)documents;                                     // array of id <XAP_CocoaPlugin_Document>

- (NSString *)selectMailMergeSource; // may return nil

/* Returns an NSMutableArray whose objects are NSMutableArray of NSString, the first row holding the
 * field names, the rest being records; returns nil on failure.
 */
- (NSMutableArray *)importMailMergeSource:(NSString *)path;
@end

#endif /* XAP_COCOAPLUGIN_H */
