/* AbiSource Program Utilities
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
#include <stdio.h>
#include <stdlib.h>
#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_color.h"
#include "ut_misc.h"
#include "ut_hash.h"
#include "ut_string.h"
#include "ut_string_class.h"

#define _RoundUp(x,y) ((((x)+((y)-1))/(y))*(y))

/*****************************************************************/

bool UT_Xpm2Bmp(UT_uint32 maxWidth,
				   UT_uint32 maxHeight,
				   const char ** pIconData,
				   UT_uint32 sizeofData,
				   HDC hdc,
				   UT_RGBColor * pBackgroundColor,
				   HBITMAP * pBitmap)
{
	// convert an XPM into a BMP using a DIB.
	// return true if successful.

	UT_DEBUG_ONLY_ARG(sizeofData);

	UT_ASSERT(pIconData && *pIconData);
	UT_ASSERT(sizeofData > 0);
	UT_ASSERT(pBackgroundColor);
	UT_ASSERT(pBitmap);

	// first row contains: width height, number of colors, chars per pixel

	UT_uint32 width, height, xpm_width, xpm_height, nrColors, charsPerPixel;
	UT_uint32 cut_left, cut_top;
	UT_DebugOnly<UT_uint32> n = sscanf(pIconData[0],"%lu %lu %lu %lu",
						 &xpm_width,&xpm_height,&nrColors,&charsPerPixel);
	UT_ASSERT(n == 4);
	UT_ASSERT(xpm_width > 0);
	UT_ASSERT(xpm_height > 0);

	UT_ASSERT(charsPerPixel > 0);

	width = xpm_width < maxWidth ? xpm_width : maxWidth;
	cut_left = xpm_width - width;
	cut_left -= cut_left / 2;
	height = xpm_height < maxHeight ? xpm_height : maxHeight;
	cut_top = xpm_height - height;
	cut_top -= cut_top / 2;
	
	UT_uint32 sizeofColorData = nrColors * sizeof(RGBQUAD);
	UT_uint32 widthRoundedUp = _RoundUp(width,sizeof(LONG));
	UT_uint32 rowPadding = widthRoundedUp - width;
	UT_uint32 sizeofPixelData = widthRoundedUp * height;
	UT_uint32 sizeofStructure = sizeof(BITMAPINFOHEADER) + sizeofColorData + sizeofPixelData;

	UT_Byte * pInfo = (UT_Byte *)UT_calloc(1,sizeofStructure);
	if (!pInfo)
		return false;

	BITMAPINFO * pbmi = (BITMAPINFO *)pInfo;
	BITMAPINFOHEADER * pbmih = &pbmi->bmiHeader;
	RGBQUAD * pRGB = (RGBQUAD *)(pInfo + sizeof(BITMAPINFOHEADER));
	UT_Byte * pPixel = (UT_Byte *)(pInfo + sizeof(BITMAPINFOHEADER) + sizeofColorData);

	pbmih->biSize			= sizeof(BITMAPINFOHEADER);
	pbmih->biWidth			= width;
	pbmih->biHeight			= -(LONG)height;	// minus height gives us a top-down bitmap
	pbmih->biPlanes			= 1;
	pbmih->biBitCount		= 8;
	pbmih->biCompression	= BI_RGB;
	pbmih->biSizeImage		= 0;
	pbmih->biXPelsPerMeter	= 0;
	pbmih->biYPelsPerMeter	= 0;
	pbmih->biClrUsed		= nrColors;	// should we verify that they are all actually used ??
	pbmih->biClrImportant	= 0;

	UT_StringPtrMap hash(256);
	UT_RGBColor color(0,0,0);
	
	// walk thru the palette

	// use a local UT_String, else every call to hash.insert and hash.pick
	// creates and destroys its own UT_String
	UT_String sTmp;

	const char ** pIconDataPalette = &pIconData[1];
	for (UT_uint32 k=0; (k < nrColors); k++)
	{
		char bufSymbol[10] = { 0 };
		char bufKey[10];
		char bufColorValue[100];

		// we expect something of the form: ".. c #000000"
		// or we can accpet ".. g #000000" for gray scale
		// but we allow a space as a character in the symbol, so we
		// get the first field the hard way.

		for (UT_uint32 kPx=0; (kPx < charsPerPixel); kPx++)
			bufSymbol[kPx] = pIconDataPalette[k][kPx];
		UT_ASSERT(strlen(bufSymbol) == charsPerPixel);
		
		UT_DebugOnly<UT_uint32> nf = sscanf(&pIconDataPalette[k][charsPerPixel+1]," %s %s",&bufKey,&bufColorValue);
		UT_ASSERT(nf == 2);
		UT_ASSERT(bufKey[0] == 'c' || bufKey[0] == 'g');

		// make the ".." a hash key and store our color index as the data.
		// we add k+1 because the hash code does not like null pointers...
		sTmp = bufSymbol;
		hash.insert(sTmp, (void *)(k+1));

		// store the actual color value in the rgb quad array with our color index.

		if (g_ascii_strcasecmp(bufColorValue,"None")==0)
		{
			pRGB[k].rgbRed		= pBackgroundColor->m_red;
			pRGB[k].rgbGreen	= pBackgroundColor->m_grn;
			pRGB[k].rgbBlue		= pBackgroundColor->m_blu;
			pRGB[k].rgbReserved	= 0;
		}
		else
		{
			// TODO fix this to also handle #ffffeeeedddd type color references
			
			UT_ASSERT((bufColorValue[0] == '#') && strlen(bufColorValue)==7);
			UT_parseColor(bufColorValue, color);
			pRGB[k].rgbRed		= color.m_red;
			pRGB[k].rgbGreen	= color.m_grn;
			pRGB[k].rgbBlue		= color.m_blu;
			pRGB[k].rgbReserved	= 0;
		}
	}

	// walk thru the image data

	const char ** pIconDataImage = &pIconDataPalette[nrColors];
	for (UT_uint32 kRow=0; (kRow < height); kRow++)
	{
		const char * p = pIconDataImage[kRow+cut_top];
		
		p += cut_left * charsPerPixel;
		for (UT_uint32 kCol=0; (kCol < width); kCol++)
		{
			char bufPixel[10] = { 0 };
			for (UT_uint32 kPx=0; (kPx < charsPerPixel); kPx++)
				bufPixel[kPx] = *p++;

			sTmp = bufPixel;
			guintptr pEntry = (guintptr) hash.pick(sTmp);
			*pPixel++ = ((UT_Byte)(pEntry)) - 1;
		}

		pPixel += rowPadding;
	}

	UT_ASSERT(pPixel == (pInfo + sizeofStructure));
	pPixel = (UT_Byte *)(pInfo + sizeof(BITMAPINFOHEADER) + sizeofColorData);
	
	HBITMAP hBitmap = CreateDIBitmap(hdc,pbmih,CBM_INIT,pPixel,pbmi,DIB_RGB_COLORS);
	*pBitmap = hBitmap;

	g_free(pInfo);
	
	return (hBitmap != 0);
}

		
			
		
		
