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

#ifndef AP_COCOAPLUGIN_H
#define AP_COCOAPLUGIN_H

#import <Cocoa/Cocoa.h>

#include "xap_CocoaPlugin.h"

class XAP_Frame;

@interface AP_CocoaPlugin_Document : NSObject <XAP_CocoaPlugin_Document>
{
	XAP_Frame *	m_pFrame;
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
