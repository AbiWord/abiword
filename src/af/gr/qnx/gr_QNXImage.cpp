/* AbiWord
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

#include <stdlib.h>
#include <string.h>
#include <png.h>

#include "gr_QNXImage.h"
#include "ut_assert.h"
#include "ut_bytebuf.h"

GR_QNXImage::GR_QNXImage(Fatmap * image, const char* szName)
{
	m_image = image;
	
	if (szName)
	{
		strcpy(m_szName, szName);
	}
	else
	{
		strcpy(m_szName, "QNXImage");
	}
}

struct _bb
{
	const UT_ByteBuf* pBB;
	UT_uint32 iCurPos;
};

static void _png_read(png_structp png_ptr, png_bytep data, png_size_t length)
{
	struct _bb* p = (struct _bb*) png_ptr->io_ptr;
	const UT_Byte* pBytes = p->pBB->getPointer(0);

	memcpy(data, pBytes + p->iCurPos, length);
	p->iCurPos += length;
}

static void _png_write(png_structp png_ptr, png_bytep data, png_size_t length)
{
	UT_ByteBuf* pBB = (UT_ByteBuf*) png_ptr->io_ptr;

	pBB->ins(pBB->getLength(), data, length);
}

static void _png_flush(png_structp /* png_ptr */)
{
}

GR_QNXImage::~GR_QNXImage()
{
	if (m_image)
	{
		if (m_image->data)
		{
			free(m_image->data);
			m_image->data = NULL;
		}

		delete m_image;
		m_image = NULL;
	}
}

UT_sint32	GR_QNXImage::getDisplayWidth(void) const
{
	return m_image->width;
}

UT_sint32	GR_QNXImage::getDisplayHeight(void) const
{
	return m_image->height;
}

UT_Bool		GR_QNXImage::convertToPNG(UT_ByteBuf** ppBB) const
{
	/*
	  The purpose of this routine is to convert our internal 24-bit
	  Fatmap into a PNG image, storing it in a ByteBuf and returning it
	  to the caller.
	*/

	// Create our bytebuf
	UT_ByteBuf* pBB = new UT_ByteBuf();

	png_structp png_ptr;
	png_infop info_ptr;

	// initialize some libpng stuff
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, (png_voidp)NULL,
									  (png_error_ptr)NULL, (png_error_ptr)NULL);
	
	info_ptr = png_create_info_struct(png_ptr);

	// libpng will longjmp back to here if a fatal error occurs
	if (setjmp(png_ptr->jmpbuf))
	{
		/* If we get here, we had a problem reading the file */
		png_destroy_write_struct(&png_ptr,  (png_infopp)NULL);
		*ppBB = NULL;
		
		return UT_FALSE;
	}

	// We want libpng to write to our ByteBuf, not stdio
	png_set_write_fn(png_ptr, (void *)pBB, _png_write, _png_flush);

	UT_uint32 iWidth = m_image->width;
	UT_uint32 iHeight = m_image->height;

	png_set_IHDR(png_ptr,
				 info_ptr,
				 iWidth,
				 iHeight,
				 8,							// 8 bits per channel (24 bits for all 3 channels)
				 PNG_COLOR_TYPE_RGB,
				 PNG_INTERLACE_NONE,
				 PNG_COMPRESSION_TYPE_BASE,
				 PNG_FILTER_TYPE_BASE);

	/* Write the file header information.  REQUIRED */
	png_write_info(png_ptr, info_ptr);

	UT_Byte * pBits = ((unsigned char*) m_image->data);
	
	UT_Byte* pData = (UT_Byte*) malloc(iWidth * iHeight * 3);
	UT_ASSERT(pData); // TODO outofmem
		
	UT_uint32 	iRow;
	UT_uint32 	iCol;
	UT_Byte* 	pRow;
	UT_uint32 	iBytesInRow;

	// We only handle 24-bit data, as that's all we'll ever be passing to
	// gdk_draw_rgb_image(), and all we'll ever hold internally.
	
	iBytesInRow = iWidth * 3;

	for (iRow = 0; iRow<iHeight; iRow++)
	{
		pRow = pBits + iRow * iBytesInRow;
			
		for (iCol=0; iCol<iWidth; iCol++)
		{
			pData[(iRow*iWidth + iCol)*3 + 0] = pRow[iCol*3 + 0];
			pData[(iRow*iWidth + iCol)*3 + 1] = pRow[iCol*3 + 1];
			pData[(iRow*iWidth + iCol)*3 + 2] = pRow[iCol*3 + 2];
		}
	}
	
	/*
	  Now that we have converted the image to a normalized 24-bit
	  representation, we can save it out to the PNG file.
	*/
	for (UT_uint32 i=0; i<iHeight; i++)
	{
		UT_Byte *pThisRow = pData + i * iWidth * 3;
		
		png_write_rows(png_ptr, &pThisRow, 1);
	}

	/*
	  We then free our 24-bit buffer.
	*/
	free(pData);

	/*
	  Wrap things up with libpng
	*/
	png_write_end(png_ptr, info_ptr);

	/* clean up after the write, and free any memory allocated */
	png_destroy_write_struct(&png_ptr, (png_infopp)NULL);

	// And pass the ByteBuf back to our caller
	*ppBB = pBB;

	return UT_TRUE;
}

UT_Bool	GR_QNXImage::convertFromPNG(const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight)
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (void*) NULL,
									 NULL, NULL);

	if (png_ptr == NULL)
	{
		return UT_FALSE;
	}

	/* Allocate/initialize the memory for image information.  REQUIRED. */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		return UT_FALSE;
	}

	/* Set error handling if you are using the setjmp/longjmp method (this is
	 * the normal method of doing things with libpng).  REQUIRED unless you
	 * set up your own error handlers in the png_create_read_struct() earlier.
	 */
	if (setjmp(png_ptr->jmpbuf))
	{
		/* Free all of the memory associated with the png_ptr and info_ptr */
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
	  
		/* If we get here, we had a problem reading the file */
		return UT_FALSE;
	}

	struct _bb myBB;
	myBB.pBB = pBB;
	myBB.iCurPos = 0;
	
	png_set_read_fn(png_ptr, (void *)&myBB, _png_read);

	/* The call to png_read_info() gives us all of the information from the
	 * PNG file before the first IDAT (image data chunk).  REQUIRED
	 */
	png_read_info(png_ptr, info_ptr);

	png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type,
				 &interlace_type, NULL, NULL);

	/* Extract multiple pixels with bit depths of 1, 2, and 4 from a single
	 * byte into separate bytes (useful for paletted and grayscale images).
	 */
	png_set_packing(png_ptr);

	/* Expand paletted colors into true RGB triplets */
	png_set_expand(png_ptr);

	/*  If we've got images with 16 bits per channel, we don't need that
		much precision.  We'll do fine with 8 bits per channel */
	png_set_strip_16(png_ptr);

	/*  For simplicity, treat grayscale as RGB */
	png_set_gray_to_rgb(png_ptr);

	/*  For simplicity, we'll ignore alpha */
	png_set_strip_alpha(png_ptr);
	
	/*  We want libpng to deinterlace the image for us */
	UT_uint32 iInterlacePasses = png_set_interlace_handling(png_ptr);

	UT_uint32 iBytesInRow = width * 3;

	Fatmap* pFM = new Fatmap;
	pFM->width = width;
	pFM->height = height;

	// allocate for 3 bytes each pixel (one for R, G, and B)
	pFM->data = (unsigned char *) calloc(pFM->width * pFM->height * 3, sizeof(unsigned char));
	
	if (!pFM->data)
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		return UT_FALSE;
	}

	UT_Byte * pBits = (UT_Byte *) pFM->data;

	UT_Byte ** pRowStarts = (UT_Byte **) calloc(height, sizeof(UT_Byte *));

	// fill a list of the starts of rows, so png_read_rows() can walk
	// the pointer it gets (&pRowStarts) up each element to get a new
	// place (row) to throw data.
	for (UT_uint32 iRow = 0; iRow < height; iRow++)
		pRowStarts[iRow] = ((UT_Byte *) pBits + (iRow * iBytesInRow));

	for (; iInterlacePasses; iInterlacePasses--)
		png_read_rows(png_ptr, pRowStarts, NULL, height);
	
	free(pRowStarts);
	
	/* read rest of file, and get additional chunks in info_ptr - REQUIRED */
	png_read_end(png_ptr, info_ptr);

	/* clean up after the read, and free any memory allocated - REQUIRED */
	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

	if (
		(((UT_sint32) width) != iDisplayWidth)
		|| (((UT_sint32) height) != iDisplayHeight)
		)
	{
		Fatmap* pDisplayFM = new Fatmap;
		Fatmap* pOtherFM = pFM;

		pDisplayFM->width = iDisplayWidth;
		pDisplayFM->height = iDisplayHeight;

		// allocate for 3 bytes each pixel (one for R, G, and B)
		pDisplayFM->data = (unsigned char *) calloc(pDisplayFM->width * pDisplayFM->height * 3, sizeof(unsigned char));
	
		if (!pDisplayFM->data)
		{
			delete pDisplayFM;
			free(pOtherFM->data);
			delete pOtherFM;
			return UT_FALSE;
		}

		// stretch the pixels from pOtherFM into pDisplayFM

		/*
		  TODO this code came from imlib.  It's not exactly
		  a match for our coding standards, so it needs a
		  certain amount of cleanup.  However, it seems
		  to be working nicely.
		*/
		
		{
			int                 x, y, *xarray;
			unsigned char     **yarray, *ptr, *ptr2, *ptr22;
			int                 pos, inc, w3;

			xarray = (int*) malloc(sizeof(int) * iDisplayWidth);

			if (!xarray)
			{
				// TODO outofmem
				return UT_FALSE;
			}
			yarray = (unsigned char**) malloc(sizeof(unsigned char *) * iDisplayHeight);

			if (!yarray)
			{
				// TODO outofmem
				return UT_FALSE;
			}
			
			ptr22 = pOtherFM->data;
			w3 = pOtherFM->width * 3;

			// set up xarray
			inc = ((pOtherFM->width) << 16) / iDisplayWidth;
			pos = 0;
			for (x = 0; x < iDisplayWidth; x++)
			{
				xarray[x] = (pos >> 16) + (pos >> 16) + (pos >> 16);
				pos += inc;
			}

			// set up yarray
			inc = ((pOtherFM->height) << 16) / iDisplayHeight;
			pos = 0;
			for (x = 0; x < iDisplayHeight; x++)
			{
				yarray[x] = ptr22 + ((pos >> 16) * w3);
				pos += inc;
			}

			// crunch the data
			ptr = pDisplayFM->data;
			for (y = 0; y < iDisplayHeight; y++)
			{
				for (x = 0; x < iDisplayWidth; x++)
				{
					ptr2 = yarray[y] + xarray[x];
					*ptr++ = (int)*ptr2++;
					*ptr++ = (int)*ptr2++;
					*ptr++ = (int)*ptr2;
				}
			}
		}

		pFM = pDisplayFM;

		free(pOtherFM->data);
		delete pOtherFM;
	}

	// should NOT already be set
	UT_ASSERT(!m_image);

	if (m_image)
	{
		// free the data in it too
		if (m_image->data)
		{
			free(m_image->data);
			m_image->data = NULL;
		}
		
		delete m_image;
		m_image = NULL;
	}

	m_image = pFM;
		
	return UT_TRUE;
}

