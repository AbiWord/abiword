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

#include <stdlib.h>
#include <png.h>

#include "xap_UnixPSImage.h"
#include "ut_assert.h"
#include "ut_bytebuf.h"

PS_Image::PS_Image(const char* szName)
{
	m_image = NULL;
	
	if (szName)
	{
		strcpy(m_szName, szName);
	}
	else
	{
		strcpy(m_szName, "PostScriptImage");
	}
}

struct _bb
{
	const UT_ByteBuf* pBB;
	UT_uint32 iCurPos;
};

static void _png_read(png_structp png_ptr, png_bytep data, png_size_t length)
{
	struct _bb* p = (struct _bb*) png_get_io_ptr(png_ptr);
	const UT_Byte* pBytes = p->pBB->getPointer(0);

	memcpy(data, pBytes + p->iCurPos, length);
	p->iCurPos += length;
}

PS_Image::~PS_Image()
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

UT_Bool PS_Image::convertToBuffer(UT_ByteBuf ** /* ppBB */) const
{
	UT_ASSERT(UT_SHOULD_NOT_HAPPEN);
	
	return UT_FALSE;
}

UT_Bool PS_Image::convertFromBuffer(const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight)
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
	if (color_type == PNG_COLOR_TYPE_PALETTE)
	{
		png_set_expand(png_ptr);
	}

	UT_uint32 iBytesInRow = width * 3;

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

	/*
	  Note that we do NOT create a PSFatmap of iDisplayWidth,iDisplayHeight, since
	  PostScript images can be stretched automatically.  So we simply remember
	  the display size for drawing later.
	*/

	setDisplaySize(iDisplayWidth, iDisplayHeight);
	
	m_image = new PSFatmap;
	m_image->width = width;
	m_image->height = height;

	// allocate for 3 bytes each pixel (one for R, G, and B)
	m_image->data = (guchar *) calloc(m_image->width * m_image->height * 3, sizeof(guchar));
	
	if (!m_image->data)
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		return UT_FALSE;
	}

	UT_Byte * pBits = (UT_Byte *) m_image->data;

	UT_Byte ** pRowStarts = (UT_Byte **) calloc(height, sizeof(UT_Byte *));

	// fill a list of the starts of rows, so png_read_rows() can walk
	// the pointer it gets (&pRowStarts) up each element to get a new
	// place (row) to throw data.
	for (UT_uint32 iRow = 0; iRow < height; iRow++)
		pRowStarts[iRow] = ((UT_Byte *) pBits + (iRow * iBytesInRow));

	png_read_rows(png_ptr, pRowStarts, NULL, height);

	free(pRowStarts);
	
	/* read rest of file, and get additional chunks in info_ptr - REQUIRED */
	png_read_end(png_ptr, info_ptr);

	/* clean up after the read, and free any memory allocated - REQUIRED */
	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

	return UT_TRUE;
}
