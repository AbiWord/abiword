/* AbiWord
 * Copyright (C) 2002-2003 Hubert Figuiere
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
/* $Id */



#import <Cocoa/Cocoa.h>

#include "xap_CocoaDialog_Utilities.h"

void LocalizeControl (id control, const XAP_StringSet * pSS, XAP_String_Id stringId)
{
	char buf [1024];
	NSString*	str;
	_convertLabelToMac(buf, sizeof (buf), pSS->getValueUTF8(stringId));

	str = [[NSString alloc] initWithUTF8String:buf];
	if ([control isKindOfClass:[NSButton class]] 
	     || [control isKindOfClass:[NSBox class]]
		 || [control isKindOfClass:[NSCell class]]) {
		[control setTitle:str];
	}
	else if ([control isKindOfClass:[NSTabViewItem class]]) {
		[control setLabel:str];
	}
	else if ([control isKindOfClass:[NSTextField class]]) {
		[control setStringValue:str];
	}
	else if ([control isKindOfClass:[NSWindow class]]) {
		[control setTitle:str];
	}
	else {
		NSLog(@"Unknown control type to localize: %@", [control class]);
	}
	[str release];
}

NSString* LocalizedString (const XAP_StringSet * pSS, XAP_String_Id stringId)
{
	char buf [1024];
	_convertLabelToMac(buf, sizeof (buf), pSS->getValueUTF8(stringId));
	return [NSString stringWithUTF8String:buf];
}


/*
	Strip the '&' et '_' from the label
	
	\param buf the result buffer
	\param bufSize the allocated size for buf
	\param label the label to convert as an UT_String
 */
void _convertLabelToMac (char * buf, size_t bufSize, const UT_String& label)
{
	UT_ASSERT(buf);
	UT_ASSERT(label.length() < bufSize);

	strcpy (buf, label.c_str());

	char * src, *dst;
	src = dst = buf;
	while (*src)
	{
		*dst = *src;
		src++;
#warning I am still wondering why some string have "&" and the other have "underscore"
		if ((*dst != '&') && (*dst != '_'))
			dst++;
	}
	*dst = 0;
}
