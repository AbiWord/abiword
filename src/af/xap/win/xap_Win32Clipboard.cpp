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

#include "ut_debugmsg.h"
#include "ut_string.h"
#include "ut_assert.h"
#include "xap_Win32Clipboard.h"

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

XAP_Win32Clipboard::XAP_Win32Clipboard(void)
{
	m_bOpen = UT_FALSE;
}

UT_Bool XAP_Win32Clipboard::openClipboard(void)
{
	if (m_bOpen)
		return UT_FALSE;

	if (!OpenClipboard(NULL))
		return UT_FALSE;

#ifdef DEBUG
	{
		UINT k = 0;
		UINT f;
		while ( (f=EnumClipboardFormats(k++)) )
			UT_DEBUGMSG(("Clipboard contains format [%d]\n",f));
	}
#endif /* DEBUG */

	m_bOpen = UT_TRUE;
	return UT_TRUE;
}

UT_Bool XAP_Win32Clipboard::closeClipboard(void)
{
	m_bOpen = UT_FALSE;
	return CloseClipboard();
}

UT_Bool XAP_Win32Clipboard::clearClipboard(void)
{
	return EmptyClipboard();
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

UT_Bool XAP_Win32Clipboard::addData(const char * format, void* pData, UT_sint32 iNumBytes)
{
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

	// TODO if SetClipboardData() fails, do we need to GlobalFree(hData) ??
}

HANDLE XAP_Win32Clipboard::getHandleInFormat(const char * format)
{
	UINT iFormat = _convertFormatString(format);
	if (iFormat == 0)
		return NULL;

	HANDLE hData = GetClipboardData(iFormat);
	return (hData);
}
	
UT_Bool XAP_Win32Clipboard::hasFormat(const char * format)
{
	HANDLE hData = getHandleInFormat(format);
	return (hData != NULL);
}

//////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////

UT_uint32 XAP_Win32Clipboard::_convertFormatString(const char * format) const
{
	// convert from named type (like "text-8bit") to a registered
	// clipboard format (either a MSFT-defined CF_ symbol or something
	// that we registered).
	
	int kLimit = m_vecFormat.getItemCount();
	int k;

	for (k=0; k<kLimit; k++)
		if (UT_stricmp(format,(const char *)m_vecFormat.getNthItem(k)) == 0)
			return (UT_uint32)m_vecCF.getNthItem(k);

	return 0;
}

const char * XAP_Win32Clipboard::_convertToFormatString(UT_uint32 fmt) const
{
	// convert from a CF_ symbol or something that we registered
	// into a named format type (like "text-8bit").
	
	int kLimit = m_vecFormat.getItemCount();
	int k;

	for (k=0; k<kLimit; k++)
		if (fmt == (UT_uint32)m_vecCF.getNthItem(k))
			return (const char *)m_vecFormat.getNthItem(k);

	return NULL;
}
