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

#ifndef GR_IMAGE_H
#define GR_IMAGE_H

#include "ut_types.h"

#define	GR_IMAGE_MAX_NAME_LEN	63

class GR_Graphics;
class UT_ByteBuf;

class GR_Image
{
public:
	GR_Image();
	virtual ~GR_Image();
	
   	virtual void		setDisplaySize(UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight) { m_iDisplayWidth = iDisplayWidth; m_iDisplayHeight = iDisplayHeight; }
	
	virtual UT_sint32	getDisplayWidth(void) const { return m_iDisplayWidth; }
	virtual UT_sint32	getDisplayHeight(void) const { return m_iDisplayHeight; }

   	virtual UT_Bool		convertToBuffer(UT_ByteBuf** ppBB) const { return UT_FALSE; }
	virtual UT_Bool		convertFromBuffer(const UT_ByteBuf* pBB, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight) { return UT_FALSE; }

	void				getName(char* szName) const;

	virtual void		setLayoutSize(UT_sint32 iLayoutWidth, UT_sint32 iLayoutHeight) { m_iLayoutWidth = iLayoutWidth; m_iLayoutHeight = iLayoutHeight; }
	UT_sint32			getLayoutWidth(void) const { return m_iLayoutWidth;}
	UT_sint32			getLayoutHeight(void) const { return m_iLayoutHeight;}
								
   	enum GRType {
	   GRT_Unknown,
	   GRT_Raster,
	   GRT_Vector
	};
   
	static GRType		getBufferType(const UT_ByteBuf* pBB);
   	virtual GRType		getType() { return GRT_Unknown; }
   	virtual UT_Bool		render(GR_Graphics *pGR, UT_sint32 iDisplayWidth, UT_sint32 iDisplayHeight) { return UT_FALSE; }

protected:
   	char			m_szName[GR_IMAGE_MAX_NAME_LEN+1];
	UT_sint32		m_iLayoutWidth;
	UT_sint32		m_iLayoutHeight;
	UT_sint32			m_iDisplayWidth;
	UT_sint32			m_iDisplayHeight;
};

class GR_RasterImage : public GR_Image
{
public:
   	virtual GRType		getType() { return GRT_Raster; }
};

class GR_ImageFactory
{
public:
   	virtual GR_Image*	createNewImage(const char* pszName, GR_Image::GRType iType = GR_Image::GRT_Raster) = 0;
};

#include "gr_VectorImage.h"

#endif /* GR_IMAGE */
