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

#include <windows.h>
#include <richedit.h>

#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "xap_Win32Clipboard.h"

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

XAP_Win32Clipboard::XAP_Win32Clipboard(void)
{
	m_bOpen = false;
}

bool XAP_Win32Clipboard::openClipboard(HWND hWnd)
{
	if (m_bOpen)
		return false;

	if (!OpenClipboard(hWnd))
		return false;

#ifdef DEBUG
	{
		/*
		UINT k = 0;
		UINT f;
		while ( (f=EnumClipboardFormats(k++)) )
			UT_DEBUGMSG(("Clipboard contains format [%d]\n",f));
		*/
	}
#endif /* DEBUG */

	m_bOpen = true;
	return true;
}

bool XAP_Win32Clipboard::closeClipboard(void)
{
	m_bOpen = false;
	return (CloseClipboard() != 0);
}

bool XAP_Win32Clipboard::clearClipboard(void)
{
	return (EmptyClipboard() != 0);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

bool XAP_Win32Clipboard::addData(const char * format, void* pData, UT_sint32 iNumBytes)
{
	UINT iFormat = convertFormatString(format);
	if (iFormat == 0)
		return false;

	HANDLE hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, iNumBytes);
	if (!hData)
		return false;
	
	void* p = GlobalLock(hData);
	if(!p) //GlobalLock can return NULL
	{
		UT_ASSERT_HARMLESS(p);
		GlobalFree(hData);
		return false;
	}
	memcpy(p, pData, iNumBytes);
	GlobalUnlock(hData);

	return (SetClipboardData(iFormat, hData) != NULL);

	// TODO if SetClipboardData() fails, do we need to GlobalFree(hData) ??
}

// Caller should release using GlobalUnlock the memory handle returned 
HANDLE XAP_Win32Clipboard::getHandleInFormat(const char * format)
{
	UT_uintptr iFormat = convertFormatString(format);
	if (iFormat == 0)
		return NULL;

	HANDLE hData = GetClipboardData(iFormat);
	return (hData);
}
	
bool XAP_Win32Clipboard::hasFormat(const char * format)
{
	UINT iFormat = convertFormatString(format);
	if (iFormat == 0)
		return false;

	return (IsClipboardFormatAvailable(iFormat) != FALSE);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

UT_uintptr XAP_Win32Clipboard::convertFormatString(const char * format) const
{
	// convert from named type (like "text-8bit") to a registered
	// clipboard format (either a MSFT-defined CF_ symbol or something
	// that we registered).
	
	int kLimit = m_vecFormat.getItemCount();
	int k;

	for (k=0; k<kLimit; k++)
		if (g_ascii_strcasecmp(format,(const char *)m_vecFormat.getNthItem(k)) == 0)
			return (UT_uintptr)m_vecCF.getNthItem(k);

	return 0;
}

const char * XAP_Win32Clipboard::convertToFormatString(UT_uintptr fmt) const
{
	// convert from a CF_ symbol or something that we registered
	// into a named format type (like "text-8bit").
	
	int kLimit = m_vecFormat.getItemCount();
	int k;

	for (k=0; k<kLimit; k++)
		if (fmt == (UT_uintptr)m_vecCF.getNthItem(k))
			return (const char *)m_vecFormat.getNthItem(k);

	return NULL;
}
