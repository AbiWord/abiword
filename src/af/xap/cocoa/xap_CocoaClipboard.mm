/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001 Hubert Figuiere
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


XAP_CocoaClipboard::XAP_CocoaClipboard()
	: m_pasteboard (nil)
{
	UT_DEBUGMSG(("Clipboard: initializing\n"));
	m_pasteboard = [NSPasteboard generalPasteboard];
	[m_pasteboard retain];
}


XAP_CocoaClipboard::~XAP_CocoaClipboard()
{
	if (m_pasteboard) {
		[m_pasteboard release];
	}
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

	
bool XAP_CocoaClipboard::addData(const char* format, void* pData, UT_sint32 iNumBytes)
{
	UT_ASSERT (UT_NOT_IMPLEMENTED);
	
	return false;
}

bool XAP_CocoaClipboard::getClipboardData(const char** formatAccepted, void ** ppData, UT_uint32 * pLen, const char ** szFormatFound)
{
	UT_ASSERT (UT_NOT_IMPLEMENTED);
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
	UT_ASSERT (m_pasteboard != nil);
	
	NSArray * availableFormat = [m_pasteboard types];
	if (strcmp (format, "rtf") == 0) 
	{
	
	}
	else if (strcmp (format, "text-8bit") == 0) 
	{
		
	}
}

