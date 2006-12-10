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
	: m_hasAlpha(false), m_image(0)
{
	if (szName)
	{
	  setName ( szName );
	}
	else
	{
	  setName ( "PostScriptImage" );
	}
}

struct _bb
{
	const UT_ByteBuf* pBB;
	UT_uint32 iCurPos;
};

static void _png_read(png_structp png_ptr, png_bytep data, png_size_t length)
{
	struct _bb* p = static_cast<struct _bb*>(png_get_io_ptr(png_ptr));
	const UT_Byte* pBytes = p->pBB->getPointer(0);

	memcpy(data, pBytes + p->iCurPos, length);
	p->iCurPos += length;
}

static void
png_simple_error_callback(png_structp png_save_ptr,
                          png_const_charp error_msg)
{
  UT_DEBUGMSG(("Fatal error in PNG image file: %s", error_msg));

  longjmp (png_save_ptr->jmpbuf, 1);
}

static void
png_simple_warning_callback(png_structp png_save_ptr,
                            png_const_charp warning_msg)
{
  UT_DEBUGMSG(("Warning in PNG image file: %s", warning_msg));
}

static gboolean
setup_png_transformations(png_structp png_read_ptr, png_infop png_info_ptr,
                          png_uint_32* width_p, png_uint_32* height_p,
                          int* color_type_p, bool* has_alpha_p)
{
        png_uint_32 width, height;
        int bit_depth, color_type, interlace_type, compression_type, filter_type;
        int channels;
        
        /* Get the image info */

        /* Must check bit depth, since png_get_IHDR generates an 
           FPE on bit_depth 0.
        */
        bit_depth = png_get_bit_depth (png_read_ptr, png_info_ptr);
        if (bit_depth < 1 || bit_depth > 16) {	  
		UT_DEBUGMSG(("Bits per channel of PNG image is invalid."));
                return FALSE;
        }
        png_get_IHDR (png_read_ptr, png_info_ptr,
                      &width, &height,
                      &bit_depth,
                      &color_type,
                      &interlace_type,
                      &compression_type,
                      &filter_type);

        /* set_expand() basically needs to be called unless
           we are already in RGB/RGBA mode
        */
        if (color_type == PNG_COLOR_TYPE_PALETTE &&
            bit_depth <= 8) {

                /* Convert indexed images to RGB */
                png_set_expand (png_read_ptr);

        } else if (color_type == PNG_COLOR_TYPE_GRAY &&
                   bit_depth < 8) {

                /* Convert grayscale to RGB */
                png_set_expand (png_read_ptr);

        } else if (png_get_valid (png_read_ptr, 
                                  png_info_ptr, PNG_INFO_tRNS)) {

                /* If we have transparency header, convert it to alpha
                   channel */
                png_set_expand(png_read_ptr);
                
        } else if (bit_depth < 8) {

                /* If we have < 8 scale it up to 8 */
                png_set_expand(png_read_ptr);


                /* Conceivably, png_set_packing() is a better idea;
                 * God only knows how libpng works
                 */
        }

        /* If we are 16-bit, convert to 8-bit */
        if (bit_depth == 16) {
                png_set_strip_16(png_read_ptr);
        }

        /* If gray scale, convert to RGB */
        if (color_type == PNG_COLOR_TYPE_GRAY ||
            color_type == PNG_COLOR_TYPE_GRAY_ALPHA) {
                png_set_gray_to_rgb(png_read_ptr);
        }
        
        /* If interlaced, handle that */
        if (interlace_type != PNG_INTERLACE_NONE) {
                png_set_interlace_handling(png_read_ptr);
        }
        
        /* Update the info the reflect our transformations */
        png_read_update_info(png_read_ptr, png_info_ptr);
        
        png_get_IHDR (png_read_ptr, png_info_ptr,
                      &width, &height,
                      &bit_depth,
                      &color_type,
                      &interlace_type,
                      &compression_type,
                      &filter_type);

        *width_p = width;
        *height_p = height;
        *color_type_p = color_type;
	*has_alpha_p = (color_type & PNG_COLOR_MASK_ALPHA);
        
        /* Check that the new info is what we want */
        
        if (width == 0 || height == 0) {
		UT_DEBUGMSG(("Transformed PNG has zero width or height."));
                return FALSE;
        }

        if (bit_depth != 8) {
		UT_DEBUGMSG(("Bits per channel of transformed PNG is not 8."));
                return FALSE;
        }

        if ( ! (color_type == PNG_COLOR_TYPE_RGB ||
                color_type == PNG_COLOR_TYPE_RGB_ALPHA) ) {	  
		UT_DEBUGMSG(("Transformed PNG not RGB or RGBA."));
                return FALSE;
        }

        channels = png_get_channels(png_read_ptr, png_info_ptr);
        if ( ! (channels == 3 || channels == 4) ) {
		UT_DEBUGMSG(("Transformed PNG has unsupported number of channels, must be 3 or 4."));
                return FALSE;
        }
        return TRUE;
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

bool PS_Image::convertToBuffer(UT_ByteBuf ** /* ppBB */) const
{
	UT_ASSERT_HARMLESS(UT_SHOULD_NOT_HAPPEN);
	
	return false;
}

bool PS_Image::convertFromBuffer(const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight)
{
	png_structp png_ptr;
	png_infop info_ptr;
	gint i, ctype;
	png_uint_32 w, h;
	png_bytepp volatile rows = NULL;
	
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, static_cast<void*>(NULL),
					 png_simple_error_callback,
					 png_simple_warning_callback);
	
	if (png_ptr == NULL)
	  return false;
	
	/* Allocate/initialize the memory for image information.  REQUIRED. */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	  {	  
	    png_destroy_read_struct(&png_ptr, static_cast<png_infopp>(NULL), static_cast<png_infopp>(NULL));
	    return false;
	  }

	/* Set error handling if you are using the setjmp/longjmp method (this is
	 * the normal method of doing things with libpng).  REQUIRED unless you
	 * set up your own error handlers in the png_create_read_struct() earlier.
	 */
	if (setjmp(png_ptr->jmpbuf))
	  {
	    g_free(rows);
	    
	    /* Free all of the memory associated with the png_ptr and info_ptr */
	    png_destroy_read_struct(&png_ptr, &info_ptr, static_cast<png_infopp>(NULL));
	    
	    /* If we get here, we had a problem reading the file */
	    return false;
	  }
	
	struct _bb myBB;
	myBB.pBB = pBB;
	myBB.iCurPos = 0;
	
	png_set_read_fn(png_ptr, static_cast<void *>(&myBB), _png_read);
	
	/* The call to png_read_info() gives us all of the information from the
	 * PNG file before the first IDAT (image data chunk).  REQUIRED
	 */
	png_read_info(png_ptr, info_ptr);
	
	if(!setup_png_transformations(png_ptr, info_ptr, &w, &h, &ctype, &m_hasAlpha)) {
	  png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
	  return false;
        }
	
	UT_uint32 rowstride = w * (m_hasAlpha ? 4 : 3);
        
	/* Always align rows to 32-bit boundaries */
	rowstride = (rowstride + 3) & ~3;
	
	// should NOT already be set
	UT_ASSERT_HARMLESS(!m_image);
	
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
	m_image->width = w;
	m_image->height = h;

	// allocate for 3 or 4 bytes
	m_image->data = static_cast<guchar *>(UT_calloc(m_image->height * rowstride, 1));
	
	if (!m_image->data)
	  {
	    png_destroy_read_struct(&png_ptr, &info_ptr, static_cast<png_infopp>(NULL));
	    return false;
	  }
	
	UT_Byte * pBits = static_cast<UT_Byte *>(m_image->data);
	
	rows = g_new (png_bytep, h);
	
	for (i = 0; i < h; i++)
	  rows[i] = pBits + i * rowstride;
	
	png_read_image (png_ptr, rows);
        png_read_end (png_ptr, info_ptr);
	
	g_free(rows);

	/* clean up after the read, and free any memory allocated - REQUIRED */
	png_destroy_read_struct(&png_ptr, &info_ptr, static_cast<png_infopp>(NULL));

	return true;
}

bool PS_Image::isTransparentAt(UT_sint32 x, UT_sint32 y)
{
  UT_ASSERT_HARMLESS(0);
  if(!hasAlpha())
  {
    return false;
  }
  UT_sint32 iOff = y*4*m_image->width + x*4 + 3;
  guchar p = m_image->data[iOff];
  if(p == 255)
  {
    return true;
  }
  return false;
}
