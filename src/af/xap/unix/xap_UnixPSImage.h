/* AbiWord
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  
 * 02111-1307, USA.
 */

#ifndef XAP_UNIXPSIMAGE_H
#define XAP_UNIXPSIMAGE_H

#include "gr_Image.h"
#include <glib.h>

// This structure is similar to the 24-bit chunk of
// data the WP's Unix GR class uses to store images.
struct PSFatmap
{
	gint width;
	gint height;

	// Always 24-bit pixel data
	guchar * data;
};

class PS_Image : public GR_RasterImage
{
public:
	PS_Image(const char* pszName);
	~PS_Image();

	virtual bool		convertToBuffer(UT_ByteBuf** ppBB) const;
	virtual bool		convertFromBuffer(const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight);

   	void			setData(PSFatmap * image) { m_image = image; }
	PSFatmap *			getData(void) const { return m_image; }
	virtual GR_Image* createImageSegment(GR_Graphics*, const UT_Rect&)
		{ UT_ASSERT_NOT_REACHED(); return NULL; }

	virtual bool hasAlpha() const {
	  return m_hasAlpha;
	}
	virtual bool isTransparentAt(UT_sint32 x, UT_sint32 y);

private:

	bool m_hasAlpha;

	PSFatmap * m_image;
};

#endif /* XAP_UNIXPSIMAGE_H */
