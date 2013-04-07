/* -*- mode: C++; tab-width: 4; c-basic-offset: 4; -*- */
/* AbiWord
 * Copyright (C) 2004-6 Tomas Frydrych <dr.tomas@yahoo.co.uk>
 * Copyright (C) 2007 Martin Sevior<msevior@physics.unimelb.edu.au>
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

#ifndef GR_UNIX_PANGOPIXMAPGRAPHICS_H
#define GR_UNIX_PANGOPIXMAPGRAPHICS_H

#include "gr_UnixCairoGraphics.h"

class ABI_EXPORT GR_UnixPixmapAllocInfo : public GR_AllocInfo
{
public:
 	GR_UnixPixmapAllocInfo(GdkPixmap * pix)
		: m_pix(pix){}
	virtual GR_GraphicsId getType() const {return GRID_UNIX_PANGO_PIXMAP;}
	virtual bool isPrinterGraphics() const {return false;}

	GdkPixmap     * m_pix;
};

//
// Class to draw into offscreen Pixbuf
//
class ABI_EXPORT GR_UnixPangoPixmapGraphics : public GR_UnixCairoGraphics
{
	friend class GR_UnixImage;
public:
	virtual ~GR_UnixPangoPixmapGraphics();

	static UT_uint32       s_getClassId() {return GRID_UNIX_PANGO_PIXMAP;}
	virtual UT_uint32      getClassId() {return s_getClassId();}
	static const char *    graphicsDescriptor(){return "Unix Pango Pixmap";}
	static GR_Graphics *   graphicsAllocator(GR_AllocInfo&);
	virtual bool      canQuickPrint(void) const
	{ return true;}
	virtual void		scroll(UT_sint32, UT_sint32) {}
	virtual void		scroll(UT_sint32 /*x_dest*/, UT_sint32 /*y_dest*/,
							   UT_sint32 /*x_src*/, UT_sint32 /*y_src*/,
							   UT_sint32 /*width*/, UT_sint32 /*height*/) {}

	virtual void		setCursor(GR_Graphics::Cursor ){};
	virtual GR_Graphics::Cursor getCursor(void) const;
	virtual bool		queryProperties(GR_Graphics::Properties gp) const;

protected:
	GR_UnixPangoPixmapGraphics(GdkPixmap * pix);
	virtual GdkDrawable * _getDrawable(void)
	{  return static_cast<GdkDrawable *>(m_pPixmap);}
	GdkPixmap *       m_pPixmap;
};

#endif
