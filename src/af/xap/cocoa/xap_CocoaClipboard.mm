/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001, 2003 Hubert Figuiere
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

#include <string.h>

#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_assert.h"

#include "xap_CocoaClipboard.h"

const char *	XAP_CocoaClipboard::XAP_CLIPBOARD_TEXTPLAIN_8BIT =		"TEXT";
const char *	XAP_CocoaClipboard::XAP_CLIPBOARD_STRING			=		"STRING";
const char *	XAP_CocoaClipboard::XAP_CLIPBOARD_COMPOUND_TEXT	=		"COMPOUND_TEXT";
const char *	XAP_CocoaClipboard::XAP_CLIPBOARD_RTF 			=		"text/rtf";

XAP_CocoaClipboard::XAP_CocoaClipboard()
{
}


XAP_CocoaClipboard::~XAP_CocoaClipboard()
{
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////
/*!
	Should be called beore adding text data to clipboard
 */
void XAP_CocoaClipboard::prepareForText(void)
{
	NSPasteboard* pb = _getPasteboard();
	[pb declareTypes:[NSArray arrayWithObjects:NSRTFPboardType, NSStringPboardType, nil] owner:nil];
}

bool XAP_CocoaClipboard::addData(const char* format, void* pData, UT_sint32 iNumBytes)
{
	NSPasteboard* pb = _getPasteboard();
	NSData* data = [[NSData alloc] initWithBytes:pData length:iNumBytes];
	if (strcmp(format, XAP_CLIPBOARD_RTF) == 0) {
		[pb setData:data forType:NSRTFPboardType];
	}
	else if (strcmp(format, XAP_CLIPBOARD_TEXTPLAIN_8BIT) == 0) {
		[pb setData:data forType:NSStringPboardType];
	}
	[data release];
	return true;
}

bool XAP_CocoaClipboard::getClipboardData(const char** formatAccepted, void ** ppData, UT_uint32 * pLen, const char ** szFormatFound)
{
	char** current = formatAccepted;
	NSString* pbType = nil;
	NSPasteboard* pb = _getPasteboard();
	while (current) {
		pbType = nil;
		if (strcmp(*current, XAP_CLIPBOARD_RTF) == 0) {
			pbType = NSRTFPboardType;
		}
		else if (strcmp(*current, XAP_CLIPBOARD_TEXTPLAIN_8BIT) == 0) {
			pbType = NSStringPboardType;
		}
		if (pbType) {
			NSData* data = [pb dataForType:pbType];
			if (data) {
				*pLen = [data length];
				*ppData = malloc(*pLen);
				memcpy(*ppData, [data bytes], *pLen);
				*szFormatFound = *current;
				return true;
			}
		}
		current++;
	}
	return false;
}


bool XAP_CocoaClipboard::clearClipboard(void)
{
	// User requested us to clear the clipboard.

	UT_DEBUGMSG(("Clipboard: explicit clear\n"));
//	UT_ASSERT (UT_NOT_IMPLEMENTED);
	return false;
}


bool XAP_CocoaClipboard::hasFormat(const char* format)
{
//	UT_ASSERT (m_pasteboard != nil);
	
//	NSArray * availableFormat = [m_pasteboard types];
	if (strcmp (format, "rtf") == 0) 
	{
	
	}
	else if (strcmp (format, "text-8bit") == 0) 
	{
		
	}
}

