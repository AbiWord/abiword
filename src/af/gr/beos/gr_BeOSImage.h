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

#ifndef GR_BEOSIMAGE_H
#define GR_BEOSIMAGE_H

#include "gr_Image.h"

class GR_BeOSImage : public GR_RasterImage
{
public:
	GR_BeOSImage(const char* pszName);
	~GR_BeOSImage();

	virtual UT_sint32	getDisplayWidth(void) const;
	virtual UT_sint32	getDisplayHeight(void) const;
	virtual bool		convertToBuffer(UT_ByteBuf** ppBB) const;
	virtual bool		convertFromBuffer(const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight);

	void			setData(BBitmap * image) { m_image = image; }
   	BBitmap *		getData(void) const { return m_image; }
	
protected:

	BBitmap *m_image;
};

#endif /* GR_BEOSIMAGE_H */
