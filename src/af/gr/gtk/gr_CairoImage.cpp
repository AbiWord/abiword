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
#include "gr_CairoGraphics.h"

#include <stdlib.h>

GR_RSVGVectorImage::GR_RSVGVectorImage(const char* name) 
	: GR_CairoVectorImage(), graphics(0), surface(0), svg(0), scaleX(1.0), scaleY(1.0), needsNewSurface(false)
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

bool GR_RSVGVectorImage::convertToBuffer(UT_ByteBuf** ppBB) const
{
	UT_ByteBuf* pBB = new UT_ByteBuf;
	
	bool bCopied = pBB->append(data.getPointer(0), data.getLength());
	
	if (!bCopied) DELETEP(pBB);
	
	*ppBB = pBB;
	
	return bCopied;
}

bool GR_RSVGVectorImage::convertFromBuffer(const UT_ByteBuf* pBB, 
										   UT_sint32 iDisplayWidth, 
										   UT_sint32 iDisplayHeight) {
	reset();
	
	data.append(pBB->getPointer(0), pBB->getLength());
	
	bool forceScale = (iDisplayWidth != -1 && iDisplayHeight != -1);
	
	gboolean result;
	
	svg = rsvg_handle_new();
		
	result = rsvg_handle_write(svg, pBB->getPointer(0), pBB->getLength(), NULL);
	if (!result) {
		g_object_unref(G_OBJECT(svg));
		svg = 0;
		
		return false;
	}
	
	result = rsvg_handle_close(svg, NULL);
	
	if (!result) {
		g_object_unref(G_OBJECT(svg));
		svg = 0;
		
		return false;
	}
	
	rsvg_handle_get_dimensions(svg, &size);
	
	if (!forceScale)
		setupScale(size.width, size.height);
	else
		setupScale(iDisplayWidth, iDisplayHeight);
	
	return true;
}

void GR_RSVGVectorImage::cairoSetSource(cairo_t *cr, double x, double y) {
	createSurface(cr);
	if (surface == NULL) {
		return;
	}
	
	cairo_set_source_surface(cr, surface, x, y);
	cairo_paint(cr);
}

void GR_RSVGVectorImage::scaleImageTo(GR_Graphics * pG, const UT_Rect & rec)
{	
	setupScale(pG->tdu(rec.width), pG->tdu(rec.height));
}

void GR_RSVGVectorImage::reset() {
	data.truncate(0);
	if (svg) {
		g_object_unref(G_OBJECT(svg));
		svg = 0;
	}
	
	if (surface) {
		cairo_surface_destroy(surface);
		surface = 0;
	}
	
	scaleX = scaleY = 1.0;
	graphics = 0;
	needsNewSurface = false;
	memset(&size, 0, sizeof(RsvgDimensionData));
}

void GR_RSVGVectorImage::setupScale(UT_sint32 w, UT_sint32 h) {
	setDisplaySize(w, h);
	
	scaleX = (double)w / size.width;
	scaleY = (double)h / size.height;
	
	needsNewSurface = true;
}

void GR_RSVGVectorImage::createSurface(cairo_t* cairo) {
	if (!needsNewSurface && cairo == graphics)
		return; // already have a similar surface for this graphics at this size
	
	if (surface != 0) { // get rid of any previous surface
		cairo_surface_destroy(surface);
		surface = 0;
	}
	
	surface = cairo_surface_create_similar(cairo_get_target(cairo), 
										   CAIRO_CONTENT_COLOR_ALPHA,
										   getDisplayWidth(),
										   getDisplayHeight());
	
	// render to the similar surface once. blit subsequently.
	cairo_t* cr = cairo_create(surface);
	cairo_scale(cr, scaleX, scaleY);
	rsvg_handle_render_cairo(svg, cr);
	cairo_destroy(cr);
}
