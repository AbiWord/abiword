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
#include "gr_Graphics.h"
#include "ut_debugmsg.h"
#include "ut_OverstrikingChars.h"

//////////////////////////////////////////////////////////////////

#define _UL(x) pGr->tlu((x))
#define _UUL(x) (x) = pGr->tlu((x))

void GR_Win32CharWidths::setCharWidthsOfRange(HDC hdc, UT_UCSChar c0, UT_UCSChar c1, GR_Graphics * pGr)
{
	UINT k;
	int w;

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
				_UUL(w);
				// handle overstriking chars here
				UT_uint32 iOver = UT_isOverstrikingChar(k);
				if(!w || iOver != UT_NOT_OVERSTRIKING)
				{
					iOver &= UT_OVERSTRIKING_TYPE;

					if(iOver == UT_OVERSTRIKING_RIGHT)
					{
						w = 0;
					}
					else
					{
						ABC abc;
						int iRes = GetCharABCWidthsW(hdc,k,k,&abc);
						UT_ASSERT( iRes );
						

						if(iOver == UT_OVERSTRIKING_LEFT)
						{
							UT_ASSERT( abc.abcB <  GR_OC_MAX_WIDTH);
							w = _UL(abc.abcB) | GR_OC_LEFT_FLUSHED;
						}
						else
						{
							w = _UL(abc.abcB /*+ abc.abcA + abc.abcC*/);
							w = -w;
						}
					}
				}
				
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

		xxx_UT_DEBUGMSG(("gr_Win32Graphics::getCharWidth: extra interchar. spacing %d\n", GetTextCharacterExtra(hdc)));
		
		if(aLogFont.lfCharSet == SYMBOL_CHARSET)
		{
			// Symbol character handling
			for (k=c0; k<=c1; k++)
			{
				SIZE Size;
				char str[sizeof(UT_UCSChar)];
				int iConverted = WideCharToMultiByte(CP_ACP, NULL, 
					(LPCWSTR) &k, 1, str, sizeof(str), NULL, NULL);
				GetTextExtentPoint32A(hdc, str, iConverted, &Size);
				setWidth(k,_UL(Size.cx));
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
					
					GetTextExtentPoint32W(hdc, sz1, 1, &Size);
					_UUL(Size.cx);
					// handle overstriking chars here
					UT_uint32 iOver = UT_isOverstrikingChar(k);
					if(!Size.cx ||  iOver != UT_NOT_OVERSTRIKING)
					{
						iOver &= UT_OVERSTRIKING_TYPE;

						if(iOver == UT_OVERSTRIKING_RIGHT)
						{
							Size.cx = 0;
						}
						else
						{
							ABC abc;
							int iRes = GetCharABCWidthsW(hdc,k,k,&abc);
							UT_ASSERT( iRes );

							if(iOver == UT_OVERSTRIKING_LEFT)
							{
								UT_ASSERT( abc.abcB <  GR_OC_MAX_WIDTH);
								Size.cx = _UL(abc.abcB) | GR_OC_LEFT_FLUSHED;
							}
							else
							{
								Size.cx = _UL(abc.abcB);
								Size.cx = -Size.cx;
							}
						}
					}
				
#if 0
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
					setWidth(k,Size.cx);
					xxx_UT_DEBUGMSG(("gr_Win32Graphics::getCharWidths: 0x%x: Size.cx %d\n", k, Size.cx));
				}
			}
		}
	}
}
