/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiSource Application Framework
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001 Hubert Figuiere
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
 
#import <Cocoa/Cocoa.h>

#include <stdio.h>
#include <string.h>

#include "ut_misc.h"
#include "ut_hash.h"
#include "ut_assert.h"
#include "ut_string.h"

#include "xap_CocoaToolbar_Icons.h"

AP_CocoaToolbar_Icons::AP_CocoaToolbar_Icons(void)
{
}

AP_CocoaToolbar_Icons::~AP_CocoaToolbar_Icons(void)
{
	// TODO do we need to keep some kind of list
	// TODO of the things we have created and
	// TODO handed out, so that we can delete them ??
}


typedef struct _my_argb {
	UT_RGBColor rgb;
	unsigned char alpha;
} my_argb;


/*!
	returns the pixmap for the named icon
	
	\param szIconName the name of the icon
	\returnvalue pwPixmap the newly allocated NSImage
 */
bool AP_CocoaToolbar_Icons::getPixmapForIcon(const char * szIconName, NSImage ** pwPixmap)
{
	UT_ASSERT(szIconName && *szIconName);
	UT_ASSERT(pwPixmap);
	NSImage *pixmap;

	UT_uint32 width, height, nrColors, charsPerPixel;
	
	const char ** pIconData = NULL;
	UT_uint32 sizeofIconData = 0;		// number of cells in the array
	
	bool bFound = _findIconDataByName(szIconName, &pIconData, &sizeofIconData);
	if (!bFound)
		return false;


	UT_uint32 n = sscanf(pIconData[0],"%ld %ld %ld %ld",
					&width,&height,&nrColors,&charsPerPixel);
	my_argb *pRGB = (my_argb*)malloc((nrColors + 1) * sizeof(my_argb));
	UT_ASSERT(pRGB);
	UT_StringPtrMap hash(61);
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
		
		UT_uint32 nf = sscanf(&pIconDataPalette[k][charsPerPixel+1],
						" %s %s",&bufKey,&bufColorValue);
		UT_ASSERT(nf == 2);
		UT_ASSERT(bufKey[0] = 'c');

		// make the ".." a hash key and store our color index as the data.
		// we add k+1 because the hash code does not like null pointers...
		hash.insert(bufSymbol,(void *)(k+1));
		
		// store the actual color value in the 
		// rgb quad array with our color index.
		if (UT_stricmp(bufColorValue,"None")==0) {
			pRGB[k].rgb.m_red = 255;
			pRGB[k].rgb.m_grn = 255;
			pRGB[k].rgb.m_blu = 255;
			pRGB[k].alpha = 0;
		}
		else
		{
			// TODO fix this to also handle 
			// #ffffeeeedddd type color references
			UT_ASSERT((bufColorValue[0] == '#') && strlen(bufColorValue)==7);
			UT_parseColor(bufColorValue, color);
			pRGB[k].rgb = color;
			pRGB[k].alpha   = 255;
		}
	}

	long bytesPerRow = width;
	NSBitmapImageRep *bitmap = [[NSBitmapImageRep alloc] initWithBitmapDataPlanes:NULL
							pixelsWide:width pixelsHigh:height bitsPerSample:8 samplesPerPixel:4
							hasAlpha:YES isPlanar:YES 
							colorSpaceName:NSDeviceRGBColorSpace bytesPerRow:0 
							bitsPerPixel:0];

	int rgb_index;
	const char ** pIconDataImage = &pIconDataPalette[nrColors];
	unsigned char *planes [5];
	[bitmap getBitmapDataPlanes:planes];

	for (UT_uint32 kRow=0; (kRow < height); kRow++) {
		const char * p = pIconDataImage[kRow];
		
		for (UT_uint32 kCol=0; (kCol < width); kCol++) {
			char bufPixel[10];
			memset(bufPixel,0,sizeof(bufPixel));
			for (UT_uint32 kPx=0; (kPx < charsPerPixel); kPx++)
				bufPixel[kPx] = *p++;

			//printf("Looking for character %s \n", bufPixel);
			const void * pEntry = hash.pick(bufPixel);
			
			rgb_index = ((UT_Byte)(pEntry)) -1;
			//printf("Returned hash index %d \n", rgb_index); 

			*(planes[0] + kRow * bytesPerRow + kCol) 
							= pRGB[rgb_index].rgb.m_red;
			*(planes[1] + kRow * bytesPerRow + kCol) 
							= pRGB[rgb_index].rgb.m_grn;
			*(planes[2] + kRow * bytesPerRow + kCol) 
							= pRGB[rgb_index].rgb.m_blu; 
			*(planes[3] + kRow * bytesPerRow + kCol) 
							= pRGB[rgb_index].alpha;
		}
	}

	FREEP(pRGB);

//	NSData * iconData = [NSData dataWithBytes:(const void *)pIconData length:sizeofIconData];
//	pixmap = [[NSImage alloc] initWithData:iconData];
	pixmap = [[NSImage alloc] initWithSize:NSMakeSize(0.0, 0.0)];
	[pixmap addRepresentation:bitmap];
	[bitmap release];
//	[iconData release];

	UT_ASSERT (pixmap);	
	if (!pixmap)
		return false;

	*pwPixmap = pixmap;
	return true;
}

