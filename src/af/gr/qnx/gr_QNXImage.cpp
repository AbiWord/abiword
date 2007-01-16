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
#include <Pt.h>

#include "gr_QNXImage.h"
#include "gr_Graphics.h"
#include "ut_assert.h"
#include "ut_bytebuf.h"

GR_QNXImage::GR_QNXImage(const char* szName)
	: m_image(0),
		m_grtype(GRT_Raster),
		m_iDisplayWidth(0),
		m_iDisplayHeight(0)
{
	
	if (szName)
	{
		setName(szName);
	}
	else
	{
	  setName ("QNXImage");
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

static void _png_write(png_structp png_ptr, png_bytep data, png_size_t length)
{
	UT_ByteBuf* pBB = (UT_ByteBuf*) png_get_io_ptr(png_ptr);

	pBB->ins(pBB->getLength(), data, length);
}

static void _png_flush(png_structp /* png_ptr */)
{
}

GR_QNXImage::~GR_QNXImage()
{
	if (m_image)
	{
		PhReleaseImage(m_image);
		FREEP(m_image);
	}
}

UT_sint32	GR_QNXImage::getDisplayWidth(void) const
{
   if (m_image == 0) return m_iDisplayWidth;
   return m_image->size.w;
}

UT_sint32	GR_QNXImage::getDisplayHeight(void) const
{
   if (m_image == 0) return m_iDisplayHeight;
   return m_image->size.h;
}

bool		GR_QNXImage::convertToBuffer(UT_ByteBuf** ppBB) const
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
		
		return false;
	}

	// We want libpng to write to our ByteBuf, not stdio
	png_set_write_fn(png_ptr, (void *)pBB, _png_write, _png_flush);

	UT_uint32 iWidth = m_image->size.w;
	UT_uint32 iHeight = m_image->size.h;

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

	UT_Byte * pBits = ((unsigned char*) m_image->image);
	
	UT_Byte* pData = (UT_Byte*) g_try_malloc(iWidth * iHeight * 3);
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

bool	GR_QNXImage::convertFromBuffer(const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight)
{
	const char *buffer = (const char *) pBB->getPointer(0);
	UT_uint32 buflen = pBB->getLength();

	if (buflen < 6) return false;

	char str1[10] = "\211PNG";
	char str2[10] = "<89>PNG";

	if ( !(strncmp(buffer, str1, 4)) || !(strncmp(buffer, str2, 6)) )
	{
		m_grtype = GRT_Raster;
		return _convertPNGFromBuffer(pBB, iDisplayWidth, iDisplayHeight);
	}

	// Otherwise, assume SVG. Do scaling when drawing; save size for then:
	m_grtype = GRT_Vector;

	m_iDisplayWidth  = iDisplayWidth;
	m_iDisplayHeight = iDisplayHeight;

	return true;
}
bool GR_QNXImage::_convertPNGFromBuffer(const UT_ByteBuf *pBB, UT_sint32 iDisplayWidth,UT_sint32 iDisplayHeight)
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


//Note! We are creating a image of original size, then scale it using photon functions.
	setDisplaySize(iDisplayWidth, iDisplayHeight);
	// should NOT already be set
	UT_ASSERT(!m_image);

	if (m_image)
	{
		PhReleaseImage(m_image);
		FREEP(m_image);
	}

	m_image = PhCreateImage(NULL,width,height,Pg_IMAGE_DIRECT_888,NULL,NULL,NULL);

	UT_Byte* pBits = (UT_Byte *)m_image->image;

	for (; iInterlacePasses; iInterlacePasses--)
	{
		for (UT_uint32 iRow = 0; iRow < height; iRow++)
		{
			UT_Byte* pRow = (UT_Byte *)pBits + iRow * m_image->bpl;

			png_read_rows(png_ptr, &pRow, NULL, 1);
		}
	}
	m_image = PiResizeImage(m_image,NULL,iDisplayWidth,iDisplayHeight,Pi_FREE);
	m_image->image_tag = PtCRC((const char *)m_image->image,m_image->bpl*m_image->size.h);

	
	/* read rest of file, and get additional chunks in info_ptr - REQUIRED */
	png_read_end(png_ptr, info_ptr);

	/* clean up after the read, and g_free any memory allocated - REQUIRED */
	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

	return true;
}
void GR_QNXImage::scaleImageTo(GR_Graphics * pG, const UT_Rect & rec)
{
	UT_sint32 width = pG->tdu(rec.width);
	UT_sint32 height = pG->tdu(rec.height);
	if(((width == getDisplayWidth()) && (height == getDisplayHeight())) || width < 0 || height < 0)
	{
		return;
	}	

	m_image = PiResizeImage(m_image,NULL,width,height,Pi_FREE);
}

GR_Image * GR_QNXImage::createImageSegment(GR_Graphics * pG,const UT_Rect & rec)
{
	UT_sint32 x = pG->tdu(rec.left);
	UT_sint32 y = pG->tdu(rec.top);
	PhRect_t rect;
	if(x < 0)
	{
		x = 0;
	}
	if(y < 0)
	{
		y = 0;
	}
	UT_sint32 width = pG->tdu(rec.width);
	UT_sint32 height = pG->tdu(rec.height);
	UT_sint32 dH = getDisplayHeight();
	UT_sint32 dW = getDisplayWidth();
	if(height > dH)
	{
		height = dH;
	}
	if(width > dW)
	{
		width = dW;
	}
	if(x + width > dW)
	{
		width = dW - x;
	}
	if(y + height > dH)
	{
		height = dH - y;
	}
	if(width < 0)
	{
		x = dW -1;
		width = 1;
	}
	if(height < 0)
	{
		y = dH -1;
		height = 1;
	}
	UT_String sName("");
	getName(sName);
	UT_String sSub("");
	UT_String_sprintf(sSub,"_segemnt_%d_%d_%d_%d",x,y,width,height);
	sName += sSub;
	GR_QNXImage * pImage = new GR_QNXImage(sName.c_str());
	UT_ASSERT(m_image);

	rect.ul.x = x;
	rect.ul.y = y;
	rect.lr.x = x+width;
	rect.lr.y = y+height;	
	pImage->m_image = PiCropImage(m_image,&rect,0);
	return static_cast<GR_Image *>(pImage);	
}
