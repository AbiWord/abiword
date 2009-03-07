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

#include "gr_CairoGraphics.h"
#include "ut_bytebuf.h"

#include <librsvg/rsvg.h>
#include <librsvg/rsvg-cairo.h>

class GR_RSVGVectorImage : public GR_CairoVectorImage {
 public:

  GR_RSVGVectorImage(const char* name);
  virtual ~GR_RSVGVectorImage();

  virtual bool convertToBuffer(UT_ByteBuf** ppBB) const;
  virtual bool convertFromBuffer(const UT_ByteBuf* pBB, 
				 UT_sint32 iDisplayWidth, 
				 UT_sint32 iDisplayHeight);	
  virtual void cairoSetSource(cairo_t *cr, double x, double y);
  virtual void scaleImageTo(GR_Graphics * pG, const UT_Rect & rec);

 private:
  void reset();
  void setupScale(UT_sint32 w, UT_sint32 h);
  void createSurface(cairo_t* cairo);
	
  UT_ByteBuf data;
  RsvgDimensionData size;
  cairo_t* graphics;
  cairo_surface_t* surface;
  RsvgHandle* svg;
  double scaleX;
  double scaleY;
  bool needsNewSurface;
};

#endif
