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

#ifndef GR_CAIROIMAGE_H
#define GR_CAIROIMAGE_H

#include <cairo.h>

#include "gr_Image.h"

class GR_CairoImage : public GR_RasterImage
{
public:
	GR_CairoImage(const char *pszName = 0);
	
	virtual ~GR_CairoImage();
	
	virtual bool convertToBuffer(UT_ByteBuf **ppBB) const;
	virtual bool convertFromBuffer(const UT_ByteBuf *pBB);
	virtual bool hasAlpha (void) const;
	virtual UT_sint32 rowStride (void) const;
	virtual GR_Image::GRType getType(void) const;
	virtual bool isTransparentAt(UT_sint32 x, UT_sint32 y);
	
	bool saveToPNG(const char * szURI);
	cairo_surface_t *getSurface();
	
private:
	GR_CairoImage();
	GR_CairoImage(const GR_CairoImage &other);
	GR_CairoImage& operator=(const GR_CairoImage &other);

	// todo: not sure about exposing this API
	void setSurface(cairo_surface_t *surface); 
	
	cairo_surface_t *m_image;
};

#endif
