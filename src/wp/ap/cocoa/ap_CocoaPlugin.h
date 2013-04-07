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

#ifndef AP_COCOAPLUGIN_H
#define AP_COCOAPLUGIN_H

#import <Cocoa/Cocoa.h>

#include "xap_CocoaPlugin.h"
#include "xap_Types.h"

class XAP_Frame;

class EV_EditMethod;
class EV_Menu_Action;

class FV_View;

class PD_Document;

@interface AP_CocoaPlugin_EditMethod : NSObject
{
	EV_EditMethod *		m_EditMethod;
	char				m_EditMethod_Name[64];

	BOOL				m_InContextMenu;

	SEL					m_Action;
	id <NSObject>		m_Target;
}
- (id)init;
- (void)dealloc;

- (const char *)editMethodName;

- (void)trigger;

- (void)setAction:(SEL)aSelector;
- (SEL)action;

- (void)setTarget:(id <NSObject>)target;
- (id <NSObject>)target;

@end

@interface AP_CocoaPlugin_MenuIDRef : NSObject
{
	id <XAP_CocoaPlugin_MenuItem>	m_NonRetainedRef;
}
+ (AP_CocoaPlugin_MenuIDRef *)menuIDRefWithMenuItem:(id <XAP_CocoaPlugin_MenuItem>)menuItem;

- (id)initWithMenuItem:(id <XAP_CocoaPlugin_MenuItem>)menuItem;

- (id <XAP_CocoaPlugin_MenuItem>)menuItem;
@end

@interface AP_CocoaPlugin_ContextMenuItem : AP_CocoaPlugin_EditMethod <XAP_CocoaPlugin_MenuItem>
{
	NSString *						m_Label;

	EV_Menu_Action *				m_pAction;

	XAP_Menu_Id						m_MenuID;

	int								m_Tag;
	int								m_State;

	BOOL							m_Enabled;
}
+ (AP_CocoaPlugin_ContextMenuItem *)itemWithLabel:(NSString *)label;

- (id)initWithLabel:(NSString *)label;
- (void)dealloc;

- (void)setLabel:(NSString *)label;
- (NSString *)label;


- (void)setTag:(int)anInt;
- (int)tag;

- (void)setState:(int)state;
- (int)state;

- (void)setEnabled:(BOOL)enabled;
- (BOOL)isEnabled;

@end

@interface AP_CocoaPlugin_FramelessDocument : NSObject <XAP_CocoaPlugin_FramelessDocument>
{
	PD_Document *	m_pDocument;
}
+ (NSString *)optionsPropertyString:(NSDictionary *)options;

+ (AP_CocoaPlugin_FramelessDocument *)documentFromFile:(NSString *)path importOptions:(NSDictionary *)options;

- (id)initWithPDDocument:(PD_Document *)document;
- (void)dealloc;

/* XAP_CocoaPlugin_FramelessDocument
 */
- (BOOL)exportDocumentToFile:(NSString *)path exportOptions:(NSDictionary *)options;
@end

@interface AP_CocoaPlugin_Document : NSObject <XAP_CocoaPlugin_Document>
{
	XAP_Frame *		m_pFrame;
}
/* Please bear in mind that documents can be closed, so don't keep references to documents
 * lying around for future use.
 */
+ (id <NSObject, XAP_CocoaPlugin_Document>)currentDocument; // may return nil;
+ (NSArray *)documents;                                     // array of id <XAP_CocoaPlugin_Document>

+ (NSString *)selectMailMergeSource; // may return nil

/* Returns an NSMutableArray whose objects are NSMutableArray of NSString, the first row holding the
 * field names, the rest being records; returns nil on failure.
 */
+ (NSMutableArray *)importMailMergeSource:(NSString *)path;

+ (BOOL)frameExists:(XAP_Frame *)frame;

- (id)initWithXAPFrame:(XAP_Frame *)frame;

- (BOOL)documentStillExists;

- (NSString *)title;

- (NSString *)selectWord;
- (NSString *)selectedText;

- (void)insertText:(NSString *)text;

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

#endif /* ! AP_COCOAPLUGIN_H */
