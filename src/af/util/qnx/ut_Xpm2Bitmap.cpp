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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ut_types.h"
#include "ut_debugmsg.h"
#include "ut_assert.h"
#include "ut_misc.h"
#include "ut_hash.h"
#include "ut_string.h"

#include <Pt.h>

/*****************************************************************/

bool UT_Xpm2Bitmap(const char ** pIconData,
				   UT_uint32 sizeofData,	//USELESS
				   PhImage_t ** ppImage) {
	PhImage_t *pImage;

	// convert an XPM into a PhImage_t structure
	// return true if successful.
	//printf("Filling Ph_Image structure ...\n");

	UT_ASSERT(pIconData && *pIconData);
	UT_ASSERT(sizeofData > 0);
	UT_ASSERT(ppImage);

	// first row contains: width height, number of colors, chars per pixel
	UT_uint32 width, height, nrColors, charsPerPixel, n;
	//printf("Scanning in the core info [%s] \n", pIconData[0]);
	n = sscanf(pIconData[0],"%u %u %u %u",
				 &width,&height,&nrColors,&charsPerPixel);
	//printf("Scanned w %d, h %d, #c %d, cpp %d\n", width, height, nrColors, charsPerPixel);
	UT_ASSERT(n == 4);
	UT_ASSERT(width > 0);
	UT_ASSERT(height > 0);
	UT_ASSERT(charsPerPixel > 0);

	//Create a new PhImage_t 
	if (!(pImage = (PhImage_t *)malloc(sizeof(*pImage))))
		return(false);
	memset(pImage, 0, sizeof(*pImage));

	pImage->type = Pg_IMAGE_DIRECT_8888; //Could use Pg_IMAGE_DIRECT_888
	pImage->bpl = sizeof(PgColor_t) * width;
	pImage->size.w = width;
	pImage->size.h = height;
	//pImage->palette_tag = ???;
	//pImage->colors = nrColors;
	//pImage->xscale = pImage->yscale = 1;
	pImage->transparent = Pg_MAGENTA;
	pImage->flags = Ph_USE_TRANSPARENCY|Ph_RELEASE_IMAGE;
	//Set these later when we know it works
	//pImage->ghost_bpl = 0;
	//pImage->ghost_bitmap = NULL;
	//pImage->mask_bpl = 0;
	//pImage->mask_bm = NULL;
	//Do we have to set the palette?
	//pImage->palette = 0;
	//printf("Allocating space for image %dx%d \n", width, height);
	pImage->image = (char *)malloc(sizeof(PgColor_t) * width * height);
	if (!pImage->image) {
		free(pImage);	
		return(false);
	}

	PgColor_t *pRGB;
	pRGB = (PgColor_t *)malloc((nrColors + 1) * sizeof(*pRGB));
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
		
		UT_uint32 nf;
		nf = sscanf(&pIconDataPalette[k][charsPerPixel+1],
						" %s %s",bufKey,bufColorValue);
		UT_ASSERT(nf == 2);
		UT_ASSERT(bufKey[0] == 'c');

		// make the ".." a hash key and store our color index as the data.
		// we add k+1 because the hash code does not like null pointers...
		hash.insert(bufSymbol, (void *)(k+1));
		
		// store the actual color value in the 
		// rgb quad array with our color index.
		if (UT_stricmp(bufColorValue,"None")==0) {
			pRGB[k]	= Pg_MAGENTA;
		}
		else
		{
			// TODO fix this to also handle 
			// #ffffeeeedddd type color references
			UT_ASSERT((bufColorValue[0] == '#') && strlen(bufColorValue)==7);
			UT_parseColor(bufColorValue, color);
			pRGB[k] = PgRGB(color.m_red, color.m_grn, color.m_blu);
		}
	}

	// walk thru the image data
	int rgb_index;
	PgColor_t *bits;
	
	bits = (PgColor_t *)pImage->image;

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
			//printf("Setting [%d][%d] to 0%08x \n", kRow, kCol, pRGB[rgb_index]); 
			*bits = pRGB[rgb_index];
			bits++;

			/* When bits was char
			*((PgColor_t *)(bits + 
			                kRow*pImage->bpl + 
							kCol*sizeof(Pg_Color_t))) = pRGB[rgb_index];
			*/
		}
	}
	pImage->image_tag = PtCRC(pImage->image,sizeof(PgColor_t) * width * height);
	free(pRGB);
	*ppImage = pImage;
	return(true);
}

		
			
		
