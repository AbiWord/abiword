/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 1999 John Brewer DBA Jera Design
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

#include "ut_string.h"
#include "ut_assert.h"

#include "xap_MacClipboard.h"

XAP_MacClipboard::XAP_MacClipboard() : XAP_FakeClipboard()
{
}

UT_Bool XAP_MacClipboard::addData(const char* format, void* pData, UT_sint32 iNumBytes)
{
	return XAP_FakeClipboard::addData(format, pData, iNumBytes);
}

UT_Bool XAP_MacClipboard::hasFormat(const char* format)
{
	return XAP_FakeClipboard::hasFormat(format);
}
