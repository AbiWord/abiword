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

AP_Win32Clipboard::AP_Win32Clipboard() : AP_Clipboard()
{
	m_cfRTF = RegisterClipboardFormat(CF_RTF);
}

UT_Bool		AP_Win32Clipboard::open(void)
{
	if (m_bOpen)
	{
		return UT_FALSE;
	}

	if (OpenClipboard(NULL))
	{
		m_bOpen = UT_TRUE;
		
		return UT_TRUE;
	}
	
	return UT_FALSE;
}

UT_Bool		AP_Win32Clipboard::close(void)
{
	m_bOpen = UT_FALSE;
	
	return CloseClipboard();
}

UT_uint32	AP_Win32Clipboard::_convertFormatString(char* format)
{
	if (0 == UT_stricmp(format, AP_CLIPBOARD_TEXTPLAIN_8BIT))
	{
		return CF_TEXT;
	}
	else if (0 == UT_stricmp(format, AP_CLIPBOARD_TEXTPLAIN_UNICODE))
	{
		// this is expected to work on Windows NT only!!!
		return CF_UNICODETEXT;
	}
	else if (0 == UT_stricmp(format, AP_CLIPBOARD_RTF))
	{
		return m_cfRTF;
	}
	else if (0 == UT_stricmp(format, AP_CLIPBOARD_IMAGE))
	{
		return CF_DIB;
	}
	else
	{
		return 0;	// unknown format
	}
}

UT_Bool		AP_Win32Clipboard::addData(char* format, void* pData, UT_sint32 iNumBytes)
{
	UT_ASSERT(!(0 == UT_stricmp(format, AP_CLIPBOARD_IMAGE)));
	
	// TODO do we need to verify that we own the clipboard?
	
	UINT iFormat = _convertFormatString(format);
	if (iFormat == 0)
	{
		return UT_FALSE;
	}
	
	HANDLE hData = NULL;

	hData = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, iNumBytes);
	// TODO handle outofmem
	void* p = GlobalLock(hData);
	memcpy(p, pData, iNumBytes);
	GlobalUnlock(hData);

	if (SetClipboardData(iFormat, hData))
	{
		return UT_TRUE;
	}
	else
	{
		return UT_FALSE;
	}
}

UT_Bool		AP_Win32Clipboard::hasFormat(char* format)
{
	UINT iFormat = _convertFormatString(format);
	if (iFormat == 0)
	{
		return UT_FALSE;
	}

	HANDLE hData = GetClipboardData(iFormat);
	if (hData)
	{
		return UT_TRUE;
	}
	else
	{
		return UT_FALSE;
	}
}

UT_sint32	AP_Win32Clipboard::getDataLen(char* format)
{
	UT_ASSERT(!(0 == UT_stricmp(format, AP_CLIPBOARD_IMAGE)));
	
	UINT iFormat = _convertFormatString(format);
	if (iFormat == 0)
	{
		return -1;
	}

	HANDLE hData = GetClipboardData(iFormat);
	if (hData)
	{
		return GlobalSize(hData);
	}
	else
	{
		return -1;
	}
}

UT_Bool		AP_Win32Clipboard::getData(char* format, void* pData)
{
	UT_ASSERT(!(0 == UT_stricmp(format, AP_CLIPBOARD_IMAGE)));
	
	UINT iFormat = _convertFormatString(format);
	if (iFormat == 0)
	{
		return UT_FALSE;
	}

	HANDLE hData = GetClipboardData(iFormat);
	if (hData)
	{
		// TODO error check the following stuff
		
		UT_uint32 iLen = GlobalSize(hData);
		void* p = GlobalLock(hData);

		memcpy(pData, p, iLen);
		GlobalUnlock(hData);

		return UT_TRUE;
	}
	else
	{
		return UT_FALSE;
	}
}

UT_sint32	AP_Win32Clipboard::countFormats(void)
{
	return CountClipboardFormats();
}

char*		AP_Win32Clipboard::getNthFormat(UT_sint32 n)
{
	UT_ASSERT(n < countFormats());
	
	UT_uint32 iCurFormat = 0;
	int	iCount = 0;

	while (0 != (iCurFormat = EnumClipboardFormats(iCurFormat)))
	{
		if (iCount == n)
		{
			
			switch (iCurFormat)
			{
			case CF_TEXT:
				return AP_CLIPBOARD_TEXTPLAIN_8BIT;

			case CF_UNICODETEXT:
				return AP_CLIPBOARD_TEXTPLAIN_UNICODE;

			// should we add CF_BITMAP here too?  (probably not)
			case CF_DIB:
				return AP_CLIPBOARD_IMAGE;
				
			default:
				if (iCurFormat == m_cfRTF)
				{
					return AP_CLIPBOARD_RTF;
				}
				
				return AP_CLIPBOARD_UNKNOWN;
			}
		}

		iCount++;
	}
	
	return AP_CLIPBOARD_UNKNOWN;
}

UT_Bool		AP_Win32Clipboard::clear(void)
{
	return EmptyClipboard();
}

GR_Image*	AP_Win32Clipboard::getImage(void)
{
	HANDLE hData = GetClipboardData(CF_DIB);
	if (hData)
	{
		// TODO error check the following stuff
		
		UT_uint32 iLen = GlobalSize(hData);
		void* p = GlobalLock(hData);

		BITMAPINFO* pDIB;
		pDIB = (BITMAPINFO*) new unsigned char[iLen];
		UT_ASSERT(pDIB); // TODO outofmem
		
		memcpy(pDIB, p, iLen);
		
		GlobalUnlock(hData);

		return new GR_Win32Image(pDIB);
	}
	else
	{
		return NULL;
	}
}

UT_Bool		AP_Win32Clipboard::addImage(GR_Image*)
{
	UT_ASSERT(UT_TODO);

	return UT_FALSE;
}

