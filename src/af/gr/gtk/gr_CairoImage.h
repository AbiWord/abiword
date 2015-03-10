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

#ifndef GR_CAIROIMAGE_H
#define GR_CAIROIMAGE_H

#include <cairo.h>

#include "gr_CairoGraphics.h"
#include "ut_bytebuf.h"
#include "gr_UnixImage.h"
#include <librsvg/rsvg.h>

// we need to add this define because of deprecations
#define __RSVG_RSVG_H_INSIDE__
#include <librsvg/rsvg-cairo.h>
#undef __RSVG_RSVG_H_INSIDE__

class GR_RSVGVectorImage : public GR_CairoVectorImage {
 public:

  GR_RSVGVectorImage(const char* name);
  virtual ~GR_RSVGVectorImage();

  virtual bool convertToBuffer(UT_ByteBuf** ppBB) const;
  virtual bool convertFromBuffer(const UT_ByteBuf* pBB,
                 const std::string& mimetype,
				 UT_sint32 iDisplayWidth,
				 UT_sint32 iDisplayHeight);
  virtual void cairoSetSource(cairo_t *cr);
  virtual void scaleImageTo(GR_Graphics * pG, const UT_Rect & rec);

  virtual bool            hasAlpha(void) const;
  virtual bool            isTransparentAt(UT_sint32 x, UT_sint32 y);
  virtual GR_Image *  createImageSegment(GR_Graphics *pG, const UT_Rect &rec);

 private:
  void reset();
  void setupScale(UT_sint32 w, UT_sint32 h);
  void createSurface(cairo_t* cairo);
  void createImageSurface();
  void renderToSurface(cairo_surface_t* surf);
  void renderToCairo(cairo_t *cr);

  UT_ByteBuf m_data;
  RsvgDimensionData m_size;
  cairo_t* m_graphics;
  cairo_surface_t* m_surface;
  cairo_surface_t* m_image_surface;
  RsvgHandle* m_svg;
  double m_scaleX;
  double m_scaleY;
  bool m_needsNewSurface;
  GR_UnixImage * m_rasterImage;
};

#endif
