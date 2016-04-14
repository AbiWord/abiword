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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include <string.h>

#include "ut_string.h"
#include "ut_assert.h"

#include "xap_FakeClipboard.h"

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

struct _ClipboardItem
{
	_ClipboardItem(const char * szFormat, const void* pData, UT_uint32 iLen);
	~_ClipboardItem();
	void replace(const void * pData, UT_uint32 iLen);

	const char *	m_szFormat;
	unsigned char *	m_pData;
	UT_uint32		m_iLen;
};

_ClipboardItem::_ClipboardItem(const char * szFormat, const void* pData, UT_uint32 iLen)
{
	m_szFormat = szFormat;
	m_pData = new unsigned char[iLen];
	memcpy(m_pData, pData, iLen);
	m_iLen = iLen;
}

_ClipboardItem::~_ClipboardItem()
{
	delete [] m_pData;
}

void _ClipboardItem::replace(const void * pData, UT_uint32 iLen)
{
	DELETEPV(m_pData);
	m_pData = new unsigned char[iLen];
	memcpy(m_pData, pData, iLen);
	m_iLen = iLen;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

XAP_FakeClipboard::XAP_FakeClipboard()
{
}

XAP_FakeClipboard::~XAP_FakeClipboard()
{
	clearClipboard();
}

bool XAP_FakeClipboard::clearClipboard(void)
{
	UT_sint32 iCount = m_vecData.getItemCount();
	for (int i=0; i<iCount; i++)
	{
		_ClipboardItem* pItem = m_vecData.getNthItem(i);
		DELETEP(pItem);
	}

	m_vecData.clear();
	
	return true;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

bool XAP_FakeClipboard::addData(const char* format, const void* pData, UT_sint32 iNumBytes)
{
	_ClipboardItem * pExistingItem = _findFormatItem(format);
	if (pExistingItem)
	{
		pExistingItem->replace(pData,iNumBytes);
		return true;
	}
	
	_ClipboardItem * pItem = new _ClipboardItem(format, pData, iNumBytes);

	UT_sint32 err = m_vecData.addItem(pItem);
	return (err >= 0);
}

_ClipboardItem* XAP_FakeClipboard::_findFormatItem(const char* format)
{
	UT_uint32 iCount = m_vecData.getItemCount();

	for (UT_uint32 i=0; i<iCount; i++)
	{
		_ClipboardItem* pItem = m_vecData.getNthItem(i);
		if (g_ascii_strcasecmp(format, pItem->m_szFormat) == 0)
			return pItem;
	}

	return NULL;
}

bool XAP_FakeClipboard::hasFormat(const char* format)
{
	_ClipboardItem* pItem = _findFormatItem(format);
	return (pItem != NULL);
}

bool XAP_FakeClipboard::getClipboardData(const char * format, void ** ppData, UT_uint32 * pLen)
{
	UT_ASSERT(ppData && pLen);

	_ClipboardItem* pItem = _findFormatItem(format);
	if (pItem)
	{
		*ppData = pItem->m_pData;
		*pLen = pItem->m_iLen;
		return true;
	}
	else
	{
		*ppData = NULL;
		*pLen = 0;
		return false;
	}
}
