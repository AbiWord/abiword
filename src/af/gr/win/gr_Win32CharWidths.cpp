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
#include "ut_Win32OS.h"
#include "gr_Win32CharWidths.h"
#include "ut_debugmsg.h"

//////////////////////////////////////////////////////////////////
//#define USE_HIGH_RESOLUTION
void GR_Win32CharWidths::setCharWidthsOfRange(HDC hdc, UT_UCSChar c0, UT_UCSChar c1)
{
#if 0
	UT_uint32 loc0 = (c0 & 0xff);
	UT_uint32 loc1 = (c1 & 0xff);
	UT_uint32 hic0 = ((c0 >> 8) & 0xff);
	UT_uint32 hic1 = ((c1 >> 8) & 0xff);

	if (hic0 == hic1)					// we are contained within one page
	{
		Array256 * pA = NULL;
		if (hic0 == 0)
			pA = &m_aLatin1;
		else if (m_vecHiByte.getItemCount() > hic0)
			pA = (Array256 *)m_vecHiByte.getNthItem(hic0);
		if (pA)
		{
			if (UT_IsWinNT())
				GetCharWidth32(hdc, loc0, loc1, &(pA->aCW[loc0]));
			else
				// GetCharWidth(hdc, loc0, loc1, &(pA->aCW[loc0]));
				GetTextExtentPoint32W( ...se below... );
			return;
		}
	}

	// if we fall out of the above, we're either spanning
	// different pages or we are on a page that hasn't
	// be loaded yet.  do it the hard way....
#endif
	UINT k;
	int w;
	//float f=0;

	// Windows NT and Windows 95 support the Unicode Font file. 
	// All of the Unicode glyphs can be rendered if the glyph is found in
	// the font file. However, Windows 95 does  not support the Unicode 
	// characters other than the characters for which the particular codepage
	// of the font file is defined.
	// Reference Microsoft knowledge base:
	// Q145754 - PRB ExtTextOutW or TextOutW Unicode Text Output Is Blank
	if (UT_IsWinNT())
	{
		for (k=c0; k<=c1; k++)
		{
			if(k == 0x200B || k == 0xFEFF)
				setWidth(k,0);
			else
			{
				GetCharWidth32W(hdc,k,k,&w);
#if 0
				bool res = GetCharWidthFloatW(hdc,k,k,&f);
				UT_DEBUGMSG(("char width1 (%d): 0x%x, int %d, float %f\n", res, k, w, f));
#endif
				setWidth(k,w);
			}
		}
	}
	else
	{
		HFONT hFont = (HFONT) GetCurrentObject(hdc, OBJ_FONT);
		LOGFONT aLogFont;
		int iRes = GetObject(hFont, sizeof(LOGFONT), &aLogFont);
		UT_ASSERT(iRes);

#ifdef USE_HIGH_RESOLUTION
		aLogFont.lfHeight *= 100;
		HFONT hBigFont = CreateFontIndirect(&aLogFont);
		UT_sint32 iOrigWidth;
		
#endif
		UT_DEBUGMSG(("gr_Win32Graphics::getCharWidth: extra interchar. spacing %d\n", GetTextCharacterExtra(hdc)));
		
		if(aLogFont.lfCharSet == SYMBOL_CHARSET)
		{
			// Symbol character handling
			for (k=c0; k<=c1; k++)
			{
				SIZE Size;
				char str[sizeof(UT_UCSChar)];
				int iConverted = WideCharToMultiByte(CP_ACP, NULL, 
					(unsigned short*) &k, 1, str, sizeof(str), NULL, NULL);
				GetTextExtentPoint32A(hdc, str, iConverted, &Size);
#if 0
				bool res = GetCharWidthFloat(hdc,k,k,&f);
				UT_DEBUGMSG(("char width2 (%d): 0x%x, int %d, float %f\n", res, k, Size.cx, f));
#endif
#ifdef USE_HIGH_RESOLUTION
				UT_sint32 myWidth = (Size.cx / 100) + ((Size.cx % 100 > 50)? 1:0);
				setWidth(k,myWidth);
				UT_DEBUGMSG(("gr_Win32Graphics::getCharWidths: myWidth %d, Size.cx %d\n", myWidth, Size.cx));
#else
				setWidth(k,Size.cx);
#endif					
			}
		}
		else
		{
			// Unicode font and default character sets
			for (k=c0; k<=c1; k++)
			{
				if(k == 0x200B || k == 0xFEFF)
					setWidth(k,0);
				else
				{
					SIZE Size;
					wchar_t sz1[2];
					sz1[0] = k;
#ifdef USE_HIGH_RESOLUTION					
					SelectObject(hdc, (HGDIOBJ) hFont);
#endif
					
					GetTextExtentPoint32W(hdc, sz1, 1, &Size);
#ifdef USE_HIGH_RESOLUTION
					iOrigWidth = Size.cx;
					
					SelectObject(hdc, (HGDIOBJ) hBigFont);
					GetTextExtentPoint32W(hdc, sz1, 1, &Size);
#endif					
#if 0
					bool res = GetCharWidthFloat(hdc,k,k,&f);
					UT_DEBUGMSG(("char width3 (%d): 0x%x, int %d, float %f\n", res, k, Size.cx, f));

					if(!res)
					{
						LPVOID lpMsgBuf;
						FormatMessage( 
									  FORMAT_MESSAGE_ALLOCATE_BUFFER | 
									  FORMAT_MESSAGE_FROM_SYSTEM | 
									  FORMAT_MESSAGE_IGNORE_INSERTS,
									  NULL,
									  GetLastError(),
									  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
									  (LPTSTR) &lpMsgBuf,
									  0,
									  NULL 
									  );
						// Process any inserts in lpMsgBuf.
						// ...
						// Display the string.
						//MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
						UT_DEBUGMSG(("GetCharWidthFloatW error: %s\n", lpMsgBuf));
						// Free the buffer.
						LocalFree( lpMsgBuf );
					}
#endif
#ifdef USE_HIGH_RESOLUTION
					UT_sint32 myWidth = (Size.cx / 100) + ((Size.cx % 100 > 50)? 1:0);
					setWidth(k,myWidth);
					UT_DEBUGMSG(("gr_Win32Graphics::getCharWidths: 0x%x: orig. width %d, myWidth %d, Size.cx %d\n", k, iOrigWidth, myWidth, Size.cx));
#else
					setWidth(k,Size.cx);
					UT_DEBUGMSG(("gr_Win32Graphics::getCharWidths: 0x%x: Size.cx %d\n", k, Size.cx));
#endif					
				}
			}
		}
#ifdef USE_HIGH_RESOLUTION
		SelectObject(hdc, (HGDIOBJ)hFont);
		DeleteObject((HGDIOBJ)hBigFont);
#endif	
	}
}

