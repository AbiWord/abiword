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

AP_MacClipboard::AP_MacClipboard() : AP_Clipboard()
{
}

UT_Bool		AP_MacClipboard::open(void)
{
	if (m_bOpen)
	{
		return UT_FALSE;
	}

	return UT_FALSE;
}

UT_Bool		AP_MacClipboard::close(void)
{
	m_bOpen = UT_FALSE;
	return UT_TRUE;
}

UT_uint32	AP_MacClipboard::_convertFormatString(char* format)
{
	return 0;
}

UT_Bool		AP_MacClipboard::addData(char* format, void* pData, UT_sint32 iNumBytes)
{
	return UT_TRUE;
}

UT_Bool		AP_MacClipboard::hasFormat(char* format)
{
	return UT_FALSE;
}

UT_sint32	AP_MacClipboard::getDataLen(char* format)
{
	return -1;
}

UT_Bool		AP_MacClipboard::getData(char* format, void* pData)
{
	return UT_FALSE;
}

UT_sint32	AP_MacClipboard::countFormats(void)
{
	return 0;
}

char*		AP_MacClipboard::getNthFormat(UT_sint32 n)
{
	return AP_CLIPBOARD_UNKNOWN;
}

UT_Bool		AP_MacClipboard::clear(void)
{
	return UT_TRUE;
}


