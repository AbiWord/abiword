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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

// *sigh* lots and lots of defines needed to not make the compiler try to
// parse over 512KB (and then some) of include files...
#define WIN32_LEAN_AND_MEAN
#define NOUSER
#define NOHELP
#define NOSYSPARAMSINFO
#define NOWINABLE
#define NOMETAFILE
//#define NOTEXTMETRIC
#define NOGDICAPMASKS
#define NOSERVICE
#define NOIME
#define NOMCX
#include <windows.h>

#include "gr_Win32Graphics.h"
#include "gr_Win32Image.h"
#include "png.h"
#include "ut_jpeg.h"

#include "ut_assert.h"
#include "ut_bytebuf.h"


GR_Win32Image::GR_Win32Image(const char* szName)
:	m_pDIB(0)
{
	if (szName)
	{
	  setName( szName );
	}
	else
	{
		
	  setName( "Win32Image" );
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


/*! 
 * Returns true if pixel at point (x,y) in device units is transparent.
 * See gr_UnixImage.cpp for how it's done in GTK.
 */
bool	GR_Win32Image::isTransparentAt(UT_sint32 /*x*/, UT_sint32 /*y*/)
{
	UT_ASSERT_HARMLESS(0);
	return false;
}

/*!
 * Returns true if there is any transparency in the image.
 */ 
bool GR_Win32Image::hasAlpha(void) const
{
	UT_ASSERT_HARMLESS(0);
	return false;
}

bool GR_Win32Image::convertFromBuffer(const UT_ByteBuf* pBB, const std::string& mimetype, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight)
{
	if (mimetype == "image/png")
		return _convertFromPNG(pBB, iDisplayWidth, iDisplayHeight);
	else if (mimetype == "image/jpeg")
		return _convertFromJPEG(pBB, iDisplayWidth, iDisplayHeight);
	return false;
}

GR_Win32Image::~GR_Win32Image()
{
	g_free(m_pDIB);
}

static void _png_write(png_structp png_ptr, png_bytep data, png_size_t length)
{
	UT_ByteBuf* pBB = (UT_ByteBuf*) png_get_io_ptr(png_ptr);

	pBB->ins(pBB->getLength(), data, length);
}

static void _png_flush(png_structp /*png_ptr*/)
{
}

#if 0 //these don't seem to be used

static void _png_warning(png_structp png_ptr, png_const_charp message)
{
}

static void _png_error(png_structp png_ptr, png_const_charp message)
{
}

#endif

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
	if (setjmp(png_jmpbuf(png_ptr)))
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
	
	UT_Byte* pData = (UT_Byte*) g_try_malloc(iWidth * iHeight * 3);
	UT_return_val_if_fail(pData, false); // TODO outofmem
		
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
		UT_ASSERT_HARMLESS(UT_TODO);
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
	  We then g_free our 24-bit buffer.
	*/
	g_free(pData);

	/*
	  Wrap things up with libpng
	*/
	png_write_end(png_ptr, info_ptr);

	/* clean up after the write, and g_free any memory allocated */
	png_destroy_write_struct(&png_ptr, (png_infopp)NULL);

	// And pass the ByteBuf back to our caller
	*ppBB = pBB;

	return true;
}

/*!
 * The idea is to create a
 * new image from the rectangular segment in device units defined by 
 * UT_Rect rec. The Image should be deleted by the calling routine.
 */
GR_Image * GR_Win32Image::createImageSegment(GR_Graphics * pG,const UT_Rect & rec)
{
	// this code assumes 24 bit RGB bitmaps ...
	UT_return_val_if_fail(pG && m_pDIB && m_pDIB->bmiHeader.biBitCount == 24, NULL);

	// the ration of x and y coords for the graphics class
	double fXYRatio = ((GR_Win32Graphics *)pG)->getXYRatio();

	// We have three different coordinate systems here:
	//   
	//   rec: the requested segment size, in layout units
	//   
	//   m_pDIB->bmiHeader.biHeight/Width(): physical size of the bitmap
	//
	//   getDisplayWidth()/Height() : size in device units for which this image was to be
	//                                rendered
	//
	// From these we need to work out the size/offset of the segment in the DIB
	// coordinaces

	// these are the DIB physical dimensions of the original
	UT_sint32 dH = m_pDIB->bmiHeader.biHeight;
	UT_sint32 dW = m_pDIB->bmiHeader.biWidth;

	// this is the internal scaling of the orginal DIB, which we need to workout
	// relationship between display units and DIB units; we need to use separate factors
	// for x and y axis, since the proportions of the DIB have no formal relationship to
	// the dimensions of the displayed rectangle (e.g., the display image could be
	// cropped).
	
	double fDibScaleFactorX = (double) dW / (double)getDisplayWidth();
	double fDibScaleFactorY = (double) dH / (double)getDisplayHeight();

	// these are the requested dimensions in device units
	UT_sint32 widthDU = (UT_sint32)((double)pG->tdu(rec.width) * fXYRatio);
	UT_sint32 heightDU = pG->tdu(rec.height);
	UT_sint32 xDU = (UT_sint32)((double)pG->tdu(rec.left) * fXYRatio);
	UT_sint32 yDU = pG->tdu(rec.top);

	// now convert the DUs into DIB units -- these are the values we need to work with
	// when copying the image segment
	UT_sint32 widthDIB = (UT_sint32)((double)widthDU * fDibScaleFactorX);
	UT_sint32 heightDIB = (UT_sint32)((double)heightDU * fDibScaleFactorY);
	UT_sint32 xDIB = (UT_sint32)((double)xDU * fDibScaleFactorX);
	UT_sint32 yDIB = (UT_sint32)((double)yDU * fDibScaleFactorY);
	
	if(xDIB < 0)
	{
		xDIB = 0;
	}
	if(yDIB < 0)
	{
		yDIB = 0;
	}

	if(heightDIB > dH)
	{
		heightDIB = dH;
	}
	if(widthDIB > dW)
	{
		widthDIB = dW;
	}
	if(xDIB + widthDIB > dW)
	{
		widthDIB = dW - xDIB;
	}
	if(yDIB + heightDIB > dH)
	{
		heightDIB = dH - yDIB;
	}
	if(widthDIB < 0)
	{
		xDIB = dW -1;
		widthDIB = 1;
	}
	if(heightDIB < 0)
	{
		yDIB = dH -1;
		heightDIB = 1;
	}
	
	UT_String sName("");
	getName(sName);
    UT_String sSub("");
	UT_String_sprintf(sSub,"_segment_%d_%d_%d_%d",xDIB,yDIB,widthDIB,heightDIB);
	sName += sSub;
	GR_Win32Image * pImage = new GR_Win32Image(sName.c_str());

	UT_return_val_if_fail( pImage, NULL );
	
	// now allocate memory -- mostly copied from convertFromBuffer()
	UT_uint32 iBytesInRow = widthDIB * 3;
	if (iBytesInRow % 4)
	{
		iBytesInRow += (4 - (iBytesInRow % 4));
	}

	pImage->m_pDIB = (BITMAPINFO*) g_try_malloc(sizeof(BITMAPINFOHEADER) + heightDIB * iBytesInRow);
	UT_return_val_if_fail( pImage->m_pDIB, NULL );

	// simply copy the whole header
	memcpy(pImage->m_pDIB, m_pDIB, sizeof(BITMAPINFOHEADER));

	// now set the image size ...
	pImage->setDisplaySize(widthDU, heightDU);
	pImage->m_pDIB->bmiHeader.biWidth = widthDIB;
	pImage->m_pDIB->bmiHeader.biHeight = heightDIB;
	pImage->m_pDIB->bmiHeader.biSizeImage = 0;

	// now copy the bitmap bits
	// the rows in both maps are/need to be padded to 4-bytes
	// NB: in the DIB lines are numbered from bottom up, while in our coord system y goes
	// from top to bottom

	// how wide is a row in the orignal ?
	UT_uint32 iOrigRowBytes = m_pDIB->bmiHeader.biWidth*3;
	if(iOrigRowBytes%4)
		iOrigRowBytes += (4 - iOrigRowBytes%4);

	// what are the coords in the orginal we want?
	UT_uint32 iByteWidth = widthDIB*3;
	UT_uint32 iLeft = xDIB*3;
	UT_uint32 iRight = iLeft + iByteWidth;
	UT_uint32 iTop = m_pDIB->bmiHeader.biHeight - yDIB;
	UT_uint32 iBottom = iTop - heightDIB;
	UT_uint32 iRightPadded = iRight + (iBytesInRow - iByteWidth);

	UT_Byte * pBits1 = ((UT_Byte*)m_pDIB) + sizeof(BITMAPINFOHEADER);
	UT_Byte * pBits2 = ((UT_Byte*)pImage->m_pDIB) + sizeof(BITMAPINFOHEADER);

	UT_uint32 iRow;
	
	for(iRow = iBottom; iRow < iTop; iRow++)
	{
		memcpy(pBits2, pBits1 + iRow * iOrigRowBytes + iLeft, iByteWidth);
		pBits2 += iByteWidth;
		
		// now the padding ...
		for(UT_uint32 iPad = iRight; iPad < iRightPadded; iPad++, pBits2++)
		{
			*pBits2 = 0;
		}
	}
	
	return static_cast<GR_Image *>(pImage);
}

bool GR_Win32Image::_convertFromPNG(const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight)
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
	if (setjmp(png_jmpbuf(png_ptr)))
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

	m_pDIB = (BITMAPINFO*) g_try_malloc(sizeof(BITMAPINFOHEADER) + height * iBytesInRow);
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

	/* clean up after the read, and g_free any memory allocated - REQUIRED */
	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

	return true;
}

bool GR_Win32Image::_convertFromJPEG(const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight)
{
	UT_sint32 iImageWidth;
	UT_sint32 iImageHeight;

	// Get the size of the jpeg image. This is a bit of a performance
	// issue, as it starts decoding the jpeg (though it does not actually decode it)
	// while we also do that in UT_JPEG_getRGBData below.
	// This however allows us to decode the image data directly into the DIB we
	// create below, which saves us a potentially huge memory copy.
	if (!UT_JPEG_getDimensions(pBB, iImageWidth, iImageHeight))
		return false;

	UT_uint32 iBytesInRow = iImageWidth * 3;
	if (iBytesInRow % 4)
	{
		iBytesInRow += (4 - (iBytesInRow % 4));
	}

	m_pDIB = (BITMAPINFO*) g_try_malloc(sizeof(BITMAPINFOHEADER) + iImageHeight * iBytesInRow);
	if (!m_pDIB)
		return false;

	/*
	  Note that we do NOT create a DIB of iDisplayWidth,iDisplayHeight, since
	  DIBs can be stretched automatically by the Win32 API.  So we simply remember
	  the display size for drawing later.
	*/

	setDisplaySize(iDisplayWidth, iDisplayHeight);

	m_pDIB->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	m_pDIB->bmiHeader.biWidth = iImageWidth;
	m_pDIB->bmiHeader.biHeight = iImageHeight;
	m_pDIB->bmiHeader.biPlanes = 1;
	m_pDIB->bmiHeader.biBitCount = 24;
	m_pDIB->bmiHeader.biCompression = BI_RGB;
	m_pDIB->bmiHeader.biSizeImage = 0;
	m_pDIB->bmiHeader.biXPelsPerMeter = 0;
	m_pDIB->bmiHeader.biYPelsPerMeter = 0;
	m_pDIB->bmiHeader.biClrUsed = 0;
	m_pDIB->bmiHeader.biClrImportant = 0;

	UT_Byte* pBuf = ((unsigned char*) m_pDIB) + m_pDIB->bmiHeader.biSize;
	
	if (!UT_JPEG_getRGBData(pBB, pBuf, iBytesInRow, true, true))
	{
		FREEP(m_pDIB);
		return false;
	}
	
	return true;
}
