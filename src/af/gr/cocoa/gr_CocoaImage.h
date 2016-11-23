/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */

/* AbiWord
 * Copyright (C) 1998 AbiSource, Inc.
 * Copyright (C) 2001-2002, 2009 Hubert Figuiere
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

#ifndef GR_COCOAIMAGE_H
#define GR_COCOAIMAGE_H

#include <cairo.h>
#include <Cocoa/Cocoa.h>

#include "gr_CairoGraphics.h"
#include "gr_Image.h"


class GR_CocoaImage : public GR_CairoRasterImage
{
public:
	GR_CocoaImage(const char * pszName);
	virtual ~GR_CocoaImage();

	virtual bool		convertToBuffer(UT_ConstByteBufPtr & ppBB) const;
	virtual bool		convertFromBuffer(const UT_ConstByteBufPtr & pBB, const std::string & mimetype, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight);

	virtual bool hasAlpha (void) const;
	virtual bool isTransparentAt(UT_sint32 x, UT_sint32 y);

//	void			setData(Fatmap * image) { m_image = image; }

   	virtual GRType		getType() const { return m_grtype; }
   	virtual bool		render(GR_Graphics *pGR, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight);

//	void setFromImageRep(NSImageRep *imgRep);

	static NSImage * imageFromPNG (NSData * data, UT_uint32 & image_width, UT_uint32 & image_height);

	virtual void cairoSetSource(cairo_t *);

	// WILL retain the surface passed. You shall destroy it.
	void setSurface(cairo_surface_t *);
protected:
	virtual GR_CairoRasterImage *makeSubimage(const std::string & n,
											  UT_sint32 x, UT_sint32 y,
											  UT_sint32 w, UT_sint32 h) const;

private:
	cairo_surface_t * m_surface;
	GRType m_grtype;

//	bool _convertPNGFromBuffer(NSData* data, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight);

	bool _convertPNGFromBuffer(const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight);
};

#endif /* GR_COCOAIMAGE_H */
