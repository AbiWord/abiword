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

#include <string.h>

#include "ut_string.h"
#include "ut_assert.h"

#include "xap_FakeClipboard.h"

struct _ClipboardItem
{
	_ClipboardItem(char* _szFormat, void* _pData, UT_uint32 _iLen);
	~_ClipboardItem();

	char*		szFormat;
	void*		pData;
	UT_uint32	iLen;
};

_ClipboardItem::_ClipboardItem(char* _szFormat, void* _pData, UT_uint32 _iLen)
{
	szFormat = _szFormat;
	pData = new char[_iLen];
	memcpy(pData, _pData, _iLen);
	iLen = _iLen;
}

_ClipboardItem::~_ClipboardItem()
{
	delete pData;
}

AP_FakeClipboard::AP_FakeClipboard() : AP_Clipboard()
{

}

UT_Bool		AP_FakeClipboard::open(void)
{
	if (m_bOpen)
	{
		return UT_FALSE;
	}

	return UT_TRUE;
}

UT_Bool		AP_FakeClipboard::close(void)
{
	m_bOpen = UT_FALSE;

	return UT_TRUE;
}

UT_Bool		AP_FakeClipboard::addData(char* format, void* pData, UT_sint32 iNumBytes)
{
	_ClipboardItem* pItem = new _ClipboardItem(format, pData, iNumBytes);

	UT_sint32 err = m_vecData.addItem(pItem);
	if (err < 0)
	{
		return UT_FALSE;
	}
	else
	{
		return UT_TRUE;
	}
}

_ClipboardItem* AP_FakeClipboard::_findFormatItem(char* format)
{
	UT_uint32 iCount = m_vecData.getItemCount();

	for (UT_uint32 i=0; i<iCount; i++)
	{
		_ClipboardItem* pItem = (_ClipboardItem*) m_vecData.getNthItem(i);

		if (0 == UT_stricmp(format, pItem->szFormat))
		{
			return pItem;
		}
	}

	return NULL;
}

UT_Bool		AP_FakeClipboard::hasFormat(char* format)
{
	_ClipboardItem* pItem = _findFormatItem(format);
	if (pItem)
	{
		return UT_TRUE;
	}
	else
	{
		return UT_FALSE;
	}
}

UT_sint32	AP_FakeClipboard::getDataLen(char* format)
{
	_ClipboardItem* pItem = _findFormatItem(format);
	if (!pItem)
	{
		return -1;
	}
	
	return pItem->iLen;
}

UT_Bool		AP_FakeClipboard::getData(char* format, void* pData)
{
	_ClipboardItem* pItem = _findFormatItem(format);
	if (!pItem)
	{
		return UT_FALSE;
	}
	
	memcpy(pData, pItem->pData, pItem->iLen);
		
	return UT_TRUE;
}

UT_sint32	AP_FakeClipboard::countFormats(void)
{
	return m_vecData.getItemCount();
}

char*		AP_FakeClipboard::getNthFormat(UT_sint32 n)
{
	_ClipboardItem* pItem = (_ClipboardItem*) m_vecData.getNthItem(n);
	UT_ASSERT(pItem);

	return pItem->szFormat;
}

UT_Bool		AP_FakeClipboard::clear(void)
{
	UT_sint32 iCount = m_vecData.getItemCount();
	for (int i=0; i<iCount; i++)
	{
		_ClipboardItem* pItem = (_ClipboardItem*) m_vecData.getNthItem(i);
		UT_ASSERT(pItem);
		
		delete pItem;
	}

	m_vecData.clear();
	
	return UT_TRUE;
}
