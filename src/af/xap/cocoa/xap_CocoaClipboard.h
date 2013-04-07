/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2005 Net Integration Technologies Inc. (written by Hubert Figuiere)
 * Copyright (C) 2001, 2003, 2009 Hubert Figuiere
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

#ifndef XAP_COCOACLIPBOARD_H
#define XAP_COCOACLIPBOARD_H

#import <AppKit/AppKit.h>

#include "ut_types.h"
#include "xap_CocoaApp.h"
#include "xap_FakeClipboard.h"

//////////////////////////////////////////////////////////////////

class XAP_CocoaClipboard
	: public XAP_FakeClipboard
{
public:

	XAP_CocoaClipboard();
	virtual ~XAP_CocoaClipboard();

	virtual bool			clearClipboard(void);

	virtual bool			addData(const char* format, void* pData, UT_sint32 iNumBytes);
	virtual bool			getClipboardData(const char** formatAccepted, void ** ppData, UT_uint32 * pLen, const char ** szFormatFound);
	virtual bool			hasFormat(const char* format);
	/*! return if clipboard has one of the formats listed
	 */
	bool					hasFormats(const char** format);
	void					prepareForText();
	NSPasteboard		*_getPasteboard () { return [NSPasteboard generalPasteboard]; };
static const char *	XAP_CLIPBOARD_TEXTPLAIN_8BIT;
static const char *	XAP_CLIPBOARD_STRING;
static const char *	XAP_CLIPBOARD_COMPOUND_TEXT;
static const char *	XAP_CLIPBOARD_RTF;
static const char * XAP_CLIPBOARD_IMAGE;

private:
	static NSString *_abi2ns_cbType(const char *);
};

#endif /* XAP_COCOACLIPBOARD_H */
