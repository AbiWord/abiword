/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 2008 Dominic Lachowicz
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

#include "gr_CairoImage.h"
#include "ut_assert.h"
#include "ut_bytebuf.h"
#include "ut_debugmsg.h"
#include "gr_Graphics.h"

#include <stdlib.h>

/**
 * DAL: we may want this to hold any cairo surface type, in which case we'll need to
 * DAL: pay more attention to surface types
 */

GR_CairoImage::GR_CairoImage(const char* szName)
	: m_image(NULL)
{
	setName(szName ? szName : "GR_CairoImage");
}

GR_CairoImage::~GR_CairoImage()
{
	if (m_image)
		{
			cairo_surface_destroy(m_image);
		}
}

GR_Image::GRType GR_CairoImage::getType(void) const
{
	return GR_Image::GRT_Raster;
}

/**
 * Returns a reference to the underlying surface. It's the caller's responsibility
 * to release the reference after they're done using it.
 */
cairo_surface_t *GR_CairoImage::getSurface()
{
	if (!m_image)
		{
			return 0;
		}
	
	return cairo_surface_reference(m_image);
}

/**
 * Acquires a reference to the image surface @surface
 */
void GR_CairoImage::setSurface(cairo_surface_t *surface)
{
	if (m_image)
		{
			cairo_surface_destroy(m_image);
		}
	
	m_image = surface ? cairo_surface_reference(surface) : 0;
}

bool GR_CairoImage::hasAlpha(void) const
{
	UT_return_val_if_fail(m_image, false);
	UT_return_val_if_fail(cairo_surface_get_type(m_image) == CAIRO_SURFACE_TYPE_IMAGE, false);
	
	cairo_format_t format = cairo_image_surface_get_format(m_image);
	
	switch (format)
		{
		case CAIRO_FORMAT_ARGB32:
		case CAIRO_FORMAT_A8:
		case CAIRO_FORMAT_A1:
			return true;
		}
	
	return false;
}

UT_sint32 GR_CairoImage::rowStride(void) const
{
	UT_return_val_if_fail(m_image, 0);
	UT_return_val_if_fail(cairo_surface_get_type(m_image) == CAIRO_SURFACE_TYPE_IMAGE, 0);

	return cairo_image_surface_get_stride(m_image);
}

bool GR_CairoImage::saveToPNG(const char *szURI)
{
	UT_ByteBuf *pBB;
	
	if (convertToBuffer(&pBB))
		{
			bool res = pBB->writeToURI(szURI);
			delete pBB;
			
			return res;
		}
	
	return false;
}

/*! 
 * Returns true if pixel at point (x,y) in device units is transparent.
 */
bool GR_CairoImage::isTransparentAt(UT_sint32 x, UT_sint32 y)
{
	UT_return_val_if_fail(m_image, false);
	UT_return_val_if_fail(cairo_surface_get_type(m_image) == CAIRO_SURFACE_TYPE_IMAGE, false);
	
	if (!hasAlpha())
		{
			return false;
		}
	
	unsigned char *data;
	
	data = cairo_image_surface_get_data(m_image);
	UT_return_val_if_fail(data, false);
	
	int stride = cairo_image_surface_get_stride(m_image);
	int height = cairo_image_surface_get_height(m_image);
	int width  = cairo_image_surface_get_width(m_image);
	
	UT_return_val_if_fail((x >= 0) && (x < width), false);
	UT_return_val_if_fail((y >= 0) && (y < height), false);
	
	cairo_format_t format = cairo_image_surface_get_format(m_image);
	
	if (CAIRO_FORMAT_A1 == format)
		{
			UT_ASSERT(UT_TODO);
			return false;
		}
	else if (CAIRO_FORMAT_A8 == format)
		{
			UT_uint8 *b = &data[(stride * y) + (x * 4)];
			
			// each pixel is a 8-bit quantity holding an alpha value
			return (*b == 0xff);
		}
	else if (CAIRO_FORMAT_ARGB32 == format)
		{
			// each pixel is a 32-bit quantity, with alpha in the upper 8 bits, then red, then green, then blue. 
			// The 32-bit quantities are stored native-endian. Pre-multiplied alpha is used.
			UT_uint8 *b = &data[(stride * y) + (x * 4)];
			UT_uint32 pixel;
			UT_uint8  alpha;
			
			memcpy (&pixel, b, sizeof (uint32_t));
			alpha = (pixel & 0xff000000) >> 24;
			
			return (alpha == 0xff);
		}
	
	UT_ASSERT_NOT_REACHED();
	
	return false;  
}

static cairo_status_t write_to_png(void *closure,
								   const unsigned char *data,
								   unsigned int length)
{
	UT_ByteBuf *pBB = static_cast<UT_ByteBuf *>(closure);
	
	if (pBB->append(data, length))
		{
			return CAIRO_STATUS_SUCCESS;
		}
	
	return CAIRO_STATUS_WRITE_ERROR;
}

/*!
 * This method fills a byte buffer with a PNG representation of itself.
 * This can be saved in the PT as a data-item and recreated.
 * ppBB is a pointer to a pointer of a byte buffer. It's the callers
 * job to delete it.
 */
bool GR_CairoImage::convertToBuffer(UT_ByteBuf **ppBB) const
{
	*ppBB = 0;
	
	UT_return_val_if_fail(m_image, false);
	
	UT_ByteBuf *pBB = new UT_ByteBuf();
	if (CAIRO_STATUS_SUCCESS == cairo_surface_write_to_png_stream(m_image, write_to_png, pBB))
		{
			*ppBB = pBB;
			return true;
		}
	else
		{
			delete pBB;
			return false;
		}
}

struct CairoPngStreamClosure
{
	CairoPngStreamClosure(const UT_ByteBuf *bb)
		: pBB(bb), pos(0)
	{
	}
	
	const UT_ByteBuf *pBB;
	UT_uint32 pos;
};

static cairo_status_t read_from_png(void *closure,
									unsigned char *data,
									unsigned int length)
{
	CairoPngStreamClosure *bb = static_cast<CairoPngStreamClosure *>(closure);
	
	if ((length + bb->pos) > bb->pBB->getLength())
		{
			return CAIRO_STATUS_READ_ERROR;
		}
	
	memcpy(data, bb->pBB->getPointer(bb->pos), length);
	bb->pos += length;
	
	return CAIRO_STATUS_SUCCESS;
}

bool GR_CairoImage::convertFromBuffer(const UT_ByteBuf *pBB)
{
	// overwrites any previous image that might've been there

	CairoPngStreamClosure closure(pBB);
	cairo_surface_t *surface = cairo_image_surface_create_from_png_stream(read_from_png, &closure);
	setSurface(surface);
	
	return surface != 0;
}
