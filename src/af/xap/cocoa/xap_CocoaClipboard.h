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

#ifndef XAP_COCOACLIPBOARD_H
#define XAP_COCOACLIPBOARD_H

#import <AppKit/AppKit.h>

#include "ut_types.h"
#include "ut_vector.h"
#include "ut_bytebuf.h"
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
	virtual bool			getClipboardData(const char* format, void ** ppData, UT_uint32 * pLen);
	virtual bool			hasFormat(const char* format);
private:
	NSPasteboard		*m_pasteboard;
};

#endif /* XAP_COCOACLIPBOARD_H */
