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
#include "ut_Win32OS.h"
#include "gr_Win32CharWidths.h"
#include "gr_Graphics.h"
#include "ut_debugmsg.h"
#include "ut_OverstrikingChars.h"
#include "ut_vector.h"
#include "ut_endian.h"

//////////////////////////////////////////////////////////////////

void GR_Win32CharWidths::setCharWidthsOfRange(HDC hdc, UT_UCSChar c0, UT_UCSChar c1)
{
	if(m_vRanges.getItemCount() == 0)
	{
		_retrieveFontInfo(hdc);
	}
	
	UINT k;
	int w;
#ifdef DEBUG
	DWORD iErrorCode = 0;
#endif

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
			if(k == 0x200B || k == 0xFEFF || k == UCS_LIGATURE_PLACEHOLDER)
				setWidth(k,0);
			else if(!_doesGlyphExist(k))
			{
				setWidth(k,GR_CW_ABSENT);
			}
			else
			{
				GetCharWidth32W(hdc,k,k,&w);
#ifdef DEBUG
				ABC abc;
				int iRes = GetCharABCWidthsW(hdc,k,k,&abc);
#endif
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
						UT_DebugOnly<int> iRes = GetCharABCWidthsW(hdc,k,k,&abc);
						UT_ASSERT( iRes );
						

						if(iOver == UT_OVERSTRIKING_LEFT)
						{
							UT_ASSERT( abc.abcB <  GR_OC_MAX_WIDTH);
							w = abc.abcB | GR_OC_LEFT_FLUSHED;
						}
						else
						{
							w = abc.abcB /*+ abc.abcA + abc.abcC*/;
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
		LOGFONTW aLogFont;
		UT_DebugOnly<int> iRes = GetObjectW(hFont, sizeof(LOGFONTW), &aLogFont);
		UT_ASSERT(iRes);

		xxx_UT_DEBUGMSG(("gr_Win32Graphics::getCharWidth: extra interchar. spacing %d\n", GetTextCharacterExtra(hdc)));
		
		if(aLogFont.lfCharSet == SYMBOL_CHARSET)
		{
			// Symbol character handling
			for (k=c0; k<=c1; k++)
			{
				if(!_doesGlyphExist(k))
				{
					setWidth(k,GR_CW_ABSENT);
				}
				else
				{
					SIZE Size;
					char str[sizeof(UT_UCSChar)];
					int iConverted = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR) &k, 1,
														 str, sizeof(str), NULL, NULL);
					GetTextExtentPoint32A(hdc, str, iConverted, &Size);
					setWidth(k, Size.cx);
				}
			}
		}
		else
		{
			// Unicode font and default character sets
			for (k=c0; k<=c1; k++)
			{
				if(k == 0x200B || k == 0xFEFF || k == UCS_LIGATURE_PLACEHOLDER)
					setWidth(k,0);
				else if(!_doesGlyphExist(k))
				{
					setWidth(k,GR_CW_ABSENT);
				}
				else
				{
					SIZE Size;
					wchar_t sz1[2];
					sz1[0] = k;
					
					GetTextExtentPoint32W(hdc, sz1, 1, &Size);
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
							iRes = GetCharABCWidthsW(hdc,k,k,&abc);

							// I have commented out the assert below,
							// because when the function above is called for
							// the first time, it seems to always
							// return 0, even though it fills the abc
							// structure with reasonable values,
							// Tomas, June 22, 2003
							// UT_ASSERT( iRes );
#ifdef DEBUG
							if(!iRes)
							{
								iErrorCode = GetLastError();
							}
#endif

							if(iOver == UT_OVERSTRIKING_LEFT)
							{
								UT_ASSERT( abc.abcB <  GR_OC_MAX_WIDTH);
								Size.cx = abc.abcB | GR_OC_LEFT_FLUSHED;
							}
							else
							{
								Size.cx = abc.abcB;
								Size.cx = -Size.cx;
							}
						}
					}
				
#ifdef DEBUG
					if(iErrorCode)
					{
						LPVOID lpMsgBuf;
						FormatMessageW( 
									  FORMAT_MESSAGE_ALLOCATE_BUFFER | 
									  FORMAT_MESSAGE_FROM_SYSTEM | 
									  FORMAT_MESSAGE_IGNORE_INSERTS,
									  NULL,
									  iErrorCode,
									  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
									  (LPWSTR) &lpMsgBuf,
									  0,
									  NULL 
									  );
						// Process any inserts in lpMsgBuf.
						// ...
						// Display the string.
						//MessageBox( NULL, (LPCTSTR)lpMsgBuf, "Error", MB_OK | MB_ICONINFORMATION );
						UT_DEBUGMSG(("char width error: %s\n", lpMsgBuf));
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

bool GR_Win32CharWidths::_doesGlyphExist(UT_UCS4Char g)
{
	for(UT_sint32 i = 0; i < m_vRanges.getItemCount() - 1; i+=2)
	{
		if((UT_uint32)m_vRanges.getNthItem(i) > g)
			return false;

		if((UT_uint32)m_vRanges.getNthItem(i+1)>= g)
			return true;
	}

	return false;
}

// the following macros and some of the code below is based on Pango
#define MAKE_TT_TABLE_NAME(c1, c2, c3, c4) \
   (((UT_uint32)c4) << 24 | ((UT_uint32)c3) << 16 | ((UT_uint32)c2) << 8 | ((UT_uint32)c1))

#define CMAP (MAKE_TT_TABLE_NAME('c','m','a','p'))

// need to do Big to little endian conversions
#ifdef UT_LITTLE_ENDIAN		
#define DO_BE2LE(x) x = ((x << 8) | (x >> 8))
#define DO_BE2LE32(x) x = ((x << 24) | (x >> 24) |   \
						   ((x & 0x00ff0000) >> 8) | ((x & 0x0000ff00) << 8))
#else
#define DO_BE2LE(x)
#define DO_BE2LE32(x)
#endif

void GR_Win32CharWidths::_retrieveFontInfo(HDC hdc)
{
	// retrieve CMAP table for this font
	DWORD iTableSize = GetFontData(hdc, CMAP, 0, NULL, 0);
	char * buff = NULL, * p;
	UT_uint16 i, j, k;
	UT_uint16 iTCount;
	
	if(!iTableSize || iTableSize==GDI_ERROR)
		goto end_processing;
	
	buff = new char[iTableSize];
	if(!buff)
		goto end_processing;
	
	if(GDI_ERROR == GetFontData(hdc, CMAP, 0, (LPVOID)buff, iTableSize))
		goto end_processing;

	
	// now process the data in the table ...
	p = buff;
	UT_uint32 iOffset;
	UT_uint16 iFormat;
	
	iTCount = *((UT_uint16*)&buff[2]);
	DO_BE2LE(iTCount);
	
	p += 4;
	
	
	for(i = 0; i < iTCount; i++)
	{
		UT_uint16 iPlatform = *((UT_uint16*)p);
		DO_BE2LE(iPlatform);
		p += 2;

		UT_uint16 iId = *((UT_uint16*)p);
		DO_BE2LE(iId);
		p += 2;

		if(iPlatform == 3 && iId == 1)
		{
			iOffset = *((UT_uint32*)p);
			DO_BE2LE32(iOffset);

			p += 4;
			
			iFormat = *((UT_uint16*)(&buff[0]+iOffset));
			DO_BE2LE(iFormat);


			if(iFormat != 4)
			{
				continue;
			}

			p = &buff[0] + iOffset + 2;

			UT_uint16 iLength = *((UT_uint16*)p);
			DO_BE2LE(iLength);
			
			p += 4;

			UT_uint16 iSegCount = *((UT_uint16*)p);
			DO_BE2LE(iSegCount);
			iSegCount /= 2;

			p += 8;

			UT_uint16 * pEnd = (UT_uint16*)p;
			p += 2*iSegCount + 2;

			UT_uint16 * pStart = (UT_uint16*)p;
			p += 2*iSegCount;
			
			//UT_uint16 * pDelta = (UT_uint16*)p;
			p += 2*iSegCount;
			
			UT_uint16 * pRangeOffset = (UT_uint16*)p;
			p += 2*iSegCount;

			for(j = 0; j < iSegCount; j++)
			{
				DO_BE2LE(pRangeOffset[j]);
				DO_BE2LE(pStart[j]);
				DO_BE2LE(pEnd[j]);

				if(pRangeOffset[j] == 0)
				{
					m_vRanges.addItem(pStart[j]);
					m_vRanges.addItem(pEnd[j]);
				}
				else
				{
					UT_uint16 iStartChar = pStart[j];
					for(k = pStart[j]; k <= pEnd[j]; k++)
					{
						if(pStart[j] == 0xffff && pEnd[j] == 0xffff)
						{
							// pEnd[j] == 0xffff indicates the last
							// segment; according to the docs this
							// segment need not, but can, contain any
							// valid mappings, and the docs do
							// not. Presumably if the segement
							// contains more than on character there
							// are some valid mapings, if not, i.e.,
							// If pStart[j] == 0xffff, we will
							// assume that the mapping is not valid
							// Tomas, Oct 25, 2003
							break;
						}
						
						if(0 == *(pRangeOffset[j]/2 + (k - pStart[j]) + &pRangeOffset[j]))
						{
							// this character is not present, dump
							// everyting up to here
							if(k > iStartChar)
							{
								m_vRanges.addItem(iStartChar);
								m_vRanges.addItem(k-1);
							}
							iStartChar = k + 1;
						}
					}

					if(iStartChar <= k - 1)
					{
						// dump what is left over
						m_vRanges.addItem(iStartChar);
						m_vRanges.addItem(k-1);
					}
				}
			}
			// not interested in any further tables
		}
		else if(iPlatform == 3 && iId == 0)
		{
			// symbol font
			m_vRanges.addItem(0x20);
			m_vRanges.addItem(0x255);
		}
		else
		{
			// move past the offset long
			p += 4;
		}

	}

	// g_free the table data
	delete [] buff;

 end_processing:	
	// sanity check
	if(m_vRanges.getItemCount() == 0)
	{
		UT_DEBUGMSG(("GR_Win32CharWidths::_retrieveFontInfo: no ranges in font!!!\n"));
		// just assume everything is present
		m_vRanges.addItem(0);
		m_vRanges.addItem(0xffffffff);
	}
#ifdef DEBUG
	for(UT_sint32 n = 0; n < m_vRanges.getItemCount() - 1; n += 2)
	{
		xxx_UT_DEBUGMSG(("GR_Win32CharWidths::_retrieveFontInfo: range 0x%04x - 0x%04x\n",
					 m_vRanges.getNthItem(n), m_vRanges.getNthItem(n+1)));
	}
	
#endif
}

#undef MAKE_TT_TABLE_NAME
#undef CMAP
#undef DO_BE2LE
