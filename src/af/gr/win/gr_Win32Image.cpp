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

// *sigh* lots and lots of defines needed to not make the compiler try to
// parse over 512KB (and then some) of include files...
#define WIN32_LEAN_AND_MEAN
#define NOUSER
#define NOHELP
#define NOSYSPARAMSINFO
#define NOWINABLE
#define NOMETAFILE
#define NOTEXTMETRIC
#define NOGDICAPMASKS
#define NOSERVICE
#define NOIME
#define NOMCX
#include <windows.h>

#include "gr_Win32Image.h"
#include "png.h"

#include "ut_assert.h"
#include "ut_bytebuf.h"


GR_Win32Image::GR_Win32Image(const char* szName)
:	m_pDIB(0)
{
	if (szName)
	{
		strcpy(m_szName, szName);
	}
	else
	{
		strcpy(m_szName, "Win32Image");
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

bool GR_Win32Image::convertFromBuffer(const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight)
{
	png_structp png_ptr;
	png_infop info_ptr;
	png_uint_32 width, height;
	int bit_depth, color_type, interlace_type;

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, (void*) NULL,
									 NULL, NULL);

	if (png_ptr == NULL)
	{
		return false;
	}

	/* Allocate/initialize the memory for image information.  REQUIRED. */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		return false;
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
		return false;
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

	/* flip the RGB pixels to BGR (or RGBA to BGRA) */
	png_set_bgr(png_ptr);

	UT_uint32 iBytesInRow = width * 3;
	if (iBytesInRow % 4)
	{
		iBytesInRow += (4 - (iBytesInRow % 4));
	}

	m_pDIB = (BITMAPINFO*) malloc(sizeof(BITMAPINFOHEADER) + height * iBytesInRow);
	if (!m_pDIB)
	{
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		return false;
	}

	/*
	  Note that we do NOT create a DIB of iDisplayWidth,iDisplayHeight, since
	  DIBs can be stretched automatically by the Win32 API.  So we simply remember
	  the display size for drawing later.
	*/

	setDisplaySize(iDisplayWidth, iDisplayHeight);

	m_pDIB->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_pDIB->bmiHeader.biWidth = width;
	m_pDIB->bmiHeader.biHeight = height;
	m_pDIB->bmiHeader.biPlanes = 1;
	m_pDIB->bmiHeader.biBitCount = 24;
	m_pDIB->bmiHeader.biCompression = BI_RGB;
	m_pDIB->bmiHeader.biSizeImage = 0;
	m_pDIB->bmiHeader.biXPelsPerMeter = 0;
	m_pDIB->bmiHeader.biYPelsPerMeter = 0;
	m_pDIB->bmiHeader.biClrUsed = 0;
	m_pDIB->bmiHeader.biClrImportant = 0;

	UT_Byte* pBits = ((unsigned char*) m_pDIB) + m_pDIB->bmiHeader.biSize;

	for (; iInterlacePasses; iInterlacePasses--)
	{
		for (UT_uint32 iRow = 0; iRow < height; iRow++)
		{
			UT_Byte* pRow = pBits + (height - iRow - 1) * iBytesInRow;

			png_read_rows(png_ptr, &pRow, NULL, 1);
		}
	}

	/* read rest of file, and get additional chunks in info_ptr - REQUIRED */
	png_read_end(png_ptr, info_ptr);

	/* clean up after the read, and free any memory allocated - REQUIRED */
	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

	return true;
}

GR_Win32Image::~GR_Win32Image()
{
	free(m_pDIB);
}

static void _png_write(png_structp png_ptr, png_bytep data, png_size_t length)
{
	UT_ByteBuf* pBB = (UT_ByteBuf*) png_get_io_ptr(png_ptr);

	pBB->ins(pBB->getLength(), data, length);
}

static void _png_flush(png_structp png_ptr)
{
}

static void _png_warning(png_structp png_ptr, png_const_charp message)
{
}

static void _png_error(png_structp png_ptr, png_const_charp message)
{
}

bool GR_Win32Image::convertToBuffer(UT_ByteBuf** ppBB) const
{
	/*
	  The purpose of this routine is to convert our DIB (m_pDIB)
	  into a PNG image, storing it in a ByteBuf and returning it
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
		
		return false;
	}

	// We want libpng to write to our ByteBuf, not stdio
	png_set_write_fn(png_ptr, (void *)pBB, _png_write, _png_flush);

	UT_uint32 iWidth = m_pDIB->bmiHeader.biWidth;
	UT_uint32 iHeight;

	/*
	  DIBs are usually bottom-up (backwards, if you ask me), but
	  sometimes they are top-down.
	*/
	bool		bTopDown = false;

	if (m_pDIB->bmiHeader.biHeight < 0)
	{
		iHeight = -(m_pDIB->bmiHeader.biHeight);
		bTopDown = true;
	}
	else
	{
		iHeight =  (m_pDIB->bmiHeader.biHeight);
	}
	
	png_set_IHDR(png_ptr,
				 info_ptr,
				 iWidth,
				 iHeight,
				 8,							// 8 bits per channel (24 bits)
				 PNG_COLOR_TYPE_RGB,
				 PNG_INTERLACE_NONE,
				 PNG_COMPRESSION_TYPE_BASE,
				 PNG_FILTER_TYPE_BASE);

	/* Write the file header information.  REQUIRED */
	png_write_info(png_ptr, info_ptr);

	/*
	  The next big thing is writing out all of the pixels into the PNG
	  file.  We've got quite a bit of code here to handle various kinds
	  of DIB images.
	*/
	
	UT_uint32 iSizeOfColorData = m_pDIB->bmiHeader.biClrUsed * sizeof(RGBQUAD);
	RGBQUAD* pColors = (RGBQUAD*) (((unsigned char*) m_pDIB) + m_pDIB->bmiHeader.biSize);
	UT_Byte* pBits = ((unsigned char*) m_pDIB) + m_pDIB->bmiHeader.biSize + iSizeOfColorData;
	
	UT_Byte* pData = (UT_Byte*) malloc(iWidth * iHeight * 3);
	UT_ASSERT(pData); // TODO outofmem
		
	UT_uint32 	iRow;
	UT_uint32 	iCol;
	UT_Byte* 	pRow;
	UT_uint32 	iBytesInRow;

	/*
	  We can handle a DIB in either 4-bit, 8-bit, or 24-bit format.
	  The way we do this is allocate a single 24-bit buffer, and
	  regardless of what the format of the DIB is, we convert it to
	  our 24-bit buffer.  Below, we will copy our 24-bit buffer into
	  the PNG file.
	*/
	switch (m_pDIB->bmiHeader.biBitCount)
	{
	case 4:
	{
		iBytesInRow = iWidth / 2;
		if (iWidth % 2)
		{
			iBytesInRow++;
		}

		if (iBytesInRow % 4)
		{
			iBytesInRow += (4 - (iBytesInRow % 4));
		}
		
		for (iRow = 0; iRow<iHeight; iRow++)
		{
			if (bTopDown)
			{
				pRow = pBits + iRow * iBytesInRow;
			}
			else
			{
				pRow = pBits + (iHeight - iRow - 1) * iBytesInRow;
			}

			for (iCol=0; iCol<iWidth; iCol++)
			{
				UT_Byte byt = pRow[iCol / 2];
				UT_Byte pixel;
				if (iCol % 2)
				{
					pixel = byt & 0xf;
				}
				else
				{
					pixel = byt >> 4;
				}

				UT_ASSERT(pixel < m_pDIB->bmiHeader.biClrUsed);

				pData[(iRow*iWidth + iCol)*3 + 0] = pColors[pixel].rgbRed;
				pData[(iRow*iWidth + iCol)*3 + 1] = pColors[pixel].rgbGreen;
				pData[(iRow*iWidth + iCol)*3 + 2] = pColors[pixel].rgbBlue;
			}
		}

		break;
	}
	
	case 8:
	{
		iBytesInRow = iWidth;
		if (iBytesInRow % 4)
		{
			iBytesInRow += (4 - (iBytesInRow % 4));
		}

		for (iRow = 0; iRow<iHeight; iRow++)
		{
			if (bTopDown)
			{
				pRow = pBits + iRow * iBytesInRow;
			}
			else
			{
				pRow = pBits + (iHeight - iRow - 1) * iBytesInRow;
			}

			for (iCol=0; iCol<iWidth; iCol++)
			{
				UT_Byte pixel = pRow[iCol];

				UT_ASSERT(pixel < m_pDIB->bmiHeader.biClrUsed);

				pData[(iRow*iWidth + iCol)*3 + 0] = pColors[pixel].rgbRed;
				pData[(iRow*iWidth + iCol)*3 + 1] = pColors[pixel].rgbGreen;
				pData[(iRow*iWidth + iCol)*3 + 2] = pColors[pixel].rgbBlue;
			}
		}

		break;
	}

	case 24:
	{
		iBytesInRow = iWidth * 3;
		if (iBytesInRow % 4)
		{
			iBytesInRow += (4 - (iBytesInRow % 4));
		}

		for (iRow = 0; iRow<iHeight; iRow++)
		{
			if (bTopDown)
			{
				pRow = pBits + iRow * iBytesInRow;
			}
			else
			{
				pRow = pBits + (iHeight - iRow - 1) * iBytesInRow;
			}

			for (iCol=0; iCol<iWidth; iCol++)
			{
				pData[(iRow*iWidth + iCol)*3 + 0] = pRow[iCol*3 + 2];
				pData[(iRow*iWidth + iCol)*3 + 1] = pRow[iCol*3 + 1];
				pData[(iRow*iWidth + iCol)*3 + 2] = pRow[iCol*3 + 0];
			}
		}

		break;
	}

	default:
		// there are DIB formats we do not support.
		UT_ASSERT(UT_TODO);
		break;
	}

	/*
	  Now that we have converted the image to a normalized 24-bit
	  representation, we can save it out to the PNG file.
	*/
	for (UT_uint32 i=0; i<iHeight; i++)
	{
		UT_Byte *pRow = pData + i * iWidth * 3;
		
		png_write_rows(png_ptr, &pRow, 1);
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

	return true;
}

