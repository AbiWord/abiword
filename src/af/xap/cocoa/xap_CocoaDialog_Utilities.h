/* AbiWord
 * Copyright (C) 2002-2005 Hubert Figuiere
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


#ifndef __XAP_COCOADIALOG_UTILS_H__
#define __XAP_COCOADIALOG_UTILS_H__

#include <string>

#import <Cocoa/Cocoa.h>

#include "ut_string_class.h"

#include "xap_Strings.h"

class XAP_Dialog;

void SetNSControlLabel (id control, const UT_UTF8String &label);
void SetNSControlLabel (id control, const std::string &label);
void LocalizeControl (id control, const XAP_StringSet * pSS, XAP_String_Id stringId);
NSString* LocalizedString (const XAP_StringSet * pSS, XAP_String_Id stringId);
void AppendLocalizedMenuItem (NSPopUpButton* menu, const XAP_StringSet * pSS, XAP_String_Id stringId, int tag);
void _convertLabelToMac (char * buf, size_t bufSize, const char * label);

@protocol XAP_CocoaDialogProtocol

/* load the nib for the CocoaDialog */
- (id)initFromNib;

/* assign the controller an XAP object */
- (void)setXAPOwner:(XAP_Dialog *)owner;
/* discard that XAP because Life of Controller might be longer than the XAP Object */
- (void)discardXAP;

@end
#endif
