/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
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

#ifndef XAP_BEOSCLIPBOARD_H
#define XAP_BEOSCLIPBOARD_H

#include "ut_types.h"
#include "ut_vector.h"

#include "xap_Clipboard.h"
#include "xap_FakeClipboard.h"

class XAP_BeOSClipboard : public XAP_FakeClipboard
{
public:
	XAP_BeOSClipboard();
	virtual ~XAP_BeOSClipboard();
	
	virtual UT_Bool			clearClipboard(void);

	virtual UT_Bool			addData(const char* format, void* pData, UT_sint32 iNumBytes);
	virtual UT_Bool			getClipboardData(const char* format, void ** ppData, UT_uint32 * pLen);
	virtual UT_Bool			hasFormat(const char* format);

};

#endif /* XAP_BEOSCLIPBOARD_H */

