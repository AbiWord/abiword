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

#include <windows.h>
#include <richedit.h>

#include "ut_string.h"
#include "ut_assert.h"
#include "xap_Win32Clipboard.h"
#include "gr_Image.h"
#include "gr_Win32Image.h"

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

XAP_Win32Clipboard::XAP_Win32Clipboard(void)
	: XAP_Clipboard()
{
}

UT_Bool XAP_Win32Clipboard::open(void)
{
	if (m_bOpen)
		return UT_FALSE;

	if (!OpenClipboard(NULL))
		return UT_FALSE;
	
	m_bOpen = UT_TRUE;
	return UT_TRUE;
}

UT_Bool XAP_Win32Clipboard::close(void)
{
	m_bOpen = UT_FALSE;
	return CloseClipboard();
}

UT_Bool XAP_Win32Clipboard::addData(const char * format, void* pData, UT_sint32 iNumBytes)
{
	// TODO do we need to verify that we own the clipboard?

	UINT iFormat = _convertFormatString(format);
	if (iFormat == 0)
		return UT_FALSE;

	HANDLE hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, iNumBytes);
	if (!hData)
		return UT_FALSE;
	
	void* p = GlobalLock(hData);
	memcpy(p, pData, iNumBytes);
	GlobalUnlock(hData);

	return (SetClipboardData(iFormat, hData) != NULL);
}

UT_Bool XAP_Win32Clipboard::hasFormat(const char * format)
{
	UINT iFormat = _convertFormatString(format);
	if (iFormat == 0)
		return UT_FALSE;

	HANDLE hData = GetClipboardData(iFormat);
	return (hData != NULL);
}

UT_sint32 XAP_Win32Clipboard::getDataLen(const char * format)
{
	UINT iFormat = _convertFormatString(format);
	if (iFormat == 0)
		return -1;

	HANDLE hData = GetClipboardData(iFormat);
	if (!hData)
		return -1;

	return GlobalSize(hData);
}

UT_Bool XAP_Win32Clipboard::getData(const char * format, void* pData)
{
	UINT iFormat = _convertFormatString(format);
	if (iFormat == 0)
		return UT_FALSE;

	HANDLE hData = GetClipboardData(iFormat);
	if (!hData)
		return UT_FALSE;

	// TODO error check the following stuff
		
	UT_uint32 iLen = GlobalSize(hData);
	void* p = GlobalLock(hData);
	memcpy(pData, p, iLen);
	GlobalUnlock(hData);

	return UT_TRUE;
}

UT_sint32 XAP_Win32Clipboard::countFormats(void)
{
	return CountClipboardFormats();
}

const char * XAP_Win32Clipboard::getNthFormat(UT_sint32 n)
{
	UT_ASSERT(n < countFormats());
	
	UT_uint32 iCurFormat = 0;
	int	iCount = 0;

	while (0 != (iCurFormat = EnumClipboardFormats(iCurFormat)))
	{
		if (iCount == n)
			return _convertToFormatString(iCurFormat);

		iCount++;
	}
	
	return NULL;
}

UT_Bool XAP_Win32Clipboard::clear(void)
{
	return EmptyClipboard();
}

GR_Image * XAP_Win32Clipboard::getImage(void)
{
	HANDLE hData = GetClipboardData(CF_DIB);
	if (!hData)
		return NULL;
	
	// TODO error check the following stuff
		
	UT_uint32 iLen = GlobalSize(hData);
	void* p = GlobalLock(hData);

	BITMAPINFO* pDIB;
	pDIB = (BITMAPINFO*) new unsigned char[iLen];
	UT_ASSERT(pDIB); // TODO outofmem
	memcpy(pDIB, p, iLen);
	GlobalUnlock(hData);
	return new GR_Win32Image(pDIB, "clipboard");
}

UT_Bool XAP_Win32Clipboard::addImage(GR_Image*)
{
	UT_ASSERT(UT_TODO);
	return UT_FALSE;
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

UT_uint32 XAP_Win32Clipboard::_convertFormatString(const char * format) const
{
	int kLimit = m_vecFormat.getItemCount();
	int k;

	for (k=0; k<kLimit; k++)
		if (UT_stricmp(format,(const char *)m_vecFormat.getNthItem(k)) == 0)
			return (UT_uint32)m_vecCF.getNthItem(k);

	return 0;
}

const char * XAP_Win32Clipboard::_convertToFormatString(UT_uint32 fmt) const
{
	int kLimit = m_vecFormat.getItemCount();
	int k;

	for (k=0; k<kLimit; k++)
		if (fmt == (UT_uint32)m_vecCF.getNthItem(k))
			return (const char *)m_vecFormat.getNthItem(k);

	return NULL;
}
