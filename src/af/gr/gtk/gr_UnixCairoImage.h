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

#ifndef GR_UNIXCAIRO_IMAGE_H
#define GR_UNIXCAIRO_IMAGE_H

#include "gr_CairoImage.h"
#include <gdk-pixbuf/gdk-pixbuf.h>

class GR_UnixCairoImage : public GR_CairoImage
{
public:

	GR_UnixCairoImage(const char *name, GdkPixbuf *pixbuf);
	GdkPixbuf *	getPixbuf(void);

private:
	GR_UnixCairoImage();
	GR_UnixCairoImage(const GR_UnixCairoImage &other);
	GR_UnixCairoImage& operator=(const GR_UnixCairoImage &other);
};

#endif
