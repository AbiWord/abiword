/* AbiSource Program Utilities
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2000 Hubert Figuiere <hfiguiere@teaser.fr>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <QuickDraw.h>
#include <MacMemory.h>

#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_misc.h"
#include "ut_hash.h"
#include "ut_string.h"
#include "ut_MacAssert.h"

#define _RoundUp(x,y) ((((x)+((y)-1))/(y))*(y))

/*****************************************************************/

UT_Bool UT_Xpm2Bmp(UT_uint32 maxWidth,
				   UT_uint32 maxHeight,
				   const char ** pIconData,
				   UT_uint32 sizeofData,
				   CGrafPtr port,
				   UT_RGBColor * pBackgroundColor,
				   PixMapHandle * pBitmap)
{
	// convert an XPM into a BMP using a DIB.
	// return true if successful.

	UT_ASSERT(pIconData && *pIconData);
	UT_ASSERT(sizeofData > 0);
	UT_ASSERT(pBackgroundColor);
	UT_ASSERT(pBitmap);

	// first row contains: width height, number of colors, chars per pixel

	UT_uint32 width, height, nrColors, charsPerPixel;
	UT_uint32 n = sscanf(pIconData[0],"%ld %ld %ld %ld",
						 &width, &height, &nrColors, &charsPerPixel);
	UT_ASSERT(n == 4);
	UT_ASSERT(width > 0);
	UT_ASSERT(height > 0);
	UT_ASSERT((nrColors > 0) && (nrColors < 256));
	UT_ASSERT(charsPerPixel > 0);

	UT_ASSERT(width <= maxWidth);
	UT_ASSERT(height <= maxHeight);
	
	UT_uint32 sizeofColorData = nrColors * sizeof(RGBColor);
	UT_uint32 widthRoundedUp = _RoundUp(width,sizeof(long));
	UT_uint32 rowPadding = widthRoundedUp - width;
	UT_uint32 sizeofPixelData = widthRoundedUp * height;
	
	
	PixMapHandle hBitmap = NewPixMap ();
	Rect bounds;
	bounds.top = 0;
	bounds.left = 0;
	bounds.right = width;
	bounds.bottom = height;
	(*hBitmap)->bounds = bounds;
	(*hBitmap)->pixelSize = 8;

	RGBColor *pRGB = (RGBColor *) NewPtr ((nrColors + 1) * sizeof(RGBColor));
	UT_ASSERT (pRGB != NULL);
	
	UT_HashTable hash(61);
	UT_RGBColor color(0,0,0);
	
	// walk thru the palette

	const char ** pIconDataPalette = &pIconData[1];
	for (UT_uint32 k=0; (k < nrColors); k++)
	{
		char bufSymbol[10];
		char bufKey[10];
		char bufColorValue[100];

		// we expect something of the form: ".. c #000000"
		// but we allow a space as a character in the symbol, so we
		// get the first field the hard way.

		memset(bufSymbol,0,sizeof(bufSymbol));
		for (UT_uint32 kPx=0; (kPx < charsPerPixel); kPx++)
			bufSymbol[kPx] = pIconDataPalette[k][kPx];
		UT_ASSERT(strlen(bufSymbol) == charsPerPixel);
		
		UT_uint32 nf = sscanf(&pIconDataPalette[k][charsPerPixel+1]," %s %s",&bufKey,&bufColorValue);
		UT_ASSERT(nf == 2);
		UT_ASSERT(bufKey[0] = 'c');

		// make the ".." a hash key and store our color index as the data.
		// we add k+1 because the hash code does not like null pointers...
		
		UT_sint32 resultHash = hash.addEntry(bufSymbol,0,(void *)(k+1));
		UT_ASSERT(resultHash != -1);
		
		// store the actual color value in the rgb quad array with our color index.

		if (UT_stricmp(bufColorValue,"None")==0)
		{
			pRGB[k].red		= pBackgroundColor->m_red;
			pRGB[k].green	= pBackgroundColor->m_grn;
			pRGB[k].blue	= pBackgroundColor->m_blu;
		}
		else
		{
			UT_ASSERT((bufColorValue[0] == '#') && strlen(bufColorValue)==7);
			UT_parseColor(bufColorValue, color);
			pRGB[k].red		= color.m_red;
			pRGB[k].green	= color.m_grn;
			pRGB[k].blue	= color.m_blu;
		}
	}

	// walk thru the image data

	UT_Byte *pPixel = NULL;

	const char ** pIconDataImage = &pIconDataPalette[nrColors];
	for (UT_uint32 kRow=0; (kRow < height); kRow++)
	{
		const char * p = pIconDataImage[kRow];
		
		for (UT_uint32 kCol=0; (kCol < width); kCol++)
		{
			char bufPixel[10];
			memset(bufPixel,0,sizeof(bufPixel));
			for (UT_uint32 kPx=0; (kPx < charsPerPixel); kPx++)
				bufPixel[kPx] = *p++;

			UT_HashEntry * pEntry = hash.findEntry(bufPixel);
			*pPixel++ = ((UT_Byte)(pEntry->pData)) - 1;
		}

		pPixel += rowPadding;
	}

//	UT_ASSERT(pPixel == (pInfo + sizeofStructure));
//	pPixel = (UT_Byte *)(pInfo + sizeof(BITMAPINFOHEADER) + sizeofColorData);
		
//	HBITMAP hBitmap = CreateDIBitmap(hdc,pbmih,CBM_INIT,pPixel,pbmi,DIB_RGB_COLORS);
	*pBitmap = hBitmap;

//	free(pInfo);
	::DisposePtr ((Ptr)pRGB);
	pRGB = NULL; 

	return (hBitmap != 0);
}

		
			
		
		
