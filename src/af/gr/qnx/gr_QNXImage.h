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

#ifndef GR_QNXIMAGE_H
#define GR_QNXIMAGE_H

#include "gr_Image.h"
#include <Pt.h>

struct Fatmap
{
	int width;
	int height;

	// Always 24-bit pixel data
	unsigned char * data;
};

class GR_QNXImage : public GR_RasterImage
{
public:
	GR_QNXImage(const char* pszName);
	virtual ~GR_QNXImage();

	virtual UT_sint32	getDisplayWidth(void) const;
	virtual UT_sint32	getDisplayHeight(void) const;
	virtual bool		convertToBuffer(UT_ByteBuf** ppBB) const;
	virtual bool		convertFromBuffer(const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight);
	virtual void		scaleImageTo(GR_Graphics *pG,const UT_Rect &rec);
	virtual GR_Image *	createImageSegment(GR_Graphics *pG,const UT_Rect &);
   	void			setData(PhImage_t * image) { m_image = image; }
	PhImage_t *			getData(void) const { return m_image; }

private:
	
	PhImage_t * m_image;
	GRType m_grtype;

	UT_sint32 m_iDisplayWidth;
	UT_sint32 m_iDisplayHeight;

	bool _convertPNGFromBuffer(const UT_ByteBuf *pBB,UT_sint32 iDisplayWidth,UT_sint32 iDisplayHeight);
};

#endif /* GR_QNXIMAGE_H */
