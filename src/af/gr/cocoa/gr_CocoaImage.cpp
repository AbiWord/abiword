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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
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





void GR_CocoaImage::cairoSetSource(cairo_t *cr)
{
	UT_return_if_fail(m_surface);
	double scaleX = (double)getDisplayWidth() / (double)cairo_image_surface_get_width(m_surface);
	double scaleY = (double)getDisplayHeight() / (double)cairo_image_surface_get_height(m_surface);
	cairo_scale(cr, scaleX, scaleY);
	cairo_set_source_surface(cr, m_surface, 0, 0);
}


bool GR_CocoaImage::convertToBuffer(UT_ConstByteBufPtr & pBB) const
{
	UT_ByteBufPtr bb(new UT_ByteBuf);

	UT_ASSERT(0);
//	[m_pngData convertToAbiByteBuf:pBB];
	
	pBB = bb;
	
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


bool	GR_CocoaImage::convertFromBuffer(const UT_ConstByteBufPtr & pBB, const std::string & mimetype,
                                         UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight)
{
	const char *buffer = (const char *) pBB->getPointer(0);
	UT_uint32 buflen = pBB->getLength();


	// We explicitely do not scale the image on load, since cairo can do
	// that during rendering. Scaling during loading will result in lost
	// image information, which in turns results in bugs like
	// in bugs like #12183.
	// So simply remember the display size for drawing later.
	setDisplaySize(iDisplayWidth, iDisplayHeight);

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
			
			_PNG_read_state closure(pBB.get());
			m_surface = cairo_image_surface_create_from_png_stream (&_UT_ByteBuf_PNG_read, &closure);
		}
	} else {
		m_grtype = GRT_Vector;
	}

	return true;
}



bool GR_CocoaImage::render(GR_Graphics * /*pGR*/, UT_sint32 /*iDisplayWidth*/, UT_sint32 /*iDisplayHeight*/)
{
	UT_DEBUGMSG(("Choosing not to render what can't be a raster image!\n"));
	
	return false;
}

