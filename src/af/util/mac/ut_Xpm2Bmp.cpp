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

#ifndef XP_MAC_TARGET_MACOSX
# include <QuickDraw.h>
# include <MacMemory.h>
# include <QuickTime.h>
#else
# include <ApplicationServices/ApplicationServices.h>
# include <QuickTime/ImageCompression.h>
#endif


#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_misc.h"
#include "ut_hash.h"
#include "ut_string.h"
#include "ut_MacAssert.h"


typedef struct _my_argb {
	RGBColor rgb;
	unsigned char alpha;
} my_argb;


bool UT_Xpm2Pict (const char ** pIconData,
					UT_uint32 sizeofData,
					PicHandle * pCIcon)
{
	UT_ASSERT(pIconData && *pIconData);
	UT_ASSERT(sizeofData > 0);
	UT_ASSERT(pCIcon);
	
	// first row contains: width height, number of colors, chars per pixel
	UT_uint32 width, height, nrColors, charsPerPixel;
	UT_uint32 n = sscanf(pIconData[0],"%ld %ld %ld %ld",
					 &width,&height,&nrColors,&charsPerPixel);
	UT_ASSERT(n == 4);
	UT_ASSERT(width > 0);
	UT_ASSERT(height > 0);
	UT_ASSERT((nrColors > 0) && (nrColors < 256));
	UT_ASSERT(charsPerPixel > 0);

	GWorldPtr  iconImage;
	PicHandle 	newPict;
	OSStatus err;
	Rect bounds;
	
	::SetRect (&bounds, 0, 0, width, height);

    // Create a gworld image for the icon (and we'll do one for the mask).
    err = ::QTNewGWorld(&iconImage, k32ARGBPixelFormat, &bounds, NULL, NULL, 0);
    if (err == noErr) {

		PixMapHandle iconPixMap = ::GetGWorldPixMap(iconImage);
		if ( NULL != iconPixMap && ::LockPixels(iconPixMap)) {
			char *        imageDstPtr;
			long        bytesPerRow;
	        imageDstPtr = ::GetPixBaseAddr (iconPixMap );
			bytesPerRow = ::GetPixRowBytes (iconPixMap );

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
					pRGB[k].rgb.red = 255;
					pRGB[k].rgb.green = 255;
					pRGB[k].rgb.blue = 255;
					pRGB[k].alpha = 255;
				}
				else
				{
					// TODO fix this to also handle 
					// #ffffeeeedddd type color references
					UT_ASSERT((bufColorValue[0] == '#') && strlen(bufColorValue)==7);
					UT_parseColor(bufColorValue, color);
					pRGB[k].rgb.red	= color.m_red ;
					pRGB[k].rgb.green	= color.m_grn;
					pRGB[k].rgb.blue	= color.m_blu;
					pRGB[k].alpha   = 0;
				}
			}


			int rgb_index;
			const char ** pIconDataImage = &pIconDataPalette[nrColors];
			
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
		
					*(imageDstPtr + kRow * bytesPerRow + kCol*4) 
									= pRGB[rgb_index].alpha;
					*(imageDstPtr + kRow * bytesPerRow + kCol*4 + 1) 
									= pRGB[rgb_index].rgb.red;
					*(imageDstPtr + kRow * bytesPerRow + kCol*4 + 2) 
									= pRGB[rgb_index].rgb.green;
					*(imageDstPtr + kRow * bytesPerRow + kCol*4 + 3) 
									= pRGB[rgb_index].rgb.blue; 
				}
			}

			free(pRGB);

			GWorldPtr offscreen;
		
			err = QTNewGWorld( &offscreen, k32ARGBPixelFormat, &bounds, NULL, NULL, 0 );
			if ((err == noErr) && (offscreen)) { 
				GrafPtr thePort;
				::GetPort (&thePort);
				::SetPort (offscreen);
				newPict = ::OpenPicture (&bounds);
				::CopyBits (GetPortBitMapForCopyBits (iconImage),
							GetPortBitMapForCopyBits (offscreen),
							&bounds, &bounds, srcCopy, NULL);
				::ClosePicture ();
				::SetPort (thePort);
				::DisposeGWorld (offscreen);
			}
			::UnlockPixels (iconPixMap);
		}
		::DisposeGWorld (iconImage);
	}
	*pCIcon = newPict;
	return true;
}



