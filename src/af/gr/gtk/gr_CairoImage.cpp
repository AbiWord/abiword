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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  
 * 02110-1301 USA.
 */

#include "gr_CairoImage.h"
#include "gr_UnixImage.h"
#include "ut_assert.h"
#include "ut_bytebuf.h"
#include "ut_debugmsg.h"
#include "gr_CairoGraphics.h"

#include <stdlib.h>

GR_RSVGVectorImage::GR_RSVGVectorImage(const char* name) 
	: GR_CairoVectorImage(),
	  m_data(new UT_ByteBuf),
	  m_graphics(0),
	  m_surface(0), 
	  m_image_surface(0), 
	  m_svg(0), 
	  m_scaleX(1.0), 
	  m_scaleY(1.0), 
	  m_needsNewSurface(false),
	  m_rasterImage(NULL)
{
	if (name)
		{
			setName(name);
		}
	else
		{
			setName("SVGImage");
		}
}

GR_RSVGVectorImage::~GR_RSVGVectorImage()
{
	reset();
}

bool GR_RSVGVectorImage::convertToBuffer(UT_ConstByteBufPtr & pBB) const
{
	UT_ByteBufPtr bb(new UT_ByteBuf);

	bool bCopied = bb->append(m_data->getPointer(0), m_data->getLength());

	if (bCopied) {
		pBB = bb;
	}

	return bCopied;
}

bool GR_RSVGVectorImage::convertFromBuffer(const UT_ConstByteBufPtr & pBB,
                                           const std::string& /*mimetype*/,
										   UT_sint32 iDisplayWidth, 
										   UT_sint32 iDisplayHeight) {
	reset();
	
	m_data->append(pBB->getPointer(0), pBB->getLength());
	
	bool forceScale = (iDisplayWidth != -1 && iDisplayHeight != -1);
	
	gboolean result;
	
	m_svg = rsvg_handle_new();
		
	result = rsvg_handle_write(m_svg, pBB->getPointer(0), pBB->getLength(), NULL);
	if (!result) {
		g_object_unref(G_OBJECT(m_svg));
		m_svg = 0;
		
		return false;
	}
	
	result = rsvg_handle_close(m_svg, NULL);
	
	if (!result) {
		g_object_unref(G_OBJECT(m_svg));
		m_svg = 0;
		
		return false;
	}
	
	rsvg_handle_get_dimensions(m_svg, &m_size);
	
	if (!forceScale)
		setupScale(m_size.width, m_size.height);
	else
		setupScale(iDisplayWidth, iDisplayHeight);

	
	return true;
}

void GR_RSVGVectorImage::cairoSetSource(cairo_t *cr)
{
	createSurface(cr);
	if (m_surface == NULL) 
    {
		return;
	}
	
	cairo_set_source_surface(cr, m_surface, 0, 0);
}

void GR_RSVGVectorImage::scaleImageTo(GR_Graphics * pG, const UT_Rect & rec)
{	
	setupScale(pG->tdu(rec.width), pG->tdu(rec.height));
}

void GR_RSVGVectorImage::reset() 
{
	m_data->truncate(0);
	if (m_svg) 
	{
		g_object_unref(G_OBJECT(m_svg));
		m_svg = 0;
	}
	
	if (m_surface) 
    {
		cairo_surface_destroy(m_surface);
		m_surface = 0;
	}

	if (m_image_surface) {
		cairo_surface_destroy(m_image_surface);
		m_image_surface = 0;
	}
	
	m_scaleX = m_scaleY = 1.0;
	m_graphics = 0;
	m_needsNewSurface = false;
	memset(&m_size, 0, sizeof(RsvgDimensionData));
	DELETEP(m_rasterImage);
}

void GR_RSVGVectorImage::setupScale(UT_sint32 w, UT_sint32 h) {
	setDisplaySize(w, h);
	
	m_scaleX = (double)w / m_size.width;
	m_scaleY = (double)h / m_size.height;
	
	m_needsNewSurface = true;
}

void GR_RSVGVectorImage::renderToSurface(cairo_surface_t* surf) {
	cairo_t* cr = cairo_create(surf);
	cairo_scale(cr, m_scaleX, m_scaleY);
	rsvg_handle_render_cairo(m_svg, cr);
	//
	// Setup Raster Image too
	//
	UT_String name;
	getName(name);
	DELETEP(m_rasterImage);
	m_rasterImage = new GR_UnixImage(name.c_str(), rsvg_handle_get_pixbuf(m_svg));
	m_rasterImage->scale(getDisplayWidth(), getDisplayHeight());
	cairo_destroy(cr);
}

void GR_RSVGVectorImage::renderToCairo(cairo_t* cr) {
	cairo_scale(cr, m_scaleX, m_scaleY);
	rsvg_handle_render_cairo(m_svg, cr);
	cairo_new_path(cr);
}

void GR_RSVGVectorImage::createImageSurface() {
	if (!m_needsNewSurface)
		return;

	if (m_image_surface != 0) 
    { // get rid of any previous surface
		cairo_surface_destroy(m_image_surface);
		m_image_surface = 0;
	}

	m_image_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32,
											   getDisplayWidth(),
											   getDisplayHeight());

	renderToSurface(m_image_surface);
	m_needsNewSurface = false;
}

void GR_RSVGVectorImage::createSurface(cairo_t* cairo) {
	if (!m_needsNewSurface && cairo == m_graphics)
		return; // already have a similar surface for this graphics at this size
	
	if (m_surface != 0) { // get rid of any previous surface
		cairo_surface_destroy(m_surface);
		m_surface = 0;
	}
	
	m_surface = cairo_surface_create_similar(cairo_get_target(cairo), 
										   CAIRO_CONTENT_COLOR_ALPHA,
										   getDisplayWidth(),
										   getDisplayHeight());
	
	// render to the similar surface once. blit subsequently.
	renderToSurface(m_surface);
	createImageSurface();
}

bool GR_RSVGVectorImage::hasAlpha(void) const
{
	return true; // assume that any SVG could contain an alpha channel
}

bool GR_RSVGVectorImage::isTransparentAt(UT_sint32 x, UT_sint32 y)
{
	if(!hasAlpha())
	{
		return false;
	}

	if (!m_image_surface)
		createImageSurface();
	UT_return_val_if_fail(m_image_surface,false);
	UT_return_val_if_fail(cairo_image_surface_get_format(m_image_surface) == CAIRO_FORMAT_ARGB32, false);

	UT_sint32 iRowStride = cairo_image_surface_get_stride(m_image_surface);
	UT_sint32 iWidth = cairo_image_surface_get_width(m_image_surface);
	UT_sint32 iHeight = cairo_image_surface_get_height(m_image_surface);
	UT_ASSERT(iRowStride/iWidth == 4);
	UT_return_val_if_fail((x>= 0) && (x < iWidth), false);
	UT_return_val_if_fail((y>= 0) && (y < iHeight), false);

	guchar * pData = cairo_image_surface_get_data(m_image_surface);
	UT_sint32 iOff = iRowStride*y;
	guchar pix0 = pData[iOff+ x*4];
	if(pix0 == 0) // the data is not pre-multiplied, ARGB. if the first 8bits are 0, the image is fully transparent
	{
		return true;
	}
	return false;
}

GR_Image *GR_RSVGVectorImage::createImageSegment(GR_Graphics * pG, const UT_Rect & rec)
{
#if 1
	// we need createImageSegment for converting inline images to positioned images via the context menu and for drawing on background images

	// TODO: can we draw the RsvgHandle to a cairo SVG surface, clip it, and return a new GR_RSVGVectorImage?
	// TODO: or do we just rasterize it?

	// For now we rasterize it
	if(!m_rasterImage || m_needsNewSurface)
	{
		createImageSurface();
	}
	return m_rasterImage->createImageSegment(pG, rec);
#else
	return NULL;
#endif
}
