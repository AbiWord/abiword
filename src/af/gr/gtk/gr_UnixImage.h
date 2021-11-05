/* AbiWord
 * Copyright (C) 2001-2002 Dom Lachowicz
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

#pragma once

#include <gdk-pixbuf/gdk-pixbuf.h>

#include "gr_CairoGraphics.h"

class ABI_EXPORT GR_UnixImage : public GR_CairoRasterImage
{
public:
	GR_UnixImage(const char* pszName);
	GR_UnixImage(const char* pszName, GdkPixbuf * pPixbif);
	GR_UnixImage(const char* pszName, GRType imageType);
	virtual ~GR_UnixImage();

	virtual bool		convertToBuffer(UT_ConstByteBufPtr & ppBB) const override;
	virtual bool		convertFromBuffer(const UT_ConstByteBufPtr& pBB,
                                          const std::string & mimetype,
                                          UT_sint32 iDisplayWidth,
                                          UT_sint32 iDisplayHeight) override;
	bool                saveToPNG(const char * szFile);
	virtual bool hasAlpha (void) const override;
	virtual UT_sint32  rowStride (void) const;
	virtual GR_Image::GRType getType(void) const override;
   	GdkPixbuf *			getData(void) const { return m_image; }
	void setData(GdkPixbuf *data)
	{ m_image = data;}
	virtual void        scaleImageTo(GR_Graphics * pG, const UT_Rect & rec) override;
	virtual bool isTransparentAt(UT_sint32 x, UT_sint32 y) override;
    void scale (UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight);
	virtual void cairoSetSource(cairo_t *) override;
protected:
	virtual GR_UnixImage *makeSubimage(const std::string & n,
											  UT_sint32 x, UT_sint32 y,
											  UT_sint32 w, UT_sint32 h) const override;
private:
	GdkPixbuf * m_image;
    GR_Image::GRType m_ImageType;
};
