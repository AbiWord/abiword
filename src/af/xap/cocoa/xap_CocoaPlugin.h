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

/**
 * \protocol XAP_CocoaPlugin_FramelessDocument XAP_CocoaPlugin.h "XAP_CocoaPlugin.h"
 * 
 * Plug-in API for manipulating documents which do not have windows. Typically these
 * loaded from a file. See XAP_CocoaPlugin's importDocumentFromFile:importOptions: method.
 */
@protocol XAP_CocoaPlugin_FramelessDocument

/** 
 * Save the document in the document file format suggested by the path's extension
 * (e.g., .html, .txt).
 * 
 * \param path    The output file path. This should have an appropriate extension.
 * \param options A dictionary mapping of NSString keys to NSString values; these
 *                options are passed to the exporter. May be nil.
 * 
 * \return Returns YES on success, NO on failure.
 */
- (BOOL)exportDocumentToFile:(NSString *)path exportOptions:(NSDictionary *)options;

@end

/**
 * \protocol XAP_CocoaPlugin_Document XAP_CocoaPlugin.h "XAP_CocoaPlugin.h"
 * 
 * Plug-in API for manipulating documents which are on-screen. The current document, if any,
 * or a list of available documents can be obtained via XAP_CocoaPlugin's currentDocument
 * and documents methods.
 * 
 * Please bear in mind that documents can be closed, so if you keep these references to
 * documents lying around for future use then you should check that the documents they refer
 * to still exist using the documentStillExists method.
 * 
 * This is really a reference to a document window, and not to the specific document itself.
 */
@protocol XAP_CocoaPlugin_Document

/**
 * It is quite possible for document windows to be closed by the user. If you retain document
 * references then it's a good idea to check that the document window still exists.
 * 
 * \return Returns YES if the document window still exists; otherwise NO.
 */
- (BOOL)documentStillExists;

/**
 * \return The title of the document, or, if the window no longer exists, the string
 *         "(This document no longer exists!)".
 */
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

/**
 * \protocol XAP_CocoaPluginDelegate XAP_CocoaPlugin.h "XAP_CocoaPlugin.h"
 * 
 * In order for AbiWord to interact with the plug-in, the plug-in needs to implement
 * this protocol. As a minimum, the bundle's principal class must implement the
 * NSObject protocol and the XAP_CocoaPluginDelegate's pluginCanRegisterForAbiWord: method.
 */
@protocol XAP_CocoaPluginDelegate

/**
 * This will be called immediately after initialization;
 * 
 * If the bundle's principal class implements *only* this method, it must immediately set
 * the real XAP_CocoaPluginDelegate object as AbiWord's delegate using XAP_CocoaPlugin's
 * setDelegate: method.
 * 
 * The plug-in should not activate - wait for XAP_CocoaPluginDelegate's pluginActivate:
 * method to be called.
 * 
 * \param AbiWord The main interface to AbiWord.
 * 
 * \return Should return NO if for some reason the plug-in won't work with the current
 *         version of AbiWord.
 */
- (BOOL)pluginCanRegisterForAbiWord:(XAP_CocoaPlugin *)AbiWord;

/**
 * \return Should return YES if the plug-in is active; otherwise NO.
 */
- (BOOL)pluginIsActive;

/**
 * This is a request for the plug-in to activate.
 */
- (void)pluginActivate;   /* Add menuitems etc. */

/**
 * This is a request for the plug-in to deactivate. The plug-in *must* comply.
 */
- (void)pluginDeactivate; /* Remove them...     */

/**
 * The plug-in should attempt to save all files, etc., since it is about to be asked to
 * deactivate. Returning NO will not necessarily prevent the request to deactivate.
 * 
 * \return Should return NO *only* if the user has answered "Cancel" to an alert about
 *         an unsaved document; otherwise YES.
 */
- (BOOL)pluginCanDeactivate;

/**
 * AbiWord will call this if the focus changes to a different document, or different window
 * or panel, or if the current document is closed.
 */
- (void)pluginCurrentDocumentHasChanged;

/**
 * \return The name of the plug-in.
 */
- (NSString *)pluginName;

/**
 * \return The name(s) of the plug-in author(s).
 */
- (NSString *)pluginAuthor;

/**
 * \return The version of the plug-in.
 */
- (NSString *)pluginVersion;

/**
 * \return A short description of what the plug-in does.
 */
- (NSString *)pluginDescription;

/**
 * \return A short description of how to use the plug-in.
 */
- (NSString *)pluginUsage;

@end

/**
 * \class XAP_CocoaPlugin XAP_CocoaPlugin.h "XAP_CocoaPlugin.h"
 * 
 * This class provides the main interface between the plug-in and AbiWord.
 */
@interface XAP_CocoaPlugin : NSObject
{
	id <NSObject, XAP_CocoaPluginDelegate>	m_delegate;
}

/**
 * (For internal use only.)
 */
- (id)init;

/**
 * (For internal use only.)
 */
- (void)dealloc;

/**
 * (For internal use only.)
 * 
 * .Abi plug-ins are bundles; loadBundle attempts to load the bundle specified to initWithPath,
 * returns NO if the bundle is already loaded (to avoid duplication) or if it can't be loaded.
 * The plug-in's principal class is initialized with init and is set as the delegate, and must
 * conform therefore to the XAP_CocoaPluginDelegate protocol (or set a new delegate when
 * pluginCanRegisterForAbiWord: is called).
 */
- (BOOL)loadBundleWithPath:(NSString *)path;

/**
 * If the plug-in bundle's principal class does not implement the XAP_CocoaPluginDelegate
 * protocol, then, when the pluginCanRegisterForAbiWord: method is called, it should set a new
 * delegate object which does implement the protocol.
 * 
 * \param delegate The new delegate (must not be nil). The delegate is not retained.
 * 
 * \see XAP_CocoaPluginDelegate
 */
- (void)setDelegate:(id <NSObject, XAP_CocoaPluginDelegate>)delegate;

/**
 * \return The delegate object - the bundle's principal class, unless the plug-in has
 *         substituted another.
 */
- (id <NSObject, XAP_CocoaPluginDelegate>)delegate;

/**
 * Add a menu item to the end of the Tools menu.
 * 
 * \param menuItem The menu item to add to the Tools menu.
 */
- (void)appendMenuItem:(NSMenuItem *)menuItem;

/**
 * Remove a menu item from the end of the Tools menu.
 * 
 * \param menuItem The menu item to remove from the Tools menu.
 */
- (void)removeMenuItem:(NSMenuItem *)menuItem;

/**
 * Please bear in mind that documents can be closed, so be careful when keeping references
 * to documents lying around for future use.
 * 
 * Note: These are really references to document windows, and sometimes the actual documents
 *       can change even though the document window is the same.
 * 
 * \return Returns a reference to the currently active document window; or nil if none.
 */
- (id <NSObject, XAP_CocoaPlugin_Document>)currentDocument; // may return nil;

/**
 * Please bear in mind that documents can be closed, so be careful when keeping references
 * to documents lying around for future use.
 * 
 * Note: These are really references to document windows, and sometimes the actual documents
 *       can change even though the document window is the same.
 * 
 * \return Returns an array of references to the available document windows.
 */
- (NSArray *)documents;                                     // array of id <NSObject, XAP_CocoaPlugin_Document>

/**
 * Pops up a File Open dialog for the user to select a source file for mail merge.
 * 
 * \return Returns the path of the selected mail merge source file; or nil if none selected.
 */
- (NSString *)selectMailMergeSource; // may return nil

/**
 * Loads a mail merge source file and builds an array of arrays containing the fields.
 * 
 * \param path The file name of the mail merge source file.
 * 
 * \return Returns an NSMutableArray whose objects are NSMutableArray of NSString, the first
 *         row holding the field names, the rest being records; returns nil on failure.
 */
- (NSMutableArray *)importMailMergeSource:(NSString *)path;

/**
 * Open the specified document without creating a document window. This can be useful for
 * automated document processing (printing, conversion, etc.).
 * 
 * \param path    The input file path.
 * \param options A dictionary mapping of NSString keys to NSString values; these
 *                options are passed to the importer. May be nil.
 * 
 * \return Returns the frameless document on success, nil on failure.
 * 
 * \see XAP_CocoaPlugin_FramelessDocument
 */
- (id <NSObject, XAP_CocoaPlugin_FramelessDocument>)importDocumentFromFile:(NSString *)path importOptions:(NSDictionary *)options;
@end

#endif /* XAP_COCOAPLUGIN_H */
