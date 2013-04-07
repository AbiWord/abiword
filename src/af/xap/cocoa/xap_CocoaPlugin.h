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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */

#ifndef XAP_COCOAPLUGIN_H
#define XAP_COCOAPLUGIN_H

#define XAP_COCOAPLUGIN_INTERFACE 20050925 /** The current version of the CocoaPlugin API. */

#import <Cocoa/Cocoa.h>

#import "xap_CocoaCompat.h"

@class XAP_CocoaPlugin;

@protocol XAP_CocoaPlugin_Tool;
@protocol XAP_CocoaPlugin_ToolProvider;

/**
 * \protocol XAP_CocoaPlugin_ToolInstance XAP_CocoaPlugin.h "XAP_CocoaPlugin.h"
 *
 * A class which has a toolbar button and, optionally, a menu item associated with
 * a specific tool. The class also manages size and image information which can be
 * saved to and loaded from a configuration file, if desired.
 */
@protocol XAP_CocoaPlugin_ToolInstance

/**
 * Information about what this tool actually is, rather than what it looks like, can
 * be obtained from the tool which created this instance.
 *
 * \return The tool associated with this object.
 */
- (id <NSObject, XAP_CocoaPlugin_Tool>)tool;

/**
 * The XAP_CocoaPlugin_ToolInstance object manages a button and, optionally, a menu
 * item.
 *
 * \return The button managed by this object.
 */
- (NSView *)toolbarButton;

/**
 * The XAP_CocoaPlugin_ToolInstance object may manage a menu item for use in case the
 * button disappears off the end of the toolbar.
 *
 * \return The menu item managed by this object, or nil if there is no menu item.
 */
- (NSMenuItem *)toolbarMenuItem;

/**
 * The XAP_CocoaPlugin_ToolInstance manages a set of configuration strings.
 *
 * \return The string describing the width of the button - should be @"auto" if it is
 *         the same as the default setting.
 */
- (NSString *)configWidth;

/**
 * The XAP_CocoaPlugin_ToolInstance manages a set of configuration strings.
 *
 * \return The string describing the height of the button - should be @"auto" if it is
 *         the same as the default setting.
 */
- (NSString *)configHeight;

/**
 * The XAP_CocoaPlugin_ToolInstance manages a set of configuration strings.
 *
 * \return The string describing the image used by the button, if any image is used at all.
 *         Should be @"auto" if it is the same as the default setting.
 */
- (NSString *)configImage;

/**
 * The XAP_CocoaPlugin_ToolInstance manages a set of configuration strings.
 *
 * \return The string describing the alternative image used by the button, if any image
 *         is used at all. Should be @"auto" if it is the same as the default setting.
 */
- (NSString *)configAltImage;

/**
 * The XAP_CocoaPlugin_ToolInstance manages a set of configuration strings.
 *
 * \param width The width setting which has been read from a configuration file.
 */
- (void)setConfigWidth:(NSString *)width;

/**
 * The XAP_CocoaPlugin_ToolInstance manages a set of configuration strings.
 *
 * \param height The height setting which has been read from a configuration file.
 */
- (void)setConfigHeight:(NSString *)height;

/**
 * The XAP_CocoaPlugin_ToolInstance manages a set of configuration strings.
 *
 * \param image The image setting (i.e., filename) which has been read from a
 *              configuration file.
 */
- (void)setConfigImage:(NSString *)image;

/**
 * The XAP_CocoaPlugin_ToolInstance manages a set of configuration strings.
 *
 * \param altImage The alternative image setting (i.e., filename) which has been
 *                 read from a configuration file.
 */
- (void)setConfigAltImage:(NSString *)altImage;
@end

/**
 * \protocol XAP_CocoaPlugin_Tool XAP_CocoaPlugin.h "XAP_CocoaPlugin.h"
 *
 * A class which provides information about a specific tool. Any given tool
 * may have any number of buttons (or menu items) in any number of toolbars;
 * the XAP_CocoaPlugin_Tool object is used to create instances of these.
 */
@protocol XAP_CocoaPlugin_Tool

/**
 * Each tool, for a given provider, has a unique identifier.
 *
 * \return The unique identifier for this tool.
 */
- (NSString *)identifier;

/**
 * The description is the tooltip or menu item name for this tool.
 *
 * \return A description of this tool.
 */
- (NSString *)description;

/**
 * When a tool is added to a tool provider, the provider sends a setProvider: message to the tool.
 *
 * \param provider The tool provider which has assumed ownership of this tool, or nil if the
 *                 provider has removed the tool from its list. (Internal use only.)
 */
- (void)setProvider:(id <NSObject, XAP_CocoaPlugin_ToolProvider>)provider;

/**
 * The tool provider which owns the tool.
 *
 * \return The tool provider which owns the tool, or nil if none does.
 */
- (id <NSObject, XAP_CocoaPlugin_ToolProvider>)provider;

/**
 * Multiple toolbar buttons can be created for a particular tool. This method instantiates a new
 * tool object which manages a toolbar button and, optionally, a menu item.
 *
 * \return A tool object which manages a toolbar button and, optionally, a menu item.
 */
- (id <NSObject, XAP_CocoaPlugin_ToolInstance>)tool;
@end

/**
 * \protocol XAP_CocoaPlugin_ToolProvider XAP_CocoaPlugin.h "XAP_CocoaPlugin.h"
 *
 * A class which provides a set of available tools which can be used to create a
 * toolbar.
 */
@protocol XAP_CocoaPlugin_ToolProvider

/**
 * Get the name of the provider.
 *
 * \return The name identifying the provider.
 */
- (NSString *)name;

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

/**
 * \protocol XAP_CocoaPlugin_SimpleXML XAP_CocoaPlugin.h "XAP_CocoaPlugin.h"
 *
 * A simple call-back interface for SAX-style XML parsing.
 */
@protocol XAP_CocoaPlugin_SimpleXML

/**
 * Called on start of a new element.
 *
 * \param name       The element name.
 * \param attributes The element's attributes as a dictionary.
 *
 * \return This method should return YES normally, but NO in order to stop parsing.
 */
- (BOOL)startElement:(NSString *)name attributes:(NSDictionary *)attributes;

/**
 * Called on end of a element.
 *
 * \param name The element name.
 *
 * \return This method should return YES normally, but NO in order to stop parsing.
 */
- (BOOL)endElement:(NSString *)name;

/**
 * Called with text data.
 *
 * \param data The character data.
 *
 * \return This method should return YES normally, but NO in order to stop parsing.
 */
- (BOOL)characterData:(NSString *)data;

@end

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

#if 0 // YAGNI
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
#endif

/**
 * \param anInt An integer which can be used to identify this particular object.
 */
- (void)setTag:(NSInteger)anInt;

/**
 * \return The integer used to identify this particular object.
 */
- (NSInteger)tag;

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

/**
 * Get a list of all the tool providers.
 * Each tool provider is of type id <NSObject, XAP_CocoaPlugin_ToolProvider>.
 *
 * \return The tool providers.
 */
- (NSArray *)toolProviders;

/**
 * Find a tool provider by name.
 * (TODO: If plug-ins are registering tool providers, we need to implement a notification
 *        system to update toolbar systems.)
 *
 * \param name The name of the tool provider to find.
 *
 * \return The tool provider, or nil if none is registered with the given name.
 */
- (id <NSObject, XAP_CocoaPlugin_ToolProvider>)toolProvider:(NSString *)name;

/**
 * Find a file in one of AbiWord's resource locations.
 * This looks in the user's AbiSuite folder first, then the system AbiSuite folder, and
 * finally the AbiWord bundle's Resources folder. Must be a regular file.
 *
 * \param relativePath A filename or relative path, e.g., "MyPlugin/config.xml".
 *
 * \return The full path to the specified resource, or nil if not found.
 */
- (NSString *)findResourcePath:(NSString *)relativePath;

/**
 * Generate a path in the user's AbiSuite resource location.
 *
 * \param relativePath A filename or relative path, e.g., "MyPlugin/config.xml".
 *
 * \return The full path to the specified resource.
 */
- (NSString *)userResourcePath:(NSString *)relativePath;

/**
 * Parse an XML file using a simple XML call-back interface.
 *
 * \param path     The path to the XML file.
 * \param callback An object which implements the XAP_CocoaPlugin_SimpleXML protocol.
 *
 * \return A string with an error message on failure, or nil on success.
 */
- (NSString *)parseFile:(NSString *)path simpleXML:(id <XAP_CocoaPlugin_SimpleXML>)callback;

@end

#endif /* XAP_COCOAPLUGIN_H */
