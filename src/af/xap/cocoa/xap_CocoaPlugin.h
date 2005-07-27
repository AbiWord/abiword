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

#define XAP_COCOAPLUGIN_INTERFACE 20050304 /** The current version of the CocoaPlugin API. */

#import <Cocoa/Cocoa.h>

@class XAP_CocoaPlugin;

/**
 * \protocol XAP_CocoaPlugin_MenuItem XAP_CocoaPlugin.h "XAP_CocoaPlugin.h"
 * 
 * A reference to a context menu item.
 */
@protocol XAP_CocoaPlugin_MenuItem

/**
 * \param label Set the label of the context menu item.
 */
- (void)setLabel:(NSString *)label;

/**
 * \return The label of the context menu item.
 */
- (NSString *)label;

/**
 * \param aSelector Set the method which is called when the call-back is triggered, or
 *                  nil if no method is to be called.
 */
- (void)setAction:(SEL)aSelector;

/**
 * \return The method which is called when the call-back is triggered, or nil if none.
 */
- (SEL)action;

/**
 * \param target Set a new target object for the action when the call-back is triggered, or
 *               nil if no method is to be called.
 */
- (void)setTarget:(id <NSObject>)target;

/**
 * \return The new target object for the action when the call-back is triggered, or nil if none.
 */
- (id <NSObject>)target;

/**
 * \param anInt An integer which can be used to identify this particular object.
 */
- (void)setTag:(int)anInt;

/**
 * \return The integer used to identify this particular object.
 */
- (int)tag;

/**
 * \param state To have a check (tick) beside the menu item, set the state to NSOnState, otherwise
 *              set it to NSOffState. The default is NSOffState.
 */
- (void)setState:(int)state;

/**
 * \return Returns YES if the menu item is checked (ticked).
 */
- (int)state;

/**
 * \param enabled YES if the menu item should be enabled, or NO if the menu item should be greyed
 *                (i.e., non-selectable). The default is YES.
 */
- (void)setEnabled:(BOOL)enabled;

/**
 * \return Returns YES if the menu item is selectable.
 */
- (BOOL)isEnabled;

@end

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

/**
 * Selects and returns the current word, if any.
 * 
 * \return The current word, or nil if none could be selected.
 */
- (NSString *)selectWord;

/**
 * Returns the current selection, if any.
 * 
 * \return The currently selected text, or nil if none is selected.
 */
- (NSString *)selectedText;

/**
 * Insert a text string into the document.
 * 
 * \param text The string to insert.
 */
- (void)insertText:(NSString *)text;

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
 * NSObject protocol and the XAP_CocoaPluginDelegate's
 * pluginCanRegisterForAbiWord:version:interface: method.
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
 * \param AbiWord   The main interface to AbiWord.
 * \param version   This is the AbiWord version string (e.g., "2.2.5").
 * \param interface This is the date (YYYYMMDD) that the CocoaPlugin API was last changed.
 * 
 * \return Should return NO if for some reason the plug-in won't work with the current
 *         version of AbiWord.
 */
- (BOOL)pluginCanRegisterForAbiWord:(XAP_CocoaPlugin *)AbiWord version:(NSString *)version interface:(unsigned long)interface;

/**
 * Returns whether the plug-in is active or not.
 * 
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
 * The name of the plug-in.
 * 
 * \return The name of the plug-in.
 */
- (NSString *)pluginName;

/**
 * The name(s) of the plug-in author(s).
 * 
 * \return The name(s) of the plug-in author(s).
 */
- (NSString *)pluginAuthor;

/**
 * The version of the plug-in.
 * 
 * \return The version of the plug-in.
 */
- (NSString *)pluginVersion;

/**
 * A short description of what the plug-in does.
 * 
 * \return A short description of what the plug-in does.
 */
- (NSString *)pluginDescription;

/**
 * A short description of how to use the plug-in.
 * 
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
	id <NSObject, XAP_CocoaPluginDelegate>	m_delegate; /** The plug-in's delegate, by default the bundle's principal class. */
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
 * pluginCanRegisterForAbiWord:version:interface: is called).
 */
- (BOOL)loadBundleWithPath:(NSString *)path;

/**
 * If the plug-in bundle's principal class does not implement the XAP_CocoaPluginDelegate
 * protocol, then, when the pluginCanRegisterForAbiWord:version:interface: method is called, it
 * should set a new delegate object which does implement the protocol.
 * 
 * \param delegate The new delegate (must not be nil). The delegate is not retained.
 * 
 * \see XAP_CocoaPluginDelegate
 */
- (void)setDelegate:(id <NSObject, XAP_CocoaPluginDelegate>)delegate;

/**
 * The plug-in's delegate.
 * 
 * \return The delegate object - the bundle's principal class, unless the plug-in has
 *         substituted another.
 */
- (id <NSObject, XAP_CocoaPluginDelegate>)delegate;

/**
 * Add a menu item to the end of the Tools menu.
 * 
 * This is an NSMenuItem object that the plug-in makes itself (or loads from a nib file) and
 * so can have submenus etc. This procedure for adding menu items to AbiWord's main menu
 * does not use AbiWord's cross-platform method for adding menu items (which may not currently
 * work anyway - I'm not sure).
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
 * Get a reference to the current document window, if any.
 * 
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
 * Get an array of references to the current document windows, if any.
 * 
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

/**
 * Add a menu item to the context menu.
 * You should retain the returned context menu item object, since the menu item is removed from
 * the context menu when the object is deleted.
 * 
 * \param label The label of the menu item in the context menu.
 * 
 * \return A reference to a context menu item object.
 * 
 * \see XAP_CocoaPlugin_MenuItem
 */
- (id <NSObject, XAP_CocoaPlugin_MenuItem>)contextMenuItemWithLabel:(NSString *)label;

@end

#endif /* XAP_COCOAPLUGIN_H */
