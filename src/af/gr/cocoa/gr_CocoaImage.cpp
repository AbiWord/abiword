/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 2001-2002, 2009 Hubert Figuiere
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
#include <math.h>

#include "ut_assert.h"
#include "ut_bytebuf.h"
#include "ut_debugmsg.h"
#include "ut_std_string.h"

#import "xap_CocoaAbiConversions.h"

#include "gr_Graphics.h"
#include "gr_CocoaImage.h"


GR_CocoaImage::GR_CocoaImage(const char * szName)
  : m_surface(NULL),
    m_grtype(GRT_Raster) // Probably the safest default.
{
	setName (szName ? szName : "CocoaImage");
}

GR_CocoaImage::~GR_CocoaImage()
{
	if(m_surface)
	{
		cairo_surface_destroy(m_surface);
	}
}





void GR_CocoaImage::cairoSetSource(cairo_t *cr, double x, double y)
{
	cairo_set_source_surface(cr, m_surface, x, y);
}


bool		GR_CocoaImage::convertToBuffer(UT_ByteBuf** ppBB) const
{
	UT_ByteBuf* pBB = new UT_ByteBuf();

	UT_ASSERT(0);
//	[m_pngData convertToAbiByteBuf:pBB];
	
	*ppBB = pBB;
	
	return true;
}


void GR_CocoaImage::setSurface(cairo_surface_t *surface)
{
	if(m_surface) {
		cairo_surface_destroy(m_surface);
	}
	m_surface = cairo_surface_reference(surface);
}


GR_CairoRasterImage *GR_CocoaImage::makeSubimage(const std::string & n,
											  UT_sint32 x, UT_sint32 y,
											  UT_sint32 w, UT_sint32 h) const
{
	GR_CocoaImage * image = new GR_CocoaImage(n.c_str());
	cairo_surface_t * surface;
	
	surface = cairo_surface_create_similar(m_surface, cairo_surface_get_content(m_surface), w, h);
	cairo_t * cr = cairo_create(surface);
	cairo_set_source_surface(cr, m_surface, x, y);
	cairo_paint(cr);
	cairo_destroy(cr);
	
	image->setSurface(surface);
	return image;
}


/*! 
 * Returns true if pixel at point (x,y) in device units is transparent.
 * See gr_UnixImage.cpp for how it's done in GTK.
 */
bool	GR_CocoaImage::isTransparentAt(UT_sint32 /*x*/, UT_sint32 /*y*/)
{
	UT_ASSERT(0);
	return false;
}


/*!
 * Returns true if there is any transparency in the image.
 */ 
bool	GR_CocoaImage::hasAlpha(void) const
{
	if(m_surface) {
		return cairo_surface_get_content(m_surface) == CAIRO_CONTENT_COLOR_ALPHA;
	}
	return false;
}




class _PNG_read_state
{
public:
	_PNG_read_state(const UT_ByteBuf* pBuf)
		: pos(0)
		, buf(pBuf)
		{
		}
	unsigned int pos;
	const UT_ByteBuf* buf;
};

static
cairo_status_t _UT_ByteBuf_PNG_read(void *closure, unsigned char *data,  unsigned int length)
{
	_PNG_read_state * state = (_PNG_read_state*)closure;
	UT_ASSERT(state);
	UT_uint32 buflen = state->buf->getLength();
	if(state->pos >= buflen) {
		return CAIRO_STATUS_READ_ERROR;
	}
	if((buflen - state->pos) < length) {
		UT_DEBUGMSG(("short read\n"));
		return CAIRO_STATUS_READ_ERROR;		
	}
	const UT_Byte * p = state->buf->getPointer(state->pos);
	memcpy(data, p, length);
	state->pos += length;
	
	return CAIRO_STATUS_SUCCESS;
}


cairo_surface_t * _rescaleTo(cairo_surface_t * surf, double width, double height)
{
	cairo_surface_t * dest;
#if 0
	dest = cairo_surface_create_similar(surf, 
	                                       CAIRO_CONTENT_COLOR_ALPHA, 
	                                       width, height);
	UT_ASSERT(CAIRO_SURFACE_TYPE_IMAGE == cairo_surface_get_type(surf));
	double ow = cairo_image_surface_get_width(surf);
	double oh = cairo_image_surface_get_height(surf);
	cairo_t *cr = cairo_create(dest);
	cairo_set_source_surface(cr, dest, 0, 0);
	cairo_scale(cr, width/ow, height/oh);
	cairo_paint(cr);
	cairo_destroy(cr);
#else
// NO-OP as this do not work.
	cairo_surface_reference(surf);
	dest = surf;
#endif
	return dest;
}

bool	GR_CocoaImage::convertFromBuffer(const UT_ByteBuf* pBB, const std::string & mimetype, 
                                         UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight)
{
	const char *buffer = (const char *) pBB->getPointer(0);
	UT_uint32 buflen = pBB->getLength();


	if(mimetype == "image/png") {

		if (buflen < 6) {
			return false;
		}

		char str1[10] = "\211PNG";
		char str2[10] = "<89>PNG";

		if ( !(strncmp(buffer, str1, 4)) || !(strncmp(buffer, str2, 6)) )
		{
			m_grtype = GRT_Raster;
			if(m_surface) {
				cairo_surface_destroy(m_surface);
			}
			
			_PNG_read_state closure(pBB);
			m_surface = cairo_image_surface_create_from_png_stream (&_UT_ByteBuf_PNG_read, &closure);
			if(CAIRO_SURFACE_TYPE_IMAGE == cairo_surface_get_type(m_surface)) 
			{
				if((cairo_image_surface_get_width(m_surface) != iDisplayWidth) ||
					(cairo_image_surface_get_height(m_surface) != iDisplayHeight)) {
					// needs resize.
					
					cairo_surface_t *rescaled = _rescaleTo(m_surface, iDisplayWidth, iDisplayHeight);
					cairo_surface_destroy(m_surface);
					m_surface = rescaled;
				}
			}
			setDisplaySize(iDisplayWidth, iDisplayHeight);
			return true;
		}
	}
	// Otherwise, assume SVG. Do scaling when drawing; save size for then:
	m_grtype = GRT_Vector;

	setDisplaySize(iDisplayWidth, iDisplayHeight);

	return true;
}



bool GR_CocoaImage::render(GR_Graphics * /*pGR*/, UT_sint32 /*iDisplayWidth*/, UT_sint32 /*iDisplayHeight*/)
{
	UT_DEBUGMSG(("Choosing not to render what can't be a raster image!\n"));
	
	return false;
}

